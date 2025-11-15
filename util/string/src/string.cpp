// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>
#include <mcs/util/hash/define.hpp>
#include <mcs/util/string.hpp>
#include <utility>

namespace mcs::util
{
  string::operator std::string() const
  {
    return _str;
  }
}

namespace std
{
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER (_str, mcs::util::string);
}

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    (oa, string, util::string)
  {
    save (oa, static_cast<std::string> (string));

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    (ia, util::string)
  {
    auto string {load<std::string> (ia)};

    return util::string {std::move (string)};
  }
}
