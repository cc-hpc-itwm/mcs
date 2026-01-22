// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::fuse::state::inode::directory
{
  template<is_content Content, typename Kind>
    requires (inode::is_kind<Content, Kind>)
    Entry<Content, Kind>::Entry
      ( std::string name
      , Inode<Content, Kind>* inode
      ) noexcept
      : _name {std::move (name)}
      , _inode {inode}
  {}

  template<is_content Content, typename Kind>
    requires (inode::is_kind<Content, Kind>)
    constexpr auto Entry<Content, Kind>::name
      (
      ) const noexcept -> std::string const&
  {
    return _name;
  }

  template<is_content Content, typename Kind>
    requires (inode::is_kind<Content, Kind>)
    constexpr auto Entry<Content, Kind>::inode
    (
    ) const noexcept -> Inode<Content, Kind>*
  {
    return _inode;
  }
}
