// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
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
  auto hash<mcs::util::string>::operator()
    ( mcs::util::string const& x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x._str);
  }
}

namespace mcs::serialization
{
  auto Implementation<util::string>::output
    ( OArchive& oa
    , util::string const& string
    ) -> OArchive&
  {
    save (oa, static_cast<std::string> (string));

    return oa;
  }
  auto Implementation<util::string>::input
    ( IArchive& ia
    ) -> util::string
  {
    auto string {load<std::string> (ia)};

    return util::string {std::move (string)};
  }
}
