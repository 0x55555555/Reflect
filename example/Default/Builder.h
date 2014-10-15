#pragma once
#include "Crate/Exceptions.h"
#include "Crate/Traits.h"
#include <vector>
#include <memory>
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
  const Crate::Type *type;

  Object()
    {
    d = nullptr;
    boxer = nullptr;
    cleanup = nullptr;
    type = nullptr;
    }

  Object(Object&& o)
    {
    d = o.d;
    o.d = nullptr;
    boxer = o.boxer;
    o.boxer = nullptr;
    cleanup = o.cleanup;
    o.cleanup = nullptr;
    type = o.type;
    o.type = nullptr;
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
      d = nullptr;
      cleanup = nullptr;
      }
    }

private:
  Object(const Object &);
  Object& operator=(const Object &);
  };

class Boxer
  {
public:
  typedef Object *BoxedData;

  const Crate::Type *getType(Object *o) const
    {
    return o ? o->type : Crate::findType<void>();
    }

  void *getMemory(Object *o)
    {
    return o->d;
    }

  const void *getMemory(Object *o) const
    {
    return o->d;
    }

  template <typename Traits, typename T> typename Traits::InitialiseTypes initialise(Object *o, const Crate::Type *t, const T *, Object::Cleanup cln)
    {
    assert(!o->type);
    assert(!o->cleanup);
    assert(!o->boxer);
    o->type = t;
    o->boxer = this;
    o->cleanup = cln;

    return Traits::Initialised;
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
    return o->type == Crate::findType<T>();
    }

  static T cast(Boxer *b, Object *o)
    {
    if (!canCast(b, o))
      {
      throw Crate::TypeException(o->type, Crate::findType<T>());
      }

    union
    {
      void *in;
      T *out;
    } conv;
    conv.in = &o->d;

    return *conv.out;
    }

  static void pack(Boxer *, Object *o, T t)
    {
    union
    {
      void *in;
      T *out;
    } conv;

    o->type = Crate::findType<T>();
    conv.in = &o->d;
    *conv.out = t;
    }
  };

