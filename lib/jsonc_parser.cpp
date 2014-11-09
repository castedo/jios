#include <jios/jsonc_parser.hpp>

#include <json-c/json_tokener.h>
#include <json-c/json_object.h>
#include <json-c/linkhash.h>

using namespace std;

namespace jios {


class jsonc_value : public ijpair
{
public:
  jsonc_value(shared_ptr<ijstate> const& p_state,
              json_object * p_node = NULL)
    : p_state_(p_state)
    , p_node_(json_object_get(p_node))
  {}

  ~jsonc_value()
  {
    if (p_node_) {
      json_object_put(p_node_);
      p_node_ = NULL;
    }
  }

  void reset_already_refcounted(json_object * p_new)
  {
    if (p_node_) {
      json_object_put(p_node_);
    }
    p_node_ = p_new;
  }

  void reset(json_object * p_new= NULL)
  {
    reset_already_refcounted(json_object_get(p_new));
  }

  void set_key(string const& key) { key_ = key; }

  json_object * jsonc_ptr() { return p_node_; }

  bool is_empty() const { return !p_node_; }

private:
  bool parse(const char * & begin, size_t & len) const;

  ijstate & do_state() override { return *p_state_; }
  ijstate const& do_state() const override { return *p_state_; }

  jios::json_type do_type() const override;

  void do_parse(int64_t & dest) override;
  void do_parse(double & dest) override;
  void do_parse(bool & dest) override;
  void do_parse(string & dest) override;
  void do_parse(buffer_iterator dest) override;

  ijarray do_begin_array() override;
  ijobject do_begin_object() override;

  string do_key() const override { return key_; }

  shared_ptr<ijstate> p_state_;
  json_object * p_node_;
  string key_;
};

class jsonc_parsed_ijsource: public ijsource
{
protected:
  jsonc_parsed_ijsource(shared_ptr<ijstate> const& p_is,
                        json_object * p_parent)
      : value_(p_is)
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

  ijstate & do_state() override { return value_.state(); }
  ijstate const& do_state() const override { return value_.state(); }

  ijpair & do_ref() override { return value_; }

  bool do_is_terminator() override
  {
    return value_.is_empty();
  }

  bool do_expecting() override { return false; }

  jsonc_value value_;
  json_object * const p_parent_;
};

class jsonc_array_ijsource : public jsonc_parsed_ijsource
{
  size_t idx_;

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

  void do_init() override
  {
    idx_ = 0;
    init();
  }

