// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/iov_backend/IOV_Backend.hpp>
#include <mcs/util/FMT/write_file.hpp>

namespace mcs::testing::iov_backend
{
  IOV_Backend::ConfigurationFile::ConfigurationFile
    ( mcs::iov_backend::Parameter parameter
    )
      : _path
        { util::FMT::write_file
            ( _temporary_directory.path() / "config"
            , "{}"
            , parameter
            )
        }
  {}
}

namespace mcs::testing::iov_backend
{
  IOV_Backend::IOV_Backend (ConfigurationFile configuration_file)
    : mcs::IOV_Backend {configuration_file._path}
  {}
}
