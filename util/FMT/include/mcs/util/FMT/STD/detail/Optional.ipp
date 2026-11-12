// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <tuple>
#include <type_traits>
#include <utility>

namespace fmt
{
  template<formattable T>
    template<typename ParseContext>
      constexpr auto formatter<mcs::util::FMT::STD::Optional<T>>::parse
        ( ParseContext& context
        )
  {
    return context.begin();
  }

  template<formattable T>
    template<typename FormatContext>
      constexpr auto formatter<mcs::util::FMT::STD::Optional<T>>::format
        ( mcs::util::FMT::STD::Optional<T> const& optional
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    if (optional._value.has_value())
    {
      return fmt::format_to
        ( context.out()
        , "Just {}"
        , std::make_tuple (optional._value.value())
        );
    }

    return fmt::format_to (context.out(), "Nothing");
  }
}

namespace mcs::util::FMT::STD
{
  template<typename T>
    constexpr Optional<T>::Optional (std::optional<T> optional) noexcept
      : _value {std::move (optional)}
  {
    static_assert (std::is_nothrow_move_constructible_v<std::optional<T>>);
  }
}

namespace mcs::util::FMT::STD::OPTIONAL
{
  template<typename T>
    auto format::operator()
      ( std::optional<T> const& optional
      ) const -> std::string
  {
    return fmt::format ("{}", Optional {optional});
  }
}
