// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <compare>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/random/value.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace
{
  struct CustomHash;

  struct UserDefined
  {
    constexpr auto operator<=> (UserDefined const&) const noexcept = default;

    constexpr explicit UserDefined (int i) noexcept : _i {i} {}
    UserDefined() = default;
    UserDefined (UserDefined const&) = delete;
    UserDefined (UserDefined&&) = default;
    auto operator= (UserDefined const&) -> UserDefined& = delete;
    auto operator= (UserDefined&&) -> UserDefined& = default;
    ~UserDefined() = default;

  private:
    friend struct CustomHash;
    int _i;
  };
  static_assert (mcs::serialization::is_serializable<UserDefined>);

  struct CustomCompare
  {
    constexpr auto operator()
      ( UserDefined const& lhs
      , UserDefined const& rhs
      ) const noexcept -> bool
    {
      return _d ? std::less<>{} (lhs, rhs) : std::greater<>{} (lhs, rhs);
    }

    constexpr explicit CustomCompare (bool d) noexcept : _d {d} {}
    CustomCompare() = default;

  private:
    bool _d;
  };
  static_assert (mcs::serialization::is_serializable<CustomCompare>);

  struct CustomHash
  {
    auto operator()
      ( UserDefined const& x
      ) const noexcept -> std::size_t
    {
      return std::hash<int>{} (x._i) ^ _bits;
    }

    constexpr explicit CustomHash (std::size_t bits) noexcept : _bits {bits} {}
    CustomHash() = default;

  private:
    std::size_t _bits;
  };
  static_assert (mcs::serialization::is_serializable<CustomHash>);

  struct CustomEq
  {
    constexpr auto operator()
      ( UserDefined const& lhs
      , UserDefined const& rhs
      ) const noexcept -> bool
    {
      return std::equal_to<>{} (lhs, rhs);
    }
  };
  static_assert (mcs::serialization::is_serializable<CustomEq>);
}

namespace mcs::testing::random
{
  template<> struct value<UserDefined>
  {
    value<int> _value{};

    auto operator()()
    {
      return UserDefined {_value()};
    }
  };
}

namespace std
{
  template<> struct hash<UserDefined>
  {
    auto operator()
      ( UserDefined const& x
      ) const noexcept -> std::size_t
    {
      return CustomHash{} (x);
    }
  };
}
