// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <mcs/fuse/content/implementation/storage/ioctl/command/Distribution.hpp>
#include <mcs/util/fopen.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/ioctl.hpp>
#include <memory>

namespace mcs::fuse::content::storage
{
  template<typename UsePart>
    requires (std::is_invocable_v<UsePart, Part const&>)
    auto for_each_part (std::filesystem::path path, UsePart use_part)
  {
    auto const file {util::fopen (path, "r")};
    auto const fd {util::syscall::fileno (file.get())};

    using Distribution = ioctl::command::Distribution;

    auto buffer {Distribution::Buffer{}};

    while (buffer)
    {
      util::syscall::ioctl (fd, Distribution::CODE, std::addressof (buffer));

      buffer.for_each_part (use_part);
    }
  }
}
