// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter
      <mcs::core::transport::implementation::libfabric::libfabric::Name>::parse
        (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter
      <mcs::core::transport::implementation::libfabric::libfabric::Name>::format
        ( mcs::core::transport::implementation::libfabric::libfabric::Name const& name
        , FormatContext& ctx
        ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "Libfabric {}", name.value());
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<core::transport::implementation::libfabric::libfabric::Name>::read
      ( State<Char>& state
      ) -> core::transport::implementation::libfabric::libfabric::Name
  {
    prefix (state, "Libfabric");

    using Name
      = core::transport::implementation::libfabric::libfabric::Name
      ;

    return Name {parse<std::vector<std::byte>> (state)};
  }
}
