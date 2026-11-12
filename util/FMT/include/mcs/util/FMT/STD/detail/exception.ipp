// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<std::exception>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<std::exception>::format
      ( std::exception const& error
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    fmt::format_to (context.out(), "{}", error.what());

    try
    {
      std::rethrow_if_nested (error);
    }
    catch (std::exception const& rethrown_exception)
    {
      fmt::format_to (context.out(), ": {}", rethrown_exception);
    }
    catch (...)
    {
      fmt::format_to (context.out(), "UNKNOWN");
    }

    return context.out();
  }

  template<typename ParseContext>
    constexpr auto formatter<std::exception_ptr>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<std::exception_ptr>::format
      ( std::exception_ptr const& error
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    try
    {
      std::rethrow_exception (error);
    }
    catch (std::exception const& rethrown_exception)
    {
      fmt::format_to (context.out(), "{}", rethrown_exception);
    }
    catch (...)
    {
      fmt::format_to (context.out(), "UNKNOWN");
    }

    return context.out();
  }
}
