#pragma once
#include "Reflect/CanCallHelper.h"
#include "Reflect/Exceptions.h"

namespace Reflect
{

/// \brief FunctionSelector picks a function from [Functions] based on input arguments, and calls it.
template <typename... Functions> class FunctionArgumentTypeSelector
  {
public:
  typedef FunctionArgumentTypeSelector<Functions...> ThisType;
  typedef std::tuple<Functions...> Selection;

  /// \brief Call to invoke the first matching function.
  /// \param data The data containing the arguments which are passed to the function.
  template <typename InvHelper> static void call(typename InvHelper::CallData data)
    {
    detail::FunctionSelectorHelper<InvHelper, Selection> helper(data);
    tupleEach<Selection>(helper);

    if (!helper.m_foundCall)
      {
      throw Reflect::OverloadException::build<InvHelper, ThisType>(data);
      }
    }

  template <typename InvHelper> static bool canCall(typename InvHelper::CallData data)
    {
    detail::FunctionSelectorCanCallHelper<InvHelper, Selection> helper(data);
    tupleEach<Selection>(helper);
    return helper.m_foundCall;
    }
  };

/// \brief FunctionSelector picks a function from [Functions] based on input argument count, and calls it.
template <typename... Functions> class FunctionArgumentCountSelector
  {
public:
  typedef FunctionArgumentCountSelector<Functions...> ThisType;
  typedef std::tuple<Functions...> Selection;

  /// \brief Call to invoke the first matching function.
  /// \param data The data containing the arguments which are passed to the function.
  template <typename InvHelper> static void call(typename InvHelper::CallData data)
    {
    detail::FunctionArgCountSelectorHelper<InvHelper, Selection> helper(InvHelper::getArgumentCount(data), data);
    tupleEach<Selection>(helper);

    if (!helper.m_foundCall)
      {
      throw Reflect::OverloadArgCountException::build<InvHelper, ThisType>(data);
      }
    }

  template <typename InvHelper> static bool canCall(typename InvHelper::CallData data)
    {
    detail::FunctionArgCountSelectorCanCallHelper<InvHelper, Selection> helper(InvHelper::getArgumentCount(data));
    tupleEach<Selection>(helper);
    return helper.m_foundCall;
    }
  };

}
