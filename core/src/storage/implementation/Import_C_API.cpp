// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstring>
#include <fmt/format.h>
#include <functional>
#include <iterator>
#include <mcs/core/storage/c_api.h>
#include <mcs/core/storage/implementation/Import_C_API.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace mcs::util
{
  auto Cast<::mcs_core_storage_memory_size, core::memory::Size>::operator()
    ( core::memory::Size size
    ) const -> ::mcs_core_storage_memory_size
  {
    return ::mcs_core_storage_memory_size
      { size_cast<::MCS_CORE_STORAGE_SIZE> (size)
      };
  }

  auto Cast<::mcs_core_storage_memory_offset, core::memory::Offset>::operator()
    ( core::memory::Offset offset
    ) const -> ::mcs_core_storage_memory_offset
  {
    return ::mcs_core_storage_memory_offset
      { offset_cast<::MCS_CORE_STORAGE_OFFSET> (offset)
      };
  }

  auto Cast<::mcs_core_storage_memory_range, core::memory::Range>::operator()
    ( core::memory::Range range
    ) const -> ::mcs_core_storage_memory_range
  {
    return ::mcs_core_storage_memory_range
      { util::cast<::mcs_core_storage_memory_offset> (begin (range))
      , util::cast<::mcs_core_storage_memory_size> (size (range))
      };
  }
}

namespace mcs::core::storage::implementation::detail
{
  auto vector_push_back
    ( void* sink
    , ::MCS_CORE_STORAGE_BYTE byte
    ) -> void
  {
    static_cast<std::vector<::MCS_CORE_STORAGE_BYTE>*> (sink)
      ->push_back (byte)
      ;
  }

  auto vector_append
    ( void* sink
    , ::MCS_CORE_STORAGE_BYTE const* data
    , ::MCS_CORE_STORAGE_SIZE size
    ) -> void
  {
    auto vector
      { static_cast<std::vector<::MCS_CORE_STORAGE_BYTE>*> (sink)
      };

    vector->insert (std::end (*vector), data, data + size);
  }

  auto vector_reserve
    ( void* sink
    , ::MCS_CORE_STORAGE_SIZE size
    ) -> void
  {
    static_cast<std::vector<::MCS_CORE_STORAGE_BYTE>*> (sink)
      ->reserve (size)
      ;
  }

  auto channel
    ( util::not_null<std::vector<::MCS_CORE_STORAGE_BYTE>> v
    ) -> ::mcs_core_storage_channel
  {
    return ::mcs_core_storage_channel
      { .sink = v.get()
      , .push_back = std::addressof (vector_push_back)
      , .append = std::addressof (vector_append)
      , .reserve = std::addressof (vector_reserve)
      };
  }
}

namespace mcs::core::storage::implementation
{
  namespace
  {
    [[nodiscard]] auto make_parameter (Import_C_API::StorageParameter const& p)
    {
      return ::mcs_core_storage_parameter
        { util::cast<::MCS_CORE_STORAGE_BYTE const*> (p.data())
        , p.size()
        };
    }
  }
}

namespace mcs::core::storage::implementation
{
  auto Import_C_API::InstanceDeleter::operator()
    ( ::mcs_core_storage_instance* state
    ) const noexcept -> void
  {
    util::execute_and_die_on_exception
      ( "storage::Import_C_API:InstanceDeleter"
      , [&]
        {
          auto const default_delete
            { nonstd::make_scope_exit_that_dies_on_exception
              ( "Import_C_API::InstanceDeleter"
              , [&]
                {
                  std::default_delete<::mcs_core_storage_instance>{}
                    ( state
                    );
                }
              )
            };

          Import_C_API::invoke_and_throw_on_error (_storage->destruct, *state);
        }
      );
  }
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Import_C_API (Parameter::Create create)
  try
    : _storage {create._storage}
    , _instance
      { new ::mcs_core_storage_instance
        { invoke_and_throw_on_error
          ( _storage.construct
          , make_parameter (create._parameter_create)
          )
        }
      , InstanceDeleter {std::addressof (_storage)}
      }
  {}
  catch (...)
  {
    std::throw_with_nested (Error::Create {create});
  }
}

namespace mcs::core::storage::implementation
{
  namespace
  {
    constexpr auto make_max_size
      ( ::MCS_CORE_STORAGE_SIZE size
      ) -> MaxSize
    {
      if (size == ::MCS_CORE_STORAGE_SIZE {0})
      {
        return storage::MaxSize::Unlimited{};
      }

      return storage::MaxSize::Limit {memory::make_size (size)};
    }
  }

