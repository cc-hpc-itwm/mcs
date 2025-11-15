// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/post.hpp>
#include <asio/signal_set.hpp>
#include <asio/thread_pool.hpp>
#include <cstdio>
#include <exception>
#include <mcs/share_service/Provider.hpp>
#include <mcs/share_service/tcp_provider.h>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/TemporaryDirectory.hpp>
#include <mcs/util/TemporaryFile.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <tuple>

  struct mcs_share_service_tcp_provider
  {
    template<typename ProtocolVersion>
      mcs_share_service_tcp_provider
        ( ProtocolVersion protocol
        , unsigned short int port
        , char const* provider_path
        , int number_of_threads
        )
          : _number_of_threads {number_of_threads}
          , _provider {_io_context, asio::ip::tcp::endpoint {protocol, port}}
          , _provider_path {provider_path}
    {
      _signals.async_wait ([&] (auto, auto) { _io_context.stop(); });
    }
    int _number_of_threads;
    asio::thread_pool _threads
      { mcs::util::cast<std::size_t> (_number_of_threads)
      };
    asio::io_context _io_context {_number_of_threads};
    asio::signal_set _signals {_io_context};
    mcs::share_service::Provider<asio::ip::tcp> _provider;
    mcs::util::TemporaryDirectory _provider_path;
    mcs::util::TemporaryFile _pid_information
      { _provider_path.path() / "PID"
      , "{}"
      , mcs::util::syscall::getpid()
      };
    mcs::util::TemporaryFile _provider_information
      { _provider_path.path() / "PROVIDER"
      , "{}"
      , mcs::util::ASIO::make_connectable (_provider.local_endpoint())
      };
  };

#ifdef __cplusplus
extern "C"
{
#endif

auto mcs_share_service_create_tcp_provider
  ( int tcp_version
  , unsigned short int port
  , char const* path
  , int number_of_threads
  , mcs_share_service_tcp_provider** provider
  ) -> int
{
  if (provider == nullptr || *provider != nullptr)
  {
    return -1;
  }

  try
  {
    if (tcp_version == 4)
    {
      *provider = new mcs_share_service_tcp_provider
        { asio::ip::tcp::v4()
        , port
        , path
        , number_of_threads
        };

      return 0;
    }
    else if (tcp_version == 6)
    {
      *provider = new mcs_share_service_tcp_provider
        { asio::ip::tcp::v6()
        , port
        , path
        , number_of_threads
        };

      return 0;
    }

    return -3;
  }
  catch (...)
  {
    mcs::util::FMT::print_noexcept
      ( stderr
      , "Error: {}\n"
      , std::current_exception()
      );

    return -2;
  }
}

auto mcs_share_service_destroy_tcp_provider
  ( mcs_share_service_tcp_provider** provider
  ) -> int
{
  if (provider == nullptr || *provider == nullptr)
  {
    return -1;
  }

  try
  {
    delete *provider;

    *provider = nullptr;

    return 0;
  }
  catch (...)
  {
    mcs::util::FMT::print_noexcept
      ( stderr
      , "Error: {}\n"
      , std::current_exception()
      );

    return -2;
  }
}

auto mcs_share_service_stop_tcp_provider_on_signal
  ( mcs_share_service_tcp_provider* provider
  , int signal
  ) -> int
{
  if (provider == nullptr)
  {
    return -1;
  }

  try
  {
    provider->_signals.add (signal);

    return 0;
  }
  catch (...)
  {
    mcs::util::FMT::print_noexcept
      ( stderr
      , "Error: {}\n"
      , std::current_exception()
      );

    return -2;
  }
}

auto mcs_share_service_run_tcp_provider
  ( mcs_share_service_tcp_provider* provider
  ) -> int
{
  if (provider == nullptr)
  {
    return -1;
  }

  try
  {
    for (auto i {0}; i != provider->_number_of_threads; ++i)
    {
      asio::post (provider->_threads, [&] { provider->_io_context.run(); });
    }

    provider->_threads.join();

    return 0;
  }
  catch (...)
  {
    mcs::util::FMT::print_noexcept
      ( stderr
      , "Error: {}\n"
      , std::current_exception()
      );

    return -2;
  }
}

auto mcs_share_service_stop_tcp_provider
  ( mcs_share_service_tcp_provider* provider
  ) -> int
{
  if (provider == nullptr)
  {
    return -1;
  }

  try
  {
    provider->_io_context.stop();

    return 0;
  }
  catch (...)
  {
    mcs::util::FMT::print_noexcept
      ( stderr
      , "Error: {}\n"
      , std::current_exception()
      );

    return -2;
  }
}

#ifdef __cplusplus
}
#endif
