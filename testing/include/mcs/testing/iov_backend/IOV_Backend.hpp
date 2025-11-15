// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <atomic>
#include <filesystem>
#include <fmt/format.h>
#include <mcs/IOV_Backend.hpp>
#include <mcs/iov_backend/Parameter.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>

namespace mcs::testing::iov_backend
{
  struct IOV_Backend : public mcs::IOV_Backend
  {
    struct ConfigurationFile
    {
      ConfigurationFile (mcs::iov_backend::Parameter);

    private:
      friend struct IOV_Backend;

      static auto instance_counter() -> std::atomic<unsigned int>&
      {
        static auto _instance_counter {std::atomic<unsigned int> {0U}};

        return _instance_counter;
      }

      testing::UniqTemporaryDirectory _temporary_directory
        { fmt::format
          ( "MCS_TEST_IOV_BACKEND_CONFIGURATION_FILE.{}"
          , instance_counter()++
          )
        };
      std::filesystem::path _path;
    };

    IOV_Backend (ConfigurationFile);
  };
}
