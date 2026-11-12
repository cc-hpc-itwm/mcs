// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/format.h>
#include <mcs/core/storage/select.hpp>
#include <mcs/fuse/content/implementation/Realloc.hpp>
#include <mcs/fuse/content/implementation/Storage.hpp>
#include <mcs/fuse/content/implementation/Vector.hpp>
#include <mcs/fuse/option.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace mcs::fuse::content::selection
{
  template<typename SupportedStorageImplementations, typename Function>
    auto select (Arguments const& arguments, Function&& function)
  {
    if (arguments.content() == "Vector")
    {
      return std::forward<Function> (function).template operator()
        < implementation::Vector
        > ( implementation::Vector::State{}
          );
    }

    if (arguments.content() == "Realloc")
    {
      return std::forward<Function> (function).template operator()
        < implementation::Realloc
        > ( implementation::Realloc::State{}
          );
    }

    if (arguments.content() != "Storage" || !arguments.storage())
    {
      throw std::invalid_argument
        { fmt::format ( "expected --{0} Vector, Realloc, or Storage"
                        " with storage options"
                      , fuse::option::content
                      )
        };
    }

    return core::storage::selection::select<SupportedStorageImplementations>
      ( *arguments.storage()
      , [&]<core::storage::is_implementation StorageImplementation>
          ( core::storage::selection::Parameters<StorageImplementation>
              parameters
          )
        {
          using Content = implementation::Storage<StorageImplementation>;

          return std::forward<Function> (function)
            .template operator()<Content>
              ( typename Content::State
                { std::move (parameters.create)
                , std::move (parameters.size_max)
                , std::move (parameters.size_used)
                , std::move (parameters.segment_create)
                , std::move (parameters.segment_remove)
                , std::move (parameters.chunk_description)
                }
              );
        }
      );
  }

  template<typename SupportedStorageImplementations, typename Function>
    auto select
      ( util::arguments::View arguments
      , Function&& function
      )
  {
    if (arguments.size() == 0UL)
    {
      throw std::invalid_argument
        { "missing CONTENT; expected 'Vector', 'Realloc', or 'Storage'"
        };
    }

    auto const content {std::string_view {arguments.at (0UL)}};

    if (content == "Vector" || content == "Realloc")
    {
      return select<SupportedStorageImplementations>
        ( Arguments {std::string {content}, {}}
        , [&]<is_content Content> (typename Content::State state)
          {
            return std::forward<Function> (function).template operator()
              < Content
              > ( std::move (state)
                , arguments.from (1UL)
                );
          }
        );
    }

    if (content == "Storage")
    {
      if (arguments.size() < 7UL)
      {
        throw std::invalid_argument
          { "'Storage' requires six storage arguments"
          };
      }

      return select<SupportedStorageImplementations>
        ( Arguments
          { "Storage"
          , core::storage::selection::Arguments
            { .storage = arguments.at (1UL)
            , .parameter_size_max = arguments.at (2UL)
            , .parameter_size_used = arguments.at (3UL)
            , .parameter_segment_create = arguments.at (4UL)
            , .parameter_segment_remove = arguments.at (5UL)
            , .parameter_chunk_description = arguments.at (6UL)
            }
          }
        , [&]<is_content Content> (typename Content::State state)
          {
            return std::forward<Function> (function).template operator()
              < Content
              > ( std::move (state)
                , arguments.from (7UL)
                );
          }
        );
    }

    throw std::invalid_argument
      { "unknown CONTENT; expected 'Vector', 'Realloc', or 'Storage'"
      };
  }
}
