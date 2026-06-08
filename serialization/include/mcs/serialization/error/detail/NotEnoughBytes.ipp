// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::serialization::error
{
  constexpr auto NotEnoughBytes::wanted() const noexcept -> std::size_t
  {
    return _wanted;
  }
  constexpr auto NotEnoughBytes::provided() const noexcept -> std::size_t
  {
    return _provided;
  }
}