  void init()
  {
    BOOST_ASSERT(json_object_is_type(p_parent_, json_type_array));
    if (json_object_is_type(p_parent_, json_type_array)) {
      value_.reset(json_object_array_get_idx(p_parent_, idx_));
    } else {
      value_.set_failbit();
    }
  }

public:
  jsonc_array_ijsource(shared_ptr<ijstate> const& p_is,
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
  bool do_hint_multiline() const override
  {
    BOOST_ASSERT(json_object_is_type(p_parent_, json_type_object));
    return json_object_is_type(p_parent_, json_type_object)
           && json_object_object_length(p_parent_) > 1;
  }

  jsonc_object_ijsource(shared_ptr<ijstate> const& p_is,
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
    value_.reset((p_member_ ? (struct json_object*)p_member_->v : NULL));
    value_.set_key(p_member_ ? (char const*)p_member_->k : "");
  }

  void do_advance() override;

  void do_init() override
  {
    BOOST_ASSERT( p_member_ == nullptr );
    p_member_ = nullptr;
  }

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

// jsonc_parser_facade

class jsonc_parser_facade
{
  json_tokener * const p_toky_;

public:
  jsonc_parser_facade()
    : p_toky_(json_tokener_new())
  {
    if (!p_toky_) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
  }

  ~jsonc_parser_facade()
  {
    if (p_toky_) {
      json_tokener_free(p_toky_);
    }
  }

  //! Parsing means more calls to parse_some or induce_parse
  //! are expected to complete partial parsing.
  bool parsing();

  //! Reinitialize parser to initial state for fresh new parsing.
  void clear();

  //! Either parse_some return an object or expecting is true.
  //! Return of nullptr with expecting() returning false means error.
  //! On success pointer 'it' is incremented by chars parsed.
  json_object * parse_some(const char * & it, streamsize len);

  //! Call only when expecting
  //! Return of nullptr means error.
  json_object * induce_parse();
};

bool jsonc_parser_facade::parsing()
{
  return json_tokener_get_error(p_toky_) == json_tokener_continue;
}

void jsonc_parser_facade::clear()
{
  BOOST_ASSERT( !parsing() );
  json_tokener_reset(p_toky_);
}

json_object * jsonc_parser_facade::parse_some(const char * & it, streamsize n)
{
  json_object * ret = json_tokener_parse_ex(p_toky_, it, n);
  json_tokener_error err = json_tokener_get_error(p_toky_);
  if (err != json_tokener_continue && err != json_tokener_success) {
    BOOST_ASSERT(!ret);
    json_tokener_reset(p_toky_);
  }
  if (p_toky_->char_offset >= 0) {
    it += p_toky_->char_offset;
  } else {
    BOOST_ASSERT(!ret);
    ret = nullptr;
  }
  return ret;
}

json_object * jsonc_parser_facade::induce_parse()
{
  BOOST_ASSERT(this->parsing());
  json_object * ret = nullptr;
  if (this->parsing()) {
    ret = json_tokener_parse_ex(p_toky_, "", -1);
    json_tokener_error err = json_tokener_get_error(p_toky_);
    if (err != json_tokener_success) {
      json_tokener_reset(p_toky_);
    }
    BOOST_ASSERT( bool(ret) == (err == json_tokener_success) );
  }
  return ret;
}

// jsonc_istream_parser

class jsonc_istream_parser : public istream_parser
{
public:
  jsonc_istream_parser(shared_ptr<ijstate> const& p_is)
    : value_(p_is)
  {
    if (!p_is) {
      BOOST_THROW_EXCEPTION(bad_alloc());
    }
  }

private:
  void do_clear() override;
  void do_parse(shared_ptr<istream_facade> const& p_is) override;
  bool do_is_parsed() const override { return !value_.is_empty(); }
  ijpair & do_result() override { return value_; }

  jsonc_parser_facade jsonc_;
  jsonc_value value_;
};

void jsonc_istream_parser::do_clear()
{
  jsonc_.clear();
  value_.reset();
}

void jsonc_istream_parser::do_parse(shared_ptr<istream_facade> const& p_is)
{
  istream_facade & is = *p_is;
  if (!jsonc_.parsing()) {
    is.eat_whitespace();
  }
  while (is.avail() > 0 && value_.is_empty() && !is.fail()) {
    const char * it = is.begin();
    value_.reset_already_refcounted(jsonc_.parse_some(it, is.avail()));
    if (value_.is_empty()) {
      BOOST_ASSERT( jsonc_.parsing() == (it != is.begin()) );
      if (!jsonc_.parsing() || it == is.begin()) {
        is.set_failbit();
      }
    }
    is.remove_until(it);
  }
  if (is.eof() && jsonc_.parsing()) {
    value_.reset_already_refcounted(jsonc_.induce_parse());
    if (value_.is_empty()) {
      is.set_failbit();
    }
  }
}

// jsonc_value methods

jios::json_type jsonc_value::do_type() const
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

void jsonc_value::do_parse(int64_t & dest)
{
  if (!json_object_is_type(p_node_, json_type_int)) {
    set_failbit();
    return;
  }
  dest = json_object_get_int64(p_node_);
}

void jsonc_value::do_parse(double & dest)
{
  bool is_numeric = (json_object_is_type(p_node_, json_type_double)
                     || json_object_is_type(p_node_, json_type_int));
  if (!is_numeric) {
    set_failbit();
    return;
  }
  dest = json_object_get_double(p_node_);
}

void jsonc_value::do_parse(bool & dest)
{
  if (!json_object_is_type(p_node_, json_type_boolean)) {
    set_failbit();
    return;
  }
  dest = json_object_get_boolean(p_node_);
}

void jsonc_value::do_parse(string & dest)
{
  const char * begin = nullptr;
  size_t len = 0;
  if (this->parse(begin, len)) {
    dest.assign(begin, begin + len);
  } else {
    this->set_failbit();
  }
}

void jsonc_value::do_parse(buffer_iterator dest)
{
  const char * begin = nullptr;
  size_t len = 0;
  if (this->parse(begin, len)) {
    copy(begin, begin + len, dest);
  } else {
    this->set_failbit();
  }
}

bool jsonc_value::parse(const char * & p, size_t & len) const
{
  switch (json_object_get_type(p_node_)) {
    case json_type_string:
      {
        len = json_object_get_string_len(p_node_);
        p = json_object_get_string(p_node_);
        return true;
      }
      break;
    case json_type_int:
      p = json_object_to_json_string(p_node_);
      break;
    case json_type_double:
      p = json_object_to_json_string(p_node_);
      break;
    case json_type_boolean:
      p = json_object_to_json_string(p_node_);
      break;
    case json_type_null:
    case json_type_array:
    case json_type_object:
      return false;
  }
  len = ::strlen(p);
  return true;
}

ijarray jsonc_value::do_begin_array()
{
  if (!this->is_array()) {
    set_failbit();
    return ijarray();
  }
  return shared_ptr<ijsource>(new jsonc_array_ijsource(p_state_,
                                                       jsonc_ptr()));
}

ijobject jsonc_value::do_begin_object()
{
  if (!this->is_object()) {
    set_failbit();
    return ijobject();
  }
  return shared_ptr<ijsource>(new jsonc_object_ijsource(p_state_,
                                                        jsonc_ptr()));
}

// factory function

shared_ptr<istream_parser>
    make_jsonc_parser(shared_ptr<istream_facade> const& p_is)
{
  return shared_ptr<istream_parser>(new jsonc_istream_parser(p_is));
}

shared_ptr<ijsource>
    make_jsonc_ijsource(shared_ptr<istream_facade> const& p_is)
{
  return make_stream_ijsource(p_is, make_jsonc_parser(p_is));
}


} // namespace jios

