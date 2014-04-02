#pragma once
#include "Reflect/CanCallHelper.h"

namespace Reflect
{

/// \brief FunctionSelector picks a function from [Functions] based on input arguments, and calls it.
template <typename InvHelper, typename... Functions> class FunctionSelector
  {
public:
  typedef std::tuple<Functions...> Selection;

  /// \brief Call to invoke the first matching function.
  /// \param data The data containing the arguments which are passed to the function.
  static void call(typename InvHelper::CallData data)
    {
    detail::FunctionSelectorCanCallHelper<InvHelper, Selection> helper(data);
    tupleEach<Selection>(helper);
    }

  static bool canCall(typename InvHelper::CallData data)
    {
    detail::FunctionSelectorCanCallHelper<InvHelper, Selection> helper(data);
    tupleEach<Selection>(helper);
    return helper.m_foundCall;
    }
  };

}
