// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, std::exception)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (error, ctx, std::exception)
  {
    fmt::format_to (ctx.out(), "{}", error.what());

    try
    {
      std::rethrow_if_nested (error);
    }
    catch (std::exception const& rethrown_exception)
    {
      fmt::format_to (ctx.out(), ": {}", rethrown_exception);
    }
    catch (...)
    {
      fmt::format_to (ctx.out(), "UNKNOWN");
    }

    return ctx.out();
  }

  MCS_UTIL_FMT_DEFINE_PARSE (ctx, std::exception_ptr)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (error, ctx, std::exception_ptr)
  {
    try
    {
      std::rethrow_exception (error);
    }
    catch (std::exception const& rethrown_exception)
    {
      fmt::format_to (ctx.out(), "{}", rethrown_exception);
    }
    catch (...)
    {
      fmt::format_to (ctx.out(), "UNKNOWN");
    }

    return ctx.out();
  }
}
