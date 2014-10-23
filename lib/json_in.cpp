#include <jios/json_in.hpp>

#include <boost/core/null_deleter.hpp>

#include <json-c/json_tokener.h>
#include <json-c/json_object.h>
#include <json-c/linkhash.h>

using namespace std;

namespace jios {


//! interface to sharable iostream-like error state

class jin_state : boost::noncopyable
{
public:
  bool fail() const { return this->do_get_failbit(); }
  void set_failbit() { this->do_set_failbit(); }

private:
  virtual bool do_get_failbit() const = 0;
  virtual void do_set_failbit() = 0;
};

class istream_jin_state : public jin_state
{
public:
  istream_jin_state(shared_ptr<istream> const& p_is) : p_is_(p_is) {}

  istream & stream()
  {
    BOOST_ASSERT(p_is_);
    return *p_is_;
  }

private:
  shared_ptr<istream> p_is_;

  bool do_get_failbit() const override
  {
    BOOST_ASSERT(p_is_);
    return (p_is_ ? p_is_->fail() : true);
  }

  void do_set_failbit() override
  {
    BOOST_ASSERT(p_is_);
    if (p_is_) {
      p_is_->setstate(ios_base::failbit);
    }
  }
};

class jsonc_ijnode : public ijsource
{
protected:
  jsonc_ijnode() = delete;

  jsonc_ijnode(shared_ptr<istream_jin_state> const& p_is,
               json_object * p_node = NULL)
    : p_state_(p_is)
    , p_node_(p_node)
  {
  }

  ~jsonc_ijnode()
  {
    if (p_node_) {
      json_object_put(p_node_);
      p_node_ = NULL;
    }
  }

private:
  bool do_get_failbit() const override
  {
     return p_state_->fail();
  }

  void do_set_failbit() override
  {
    p_state_->set_failbit();
    if (p_node_) {
      json_object_put(p_node_);
      p_node_ = NULL;
    }
  }

  jios::json_type do_type() const override;

  void do_parse(int64_t & dest) override;
  void do_parse(double & dest) override;
  void do_parse(bool & dest) override;
  void do_parse(string & dest) override;

  ijarray do_begin_array() override;
  ijobject do_begin_object() override;

  bool do_is_terminator() const override
  {
    return !p_node_ || p_state_->fail();
  }

  virtual bool do_pending() { return false; }

protected:
  shared_ptr<istream_jin_state> p_state_;
  json_object * p_node_;
};

class jsonc_parsed_ijsource: public jsonc_ijnode
{
protected:
  jsonc_parsed_ijsource(shared_ptr<istream_jin_state> const& p_is,
                        json_object * p_parent)
      : jsonc_ijnode(p_is)
      , p_parent_(json_object_get(p_parent))
  {
    BOOST_ASSERT(p_parent_);
  }

  ~jsonc_parsed_ijsource() override
  {
    if (p_parent_) {
      json_object_put(p_parent_);
    }
  }

  json_object * const p_parent_;
};

class jsonc_array_ijsource : public jsonc_parsed_ijsource
{
  size_t idx_;

  string do_key() const override
  {
    BOOST_ASSERT(false);
    return string();
  }

  bool do_hint_multiline() const override
  {
    BOOST_ASSERT(json_object_is_type(p_parent_, json_type_array));
    return json_object_is_type(p_parent_, json_type_array)
           && json_object_array_length(p_parent_) > 1;
  }

  void do_advance() override
  {
    ++idx_;
    init();
  }

  void init()
  {
    BOOST_ASSERT(json_object_is_type(p_parent_, json_type_array));
    if (json_object_is_type(p_parent_, json_type_array)) {
      p_node_ = json_object_array_get_idx(p_parent_, idx_);
    } else {
      this->set_failbit();
    }
  }

public:
  jsonc_array_ijsource(shared_ptr<istream_jin_state> const& p_is,
                       json_object * p_parent)
    : jsonc_parsed_ijsource(p_is, p_parent)
    , idx_(0)
  {
    init();
  }
};

class jsonc_object_ijsource : public jsonc_parsed_ijsource
{
public:
  string do_key() const override
  {
    return string(p_member_ ? (char const*)p_member_->k : "");
  }

