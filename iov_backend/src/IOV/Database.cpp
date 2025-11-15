// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fstream>
#include <iterator>
#include <mcs/IOV_Database.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/map.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (iov::meta::Key);
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (iov::meta::Value);
}

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    ( oa
    , key
    , iov::meta::Key
    )
  {
    save (oa, key.str());

    return oa;
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    ( ia
    , iov::meta::Key
    )
  {
    auto str {load<std::string> (ia)};

    return iov::meta::Key {str};
  }
}

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    ( oa
    , value
    , iov::meta::Value
    )
  {
    save (oa, value.str());

    return oa;
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    ( ia
    , iov::meta::Value
    )
  {
    auto str {load<std::string> (ia)};

    return iov::meta::Value {str};
  }
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (iov::meta::Key);
  template<> MCS_UTIL_FMT_DECLARE (iov::meta::Value);
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, iov::meta::Key)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (key, context, iov::meta::Key)
  {
    return fmt::format_to (context.out(), "iov::meta::Key '{}'", key.str());
  }
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, iov::meta::Value)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (value, context, iov::meta::Value)
  {
    return fmt::format_to (context.out(), "iov::meta::Value '{}'", value.str());
  }
}

namespace mcs
{
  IOV_Database::IOV_Database (std::filesystem::path path)
    : _entries {serialization::load_from<decltype (_entries)> (path)}
  {}
  auto IOV_Database::save (std::filesystem::path path) const -> void
  {
    std::ofstream {path} << serialization::OArchive {_entries};
  }

  auto IOV_Database::assign (WriteAccess const&, Key key, Value value) -> void
  {
    // \note OVERWRITES existing values
    _entries.insert ({key, value});
  }

  auto IOV_Database::remove (WriteAccess const&, Key key) -> Value
  {
    auto entry {_entries.find (key)};

    if (entry == std::end (_entries))
    {
      throw std::invalid_argument
        { fmt::format ("Key '{}' not present in database.", key)
        };
    }

    auto const remove_entry
      { nonstd::make_scope_exit_that_dies_on_exception
        ( "IOV_Database::remove"
        , [&]
          {
            _entries.erase (entry);
          }
        )
      };

    return entry->second;
  }

  auto IOV_Database::contains
    ( ReadAccess const&
    , Key const& key
    ) const noexcept -> bool
  {
    return _entries.contains (key);
  }
  auto IOV_Database::empty (ReadAccess const&) const noexcept -> bool
  {
    return _entries.empty();
  }
  auto IOV_Database::size (ReadAccess const&) const noexcept -> std::size_t
  {
    return _entries.size();
  }
  auto IOV_Database::at (ReadAccess const&, Key const& key) const -> Value
  {
    return _entries.at (key);
  }

  // \todo would lower_bound_that_starts_with (access, key, prefix) be
  // faster?
  auto IOV_Database::lower_bound
    ( ReadAccess const&
    , Key const& key
    ) const -> std::optional<Entries::value_type>
  {
    if ( auto const first_of_the_not_smaller {_entries.lower_bound (key)}
       ; first_of_the_not_smaller != std::cend (_entries)
       )
    {
      return *first_of_the_not_smaller;
    }

    return {};
  }
  auto IOV_Database::upper_bound
    ( ReadAccess const&
    , Key const& key
    ) const -> std::optional<Entries::value_type>
  {
    if ( auto const first_of_the_greater {_entries.upper_bound (key)}
       ; first_of_the_greater != std::cend (_entries)
       )
    {
      return *first_of_the_greater;
    }

    return {};
  }

  auto IOV_Database::iov_key_value_map
    ( ReadAccess const&
    ) const -> iov::meta::KeyValueMap
  {
    return {std::cbegin (_entries), std::cend (_entries)};
  }

  auto IOV_Database::read_access() const -> ReadAccess
  {
    while (_number_of_writers.load() != 0)
    {
      std::this_thread::yield();
    }

    return ReadAccess {std::shared_lock {_guard}};
  }

  auto IOV_Database::WriteAccessImpl::Deleter::operator()
    ( WriteAccessImpl* write_access
    ) const noexcept -> void
  {
    --*_number_of_writers;

    std::default_delete<WriteAccessImpl>{} (write_access);
  }

