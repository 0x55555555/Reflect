#pragma once
#include <type_traits>
#include "Crate/Exceptions.h"
#include "Crate/BaseTraits.h"

namespace Crate
{


template <typename T, typename Derived> template<typename Box> bool BaseTraits<T, Derived>::canUnbox(Box *ifc, typename Box::BoxedData data)
  {
  const auto neededType = getType();
  for (auto type = ifc->getType(data); type; type = type->parent())
    {
    if (type == neededType)
      {
      return true;
      }
    }
  return false;
  }

template <typename T, typename Derived> const Type *BaseTraits<T, Derived>::getType()
  {
  return findType<T>();
  }

template <typename T, typename Derived> template <typename Box> void BaseTraits<T, Derived>::checkUnboxable(Box *ifc, typename Box::BoxedData data)
  {
  if (!Derived::canUnbox(ifc, data))
    {
    throw TypeException(getType(), ifc->getType(data));
    }
  }


namespace detail
{

template<class T, bool Abstract=std::is_abstract<T>::value>
struct align_helper
{
  T _Elt0;
  char _Elt1;
  T _Elt2;

  align_helper();
  ~align_helper();
};

template<class T>
struct align_helper<T, true>
{
};

#define CRATE_ALIGN_HELPER(T) (sizeof (align_helper<T>) - 2 * sizeof (T))

  template<class T>
  struct alignment_of : std::integral_constant<size_t, CRATE_ALIGN_HELPER(typename std::remove_reference<T>::type)> { };

#undef CRATE_ALIGN_HELPER

}

}
