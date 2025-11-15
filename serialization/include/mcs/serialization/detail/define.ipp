// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#define MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT_IMPL(_oa, _name, _type...) \
  auto Implementation<_type>::output                                       \
    ( OArchive& _oa                                                        \
    , _type const& _name                                                   \
    ) -> OArchive&

#define MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT_IMPL(_ia, _type...) \
  auto Implementation<_type>::input                                        \
    ( IArchive& _ia                                                        \
    ) -> _type

#define MCS_SERIALIZATION_LOAD_FIELD_IMPL(_ia, _field, _type...)           \
  auto _field {load<decltype (std::declval<_type>()._field)> (_ia)}

#define MCS_SERIALIZATION_SAVE_FIELD_IMPL(_oa, _name, _field)              \
  save (_oa, _name._field)
