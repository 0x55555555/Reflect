#pragma once
#include "Crate/BaseTraits.h"

namespace Crate
{

/// \brief CopyTraits stores a copy of a class in memory owned by the receiver.
template <typename T> class CopyTraits : public BaseTraits<T, CopyTraits<T>>
  {
public:
  typedef std::integral_constant<size_t, sizeof(T)> TypeSize;
  typedef std::integral_constant<size_t, detail::alignment_of<T>::value> TypeAlignment;

  typedef BaseTraits<T, CopyTraits<T>> Base;

  template <typename Box> static T *getMemory(Box *ifc, typename Box::BoxedData data)
    {
    return static_cast<T *>(ifc->getMemory(data));
    }

  template<typename Box> static T *unbox(Box *ifc, typename Box::BoxedData data)
    {
    Base::checkUnboxable(ifc, data);

    return getMemory(ifc, data);
    }

  template <typename Box> static void cleanup(Box *ifc, typename Box::BoxedData data)
    {
    T *mem = unbox(ifc, data);
    (void)mem;
    mem->~T();
    }

  template<typename Box> static void box(Box *ifc, typename Box::BoxedData data, const T *dataIn)
    {
    if (ifc->initialise<CopyTraits<T>, T, cleanup<Box>>(data, Base::getType(), dataIn) == AlreadyInitialised)
      {
      return;
      }

    T *memory = getMemory(ifc, data);
    new(memory) T(*dataIn);
    }
  };

}
