// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::tracer::log_file::Tag>::parse
      (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::tracer::log_file::Tag>::format
      ( mcs::core::storage::tracer::log_file::Tag const& tag
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    std::ignore = tag;

    return fmt::format (context.out(), "tracer::LogFile");
  }
}
