// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/state/Inode.hpp>
#include <mcs/fuse/state/inode/Kind.hpp>
#include <string>

namespace mcs::fuse::state::inode::directory
{
  template<is_content Content, typename Kind>
    requires (inode::is_kind<Content, Kind>)
    struct Entry
  {
    [[nodiscard]] Entry (std::string, Inode<Content, Kind>*) noexcept;

    [[nodiscard]] constexpr auto name() const noexcept -> std::string const&;
    [[nodiscard]] constexpr auto inode() const noexcept -> Inode<Content, Kind>*;

#if defined (MCS_FUSE_INODE_DIRECTORY_CONTAINER_ORDERED_SET)
    struct Less
    {
      using is_transparent = std::true_type;

      [[nodiscard]] constexpr auto operator()
        ( Entry const& lhs
        , Entry const& rhs
        ) const noexcept -> bool
      {
        return std::less{} (lhs._name, rhs._name);
      }
      [[nodiscard]] constexpr auto operator()
        ( Entry const& lhs
        , std::string const& rhs
        ) const noexcept -> bool
      {
        return std::less{} (lhs._name, rhs);
      }
      [[nodiscard]] constexpr auto operator()
        ( std::string const& lhs
        , Entry const& rhs
        ) const noexcept -> bool
      {
        return std::less{} (lhs, rhs._name);
      }
    };
#endif

#if defined (MCS_FUSE_INODE_DIRECTORY_CONTAINER_UNORDERED_SET)
    struct Equal
    {
      using is_transparent = std::true_type;

      [[nodiscard]] constexpr auto operator()
        ( Entry lhs
        , Entry rhs
        ) const noexcept -> bool
      {
        return std::equal_to{} (lhs._name, rhs._name);
      }
      [[nodiscard]] constexpr auto operator()
        ( Entry lhs
        , std::string const& rhs
        ) const noexcept -> bool
      {
        return std::equal_to{} (lhs._name, rhs);
      }
      [[nodiscard]] constexpr auto operator()
        ( std::string const& lhs
        , Entry rhs
        ) const noexcept -> bool
      {
        return std::equal_to{} (lhs, rhs._name);
      }
    };
    struct Hash
    {
      using is_transparent = std::true_type;
      using is_transparent_key_equal = Equal;

      [[nodiscard]] constexpr auto operator()
        ( Entry entry
        ) const noexcept -> std::size_t
      {
        return std::hash<std::string>{} (entry._name);
      }
      [[nodiscard]] constexpr auto operator()
        ( std::string const& name
        ) const noexcept -> std::size_t
      {
        return std::hash<std::string>{} (name);
      }
    };
#endif

  private:
#if defined (MCS_FUSE_INODE_DIRECTORY_CONTAINER_ORDERED_SET)
    friend struct Less;
#endif
#if defined (MCS_FUSE_INODE_DIRECTORY_CONTAINER_UNORDERED_SET)
    friend struct Equal;
    friend struct Hash;
#endif
    // \todo anonymous tmpfile -> optional name
    std::string _name;
    Inode<Content, Kind>* _inode;
  };
}

#include "detail/Entry.ipp"
