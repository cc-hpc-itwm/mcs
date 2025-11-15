// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstring>
#include <fmt/format.h>
#include <mcs/iov_backend/Parameter.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

#define MCS_IOV_BACKEND_OPTION(option) 0 == std::strcmp (args[pos], #option)

namespace
{
  auto make_parameter_main (mcs::util::Args args) -> int
  {
    auto const usage
      { [&]
        {
          return fmt::format
            ( "usage: {}"
            " provider"
            " [--number-of-threads:transport-clients <NUMBER_OF_THREADS>]"
            " [--indirect-communication:number_of_buffers <NUM>]"
            " [--indirect-communication:maximum_transfer_size <MEMORY_SIZE>]"
            " [--indirect-communication:acquire_buffer_timeout_in_milliseconds <NUM>]"
            " [--indirect-communication:maximum_number_of_parallel_streams <NUM>]"
            " [--direct-communication:maximum_transfer_size <MEMORY_SIZE>]"
            " [--direct-communication:maximum_number_of_parallel_streams <NUM>]"
            , args[0]
            );
        }
      };

    if (args.size() < 2 || args.size() % 2 != 0)
    {
      throw std::invalid_argument {usage()};
    }

    auto parameter
      { mcs::iov_backend::Parameter
          { mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])
          }
      };

    // \todo command line parser
    for (auto pos {decltype (args.size()) {2}}; pos != args.size(); ++pos)
    {
      if (MCS_IOV_BACKEND_OPTION (--number-of-threads:transport-clients))
      {
        ++pos;

        parameter._number_of_threads._transport_clients
          = mcs::util::read::read<mcs::rpc::ScopedRunningIOContext::NumberOfThreads>
              ( args[pos]
              )
          ;
      }
      else if (MCS_IOV_BACKEND_OPTION (--indirect-communication:number_of_buffers))
      {
        ++pos;

        parameter._indirect_communication._number_of_buffers
          = mcs::util::read::read<std::size_t> (args[pos])
          ;
      }
      else if (MCS_IOV_BACKEND_OPTION (--indirect-communication:maximum_transfer_size))
      {
        ++pos;

        parameter._indirect_communication._maximum_transfer_size
          = mcs::util::read::read<mcs::core::memory::Size> (args[pos])
          ;
      }
      else if (MCS_IOV_BACKEND_OPTION (--indirect-communication:acquire_buffer_timeout_in_milliseconds))
      {
        ++pos;

        parameter._indirect_communication._acquire_buffer_timeout_in_milliseconds
          = mcs::util::read::read<std::size_t> (args[pos])
          ;
      }
      else if (MCS_IOV_BACKEND_OPTION (--indirect-communication:maximum_number_of_parallel_streams))
      {
        ++pos;

        parameter._indirect_communication._maximum_number_of_parallel_streams
          = mcs::util::read::read<std::size_t> (args[pos])
          ;
      }
      else if (MCS_IOV_BACKEND_OPTION (--direct-communication:maximum_transfer_size))
      {
        ++pos;

        parameter._direct_communication._maximum_transfer_size
          = mcs::util::read::read<mcs::core::memory::Size> (args[pos])
          ;
      }
      else if (MCS_IOV_BACKEND_OPTION (--direct-communication:maximum_number_of_parallel_streams))
      {
        ++pos;

        parameter._direct_communication._maximum_number_of_parallel_streams
          = mcs::util::read::read<std::size_t> (args[pos])
          ;
      }
      else
      {
        throw std::invalid_argument {usage()};
      }
    }

    fmt::print ("{}", parameter);

    return EXIT_SUCCESS;
 }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, make_parameter_main);
}
