// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <array>
#include <fmt/format.h>
#include <functional>
#include <mcs/core/storage/select.hpp>
#include <mcs/fuse/content/select.hpp>
#include <mcs/fuse/option.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace mcs::fuse::content::selection
{
  namespace
  {
    constexpr auto storage_options
      { std::array
        { fuse::option::storage
        , fuse::option::storage_size_max
        , fuse::option::storage_size_used
        , fuse::option::storage_segment_create
        , fuse::option::storage_segment_remove
        , fuse::option::storage_chunk_description
        }
      };
  }

  auto options::schema() -> util::command_line::Schema
  {
    auto schema {util::command_line::Schema{}.add (fuse::option::content)};

    std::ranges::for_each
      ( storage_options
      , [&] (auto const& parameter)
        {
          schema.add (parameter);
        }
      );

    return schema;
  }

  Arguments::Arguments (util::command_line::Parsed const& parsed)
    : _content {parsed.at (fuse::option::content)}
    , _storage
      { std::invoke
        ( [&]() -> decltype (_storage)
          {
            if (_content == "Storage")
            {
              auto const value
                { [&] (auto const& name)
                  {
                    return std::string {parsed.at (name)};
                  }
                };

              return core::storage::selection::Arguments
                { .storage = value (option::storage)
                , .parameter_size_max = value (option::storage_size_max)
                , .parameter_size_used = value (option::storage_size_used)
                , .parameter_segment_create = value (option::storage_segment_create)
                , .parameter_segment_remove = value (option::storage_segment_remove)
                , .parameter_chunk_description = value (option::storage_chunk_description)
                };
            }
            else if ( std::ranges::any_of
                      ( storage_options
                      , [&] (auto const& parameter) noexcept
                        {
                          return !parsed.occurrences (parameter).empty();
                        }
                      )
                    )
            {
              throw std::invalid_argument
                { fmt::format
                  ( "storage options require --{} Storage"
                  , fuse::option::content
                  )
                };
            }

            return {};
          }
        )
      }
  {}

  Arguments::Arguments
    ( std::string content
    , std::optional<core::storage::selection::Arguments> storage
    )
      : _content {std::move (content)}
      , _storage {std::move (storage)}
  {}

  auto Arguments::content() const noexcept -> std::string const&
  {
    return _content;
  }

  auto Arguments::storage
    (
    ) const noexcept -> std::optional<core::storage::selection::Arguments> const&
  {
    return _storage;
  }
}
