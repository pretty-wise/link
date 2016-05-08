/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "jsmn.h"

#include "base/core/str.h"

#include <string>
#include <vector>

namespace Link {

/// Utility class that parses a JSON document and allows for reading values
/// from the document by name. This class depends on the jsmn tokenizer which
/// is provided as a precompiled external library.
///
/// Sample Usage:
///	 JsonReader reader;
///	 if(reader.Parse(json_data, json_data_length)) {
///		 bool ok = true;
///		 ok = ok && reader.Read("key_name_1", &value1);
///		 ok = ok && reader.Read("key_name_2", &value2);
///		 ok = ok && reader.Read("key_name_3", &value3);
///		 ok = ok && reader.Read("key_name_4", &value4);
///		 value5 = reader.Read("key_name_5", 42);
///	 }
class JsonReader {
public:
  ///! Default constructor
  JsonReader();
  ///! Default destructor
  ~JsonReader();

  ///! Parses a JSON document that is stored in the memory pointed to by the
  ///! json parameter. NOTE: Do not delete or free the supplied JSON document.
  ///! The JsonReader instance does not copy any content of the JSON docuemnt
  ///! as a memory optimization. It simply stores references to offsets in the
  ///! document for later calls to Read().
  ///!
  ///! @param json Memory pointer to a JSON document.
  ///! @param json_length The length of the JSON document in characters.
  ///! @return True if the parse succeeded, false otherwise.
  bool Parse(const char *json, size_t json_length);

  ///! Template for reading a value primitive from the document by name.
  ///!
  ///! @param name The key name to search for in the document.
  ///! @param value A pointer to memory to store the result value.
  ///! @return True if the read attempt succeded, false otherwise. If the read
  ///! failed then the contents of value is undefined.
  template <class T> bool Read(const char *name, T *value);

  ///! Template for reading a value primitive from the document by name and
  ///! setting the return to a default value if the read fails.
  ///!
  ///! @param name The key name to search for in the document.
  ///! @param default_value A default value to return if the read fails.
  ///! @return The read value or the default_value if the read failed. Note
  ///! that there is no way to determine a failure if the read value happens
  ///! to match the default value.
  template <class T> T Read(const char *name, const T &default_value);

  ///! Specialized read method reading string value primitives.
  ///!
  ///! @param name The key name to search for in the document.
  ///! @param string_buffer A pointer to memory to store the result string.
  ///! @param string_buffer_length The length of string_buffer in characters.
  ///! @return True if the read attempt succeded, false otherwise. If the read
  ///! failed then the contents of string_buffer is undefined.
  bool Read(const char *name, char *string_buffer, size_t string_buffer_length);

  ///! Template for reading a vector of value primitives from the document by
  ///! name.
  ///!
  ///! @param name The key name to search for in the document.
  ///! @param values The vector where the results should be stored.
  ///! @return True if the read attempt succeded, false otherwise. If the read
  ///! failed then the contents of values is unchanged.
  template <class T> bool Read(const char *name, std::vector<T> &values);

  ///! Template for reading an array of value primitives from the document by
  ///! name.
  ///!
  ///! @param name The key name to search for in the document.
  ///! @param values The array where the results should be stored.
  ///! @param value_count The size of the values array in elements.
  ///! @return The number of primitives written to the values array or zero if
  ///! the read failed. If the source array in the document is larger than
  ///! value_count the result is truncated to fit within the specified values
  ///! array.
  template <class T>
  size_t Read(const char *name, T *values, size_t value_count);

private:
  struct FindVars {
    unsigned int m_found_token;
    const char *m_search;
  };

  unsigned int FindValueToken(const char *name) const;
  unsigned int ProcessKeyValue(FindVars &vars, unsigned int index) const;
  unsigned int ProcessArray(FindVars &vars, unsigned int index) const;
  unsigned int ProcessObject(FindVars &vars, unsigned int index) const;
  void UnescapeString(const jsmntok_t &token, std::string *dest) const;
  void UnescapeString(const jsmntok_t &token, char *dest,
                      size_t dest_length) const;

  const char *m_json;
  size_t m_json_length;
  jsmntok_t *m_tokens;
  unsigned int m_token_count;
  unsigned int m_parent_token;
  bool m_token_owner;
  bool m_parse_ok;

