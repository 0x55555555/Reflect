#pragma once

namespace Reflect
{

namespace detail
{

template <std::size_t I, typename Tuple, typename Fwd> class InjectorArgGetter
  {
public:
  static typename std::tuple_element<I, Tuple>::type unpackArgument(typename Fwd::CallData args)
    {
    return Fwd::template unpackArgument<I, Tuple>(args);
    }
  };

template <typename Tuple, typename Fwd> class InjectorArgGetter<0, Tuple, Fwd>
  {
public:
  typedef typename std::tuple_element<0, Tuple>::type Result;
  static Result unpackArgument(typename Fwd::CallData args)
    {
    return Fwd::template getThis<Result>(args);
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

  template <std::size_t I, typename Tuple>
      static typename std::tuple_element<I, Tuple>::type unpackArgument(CallData args)
    {
    return detail::InjectorArgGetter<I, Tuple, Fwd>::unpackArgument(args);
    }

  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    Fwd::packReturn(data, result);
    }
  };

}
