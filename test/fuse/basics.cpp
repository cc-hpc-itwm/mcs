// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "MCSFuseT.hpp"
#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <limits>
#include <linux/fs.h>
#include <mcs/has_renameat2.hpp>
#include <mcs/testing/UniqID.hpp>
#include <mcs/testing/random/one_of.hpp>
#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/unique_values.hpp>
#include <mcs/testing/random/value/STD/byte.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/STD/vector.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/fopen.hpp>
#include <mcs/util/syscall/copy_file_range_with_fallback_to_sendfile.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/read.hpp>
#include <mcs/util/syscall/write.hpp>
#include <mcs/util/touch.hpp>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

namespace mcs::fuse
{
  namespace
  {
    auto write
      ( std::filesystem::path file_path
      , std::vector<std::byte> data
      ) -> std::vector<std::byte>
    {
      auto const open_file {util::fopen (file_path, "wb+")};

      if ( auto const bytes_written
             { util::syscall::write
               ( util::syscall::fileno (open_file.get())
               , data.data()
               , data.size()
               )
             }
         ; std::cmp_not_equal (bytes_written, data.size())
         )
      {
        throw std::runtime_error
          { fmt::format
            ( "Could not write all bytes from '{}': read = {}, size = {}"
            , file_path
            , bytes_written
            , data.size()
            )
          };
      }

      return data;
    }

    [[nodiscard]] auto generate_and_write_random_data
      ( std::filesystem::path file_path
      ) -> std::vector<std::byte>
    {
      return write
        ( file_path
        , std::invoke (testing::random::value<std::vector<std::byte>>{})
        );
    }

    auto read (std::filesystem::path file_path) -> std::vector<std::byte>
    {
      auto const open_file {util::fopen (file_path, "rb")};

      auto buffer
        { std::vector<std::byte> (std::filesystem::file_size (file_path))
        };

      if ( auto const bytes_read
             { util::syscall::read
               ( util::syscall::fileno (open_file.get())
               , buffer.data()
               , buffer.size()
               )
             }
         ; std::cmp_not_equal (bytes_read, buffer.size())
         )
      {
        throw std::runtime_error
          { fmt::format
            ( "Could not read all bytes from '{}': read = {}, size = {}"
            , file_path
            , bytes_read
            , buffer.size()
            )
          };
      }

      return buffer;
    }
  }

  TYPED_TEST (MCSFuseT, fresh_mountpoint_is_empty)
  {
    ASSERT_EQ ( std::filesystem::directory_iterator {this->_path}
              , std::filesystem::directory_iterator{}
              );
  }

