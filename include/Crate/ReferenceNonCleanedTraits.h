#pragma once
#include "Crate/BaseTraits.h"

namespace Crate
{

/// \brief ReferenceTraits stores a pointer of a class in memory owned by the receiver,
///        it will not delete the pointer when unreferenced.
template <typename T> class ReferenceNonCleanedTraits : public BaseTraits<T, ReferenceNonCleanedTraits<T>>
  {
public:
  typedef std::integral_constant<bool, true> Managed;

  typedef std::integral_constant<size_t, sizeof(T*)> TypeSize;
  typedef std::integral_constant<size_t, std::alignment_of<T*>::value> TypeAlignment;

  typedef BaseTraits<T, ReferenceNonCleanedTraits<T>> Base;

  template <typename Box, typename Data> static T **getMemory(Box *ifc, Data data)
    {
    return static_cast<T **>(ifc->getMemory(data));
    }

  template<typename Box, typename Data> static T *unbox(Box *ifc, Data data)
    {
    Base::checkUnboxable(ifc, data);

    return *getMemory(ifc, data);
    }

  template <typename Box> static void cleanup(Box *, typename Box::BoxedData)
    {
    }

  template<typename Box, typename Boxable> static void box(Box *ifc, Boxable data, T *dataIn)
    {
    if (ifc->template initialise<ReferenceNonCleanedTraits<T>, T>(data, Base::getType(), dataIn, cleanup<Box>) == Base::AlreadyInitialised)
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
