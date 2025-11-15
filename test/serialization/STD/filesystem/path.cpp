// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <filesystem>
#include <gtest/gtest.h>
#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>

TEST (Serialization, path)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (std::filesystem::temp_directory_path());
}
