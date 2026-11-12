// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <fmt/base.h>
#include <functional>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <string_view>

namespace mcs::testing
{
  // Provides a test ID based on a caller prefix and Google Test
  // identity. The digest() is seeded with the prefix, process ID,
  // suite, test name, and optional type and value parameters.
  //
  struct UniqID
  {
    // EXAMPLE
    //
    //     UniqID {"SOME-PREFIX"}
    //
    template<typename Prefix> UniqID (Prefix);

    [[nodiscard]] auto operator()() const -> std::string;

    // Accumulates a deterministic 128-bit digest, formatted as 32
    // lowercase hexadecimal digits. Optional values include a
    // presence marker.
    //
    struct Digest
    {
      Digest() = default;

      template<typename Value>
        auto operator<< (std::optional<Value> const&) -> Digest&;
      auto operator<< (std::string_view) -> Digest&;

    private:
      std::uint64_t _laneA {UINT64_C (0xcbf29ce484222325)};
      std::uint64_t _laneB {UINT64_C (0x84222325cbf29ce4)};

      friend struct fmt::formatter<Digest>;
    };

    [[nodiscard]] auto digest() const noexcept -> Digest;

  protected:
    ::testing::TestInfo const* _test_info;
    std::string const _name;
    std::string const _test_suite_name;
    std::string const _prefix;
    std::string const _process_id;
    std::string const _id;
    std::optional<std::string> const _type_param
      { std::invoke
        ( [&]() -> std::optional<std::string>
          {
            if (_test_info->type_param() != nullptr)
            {
              return std::string {_test_info->type_param()};
            }
            return {};
          }
        )
      };
    std::optional<std::string> const _value_param
      { std::invoke
        ( [&]() -> std::optional<std::string>
          {
            if (_test_info->value_param() != nullptr)
            {
              return std::string {_test_info->value_param()};
            }
            return {};
          }
        )
      };

  private:
    Digest const _digest;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::testing::UniqID::Digest>
    {
      template<typename ParseContext>
        constexpr auto parse (ParseContext&);

      template<typename FormatContext>
        constexpr auto format
          ( mcs::testing::UniqID::Digest const&
          , FormatContext& context
          ) const -> decltype (context.out());
    };
}

#include "detail/UniqID.ipp"
