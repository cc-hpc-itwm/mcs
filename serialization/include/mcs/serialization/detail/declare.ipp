// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#define MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION_IMPL(_type...)   \
  struct Implementation<_type>                                             \
  {                                                                        \
    static auto output                                                     \
      ( OArchive&                                                          \
      , _type const&                                                       \
      ) -> OArchive&                                                       \
      ;                                                                    \
    static auto input                                                      \
      ( IArchive&                                                          \
      ) -> _type                                                           \
      ;                                                                    \
  }

#define MCS_SERIALIZATION_DECLARE_INTRUSIVE_CTOR_AND_SAVE_IMPL(_type)    \
  explicit _type (serialization::IArchive&);                             \
  auto save (serialization::OArchive&) const                             \
    -> serialization::OArchive&
