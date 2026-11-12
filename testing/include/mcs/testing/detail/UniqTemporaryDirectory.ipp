// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/FMT/write_file.hpp>

namespace mcs::testing
{
  template<typename Prefix>
    UniqTemporaryDirectory::UniqTemporaryDirectory (Prefix prefix)
      : UniqID {prefix}
      , _temporary_directory
          { std::filesystem::temp_directory_path()
          / fmt::format ("MCS-TEST-{}", digest() << "MCS-TEST")
          }
  {
    auto test_information {fmt::format ("{}.{}", _test_suite_name, _name)};

    if (_type_param.has_value())
    {
      test_information += fmt::format ("<{}>", *_type_param);
    }

    if (_value_param.has_value())
    {
      test_information += fmt::format ("/{}", *_value_param);
    }

    auto const directory {_temporary_directory.path()};

    util::FMT::write_file (directory / "TEST", "{}\n", test_information);
    util::FMT::write_file (directory / "PID", "{}\n", _process_id);
    util::FMT::write_file (directory / "PREFIX", "{}\n", _prefix);
  }
}
