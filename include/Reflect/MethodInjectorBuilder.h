#pragma once

namespace Reflect
{

namespace detail
{

template <std::size_t I, typename Arg, typename Fwd> class InjectorArgGetter
  {
public:
  static Arg unpackArgument(typename Fwd::CallData args)
    {
    return Fwd::template unpackArgument<I-1, Arg>(args);
    }
  };

template <typename Arg, typename Fwd> class InjectorArgGetter<0, Arg, Fwd>
  {
public:
  static Arg unpackArgument(typename Fwd::CallData args)
    {
    return Fwd::template getThis<Arg>(args);
    }
  };
}

template <typename Fwd> class MethodInjectorBuilder
  {
public:
  typedef typename Fwd::CallData CallData;
  typedef typename Fwd::Result Result;

  template <typename Builder> static Result build()
    {
    return Fwd::template build<Builder>();
    }

  template <typename Builder> static void call(CallData data)
    {
    Builder::call(data);
    }

  template <typename T> static T getThis(CallData)
    {
    return nullptr;
    }

  template <std::size_t I, typename Arg>
      static Arg unpackArgument(CallData args)
    {
    return detail::InjectorArgGetter<I, Arg, Fwd>::unpackArgument(args);
    }

  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    Fwd::packReturn(data, result);
    }
  };

}
