/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "common/json/json_reader.h"
#include "base/core/assert.h"
#include "jsmn.h"
#include <stdlib.h>

namespace Link {

JsonReader::JsonReader()
    : m_json(0), m_json_length(0), m_tokens(0), m_token_count(0),
      m_parent_token(0), m_token_owner(false), m_parse_ok(false) {}

JsonReader::JsonReader(const JsonReader &other, unsigned int parent_token)
    : m_json(other.m_json), m_json_length(other.m_json_length),
      m_tokens(other.m_tokens), m_token_count(other.m_token_count),
      m_parent_token(parent_token), m_token_owner(false),
      m_parse_ok(other.m_parse_ok) {}

JsonReader::~JsonReader() {
  if(m_token_owner && m_tokens) {
    delete[] m_tokens;
  }
}

bool JsonReader::Parse(const char *json, size_t json_length) {
  if(m_tokens || m_token_count) {
    return false;
  }

  m_json = json;
  m_json_length = json_length;

  unsigned int token_array_length = static_cast<unsigned int>(json_length >> 1);
  m_tokens = new jsmntok_t[token_array_length];
  m_token_owner = true;

  jsmn_parser parser;
  jsmn_init(&parser);

  jsmnerr_t err =
      jsmn_parse(&parser, m_json, m_json_length, m_tokens, token_array_length);
  if(0 < err) {
    m_token_count = parser.toknext;
    m_parse_ok = 0 < m_token_count;
  }

  return m_parse_ok;
}

unsigned int JsonReader::FindValueToken(const char *name) const {
  FindVars vars;
  vars.m_found_token = m_token_count;
  vars.m_search = name;
  if(m_parse_ok && m_parent_token < m_token_count) {
    ProcessObject(vars, m_parent_token);
  }
  return vars.m_found_token;
}

unsigned int JsonReader::ProcessKeyValue(FindVars &vars,
                                         unsigned int index) const {
  if(index >= m_token_count) {
    BASE_ASSERT(index < m_token_count);
    return m_token_count; // force exit
  }

  const jsmntok_t &key_token = m_tokens[index++];
  if(JSMN_STRING != key_token.type) {
    BASE_ASSERT(JSMN_STRING == key_token.type);
    return m_token_count; // force exit
  }

  if(m_parent_token == static_cast<unsigned int>(key_token.parent)) {
    if(0 == Base::String::CompareN(vars.m_search, &m_json[key_token.start],
                                   key_token.end - key_token.start)) {
      vars.m_found_token = index;
      return m_token_count;
    }
  }

  const jsmntok_t &value_token = m_tokens[index];
  switch(value_token.type) {
  case JSMN_PRIMITIVE:
  // explicit fallthrough
  case JSMN_STRING:
    index++;
    break;
  case JSMN_ARRAY:
    index = ProcessArray(vars, index);
    break;
  case JSMN_OBJECT:
    index = ProcessObject(vars, index);
    break;
  default:
    index = m_token_count; // unknown type: force exit
    break;
  }
  return index;
}

unsigned int JsonReader::ProcessArray(FindVars &vars,
                                      unsigned int index) const {
  if(index >= m_token_count) {
    BASE_ASSERT(index < m_token_count);
    return m_token_count; // force exit
  }

  const jsmntok_t &array_token = m_tokens[index++];
  if(JSMN_ARRAY != array_token.type) {
    BASE_ASSERT(JSMN_ARRAY == array_token.type);
    return m_token_count; // force exit
  }

  for(int i = 0; i < array_token.size && index < m_token_count; ++i) {
    const jsmntok_t &element_token = m_tokens[index];
    switch(element_token.type) {
    case JSMN_PRIMITIVE:
    // explicit fallthrough
    case JSMN_STRING:
      index++;
      break;
    case JSMN_ARRAY:
      index = ProcessArray(vars, index);
      break;
    case JSMN_OBJECT:
      index = ProcessObject(vars, index);
      break;
    default:
      index = m_token_count; // unknown type: force exit
      break;
    }
  }
  return index;
}

unsigned int JsonReader::ProcessObject(FindVars &vars,
                                       unsigned int index) const {
  if(index >= m_token_count) {
    BASE_ASSERT(index < m_token_count);
    return m_token_count; // force exit
  }

  const jsmntok_t &object_token = m_tokens[index++];
  if(JSMN_OBJECT != object_token.type) {
    BASE_ASSERT(JSMN_OBJECT == object_token.type, "reading: %s", m_json);
    return m_token_count; // force exit
  }

  for(int i = 0; i < object_token.size && index < m_token_count; i += 2) {
    index = ProcessKeyValue(vars, index);
  }
  return index;
}

void JsonReader::UnescapeString(const jsmntok_t &token,
                                std::string *dest) const {
  //*dest = std::string(&m_json[t.start], t.end-t.start);
  for(int i = token.start; i < token.end; ++i) {
    if(i + 1 < token.end && '\\' == m_json[i]) {
      if('"' == m_json[i + 1]) {
        *dest += '"';
      } else if('\\' == m_json[i + 1]) {
        *dest += '\\';
      } else if('/' == m_json[i + 1]) {
        *dest += '/';
      } else if('b' == m_json[i + 1]) {
        *dest += '\b';
      } else if('f' == m_json[i + 1]) {
        *dest += '\f';
      } else if('n' == m_json[i + 1]) {
        *dest += '\n';
      } else if('r' == m_json[i + 1]) {
        *dest += '\r';
      } else if('t' == m_json[i + 1]) {
        *dest += '\t';
      } // else unexpected escape character
      ++i;
    } else {
      *dest += m_json[i];
    }
  }
}

void JsonReader::UnescapeString(const jsmntok_t &token, char *dest,
                                size_t dest_length) const {
  std::string temp_dest;
  UnescapeString(token, &temp_dest);
  size_t n_to_copy = temp_dest.length() + 1 < dest_length - 1
                         ? temp_dest.length() + 1
                         : dest_length - 1;
  Base::String::strncpy(dest, temp_dest.c_str(), (u32)n_to_copy);
  dest[n_to_copy] = 0;
}

} // namepsace Link
