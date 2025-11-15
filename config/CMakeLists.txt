# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_config INTERFACE)
target_compile_features (mcs_config INTERFACE cxx_std_${MCS_CXX_STANDARD})
target_include_directories (mcs_config INTERFACE include)

include ("${CMAKE_CURRENT_LIST_DIR}/options.cmake")

if (MCS_WARNINGS)
  include (CheckCXXCompilerFlag)
  function (add_compiler_flag flag)
    string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${flag}" compiler_flag_var)
    check_cxx_compiler_flag (${flag} ${compiler_flag_var})
    if (${${compiler_flag_var}})
      target_compile_options (mcs_config INTERFACE ${flag})
    endif()
  endfunction()

  add_compiler_flag (-W)
  add_compiler_flag (-Wall)
  add_compiler_flag (-Werror)
  add_compiler_flag (-Wextra)
  add_compiler_flag (-Wpedantic)

  add_compiler_flag (-Wcomments)
  add_compiler_flag (-Wctor-dtor-privacy)
  add_compiler_flag (-Wdangling-else)
  add_compiler_flag (-Wdelete-non-virtual-dtor)
  add_compiler_flag (-Wduplicated-branches)
  add_compiler_flag (-Wduplicated-cond)
  add_compiler_flag (-Wextra-semi)
  add_compiler_flag (-Wimplicit-fallthrough)
  add_compiler_flag (-Wlogical-op)
  add_compiler_flag (-Wmissing-declarations)
  add_compiler_flag (-Wnon-virtual-dtor)
  add_compiler_flag (-Wold-style-cast)
  add_compiler_flag (-Wswitch-enum)
  add_compiler_flag (-Wswitch-unreachable)
  add_compiler_flag (-Wundef)

  add_compiler_flag (-Wdeprecated-copy-dtor)
  add_compiler_flag (-Wdeprecated-enum-enum-conversion)
  add_compiler_flag (-Wdeprecated-enum-float-conversion)

  add_compiler_flag (-Wshadow)
  add_compiler_flag (-Wuninitialized)
  add_compiler_flag (-Wunused)

  add_compiler_flag (-Wconversion)
  add_compiler_flag (-Wconversion-null)
  add_compiler_flag (-Wdouble-promotion)
  add_compiler_flag (-Wfloat-equal)
  add_compiler_flag (-Wlong-long)
  add_compiler_flag (-Wnoexcept)
  # gcc1410 clash with asio
  # add_compiler_flag (-Wnull-dereference)
  add_compiler_flag (-Wshift-count-negative)
  add_compiler_flag (-Wshift-count-overflow)
  add_compiler_flag (-Wshift-overflow)
  add_compiler_flag (-Wsign-conversion)
  add_compiler_flag (-Wsign-promo)
  add_compiler_flag (-Wuseless-cast)

  add_compiler_flag (-Wsuggest-attribute=noreturn)
  add_compiler_flag (-Wsuggest-final-methods)
  add_compiler_flag (-Wsuggest-final-types)
  add_compiler_flag (-Wsuggest-override)

  add_compiler_flag (-Wconditionally-supported)
  add_compiler_flag (-Wdate-time)
  add_compiler_flag (-Wdisabled-optimization)
  add_compiler_flag (-Winvalid-memory-model)

  add_compiler_flag (-faligned-new)
  add_compiler_flag (-Wcast-align)

  add_compiler_flag (-Wno-variadic-macros)

  # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107919
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if ( (  CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11"
         OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12"
         OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13"
         OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "14"
         OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15"
         )
       AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16"
       )
       add_compiler_flag (-Wno-error=maybe-uninitialized)
    endif()
  endif()
endif()
