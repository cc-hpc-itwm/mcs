// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( ctx
    , mcs::core::transport::implementation::libfabric::libfabric::Name
    )
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT
    ( name
    , ctx, mcs::core::transport::implementation::libfabric::libfabric::Name
    )
  {
    return fmt::format_to (ctx.out(), "Libfabric {}", name.value());
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    (state, core::transport::implementation::libfabric::libfabric::Name)
  {
    prefix (state, "Libfabric");

    using Name
      = core::transport::implementation::libfabric::libfabric::Name
      ;

    return Name {parse<std::vector<std::byte>> (state)};
  }
}
