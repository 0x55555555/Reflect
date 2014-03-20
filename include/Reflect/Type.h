#pragma once

#include <type_traits>
#include <string>

namespace Reflect
{

class Type
  {
public:
  Type(const char *n) : m_name(n)
    {
    }

  ~Type()
    {
    }

  const std::string &name() const
    {
    return m_name;
    }

private:
  std::string m_name;
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
