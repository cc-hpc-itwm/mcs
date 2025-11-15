// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/syscall/getpid.hpp>

namespace mcs::testing
{
  template<typename Prefix>
    UniqID::UniqID (Prefix prefix)
      : _test_info {::testing::UnitTest::GetInstance()->current_test_info()}
      , _name {_test_info->name()}
      , _test_suite_name {_test_info->test_suite_name()}
      , _id { fmt::format ( "{}-{}-{}-{}"
                          , prefix
                          , util::syscall::getpid()
                          , std::hash<std::string>{} (_test_suite_name)
                          , std::hash<std::string>{} (_name)
                          )
            }
  {}
}
