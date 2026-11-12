// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::rpc::multi_client::detail
{
  constexpr auto CallID::operator++() noexcept -> CallID&
  {
    ++_value;

    return *this;
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::rpc::multi_client::detail::CallID>::parse
      (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::rpc::multi_client::detail::CallID>::format
      ( mcs::rpc::multi_client::detail::CallID const& id
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to (context.out(), "ci_{}", id._value);
  }
}
