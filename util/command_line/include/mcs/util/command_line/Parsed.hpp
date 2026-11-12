// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <map>
#include <mcs/util/command_line/Schema.hpp>
#include <mcs/util/command_line/error/UnknownMarker.hpp>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace mcs::util::command_line
{
  // Owns the containers and registered marker names. Option names,
  // values and embedded arguments borrow their strings from the
  // input. Input strings must remain alive and unchanged while
  // results or returned views are used. The pointer array and schema
  // need not. Values and embedded arguments retain their null
  // terminators. Option names may be bounded subviews without a
  // terminator.
  //
  struct Parsed
  {
    using Occurrence = std::vector<std::string_view>;
    using Occurrences = std::vector<Occurrence>;

    [[nodiscard]] explicit Parsed
      ( std::span<char const* const>
      , Schema const&
      );

    [[nodiscard]] auto at (std::string_view) const -> std::string_view;
    [[nodiscard]] auto optional
      ( std::string_view
      ) const -> std::optional<std::string_view>
      ;
    [[nodiscard]] auto occurrences
      ( std::string_view
      ) const -> Occurrences const&
      ;
    [[nodiscard]] auto flag (std::string_view) const -> bool;

    // Returns the registered marker's arguments, empty if unused.
    // Throws error::UnknownMarker if the marker was not registered.
    //
    [[nodiscard]] auto embedded
      ( std::string_view
      ) const -> std::vector<std::string_view> const&
      ;

  private:
    std::map<std::string_view, Occurrences, std::less<>> _options;
    Schema::Embedded _embedded;
  };
}
