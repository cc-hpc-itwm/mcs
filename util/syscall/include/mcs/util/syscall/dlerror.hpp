// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mutex>
#include <string>

namespace mcs::util::syscall::detail
{
  struct CheckDLError
  {
    CheckDLError (std::string call);

    template<typename Ret> auto operator() (Ret value) const -> Ret
    {
      throw_if_there_is_a_dlerror();

      return value;
    }

  private:
    std::string _call;

    auto throw_if_there_is_a_dlerror() const -> void;

    // \note dlerror() is specified to be not threadsafe. In fact,
    // it uses a single static buffer to print the error message
    // in. Thus, sequentialize all functions that call dlerror.
    //
    static std::mutex _guard;
    std::scoped_lock<std::mutex> const _lock {_guard};
  };
}

namespace mcs::util::syscall::error
{
  // Functions that check dlerror() throw this exception in case there
  // is an error detected.
  //
  struct DLError : public mcs::Error
  {
    [[nodiscard]] auto call() const noexcept -> std::string;
    [[nodiscard]] auto reason() const noexcept -> std::string;

    ~DLError() override;
    DLError (DLError const&) = default;
    DLError (DLError&&) noexcept = default;
    auto operator= (DLError const&) -> DLError& = default;
    auto operator= (DLError&&) noexcept  -> DLError& = default;

  private:
    friend struct ::mcs::util::syscall::detail::CheckDLError;

    DLError (std::string, char const*);

    std::string _call;
    std::string _reason;
  };
}