  auto IOV_Database::write_access() const -> WriteAccess
  {
    ++_number_of_writers;

    return WriteAccess
      { new WriteAccessImpl {std::unique_lock {_guard}}
      , WriteAccessImpl::Deleter {std::addressof (_number_of_writers)}
      };
  }
}

namespace mcs
{
  IOV_DatabaseDeleter::IOV_DatabaseDeleter (std::filesystem::path path)
    : _path {path}
  {}

  auto IOV_DatabaseDeleter::operator()
    ( IOV_Database* database
    ) const noexcept -> void
  {
    if (0 == std::uncaught_exceptions())
    {
      util::execute_and_die_on_exception
        ( fmt::format ("Error during update of database '{}'", _path)
        , [&]
          {
            database->save (_path);
          }
        );
    }

    std::default_delete<IOV_Database>{} (database);
  }

  auto make_scoped_iov_database
    ( std::filesystem::path path
    ) -> IOV_ScopedDatabase
  {
    return IOV_ScopedDatabase
      { new IOV_Database {path}
      , IOV_DatabaseDeleter {path}
      };
  }
}

namespace mcs
{
  IOV_DBRef::IOV_DBRef (util::not_null<IOV_Database> db)
    : _db {db}
  {}

  auto IOV_DBRef::get
    ( iov::meta::Key const& key
    ) const noexcept -> Expected<iov::meta::Value>
  try
  {
    return _db->at (_db->read_access(), key);
  }
  catch (...)
  {
    return iov::unexpected
      { iov::ErrorReason
        { fmt::format ("db::get: {}", std::current_exception())
        }
      };
  }

  auto IOV_DBRef::put
    ( iov::meta::Key const& key
    , iov::meta::Value const& value
    ) -> Expected<void>
  try
  {
    _db->assign (_db->write_access(), key, value);

    return {};
  }
  catch (...)
  {
    return iov::unexpected
      { iov::ErrorReason
        { fmt::format ("db::put: {}", std::current_exception())
        }
      };
  }

  auto IOV_DBRef::remove
    ( iov::meta::Key const& key
    ) -> Expected<void>
  try
  {
    _db->remove (_db->write_access(), key);

    return {};
  }
  catch (...)
  {
    return iov::unexpected
      { iov::ErrorReason
        { fmt::format ("db::erase: {}", std::current_exception())
        }
      };
  }

  auto IOV_DBRef::contains
    ( iov::meta::Key const& key
    ) -> Expected<bool>
  try
  {
    return _db->contains (_db->read_access(), key);
  }
  catch (...)
  {
    return iov::unexpected
      { iov::ErrorReason
        { fmt::format ("db::contains {}", std::current_exception())
        }
      };
  }

  auto IOV_DBRef::put_many
    ( iov::meta::KeyValueMap const& kvm
    ) -> Expected<void>
  try
  {
    std::ranges::for_each
      ( kvm
      , [&] (auto const& kv)
        {
          auto const& [key, value] {kv};

          if (auto put_result {put (key, value)}; !put_result)
          {
            throw std::runtime_error {put_result.error().reason()};
          }
        }
      );

    return {};
  }
  catch (...)
  {
    return iov::unexpected
      { iov::ErrorReason
        { fmt::format ("db::put_many {}", std::current_exception())
        }
      };
  }

  auto IOV_DBRef::get_all
    (
    ) const -> Expected<iov::meta::KeyValueMap>
  {
    return _db->iov_key_value_map (_db->read_access());
  }

  auto IOV_DBRef::iterator
    (
    ) const -> Expected<UniqueIterator>
  {
    return UniqueIterator {new IOV_DBRefIterator {_db}};
  }

  auto IOV_DBRef::iterator
    ( std::string const& prefix
    ) const -> Expected<UniqueIterator>
  {
    return UniqueIterator {new IOV_DBRefFilteredIterator {_db, prefix}};
  }
}

namespace mcs
{
  IOV_DBRefIteratorBase::Error::PreconditionValidViolated
    ::PreconditionValidViolated()
      : mcs::Error
        { "IOV_DBRefIteratorBase::next: precondition 'valid()' violated"
        }
  {}
  IOV_DBRefIteratorBase::Error::PreconditionValidViolated
    ::~PreconditionValidViolated() = default
    ;

