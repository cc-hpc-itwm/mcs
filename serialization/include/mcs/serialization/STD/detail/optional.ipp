// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>

namespace mcs::serialization
{
  template<is_serializable T>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , opt
      , std::optional<T>
      )
  {
    oa.tag<detail::tag::STD::Optional> (opt.has_value());

    if (opt.has_value())
    {
      save (oa, opt.value());
    }

    return oa;
  }

  template<is_serializable T>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , std::optional<T>
      )
  {
    auto has_value {ia.tag<detail::tag::STD::Optional>().has_value};

    if (has_value)
    {
      return load<T> (ia);
    }

    return {};
  }
}
