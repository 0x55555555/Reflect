#pragma once
#include "Crate/CopyTraits.h"
#include <memory>

namespace Crate
{

/// \brief SmartPointerTraits stores a copy of a smart pointer class in memory owned by the receiver.
template <typename T> class SmartPointerTraits : public CopyTraitsBase<T, SmartPointerTraits<T>>
  {
public:
  typedef std::integral_constant<bool, true> Managed;
  typedef T Pointer;
  typedef CopyTraitsBase<T, SmartPointerTraits<T>> Base;

  static const void *makeObjectKey(const T *t)
    {
    return t->get();
    }
  };

/// \brief SmartPointerTraits stores a copy of a smart pointer class in memory owned by the receiver.
template <typename T> class WeakSmartPointerTraits : public CopyTraitsBase<T, WeakSmartPointerTraits<T>>
  {
public:
  typedef std::integral_constant<bool, true> Managed;
  typedef T Pointer;
  typedef CopyTraitsBase<T, WeakSmartPointerTraits<T>> Base;

  static const void *makeObjectKey(const T *t)
    {
    return t->lock().get();
    }
  };

/// \brief CopyTraits SmartPointerObjectTraits a copy of a class in memory owned by the receiver.
template <typename T, typename SmartPointerTraits> class SmartPointerObjectTraitsBase
  {
public:
  struct UnboxResult
    {
    typename SmartPointerTraits::Pointer *t;
    UnboxResult(typename SmartPointerTraits::Pointer *val) : t(val)
      {
      REFLECT_ASSERT(t->get());
      }

    operator T*()
      {
      return t->get();
      }
    };

  template<typename Box, typename Data> static bool canUnbox(Box *ifc, Data data)
    {
    return SmartPointerTraits::canUnbox(ifc, data);
    }

  template<typename Box, typename Data> static UnboxResult unbox(Box *ifc, Data data)
    {
    auto smartPointer = SmartPointerTraits::unbox(ifc, data);

    if (!*smartPointer)
      {
      throw Crate::TypeException(Crate::findType<T>(), Crate::findType<void>());
      }

    return UnboxResult{smartPointer};
    }

  template <typename Box> static void cleanup(Box *ifc, typename Box::BoxedData data)
    {
    // This is called because the smart pointer and this type share a type, we just forward on.
    // There is no box(...) method in this class - so these traits will never box themselves.
    SmartPointerTraits::cleanup(ifc, data);
    }

  static const void *makeObjectKey(const T *t)
    {
    return SmartPointerTraits::makeObjectKey(t);
    }
  };

template <typename T, typename SmartPointerTraits> class SmartPointerObjectTraits : public SmartPointerObjectTraitsBase<T, SmartPointerTraits>
  {
public:
  };

template <typename T, typename SmartPointerTraits> class WeakSmartPointerObjectTraits : public SmartPointerObjectTraitsBase<T, SmartPointerTraits>
  {
public:
  struct UnboxResult
    {
    std::shared_ptr<T> t;

    operator T*()
      {
      return t.get();
      }
    };

  template<typename Box> static UnboxResult unbox(Box *ifc, typename Box::BoxedData data)
    {
    auto smartPointer = SmartPointerTraits::unbox(ifc, data);

    auto locked = smartPointer->lock();
    if (!locked)
      {
      throw Crate::TypeException(Crate::findType<T>(), Crate::findType<void>());
      }

    return UnboxResult{locked};
    }
  };

}
