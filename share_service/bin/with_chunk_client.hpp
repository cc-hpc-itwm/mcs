// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/share_service/Chunk.hpp>

namespace mcs::share_service::bin
{
  template<typename Fun>
    auto with_chunk_client (share_service::Chunk const&, Fun const&);
}

#include "detail/with_chunk_client.ipp"
