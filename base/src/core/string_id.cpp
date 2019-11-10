/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/string_id.h"

namespace Base {

std::map<StringId, std::string> StringId::s_string_storage;

const StringId StringId::Invalid;

void StringId::RegisterString(StringId id, const char *string) {
  s_string_storage.insert(std::make_pair(id, string));
}

} // namespace Base
