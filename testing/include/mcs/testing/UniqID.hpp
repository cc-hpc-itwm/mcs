// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <gtest/gtest.h>
#include <optional>
#include <string>

namespace mcs::testing
{
  // Provides a unique test id based on a prefix, the pid() and some
  // hashed information from gtest's TestInfo
  //
  struct UniqID
  {
    // EXAMPLE
    //
    //     UniqID {"SOME-PREFIX"}
    //
    template<typename Prefix> UniqID (Prefix);

    auto operator()() const -> std::string;

  protected:
    ::testing::TestInfo const* _test_info;
    std::string const _name;
    std::string const _test_suite_name;
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
  };
}

#include "detail/UniqID.ipp"
