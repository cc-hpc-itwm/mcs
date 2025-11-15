// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/iov_backend/Parameter.hpp>

namespace mcs::iov_backend
{
  auto Parameter::operator<<
    ( util::ASIO::AnyConnectable provider
    ) -> Parameter&
  {
    _provider = provider;

    return *this;
  }

  auto Parameter::operator<<
    ( NumberOfThreads number_of_threads
    ) -> Parameter&
  {
    _number_of_threads = number_of_threads;

    return *this;
  }

  auto Parameter::operator<<
    ( IndirectCommunication indirect_communication
    ) -> Parameter&
  {
    _indirect_communication = indirect_communication;

    return *this;
  }

  auto Parameter::operator<<
    ( DirectCommunication direct_communication
    ) -> Parameter&
  {
    _direct_communication = direct_communication;

    return *this;
  }
}
