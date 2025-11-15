// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/detail/tag/STD/List.hpp>
#include <mcs/serialization/detail/tag/STD/Map.hpp>
#include <mcs/serialization/detail/tag/STD/Optional.hpp>
#include <mcs/serialization/detail/tag/STD/Set.hpp>
#include <mcs/serialization/detail/tag/STD/String.hpp>
#include <mcs/serialization/detail/tag/STD/UniquePtr.hpp>
#include <mcs/serialization/detail/tag/STD/UnorderedMap.hpp>
#include <mcs/serialization/detail/tag/STD/UnorderedSet.hpp>
#include <mcs/serialization/detail/tag/STD/Variant.hpp>
#include <mcs/serialization/detail/tag/STD/Vector.hpp>
#include <variant>

namespace mcs::serialization::detail
{
  using Tag = std::variant
    < tag::STD::List
    , tag::STD::Map
    , tag::STD::Optional
    , tag::STD::Set
    , tag::STD::String
    , tag::STD::UniquePtr
    , tag::STD::UnorderedMap
    , tag::STD::UnorderedSet
    , tag::STD::Variant
    , tag::STD::Vector
    >;
}
