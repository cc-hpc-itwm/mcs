// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::core::storage::c_api
{
  constexpr auto Traits<chunk::access::Const>::chunk_description
    ( ::mcs_core_storage const& storage
    )
  {
    return storage.chunk_const_description;
  }
  constexpr auto Traits<chunk::access::Mutable>::chunk_description
    ( ::mcs_core_storage const& storage
    )
  {
    return storage.chunk_mutable_description;
  }
}

namespace mcs::core::storage::c_api
{
  constexpr auto Traits<chunk::access::Const>::chunk_state
    ( ::mcs_core_storage const& storage
    )
  {
    return storage.chunk_const_state;
  }
  constexpr auto Traits<chunk::access::Mutable>::chunk_state
    ( ::mcs_core_storage const& storage
    )
  {
    return storage.chunk_mutable_state;
  }
}

namespace mcs::core::storage::c_api
{
  constexpr auto Traits<chunk::access::Const>::chunk_state_destruct
    ( ::mcs_core_storage const& storage
    )
  {
    return storage.chunk_const_state_destruct;
  }
  constexpr auto Traits<chunk::access::Mutable>::chunk_state_destruct
    ( ::mcs_core_storage const& storage
    )
  {
    return storage.chunk_mutable_state_destruct;
  }
}
