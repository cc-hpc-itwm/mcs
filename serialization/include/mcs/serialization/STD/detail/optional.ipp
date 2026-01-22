// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  template<is_serializable T>
    auto Implementation<std::optional<T>>::output
      ( OArchive& oa
      , std::optional<T> const& opt
      ) -> OArchive&
  {
    oa.tag<detail::tag::STD::Optional> (opt.has_value());

    if (opt.has_value())
    {
      save (oa, opt.value());
    }

    return oa;
  }

  template<is_serializable T>
    auto Implementation<std::optional<T>>::input
      ( IArchive& ia
      ) -> std::optional<T>
  {
    auto has_value {ia.tag<detail::tag::STD::Optional>().has_value};

    if (has_value)
    {
      return load<T> (ia);
    }

    return {};
  }
}