  IOV_DBRefIteratorBase::IOV_DBRefIteratorBase
    ( util::not_null<IOV_Database> db
    , std::optional<Key_Value> kv
    )
    : _db {db}
    , _key_value {kv}
  {}

  auto IOV_DBRefIteratorBase::valid() -> bool
  {
    return _key_value.has_value();
  }
  auto IOV_DBRefIteratorBase::key() -> Expected<iov::meta::Key>
  {
    if (valid())
    {
      // optional is checked by valid()
      // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
      return _key_value->first;
    }

    return iov::unexpected<iov::ErrorReason>
      { "IOV_DBRefIteratorBase is invalid"
      };
  }
  auto IOV_DBRefIteratorBase::value() -> Expected<iov::meta::Value>
  {
    if (valid())
    {
      // optional is checked by valid()
      // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
      return _key_value->second;
    }

    return iov::unexpected<iov::ErrorReason>
      { "IOV_DBRefIteratorBase is invalid"
      };
  }
  auto IOV_DBRefIteratorBase::error() -> std::optional<iov::ErrorReason>
  {
    // \todo what is the semantic of error()/valid()?
    //    if (!valid())
    //    {
    //      return iov::ErrorReason {"IOV_DBRefIteratorBase is invalid"};
    //    }

    return {};
  }
}

namespace mcs
{
  IOV_DBRefIterator::IOV_DBRefIterator
    ( util::not_null<IOV_Database> db
    )
    : IOV_DBRefIteratorBase {db, find_first_valid_key_value (db)}
  {}

  auto IOV_DBRefIterator::find_first_valid_key_value
      ( util::not_null<IOV_Database> db
      ) const -> std::optional<Key_Value>
  {
    // lower_bound returns the first element that is not less than the
    // key. There is no key that is less than Key{}, therefore in this
    // case the first element in the database is returned. Analogous
    // to std::begin.
    return db->lower_bound (db->read_access(), iov::meta::Key {{}});
  }

  auto IOV_DBRefIterator::next() -> void
  {
    auto const key {this->key()};

    if (!key)
    {
      throw Error::PreconditionValidViolated{};
    }

    _key_value.reset();

    if (auto first_of_the_greater {_db->upper_bound (_db->read_access(), *key)})
    {
      _key_value.emplace (*first_of_the_greater);
    }
  }
}

namespace mcs
{
  IOV_DBRefFilteredIterator::IOV_DBRefFilteredIterator
    ( util::not_null<IOV_Database> db
    , std::string prefix
    )
    : IOV_DBRefIteratorBase {db, find_first_valid_key_value (db, prefix)}
    , _prefix {prefix}
  {}

  auto IOV_DBRefFilteredIterator::find_first_valid_key_value
      ( util::not_null<IOV_Database> db
      , std::string const& prefix
      ) const -> std::optional<Key_Value>
  {
    // lower_bound returns the first element that is not less than prefix.
    // Since std::string::operator<=> compares lexicographically the key of
    // the found element is either:
    // * the prefix itself (i.e. equal) or the
    // * first element starting with prefix (greater than the prefix), or the
    // * first element that is greater than the prefix.
    // In that order.
    // If no such element exists then it is std::map::end()
    //
    // \todo Is a character by character approach faster? e.g. shrink
    // the relevant range using the first character of the prefix,
    // then shrink the relevant range using the second character of
    // the prefix, and so on until either the relevant range becomes
    // empty or all characters of the prefix have been used
    //
    if ( auto first_of_the_not_smaller
          { db->lower_bound (db->read_access(), IOV_Database::Key {prefix})
          }
       )
    {
      if (first_of_the_not_smaller->first.str().starts_with (prefix))
      {
        return first_of_the_not_smaller;
      }
    }

    return {};
  }

  auto IOV_DBRefFilteredIterator::next() -> void
  {
    auto const key {this->key()};

    if (!key)
    {
      throw Error::PreconditionValidViolated{};
    }

    _key_value.reset();

    // upper_bound returns the first element (if any) where the key starts
    // with `_prefix` and it is greater than the key at the current position.
    //
    if ( auto first_of_the_greater
          { _db->upper_bound (_db->read_access(), *key)
          }
       )
    {
      if (first_of_the_greater->first.str().starts_with (_prefix))
      {
        _key_value.emplace (*first_of_the_greater);
      }
    }
  }
}
