// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <functional>
#include <stdexcept>
#include <utility>

namespace mcs::iov_backend
{
  namespace detail
  {
    // Dispatcher is required because tl::expected (in cxx20) does not
    // handle void well. When cxx23 and std::expected is used, then
    // the implementation of Invoke<R> can be used directly.
    //
    template<typename...> struct Dispatch;

    template<> struct Dispatch<void>
    {
      template<typename Fun, typename... Args>
        static auto invoke
          ( Fun&& fun
          , std::string error_description
          , Args&&... args
          ) -> void
      {
        auto maybe_result
          {std::invoke (std::forward<Fun> (fun), std::forward<Args> (args)...)};

        if (!maybe_result)
        {
          throw std::runtime_error
            { fmt::format ( "{}: {}"
                          , error_description
                          , maybe_result.error().reason()
                          )
            };
        }

        return;
      }
    };

    template<typename R> struct Dispatch<R>
    {
      template<typename Fun, typename... Args>
        static auto invoke
          ( Fun&& fun
          , std::string error_description
          , Args&&... args
          ) -> R
      {
        auto maybe_result
          {std::invoke (std::forward<Fun> (fun), std::forward<Args> (args)...)};

        if (!maybe_result)
        {
          throw std::runtime_error
            { fmt::format ( "{}: {}"
                          , error_description
                          , maybe_result.error().reason()
                          )
            };
        }

        return maybe_result.value();
      }
    };
  }

  template<typename Fun, typename... Args>
    auto invoke_and_throw_on_unexpected
      ( Fun&& fun
      , std::string error_description
      , Args&&... args
      ) -> typename std::invoke_result_t<Fun, Args&&...>::value_type
  {
    using R = typename std::invoke_result_t<Fun, Args&&...>::value_type;
    return detail::Dispatch<R>::invoke
      ( std::forward<Fun> (fun)
      , error_description
      , std::forward<Args> (args)...
      );
  }
}
