// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::rpc::detail
{
  constexpr auto CallID::operator++() noexcept -> CallID&
  {
    ++_value;

    return *this;
  }

  constexpr auto CallID::operator++(int) noexcept -> CallID
  {
    auto old {CallID {*this}};

    ++_value;

    return old;
  }
}

namespace fmt
{
  template<> struct formatter<mcs::rpc::detail::CallID>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext& ctx) const
    {
      return ctx.begin();
    }

    template<typename FormatContext>
      constexpr auto format
        ( mcs::rpc::detail::CallID const& call_id
        , FormatContext& ctx
        ) const
    {
      return fmt::format_to (ctx.out(), "CallID {}", call_id._value);
    }
  };
}
