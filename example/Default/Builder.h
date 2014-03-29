#pragma once
#include "Crate/TypeException.h"
#include "Crate/Traits.h"
#include <vector>
#include <cassert>

namespace Reflect
{

namespace example
{

class Boxer;

class Object
  {
public:
  union
    {
    float f;
    double db;
    int i;
    bool b;
    uint8_t *d;
    };

  typedef void (*Cleanup)(Boxer *, Object *);

  Boxer *boxer;
  Cleanup cleanup;
  const Reflect::Type *type;

  Object()
    {
    d = nullptr;
    boxer = nullptr;
    cleanup = nullptr;
    type = nullptr;
    }

  void init(size_t s)
    {
    assert(!d);
    assert(!type);
    assert(!boxer);
    assert(!cleanup);
    d = new uint8_t[s];
    }

  ~Object()
    {
    if (cleanup)
      {
      cleanup(boxer, this);
      delete [] d;
      }
    }
  };

class Boxer
  {
public:
  typedef Object *BoxedData;

  const Reflect::Type *getType(Object *o) const
    {
    return o->type;
    }

  void *getMemory(Object *o)
    {
    return o->d;
    }

  const void *getMemory(Object *o) const
    {
    return o->d;
    }

  void initialise(Object *o, const Type *t, Object::Cleanup c)
    {
    o->type = t;
    o->boxer = this;
    o->cleanup = c;
    }

  template <typename Traits> std::unique_ptr<Object> create()
    {
    auto ptr = std::unique_ptr<Object>(new Object());
    setupBox<Traits>(ptr.get());
    return std::move(ptr);
    }

  template <typename Traits> void setupBox(Object *o)
    {
    typedef typename Traits::TypeSize Size;
    typedef typename Traits::TypeAlignment Alignment;
    typedef std::aligned_storage<Size::value, Alignment::value> Aligned;
    o->init(sizeof(typename Aligned::type));
    }
  };

template <typename T> class PodCaster
  {
public:
  typedef T Result;

  static bool canCast(Boxer *, const Object *o)
    {
    return o->type == Reflect::findType<T>();
    }

  static T cast(Boxer *b, Object *o)
    {
    if (!canCast(b, o))
      {
      throw Crate::TypeException(o->type, Reflect::findType<T>());
      }

    return *reinterpret_cast<T*>(&o->d);
    }

  static void pack(Boxer *, Object *o, T t)
    {
    o->type = Reflect::findType<T>();
    *reinterpret_cast<T*>(&o->d) = t;
    }
  };

template <typename T> class Caster : Caster<T *>
  {
public:
  typedef T &Result;
  typedef Crate::Traits<T> ClassTraits;

  static bool canCast(Boxer *boxer, const Object *o)
    {
    return Caster<T *>::canCast(boxer, o) && ClassTraits::unbox(boxer, o) != nullptr;
    }

  static T &cast(Boxer *b, Object *o)
    {
    T *data = Caster<T *>::cast(b, o);
    if (!data)
      {
      throw Crate::TypeException(Reflect::findType<T>(), nullptr);
      }

    return *data;
    }

  static void pack(Boxer *b, Object *o, T &t)
    {
    b->setupBox<ClassTraits>(o);
    ClassTraits::box(b, o, &t);
    }
  };

template <typename T> class Caster<T *>
  {
public:
  typedef T *Result;
  typedef Crate::Traits<T> ClassTraits;

  static bool canCast(Boxer *boxer, const Object *o)
    {
    return o && ClassTraits::canUnbox(boxer, o);
    }

  static T *cast(Boxer *b, Object *o)
    {
    return ClassTraits::unbox(b, o);
    }

  static void pack(Boxer *b, Object *o, T *t)
    {
    b->setupBox<ClassTraits>(o);
    ClassTraits::box(b, o, t);
    }
  };

template <typename T> class Caster<T &> : public Caster<T> { };
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
    Boxer *boxer,
    Object (&arr)[3],
    Object *(&ptr)[3],
    A a,
    B b,
    C c)
  {
  initArg(boxer, arr[0], ptr[0], a);
  initArg(boxer, arr[1], ptr[1], b);
  initArg(boxer, arr[2], ptr[2], c);
  }

template <typename A, typename B> static void initArgs(
    Boxer *boxer,
    Object (&arr)[2],
    Object *(&ptr)[2],
    A a,
    B b)
  {
  initArg(boxer, arr[0], ptr[0], a);
  initArg(boxer, arr[1], ptr[1], b);
  }

template <typename A> static void initArgs(
    Boxer *boxer,
    Object (&arr)[1],
    Object *(&ptr)[1],
    A a)
  {
  initArg(boxer, arr[0], ptr[0], a);
  }

template <typename T> static void initArg(
    Boxer *boxer,
    Object &arr,
    Object *&ptr,
    T t)
  {
  Reflect::example::Caster<T>::pack(boxer, &arr, t);
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

    Boxer *boxer;

    std::vector<Object> results;
    };
  typedef Arguments *CallData;

  template <typename T> static T getThis(CallData args)
    {
    try
      {
      return Caster<T>::cast(args->boxer, args->ths);
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
      return Caster<Arg>::cast(args->boxer, args->args[I]);
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

    Caster<Return>::pack(data->boxer, &b, result);
    }
  };

}

}
