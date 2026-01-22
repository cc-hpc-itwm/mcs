// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "MCSFuseT.hpp"
#include <algorithm>
#include <filesystem>
#include <fmt/base.h>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/util/Buffer.hpp>
#include <mcs/util/fopen.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/read.hpp>
#include <mcs/util/syscall/write.hpp>
#include <mcs/util/timed.hpp>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mcs::fuse
{
  TYPED_TEST (MCSFuseT, create_remove_write)
  {
    auto const size {std::size_t {1 << 16}};
    auto const number_of_files {std::size_t {1 << 14}};

    auto const buffer
      { util::Buffer<char[]>
        { size
        , std::make_unique_for_overwrite<char[]> (size)
        }
      };

    std::ranges::generate
      ( buffer.data<char>()
      , [ engine {std::default_random_engine()}
        , generator {std::uniform_int_distribution<char>{}}
        ]() mutable
        {
          return generator (engine);
        }
      );

    auto const file_names
      { std::invoke
        ( [&]
          {
            auto _file_names {std::vector<std::filesystem::path>{}};
            _file_names.reserve (number_of_files);

            std::generate_n
              ( std::back_inserter (_file_names)
              , number_of_files
              , [&, i {std::size_t {0}}]() mutable
                {
                  return this->_path / std::to_string (i++);
                }
              );

            return _file_names;
          }
        )
      };

    auto const benchmark
      { [&] (auto description, auto s, auto k, auto tag, auto fun)
        {
          auto const time
            { util::timed
              ( [&]
                {
                  std::ranges::for_each
                    ( file_names
                    , [&] (auto const& file_name)
                      {
                        std::invoke (fun, file_name);
                      }
                    );
                }
              )
            };

          fmt::print
            ( "{}: {} x {} = {}: {} -> {} {}/sec\n"
            , description
            , file_names.size(), s
            , file_names.size() * s
            , time
            , time.per_second (file_names.size() * s / k)
            , tag
            );
        }
      };

    for (auto description : {"create_non-existing", "create_existing"})
    {
      benchmark
        ( description
        , 1UL, 1UL << 10UL, "KFiles"
        , [&] (auto const& file_name)
          {
            std::ignore = util::fopen (file_name, "wb");
          }
        );
    }

    for (auto description : {"remove_existing_empty", "remove_non-existing"})
    {
      benchmark
        ( description
        , 1UL, 1UL << 10UL, "KFiles"
        , [&] (auto const& file_name)
          {
            std::filesystem::remove (file_name);
          }
        );
    }

    for (auto description : {"write_into_nonexisting", "write_into_existing"})
    {
      benchmark
        ( description
        , size, 1UL << 20UL, "MByte"
        , [&] (auto const& file_name)
          {
            auto const open_file {util::fopen (file_name, "wb")};
            auto const bytes_written
              { util::syscall::write
                ( util::syscall::fileno (open_file.get())
                , buffer.data<char>().data()
                , size
                )
              };

            if (std::cmp_not_equal (bytes_written, size))
            {
              throw std::runtime_error {"Could not write all bytes"};
            }
          }
        );
    }

    {
      auto const read_buffer
        { mcs::util::Buffer<char[]>
          { size
          , std::make_unique_for_overwrite<char[]> (size)
          }
        };

      benchmark
        ( "read_and_verify"
        , size, 1UL << 20UL, "MByte"
        , [&] (auto const& file_name)
          {
            std::ranges::fill (read_buffer.data<char>(), '\0');

            auto const open_file {util::fopen (file_name, "rb")};
            auto const bytes_read
              { util::syscall::read
                ( util::syscall::fileno (open_file.get())
                , read_buffer.data<char>().data()
                , size
                )
              };

            if (std::cmp_not_equal (bytes_read, size))
            {
              throw std::runtime_error {"Could not read all bytes"};
            }

            if (! std::ranges::equal
                 ( buffer.data<char>()
                 , read_buffer.data<char>()
                 )
               )
            {
              throw std::runtime_error {"Verification failed"};
            }
          }
        );
    }

    for (auto description : {"remove_existing_filled"})
    {
      benchmark
        ( description
        , 1UL, 1UL << 10UL, "KFiles"
        , [&] (auto const& file_name)
          {
            std::filesystem::remove (file_name);
          }
        );
    }
  }
}
