#pragma once
#include "Crate/CopyTraits.h"
#include "Crate/ReferenceTraits.h"
#include "Crate/ReferenceNonCleanedTraits.h"
#include "Crate/DerivedTraits.h"

namespace Crate
{

/// \brief Traits - can be specialised for certain classes, if required
template <typename T> class Traits : public CopyTraits<T>
  {
  };

}
