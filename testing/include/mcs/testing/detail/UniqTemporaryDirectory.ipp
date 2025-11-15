// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fstream>

namespace mcs::testing
{
  template<typename Prefix>
    UniqTemporaryDirectory::UniqTemporaryDirectory (Prefix prefix)
      : UniqID {prefix}
  {
    auto info {std::ofstream {_temporary_directory.path() / "INFO"}};

    info << _test_suite_name << '.' << _name;
    if (_type_param.has_value())
    {
      info << '<' << *_type_param << '>';
    }
    if (_value_param.has_value())
    {
      info << '/' << *_value_param;
    }
    info << '\n';
  }
}
