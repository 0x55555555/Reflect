#pragma once
#include "Crate/CopyTraits.h"
#include "Crate/ReferenceTraits.h"
#include "Crate/ReferenceNonCleanedTraits.h"
#include "Crate/DerivedTraits.h"
#include "Crate/SmartPointerTraits.h"
#include "Crate/EnumTraits.h"

namespace Crate
{

/// \brief Traits - can be specialised for certain classes, if required
template <typename T> class Traits : public CopyTraits<T>
  {
  };


template <> class Traits<void> : public BaseTraits<void, Traits<void>>
  {
public:
  template <typename Box> static void **getMemory(Box *, typename Box::BoxedData)
    {
    return nullptr;
    }

  template<typename Box> static void *unbox(Box *, typename Box::BoxedData)
    {
    return nullptr;
    }

  template<typename Box> static void box(Box *, typename Box::BoxedData, void *)
    {
    }

  template <typename Box> static void cleanup(Box *, typename Box::BoxedData)
    {
    }
  };

}
