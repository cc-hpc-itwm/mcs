// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdint>
#include <mcs/serialization/define.hpp>

namespace mcs::serialization
{
  namespace detail
  {
    template<std::size_t I, typename Tuple>
      auto input (IArchive& ia)
    {
      if constexpr (I < std::tuple_size<Tuple>{})
      {
        auto x {load<std::tuple_element_t<I, Tuple>> (ia)};

        return std::tuple_cat
          ( std::make_tuple (std::move (x))
          , input<I + 1, Tuple> (ia)
          );
      }
      else
      {
        return std::make_tuple();
      }
    }
  }

  template<is_serializable... Ts>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , tuple
      , std::tuple<Ts...>
      )
  {
    std::apply
      ( [&] (auto const&... ts) noexcept (noexcept ((save (oa, ts),...)))
        {
          (save (oa, ts),...);
        }
      , tuple
      );

    return oa;
  }

  template<is_serializable... Ts>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , std::tuple<Ts...>
      )
  {
    return detail::input<0, std::tuple<Ts...>> (ia);
  }
}
