# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_rpc name)
  add_executable (mcs_test_rpc_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_rpc_${name}
    PRIVATE mcs_config
    PRIVATE mcs_rpc
    PRIVATE mcs_testing
    PRIVATE mcs_testing_RPC
    ${ARGN}
  )
  gtest_discover_tests (mcs_test_rpc_${name})
endfunction()

mcs_test_rpc (Concepts)
mcs_test_rpc (ScopedRunningIOContextNumberOfThreads)
mcs_test_rpc (call)
mcs_test_rpc (clients_must_support_a_prefix_of_the_provided_commands)
mcs_test_rpc (count)
mcs_test_rpc (drop_async_operation)
mcs_test_rpc (drop_client_while_operation_is_in_progress)
mcs_test_rpc (execution_order)
mcs_test_rpc (kvs)
mcs_test_rpc (many PRIVATE mcs_util_syscall)
mcs_test_rpc (many_concurrent_requests)
mcs_test_rpc (messing_up_functions_with_the_same_signature_causes_an_error_during_handeshake)
mcs_test_rpc (multi_client)
mcs_test_rpc (multi_threaded_server)
mcs_test_rpc (streaming_client)
mcs_test_rpc (streaming_handler)

add_subdirectory (bin)
