// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdint>
#include <fmt/format.h>
#include <mcs/serialization/define.hpp>
#include <stdexcept>

namespace mcs::serialization
{
  namespace detail
  {
    template<class Variant, std::size_t I = 0>
      constexpr auto from_index (IArchive& ia, std::size_t index) -> Variant
    {
      if constexpr (I >= std::variant_size_v<Variant>)
      {
        throw std::logic_error
          {fmt::format ("Variant index '{}' out of bounds", I + index)};
      }
      else if (index == 0)
      {
        return load<std::variant_alternative_t<I, Variant>> (ia);
      }
      else
      {
        return from_index<Variant, I + 1> (ia, index - 1);
      }
    }
  }

  template<is_serializable... Ts>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , variant
      , std::variant<Ts...>
      )
  {
    oa.tag<detail::tag::STD::Variant> (variant.index());

    std::visit
      ( [&] (auto const& x)
        {
          save (oa, x);
        }
      , variant
      );

    return oa;
  }

  template<is_serializable... Ts>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , std::variant<Ts...>
      )
  {
    auto const index {ia.tag<detail::tag::STD::Variant>().index};
    return detail::from_index<std::variant<Ts...>> (ia, index);
  }
}
