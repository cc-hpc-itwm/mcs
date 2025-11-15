// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <functional>
#include <mcs/Error.hpp>
#include <mcs/util/require_semi.hpp>
#include <utility>

namespace
{
  struct IOVBackendBinError : public mcs::Error
  {
    template<typename Description>
      IOVBackendBinError (Description description, std::exception_ptr error)
        : mcs::Error {fmt::format ("{}: {}", description, error)}
    {}

    MCS_ERROR_COPY_MOVE_DEFAULT (IOVBackendBinError);
  };
  IOVBackendBinError::~IOVBackendBinError() = default;
}

#define MCS_IOV_BACKEND_BIN_MAKE_ERROR_IMPL(_name)      \
  namespace                                             \
  {                                                     \
    struct _name final : public IOVBackendBinError      \
    {                                                   \
      explicit _name (std::exception_ptr error)         \
        : IOVBackendBinError {#_name, error}            \
      {}                                                \
                                                        \
      MCS_ERROR_COPY_MOVE_DEFAULT (_name);              \
    };                                                  \
    _name::~_name() = default;                          \
  } MCS_UTIL_REQUIRE_SEMI()

namespace
{
  template<typename... Queues>
    ErrorCollector<Queues...>::ErrorCollector (Queues*... queues) noexcept
      : _queues {std::make_tuple (queues...)}
  {}

  template<typename... Queues>
     constexpr auto ErrorCollector<Queues...>::empty() const noexcept -> bool
  {
    return _errors.empty();
  }
  template<typename... Queues>
     constexpr auto ErrorCollector<Queues...>::errors
      (
      ) const noexcept -> std::vector<std::exception_ptr> const&
  {
    return _errors;
  }

  template<typename... Queues>
    template<typename Error, typename Fun>
      auto ErrorCollector<Queues...>::collect_errors_as (Fun&& fun)
  {
    try
    {
      std::invoke (std::forward<Fun> (fun));
    }
    catch (IOVBackendBinError const&) // NOLINT (bugprone-empty-catch)
    {
      // do nothing, go home, produced by another thread
    }
    catch (...)
    {
      auto error {std::make_exception_ptr (Error {std::current_exception()})};

      auto const lock {std::unique_lock {_guard}};

      if (_errors.empty())
      {
        std::apply
          ( [&] (Queues*... queues) noexcept
            {
              (queues->error (error), ...);
            }
          , _queues
          );
      }

      _errors.emplace_back (error);
    }
  }

  template<typename... Queues>
    constexpr auto make_error_collector (Queues*... queues)
  {
    return ErrorCollector<Queues...> {queues...};
  }
}
