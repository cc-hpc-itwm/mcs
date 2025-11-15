// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/Implement_C_API.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/c_api/channel.h>
#include <mcs/core/storage/c_api/storage.h>
#include <mcs/core/storage/c_api/types.h>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <string>

namespace mcs::core::storage::implement_c_api
{
  auto write_into_channel
    ( util::not_null<::mcs_core_storage_channel>
    , char const*
    ) noexcept -> void
    ;

  auto write_into_channel
    ( util::not_null<::mcs_core_storage_channel>
    , std::string
    ) noexcept -> void
    ;

  auto write_into_channel
    ( util::not_null<::mcs_core_storage_channel>
    , std::exception_ptr
    ) noexcept -> void
    ;

  auto write_into_channel
    ( util::not_null<::mcs_core_storage_channel>
    , ::MCS_CORE_STORAGE_SIZE
    ) noexcept -> void
    ;
}

namespace mcs::util
{
  template<>
    struct Cast<::MCS_CORE_STORAGE_SIZE, core::storage::MaxSize>
  {
    constexpr auto operator()
      ( core::storage::MaxSize const&
      ) const -> ::MCS_CORE_STORAGE_SIZE
      ;
  };
}

namespace mcs::util
{
  template<core::storage::is_implementation Implementation>
    struct Cast<Implementation*, ::mcs_core_storage_instance>
  {
    constexpr auto operator()
      ( ::mcs_core_storage_instance
      ) const -> Implementation*
      ;
  };

  template<core::storage::is_implementation Implementation>
    struct Cast<Implementation const*, ::mcs_core_storage_instance>
  {
    constexpr auto operator()
      ( ::mcs_core_storage_instance
      ) const -> Implementation const*
      ;
  };
}

