#pragma once
#include "Crate/Exceptions.h"
#include "Crate/Type.h"
#include "Crate/EmbeddedTypes.h"

namespace Crate
{

inline TypeException::TypeException(const Type *expected, const Type *actual)
    : m_expected(expected ? expected : findType<void>()),
      m_actual(actual ? actual : findType<void>())
  {
  m_error = "Expected '" + m_expected->name() + "' got '" + m_actual->name() + "'";
  }

inline TypeException::~TypeException() throw()
  {
  }

inline const char* TypeException::what() const throw()
  {
  return m_error.c_str();
  }

inline const Type *TypeException::expected() const
  { 
  return m_expected; 
  }

inline const Type *TypeException::actual() const
  {
  return m_actual; 
  }

inline bool TypeException::operator==(const TypeException &e) const
  {
  return m_expected == e.m_expected && m_actual == e.m_actual;
  }

inline ThisException::ThisException(const TypeException &t)
  : TypeException(t)
{
  m_error = "Expected '" + expected()->name() + "' got '" + actual()->name() + "' for 'this' parameter.";
}

inline ThisException::~ThisException() throw()
  {
  }

inline ArgException::ArgException(const TypeException &t, std::size_t index)
    : TypeException(t),
      m_index(index)
  {
  m_error = "Expected '" + expected()->name() + "' got '" + actual()->name() + "' for argument " + std::to_string((uint64_t)index) + " parameter.";
  }

inline ArgException::~ArgException() throw()
  {
  }

inline std::size_t ArgException::index() const
  { 
  return m_index;
  }

inline bool ArgException::operator==(const ArgException &e) const
  {
  return TypeException::operator==(e) && m_index == e.m_index;
  }

}
