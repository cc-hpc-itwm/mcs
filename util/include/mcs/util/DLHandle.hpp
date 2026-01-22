// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <dlfcn.h>
#include <exception>
#include <filesystem>
#include <mcs/Error.hpp>
#include <memory>

namespace mcs::util
{
  // RAII wrapper to the dlopen() family functions for runtime loading
  // of shared objects.
  //
  struct DLHandle
  {
    static constexpr auto flags_default {RTLD_NOW};

    // Open the shared object at path, with flags passed to dlopen().
    //
    DLHandle ( std::filesystem::path const& path
             , int flags = flags_default
             );

    // Retrieve the symbol identified by name and cast it to T.
    //
    template<typename T>
      auto symbol (char const*) const -> T*;

    struct Error
    {
      struct Symbol : public mcs::Error
      {
        [[nodiscard]] constexpr auto name() const noexcept -> char const*;

        ~Symbol() override;
        Symbol (Symbol const&) = default;
        Symbol (Symbol&&) noexcept = default;
        auto operator= (Symbol const&) -> Symbol& = default;
        auto operator= (Symbol&&) noexcept  -> Symbol& = default;

      private:
        friend struct DLHandle;

        explicit Symbol (char const*);

        char const* _name;
      };
    };

  private:
    struct DLClose
    {
      auto operator() (void*) const noexcept -> void;
    };
    std::unique_ptr<void, DLClose> _loaded_library;
  };
}

// Given a DLHandle, call symbol (name) with the name and type of
// the given symbol.
//
// \note The name is *not* mangled but taken verbatim, i.e. requires
// an 'extern "C"' specification on the symbol given.
//
// EXAMPLE:
//   extern "C" { auto f_impl (int) -> void; }
//   auto const dlhandle {path};
//   auto const f {dlhandle.symbol<decltype (f_impl)> ("f_impl")};
//   std::invoke (f, 42);

#include "detail/DLHandle.ipp"