template <typename T> class Caster : Caster<T *>
  {
public:
  typedef T &Result;
  typedef Crate::Traits<T> ClassTraits;

  static bool canCast(Boxer *boxer, const Object *o)
    {
    auto obj = const_cast<Object*>(o);
    return Caster<T *>::canCast(boxer, obj) && ClassTraits::unbox(boxer, obj) != nullptr;
    }

  static T &cast(Boxer *b, Object *o)
    {
    T *data = Caster<T *>::cast(b, o);
    if (!data)
      {
      throw Crate::TypeException(Crate::findType<T>(), nullptr);
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

  static bool canCast(Boxer *boxer, Object *o)
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
template <typename T> class Caster<const T> : public Caster<T>
  {
public:
  static void pack(Boxer *b, Object *o, const T &t)
    {
    Caster<T>::pack(b, o, const_cast<T &>(t));
    }
  };

template <typename T> class Caster<const T *> : public Caster<T *>
  {
  public:
    static void pack(Boxer *b, Object *o, const T *t)
    {
    Caster<T *>::pack(b, o, const_cast<T *>(t));
    }
  };

template <> class Caster<void *> { };

template <> class Caster<float> : public PodCaster<float> { };
template <> class Caster<double> : public PodCaster<double> { };
template <> class Caster<int> : public PodCaster<int> { };
template <> class Caster<bool> : public PodCaster<bool> { };

template <> class Caster<const float &> : public PodCaster<float> { };
template <> class Caster<const double &> : public PodCaster<double> { };
template <> class Caster<const int &> : public PodCaster<int> { };
template <> class Caster<const bool &> : public PodCaster<bool> { };

template <> class Caster<const char *>
  {
public:
  typedef const char *Result;

  static bool canCast(Boxer *, Object *o)
    {
    return o && o->type == Crate::findType<char *>();
    }

  static const char *cast(Boxer *b, Object *o)
    {
    if (!canCast(b, o))
      {
      throw Crate::TypeException(Crate::findType<char *>(), o ? o->type : nullptr);
      }
    return (char*)o->d;
    }

  static void pack(Boxer *, Object *o, const char *t)
    {
    o->d = (uint8_t*)t;
    o->type = Crate::findType<char *>();
    }
  };

template <> class Caster<const wchar_t *>
  {
  public:
  typedef const wchar_t *Result;

  static bool canCast(Boxer *, Object *o)
    {
    return o && o->type == Crate::findType<wchar_t *>();
    }

  static const wchar_t *cast(Boxer *b, Object *o)
    {
    if (!canCast(b, o))
      {
      throw Crate::TypeException(Crate::findType<wchar_t *>(), o ? o->type : nullptr);
      }
    return (wchar_t*)o->d;
    }

  static void pack(Boxer *, Object *o, const wchar_t *t)
    {
    o->d = (uint8_t*)t;
    o->type = Crate::findType<wchar_t *>();
    }
  };

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
    Arguments(Object **a = nullptr, std::size_t c = 0, Object *t = nullptr)
        : args(a), argCount(c), ths(t), resultCount(0)
      {
      }

    Object **args;
    std::size_t argCount;
    Object *ths;

    Object results[10];
    std::size_t resultCount;

  private:
    Arguments(const Arguments &);
    };

  struct Call
    {
    Arguments *args;
    Boxer *boxer;
    };
  typedef Call *CallData;
  typedef std::integral_constant<bool, false> ForceMember;

#if REFLECT_DESCRIPTIVE_EXCEPTIONS
  static std::string describeArguments(CallData args)
    {
    auto voidType = Crate::findType<void>();

    std::string argDesc;
    for (size_t i = 0; i < args->args->argCount; ++i)
      {
      if (i != 0)
        {
        argDesc += ", ";
        }
      argDesc += args->args->args[i] ? args->args->args[i]->type->name() : voidType->name();
      }

    std::string result;
    if (args->args->ths)
      {
      result = args->args->ths->type->name();
      }
    else
      {
      result = voidType->name();
      }
    result += " ->( " + argDesc + " )";
    return result;
    }

  template <typename Fn> static std::string describeFunction()
    {
    typedef typename Fn::Helper Helper;
    return describeFunction<typename Helper::Class, typename Helper::Arguments>(0);
    }

  template <typename Arguments> class ArgHelper
    {
  public:
    ArgHelper(std::size_t start)
        : m_start(start)
      {
      }

    void append(std::size_t i, const Crate::Type *type)
      {
      if (i > m_start)
        {
        m_result += ", ";
        }

      m_result += type->name();
      }

    template <std::size_t Idx> bool visit()
      {
      typedef typename std::tuple_element<Idx, Arguments>::type Element;
      append(Idx, Crate::findType<Element>());

      return false;
      }

    std::size_t m_start;
    std::string m_result;
    };

  template <typename Class, typename Arguments> static std::string describeFunction(size_t argStart)
    {
    ArgHelper<Arguments> helper(argStart);
    tupleEach<Arguments>(helper);

    return Crate::findType<Class>()->name() + " ->( " + helper.m_result + " )";
    }
#endif

  static std::size_t getArgumentCountWithThis(CallData args)
    {
    return args->args->argCount + (args->args->ths ? 1 : 0);
    }

  template <typename T> static T unpackThis(CallData args)
    {
    try
      {
      return Caster<T>::cast(args->boxer, args->args->ths);
      }
    catch(const Crate::TypeException &type)
      {
      throw Crate::ThisException(type);
      }
    }

  template <typename T> static bool canUnpackThis(CallData args)
    {
    return Caster<T>::canCast(args->boxer, args->args->ths);
    }

  template <typename Arg> class ReturnType
    {
    typedef typename Caster<Arg>::Result Type;
    };

  template <typename Arg>
      static typename Caster<Arg>::Result unpackArgument(CallData data, bool, size_t i)
    {
    if (data->args->argCount <= i)
      {
      throw Reflect::ArgCountException(i, data->args->argCount);
      }

    try
      {
      return Caster<Arg>::cast(data->boxer, data->args->args[i]);
      }
    catch(const Crate::TypeException &type)
      {
      throw Crate::ArgException(type, i);
      }
    }

  template <typename Arg>
      static bool canUnpackArgument(CallData data, bool, size_t i)
    {
    if (data->args->argCount <= i)
      {
      return false;
      }
    return Caster<Arg>::canCast(data->boxer, data->args->args[i]);
    }

  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    Object &b = data->args->results[data->args->resultCount++];

    Caster<Return>::pack(data->boxer, &b, result);
    }
  };

}

}
