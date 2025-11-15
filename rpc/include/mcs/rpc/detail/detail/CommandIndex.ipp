// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <type_traits>

namespace mcs::rpc::detail
{
  constexpr auto CommandIndex::operator++() noexcept -> CommandIndex&
  {
    ++_value;

    return *this;
  }
  constexpr auto CommandIndex::operator--() noexcept -> CommandIndex&
  {
    --_value;

    return *this;
  }

  template<typename T, typename Head, typename... Ts>
    constexpr auto command_index() noexcept -> CommandIndex
  {
    if constexpr (std::is_same_v<T, Head>)
    {
      return CommandIndex{};
    }
    else
    {
      return++ command_index<T, Ts...>();
    }
  }
}

namespace fmt
{
  template<> struct formatter<mcs::rpc::detail::CommandIndex>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext& ctx) const
    {
      return ctx.begin();
    }

    template<typename FormatContext>
      constexpr auto format
        ( mcs::rpc::detail::CommandIndex const& command_index
        , FormatContext& ctx
        ) const
    {
      return fmt::format_to
        (ctx.out(), "CommandIndex {}", command_index._value);
    }
  };
}
