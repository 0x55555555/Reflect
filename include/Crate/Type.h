#pragma once

#include <type_traits>
#include <string>
#include "CrateTypeHelper.h"

namespace Crate
{

class Type
  {
public:
  Type()
    {
    }

  ~Type()
    {
    }

  template <typename T> void initialise(const char *n, const Type *parent)
    {
    m_name = n;
    m_parent = parent;
    m_userData.initialise<T>();
    }

  const std::string &name() const
    {
    return m_name;
    }

  const Type *parent() const
    {
    return m_parent;
    }

  const Crate::TypeUserData& userData() const
    {
    return m_userData;
    }

private:
  std::string m_name;
  const Type *m_parent;
  Crate::TypeUserData m_userData;
  };

namespace detail
{
template <typename T> struct TypeResolver;

template <typename T> struct TypeResolver<const T> : public TypeResolver<T> { };
template <typename T> struct TypeResolver<T &> : public TypeResolver<T> { };
template <typename T> struct TypeResolver<T *> : public TypeResolver<T> { };
}

/// \brief Find a type for a specified class. This will fail to compile if
///        a TypeResolver is not defined for the type.
template <typename T> inline const Type *findType()
  {
  typedef detail::TypeResolver<T> Resolver;

  return Resolver::find();
  }

}
