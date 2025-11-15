// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/detail/Implementation.fwd.hpp>

#define MCS_SERIALIZATION_ACCESS_IMPL()                                 \
  template<typename> friend struct serialization::Implementation
