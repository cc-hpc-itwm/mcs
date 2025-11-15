// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/serialization/define.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/require_semi.hpp>
#include <tuple>

#define MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL(_prefix, _type, _members...)      \
  namespace fmt                                                            \
  {                                                                        \
    MCS_UTIL_FMT_DEFINE_PARSE (ctx, _type)                                 \
    {                                                                      \
      return ctx.begin();                                                  \
    }                                                                      \
    MCS_UTIL_FMT_DEFINE_FORMAT (value, ctx, _type)                         \
    {                                                                      \
      std::ignore = value; /* to support zero members */                   \
                                                                           \
      return fmt::format_to                                                \
        ( ctx.out()                                                        \
        , "{}{}"                                                           \
        , _prefix                                                          \
        , std::make_tuple (_members)                                       \
        );                                                                 \
    }                                                                      \
  }

#define MCS_UTIL_TUPLISH_DEFINE_READ_IMPL(_prefix, _type, _member_types...) \
  namespace mcs::util::read                                                \
  {                                                                        \
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, _type)        \
    {                                                                      \
      prefix (state, _prefix);                                             \
                                                                           \
      return std::make_from_tuple<_type>                                   \
        (parse<std::tuple<_member_types>> (state));                        \
    }                                                                      \
  }

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ0_IMPL(_prefix, _type)             \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_IMPL(_prefix, _type, _m0)                 \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                                  \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                                 \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ2_IMPL(_prefix, _type, _m0, _m1)   \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ3_IMPL(_prefix, _type, _m0, _m1, _m2) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ4_IMPL(_prefix, _type, _m0, _m1, _m2, _m3)  \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ5_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    , value._m4                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    , decltype (std::declval<_type>()._m4)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ6_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    , value._m4                                                            \
    , value._m5                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    , decltype (std::declval<_type>()._m4)                                 \
    , decltype (std::declval<_type>()._m5)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ7_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    , value._m4                                                            \
    , value._m5                                                            \
    , value._m6                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    , decltype (std::declval<_type>()._m4)                                 \
    , decltype (std::declval<_type>()._m5)                                 \
    , decltype (std::declval<_type>()._m6)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ8_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    , value._m4                                                            \
    , value._m5                                                            \
    , value._m6                                                            \
    , value._m7                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    , decltype (std::declval<_type>()._m4)                                 \
    , decltype (std::declval<_type>()._m5)                                 \
    , decltype (std::declval<_type>()._m6)                                 \
    , decltype (std::declval<_type>()._m7)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ9_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    , value._m4                                                            \
    , value._m5                                                            \
    , value._m6                                                            \
    , value._m7                                                            \
    , value._m8                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    , decltype (std::declval<_type>()._m4)                                 \
    , decltype (std::declval<_type>()._m5)                                 \
    , decltype (std::declval<_type>()._m6)                                 \
    , decltype (std::declval<_type>()._m7)                                 \
    , decltype (std::declval<_type>()._m8)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ10_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    , value._m4                                                            \
    , value._m5                                                            \
    , value._m6                                                            \
    , value._m7                                                            \
    , value._m8                                                            \
    , value._m9                                                            \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    , decltype (std::declval<_type>()._m4)                                 \
    , decltype (std::declval<_type>()._m5)                                 \
    , decltype (std::declval<_type>()._m6)                                 \
    , decltype (std::declval<_type>()._m7)                                 \
    , decltype (std::declval<_type>()._m8)                                 \
    , decltype (std::declval<_type>()._m9)                                 \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ11_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    , value._m4                                                            \
    , value._m5                                                            \
    , value._m6                                                            \
    , value._m7                                                            \
    , value._m8                                                            \
    , value._m9                                                            \
    , value._m10                                                           \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    , decltype (std::declval<_type>()._m4)                                 \
    , decltype (std::declval<_type>()._m5)                                 \
    , decltype (std::declval<_type>()._m6)                                 \
    , decltype (std::declval<_type>()._m7)                                 \
    , decltype (std::declval<_type>()._m8)                                 \
    , decltype (std::declval<_type>()._m9)                                 \
    , decltype (std::declval<_type>()._m10)                                \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ12_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10, _m11) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_IMPL                                         \
    ( _prefix                                                              \
    , _type                                                                \
    , value._m0                                                            \
    , value._m1                                                            \
    , value._m2                                                            \
    , value._m3                                                            \
    , value._m4                                                            \
    , value._m5                                                            \
    , value._m6                                                            \
    , value._m7                                                            \
    , value._m8                                                            \
    , value._m9                                                            \
    , value._m10                                                           \
    , value._m11                                                           \
    )                                                                      \
                                                                           \
  MCS_UTIL_TUPLISH_DEFINE_READ_IMPL                                        \
    ( _prefix                                                              \
    , _type                                                                \
    , decltype (std::declval<_type>()._m0)                                 \
    , decltype (std::declval<_type>()._m1)                                 \
    , decltype (std::declval<_type>()._m2)                                 \
    , decltype (std::declval<_type>()._m3)                                 \
    , decltype (std::declval<_type>()._m4)                                 \
    , decltype (std::declval<_type>()._m5)                                 \
    , decltype (std::declval<_type>()._m6)                                 \
    , decltype (std::declval<_type>()._m7)                                 \
    , decltype (std::declval<_type>()._m8)                                 \
    , decltype (std::declval<_type>()._m9)                                 \
    , decltype (std::declval<_type>()._m10)                                \
    , decltype (std::declval<_type>()._m11)                                \
    ) MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1_IMPL(_type, _m0)         \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
                                                                           \
      return _type {_m0};                                                  \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2_IMPL(_type, _m0, _m1)       \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
                                                                           \
      return _type {_m0, _m1};                                             \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION3_IMPL(_type, _m0, _m1, _m2)  \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
                                                                           \
      return _type {_m0, _m1, _m2};                                        \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION4_IMPL(_type, _m0, _m1, _m2, _m3) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
                                                                           \
      return _type {_m0, _m1, _m2, _m3};                                   \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION5_IMPL(_type, _m0, _m1, _m2, _m3, _m4) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m4);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m4, _type);                       \
                                                                           \
      return _type {_m0, _m1, _m2, _m3, _m4};                              \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION6_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m4);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m5);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m4, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m5, _type);                       \
                                                                           \
      return _type {_m0, _m1, _m2, _m3, _m4, _m5};                         \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION7_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m4);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m5);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m6);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m4, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m5, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m6, _type);                       \
                                                                           \
      return _type {_m0, _m1, _m2, _m3, _m4, _m5, _m6};                    \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION8_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m4);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m5);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m6);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m7);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m4, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m5, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m6, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m7, _type);                       \
                                                                           \
      return _type {_m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7};               \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION9_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m4);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m5);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m6);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m7);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m8);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m4, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m5, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m6, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m7, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m8, _type);                       \
                                                                           \
      return _type {_m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8};          \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION10_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m4);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m5);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m6);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m7);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m8);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m9);                       \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m4, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m5, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m6, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m7, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m8, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m9, _type);                       \
                                                                           \
      return _type {_m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9};     \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION11_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m4);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m5);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m6);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m7);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m8);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m9);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m10);                      \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m4, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m5, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m6, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m7, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m8, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m9, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m10, _type);                      \
                                                                           \
      return _type {_m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10}; \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION12_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10, _m11) \
  namespace mcs::serialization                                             \
  {                                                                        \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT            \
      (oa, value, _type)                                                   \
    {                                                                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m0);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m1);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m2);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m3);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m4);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m5);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m6);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m7);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m8);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m9);                       \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m10);                      \
      MCS_SERIALIZATION_SAVE_FIELD (oa, value, _m11);                      \
                                                                           \
      return oa;                                                           \
    }                                                                      \
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT             \
      (ia, _type)                                                          \
    {                                                                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m0, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m1, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m2, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m3, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m4, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m5, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m6, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m7, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m8, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m9, _type);                       \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m10, _type);                      \
      MCS_SERIALIZATION_LOAD_FIELD (ia, _m11, _type);                      \
                                                                           \
      return _type {_m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10, _m11}; \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_SIMPLE_IMPL(_prefix, _type, _m0) \
  namespace fmt                                                            \
  {                                                                        \
    MCS_UTIL_FMT_DEFINE_PARSE (ctx, _type)                                 \
    {                                                                      \
      return ctx.begin();                                                  \
    }                                                                      \
    MCS_UTIL_FMT_DEFINE_FORMAT (value, ctx, _type)                         \
    {                                                                      \
      return fmt::format_to (ctx.out(), "{}{}", _prefix, value._m0);       \
    }                                                                      \
  }                                                                        \
                                                                           \
  namespace mcs::util::read                                                     \
  {                                                                        \
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, _type)        \
    {                                                                      \
      prefix (state, _prefix);                                             \
                                                                           \
      return _type {parse<decltype (_type::_m0)> (state)};                 \
    }                                                                      \
  } MCS_UTIL_REQUIRE_SEMI()