  bool do_hint_multiline() const override
  {
    BOOST_ASSERT(json_object_is_type(p_parent_, json_type_object));
    return json_object_is_type(p_parent_, json_type_object)
           && json_object_object_length(p_parent_) > 1;
  }

  jsonc_object_ijsource(shared_ptr<istream_jin_state> const& p_is,
                        json_object * p_parent)
    : jsonc_parsed_ijsource(p_is, p_parent)
    , p_member_(NULL)
  {
    BOOST_ASSERT(json_object_is_type(p_parent_, json_type_object));
    if (json_object_is_type(p_parent_, json_type_object)) {
      p_member_ = json_object_get_object(p_parent_)->head;
    }
    init();
  }

private:
  void init()
  {
    p_node_ = (p_member_ ? (struct json_object*)p_member_->v : NULL);
  }

  void do_advance() override;

  lh_entry * p_member_;
};

void jsonc_object_ijsource::do_advance()
{
  BOOST_ASSERT(p_parent_);
  if (p_parent_) {
    if (p_member_) {
      p_member_ = p_member_->next;
    }
  }
  init();
}

class ijsource_parser : public jsonc_ijnode
{
public:
  ijsource_parser(shared_ptr<istream_jin_state> const& p_is)
    : jsonc_ijnode(p_is)
  {}

  streamsize parse_some(const char* p, streamsize n)
  {
    return do_parse_some(p, n);
  }

  void compel_parse() { do_compel_parse(); }

private:
  virtual streamsize do_parse_some(const char* p, streamsize n) = 0;
  virtual void do_compel_parse() = 0;
};

class jsonc_parser_node : public ijsource_parser
{
public:
  jsonc_parser_node(shared_ptr<istream_jin_state> const& p_is)
    : ijsource_parser(p_is)
    , p_toky_(json_tokener_new())
  {
    if (!p_toky_) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
  }

  ~jsonc_parser_node()
   {
     if (p_toky_) {
       json_tokener_free(p_toky_);
     }
   }

private:
  streamsize do_parse_some(const char* p, streamsize n) override;
  void do_compel_parse() override;

  json_tokener * const p_toky_;
};

streamsize jsonc_parser_node::do_parse_some(const char* buf, streamsize len)
{
  p_node_ = json_tokener_parse_ex(p_toky_, buf, len);
  json_tokener_error jerr = json_tokener_get_error(p_toky_);
  if (jerr != json_tokener_continue && jerr != json_tokener_success) {
    set_failbit();
    json_tokener_reset(p_toky_);
    return 0;
  }
  if (p_toky_->char_offset == 0) {
    set_failbit();
  }
  return p_toky_->char_offset;
}

void jsonc_parser_node::do_compel_parse()
{
  p_node_ = json_tokener_parse_ex(p_toky_, "", -1);
  json_tokener_error jerr = json_tokener_get_error(p_toky_);
  if (jerr != json_tokener_success) {
    set_failbit();
    json_tokener_reset(p_toky_);
  }
}

class jsonc_root_ijnode : public jsonc_parser_node
{
public:
  string do_key() const override
  {
    BOOST_ASSERT(false);
    return string(); 
  }

  jsonc_root_ijnode(shared_ptr<istream> const& p_is)
    : jsonc_parser_node(make_shared<istream_jin_state>(p_is))
    , p_is_(p_state_)
    , buf_(4096)
    , bytes_avail_(0)
    , dirty_(false)
  {
    parse();
  }

  bool pending() { return do_pending(); }

private:
  void do_advance() override;
  bool do_pending() override;
  void parse();

