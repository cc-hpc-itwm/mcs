// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/testing/UniqID.hpp>
#include <mcs/util/TemporaryDirectory.hpp>

namespace mcs::testing
{
  // Creates a temporary directory with a name based on the unique
  // test ID. The constructor will create the directory and write test
  // information in a file "INFO" in this directory.
  //
  // EXAMPLE
  //
  //     TEST_F (Suite, name)
  //     {
  //       UniqTemporaryDirectory temp {PREFIX};
  //
  // will produce the file
  //
  //     /$TMPDIR/PREFIX-PID-...-.../INFO
  //
  // with the content
  //
  //    Suite.name
  //
  // and will produce the folder
  //
  //     /$TMPDIR/PREFIX-PID-...-.../DATA
  //
  // which is returned by the method path().
  //
  struct UniqTemporaryDirectory : public UniqID
  {
    template<typename Prefix> UniqTemporaryDirectory (Prefix);

    auto path() const -> std::filesystem::path;

  private:
    util::TemporaryDirectory _temporary_directory
      {std::filesystem::temp_directory_path() / _id};
    util::TemporaryDirectory _temporary_data_path
      {_temporary_directory.path() / "DATA"};
  };
}

#include "detail/UniqTemporaryDirectory.ipp"
