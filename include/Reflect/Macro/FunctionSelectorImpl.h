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
          typename B=detail::NilFunction,
          typename C=detail::NilFunction,
          typename D=detail::NilFunction,
          typename E=detail::NilFunction,
          typename F=detail::NilFunction,
          typename G=detail::NilFunction,
          typename H=detail::NilFunction> class FunctionArgumentTypeSelector
  {
public:
  typedef FunctionArgumentTypeSelector<A, B, C, D, E, F, G, H> ThisType;

  typedef std::tuple<A> Selection1;
  typedef std::tuple<A, B> Selection2;
  typedef std::tuple<A, B, C> Selection3;
  typedef std::tuple<A, B, C, D> Selection4;
  typedef std::tuple<A, B, C, D, E> Selection5;
  typedef std::tuple<A, B, C, D, E, F> Selection6;
  typedef std::tuple<A, B, C, D, E, F, G> Selection7;
  typedef std::tuple<A, B, C, D, E, F, G, H> Selection8;

  typedef std::tuple<Selection1, Selection2, Selection3, Selection4, Selection5, Selection6, Selection7, Selection8> SelectionOptions;

  typedef std::integral_constant<size_t, std::is_same<B, detail::NilFunction>::value ? 1
                                       : std::is_same<C, detail::NilFunction>::value ? 2
                                       : std::is_same<D, detail::NilFunction>::value ? 3
                                       : std::is_same<E, detail::NilFunction>::value ? 4
                                       : std::is_same<F, detail::NilFunction>::value ? 5
                                       : std::is_same<G, detail::NilFunction>::value ? 6
                                       : std::is_same<H, detail::NilFunction>::value ? 7
                                       : 8> Index;

  typedef typename std::tuple_element<Index::value-1, SelectionOptions>::type Selection;

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
template <typename A,
          typename B,
          typename C=detail::NilFunction,
          typename D=detail::NilFunction,
          typename E=detail::NilFunction,
          typename F=detail::NilFunction,
          typename G=detail::NilFunction,
          typename H=detail::NilFunction> class FunctionArgumentCountSelector
  {
public:
  typedef FunctionArgumentCountSelector<A, B, C, D> ThisType;

  typedef std::tuple<A, B> Selection2;
  typedef std::tuple<A, B, C> Selection3;
  typedef std::tuple<A, B, C, D> Selection4;
  typedef std::tuple<A, B, C, D, E> Selection5;
  typedef std::tuple<A, B, C, D, E, F> Selection6;
  typedef std::tuple<A, B, C, D, E, F, G> Selection7;
  typedef std::tuple<A, B, C, D, E, F, G, H> Selection8;

  typedef std::tuple<Selection2, Selection3, Selection4, Selection5, Selection6, Selection7, Selection8> SelectionOptions;

  typedef std::integral_constant<size_t, std::is_same<C, detail::NilFunction>::value ? 2
                                       : std::is_same<D, detail::NilFunction>::value ? 3
                                       : std::is_same<E, detail::NilFunction>::value ? 4
                                       : std::is_same<F, detail::NilFunction>::value ? 5
                                       : std::is_same<G, detail::NilFunction>::value ? 6
                                       : std::is_same<H, detail::NilFunction>::value ? 7
                                       : 8> Index;

  typedef typename std::tuple_element<Index::value-2, SelectionOptions>::type Selection;

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
