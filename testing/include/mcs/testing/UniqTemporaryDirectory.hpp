// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/testing/UniqID.hpp>
#include <mcs/util/TemporaryDirectory.hpp>

namespace mcs::testing
{
  // Creates a temporary directory with a name based on the unique
  // test ID. The constructor writes test information to separate
  // files.
  //
  // EXAMPLE
  //
  //     TEST_F (Suite, name)
  //     {
  //       UniqTemporaryDirectory temp {PREFIX};
  //
  // will produce the files
  //
  //     /$TMPDIR/MCS-TEST-<128-bit-digest>/TEST
  //     /$TMPDIR/MCS-TEST-<128-bit-digest>/PID
  //     /$TMPDIR/MCS-TEST-<128-bit-digest>/PREFIX
  //
  // TEST contains the test suite and name, including optional type
  // and value parameters. PID contains the process ID, and PREFIX
  // contains the caller-provided prefix.
  //
  // It also produces the folder
  //
  //     /$TMPDIR/MCS-TEST-<128-bit-digest>/DATA
  //
  // which is returned by the method path().
  //
  struct UniqTemporaryDirectory : public UniqID
  {
    template<typename Prefix> UniqTemporaryDirectory (Prefix);

    [[nodiscard]] auto path() const -> std::filesystem::path;

  private:
    util::TemporaryDirectory _temporary_directory;
    util::TemporaryDirectory _temporary_data_path
      { _temporary_directory.path() / "DATA"
      };
  };
}

#include "detail/UniqTemporaryDirectory.ipp"
