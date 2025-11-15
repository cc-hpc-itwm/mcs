// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#define MCS_UTIL_HASH_DECLARE_IMPL(_type...)                            \
  struct hash<_type>                                                    \
  {                                                                     \
    auto operator() (_type const&) const noexcept -> size_t;            \
  }

#define MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER_IMPL(_member, _type...) \
  struct hash<_type>                                                    \
  {                                                                     \
    auto operator() (_type const&) const noexcept -> size_t;            \
                                                                        \
  private:                                                              \
    hash<decltype (_type::_member)> _hash;                              \
  }

#define MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE_IMPL(_type...) \
  struct hash<_type>                                                    \
  {                                                                     \
    auto operator() (_type const&) const noexcept -> size_t;            \
                                                                        \
  private:                                                              \
    hash<_type::underlying_type> _hash;                                 \
  }