namespace mcs::util
{
  template<typename T>
    struct Cast<T, ::mcs_core_storage_parameter>
  {
    constexpr auto operator()
      ( ::mcs_core_storage_parameter const&
      ) const -> T
      ;
  };
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::construct
      ( ::mcs_core_storage_parameter parameter_create
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> ::mcs_core_storage_instance
  try
  {
    return ::mcs_core_storage_instance
      { .state = new Implementation
        { util::cast<typename Implementation::Parameter::Create>
          ( parameter_create
          )
        }
      };
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );

    return ::mcs_core_storage_instance {.state = nullptr};
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::destruct
      ( ::mcs_core_storage_instance instance
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> void
  try
  {
    delete util::cast<Implementation*> (instance.state);
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::size_max
      ( ::mcs_core_storage_instance instance
      , ::mcs_core_storage_parameter parameter_size_max
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> ::mcs_core_storage_memory_size
  try
  {
    return ::mcs_core_storage_memory_size
      { .value = util::cast<::MCS_CORE_STORAGE_SIZE>
        ( util::cast<Implementation const*> (instance)->size_max
          ( util::cast<typename Implementation::Parameter::Size::Max>
            ( parameter_size_max
            )
          )
        )
      };
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );

    return ::mcs_core_storage_memory_size
      { .value = ::MCS_CORE_STORAGE_SIZE {0}
      };
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::size_used
      ( ::mcs_core_storage_instance instance
      , ::mcs_core_storage_parameter parameter_size_used
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> ::mcs_core_storage_memory_size
  try
  {
    return ::mcs_core_storage_memory_size
      { .value = size_cast<::MCS_CORE_STORAGE_SIZE>
        ( util::cast<Implementation const*> (instance)->size_used
          ( util::cast<typename Implementation::Parameter::Size::Used>
            ( parameter_size_used
            )
          )
        )
      };
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );

    return ::mcs_core_storage_memory_size
      { .value = ::MCS_CORE_STORAGE_SIZE {0}
      };
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::segment_create
      ( ::mcs_core_storage_instance instance
      , ::mcs_core_storage_parameter parameter_segment_create
      , ::mcs_core_storage_memory_size size
      , ::mcs_core_storage_channel bad_alloc_channel
      , ::mcs_core_storage_channel error_channel
      ) -> ::mcs_core_storage_segment_id
  try
  {
    return util::cast<::mcs_core_storage_segment_id>
      ( util::cast<Implementation*> (instance)->segment_create
        ( util::cast<typename Implementation::Parameter::Segment::Create>
          ( parameter_segment_create
          )
        , memory::make_size (size.value)
        )
      );
  }
  catch (typename Implementation::Error::BadAlloc const& bad_alloc)
  {
    implement_c_api::write_into_channel
      ( std::addressof (bad_alloc_channel)
      , size_cast<::MCS_CORE_STORAGE_SIZE> (bad_alloc.requested())
      );
    implement_c_api::write_into_channel
      ( std::addressof (bad_alloc_channel)
      , size_cast<::MCS_CORE_STORAGE_SIZE> (bad_alloc.used())
      );
    implement_c_api::write_into_channel
      ( std::addressof (bad_alloc_channel)
      , util::cast<::MCS_CORE_STORAGE_SIZE> (bad_alloc.max())
      );

    return ::mcs_core_storage_segment_id
      { .value = ::MCS_CORE_STORAGE_SEGMENT_ID {0}
      };
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );

    return ::mcs_core_storage_segment_id
      { .value = ::MCS_CORE_STORAGE_SEGMENT_ID {0}
      };
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::segment_remove
      ( ::mcs_core_storage_instance instance
      , ::mcs_core_storage_parameter parameter_segment_remove
      , ::mcs_core_storage_segment_id segment_id
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> ::mcs_core_storage_memory_size
  try
  {
    return ::mcs_core_storage_memory_size
      { .value = size_cast<::MCS_CORE_STORAGE_SIZE>
        ( util::cast<Implementation*> (instance)->segment_remove
          ( util::cast<typename Implementation::Parameter::Segment::Remove>
            ( parameter_segment_remove
            )
          , util::cast<segment::ID> (segment_id)
          )
        )
      };
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );

    return ::mcs_core_storage_memory_size
      { .value = ::MCS_CORE_STORAGE_SIZE {0}
      };
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation, chunk::is_access Access>
    auto Implement_C_API::chunk_description
      ( ::mcs_core_storage_instance instance
      , ::mcs_core_storage_parameter parameter_chunk_description
      , ::mcs_core_storage_segment_id segment_id
      , ::mcs_core_storage_memory_range range
      , ::mcs_core_storage_channel description_channel
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> void
  try
  {
    auto const chunk_description
      { util::cast<Implementation const*> (instance)
        ->template chunk_description<Access>
          ( util::cast<typename Implementation::Parameter::Chunk::Description>
            ( parameter_chunk_description
            )
          , util::cast<segment::ID> (segment_id)
          , memory::make_range
            ( memory::make_offset (range.begin.value)
            , memory::make_size (range.size.value)
            )
          )
      };

    auto const bytes {serialization::OArchive (chunk_description).bytes()};

    description_channel.append
      ( description_channel.sink
      , util::cast<::MCS_CORE_STORAGE_BYTE const*> (bytes.data())
      , bytes.size()
      );
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation, chunk::is_access Access>
    auto Implement_C_API::chunk_state
      ( ::mcs_core_storage_instance
      , ::mcs_core_storage_parameter chunk_description
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> typename c_api::Traits<Access>::ChunkState
  try
  {
    using Description = Implementation::Chunk::template Description<Access>;

    auto state
      { std::make_unique<typename Description::State>
        ( util::cast<Description> (chunk_description)
        )
      };
    auto const bytes {state->bytes()};

    return typename c_api::Traits<Access>::ChunkState
      { .state = state.release()
      , .data = util::cast<typename c_api::Traits<Access>::Byte*> (bytes.data())
      , .size = bytes.size()
      };
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );

    return typename c_api::Traits<Access>::ChunkState
      { .state = nullptr
      , .data = nullptr
      , .size = ::MCS_CORE_STORAGE_SIZE {0}
      };
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation, chunk::is_access Access>
    auto Implement_C_API::chunk_state_destruct
      ( ::mcs_core_storage_instance
      , typename c_api::Traits<Access>::ChunkState chunk_state
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> void
  try
  {
    using Description = Implementation::Chunk::template Description<Access>;

    delete util::cast<typename Description::State*> (chunk_state.state);
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::file_read
      ( ::mcs_core_storage_instance instance
      , ::mcs_core_storage_parameter parameter_file_read
      , ::mcs_core_storage_segment_id segment_id
      , ::mcs_core_storage_memory_offset offset
      , ::mcs_core_storage_parameter file_name
      , ::mcs_core_storage_memory_range range
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> ::mcs_core_storage_memory_size
  try
  {
    return ::mcs_core_storage_memory_size
      { .value = size_cast<::MCS_CORE_STORAGE_SIZE>
        ( util::cast<Implementation const*> (instance)->file_read
          ( util::cast<typename Implementation::Parameter::File::Read>
            ( parameter_file_read
            )
          , util::cast<segment::ID> (segment_id)
          , memory::make_offset (offset.value)
          , std::filesystem::path
            { util::cast<char const*> (file_name.data)
            , util::cast<char const*> (file_name.data) + file_name.size
            }
          , memory::make_range
            ( memory::make_offset (range.begin.value)
            , memory::make_size (range.size.value)
            )
          )
        )
      };
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );

    return ::mcs_core_storage_memory_size
      { .value = ::MCS_CORE_STORAGE_SIZE {0}
      };
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::file_write
      ( ::mcs_core_storage_instance instance
      , ::mcs_core_storage_parameter parameter_file_write
      , ::mcs_core_storage_segment_id segment_id
      , ::mcs_core_storage_memory_offset offset
      , ::mcs_core_storage_parameter file_name
      , ::mcs_core_storage_memory_range range
      , ::mcs_core_storage_channel error_channel
      ) noexcept -> ::mcs_core_storage_memory_size
  try
  {
    return ::mcs_core_storage_memory_size
      { .value = size_cast<::MCS_CORE_STORAGE_SIZE>
        ( util::cast<Implementation const*> (instance)->file_write
          ( util::cast<typename Implementation::Parameter::File::Write>
            ( parameter_file_write
            )
          , util::cast<segment::ID> (segment_id)
          , memory::make_offset (offset.value)
          , std::filesystem::path
            { util::cast<char const*> (file_name.data)
            , util::cast<char const*> (file_name.data) + file_name.size
            }
          , memory::make_range
            ( memory::make_offset (range.begin.value)
            , memory::make_size (range.size.value)
            )
          )
        )
      };
  }
  catch (...)
  {
    implement_c_api::write_into_channel
      ( std::addressof (error_channel)
      , std::current_exception()
      );

    return ::mcs_core_storage_memory_size
      { .value = ::MCS_CORE_STORAGE_SIZE {0}
      };
  }
}

namespace mcs::core::storage
{
  template<is_implementation Implementation>
    auto Implement_C_API::methods() noexcept -> ::mcs_core_storage
  {
    return ::mcs_core_storage
      { .construct = std::addressof (construct<Implementation>)
      , .destruct = std::addressof (destruct<Implementation>)
      , .size_max = std::addressof (size_max<Implementation>)
      , .size_used = std::addressof (size_used<Implementation>)
      , .segment_create = std::addressof (segment_create<Implementation>)
      , .segment_remove = std::addressof (segment_remove<Implementation>)
      , .chunk_const_description = std::addressof
          ( chunk_description<Implementation, chunk::access::Const>
          )
      , .chunk_mutable_description = std::addressof
          ( chunk_description<Implementation, chunk::access::Mutable>
          )
      , .chunk_const_state = std::addressof
          ( chunk_state<Implementation, chunk::access::Const>
          )
      , .chunk_mutable_state = std::addressof
          ( chunk_state<Implementation, chunk::access::Mutable>
          )
      , .chunk_const_state_destruct = std::addressof
          ( chunk_state_destruct<Implementation, chunk::access::Const>
          )
      , .chunk_mutable_state_destruct = std::addressof
          ( chunk_state_destruct<Implementation, chunk::access::Mutable>
          )
      , .file_read = std::addressof (file_read<Implementation>)
      , .file_write = std::addressof (file_write<Implementation>)
      };
  }
}

namespace mcs::util
{
  constexpr auto Cast<::MCS_CORE_STORAGE_SIZE, core::storage::MaxSize>::operator()
    ( core::storage::MaxSize const& max_size
    ) const -> ::MCS_CORE_STORAGE_SIZE
  {
    return max_size.is_unlimited()
      ? ::MCS_CORE_STORAGE_SIZE {0}
      : core::memory::size_cast<::MCS_CORE_STORAGE_SIZE> (max_size.limit())
      ;
  }
}

namespace mcs::util
{
  namespace detail
  {
    template<typename Pointer> auto as
      ( ::mcs_core_storage_instance instance
      ) -> Pointer
    {
      if (!instance.state)
      {
        using Implement_C_API = core::storage::Implement_C_API;
        throw Implement_C_API::Error::InstanceMustNotBeNull{};
      }

      return util::cast<Pointer> (instance.state);
    }
  }

  template<core::storage::is_implementation Implementation>
    constexpr auto Cast<Implementation*, ::mcs_core_storage_instance>::operator()
      ( ::mcs_core_storage_instance instance
      ) const -> Implementation*
  {
    return detail::as<Implementation*> (instance);
  }

  template<core::storage::is_implementation Implementation>
    constexpr auto Cast<Implementation const*, ::mcs_core_storage_instance>::operator()
      ( ::mcs_core_storage_instance instance
      ) const -> Implementation const*
  {
    return detail::as<Implementation const*> (instance);
  }
}

namespace mcs::util
{
  template<typename T>
    constexpr auto Cast<T, ::mcs_core_storage_parameter>::operator()
      ( ::mcs_core_storage_parameter const& parameter
      ) const -> T
  {
    return serialization::load_from<T>
      ( util::cast<std::byte const*> (parameter.data)
      , parameter.size
      );
  }
}
