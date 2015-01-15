#pragma once
#include "Crate/BaseTraits.h"

namespace Crate
{

/// \brief CopyTraits stores a copy of a class in memory owned by the receiver.
template <typename T> class EnumTraits : public BaseTraits<T, EnumTraits<T>>
  {
public:
  typedef std::integral_constant<size_t, sizeof(T)> TypeSize;
  typedef std::integral_constant<size_t, detail::alignment_of<T>::value> TypeAlignment;

  struct EnumWrap
    {
    typedef T DereferenceResult;
    EnumWrap(int t) : m_t((T)t)
      {
      }

    bool operator!() const
      {
      return false;
      }

    bool operator!=(std::nullptr_t) const
      {
      return true;
      }

    T operator*() const
      {
      return m_t;
      }

    operator T()
      {
      return m_t;
      }

    T m_t;
    };

  typedef BaseTraits<T, EnumTraits<T>> Base;
  typedef EnumWrap UnboxResult;

  template<typename Box, typename Data> static UnboxResult unbox(Box *ifc, Data data)
    {
    return UnboxResult(ifc->unboxEnum(data));
    }

  template<typename Box, typename Data> static void box(Box *ifc, Data data, const T *dataIn)
    {
    ifc->boxEnum(data, (int)*dataIn);
    }

  template<typename Box, typename Data> static bool canUnbox(Box *ifc, Data data)
    {
    return ifc->canUnboxEnum(data);
    }
  };

}
