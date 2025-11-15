# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_util name)
  add_executable (mcs_test_util_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util
    ${ARGN}
  )
  gtest_discover_tests (mcs_test_util_${name})
endfunction()

mcs_test_util (Buffer)
mcs_test_util (HeterogeneousMap PRIVATE mcs_util_HeterogeneousMap)
mcs_test_util (MapWithHitMissCallbacks)
mcs_test_util (RangesIterator)
mcs_test_util (TaggedRange)
mcs_test_util (TemporaryDirectory PRIVATE mcs_util_FMT)
mcs_test_util (TemporaryFile)
mcs_test_util (cast)
mcs_test_util (divru)
mcs_test_util (fwd_capture)
mcs_test_util (member_AUTO)
mcs_test_util (not_null)
mcs_test_util (select)
mcs_test_util (true_once)

add_subdirectory (ASIO)
add_subdirectory (FMT)
add_subdirectory (distribution)
add_subdirectory (dlhandle)
add_subdirectory (read)
add_subdirectory (type)
