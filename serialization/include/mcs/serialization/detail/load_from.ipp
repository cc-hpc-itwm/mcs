// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstddef>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/error/AdditionalBytes.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/read_file.hpp>
#include <span>
#include <stdexcept>
#include <utility>

namespace mcs::serialization
{
  template<is_serializable T, typename... BufferArgs>
    auto load_from (BufferArgs&&... buffer_args) -> T
  {
    auto ia {IArchive {std::forward<BufferArgs> (buffer_args)...}};
    auto x {load<T> (ia)};
    if (ia._buffer.size() > 0)
    {
      throw error::AdditionalBytes {ia._buffer.size()};
    }
    return x;
  }

  template<is_serializable T>
    auto load_from (std::filesystem::path path) -> T
  {
    // \todo streaming, avoid to read the whole file
    auto const bytes {util::read_file (path)};
    return serialization::load_from<T> (std::as_bytes (std::span {bytes}));
  }
}
