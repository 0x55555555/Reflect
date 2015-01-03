#pragma once
#include "Reflect/Utils/CanCallHelper.h"
#include "Reflect/Utils/Exceptions.h"
#include "Reflect/Utils/StaticCombiner.h"

namespace Reflect
{

/// \brief FunctionSelector picks a function from [Functions] based on input arguments, and calls it.
template <typename... Functions> class FunctionArgumentTypeSelector
  {
public:
  typedef FunctionArgumentTypeSelector<Functions...> ThisType;
  typedef std::tuple<Functions...> Selection;
  struct Helper
    {
    typedef typename StaticCombiner<Selection>::Value Static;
    };

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
  struct Helper
    {
    typedef typename StaticCombiner<Selection>::Value Static;
    typedef std::integral_constant<size_t, 0> ArgumentCount;
    };

  /// \brief Call to invoke the first matching function.
  /// \param data The data containing the arguments which are passed to the function.
  template <typename InvHelper> static void call(typename InvHelper::CallData data)
    {
    detail::FunctionArgCountSelectorHelper<InvHelper, Selection> helper(InvHelper::getArgumentCountWithThis(data), data);
    tupleEach<Selection>(helper);

    if (!helper.m_foundCall)
      {
      throw Reflect::OverloadArgCountException::build<InvHelper, ThisType>(data);
      }
    }

  template <typename InvHelper> static bool canCall(typename InvHelper::CallData data)
    {
    detail::FunctionArgCountSelectorCanCallHelper<InvHelper, Selection> helper(InvHelper::getArgumentCountWithThis(data));
    tupleEach<Selection>(helper);
    return helper.m_foundCall;
    }
  };

}
