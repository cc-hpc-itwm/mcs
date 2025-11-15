// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#define MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION_IMPL(_state, _type...) \
  template<typename Char>                                                  \
    auto Read<_type>::read                                                 \
      ( State<Char>& _state                                                \
      ) -> _type
