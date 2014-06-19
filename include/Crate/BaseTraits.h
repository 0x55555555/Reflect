#pragma once
#include <type_traits>

namespace Crate
{

class Type;

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
  enum InitialiseTypes
  {
    AlreadyInitialised,
    Initialised
  };
  
  typedef std::integral_constant<bool, false> Managed;

  template<typename Box> static bool canUnbox(Box *ifc, typename Box::BoxedData data);

  static const Type *getType();
  
  template <typename Box> static void checkUnboxable(Box *ifc, typename Box::BoxedData data);
  };

}

#include "Crate/BaseTraits.inl"
