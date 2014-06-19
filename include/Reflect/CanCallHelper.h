#pragma once


namespace Reflect
{

namespace detail
{

template <typename InvHelper, typename Tuple> class CanCastTupleHelper
  {
public:
  CanCastTupleHelper(typename InvHelper::CallData &data, bool isStatic)
      : m_data(data),
        m_static(isStatic),
        m_result(true)
    {
    }

  template <std::size_t Idx> bool visit()
    {
    typedef typename std::tuple_element<Idx, Tuple>::type ElementType;

    m_result = InvHelper::template canUnpackArgument<Idx, ElementType>(m_data, !m_static);
    return !m_result;
    }

  typename InvHelper::CallData &m_data;
  bool m_static;
  bool m_result;
  };

template <typename InvHelper> class CanCallHelper
  {
public:
  template <typename Args, bool Static> static bool canCast(typename InvHelper::CallData data)
    {
    size_t expectedArgCount = std::tuple_size<Args>::value + (Static ? 0 : 1);
    if (InvHelper::getArgumentCountWithThis(data) != expectedArgCount)
      {
      return false;
      }

    CanCastTupleHelper<InvHelper, Args> helper(data, Static);
    tupleEach<Args>(helper);
    return helper.m_result;
    }

  template <typename Type> static bool canCastThis(typename InvHelper::CallData data)
    {
    return InvHelper::template canUnpackThis<Type>(data);
    }
  };

}
}
