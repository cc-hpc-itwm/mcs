// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/select.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <optional>
#include <string>

namespace mcs::fuse::content::selection
{
  class Arguments
  {
  public:
    explicit Arguments (util::command_line::Parsed const&);

    Arguments
      ( std::string
      , std::optional<core::storage::selection::Arguments>
      );

    [[nodiscard]] auto content() const noexcept -> std::string const&;
    [[nodiscard]] auto storage() const noexcept
      -> std::optional<core::storage::selection::Arguments> const&;

  private:
    std::string _content;
    std::optional<core::storage::selection::Arguments> _storage;
  };

  namespace options
  {
    [[nodiscard]] auto schema() -> util::command_line::Schema;
  }

  template<typename SupportedStorageImplementations, typename Function>
    [[nodiscard]] auto select (Arguments const&, Function&&);

  // Select a FUSE content implementation from command-line arguments.
  //
  // 'Vector' and 'Realloc' consume one argument. 'Storage' consumes
  // its name and the six core storage arguments, delegating their
  // parsing to core::storage::selection::select. The function
  // receives the selected content state and all remaining arguments.
  //
  // EXAMPLE:
  //
  //     using SupportedStorages = util::type::List<Files, Heap>;
  //
  //     content::selection::select<SupportedStorages>
  //       ( arguments
  //       , []<is_content Content>
  //           ( typename Content::State state
  //           , auto remaining
  //           )
  //         {
  //           return run<Content> (std::move (state), remaining);
  //         }
  //       );
  //
  template<typename SupportedStorageImplementations, typename Function>
    auto select (util::arguments::View, Function&&);
}

#include "detail/select.ipp"