  shared_ptr<istream_jin_state> p_is_;
  vector<char> buf_;
  streamsize bytes_avail_;
  bool dirty_;
};

void jsonc_root_ijnode::do_advance()
{
  if (p_node_) {
    json_object_put(p_node_);
    p_node_ = NULL;
  }
  if (!fail()) { parse(); }
}

void readsome_until_nonws(istream & is, vector<char> & buf, streamsize & count)
{
  if (!count && is.good()) {
    count = is.readsome(buf.data(), buf.size());
  }
  char const* it = buf.data();
  while (count > 0 && ::isspace(*it)) {
    ++it;
    --count;
    if (!count && is.good()) {
      count = is.readsome(buf.data(), buf.size());
      it = buf.data();
    }
  }
  if (count > 0 && it != buf.data()) {
    copy(it, it + count, buf.data());
  }
}

bool jsonc_root_ijnode::do_pending()
{
  BOOST_ASSERT(p_is_);
  if (!p_is_) return false;
  if (fail()) return false;
  istream & is = p_is_->stream();
  if (!p_node_) {
    readsome_until_nonws(is, buf_, bytes_avail_);
    if (bytes_avail_) {
      dirty_ = true;
    }
    while (bytes_avail_ > 0 && !p_node_ && !fail()) {
      streamsize parsed = parse_some(buf_.data(), bytes_avail_);
      if (!fail()) {
        bytes_avail_ -= parsed;
        if (bytes_avail_ > 0) {
          char const* rest = buf_.data() + parsed;
          copy(rest, rest + bytes_avail_, buf_.data());
        } else {
          bytes_avail_ = is.readsome(buf_.data(), buf_.size());
        }
      }
    }
    if (p_node_) {
      readsome_until_nonws(is, buf_, bytes_avail_);
      dirty_ = (bytes_avail_ > 0);
    }
  }
  return is.good() && !p_node_;
}

void jsonc_root_ijnode::parse()
{
  BOOST_ASSERT(!p_node_);
  istream & is = p_is_->stream();
  while (this->pending()) {
    is.peek();
  }
  if (is.eof() && dirty_) {
    BOOST_ASSERT(!p_node_);
    BOOST_ASSERT(!bytes_avail_);
    compel_parse();
    dirty_ = false;
  }
}

// jsonc_ijnode methods

jios::json_type jsonc_ijnode::do_type() const
{
  using jios::json_type;
  switch (json_object_get_type(p_node_)) {
    case json_type_null:    return json_type::jnull;
    case json_type_boolean: return json_type::jbool;
    case json_type_int:     return json_type::jinteger;
    case json_type_double:  return json_type::jfloat;
    case json_type_string:  return json_type::jstring;
    case json_type_array:   return json_type::jarray;
    case json_type_object:  return json_type::jobject;
  }
  BOOST_ASSERT(false);
  return json_type::jnull;
}

void jsonc_ijnode::do_parse(int64_t & dest)
{
  if (!json_object_is_type(p_node_, json_type_int)) {
    set_failbit();
    return;
  }
  dest = json_object_get_int64(p_node_);
}

void jsonc_ijnode::do_parse(double & dest)
{
  bool is_numeric = (json_object_is_type(p_node_, json_type_double)
                     || json_object_is_type(p_node_, json_type_int));
  if (!is_numeric) {
    set_failbit();
    return;
  }
  dest = json_object_get_double(p_node_);
}

void jsonc_ijnode::do_parse(bool & dest)
{
  if (!json_object_is_type(p_node_, json_type_boolean)) {
    set_failbit();
    return;
  }
  dest = json_object_get_boolean(p_node_);
}

void jsonc_ijnode::do_parse(string & dest)
{
  if (!p_node_) {
    set_failbit();
    return;
  }
  switch (json_object_get_type(p_node_)) {
    case json_type_string:
      {
        size_t len = json_object_get_string_len(p_node_);
        dest.resize(len);
        copy_n(json_object_get_string(p_node_), len, dest.begin());
      }
      break;
    case json_type_int:
      dest = json_object_to_json_string(p_node_);
      break;
    case json_type_double:
      dest = json_object_to_json_string(p_node_);
      break;
    case json_type_boolean:
      dest = json_object_to_json_string(p_node_);
      break;
    case json_type_null:
    case json_type_array:
    case json_type_object:
      set_failbit();
      break;
  }
}

ijarray jsonc_ijnode::do_begin_array()
{
  if (!this->is_array()) {
    set_failbit();
    return ijarray();
  }
  return shared_ptr<ijsource>(new jsonc_array_ijsource(p_state_, p_node_));
}

ijobject jsonc_ijnode::do_begin_object()
{
  if (!this->is_object()) {
    set_failbit();
    return ijobject();
  }
  return shared_ptr<ijsource>(new jsonc_object_ijsource(p_state_, p_node_));
}


// factory functions

ijstream json_in(shared_ptr<istream> const& p_is)
{
  return shared_ptr<ijsource>(new jsonc_root_ijnode(p_is));
}

ijstream json_in(istream & is)
{
  return json_in(shared_ptr<istream>(&is, boost::null_deleter()));
}


} // namespace

