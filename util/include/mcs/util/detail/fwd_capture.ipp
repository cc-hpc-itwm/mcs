// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <utility>

namespace mcs::util
{
  // https://vittorioromeo.info/index/blog/capturing_perfectly_forwarded_objects_in_lambdas.html
  namespace fwd_capture_implementation
  {
    template<typename T>
      class by_value
    {
    private:
      T _x;

    public:
      // NOLINTNEXTLINE (bugprone-forwarding-reference-overload)
      by_value (T&& x) : _x {std::forward<T> (x)} {}

      auto get() & -> T& { return _x; }
      auto get() const& -> T const& { return _x; }
      auto get() && -> T&& { return std::move (_x); }
    };

    template<typename T>
      class by_ref
    {
    private:
      std::reference_wrapper<T> _x;

    public:
      by_ref (T& x) : _x {x} {}

      auto get() & -> T& { return _x.get(); }
      auto get() const& -> T const& { return _x.get(); }
      auto get() && -> T&& { return std::move (_x.get()); }
    };
  }

  template<typename T>
    struct fwd_capture_wrapper : fwd_capture_implementation::by_value<T>
  {
    using fwd_capture_implementation::by_value<T>::by_value;
  };

  template<typename T>
    struct fwd_capture_wrapper<T&> : fwd_capture_implementation::by_ref<T>
  {
    using fwd_capture_implementation::by_ref<T>::by_ref;
  };

  template<typename T>
    auto fwd_capture (T&& x)
  {
    return fwd_capture_wrapper<T> {std::forward<T> (x)};
  }
}
