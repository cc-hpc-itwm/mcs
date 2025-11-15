# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_rpc
  PRIVATE ScopedRunningIOContext.cpp
  PRIVATE access_policy/Concurrent.cpp
  PRIVATE access_policy/Exclusive.cpp
  PRIVATE access_policy/Sequential.cpp
  PRIVATE detail/Buffer.cpp
  PRIVATE detail/Completion.cpp
  PRIVATE detail/ResultOrError.cpp
  PRIVATE error/Completion.cpp
  PRIVATE error/HandlerException.cpp
  PRIVATE error/HandshakeFailed.cpp
  PRIVATE error/internal/UnknownCommand.cpp
  PRIVATE multi_client/Errors.cpp
  PRIVATE multi_client/ParallelCallsLimit.cpp
  PRIVATE multi_client/detail/CallID.cpp
  PRIVATE multi_client/detail/ClientObserver.cpp
  PRIVATE multi_client/detail/Counters.cpp
)
set_target_properties (mcs_rpc PROPERTIES POSITION_INDEPENDENT_CODE ON)
