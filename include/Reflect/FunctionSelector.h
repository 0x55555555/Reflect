#pragma once

namespace Reflect
{
namespace detail
{

/// \brief Helper to find a function which can be called
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

/// \brief Helper to find a function which can be called, and call it if found
template <typename InvHelper, typename Functions> class FunctionSelectorHelper
  {
public:
  FunctionSelectorHelper(typename InvHelper::CallData &data)
      : m_data(data),
        m_foundCall(false)
    {
    }

  template <std::size_t Idx> bool visit()
    {
    typedef typename std::tuple_element<Idx, Functions>::type Function;

    if (Function::template canCall<InvHelper>(m_data))
      {
      Function::template call<InvHelper>(m_data);
      m_foundCall = true;
      return true;
      }
    return false;
    }

  typename InvHelper::CallData &m_data;
  bool m_foundCall;
  };

/// \brief Helper to find a function with the correct arg count
template <typename InvHelper, typename Functions> class FunctionArgCountSelectorCanCallHelper
  {
public:
  FunctionArgCountSelectorCanCallHelper(std::size_t argCount)
      : m_argCount(argCount),
        m_foundCall(false)
    {
    }

  template <std::size_t Idx> bool visit()
    {
    typedef typename std::tuple_element<Idx, Functions>::type Block;

    typedef typename Block::Count Count;
    if (m_argCount == Count::value)
      {
      m_foundCall = true;
      return true;
      }
    return false;
    }

  std::size_t m_argCount;
  bool m_foundCall;
  };

/// \brief Helper to find a function with the correct arg count, and call it if possible
template <typename InvHelper, typename Functions> class FunctionArgCountSelectorHelper
  {
public:
  FunctionArgCountSelectorHelper(std::size_t argCount, typename InvHelper::CallData &data)
      : m_data(data),
        m_argCount(argCount),
        m_foundCall(false)
    {
    }

  template <std::size_t Idx> bool visit()
    {
    typedef typename std::tuple_element<Idx, Functions>::type Block;

    typedef typename Block::Count Count;
    if (m_argCount == Count::value)
      {
      typedef typename Block::Function Function;
      Function::template call<InvHelper>(m_data);

      m_foundCall = true;
      return true;
      }
    return false;
    }

  typename InvHelper::CallData &m_data;
  std::size_t m_argCount;
  bool m_foundCall;
  };

}

/// \brief Helper block for argument count selection
template <std::size_t _Count, typename _Fn> struct FunctionArgCountSelectorBlock
  {
  typedef std::integral_constant<std::size_t, _Count> Count;
  typedef _Fn Function;
  typedef typename Function::Helper Helper;
  };

}

#ifdef REFLECT_MACRO_IMPL
# include "Reflect/Macro/FunctionSelectorImpl.h"
#else
# include "Reflect/Cpp11/FunctionSelectorImpl.h"
#endif

