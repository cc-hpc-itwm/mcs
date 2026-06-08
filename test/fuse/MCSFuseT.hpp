// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <list>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/Session.hpp>
#include <mcs/fuse/content/implementation/Realloc.hpp>
#include <mcs/fuse/content/implementation/Storage.hpp>
#include <mcs/fuse/content/implementation/Vector.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <vector>

namespace mcs::fuse
{
  namespace
  {
    struct FuseArgsContainer
    {
      [[nodiscard]] auto fuse_args() -> ::fuse_args
      {
        return ::fuse_args
          { .argc = util::cast<int> (_argv.size())
          , .argv = _argv.data()
          , .allocated = 0
          };
      }

      template<typename... Args>
        auto add (Args&&... args)
      {
        auto const _add
          { [&] (auto arg)
            {
              _argv.push_back (_args.emplace_back (arg).data());
            }
          };

        (_add (std::forward<Args> (args)), ...);
      }

      FuseArgsContainer()
      {
        add ("MCSFuse");
        add ("-o", "auto_unmount");
      }
      FuseArgsContainer (FuseArgsContainer const&) = delete;
      FuseArgsContainer (FuseArgsContainer&&) = delete;
      auto operator= (FuseArgsContainer const&) -> FuseArgsContainer& = delete;
      auto operator= (FuseArgsContainer&&) -> FuseArgsContainer& = delete;
      ~FuseArgsContainer() = default;

    private:
      std::list<std::string> _args;
      std::vector<char*> _argv;
    };

    template<fuse::is_content C, typename MCS>
      struct ContentImplementation
    {
      using Content = C;
      using MakeContentState = MCS;
    };

    template<fuse::is_content Content> struct EmptyContentState
    {
      [[nodiscard]] constexpr auto operator()
        (
        ) -> util::not_null<typename Content::State>
      {
        return std::addressof (_content_state);
      }

    private:
      typename Content::State _content_state;
    };

    template<typename TestingStorage>
      struct TestingStorageContentState
    {
      using Content = fuse::content::implementation::Storage
        < typename TestingStorage::Storage
        >
        ;

      [[nodiscard]] constexpr auto operator()
        (
        ) -> util::not_null<typename Content::State>
      {
        return std::addressof (_content_state);
      }

    private:
      TestingStorage _testing_storage;
      typename Content::State _content_state
        { _testing_storage.parameter_create()
        , _testing_storage.parameter_size_max()
        , _testing_storage.parameter_size_used()
        , _testing_storage.parameter_segment_create()
        , _testing_storage.parameter_segment_remove()
        , _testing_storage.parameter_chunk_description()
        };
    };

    using ContentImplementations = ::testing::Types
      < ContentImplementation
          < fuse::content::implementation::Vector
          , EmptyContentState<fuse::content::implementation::Vector>
          >
      , ContentImplementation
          < fuse::content::implementation::Realloc
          , EmptyContentState<fuse::content::implementation::Realloc>
          >
      , ContentImplementation
          < fuse::content::implementation::Storage
              < core::storage::implementation::Heap
              >
          , TestingStorageContentState
             < testing::core::storage::implementation::Heap
             >
          >
      , ContentImplementation
          < fuse::content::implementation::Storage
              < core::storage::implementation::SHMEM
              >
          , TestingStorageContentState
             < testing::core::storage::implementation::SHMEM
             >
          >
      , ContentImplementation
          < fuse::content::implementation::Storage
              < core::storage::implementation::Files
              >
          , TestingStorageContentState
             < testing::core::storage::implementation::Files
             >
          >
      >;

    template<class ContentImplementation> struct MCSFuseT
      : public ::testing::Test
    {
      MCS_UTIL_MEMBER_AUTO
        ( _temporary_directory
        , testing::UniqTemporaryDirectory {"MCSFuseT_mount"}
        );
      MCS_UTIL_MEMBER_AUTO (_path, _temporary_directory.path());

      MCS_UTIL_MEMBER_AUTO (_fuse_args_container, FuseArgsContainer{});
      MCS_UTIL_MEMBER_AUTO (_fuse_args, _fuse_args_container.fuse_args());

      MCS_UTIL_MEMBER_AUTO
        ( _make_content_state
        , typename ContentImplementation::MakeContentState{}
        );
      MCS_UTIL_MEMBER_AUTO
        ( _session
        , std::make_unique<Session<typename ContentImplementation::Content>>
          ( std::addressof (_fuse_args)
          , std::invoke (_make_content_state)
          , session::Options
            { session::option::Mountpoint {_path.c_str()}
            , session::option::Singlethread::No{}
            , session::option::Foreground::No{}
            , session::option::Clone_fd::No{}
            , session::option::MaxThreads {4u}
            }
          )
        );

      MCS_UTIL_MEMBER_AUTO (_session_result, _session->result());

      MCSFuseT() = default;
      MCSFuseT (MCSFuseT const&) = delete;
      MCSFuseT (MCSFuseT&&) = delete;
      auto operator= (MCSFuseT const&) -> MCSFuseT& = delete;
      auto operator= (MCSFuseT&&) -> MCSFuseT& = delete;
      ~MCSFuseT() override
      {
        // ~Session signals fuse_session_exit and joins the loop
        // thread it owns before destroying its members, so by the
        // time reset() returns the loop has fully unwound and the
        // shared future is ready.
        //
        auto const shutdown
          { [&]
            {
              _session.reset();

              ASSERT_EQ (_session_result.get(), 0);
            }
          };

        std::invoke (shutdown);
      }
    };
    TYPED_TEST_SUITE (MCSFuseT, ContentImplementations);
  }
}
