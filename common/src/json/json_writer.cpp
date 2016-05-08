/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "common/json/json_writer.h"

namespace Link {

JsonWriter::JsonWriter(std::string &destination)
    : m_destination(destination), m_write_count(0), m_finalized(false) {
  // write the object start marker
  m_destination += "{";
}

JsonWriter::~JsonWriter() { Finalize(); }

void JsonWriter::WriteRaw(const char *name, const std::string &raw) {
  if(m_finalized)
    return;
  AppendName(name);
  m_destination += raw;
  m_write_count++;
}

void JsonWriter::WriteRaw(const char *name, const char *raw,
                          size_t raw_length) {
  WriteRaw(name, std::string(raw, raw_length));
}

void JsonWriter::AppendName(const char *name) {
  if(0 != m_write_count) {
    m_destination += ",";
  }

  m_destination += "\"";
  EscapeAppend(name, Base::String::strlen(name));
  m_destination += "\": ";
}

void JsonWriter::Finalize() {
  if(!m_finalized) {
    // write the object end marker
    m_destination += "}";
  }
  m_finalized = true;
}

void JsonWriter::EscapeAppend(const char *string_data, size_t string_length) {
  for(size_t i = 0; i < string_length; ++i) {
    if('"' == string_data[i]) {
      m_destination += '\\';
      m_destination += '"';
    } else if('\\' == string_data[i]) {
      m_destination += '\\';
      m_destination += '\\';
    } else if('/' == string_data[i]) {
      m_destination += '\\';
      m_destination += '/';
    } else if('\b' == string_data[i]) {
      m_destination += '\\';
      m_destination += 'b';
    } else if('\f' == string_data[i]) {
      m_destination += '\\';
      m_destination += 'f';
    } else if('\n' == string_data[i]) {
      m_destination += '\\';
      m_destination += 'n';
    } else if('\r' == string_data[i]) {
      m_destination += '\\';
      m_destination += 'r';
    } else if('\t' == string_data[i]) {
      m_destination += '\\';
      m_destination += 't';
    } else {
      m_destination += string_data[i];
    }
  }
}

} // namepsace Link
