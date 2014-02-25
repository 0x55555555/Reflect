#pragma once

#include <type_traits>

namespace Reflect
{

class Type
  {
  };

namespace detail
{
template <typename T> struct TypeResolver;
}

/// \brief Find a type for a specified class. This will fail to compile if
///        a TypeResolver is not defined for the type.
template <typename T> inline const Type *findType()
  {
  typedef typename std::remove_reference<T>::type NonReferenced;
  typedef typename std::remove_const<NonReferenced>::type NonConst;
  typedef typename std::remove_pointer<NonConst>::type NonPointer;

  typedef detail::TypeResolver<NonPointer> Resolver;

  return Resolver::find();
  }

}
