// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>

// \todo provide "to_param", somewhere
#include <algorithm>
#include <iterator>
#include <mcs/core/storage/c_api/types.h>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/util/cast.hpp>

namespace mcs::testing::core::storage::implementation
{
  namespace
  {
    template<typename T>
      auto make_storage_parameter (T const& x)
    {
      auto const bytes {serialization::OArchive {x}.bytes()};
      auto storage_parameter
        { mcs::core::storage::implementation::Virtual::StorageParameter{}
        };
      auto const data
        { util::cast<::MCS_CORE_STORAGE_BYTE const*> (bytes.data())
        };
      std::copy
        ( data
        , data + bytes.size()
        , std::back_inserter (storage_parameter)
        );
      return storage_parameter;
    }
  }

  template<typename TestingStorage>
    struct SharedObject;

  template<> struct SharedObject<Files>
  {
    static auto path() -> std::filesystem::path;
  };
  template<> struct SharedObject<Heap>
  {
    static auto path() -> std::filesystem::path;
  };
  template<> struct SharedObject<SHMEM>
  {
    static auto path() -> std::filesystem::path;
  };

  template<typename TestingStorage>
    auto Virtual<TestingStorage>::parameter_create
      (
      ) const -> typename Storage::Parameter::Create
  {
    return { SharedObject<TestingStorage>::path()
           , make_storage_parameter
             ( static_cast<Base const*> (this)->parameter_create()
             )
           };
  }

  template<typename TestingStorage>
    auto Virtual<TestingStorage>::parameter_size_max
    (
    ) const -> typename Storage::Parameter::Size::Max
  {
    return
      { make_storage_parameter
          ( static_cast<Base const*> (this)->parameter_size_max()
          )
      };
  }
  template<typename TestingStorage>
    auto Virtual<TestingStorage>::parameter_size_used
    (
    ) const -> typename Storage::Parameter::Size::Used
  {
    return
      { make_storage_parameter
          ( static_cast<Base const*> (this)->parameter_size_used()
          )
      };
  }

  template<typename TestingStorage>
    auto Virtual<TestingStorage>::parameter_segment_create
    (
    ) const -> typename Storage::Parameter::Segment::Create
  {
    return
      { make_storage_parameter
          ( static_cast<Base const*> (this)->parameter_segment_create()
          )
      };
  }
  template<typename TestingStorage>
    auto Virtual<TestingStorage>::parameter_segment_remove
    (
    ) const -> typename Storage::Parameter::Segment::Remove
  {
    return
      { make_storage_parameter
          ( static_cast<Base const*> (this)->parameter_segment_remove()
          )
      };
  }

  template<typename TestingStorage>
    auto Virtual<TestingStorage>::parameter_chunk_description
    (
    ) const -> typename Storage::Parameter::Chunk::Description
  {
    return
      { make_storage_parameter
          ( static_cast<Base const*> (this)->parameter_chunk_description()
          )
      };
  }

  template<typename TestingStorage>
    auto Virtual<TestingStorage>::parameter_file_read
    (
    ) const -> typename Storage::Parameter::File::Read
  {
    return
      { make_storage_parameter
          ( static_cast<Base const*> (this)->parameter_file_read()
          )
      };
  }

  template<typename TestingStorage>
    auto Virtual<TestingStorage>::parameter_file_write
    (
    ) const -> typename Storage::Parameter::File::Write
  {
    return
      { make_storage_parameter
          ( static_cast<Base const*> (this)->parameter_file_write()
          )
      };
  }
}
