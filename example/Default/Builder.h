#pragma once
#include "Crate/TypeException.h"
#include <vector>

namespace Reflect
{

namespace example
{

class Object
  {
public:
  union
    {
    float f;
    double db;
    int i;
    bool b;
    void *d;
    };

  const Reflect::Type *type;
  };

template <typename T> class PodCaster
  {
public:
  typedef T Result;

  static bool canCast(const Object *o)
    {
    return o->type == Reflect::findType<T>();
    }

  static T cast(Object *o)
    {
    if (!canCast(o))
      {
      throw Crate::TypeException(o->type, Reflect::findType<T>());
      }

    return *reinterpret_cast<T*>(&o->d);
    }

  static void pack(Object *o, T t)
    {
    o->type = Reflect::findType<T>();
    *reinterpret_cast<T*>(&o->d) = t;
    }
  };

template <typename T> class Caster;

template <typename T> class Caster<T *>
  {
public:
  typedef T *Result;

  static bool canCast(const Object *o)
    {
    return o && o->type == Reflect::findType<T>();
    }

  static T *cast(Object *o)
    {
    if (!canCast(o))
      {
      throw Crate::TypeException(Reflect::findType<T>(), o ? o->type : nullptr);
      }

    return static_cast<T *>(o->d);
    }

  static void pack(Object *o, T *t)
    {
    o->type = Reflect::findType<T>();
    o->d = t;
    }
  };

template <typename T> class Caster<const T> : public Caster<T> { };

template <> class Caster<void *> { };

template <> class Caster<float> : public PodCaster<float> { };
template <> class Caster<double> : public PodCaster<double> { };
template <> class Caster<int> : public PodCaster<int> { };
template <> class Caster<bool> : public PodCaster<bool> { };

template <> class Caster<const float &> : public PodCaster<float> { };
template <> class Caster<const double &> : public PodCaster<double> { };
template <> class Caster<const int &> : public PodCaster<int> { };
template <> class Caster<const bool &> : public PodCaster<bool> { };

template <typename A, typename B, typename C> static void initArgs(
    Object (&arr)[3],
    Object *(&ptr)[3],
    A a,
    B b,
    C c)
  {
  initArg(arr[0], ptr[0], a);
  initArg(arr[1], ptr[1], b);
  initArg(arr[2], ptr[2], c);
  }

template <typename A, typename B> static void initArgs(
    Object (&arr)[2],
    Object *(&ptr)[2],
    A a,
    B b)
  {
  initArg(arr[0], ptr[0], a);
  initArg(arr[1], ptr[1], b);
  }

template <typename A> static void initArgs(
    Object (&arr)[1],
    Object *(&ptr)[1],
    A a)
  {
  initArg(arr[0], ptr[0], a);
  }

template <typename T> static void initArg(
    Object &arr,
    Object *&ptr,
    T t)
  {
  Reflect::example::Caster<T>::pack(&arr, t);
  ptr = &arr;
  }

class Builder
  {
public:
  class Arguments
    {
  public:
    Object **args;
    std::size_t argCount;
    Object *ths;

    std::vector<Object> results;
    };
  typedef Arguments *CallData;

  template <typename T> static T getThis(CallData args)
    {
    try
      {
      return Caster<T>::cast(args->ths);
      }
    catch(const Crate::TypeException &type)
      {
      throw Crate::ThisException(type);
      }
    }

  template <std::size_t I, typename Arg>
      static typename Caster<Arg>::Result unpackArgument(CallData args)
    {
    try
      {
      return Caster<Arg>::cast(args->args[I]);
      }
    catch(const Crate::TypeException &type)
      {
      throw Crate::ArgException(type, I);
      }
    }

  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    data->results.push_back(Object());
    Object &b = data->results.back();

    Caster<Return>::pack(&b, result);
    }
  };

}

}
