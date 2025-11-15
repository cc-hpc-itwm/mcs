// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <utility>

namespace mcs::core::storage::tracer
{
  template<is_implementation Storage>
    LogFile<Storage>::LogFile (Parameter::Create create)
      : _log {create._path}
  {
    if (!_log)
    {
      throw typename Error::CouldNotOpenFile {create._path};
    }
  }
}

namespace mcs::core::storage::tracer
{
  template<is_implementation Storage>
    template<typename Event, typename... Args>
      requires ( trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
      auto LogFile<Storage>::trace (Args&&... args) -> void
  {
    util::execute_and_die_on_exception
      ( "Could not print trace event"
      , [&]
        {
          fmt::print (_log, "{}\n", Event {std::forward<Args> (args)...});
        }
      );
  }
}

namespace mcs::core::storage::tracer
{
  template<is_implementation Storage>
    LogFile<Storage>::Error::CouldNotOpenFile::CouldNotOpenFile
      ( std::filesystem::path path
      )
        : mcs::Error
          { fmt::format ("tracer::LogFile::CouldNotOpenFile: {}", path)
          }
        , _path {path}
  {}
  template<is_implementation Storage>
    LogFile<Storage>::Error::CouldNotOpenFile::~CouldNotOpenFile() = default;

  template<is_implementation Storage>
    constexpr auto LogFile<Storage>::Error::CouldNotOpenFile::path
      (
      ) const noexcept -> std::filesystem::path const&
  {
    return _path;
  }
}
