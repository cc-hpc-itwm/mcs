// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/Concepts.hpp>
#include <string>

namespace mcs::core::storage::selection
{
  struct Arguments
  {
    std::string storage;
    std::string parameter_size_max;
    std::string parameter_size_used;
    std::string parameter_segment_create;
    std::string parameter_segment_remove;
    std::string parameter_chunk_description;
  };

  template<is_implementation Implementation>
    struct Parameters
  {
    using Parameter = Implementation::Parameter;

    typename Parameter::Create create;
    typename Parameter::Size::Max size_max;
    typename Parameter::Size::Used size_used;
    typename Parameter::Segment::Create segment_create;
    typename Parameter::Segment::Remove segment_remove;
    typename Parameter::Chunk::Description chunk_description;
  };

  // Select one of the caller-provided storage implementations.
  //
  // Parses the creation parameter to identify the implementation,
  // parses the remaining arguments as that implementation's parameter
  // types, and invokes the function with the resulting typed
  // Parameters.
  //
  // EXAMPLE:
  //
  //     using SupportedImplementations = util::type::List<Files, Heap>;
  //
  //     selection::select<SupportedImplementations>
  //       ( arguments
  //       , []<is_implementation Implementation>
  //           (Parameters<Implementation> parameters)
  //         {
  //           return use<Implementation> (std::move (parameters));
  //         }
  //       );
  //
  template<typename SupportedImplementations, typename Function>
    auto select (Arguments const&, Function&&);
}

#include "detail/select.ipp"
