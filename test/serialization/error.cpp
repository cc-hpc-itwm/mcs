// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <functional>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/set.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/serialization/error/AdditionalBytes.hpp>
#include <mcs/serialization/error/Load.hpp>
#include <mcs/serialization/error/Save.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/vector.hpp>
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

  TEST (Serialization, throws_when_load_from_finds_not_enough_bytes)
  {
    auto const xs {testing::random::value<int>{}()};
    auto const oa {serialization::OArchive {xs}};
    auto bytes {oa.bytes()};
    ASSERT_GT (bytes.size(), 0);

    auto const wanted {bytes.size()};

    using RandomSize = testing::random::value<std::size_t>;
    bytes.resize
      ( std::invoke (RandomSize {RandomSize::Max {bytes.size() - 1}})
      );

    testing::require_exception
      ( [&]
        {
          std::ignore = serialization::load_from<int> (bytes);
        }
      , testing::assert_type_and_what<error::Load>
          ( "serialization::error::Load"
          )
      , testing::Assert<error::NotEnoughBytes>
          { [&] (auto const& caught)
            {
              ASSERT_EQ (caught.wanted(), wanted);
              ASSERT_EQ (caught.provided(), bytes.size());
              ASSERT_STREQ
                ( caught.what()
                , fmt::format ( "serialization::error::NotEnoughBytes:"
                                " {} bytes wanted but archive has only {} left."
                              , caught.wanted()
                              , caught.provided()
                              ).c_str()
                );
            }
          }
      );
  }

  TEST (Serialization, throws_when_load_the_wrong_archive_tag)
  {
    auto const xs {testing::random::value<std::vector<int>>{}()};
    auto const bytes {serialization::OArchive {xs}.bytes()};
    auto ia {IArchive {bytes}};

    testing::require_exception
      ( [&]
        {
          std::ignore = serialization::load<std::set<int>> (ia);
        }
      , testing::assert_type_and_what<error::Load>
          ( "serialization::error::Load"
          )
      , testing::assert_type_and_what<error::WrongTag>
          ( "serialization::error::WrongTag"
          )
      );
  }
}
