// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/config/cxx23_constexpr.hpp>
#include <mcs/rpc/Concepts.hpp>

namespace mcs::rpc::detail
{
  template<is_command... Commands>
    MCS_CXX23_CONSTEXPR auto make_handshake_data();
}

#include "detail/make_handshake_data.ipp"
