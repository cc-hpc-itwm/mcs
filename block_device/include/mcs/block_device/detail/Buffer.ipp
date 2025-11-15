// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/nonstd/scope.hpp>
#include <stdexcept>

namespace mcs::block_device
{
  template<core::storage::is_implementation... StorageImplementations>
    Buffer<util::type::List<StorageImplementations...>>::Buffer
      ( util::not_null<core::Storages<SupportedStorageImplementations>> storages
      , core::storage::ID storage_id
      , core::storage::Parameter storage_parameter_chunk_description
      , core::storage::segment::ID segment_id
      , core::memory::Range range
      , block_device::block::Size block_size
      )
        : _storages {storages}
        , _storage_id {storage_id}
        , _storage_parameter_chunk_description
            {storage_parameter_chunk_description}
        , _segment_id {segment_id}
        , _block_size {core::memory::make_size (block_size)}
  {
    if (size (range) < core::memory::make_size (block_size))
    {
      // \todo specific exception
      throw std::runtime_error
        { "block_device::Buffer: Not enough memory to hold a single block."
        };
    }

    for ( auto offset {begin (range)}
        ; ! (offset + _block_size > end (range))
        ; offset += _block_size
        )
    {
      _available.push (offset);
    }
  }

  template<core::storage::is_implementation... StorageImplementations>
    Buffer<util::type::List<StorageImplementations...>>::Error::Block::Interrupted::Interrupted()
      : mcs::Error {"block_device::Buffer::block::Interrupted"}
  {}
  template<core::storage::is_implementation... StorageImplementations>
    Buffer<util::type::List<StorageImplementations...>>::Error::Block::Interrupted::~Interrupted() = default
    ;

  template<core::storage::is_implementation... StorageImplementations>
    Buffer<util::type::List<StorageImplementations...>>::Error::Block::Timeout::Timeout()
      : mcs::Error {"block_device::Buffer::block::Timeout"}
  {}
  template<core::storage::is_implementation... StorageImplementations>
    Buffer<util::type::List<StorageImplementations...>>::Error::Block::Timeout::~Timeout() = default
    ;

  template<core::storage::is_implementation... StorageImplementations>
    template<typename TimePoint>
      auto Buffer<util::type::List<StorageImplementations...>>::block
        ( InterruptionContext const& interruption_context
        , TimePoint const& time_point
        ) -> BufferedBlock
  {
          auto lock {std::unique_lock {_guard}};

          if (! _block_available_or_interrupted.wait_until
                ( lock
                , time_point
                , [&]
                  {
                    return interruption_context._interrupted
                      || !_available.empty()
                      ;

                  }
                )
             )
          {
            throw typename Error::Block::Timeout{};
          }

          if (interruption_context._interrupted)
          {
            throw typename Error::Block::Interrupted{};
          }

          auto const offset {_available.top()};
          _available.pop();

          return BufferedBlock
            { new Block<SupportedStorageImplementations>
                { _storages
                , _storage_id
                , _storage_parameter_chunk_description
                , _segment_id
                , core::memory::make_range (offset, _block_size)
                }
            , ReleaseOffset {this}
            };
  }

  template<core::storage::is_implementation... StorageImplementations>
    Buffer<util::type::List<StorageImplementations...>>
      ::ReleaseOffset::ReleaseOffset
        ( util::not_null<Buffer<util::type::List<StorageImplementations...>>>
            buffer
        )
          : _buffer {buffer}
  {}
  template<core::storage::is_implementation... StorageImplementations>
    auto Buffer<util::type::List<StorageImplementations...>>
      ::ReleaseOffset::operator()
        ( Block<SupportedStorageImplementations>* block
        ) const noexcept -> void
  {
    auto const delete_block
      { nonstd::make_scope_exit_that_dies_on_exception
          ( "block_device::Buffer::ReleaseOffset"
          , [&]
            {
              std::default_delete
                < Block<SupportedStorageImplementations>
                >{} (block);
            }
          )
      };

    auto const lock {std::unique_lock {_buffer->_guard}};

    _buffer->_available.push (block->address().offset);

    // \note notify_one would not be enough as the one "notified"
    // might just have been interrupted or timed out
    _buffer->_block_available_or_interrupted.notify_all();
  }

  template<core::storage::is_implementation... StorageImplementations>
    auto Buffer<util::type::List<StorageImplementations...>>::interrupt
      ( util::not_null<InterruptionContext> interruption_context
      ) -> void
  {
    auto const lock {std::unique_lock {_guard}};

    interruption_context->_interrupted = true;

    _block_available_or_interrupted.notify_all();
  }
}
