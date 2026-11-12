// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/syscall/getpid.hpp>

namespace mcs::testing
{
  template<typename Value>
    auto UniqID::Digest::operator<<
      ( std::optional<Value> const& value
      ) -> Digest&
  {
    if (value.has_value())
    {
      return *this << "1" << std::string_view {*value};
    }

    return *this << "0";
  }

  template<typename Prefix>
    UniqID::UniqID (Prefix prefix)
      : _test_info {::testing::UnitTest::GetInstance()->current_test_info()}
      , _name {_test_info->name()}
      , _test_suite_name {_test_info->test_suite_name()}
      , _prefix {fmt::format ("{}", prefix)}
      , _process_id {fmt::format ("{}", util::syscall::getpid())}
      , _id { fmt::format ( "{}-{}-{}-{}"
                          , _prefix
                          , _process_id
                          , std::hash<std::string>{} (_test_suite_name)
                          , std::hash<std::string>{} (_name)
                          )
            }
      , _digest
          { std::invoke
            ( [this]
              {
                return Digest{}
                  << _prefix
                  << _process_id
                  << _test_suite_name
                  << _name
                  << _type_param
                  << _value_param
                  ;
              }
            )
          }
  {}
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::testing::UniqID::Digest>::parse
      ( ParseContext& context
      )
  {
    return context.begin();
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::testing::UniqID::Digest>::format
      ( mcs::testing::UniqID::Digest const& digest
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "{:016x}{:016x}"
      , digest._laneA
      , digest._laneB
      );
  }
}
