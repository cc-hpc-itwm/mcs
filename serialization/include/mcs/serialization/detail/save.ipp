// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/serialization/error/Save.hpp>
#include <memory>

namespace mcs::serialization
{
  template<is_serializable T>
    constexpr auto save (OArchive& oa, T const& x) -> OArchive&
  try
  {
    if constexpr (is_empty<T>)
    {
      return oa;
    }

    if constexpr (is_trivial<T>)
    {
      return oa.append (std::span {std::addressof (x), 1});
    }

    if constexpr (has_intrusive_ctor_and_save<T>)
    {
      return x.save (oa);
    }

    if constexpr (has_intrusive_serialize<T>)
    {
      const_cast<T&> (x).serialize (oa, 0u);

      return oa;
    }

    if constexpr (has_nonintrusive_Implementation<T>)
    {
      return Implementation<T>::output (oa, x);
    }
  }
  catch (...)
  {
    std::throw_with_nested (error::Save{});
  }

  template<is_serializable T>
    constexpr auto save (OArchive& oa, T& x) -> OArchive&
  {
    return save (oa, static_cast<T const&> (x));
  }

  template<is_serializable T>
    auto save (OArchive& oa, T&& x) -> OArchive&
  try
  {
    if constexpr (is_empty<T>)
    {
      return oa;
    }

    if constexpr (is_trivial<T>)
    {
      auto const value {T {std::forward<T> (x)}};
      auto const data {std::as_bytes (std::span {std::addressof (value), 1})};

      return oa.append
        ( std::span { oa._bytes_for_temporaries.emplace_back
                      ( std::begin (data), std::end (data)
                      )
                    }
        );
    }

    if constexpr (has_intrusive_ctor_and_save<T>)
    {
      auto _oa {OArchive{}};
      std::forward<T> (x).save (_oa);
      return oa.store (_oa);
    }

    if constexpr (has_intrusive_serialize<T>)
    {
      auto _oa {OArchive{}};
      std::forward<T> (x).serialize (_oa, 0u);
      return oa.store (_oa);
    }

    if constexpr (has_nonintrusive_Implementation<T>)
    {
      auto _oa {OArchive{}};
      Implementation<T>::output (_oa, std::forward<T> (x));
      return oa.store (_oa);
    }
  }
  catch (...)
  {
    std::throw_with_nested (error::Save{});
  }
}
