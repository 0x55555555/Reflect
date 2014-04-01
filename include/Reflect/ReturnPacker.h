#pragma once
#include <tuple>
#include "TupleEach.h"

namespace Reflect
{
namespace detail
{

template <typename T, typename InvHelper> class ReturnPacker
  {
public:
  static void pack(typename InvHelper::CallData data, T &&result)
    {
    InvHelper::template packReturn<T>(data, std::move(result));
    }
  };


template <typename Tuple, typename InvHelper> class TuplePackerHelper
  {
public:
  TuplePackerHelper(Tuple& tup, typename InvHelper::CallData &d)
      : m_tuple(tup),
        m_data(d)
    {
    }

  template <std::size_t I> void visit()
    {
    typedef typename std::tuple_element<I, Tuple>::type ElementType;

    ReturnPacker<ElementType, InvHelper>::pack(m_data, std::move(std::get<I>(m_tuple)));
    }

private:
  Tuple &m_tuple;
  typename InvHelper::CallData &m_data;
  };

#if defined(_MSC_VER) && _MSC_VER < 1800

template <typename InvHelper, typename A, typename B, typename C, typename D> class ReturnPacker<std::tuple<A, B, C, D>, InvHelper>
  {
public:
  typedef std::tuple<A, B, C, D> Tuple;

  static void pack(typename InvHelper::CallData data, Tuple &&result)
    {
    tupleEach<TuplePackerHelper>(TuplePackerHelper<Tuple, InvHelper>(&result, data));
    }
  };

#else

template <typename InvHelper, typename... Args> class ReturnPacker<std::tuple<Args...>, InvHelper>
  {
public:
  typedef std::tuple<Args...> Tuple;
  static void pack(typename InvHelper::CallData data, std::tuple<Args...> &&result)
    {
    TuplePackerHelper<Tuple, InvHelper> helper(result, data);
    tupleEach<Tuple>(helper);
    }
  };

#endif


}
}
