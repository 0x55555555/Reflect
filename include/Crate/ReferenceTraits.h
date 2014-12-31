#pragma once
#include "Crate/BaseTraits.h"

namespace Crate
{

/// \brief ReferenceTraits stores a pointer of a class in memory owned by the receiver.
template <typename T> class ReferenceTraits : public BaseTraits<T, ReferenceTraits<T>>
  {
  public:
  typedef std::integral_constant<bool, true> Managed;

  typedef std::integral_constant<size_t, sizeof(T*)> TypeSize;
  typedef std::integral_constant<size_t, std::alignment_of<T*>::value> TypeAlignment;

  typedef BaseTraits<T, ReferenceTraits<T>> Base;

  template <typename Box, typename Data> static T **getMemory(Box *ifc, Data data)
    {
    return static_cast<T **>(ifc->getMemory(data));
    }

  template<typename Box> static T *unbox(Box *ifc, typename Box::BoxedData data)
    {
    Base::checkUnboxable(ifc, data);

    return *getMemory(ifc, data);
    }

  template <typename Box> static void cleanup(Box *ifc, typename Box::BoxedData data)
    {
    T *mem = unbox(ifc, data);
    delete mem;
    }
  
  template<typename Box, typename Boxable> static void box(Box *ifc, Boxable data, T *dataIn)
    {
    if (ifc->template initialise<ReferenceTraits<T>, T>(data, Base::getType(), dataIn, cleanup<Box>) == Base::AlreadyInitialised)
      {
      return;
      }

    T **memory = getMemory(ifc, data);
    *memory = dataIn;
    }

  static const void *makeObjectKey(const T *t)
    {
    return t;
    }
  };

}
