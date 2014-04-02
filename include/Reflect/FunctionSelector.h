#pragma once

namespace Reflect
{
namespace detail
{

template <typename InvHelper, typename Functions> class FunctionSelectorCanCallHelper
  {
public:
  FunctionSelectorCanCallHelper(typename InvHelper::CallData &data)
      : m_data(data),
        m_foundCall(false)
    {
    }

  template <std::size_t Idx> bool visit()
    {
    typedef typename std::tuple_element<Idx, Functions>::type Function;

    if (Function::template canCall<InvHelper>(m_data))
      {
      m_foundCall = true;
      return true;
      }

    return false;
    }

  typename InvHelper::CallData &m_data;
  bool m_foundCall;
  };

template <typename InvHelper, typename Functions> class FunctionSelectorHelper
  {
  FunctionSelectorHelper(typename InvHelper::CallData &data)
      : m_data(data),
        m_foundCall(false)
    {
    }

  template <std::size_t Idx> bool visit()
    {
    typedef typename std::tuple_element<Idx, Functions>::type Function;

    if (Function::call(m_data))
      {
      m_foundCall = true;
      return true;
      }
    }

  typename InvHelper::CallData &m_data;
  bool m_foundCall;
  };

}
}

#ifdef REFLECT_MACRO_IMPL
# include "Reflect/Macro/FunctionSelectorImpl.h"
#else
# include "Reflect/Cpp11/FunctionSelectorImpl.h"
#endif

