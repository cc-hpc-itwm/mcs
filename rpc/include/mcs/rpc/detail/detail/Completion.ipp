// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <future>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/Buffer.hpp>
#include <mcs/rpc/detail/ResultOrError.hpp>
#include <mcs/rpc/error/Completion.hpp>
#include <mcs/rpc/error/HandlerException.hpp>
#include <mcs/util/overloaded.hpp>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

namespace mcs::rpc::detail
{
  template<typename T>
    auto fulfill_promise
      ( std::promise<T>& promise
      , std::exception_ptr rpc_error
      , Buffer buffer
      ) -> void
  {
    if (rpc_error)
    {
      try
      {
        throw error::Completion {rpc_error};
      }
      catch (...)
      {
        promise.set_exception (std::current_exception());
      }
    }
    else
    {
      try
      {
        std::visit
          ( util::overloaded
            ( [&] (Error handler_error)
              {
                try
                {
                  // \todo rethrow the complete (de/serialized!) exception
                  throw error::HandlerError {handler_error.reason};
                }
                catch (...)
                {
                  promise.set_exception (std::current_exception());
                }
              }
            , [&] (Result<T> result)
              {
                if constexpr (std::is_same_v<T, void>)
                {
                  promise.set_value();
                }
                else
                {
                  promise.set_value (std::move (result.value));
                }
              }
            )
          , buffer.template load<ResultOrError<T>>()
          );
      }
      catch (...)
      {
        promise.set_exception (std::current_exception());
      }
    }
  }

  template<typename T, is_access_policy AccessPolicy>
    constexpr Completion::Completion
      ( std::promise<T> promise
      , std::shared_ptr<AccessPolicy> policy
      )
        : Completion {Initialize{}, std::move (promise), policy}
  {
    if constexpr (has_call_started<AccessPolicy>)
    {
      policy->call_started();
    }
  }

  template<typename T, is_access_policy AccessPolicy>
    requires (  !has_call_completed<AccessPolicy>
             && !has_error_execution<AccessPolicy>
             )
    constexpr Completion::Completion
      ( Initialize
      , std::promise<T> promise
      , std::shared_ptr<AccessPolicy>
      )
        : _complete
          { [ shared_promise
              = std::make_shared<std::promise<T>> (std::move (promise))
            ] (std::exception_ptr rpc_error, Buffer buffer)
            {
              fulfill_promise (*shared_promise, rpc_error, std::move (buffer));
            }
          }
  {}

  template<typename T, is_access_policy AccessPolicy>
    requires (  has_call_completed<AccessPolicy>
             || has_error_execution<AccessPolicy>
             )
    constexpr Completion::Completion
      ( Initialize
      , std::promise<T> promise
      , std::shared_ptr<AccessPolicy> policy
      )
        : _complete
          { [ shared_promise
              = std::make_shared<std::promise<T>> (std::move (promise))
            , policy = std::move (policy)
            ] (std::exception_ptr rpc_error, Buffer buffer)
            {
              if constexpr (has_error_execution<AccessPolicy>)
              {
                if (rpc_error)
                {
                  policy->error_execution();
                }
              }

              if constexpr (has_call_completed<AccessPolicy>)
              {
                if (!rpc_error)
                {
                  policy->call_completed();
                }
              }

              fulfill_promise (*shared_promise, rpc_error, std::move (buffer));
            }
          }
  {}
}
