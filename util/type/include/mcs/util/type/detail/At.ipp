// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util::type
{
  template<std::size_t I, typename Head, typename... Tail>
    struct At<I, Head, Tail...> : At<I - 1, Tail...>
  {};

  template<typename Head, typename... Tail>
    struct At<0, Head, Tail...>
  {
    using type = Head;
  };
}
