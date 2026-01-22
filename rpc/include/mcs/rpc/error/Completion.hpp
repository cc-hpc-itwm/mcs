// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <mcs/Error.hpp>

namespace mcs::rpc::error
{
  // The completion handler of the remote call received an exception,
  // e.g. "lost connection". It is unspecified (and unknown) whether
  // or not the remote handler has been started or even completed.
  //
  // The concrete error is saved and accessible via reason().
  //
  struct Completion : public mcs::Error
  {
    explicit Completion (std::exception_ptr) noexcept;

    ~Completion() override;
    Completion (Completion const&) = default;
    Completion (Completion&&) noexcept = default;
    auto operator= (Completion const&) -> Completion& = default;
    auto operator= (Completion&&) noexcept  -> Completion& = default;

    // Returns the concrete error received by the completion handler.
    //
    auto reason() const noexcept -> std::exception_ptr;

  private:
    std::exception_ptr _reason;
  };
}
