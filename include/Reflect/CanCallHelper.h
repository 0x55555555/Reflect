#pragma once


namespace Reflect
{

namespace detail
{

template <typename InvHelper, typename Tuple> class CanCastTupleHelper
  {
public:
  CanCastTupleHelper(typename InvHelper::CallData &data)
      : m_data(data),
        m_result(true)
    {
    }

  template <std::size_t Idx> bool visit()
    {
    typedef typename std::tuple_element<Idx, Tuple>::type ElementType;

    m_result = InvHelper::template canUnpackArgument<Idx, ElementType>(m_data);
    return !m_result;
    }

  typename InvHelper::CallData &m_data;
  bool m_result;
  };

template <typename InvHelper> class CanCallHelper
  {
public:
  template <typename Args> static bool canCast(typename InvHelper::CallData data)
    {
    if (InvHelper::getArgumentCount(data) != std::tuple_size<Args>::value)
      {
      return false;
      }

    CanCastTupleHelper<InvHelper, Args> helper(data);
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
