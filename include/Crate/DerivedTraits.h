#pragma once
#include "Crate/BaseTraits.h"

namespace Crate
{

template <typename Root, typename Derived> class CastHelper;

template <typename T> class Traits;

/// \brief CopyTraits stores a copy of a class in memory owned by the receiver.
template <typename T, typename Parent, typename Root> class DerivedTraits : public BaseTraits<T, DerivedTraits<T, Parent, Root>>
  {
public:
  typedef std::integral_constant<size_t, sizeof(T)> TypeSize;
  typedef std::integral_constant<size_t, std::alignment_of<T>::value> TypeAlignment;

  typedef BaseTraits<T, DerivedTraits<T, Parent, Root>> BaseTraits;
  typedef Traits<Root> RootTraits;

  template<typename Box> static bool canUnbox(Box *ifc, typename Box::BoxedData data)
    {
    return RootTraits::canUnbox(ifc, data) && CastHelper<Root, T>::canCast(ifc, data);
    }

  template<typename Box> static T *unbox(Box *ifc, typename Box::BoxedData data)
    {
    RootTraits::checkUnboxable(ifc, data);

    return static_cast<T*>(*RootTraits::getMemory(ifc, data));
    }

  template<typename Box> static void box(Box *ifc, typename Box::BoxedData data, T *dataIn)
    {
    ifc->initialise(data, RootTraits::getType(), BaseTraits::template cleanup<Box>);

    auto memory = RootTraits::getMemory(ifc, data);
    *memory = dataIn;
    }
  };

}
