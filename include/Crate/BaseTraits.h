#pragma once
#include <type_traits>
#include "Crate/Exceptions.h"

namespace Crate
{

/// \brief The Boxing interface allows interaction with memory from the receiver.
/// This is the list of expected functions - it should be implemented in the receiver.
/*
class BoxingInterface
  {
public:
  typedef void *BoxedData;

  typedef void (*Cleanup)(BoxingInterface *, BoxedData);

  void initialise(BoxedData, const Type *, Cleanup);

  const Type *getType(BoxedData);

  void* getMemory(BoxedData);
  const void* getMemory(BoxedData) const;
  };*/

/// \brief Base traits provides traits common to all Class Traits
template <typename T, typename Derived> class BaseTraits
  {
public:
  template<typename Box> static bool canUnbox(Box *ifc, typename Box::BoxedData data)
    {
    auto type = ifc->getType(data);
    return type == getType();
    }

  static const Type *getType()
    {
    return findType<T>();
    }

  template <typename Box> static void cleanup(Box *ifc, typename Box::BoxedData data)
    {
    T *mem = Derived::unbox(ifc, data);
    mem->~T();
    }

  template <typename Box> static void checkUnboxable(Box *ifc, typename Box::BoxedData data)
    {
    if (!Derived::canUnbox(ifc, data))
      {
      throw TypeException(getType(), ifc->getType(data));
      }
    }

  };

}
