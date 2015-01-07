#pragma once
#include "Crate/BaseTraits.h"
#include <type_traits>

namespace Crate
{

template <typename Root, typename Derived> class CastHelper;

template <typename T> class Traits;

/// \brief CopyTraits stores a copy of a class in memory owned by the receiver.
template <typename T, typename Parent, typename Root> class DerivedTraits : public BaseTraits<T, DerivedTraits<T, Parent, Root>>
  {
public:
  typedef std::integral_constant<size_t, sizeof(T)> TypeSize;
  typedef std::integral_constant<size_t, detail::alignment_of<T>::value> TypeAlignment;

  typedef BaseTraits<T, DerivedTraits<T, Parent, Root>> BaseClassTraits;
  typedef Traits<Root> RootTraits;

  template<typename Box, typename Data> static bool canUnbox(Box *ifc, Data data)
    {
    return BaseClassTraits::canUnbox(ifc, data);
    }

  template<typename Box, typename Data> static T *unbox(Box *ifc, Data data)
    {
    RootTraits::checkUnboxable(ifc, data);

    return static_cast<T*>(RootTraits::unbox(ifc, data));
    }

  template <typename Box> static void cleanup(Box *ifc, typename Box::BoxedData data)
    {
    RootTraits::cleanup(ifc, data);
    }
  
  template<typename Box, typename Boxable> static void box(Box *ifc, Boxable data, T *dataIn)
    {
    return RootTraits::box(ifc, data, dataIn);
    }
  };

}
