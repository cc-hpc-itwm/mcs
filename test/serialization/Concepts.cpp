// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/declare.hpp>
#include <string>
#include <type_traits>

namespace mcs::serialization
{
  TEST ( SerializationConcepts
       , pods_are_serializable
       )
  {
    static_assert (is_serializable<char>);
    static_assert (is_serializable<int>);
    static_assert (is_serializable<long>);
    static_assert (is_serializable<short>);
    static_assert (is_serializable<unsigned char>);
    static_assert (is_serializable<unsigned int>);
    static_assert (is_serializable<unsigned long>);
    static_assert (is_serializable<unsigned short>);
  }

  TEST ( SerializationConcepts
       , empty_with_default_ctor_is_serializable
       )
  {
    struct S{};
    static_assert (std::is_empty_v<S>);
    static_assert (std::is_default_constructible_v<S>);
    static_assert (is_serializable<S>);
  }
  TEST ( SerializationConcepts
       , empty_without_default_ctor_is_not_serializable
       )
  {
    struct S { S() = delete; };
    static_assert (std::is_empty_v<S>);
    static_assert (!std::is_default_constructible_v<S>);
    static_assert (!is_serializable<S>);
  }
  TEST ( SerializationConcepts
       , empty_without_default_ctor_and_with_intrusive_ctor_and_save_is_serializable
       )
  {
    struct S
    {
      explicit S (IArchive&);
      auto save (OArchive&) const -> OArchive&;
    };
    static_assert (std::is_empty_v<S>);
    static_assert (!std::is_default_constructible_v<S>);
    static_assert (is_serializable<S>);
  }

  TEST ( SerializationConcepts
       , trivially_copyable_with_default_ctor_is_serializable
       )
  {
    struct S { int i; };
    static_assert (!std::is_empty_v<S>);
    static_assert (std::is_default_constructible_v<S>);
    static_assert (std::is_trivially_copyable_v<S>);
    static_assert (is_serializable<S>);
  }
  TEST ( SerializationConcepts
       , trivially_copyable_without_default_ctor_is_not_serializable
       )
  {
    struct S { int i; S() = delete; };
    static_assert (!std::is_empty_v<S>);
    static_assert (!std::is_default_constructible_v<S>);
    static_assert (std::is_trivially_copyable_v<S>);
    static_assert (!is_serializable<S>);
  }
  TEST ( SerializationConcepts
       , trivially_copyable_without_default_ctor_and_with_intrusive_ctor_and_save_is_serializable
       )
  {
    struct S
    {
      int i;
      explicit S (IArchive&);
      auto save (OArchive&) const -> OArchive&;
    };
    static_assert (!std::is_empty_v<S>);
    static_assert (!std::is_default_constructible_v<S>);
    static_assert (std::is_trivially_copyable_v<S>);
    static_assert (is_serializable<S>);
  }

  TEST ( SerializationConcepts
       , nontrivial_with_default_ctor_is_not_serializable
       )
  {
    struct S
    {
      std::string s;
    };
    static_assert (!std::is_empty_v<S>);
    static_assert (!std::is_trivially_copyable_v<S>);
    static_assert (std::is_default_constructible_v<S>);
    static_assert (!is_serializable<S>);
  }
  TEST ( SerializationConcepts
       , nontrivial_with_default_ctor_and_intrusive_serialize_is_serializable
       )
  {
    struct S
    {
      std::string s;
      auto serialize (IArchive&, unsigned int) -> void;
      auto serialize (OArchive&, unsigned int) -> void;
    };
    static_assert (!std::is_empty_v<S>);
    static_assert (!std::is_trivially_copyable_v<S>);
    static_assert (std::is_default_constructible_v<S>);
    static_assert (is_serializable<S>);
  }
  TEST ( SerializationConcepts
       , nontrivial_with_default_ctor_and_intrusive_ctor_and_save_is_serializable
       )
  {
    struct S
    {
      std::string s;
      S() = default;
      explicit S (IArchive&);
      auto save (OArchive&) const -> OArchive&;
    };
    static_assert (!std::is_empty_v<S>);
    static_assert (!std::is_trivially_copyable_v<S>);
    static_assert (std::is_default_constructible_v<S>);
    static_assert (is_serializable<S>);
  }
  TEST ( SerializationConcepts
       , nontrivial_without_default_ctor_and_intrusive_ctor_and_save_is_serializable
       )
  {
    struct S
    {
      std::string s;
      S() = delete;
      explicit S (IArchive&);
      auto save (OArchive&) const -> OArchive&;
    };
    static_assert (!std::is_empty_v<S>);
    static_assert (!std::is_trivially_copyable_v<S>);
    static_assert (!std::is_default_constructible_v<S>);
    static_assert (is_serializable<S>);
  }

  struct NonTrivialWithoutDefaultCtor
  {
    NonTrivialWithoutDefaultCtor() = delete;
    std::string s;
  };
  template<> struct Implementation<NonTrivialWithoutDefaultCtor>
  {
    static auto input (IArchive&) -> NonTrivialWithoutDefaultCtor;
    static auto output (OArchive&, NonTrivialWithoutDefaultCtor) -> OArchive&;
  };
  TEST ( SerializationConcepts
       , non_trivial_without_default_ctor_and_nonintrusive_implementation_is_serializable
       )
  {
    static_assert (!std::is_empty_v<NonTrivialWithoutDefaultCtor>);
    static_assert (!std::is_trivially_copyable_v<NonTrivialWithoutDefaultCtor>);
    static_assert (!std::is_default_constructible_v<NonTrivialWithoutDefaultCtor>);
    static_assert (is_serializable<NonTrivialWithoutDefaultCtor>);
  }
}
