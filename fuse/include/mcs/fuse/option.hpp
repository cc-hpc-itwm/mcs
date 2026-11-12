// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/command_line/option/Parameter.hpp>

namespace mcs::fuse::option
{
  using util::command_line::option::Parameter;
  using Presence = Parameter::Presence;

  inline constexpr auto content {Parameter {"content"}};
  inline constexpr auto fuse_embedding {"FUSE"};
  inline constexpr auto mountpoint {Parameter {"mountpoint"}};
  inline constexpr auto storage {Parameter {"storage", Presence::Optional{}}};
  inline constexpr auto storage_size_max
    { Parameter {"storage-size-max", Presence::Optional{}}
    };
  inline constexpr auto storage_size_used
    { Parameter {"storage-size-used", Presence::Optional{}}
    };
  inline constexpr auto storage_segment_create
    { Parameter {"storage-segment-create", Presence::Optional{}}
    };
  inline constexpr auto storage_segment_remove
    { Parameter {"storage-segment-remove", Presence::Optional{}}
    };
  inline constexpr auto storage_chunk_description
    { Parameter {"storage-chunk-description", Presence::Optional{}}
    };
}
