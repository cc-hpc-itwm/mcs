// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <fstream>
#include <mcs/Error.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/Events.hpp>
#include <mcs/core/storage/tracer/log_file/Tag.hpp>
#include <mcs/core/storage/tracer/log_file/parameter/Create.hpp>

namespace mcs::core::storage::tracer
{
  // A tracer that prints all events into a file.
  //
  template<is_implementation Storage>
    struct LogFile
  {
    using Tag = log_file::Tag;

    struct Parameter
    {
      using Create = log_file::parameter::Create<Storage>;
    };

    explicit LogFile (Parameter::Create);

    template<typename Event, typename... Args>
      requires ( trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
      auto trace (Args&&...) -> void;

    struct Error
    {
      struct CouldNotOpenFile : public mcs::Error
      {
        [[nodiscard]] constexpr auto path
          (
          ) const noexcept -> std::filesystem::path const&
          ;

        ~CouldNotOpenFile() override;
        CouldNotOpenFile (CouldNotOpenFile const&) = default;
        CouldNotOpenFile (CouldNotOpenFile&&) noexcept = default;
        auto operator= (CouldNotOpenFile const&) -> CouldNotOpenFile& = default;
        auto operator= (CouldNotOpenFile&&) noexcept  -> CouldNotOpenFile& = default;

      private:
        template<is_implementation> friend struct LogFile;
        explicit CouldNotOpenFile (std::filesystem::path);
        std::filesystem::path _path;
      };
    };

  private:
    std::ofstream _log;
  };
}

#include "detail/LogFile.ipp"
