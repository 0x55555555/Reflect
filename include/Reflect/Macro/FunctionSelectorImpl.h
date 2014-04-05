#pragma once
#include "Reflect/CanCallHelper.h"
#include <type_traits>
#include <tuple>

namespace Reflect
{

namespace detail
{
struct NilFunction
  {
  };
}

/// \brief FunctionSelector picks a function from [Functions] based on input arguments, and calls it.
template <typename A,
          typename B,
          typename C=detail::NilFunction,
          typename D=detail::NilFunction> class FunctionArgumentTypeSelector
  {
public:
  typedef FunctionArgumentTypeSelector<A, B, C, D> ThisType;

  typedef std::tuple<A, B> Selection2;
  typedef std::tuple<A, B, C> Selection3;
  typedef std::tuple<A, B, C, D> Selection4;

  typedef std::tuple<Selection2, Selection3, Selection4> SelectionOptions;

  typedef std::integral_constant<size_t, std::is_same<C, detail::NilFunction>::value ? 2
                                       : std::is_same<D, detail::NilFunction>::value ? 3
                                       : 4> Index;

  typedef typename std::tuple_element<Index::value-2, SelectionOptions>::type Selection;

  /// \brief Call to invoke the first matching function.
  /// \param data The data containing the arguments which are passed to the function.
  template <typename InvHelper> static void call(typename InvHelper::CallData data)
    {
    detail::FunctionSelectorCanCallHelper<InvHelper, Selection> helper(data);
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
template <typename A,
          typename B,
          typename C=detail::NilFunction,
          typename D=detail::NilFunction> class FunctionArgumentCountSelector
  {
public:
  typedef FunctionArgumentCountSelector<A, B, C, D> ThisType;

  typedef std::tuple<A, B> Selection2;
  typedef std::tuple<A, B, C> Selection3;
  typedef std::tuple<A, B, C, D> Selection4;

  typedef std::tuple<Selection2, Selection3, Selection4> SelectionOptions;

  typedef std::integral_constant<size_t, std::is_same<C, detail::NilFunction>::value ? 2
                                       : std::is_same<D, detail::NilFunction>::value ? 3
                                       : 4> Index;

  typedef typename std::tuple_element<Index::value-2, SelectionOptions>::type Selection;

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
