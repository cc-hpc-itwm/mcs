// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/read.hpp>
#include <utility>

namespace mcs::core::storage::selection
{
  namespace detail
  {
    template<is_implementation Implementation>
      struct Create
    {
      using type = Implementation::Parameter::Create;
    };
  }

  template<typename SupportedImplementations, typename Function>
    auto select (Arguments const& arguments, Function&& function)
  {
    return SupportedImplementations::template parse_id_and_run<detail::Create>
      ( arguments.storage
      , [&]<is_implementation Implementation>
          (typename Implementation::Parameter::Create create)
        {
          using Parameter = typename Implementation::Parameter;
          using util::read::read;

          return std::forward<Function> (function)
            .template operator()<Implementation>
              ( Parameters<Implementation>
                { std::move (create)
                , read<typename Parameter::Size::Max>
                    (arguments.parameter_size_max)
                , read<typename Parameter::Size::Used>
                    (arguments.parameter_size_used)
                , read<typename Parameter::Segment::Create>
                    (arguments.parameter_segment_create)
                , read<typename Parameter::Segment::Remove>
                    (arguments.parameter_segment_remove)
                , read<typename Parameter::Chunk::Description>
                    (arguments.parameter_chunk_description)
                }
              );
        }
      );
  }
}
