// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mutex>
#include <tuple>
#include <vector>

#define MCS_IOV_BACKEND_BIN_MAKE_ERROR(_name)   \
  MCS_IOV_BACKEND_BIN_MAKE_ERROR_IMPL(_name)

namespace
{
  template<typename... Queues>
    struct ErrorCollector
  {
    [[nodiscard]] constexpr auto empty() const noexcept -> bool;
    [[nodiscard]] constexpr auto errors
      (
      ) const noexcept -> std::vector<std::exception_ptr> const&
      ;

    template<typename Error, typename Fun>
      auto collect_errors_as (Fun&&);

  private:
    template<typename... Qs>
      friend constexpr auto make_error_collector (Qs*...);

    ErrorCollector (Queues*...) noexcept;

    std::mutex _guard;
    std::tuple<Queues*...> _queues;
    std::vector<std::exception_ptr> _errors;
  };

  template<typename... Queues>
    [[nodiscard]] constexpr auto make_error_collector (Queues*...);
}

#include "detail/error.ipp"
