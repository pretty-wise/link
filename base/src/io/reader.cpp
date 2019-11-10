/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/io/reader.h"
#include "base/io/stream.h"

namespace Base {

bool Reader::IsEOF() const { return m_stream.IsEOF(); }

} // namespace Base
