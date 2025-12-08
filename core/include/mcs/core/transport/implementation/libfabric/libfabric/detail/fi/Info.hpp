// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    struct Info
    {
      struct Deleter
      {
        auto operator() (Info*) const noexcept -> void;
      };

      Info (util::not_null<fi_info>);

      [[nodiscard]] auto get() const noexcept -> util::not_null<fi_info>;

    private:
      friend struct Deleter;
      util::not_null<fi_info> _value;
    };
  }

  using Info = std::unique_ptr<detail::Info, detail::Info::Deleter>;

  auto make_info (util::not_null<fi_info>) -> Info;
}
