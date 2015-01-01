#pragma once
#include "Crate/BaseTraits.h"
#include "assert.h"

namespace Crate
{

/// \brief CopyTraitsBases stores a copy of a class in memory owned by the receiver.
template <typename T, typename Derived> class CopyTraitsBase : public BaseTraits<T, Derived>
  {
public:
  typedef std::integral_constant<size_t, sizeof(T)> TypeSize;
  typedef std::integral_constant<size_t, detail::alignment_of<T>::value < 4 ? 4 : detail::alignment_of<T>::value> TypeAlignment;

  typedef BaseTraits<T, Derived> Base;

  template <typename Box, typename Data> static T *getMemory(Box *ifc, Data data)
    {
    auto mem = ifc->getMemory(data);

    size_t memS = (size_t)mem;
    auto diff = memS % TypeAlignment::value;
    memS += TypeAlignment::value - diff;

    return static_cast<T *>((void*)memS);
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

  template<typename Box, typename Boxable> static void box(Box *ifc, Boxable data, T *dataIn)
    {
    if (ifc->template initialise<Derived, T>(data, Base::getType(), dataIn, cleanup<Box>) == Base::AlreadyInitialised)
      {
      return;
      }

    T *memory = getMemory(ifc, data);
    new(memory) T(std::move(*dataIn));
    }

  template<typename Box, typename Boxable> static void box(Box *ifc, Boxable data, const T *dataIn)
    {
    if (ifc->template initialise<Derived, T>(data, Base::getType(), dataIn, cleanup<Box>) == Base::AlreadyInitialised)
      {
      return;
      }

    T *memory = getMemory(ifc, data);
    new(memory) T(*dataIn);
    }

  static const void *makeObjectKey(const T *)
    {
    assert(false);
    return nullptr;
    }
  };

template <typename T> class CopyTraits : public CopyTraitsBase<T, CopyTraits<T>>
  {
  };

}