  TYPED_TEST (MCSFuseT, file_created_can_be_listed)
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {""})};

    util::touch (file_path);

    auto di {std::filesystem::directory_iterator {this->_path}};

    ASSERT_NE (di, std::filesystem::directory_iterator{});

    ASSERT_EQ (di->path(), file_path);

    ++di;

    ASSERT_EQ (di, std::filesystem::directory_iterator{});
  }

  TYPED_TEST (MCSFuseT, file_created_can_be_removed)
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {""})};

    util::touch (file_path);
    std::filesystem::remove (file_path);

    ASSERT_EQ ( std::filesystem::directory_iterator {this->_path}
              , std::filesystem::directory_iterator{}
              );
  }

  TYPED_TEST (MCSFuseT, new_file_has_size_zero)
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {""})};

    util::touch (file_path);

    ASSERT_EQ (std::filesystem::file_size (file_path), 0);
  }

  TYPED_TEST (MCSFuseT, data_can_be_written_into_a_file_and_file_size_changes)
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {""})};

    auto const random_data {generate_and_write_random_data (file_path)};

    ASSERT_EQ (std::filesystem::file_size (file_path), random_data.size());
  }

  TYPED_TEST (MCSFuseT, data_can_be_written_into_a_file_and_read_back)
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {""})};

    auto const random_data {generate_and_write_random_data (file_path)};

    ASSERT_EQ (random_data, read (file_path));
  }

  TYPED_TEST (MCSFuseT, directory_created_can_be_listed)
  {
    auto const directory_path
      { this->_path / std::invoke (testing::UniqID {""})
      };

    ASSERT_TRUE (std::filesystem::create_directory (directory_path));

    auto di {std::filesystem::directory_iterator {this->_path}};

    ASSERT_NE (di, std::filesystem::directory_iterator{});

    ASSERT_EQ (di->path(), directory_path);
    ASSERT_TRUE (di->is_directory());

    ++di;

    ASSERT_EQ (di, std::filesystem::directory_iterator{});
  }

  TYPED_TEST (MCSFuseT, directory_created_can_be_removed)
  {
    auto const directory_path
      { this->_path / std::invoke (testing::UniqID {""})
      };

    ASSERT_TRUE (std::filesystem::create_directory (directory_path));
    ASSERT_TRUE (std::filesystem::remove (directory_path));

    ASSERT_EQ ( std::filesystem::directory_iterator {this->_path}
              , std::filesystem::directory_iterator{}
              );
  }

  TYPED_TEST
    ( MCSFuseT
    , data_can_be_written_into_a_file_and_read_back_via_a_hard_link
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const link_path {this->_path / std::invoke (testing::UniqID {"link"})};

    util::touch (file_path);

    std::filesystem::create_hard_link (file_path, link_path);

    auto const random_data {generate_and_write_random_data (file_path)};

    ASSERT_TRUE (std::filesystem::remove (file_path));

    ASSERT_EQ (random_data, read (link_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , data_can_be_written_into_a_file_and_read_back_via_a_symbolic_link
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const link_path {this->_path / std::invoke (testing::UniqID {"link"})};

    util::touch (file_path);

    std::filesystem::create_symlink (file_path, link_path);

    auto const random_data {generate_and_write_random_data (file_path)};

    ASSERT_EQ (random_data, read (link_path));
  }

  // \todo test get/set attr
  // \todo test flock

  #define MCS_TEST_FUSE_BASICS_ASSERT_MINUS_ONE_AND_ERRNO(_errno, _fun, _args...) \
  do                                                                              \
  {                                                                               \
    auto const return_value {std::invoke (_fun, _args)};                          \
    auto const ec {errno};                                                        \
    ASSERT_EQ (return_value, -1);                                                 \
    ASSERT_EQ (ec, _errno);                                                       \
  } while (0)

  TYPED_TEST
    ( MCSFuseT
    , mknod_and_unlink_works
    )
  {
    auto const reg_path {this->_path / std::invoke (testing::UniqID {"reg"})};
    auto const chr_path {this->_path / std::invoke (testing::UniqID {"chr"})};
    auto const blk_path {this->_path / std::invoke (testing::UniqID {"blk"})};
    auto const fifo_path {this->_path / std::invoke (testing::UniqID {"fifo"})};
    auto const sock_path {this->_path / std::invoke (testing::UniqID {"sock"})};

    ASSERT_EQ (::mknod (reg_path.c_str(), S_IFREG, 0), 0);

    MCS_TEST_FUSE_BASICS_ASSERT_MINUS_ONE_AND_ERRNO
      ( EOPNOTSUPP
      , ::mknod
      , chr_path.c_str()
      , S_IFCHR
      , 0
      );

    MCS_TEST_FUSE_BASICS_ASSERT_MINUS_ONE_AND_ERRNO
      ( EPERM
      , ::mknod
      , blk_path.c_str()
      , S_IFBLK
      , 0
      );

    MCS_TEST_FUSE_BASICS_ASSERT_MINUS_ONE_AND_ERRNO
      ( EOPNOTSUPP
      , ::mknod
      , fifo_path.c_str()
      , S_IFIFO
      , 0
      );

    MCS_TEST_FUSE_BASICS_ASSERT_MINUS_ONE_AND_ERRNO
      ( EOPNOTSUPP
      , ::mknod
      , sock_path.c_str()
      , S_IFSOCK
      , 0
      );

    ASSERT_TRUE (std::filesystem::exists (reg_path));
    ASSERT_TRUE (std::filesystem::is_regular_file (reg_path));

    ASSERT_FALSE (std::filesystem::exists (chr_path));
    ASSERT_FALSE (std::filesystem::exists (blk_path));
    ASSERT_FALSE (std::filesystem::exists (fifo_path));
    ASSERT_FALSE (std::filesystem::exists (sock_path));

    ASSERT_EQ (::unlink (reg_path.c_str()), 0);

    ASSERT_FALSE (std::filesystem::exists (reg_path));
  }

  #undef MCS_TEST_FUSE_BASICS_ASSERT_MINUS_ONE_AND_ERRNO

  TYPED_TEST
    ( MCSFuseT
    , file_can_be_renamed
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (old_path)};

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_EQ (old_data, read (new_path));
  }

  #define MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO(_old_path, _new_path, _errno) \
  do                                                                             \
  {                                                                              \
    auto ec {std::error_code{}};                                                 \
    std::filesystem::rename (_old_path, _new_path, ec);                          \
    ASSERT_EQ (ec.value(), _errno);                                              \
  } while (0)

  TYPED_TEST
    ( MCSFuseT
    , non_existing_file_can_not_be_renamed
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, ENOENT);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_FALSE (std::filesystem::exists (new_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , file_can_overwrite_another_file_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (old_path)};
    std::ignore = generate_and_write_random_data (new_path);

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_EQ (old_data, read (new_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , file_can_not_corrupt_itself_with_rename
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const link_path {this->_path / std::invoke (testing::UniqID {"link"})};

    auto const random_data {generate_and_write_random_data (file_path)};
    std::filesystem::create_hard_link (file_path, link_path);

    std::filesystem::rename (file_path, file_path);
    std::filesystem::rename (file_path, link_path);
    std::filesystem::rename (link_path, file_path);
    std::filesystem::rename (link_path, link_path);

    ASSERT_TRUE (std::filesystem::exists (file_path));
    ASSERT_TRUE (std::filesystem::exists (link_path));
    ASSERT_TRUE (std::filesystem::is_regular_file (file_path));
    ASSERT_TRUE (std::filesystem::is_regular_file (link_path));
    ASSERT_EQ (random_data, read (file_path));
    ASSERT_EQ (random_data, read (link_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , file_can_not_overwrite_an_empty_dir_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (old_path)};
    ASSERT_TRUE (std::filesystem::create_directory (new_path));

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, EISDIR);

    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::is_directory (new_path));
    ASSERT_TRUE (std::filesystem::is_empty (new_path));
    ASSERT_EQ (old_data, read (old_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , file_can_not_overwrite_a_non_empty_dir_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const new_file_path
      { new_path / std::invoke (testing::UniqID {"file"})
      };

    auto const old_data {generate_and_write_random_data (old_path)};
    ASSERT_TRUE (std::filesystem::create_directory (new_path));
    auto const new_data {generate_and_write_random_data (new_file_path)};

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, EISDIR);

    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (old_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , file_can_overwrite_a_hard_link_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const new_file_path
      { this->_path / std::invoke (testing::UniqID {"file"})
      };

    auto const old_data {generate_and_write_random_data (old_path)};
    auto const new_data {generate_and_write_random_data (new_file_path)};
    std::filesystem::create_hard_link (new_file_path, new_path);

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (new_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , file_can_overwrite_a_symlink_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const new_file_path
      { this->_path / std::invoke (testing::UniqID {"file"})
      };

    auto const old_data {generate_and_write_random_data (old_path)};
    auto const new_data {generate_and_write_random_data (new_file_path)};
    std::filesystem::create_symlink (new_file_path, new_path);

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (new_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , dir_can_be_renamed
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const file_name {std::invoke (testing::UniqID {"file"})};
    auto const old_file_path {old_path / file_name};
    auto const new_file_path {new_path / file_name};

    std::filesystem::create_directory (old_path);
    auto const old_data {generate_and_write_random_data (old_file_path)};

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_FALSE (std::filesystem::exists (old_file_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , dir_can_not_overwrite_another_file_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const old_file_path
      { old_path / std::invoke (testing::UniqID {"file"})
      };
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    std::filesystem::create_directory (old_path);
    auto const old_data {generate_and_write_random_data (old_file_path)};
    auto const new_data {generate_and_write_random_data (new_path)};

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, ENOTDIR);

    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (old_file_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_EQ (old_data, read (old_file_path));
    ASSERT_EQ (new_data, read (new_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , dir_can_not_corrupt_itself_with_rename
    )
  {
    auto const dir_path {this->_path / std::invoke (testing::UniqID {"dir"})};
    auto const file_path {dir_path / std::invoke (testing::UniqID {"file"})};

    std::filesystem::create_directory (dir_path);
    auto const random_data {generate_and_write_random_data (file_path)};

    std::filesystem::rename (dir_path, dir_path);

    ASSERT_TRUE (std::filesystem::exists (dir_path));
    ASSERT_TRUE (std::filesystem::exists (file_path));
    ASSERT_EQ (random_data, read (file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , dir_can_overwrite_an_empty_dir_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const file_name {std::invoke (testing::UniqID {"file"})};
    auto const old_file_path {old_path / file_name};
    auto const new_file_path {new_path / file_name};

    std::filesystem::create_directory (old_path);
    auto const old_data {generate_and_write_random_data (old_file_path)};
    std::filesystem::create_directory (new_path);

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_FALSE (std::filesystem::exists (old_file_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , dir_can_not_overwrite_a_non_empty_dir_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const old_file_path
      { old_path / std::invoke (testing::UniqID {"oldfile"})
      };
    auto const new_file_path
      { new_path / std::invoke (testing::UniqID {"newfile"})
      };

    ASSERT_TRUE (std::filesystem::create_directory (old_path));
    auto const old_data {generate_and_write_random_data (old_file_path)};
    ASSERT_TRUE (std::filesystem::create_directory (new_path));
    auto const new_data {generate_and_write_random_data (new_file_path)};

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, ENOTEMPTY);

    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (old_file_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (old_file_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , dir_can_not_overwrite_a_hard_link_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const old_file_path
      { old_path / std::invoke (testing::UniqID {"file"})
      };
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const new_file_path
      { this->_path / std::invoke (testing::UniqID {"file"})
      };

    ASSERT_TRUE (std::filesystem::create_directory (old_path));
    auto const old_data {generate_and_write_random_data (old_file_path)};
    auto const new_data {generate_and_write_random_data (new_file_path)};
    std::filesystem::create_hard_link (new_file_path, new_path);

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, ENOTDIR);

    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (old_file_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (old_file_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , dir_can_not_overwrite_a_symlink_with_rename
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const old_file_path
      { old_path / std::invoke (testing::UniqID {"file"})
      };
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const new_file_path
      { this->_path / std::invoke (testing::UniqID {"file"})
      };

    ASSERT_TRUE (std::filesystem::create_directory (old_path));
    auto const old_data {generate_and_write_random_data (old_file_path)};
    auto const new_data {generate_and_write_random_data (new_file_path)};
    std::filesystem::create_symlink (new_file_path, new_path);

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, ENOTDIR);

    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (old_file_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (old_file_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , symlink_can_be_renamed
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (file_path)};
    std::filesystem::create_symlink (file_path, old_path);

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (file_path));
    ASSERT_EQ (old_data, read (new_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , symlink_can_overwrite_another_file_with_rename
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (file_path)};
    std::filesystem::create_symlink (file_path, old_path);
    std::ignore = generate_and_write_random_data (new_path);

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (file_path));
    ASSERT_EQ (old_data, read (new_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , symlink_can_not_corrupt_itself_with_rename
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const link_path {this->_path / std::invoke (testing::UniqID {"link"})};

    auto const random_data {generate_and_write_random_data (file_path)};
    std::filesystem::create_symlink (file_path, link_path);

    std::filesystem::rename (link_path, link_path);

    ASSERT_TRUE (std::filesystem::exists (file_path));
    ASSERT_TRUE (std::filesystem::exists (link_path));
    ASSERT_EQ (random_data, read (file_path));
    ASSERT_EQ (random_data, read (link_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , symlink_can_not_overwrite_an_empty_dir_with_rename
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (file_path)};
    std::filesystem::create_symlink (file_path, old_path);
    ASSERT_TRUE (std::filesystem::create_directory (new_path));

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, EISDIR);

    ASSERT_TRUE (std::filesystem::exists (file_path));
    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::is_directory (new_path));
    ASSERT_TRUE (std::filesystem::is_empty (new_path));
    ASSERT_EQ (old_data, read (old_path));
    ASSERT_EQ (old_data, read (file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , symlink_can_not_overwrite_a_non_empty_dir_with_rename
    )
  {
    auto const old_file_path
      { this->_path / std::invoke (testing::UniqID {"file"})
      };
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};
    auto const new_file_path
      { new_path / std::invoke (testing::UniqID {"file"})
      };

    auto const old_data {generate_and_write_random_data (old_file_path)};
    std::filesystem::create_symlink (old_file_path, old_path);
    ASSERT_TRUE (std::filesystem::create_directory (new_path));
    auto const new_data {generate_and_write_random_data (new_file_path)};

    MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO (old_path, new_path, EISDIR);

    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_EQ (old_data, read (old_path));
    ASSERT_EQ (old_data, read (old_file_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

  #undef MCS_TEST_FUSE_BASICS_ASSERT_RENAME_ERRNO

  TYPED_TEST
    ( MCSFuseT
    , symlink_can_overwrite_a_hard_link_with_rename
    )
  {
    auto const old_file_path
      { this->_path / std::invoke (testing::UniqID {"oldfile"})
      };
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_file_path
      { this->_path / std::invoke (testing::UniqID {"newfile"})
      };
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (old_file_path)};
    std::filesystem::create_symlink (old_file_path, old_path);
    auto const new_data {generate_and_write_random_data (new_file_path)};
    std::filesystem::create_hard_link (new_file_path, new_path);

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (old_file_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (new_path));
    ASSERT_EQ (old_data, read (old_file_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

  TYPED_TEST
    ( MCSFuseT
    , symlink_can_overwrite_a_symlink_with_rename
    )
  {
    auto const old_file_path
      { this->_path / std::invoke (testing::UniqID {"oldfile"})
      };
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_file_path
      { this->_path / std::invoke (testing::UniqID {"newfile"})
      };
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (old_file_path)};
    std::filesystem::create_symlink (old_file_path, old_path);
    auto const new_data {generate_and_write_random_data (new_file_path)};
    std::filesystem::create_symlink (new_file_path, new_path);

    std::filesystem::rename (old_path, new_path);

    ASSERT_FALSE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_TRUE (std::filesystem::exists (old_file_path));
    ASSERT_TRUE (std::filesystem::exists (new_file_path));
    ASSERT_EQ (old_data, read (new_path));
    ASSERT_EQ (old_data, read (old_file_path));
    ASSERT_EQ (new_data, read (new_file_path));
  }

#if defined (MCS_CONFIG_HAS_RENAMEAT2)
  TYPED_TEST
    ( MCSFuseT
    , rename_exchange_swaps_two_files
    )
  {
    auto const old_path {this->_path / std::invoke (testing::UniqID {"old"})};
    auto const new_path {this->_path / std::invoke (testing::UniqID {"new"})};

    auto const old_data {generate_and_write_random_data (old_path)};
    auto const new_data {generate_and_write_random_data (new_path)};

    // Use renameat2 with RENAME_EXCHANGE to atomically swap the two files
    auto const result
      { ::renameat2
        ( AT_FDCWD
        , old_path.c_str()
        , AT_FDCWD
        , new_path.c_str()
        , RENAME_EXCHANGE
        )
      };
    ASSERT_EQ (result, 0) << "renameat2 failed: " << strerror (errno);

    // After exchange: old_path should have new_data, new_path should have old_data
    ASSERT_TRUE (std::filesystem::exists (old_path));
    ASSERT_TRUE (std::filesystem::exists (new_path));
    ASSERT_EQ (new_data, read (old_path));
    ASSERT_EQ (old_data, read (new_path));
  }
#endif

#if defined (MCS_CONFIG_HAS_RENAMEAT2)
  TYPED_TEST
    ( MCSFuseT
    , rename_exchange_swaps_file_and_symlink
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const target_path {this->_path / std::invoke (testing::UniqID {"target"})};
    auto const link_path {this->_path / std::invoke (testing::UniqID {"link"})};

    auto const file_data {generate_and_write_random_data (file_path)};
    auto const target_data {generate_and_write_random_data (target_path)};
    std::filesystem::create_symlink (target_path, link_path);

    auto const result
      { ::renameat2
        ( AT_FDCWD
        , file_path.c_str()
        , AT_FDCWD
        , link_path.c_str()
        , RENAME_EXCHANGE
        )
      };
    ASSERT_EQ (result, 0) << "renameat2 failed: " << strerror (errno);

    ASSERT_TRUE (std::filesystem::is_symlink (file_path));
    ASSERT_TRUE (std::filesystem::is_regular_file (link_path));
    ASSERT_EQ (target_data, read (file_path));
    ASSERT_EQ (file_data, read (link_path));
  }
#endif

#if defined (MCS_CONFIG_HAS_RENAMEAT2)
  TYPED_TEST
    ( MCSFuseT
    , rename_exchange_swaps_symlink_and_file
    )
  {
    auto const target_path {this->_path / std::invoke (testing::UniqID {"target"})};
    auto const link_path {this->_path / std::invoke (testing::UniqID {"link"})};
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};

    auto const target_data {generate_and_write_random_data (target_path)};
    std::filesystem::create_symlink (target_path, link_path);
    auto const file_data {generate_and_write_random_data (file_path)};

    auto const result
      { ::renameat2
        ( AT_FDCWD
        , link_path.c_str()
        , AT_FDCWD
        , file_path.c_str()
        , RENAME_EXCHANGE
        )
      };
    ASSERT_EQ (result, 0) << "renameat2 failed: " << strerror (errno);

    ASSERT_TRUE (std::filesystem::is_regular_file (link_path));
    ASSERT_TRUE (std::filesystem::is_symlink (file_path));
    ASSERT_EQ (file_data, read (link_path));
    ASSERT_EQ (target_data, read (file_path));
  }
#endif

#if defined (MCS_CONFIG_HAS_RENAMEAT2)
  TYPED_TEST
    ( MCSFuseT
    , rename_exchange_swaps_two_symlinks
    )
  {
    auto const target1_path {this->_path / std::invoke (testing::UniqID {"target1"})};
    auto const link1_path {this->_path / std::invoke (testing::UniqID {"link1"})};
    auto const target2_path {this->_path / std::invoke (testing::UniqID {"target2"})};
    auto const link2_path {this->_path / std::invoke (testing::UniqID {"link2"})};

    auto const target1_data {generate_and_write_random_data (target1_path)};
    std::filesystem::create_symlink (target1_path, link1_path);
    auto const target2_data {generate_and_write_random_data (target2_path)};
    std::filesystem::create_symlink (target2_path, link2_path);

    auto const result
      { ::renameat2
        ( AT_FDCWD
        , link1_path.c_str()
        , AT_FDCWD
        , link2_path.c_str()
        , RENAME_EXCHANGE
        )
      };
    ASSERT_EQ (result, 0) << "renameat2 failed: " << strerror (errno);

    ASSERT_TRUE (std::filesystem::is_symlink (link1_path));
    ASSERT_TRUE (std::filesystem::is_symlink (link2_path));
    ASSERT_EQ (target2_data, read (link1_path));
    ASSERT_EQ (target1_data, read (link2_path));
  }
#endif

  TYPED_TEST
    ( MCSFuseT
    , xattr_works
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {""})};

    util::touch (file_path);

    struct Attribute
    {
      std::string name;
      std::string value;
      auto operator<=> (Attribute const& other) const = default;
    };

    auto const get_attribute_value
      { [&](auto name)
        {
          auto value
            { std::vector<char>
              ( util::cast<std::vector<char>::size_type>
                ( ::getxattr
                  ( file_path.c_str()
                  , name.c_str()
                  , nullptr
                  , 0
                  )
                )
              )
            };

          if ( auto const size
                 { util::cast<std::vector<char>::size_type>
                   ( ::getxattr
                     ( file_path.c_str()
                     , name.c_str()
                     , value.data()
                     , value.size()
                     )
                   )
                 }
             ; size != value.size()
             )
          {
            throw std::runtime_error
              { fmt::format
                ( "Expected size '{}' from getxattr but received '{}'"
                , value.size()
                , size
                )
              };
          }

          return std::string {value.begin(), value.end()};
        }
      };

    auto const get_attributes
      { [&]
        {
          auto names
            { std::vector<char>
              ( util::cast<std::vector<char>::size_type>
                ( ::listxattr
                  ( file_path.c_str()
                  , nullptr
                  , 0
                  )
                )
              )
            };

          if ( auto const size
                 { util::cast<std::vector<char>::size_type>
                   ( ::listxattr
                     ( file_path.c_str()
                     , names.data()
                     , names.size()
                     )
                   )
                 }
             ; size != names.size()
             )
          {
            throw std::runtime_error
              { fmt::format
                ( "Expected size '{}' from listxattr but received '{}'"
                , names.size()
                , size
                )
              };
          }

          auto attributes {std::vector<Attribute>{}};
          {
            auto c {std::begin (names)};

            while (c != std::end (names))
            {
              auto const zero {std::find (c, std::end (names), '\0')};

              if (zero == std::end (names))
              {
                throw std::runtime_error
                  { fmt::format
                    ( "Expected null byte at the end of extended attributes"
                    )
                  };
              }

              auto const name {std::string {c, zero}};

              if (name.empty())
              {
                throw std::runtime_error
                  { fmt::format
                    ( "Unexpected null byte in extended attributes"
                    )
                  };
              }

              attributes.emplace_back (name, get_attribute_value (name));

              c = zero + 1;
            }
          }

          return attributes;
        }
      };

#if defined (MCS_CONFIG_GCC_WORKAROUND_BROKEN_RANGES_SORT)
    #define MCS_TEST_FUSE_BASICS_ASSERT_ALL_ATTRIBUTES_ARE_STORED(_attributes) \
    do                                                                         \
    {                                                                          \
      auto expected_attributes {_attributes};                                  \
      auto got_attributes {get_attributes()};                                  \
      ASSERT_EQ (expected_attributes.size(), got_attributes.size());           \
      std::sort (expected_attributes.begin(), expected_attributes.end());      \
      std::sort (got_attributes.begin(), got_attributes.end());                \
      ASSERT_EQ (expected_attributes, got_attributes);                         \
    } while (0)
#else
    #define MCS_TEST_FUSE_BASICS_ASSERT_ALL_ATTRIBUTES_ARE_STORED(_attributes) \
    do                                                                         \
    {                                                                          \
      auto expected_attributes {_attributes};                                  \
      auto got_attributes {get_attributes()};                                  \
      ASSERT_EQ (expected_attributes.size(), got_attributes.size());           \
      std::ranges::sort (expected_attributes);                                 \
      std::ranges::sort (got_attributes);                                      \
      ASSERT_EQ (expected_attributes, got_attributes);                         \
    } while (0)
#endif

    auto const random_name
      { []
        {
          return std::string
            { std::string {"user."}
            + std::invoke
              ( testing::random::value<std::string>
                { testing::random::value<std::string>::Length {1, 10}
                , 'a'
                , 'z'
                }
              )
            };
        }
      };

    auto const random_value
      { []
        {
          return std::string
            { std::invoke
              ( testing::random::value<std::string>
                { testing::random::value<std::string>::Length {1, 10}
                , 'a'
                , 'z'
                }
              )
            };
        }
      };

    auto const N {std::invoke (testing::random::value<std::size_t> {0, 100})};

    auto random_names
      { testing::random::unique_values<std::string> (random_name, N)
      };

    auto random_values
      { testing::random::unique_values<std::string> (random_value, N)
      };

    // Retrieve all attributes that are already there, e.g. set by the
    // file system for all files.
    //
    auto attributes {get_attributes()};

    for (auto i {std::size_t {0}}; i != N; ++i)
    {
      auto const name {random_names.back()};
      random_names.pop_back();
      auto const value {random_values.back()};
      random_values.pop_back();
      ASSERT_EQ
        ( ::setxattr
          ( file_path.c_str()
          , name.c_str()
          , value.data()
          , value.size()
          , 0
          )
        , 0
        );
      attributes.emplace_back (name, value);
      MCS_TEST_FUSE_BASICS_ASSERT_ALL_ATTRIBUTES_ARE_STORED (attributes);
    }

    std::ranges::shuffle (attributes, testing::random::random_device());

    while (!attributes.empty())
    {
      ASSERT_EQ
        ( ::removexattr
          ( file_path.c_str()
          , attributes.back().name.c_str()
          )
        , 0
        );
      attributes.pop_back();
      MCS_TEST_FUSE_BASICS_ASSERT_ALL_ATTRIBUTES_ARE_STORED (attributes);
    }
  }

  #undef MCS_TEST_FUSE_BASICS_ASSERT_ALL_ATTRIBUTES_ARE_STORED

  TYPED_TEST
    ( MCSFuseT
    , xattr_can_have_special_characters
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {""})};

    util::touch (file_path);

    #define MCS_TEST_FUSE_BASICS_ASSERT_NAME_AND_VALUE_ARE_STORED(_name, _value) \
    do                                                                         \
    {                                                                          \
      ASSERT_EQ                                                                \
        ( ::setxattr                                                           \
          ( file_path.c_str()                                                  \
          , _name.c_str()                                                      \
          , _value.data()                                                      \
          , _value.size()                                                      \
          , 0                                                                  \
          )                                                                    \
        , 0                                                                    \
        );                                                                     \
                                                                               \
      auto _bytes                                                              \
        { std::vector<char>                                                    \
          ( util::cast<std::vector<char>::size_type>                           \
            ( ::getxattr                                                       \
              ( file_path.c_str()                                              \
              , _name.c_str()                                                  \
              , nullptr                                                        \
              , 0                                                              \
              )                                                                \
            )                                                                  \
          )                                                                    \
        };                                                                     \
                                                                               \
      ASSERT_EQ                                                                \
        ( ::getxattr                                                           \
          ( file_path.c_str()                                                  \
          , _name.c_str()                                                      \
          , _bytes.data()                                                      \
          , _bytes.size()                                                      \
          )                                                                    \
        , _bytes.size()                                                        \
        );                                                                     \
                                                                               \
      ASSERT_EQ (_value, std::string (_bytes.begin(), _bytes.end()));          \
                                                                               \
      ASSERT_EQ                                                                \
        ( ::removexattr                                                        \
          ( file_path.c_str()                                                  \
          , _name.c_str()                                                      \
          )                                                                    \
        , 0                                                                    \
        );                                                                     \
    } while (0)

    for ( auto c {std::numeric_limits<char>::min()}
        ; c < std::numeric_limits<char>::max()
        ; ++c
        )
    {
      if (c == '\0')
      {
        continue;
      }

      auto const name {std::string {"user."} + std::string (1, c)};
      auto const value {name};
      MCS_TEST_FUSE_BASICS_ASSERT_NAME_AND_VALUE_ARE_STORED (name, value);
    }

    auto const name {std::string {"user.name"}};
    auto const value
      { std::string
        { '\0'
        , 'n'
        , 'u'
        , 'l'
        , 'l'
        , '\0'
        , 'b'
        , 'y'
        , 't'
        , 'e'
        , 's'
        , '\0'
        }
      };
    MCS_TEST_FUSE_BASICS_ASSERT_NAME_AND_VALUE_ARE_STORED (name, value);

    #undef MCS_TEST_FUSE_BASICS_ASSERT_NAME_AND_VALUE_ARE_STORED
  }

  TYPED_TEST
    ( MCSFuseT
    , copy_file_range_works
    )
  {
    auto const in_path {this->_path / std::invoke (testing::UniqID {"in"})};
    auto const out_path {this->_path / std::invoke (testing::UniqID {"out"})};

    auto const in_data {generate_and_write_random_data (in_path)};
    auto const out_data {generate_and_write_random_data (out_path)};

    auto const in_file {util::fopen (in_path, "rb")};
    auto const out_file {util::fopen (out_path, "wb")};

    auto const range_length
      { std::min (in_data.size(), out_data.size())
      };

    ASSERT_EQ
      ( util::syscall::copy_file_range_with_fallback_to_sendfile
        ( util::syscall::fileno (in_file.get())
        , util::syscall::fileno (out_file.get())
        , range_length
        )
      , range_length
      );

    auto const written_data {read (out_path)};

    for (auto i {std::size_t {0}}; i != range_length; i++)
    {
      ASSERT_EQ (in_data[i], written_data[i]);
    }
  }

  TYPED_TEST
    ( MCSFuseT
    , lseek_works
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};
    auto const file_data {generate_and_write_random_data (file_path)};
    auto const open_file {util::fopen (file_path, "rb")};

    auto const read_rest
      { [&]
        {
          auto content {std::vector<std::byte>{}};
          auto buffer {std::vector<std::byte> (4096)};

          auto const do_read
            { [&]
              {
                return util::syscall::read
                  ( util::syscall::fileno (open_file.get())
                  , buffer.data()
                  , buffer.size()
                  );
              }
            };

          for ( auto bytes_read {std::invoke (do_read)}
              ; bytes_read > ssize_t {0}
              ; bytes_read = std::invoke (do_read)
              )
          {
            std::copy
              ( buffer.begin()
              , buffer.begin() + bytes_read
              , std::back_inserter (content)
              );
          }

          return content;
        }
      };

    ASSERT_EQ
      ( ::lseek (util::syscall::fileno (open_file.get()), off_t {0}, SEEK_SET)
      , off_t {0}
      );
    ASSERT_EQ (file_data, read_rest());

    ASSERT_EQ
      ( ::lseek
        ( util::syscall::fileno (open_file.get())
        , util::cast<off_t> (file_data.size())
        , SEEK_SET
        )
      , util::cast<off_t> (file_data.size())
      );
    ASSERT_EQ (std::vector<std::byte>{}, read_rest());

    auto const random_offset
      { std::invoke
        ( testing::random::value<off_t>
          { off_t {0}
          , util::cast<off_t> (file_data.size())
          }
        )
      };

    ASSERT_EQ
      ( ::lseek
        ( util::syscall::fileno (open_file.get())
        , random_offset
        , SEEK_SET
        )
      , random_offset
      );
    ASSERT_EQ
      ( std::vector<std::byte>
        ( file_data.begin() + random_offset
        , file_data.end()
        )
      , read_rest()
      );

    auto const random_move
      { std::invoke
        ( testing::random::value<off_t>
          { off_t {0}
          , util::cast<off_t> (file_data.size()) - random_offset
          }
        )
      };

    ASSERT_EQ
      ( ::lseek
        ( util::syscall::fileno (open_file.get())
        , random_offset
        , SEEK_SET
        )
      , random_offset
      );
    ASSERT_EQ
      ( ::lseek
        ( util::syscall::fileno (open_file.get())
        , random_move
        , SEEK_CUR
        )
      , random_offset + random_move
      );
    ASSERT_EQ
      ( std::vector<std::byte>
        ( file_data.begin() + random_offset + random_move
        , file_data.end()
        )
      , read_rest()
      );

    ASSERT_EQ
      ( ::lseek (util::syscall::fileno (open_file.get()), off_t {0}, SEEK_END)
      , util::cast<off_t> (file_data.size())
      );
    ASSERT_EQ (std::vector<std::byte>{}, read_rest());
  }

  TYPED_TEST
    ( MCSFuseT
    , files_in_directory_can_be_removed_during_iteration
    )
  {
    auto const directory
      { this->_path / std::invoke (testing::UniqID {"directory"})
      };

    ASSERT_TRUE (std::filesystem::create_directory (directory));

    auto files {std::unordered_set<std::filesystem::path>{}};

    std::generate_n
      ( std::inserter (files, std::end (files))
      , std::invoke (testing::random::value<int> {0, 100})
      , [&directory, n {0}]() mutable
        {
          return directory
            / std::invoke (testing::UniqID {std::to_string (n++)})
            ;
        }
      );

    std::ranges::for_each (files, &util::touch);

    auto seen {std::unordered_set<std::filesystem::path>{}};

    // Create a directory iterator...
    auto di {std::filesystem::directory_iterator {directory}};

    // ...remove about one half of the files...
    std::ranges::for_each
      ( files
      , [] (auto file)
        {
          if (testing::random::one_of ("0123456789") < '5')
          {
            std::filesystem::remove (file);
          }
        }
      );

    // ...and only now iterate...
    while (di != std::filesystem::directory_iterator{})
    {
      ASSERT_TRUE (seen.emplace (di->path()).second);

      ++di;
    }

    // ...to still see all the files.
    ASSERT_EQ (seen, files);
  }

  TYPED_TEST
    ( MCSFuseT
    , open_file_can_be_read_after_remove
    )
  {
    auto const file_path {this->_path / std::invoke (testing::UniqID {"file"})};

    auto const data_written {generate_and_write_random_data (file_path)};
    auto const file_size {std::filesystem::file_size (file_path)};

    auto const open_file {util::fopen (file_path, "rb")};

    ASSERT_TRUE (std::filesystem::remove (file_path));

    auto data_read
      { std::vector<std::byte> (file_size)
      };
    if ( auto const bytes_read
           { util::syscall::read
             ( util::syscall::fileno (open_file.get())
             , data_read.data()
             , data_read.size()
             )
           }
       ; std::cmp_not_equal (bytes_read, data_read.size())
       )
    {
      throw std::runtime_error
        { fmt::format
          ( "Could not read all bytes from '{}': read = {}, size = {}"
          , file_path
          , bytes_read
          , data_read.size()
          )
        };
    }

    ASSERT_EQ (data_written, data_read);
  }
}
