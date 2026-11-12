// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/Error.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/daemon/option.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/true_once.hpp>
#include <optional>
#include <string>
#include <type_traits>

namespace mcs::util
{
  // Coordinate startup signaling between daemon child and parent.
  //
  // The main implementation must return int and call ready() exactly
  // once or throw an exception. In daemonized mode ready() flushes
  // stdout and stderr, redirects the standard streams to /dev/null,
  // and then releases the startup parent. Startup failures are
  // printed by the startup parent. Foreground mode preserves the
  // standard streams and enforces the same signaling
  // contract. Applications that require runtime logging must
  // configure another sink before calling ready().
  //
  // EXAMPLE:
  //   namespace
  //   {
  //     auto help (mcs::util::arguments::View args) -> void
  //     {
  //       fmt::print
  //         ( "Usage: {} endpoint [{}]\n"
  //         , args.at (0)
  //         , mcs::util::daemon::option::foreground
  //         );
  //     }
  //
  //     auto daemon_main
  //       ( mcs::util::arguments::View args
  //       , mcs::util::Daemon& daemon
  //       ) -> int
  //     {
  //       // Process args, startup, publish connection information, ...
  //       std::ignore = args;
  //
  //       // Signal that startup succeeded and parent may exit.
  //       daemon.ready();
  //
  //       // Run forever.
  //       while (true)
  //       {
  //       }
  //     }
  //   }
  //
  //   auto main (int argc, char const** argv) noexcept -> int
  //   {
  //     return mcs::util::daemonize (argc, argv, help, daemon_main);
  //   }
  //
  struct Daemon;

  template<typename Main>
    concept is_daemonized_main_implementation
      = std::invocable<Main, arguments::View, Daemon&>
      && std::same_as
           < std::invoke_result_t<Main, arguments::View, Daemon&>
           , int
           >
      ;

  // Consumes the top-level daemon::option::foreground flag and then
  // forwards the remaining arguments to the main implementation.
  // Foreground errors and startup-parent errors propagate to the
  // caller. The daemon child reports startup errors to the parent and
  // returns EXIT_FAILURE.
  //
  template<is_daemonized_main_implementation Main>
    [[nodiscard]] auto daemonize (arguments::View, Main&&) -> int;

  // Command-line adapter using util::main to construct
  // arguments::View, handle help before daemonization, and report
  // exceptions to stderr as EXIT_FAILURE.
  //
  template< is_help_implementation Help
          , is_daemonized_main_implementation Main
          >
    [[nodiscard]] auto daemonize
      ( int
      , char const**
      , Help&&
      , Main&&
      ) noexcept -> int
    ;
}

namespace mcs::util
{
  // Startup completion is signaled explicitly with a call to ready()
  // or an exception. Calling ready() more than once throws
  // Daemon::Error::MultipleSignals. If neither ready() is called nor
  // an exception is thrown, then daemonize() reports startup failure.
  //
  struct Daemon
  {
    auto ready() -> void;

    struct Error
    {
      struct MultipleSignals : public mcs::Error
      {
      public:
        ~MultipleSignals() override;
        MultipleSignals (MultipleSignals const&) = default;
        MultipleSignals (MultipleSignals&&) noexcept = default;
        auto operator= (MultipleSignals const&) -> MultipleSignals& = default;
        auto operator= (MultipleSignals&&) noexcept -> MultipleSignals& = default;

      private:
        friend struct Daemon;

        MultipleSignals();
      };
    };

    ~Daemon() = default;
    Daemon (Daemon const&) = delete;
    Daemon (Daemon&&) = delete;
    auto operator= (Daemon const&) -> Daemon& = delete;
    auto operator= (Daemon&&) -> Daemon& = delete;

  private:
    template<is_daemonized_main_implementation Main>
      friend auto daemonize (arguments::View, Main&&) -> int;

    [[nodiscard]] constexpr Daemon() = default;
    [[nodiscard]] constexpr explicit Daemon (int);

    template<is_daemonized_main_implementation Main>
      auto run (arguments::View, Main&&) -> int;

    auto fail (std::string reason) -> void;

    util::TrueOnce _signal_pending{};
    std::optional<int> _write_end{};
    std::optional<std::string> _foreground_failure_reason{};
  };
}

#include "detail/daemon.ipp"