  auto Import_C_API::size_max
    ( Parameter::Size::Max parameter_size_max
    ) const -> MaxSize
  try
  {
    return make_max_size
      ( invoke_and_throw_on_error
        ( _storage.size_max
        , *_instance
        , make_parameter (parameter_size_max._parameter_size_max)
        ).value
      );
  }
  catch (...)
  {
    std::throw_with_nested (Error::Size::Max {parameter_size_max});
  }
}

namespace mcs::core::storage::implementation
{
  auto Import_C_API::size_used
    ( Parameter::Size::Used parameter_size_used
    ) const -> memory::Size
  try
  {
    return memory::make_size
      ( invoke_and_throw_on_error
        ( _storage.size_used
        , *_instance
        , make_parameter (parameter_size_used._parameter_size_used)
        ).value
      );
  }
  catch (...)
  {
    std::throw_with_nested (Error::Size::Used {parameter_size_used});
  }
}

namespace mcs::core::storage::implementation
{
  auto Import_C_API::segment_create
    ( Parameter::Segment::Create parameter_segment_create
    , memory::Size size
    ) -> segment::ID
  {
    auto bad_alloc {std::vector<::MCS_CORE_STORAGE_BYTE>{}};

    auto const segment_id
      { std::invoke
        ( [&]
          {
            try
            {
              return invoke_and_throw_on_error
                ( _storage.segment_create
                , *_instance
                , make_parameter (parameter_segment_create._parameter_segment_create)
                , util::cast<::mcs_core_storage_memory_size> (size)
                , detail::channel (std::addressof (bad_alloc))
                );
            }
            catch (...)
            {
              std::throw_with_nested
                ( Error::Segment::Create
                    { parameter_segment_create
                    , size
                    }
                );
            }
          }
        )
      };

    if (!bad_alloc.empty())
    {
      struct BadAllocData
      {
        ::MCS_CORE_STORAGE_SIZE requested;
        ::MCS_CORE_STORAGE_SIZE used;
        ::MCS_CORE_STORAGE_SIZE max;
      };
      static_assert (std::is_trivially_copyable_v<BadAllocData>);

      if (bad_alloc.size() != sizeof (BadAllocData))
      {
        throw std::logic_error
          { fmt::format
              ( "Wrong number of bytes in bad_alloc_channel:"
                " Expected {}, got {}"
              , sizeof (BadAllocData)
              , bad_alloc.size()
              )
          };
      }

      auto bad_alloc_data {BadAllocData{}};

      std::memcpy ( util::cast<std::byte*> (std::addressof (bad_alloc_data))
                  , util::cast<std::byte const*> (bad_alloc.data())
                  , bad_alloc.size()
                  );

      throw Error::BadAlloc
        { memory::make_size (bad_alloc_data.requested)
        , memory::make_size (bad_alloc_data.used)
        , make_max_size (bad_alloc_data.max)
        };
    }

    return util::cast<segment::ID> (segment_id);
  }
}

namespace mcs::core::storage::implementation
{
  auto Import_C_API::segment_remove
    ( Parameter::Segment::Remove parameter_segment_remove
    , segment::ID segment_id
    ) -> memory::Size
  try
  {
    return memory::make_size
      ( invoke_and_throw_on_error
        ( _storage.segment_remove
        , *_instance
        , make_parameter (parameter_segment_remove._parameter_segment_remove)
        , util::cast<::mcs_core_storage_segment_id> (segment_id)
        ).value
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error::Segment::Remove
          { parameter_segment_remove
          , segment_id
          }
      );
  }
}

namespace mcs::core::storage::implementation
{
  auto Import_C_API::file_read
    ( Parameter::File::Read parameter_file_read
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    ) const -> memory::Size
  try
  {
    if (!_storage.file_read)
    {
      // \todo implement fallback
      throw std::logic_error {"NYI: file_read fallback"};
    }

    return memory::make_size
      ( invoke_and_throw_on_error
        ( _storage.file_read
        , *_instance
        , make_parameter (parameter_file_read._parameter_file_read)
        , util::cast<::mcs_core_storage_segment_id> (segment_id)
        , util::cast<::mcs_core_storage_memory_offset> (offset)
        , ::mcs_core_storage_parameter
          { util::cast<::MCS_CORE_STORAGE_BYTE const*> (path.c_str())
          , path.string().size()
          }
        , util::cast<::mcs_core_storage_memory_range> (range)
        ).value
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error::File::Read
          { parameter_file_read
          , segment_id
          , offset
          , path
          , range
          }
      );
  }

