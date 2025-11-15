// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/execute_and_die_on_exception.hpp>
#include <utility>

namespace mcs::core::storage::implementation
{
  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    Trace<Tracer, Storage>::StorageDeleter::StorageDeleter
      ( util::not_null<Tracer> tracer
      ) noexcept
        : _tracer {tracer}
  {}

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    auto Trace<Tracer, Storage>::StorageDeleter::operator()
      ( Storage* storage
      ) const noexcept -> void
  {
    util::execute_and_die_on_exception
      ( "storage::implementation::Tracer::StorageDeleter"
      , [&]
        {
          _tracer->template trace<storage::trace::event::Destruct>();

          std::default_delete<Storage>{} (storage);
        }
      );
  }

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    Trace<Tracer, Storage>::Trace (Parameter::Create parameter_create)
      : _tracer {parameter_create._parameter_tracer_create}
      , _storage
        { std::invoke
          ( [&]
            {
              trace<storage::trace::event::Create<Storage>>
                ( parameter_create._parameter_storage_create
                );

              return new Storage
                { parameter_create._parameter_storage_create
                };
            }
          )
        , StorageDeleter {std::addressof (_tracer)}
        }
  {}

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    template<typename Event, typename... Args>
      requires (std::is_constructible_v<Event, Args...>)
      auto Trace<Tracer, Storage>::trace (Args&&... args) const -> void
  {
    _tracer.template trace<Event> (std::forward<Args> (args)...);
  }
}

namespace mcs::core::storage::implementation
{
  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    auto Trace<Tracer, Storage>::size_max
      ( Parameter::Size::Max parameter_size_max
      ) const -> MaxSize
  {
    trace<storage::trace::event::size::Max<Storage>>
      ( parameter_size_max
      );

    auto size_max {_storage->size_max (parameter_size_max)};

    trace<storage::trace::event::size::max::Result>
      ( size_max
      );

    return size_max;
  }

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    auto Trace<Tracer, Storage>::size_used
      ( Parameter::Size::Used parameter_size_used
      ) const -> memory::Size
  {
    trace<storage::trace::event::size::Used<Storage>>
      ( parameter_size_used
      );

    auto size_used {_storage->size_used (parameter_size_used)};

    trace<storage::trace::event::size::used::Result>
      ( size_used
      );

    return size_used;
  }

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    auto Trace<Tracer, Storage>::segment_create
      ( Parameter::Segment::Create parameter_segment_create
      , memory::Size size
      ) -> segment::ID
  {
    trace<storage::trace::event::segment::Create<Storage>>
      ( parameter_segment_create
      , size
      );

    auto segment_id {_storage->segment_create (parameter_segment_create, size)};

    trace<storage::trace::event::segment::create::Result>
      ( segment_id
      );

    return segment_id;
  }

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    auto Trace<Tracer, Storage>::segment_remove
      ( Parameter::Segment::Remove parameter_segment_remove
      , segment::ID segment_id
      ) -> memory::Size
  {
    trace<storage::trace::event::segment::Remove<Storage>>
      ( parameter_segment_remove
      , segment_id
      );

    auto size {_storage->segment_remove (parameter_segment_remove, segment_id)};

    trace<storage::trace::event::segment::remove::Result>
      ( size
      );

    return size;
  }

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    template<core::chunk::is_access Access>
      auto Trace<Tracer, Storage>::chunk_description
          ( Parameter::Chunk::Description parameter_chunk_description
          , segment::ID segment_id
          , memory::Range memory_range
          ) const -> Chunk::template Description<Access>
  {
    trace<storage::trace::event::chunk::Description<Storage, Access>>
      ( parameter_chunk_description
      , segment_id
      , memory_range
      );

    auto chunk_description
      { _storage->template chunk_description<Access>
          ( parameter_chunk_description
          , segment_id
          , memory_range
          )
      };

    trace<storage::trace::event::chunk::description::Result<Storage, Access>>
      ( chunk_description
      );

    return chunk_description;
  }

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    auto Trace<Tracer, Storage>::file_read
      ( Parameter::File::Read parameter_file_read
      , storage::segment::ID segment_id
      , memory::Offset offset
      , std::filesystem::path path
      , memory::Range range
      ) const -> memory::Size
  {
    trace<storage::trace::event::file::Read<Storage>>
      ( parameter_file_read
      , segment_id
      , offset
      , path
      , range
      );

    auto bytes_read
      { _storage->file_read
          ( parameter_file_read
          , segment_id
          , offset
          , path
          , range
          )
      };

    trace<storage::trace::event::file::read::Result>
      ( bytes_read
      );

    return bytes_read;
  }

  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    auto Trace<Tracer, Storage>::file_write
      ( Parameter::File::Write parameter_file_write
      , storage::segment::ID segment_id
      , memory::Offset offset
      , std::filesystem::path path
      , memory::Range range
      ) const -> memory::Size
  {
    trace<storage::trace::event::file::Write<Storage>>
      ( parameter_file_write
      , segment_id
      , offset
      , path
      , range
      );

    auto bytes_written
      { _storage->file_write
          ( parameter_file_write
          , segment_id
          , offset
          , path
          , range
          )
      };

    trace<storage::trace::event::file::write::Result>
      ( bytes_written
      );

    return bytes_written;
  }
}
