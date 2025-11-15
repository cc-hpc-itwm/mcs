// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#define MCS_ERROR_COPY_MOVE_DEFAULT_IMPL(_name)         \
  ~_name() override;                                    \
  _name (_name const&) = default;                       \
  _name (_name&&) noexcept = default;                   \
  auto operator= (_name const&) -> _name& = default;    \
  auto operator= (_name&&) noexcept  -> _name& = default