  auto Import_C_API::file_write
    ( Parameter::File::Write parameter_file_write
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    ) const -> memory::Size
  try
  {
    if (!_storage.file_read)
    {
      // \todo implement fallback
      throw std::logic_error {"NYI: file_write fallback"};
    }

    return memory::make_size
      ( invoke_and_throw_on_error
        ( _storage.file_write
        , *_instance
        , make_parameter (parameter_file_write._parameter_file_write)
        , util::cast<::mcs_core_storage_segment_id> (segment_id)
        , util::cast<::mcs_core_storage_memory_offset> (offset)
        , ::mcs_core_storage_parameter
          { util::cast<::MCS_CORE_STORAGE_BYTE const*> (path.c_str())
          , path.string().size()
          }
        , util::cast<::mcs_core_storage_memory_range> (range)
        ).value
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error::File::Write
          { parameter_file_write
          , segment_id
          , offset
          , path
          , range
          }
      );
  }
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::Implementation::Implementation (Bytes error)
    : mcs::Error
      { fmt::format
          ( "storage::Import_C_API::Implementation: {}"
          , std::string { util::cast<char const*> (error.data())
                        , error.size()
                        }
          )
      }
    , _error {error}
  {}
  Import_C_API::Error::Implementation::~Implementation() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::MethodNotProvided::MethodNotProvided()
    : mcs::Error {"storage::Import_C_API::MethodNotProvided"}
  {}
  Import_C_API::Error::MethodNotProvided::~MethodNotProvided() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::Create::Create
    ( Parameter::Create parameter
    )
      : mcs::Error
        { fmt::format ( "storage::Import_C_API::Create ({})"
                      , parameter._parameter_create
                      )
        }
      , _parameter {parameter}
  {}
  Import_C_API::Error::Create::~Create() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::Size::Max::Max
    ( Parameter::Size::Max parameter
    )
      : mcs::Error
        { fmt::format ( "storage::Import_C_API::Error::Size::Max ({})"
                      , parameter
                      )
        }
      , _parameter {parameter}
  {}
  Import_C_API::Error::Size::Max::~Max() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::Size::Used::Used
    ( Parameter::Size::Used parameter
    )
      : mcs::Error
        { fmt::format ( "storage::Import_C_API::Error::Size::Used ({})"
                      , parameter
                      )
        }
      , _parameter {parameter}
  {}
  Import_C_API::Error::Size::Used::~Used() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::Segment::Create::Create
    ( Parameter::Segment::Create parameter
    , memory::Size size
    )
      : mcs::Error
        { fmt::format ( "storage::Import_C_API::Error::Segment::Create ({}, {})"
                      , parameter
                      , size
                      )
        }
      , _parameter {parameter}
      , _size {size}
  {}
  Import_C_API::Error::Segment::Create::~Create() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::Segment::Remove::Remove
    ( Parameter::Segment::Remove parameter
    , segment::ID segment_id
    )
      : mcs::Error
        { fmt::format ( "storage::Import_C_API::Error::Segment::Remove ({}, {})"
                      , parameter
                      , segment_id
                      )
        }
      , _parameter {parameter}
      , _segment_id {segment_id}
  {}
  Import_C_API::Error::Segment::Remove::~Remove() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::File::Read::Read
    ( Parameter::File::Read parameter
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    )
      : mcs::Error
        { fmt::format
            ( "storage::Import_C_API::Error::File::Read ({}, {}, {}, {}, {})"
            , parameter
            , segment_id
            , offset
            , path
            , range
            )
        }
      , _parameter {parameter}
      , _segment_id {segment_id}
      , _offset {offset}
      , _path {path}
      , _range {range}
  {}
  Import_C_API::Error::File::Read::~Read() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::File::Write::Write
    ( Parameter::File::Write parameter
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    )
      : mcs::Error
        { fmt::format
            ( "storage::Import_C_API::Error::File::Write ({}, {}, {}, {}, {})"
            , parameter
            , segment_id
            , offset
            , path
            , range
            )
        }
      , _parameter {parameter}
      , _segment_id {segment_id}
      , _offset {offset}
      , _path {path}
      , _range {range}
  {}
  Import_C_API::Error::File::Write::~Write() = default;
}

namespace mcs::core::storage::implementation
{
  Import_C_API::Error::BadAlloc::BadAlloc
    ( memory::Size requested
    , memory::Size used
    , MaxSize max
    ) noexcept
      : mcs::Error
        { fmt::format
          ( "storage::implementation::Import_C_API::BadAlloc: requested {}, used {}, max {}"
          , requested
          , used
          , max
          )
        }
      , _requested {requested}
      , _used {used}
      , _max {max}
  {}
  Import_C_API::Error::BadAlloc::~BadAlloc() = default;
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max
  , _parameter_size_max
  );
MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used
  , _parameter_size_used
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create
  , _parameter_segment_create
  );
MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove
  , _parameter_segment_remove
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description
  , _parameter_chunk_description
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Import_C_API::Parameter::File::Read
  , _parameter_file_read
  );
MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Import_C_API::Parameter::File::Write
  , _parameter_file_write
  );
