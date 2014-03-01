#pragma once
#include <stdexcept>
#include "Reflect/Type.h"

namespace Crate
{

class TypeException : public std::exception
  {
public:
  TypeException(const Reflect::Type *expected, const Reflect::Type *actual)
      : m_expected(expected), m_actual(actual)
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

private:
  std::string m_error;
  const Reflect::Type *m_expected;
  const Reflect::Type *m_actual;
  };

}
