#pragma once
#include "Reflect/Exceptions.h"
#include "Reflect/Type.h"
#include "Reflect/EmbeddedTypes.h"

namespace Crate
{

class TypeException : public Reflect::CallException
  {
public:
  TypeException(const Reflect::Type *expected, const Reflect::Type *actual)
      : m_expected(expected ? expected : Reflect::findType<void>()),
        m_actual(actual ? actual : Reflect::findType<void>())
    {
    m_error = "Expected '" + m_expected->name() + "' got '" + m_actual->name() + "'";
    }

  ~TypeException() throw()
    {
    }

  const char* what() const throw()
    {
    return m_error.c_str();
    }

  const Reflect::Type *expected() const { return m_expected; }
  const Reflect::Type *actual() const { return m_actual; }

  bool operator==(const TypeException &e) const
    {
    return m_expected == e.m_expected && m_actual == e.m_actual;
    }

protected:
  std::string m_error;

private:
  const Reflect::Type *m_expected;
  const Reflect::Type *m_actual;
  };

class ThisException : public TypeException
  {
public:
  ThisException(const TypeException &t)
      : TypeException(t)
    {
    m_error = "Expected '" + expected()->name() + "' got '" + actual()->name() + "' for 'this' parameter.";
    }

  ~ThisException() throw()
    {
    }
  };

class ArgException : public TypeException
  {
public:
  ArgException(const TypeException &t, std::size_t index)
      : TypeException(t),
        m_index(index)
    {
    m_error = "Expected '" + expected()->name() + "' got '" + actual()->name() + "' for argument " + std::to_string(index) + " parameter.";
    }

  ~ArgException() throw()
    {
    }

  std::size_t index() const { return m_index; }

  bool operator==(const ArgException &e) const
    {
    return TypeException::operator==(e) && m_index == e.m_index;
    }

private:
  std::size_t m_index;
  };

}
