// Copyright (C) 2022,2024-2026 Fraunhofer ITWM
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
  template<>
    struct formatter<mcs::rpc::detail::CallID>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext& context) const
    {
      return context.begin();
    }

    template<typename FormatContext>
      constexpr auto format
        ( mcs::rpc::detail::CallID const& call_id
        , FormatContext& context
        ) const
    {
      return fmt::format_to (context.out(), "CallID {}", call_id._value);
    }
  };
}
