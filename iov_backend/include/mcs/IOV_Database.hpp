// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <iosfwd>
#include <iov/iov.hpp>
#include <map>
#include <mcs/Error.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>

namespace mcs
{
  struct IOV_Database
  {
    IOV_Database() = default;

    explicit IOV_Database (std::filesystem::path);
    auto save (std::filesystem::path) const -> void;

    using Key = iov::meta::Key;
    using Value = iov::meta::Value;
    using Entries = std::map<Key, Value>;

    struct ReadAccess
    {
      std::shared_lock<std::shared_mutex> _lock;
    };
    struct WriteAccessImpl
    {
      std::unique_lock<std::shared_mutex> _lock;

      struct Deleter
      {
        auto operator() (WriteAccessImpl*) const noexcept -> void;
        std::atomic<unsigned int>* _number_of_writers;
      };
    };
    using WriteAccess
      = std::unique_ptr<WriteAccessImpl, WriteAccessImpl::Deleter>
    ;

    [[nodiscard]] auto read_access() const -> ReadAccess;
    [[nodiscard]] auto write_access() const -> WriteAccess;

    auto assign (WriteAccess const&, Key, Value) -> void;
    auto remove (WriteAccess const&, Key) -> Value;

    [[nodiscard]] auto contains
      ( ReadAccess const&
      , Key const&
      ) const noexcept -> bool
      ;

    [[nodiscard]] auto empty (ReadAccess const&) const noexcept -> bool;
    [[nodiscard]] auto size (ReadAccess const&) const noexcept -> std::size_t;

    [[nodiscard]] auto at (ReadAccess const&, Key const&) const -> Value;

    // Returns: The first element that is not less than the given key.
    [[nodiscard]] auto lower_bound
      ( ReadAccess const&
      , Key const&
      ) const -> std::optional<Entries::value_type>
      ;

    // Returns: The first element that is greater than the given key.
    [[nodiscard]] auto upper_bound
      ( ReadAccess const&
      , Key const&
      ) const -> std::optional<Entries::value_type>
      ;

    [[nodiscard]] auto iov_key_value_map
      ( ReadAccess const&
      ) const -> iov::meta::KeyValueMap
      ;

  private:
    Entries _entries {{Key {"dbinfo:format"}, Value {"1-WIP"}}};

    mutable std::shared_mutex _guard;
    mutable std::atomic<unsigned int> _number_of_writers {0};
  };
}

namespace mcs
{
  // Updates the database if destructed normally. Does not update the
  // database destructed via stack unwinding, e.g. when there is
  // uncaught_exception().
  //
  struct IOV_DatabaseDeleter
  {
    explicit IOV_DatabaseDeleter (std::filesystem::path);
    auto operator() (IOV_Database*) const noexcept -> void;

  private:
    std::filesystem::path _path;
  };

  using IOV_ScopedDatabase = std::unique_ptr<IOV_Database, IOV_DatabaseDeleter>;

  auto make_scoped_iov_database (std::filesystem::path) -> IOV_ScopedDatabase;
}

// \todo split files
namespace mcs
{
  struct IOV_DBRef final : public iov::meta::AbstractDatabaseBackend
  {
    IOV_DBRef (util::not_null<IOV_Database>);

    template<typename Result>
      using Expected = iov::expected<Result, iov::ErrorReason>;

    [[nodiscard]] auto get
      ( iov::meta::Key const&
      ) const noexcept -> Expected<iov::meta::Value> override
      ;

    [[nodiscard]] auto put
      ( iov::meta::Key const&
      , iov::meta::Value const&
      ) -> Expected<void> override
      ;

    [[nodiscard]] auto remove
      ( iov::meta::Key const&
      ) -> Expected<void> override
      ;

    [[nodiscard]] auto contains
      ( iov::meta::Key const&
      ) -> Expected<bool> override
      ;

    [[nodiscard]] auto put_many
      ( iov::meta::KeyValueMap const&
      ) -> Expected<void> override
      ;

    [[nodiscard]] auto get_all
      (
      ) const -> Expected<iov::meta::KeyValueMap> override
      ;

    using UniqueIterator = std::unique_ptr<iov::meta::AbstractDatabaseIterator>;

    [[nodiscard]] auto iterator
      (
      ) const -> Expected<UniqueIterator> override
      ;

    [[nodiscard]] auto iterator
      ( std::string const&
      ) const -> Expected<UniqueIterator> override
      ;

  private:
    util::not_null<IOV_Database> _db;
  };

  struct IOV_DBRefIteratorBase : public iov::meta::AbstractDatabaseIterator
  {
    [[nodiscard]] auto valid() -> bool final;

    template<typename Result>
      using Expected = iov::expected<Result, iov::ErrorReason>;

    struct Error
    {
      struct PreconditionValidViolated : public mcs::Error
      {
        PreconditionValidViolated();

        MCS_ERROR_COPY_MOVE_DEFAULT (PreconditionValidViolated);
      };
    };

    // Returns the key at the current position
    // pre: valid()
    //
    [[nodiscard]] auto key() -> Expected<iov::meta::Key> final;

    // Returns the value at the current position
    // pre: valid()
    //
    [[nodiscard]] auto value() -> Expected<iov::meta::Value> final;

    // Advances the iterator to the next valid element if there is any.
    // Otherwise the iterator is invalid.
    //
    // pre: valid()
    // to advance means:
    //     if (auto const key {it.key()})
    //     {
    //       it.next();
    //       if (it.valid())
    //       {
    //         it.value().has_value();
    //         *key < *it.key();
    //       }
    //     }
    //
    auto next() -> void override = 0;

    [[nodiscard]] auto error() -> std::optional<iov::ErrorReason> final;

  protected:
    using Key_Value = IOV_Database::Entries::value_type;

    IOV_DBRefIteratorBase
      ( util::not_null<IOV_Database>
      , std::optional<Key_Value>
      );

    util::not_null<IOV_Database> _db;
    std::optional<Key_Value> _key_value;
  };

  struct IOV_DBRefIterator final : public IOV_DBRefIteratorBase
  {
    auto next() -> void final;

  protected:
    friend IOV_DBRef;
    IOV_DBRefIterator (util::not_null<IOV_Database>);

    [[nodiscard]] auto find_first_valid_key_value
      ( util::not_null<IOV_Database>
      ) const -> std::optional<Key_Value>;
  };

  struct IOV_DBRefFilteredIterator final : public IOV_DBRefIteratorBase
  {
    auto next() -> void final;

  private:
    friend IOV_DBRef;
    IOV_DBRefFilteredIterator (util::not_null<IOV_Database>, std::string);

    [[nodiscard]] auto find_first_valid_key_value
      ( util::not_null<IOV_Database>
      , std::string const&
      ) const -> std::optional<Key_Value>;

    std::string _prefix;
  };
}
