// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/State.hpp>
#include <mcs/util/read/read.hpp>

#define MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION_IMPL(_type...)   \
  struct Read<_type>                                                       \
  {                                                                        \
    template<typename Char>                                                \
      static auto read                                                     \
        ( State<Char>&                                                     \
        ) -> _type                                                         \
        ;                                                                  \
  }

#define MCS_UTIL_READ_DECLARE_INTRUSIVE_CTOR_IMPL(_type)                   \
  struct has_intrusive_ctor_from_read_State;                               \
  template<typename Char>                                                  \
    explicit _type (mcs::util::read::State<Char>&)
