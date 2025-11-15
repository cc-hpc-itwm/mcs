// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/rpc/detail/ResultOrError.hpp>
#include <mcs/rpc/error/Completion.hpp>
#include <mcs/rpc/error/HandlerException.hpp>
#include <mcs/util/overloaded.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

namespace mcs::rpc::detail
{
  template<typename T>
    constexpr Completion::Completion (std::promise<T> promise)
      : _complete
     //! \note std::function must be copy-constructible, therefore the
     //! shared pointer.
  { [_promise = std::make_shared<std::promise<T>> (std::move (promise))]
      ( std::exception_ptr rpc_error
      , Buffer buffer
      ) mutable
    {
      if (rpc_error)
      {
        try
        {
          throw error::Completion {rpc_error};
        }
        catch (...)
        {
          _promise->set_exception (std::current_exception());
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
                    // \todo rethrow the complete (de/serialized!)
                    // exception
                    throw error::HandlerError {handler_error.reason};
                  }
                  catch (...)
                  {
                    _promise->set_exception (std::current_exception());
                  }
                }
              , [&] (Result<T> result)
                {
                  if constexpr (std::is_same_v<T, void>)
                  {
                     _promise->set_value();
                  }
                  else
                  {
                    _promise->set_value (std::move (result.value));
                  }
                }
              )
            , buffer.template load<ResultOrError<T>>()
            );
          }
          catch (...)
          {
            _promise->set_exception (std::current_exception());
          }
        }
      }
    }
  {}
}
