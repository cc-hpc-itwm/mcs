// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>

namespace mcs::serialization
{
  IArchive::IArchive (std::vector<std::byte> const& bytes) noexcept
    : IArchive {bytes.data(), bytes.size()}
  {}
  IArchive::IArchive (std::span<std::byte> const& bytes) noexcept
    : IArchive {bytes.data(), bytes.size()}
  {}
}