  JsonReader(const JsonReader &other, unsigned int parent_token);
  // no copying
  JsonReader(const JsonReader &);
  JsonReader &operator=(const JsonReader &);
};

template <class T> inline bool JsonReader::Read(const char *name, T *value) {
  unsigned int token_index = FindValueToken(name);
  if(m_token_count <= token_index) {
    return false;
  }
  const jsmntok_t &t = m_tokens[token_index];
  if(JSMN_PRIMITIVE != t.type) {
    return false;
  }
  Base::String::FromString(&m_json[t.start], *value);
  return true;
}

template <> inline bool JsonReader::Read(const char *name, bool *value) {
  unsigned int token_index = FindValueToken(name);
  if(m_token_count <= token_index) {
    return false;
  }
  const jsmntok_t &t = m_tokens[token_index];
  if(JSMN_PRIMITIVE != t.type) {
    return false;
  }
  if(0 == Base::String::CompareNoCase(&m_json[t.start], "true", 4)) {
    *value = true;
  } else if(0 == Base::String::CompareNoCase(&m_json[t.start], "false", 5)) {
    *value = false;
  } else {
    u8 typed_value = 0;
    Base::String::FromString(&m_json[t.start], typed_value);
    *value = typed_value != 0;
  }
  return true;
}

template <> inline bool JsonReader::Read(const char *name, std::string *value) {
  unsigned int token_index = FindValueToken(name);
  if(m_token_count <= token_index) {
    return false;
  }
  const jsmntok_t &t = m_tokens[token_index];
  if(JSMN_STRING == t.type) {
    UnescapeString(t, value);
  } else {
    *value = std::string(&m_json[t.start], t.end - t.start);
  }
  return true;
}

template <class T>
inline T JsonReader::Read(const char *name, const T &default_value) {
  T ret;
  if(!Read(name, &ret)) {
    ret = default_value;
  }
  return ret;
}

template <class T>
inline bool JsonReader::Read(const char *name, std::vector<T> &values) {
  unsigned int token_index = FindValueToken(name);
  if(m_token_count <= token_index) {
    return false;
  }
  const jsmntok_t &t = m_tokens[token_index];
  if(JSMN_ARRAY != t.type) {
    return false;
  }
  values.resize(t.size);
  for(int i = 0; i < t.size; i++) {
    const jsmntok_t &e = m_tokens[token_index + 1 + i];
    Base::String::FromString(&m_json[e.start], values[i]);
  }
  return true;
}

template <>
inline bool JsonReader::Read(const char *name,
                             std::vector<std::string> &values) {
  unsigned int token_index = FindValueToken(name);
  if(m_token_count <= token_index) {
    return false;
  }
  const jsmntok_t &t = m_tokens[token_index];
  if(JSMN_ARRAY != t.type) {
    return false;
  }
  values.resize(t.size);
  for(int i = 0; i < t.size; i++) {
    const jsmntok_t &e = m_tokens[token_index + 1 + i];
    if(JSMN_STRING == e.type) {
      UnescapeString(e, &values[i]);
    } else {
      values[i] = std::string(&m_json[e.start], e.end - e.start);
    }
  }
  return true;
}

inline bool JsonReader::Read(const char *name, char *string_buffer,
                             size_t string_buffer_length) {
  unsigned int token_index = FindValueToken(name);
  if(m_token_count <= token_index) {
    return false;
  }
  const jsmntok_t &t = m_tokens[token_index];
  if(JSMN_STRING == t.type) {
    UnescapeString(t, string_buffer, string_buffer_length);
  } else {
    unsigned int n_to_copy = t.end - t.start + 1;
    if(n_to_copy >= string_buffer_length) {
      n_to_copy = static_cast<unsigned int>(string_buffer_length - 1);
    }
    Base::String::strncpy(string_buffer, &m_json[t.start], n_to_copy);
    string_buffer[n_to_copy] = 0;
  }
  return true;
}

template <class T>
inline size_t JsonReader::Read(const char *name, T *values,
                               size_t value_count) {
  unsigned int token_index = FindValueToken(name);
  if(m_token_count <= token_index) {
    return 0;
  }
  const jsmntok_t &t = m_tokens[token_index];
  if(JSMN_ARRAY != t.type) {
    return 0;
  }
  int n = static_cast<int>(value_count) > t.size
              ? t.size
              : static_cast<int>(value_count);
  for(int i = 0; i < n; i++) {
    const jsmntok_t &e = m_tokens[token_index + 1 + i];
    Base::String::FromString(&m_json[e.start], values[i]);
  }
  return n;
}

} // namespace dcn
