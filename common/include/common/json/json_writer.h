/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
/*******************************************************************************
 * JsonWriter Definition
 *
 * (c) Copyright 2014 Ubisoft. All rights reserved.
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

#include "base/core/str.h"

namespace Link {

///! Utility class that writes a JSON document.
///!
///! Sample Usage:
///!	 std::string json;
///!	 JsonWriter writer(json);
///!	 writer.Write("A", 42);
///!	 writer.Write("B", 3.14f);
///!	 writer.Write("C", "See");
///!	 writer.Finalize();
///!	 ... use the contents of json ...
class JsonWriter {
public:
  ///! Default Constructor. Do not destruct the destination string while a
  ///! JsonWriter holds a reference to it.
  JsonWriter(std::string &destination);
  ///! Default Destructor
  ~JsonWriter();

  ///! Writes a key-value pair to the destination buffer.
  ///!
  ///! @param name The key name to write.
  ///! @param value The value to write.
  template <class T> void Write(const char *name, const T &value);

  ///! Writes a vector of values to the destination buffer as a key name and
  ///! JSON array.
  ///!
  ///! @param name The key name to write.
  ///! @param values The vector of values to write.
  template <class T> void Write(const char *name, const std::vector<T> &values);

  ///! Writes an array of values to the destination buffer as a key name and
  ///! JSON array.
  ///!
  ///! @param name The key name to write.
  ///! @param values The array of values to write.
  ///! @param value_count The number array of values to write.
  template <class T>
  void Write(const char *name, const T *values, size_t value_count);

  ///! Copies a raw value to the destination buffer without applying any JSON
  ///! formatting.
  ///!
  ///! @param name The key name to write.
  ///! @param raw The raw value to write.
  void WriteRaw(const char *name, const std::string &raw);

  ///! Copies a raw value to the destination buffer without applying any JSON
  ///! formatting.
  ///!
  ///! @param name The key name to write.
  ///! @param raw The raw value to write.
  ///! @param raw_length The length of the raw value to write in characters.
  void WriteRaw(const char *name, const char *raw, size_t raw_length);

  ///! Finalizes the JSON document by inserting any needed closing brackets
  /// and ! prevents further writing to the output buffer.
  void Finalize();

private:
  void AppendName(const char *name);
  void AppendValue(const char *value);
  template <class T> void AppendValue(const T &value);

  void EscapeAppend(const char *string_data, size_t string_length);

  std::string &m_destination;
  int m_write_count;
  bool m_finalized;

  // no copying
  JsonWriter(const JsonWriter &);
  JsonWriter &operator=(const JsonWriter &);
};

template <class T>
inline void JsonWriter::Write(const char *name, const T &value) {
  if(m_finalized)
    return;
  AppendName(name);
  AppendValue(value);
  m_write_count++;
}

template <class T>
inline void JsonWriter::Write(const char *name, const std::vector<T> &values) {
  Write(name, &values[0], values.size());
}

template <class T>
inline void JsonWriter::Write(const char *name, const T *values,
                              size_t value_count) {
  if(m_finalized)
    return;
  AppendName(name);
  m_destination += "[";
  for(size_t i = 0; i < value_count; ++i) {
    if(0 < i) {
      m_destination += ",";
    }
    AppendValue(values[i]);
  }
  m_destination += "]";
  m_write_count++;
}

template <>
inline void JsonWriter::Write(const char *name, const char *value,
                              size_t value_count) {
  if(m_finalized)
    return;
  AppendName(name);
  m_destination += "\"";
  EscapeAppend(value, value_count);
  m_destination += "\"";
  m_write_count++;
}

inline void JsonWriter::AppendValue(const char *value) {
  m_destination += "\"";
  EscapeAppend(value, Base::String::strlen(value));
  m_destination += "\"";
}

template <> inline void JsonWriter::AppendValue(const std::string &value) {
  m_destination += "\"";
  EscapeAppend(value.c_str(), value.length());
  m_destination += "\"";
}

template <> inline void JsonWriter::AppendValue(const bool &value) {
  if(value) {
    m_destination += "true";
  } else {
    m_destination += "false";
  }
}

template <class T> inline void JsonWriter::AppendValue(const T &value) {
  const u32 kTempLen = 64;
  char t[kTempLen];
  Base::String::ToString(value, t, kTempLen);
  m_destination += t;
}

} // namespace Link
