#pragma once
#include "Reflect/Utils/Exceptions.h"

namespace Crate
{

class Type;

class TypeException : public Reflect::CallException
  {
public:
  TypeException(const Type *expected, const Type *actual);
  ~TypeException() throw();

  const char* what() const throw();

  const Type *expected() const;
  const Type *actual() const;

  bool operator==(const TypeException &e) const;

protected:
  std::string m_error;

private:
  const Type *m_expected;
  const Type *m_actual;
  };

class ThisException : public TypeException
  {
public:
  ThisException(const TypeException &t);
  ~ThisException() throw();
  };

class ArgException : public TypeException
  {
public:
  ArgException(const TypeException &t, std::size_t index);
  ~ArgException() throw();

  std::size_t index() const;

  bool operator==(const ArgException &e) const;

private:
  std::size_t m_index;
  };

}

#include "Crate/Exceptions.inl"
