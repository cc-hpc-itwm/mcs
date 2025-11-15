// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/serialization/declare.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/serialization/error/AdditionalBytes.hpp>
#include <mcs/serialization/error/Load.hpp>
#include <mcs/serialization/error/Save.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <stdexcept>
#include <tuple>
#include <typeinfo>

namespace mcs::serialization
{
  namespace
  {
    struct ThrowsWhenSaved
    {
      constexpr explicit ThrowsWhenSaved (int i) noexcept
        : _i {i}
      {}

      ThrowsWhenSaved (IArchive&);
      [[noreturn]] auto save (OArchive&) const -> OArchive&;

    private:
      int _i {0};
    };

    ThrowsWhenSaved::ThrowsWhenSaved (IArchive&)
    {}
    [[noreturn]] auto ThrowsWhenSaved::save (OArchive&) const -> OArchive&
    {
      throw std::runtime_error (fmt::format ("{}", _i));
    }
  }

  TEST (Serialization, errors_when_saved_are_wrapped_and_thrown)
  {
    auto const value {testing::random::value<int>{}()};

    testing::require_exception
      ( [&]
        {
          std::ignore = serialization::OArchive {ThrowsWhenSaved {value}};
        }
      , testing::assert_type_and_what<error::Save>
          ("serialization::error::Save")
      , testing::assert_type_and_what<std::runtime_error>
          (fmt::format ("{}", value))
      );
  }

  namespace
  {
    struct ThrowsWhenLoaded
    {
      constexpr explicit ThrowsWhenLoaded (int i) noexcept
        : _i {i}
      {}

      [[noreturn]] ThrowsWhenLoaded (IArchive&);
      auto save (OArchive&) const -> OArchive&;

    private:
      int _i {0};
    };

    [[noreturn]] ThrowsWhenLoaded::ThrowsWhenLoaded (IArchive& ia)
      : _i {serialization::load<decltype (_i)> (ia)}
    {
      throw std::runtime_error (fmt::format ("{}", _i));
    }
    auto ThrowsWhenLoaded::save (OArchive& oa) const -> OArchive&
    {
     return serialization::save (oa, _i);
    }
  }

  TEST (Serialization, errors_when_loaded_are_wrapped_and_thrown)
  {
    auto const value {testing::random::value<int>{}()};
    auto const x {ThrowsWhenLoaded {value}};

    testing::require_exception
      ( [&]
        {
          auto const bytes {serialization::OArchive {x}.bytes()};
          auto ia {IArchive {bytes}};

          std::ignore = serialization::load<ThrowsWhenLoaded> (ia);
        }
      , testing::assert_type_and_what<error::Load>
          ("serialization::error::Load")
      , testing::assert_type_and_what<std::runtime_error>
          (fmt::format ("{}", value))
      );
  }

  TEST (Serialization, throws_when_load_from_finds_additional_bytes)
  {
    using testing::random::value;
    auto const x {value<int>{}()};
    auto const additional_bytes {value<std::size_t> {1ul, 1ul << 20ul}()};

    testing::require_exception
      ( [&]
        {
          auto bytes {serialization::OArchive {x}.bytes()};
          bytes.resize (bytes.size() + additional_bytes);
          std::ignore = serialization::load_from<int> (bytes);
        }
      , testing::assert_type_and_what<error::AdditionalBytes>
          ( fmt::format ("serialization::error::AdditionalBytes:"
                         " {} bytes left in archive after load has returned."
                        , additional_bytes
                        )
          )
      );
  }
}
