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

  static bool canUnpackArgument(typename Fwd::CallData args)
    {
    return Fwd::template canUpackArgument<I-1, Arg>(args);
    }
  };

template <typename Arg, typename Fwd> class InjectorArgGetter<0, Arg, Fwd>
  {
public:
  static Arg unpackArgument(typename Fwd::CallData args)
    {
    return Fwd::template unpackThis<Arg>(args);
    }

  static bool canUnpackArgument(typename Fwd::CallData args)
    {
    return Fwd::template canUnpackThis<Arg>(args);
    }
  };
}

template <typename Fwd> class MethodInjectorBuilder
  {
public:
  typedef typename Fwd::CallData CallData;
  typedef typename Fwd::Result Result;

  template <typename Builder, typename SubType> static Result buildWrappedCall()
  {
    return Fwd::template buildWrappedCall<Builder, SubType>();
  }

  template <typename Builder, typename SubType> static Result buildWrappedCanCall()
  {
    return Fwd::template buildWrappedCanCall<Builder, SubType>();
  }

  static std::string describeArguments(CallData args)
    {
    return Fwd::describeArguments(args);
    }

  template <typename Fn> static std::string describeFunction()
    {
    typedef typename std::tuple_element<0, typename Fn::Arguments>::type Class;
    return describeFunction<Class, typename Fn::Arguments>(1);
    }

  static std::size_t getArgumentCount(CallData data)
    {
    std::size_t count = Fwd::getArgumentCount(data);
    return count == 0 ? 0 : count + 1;
    }

  template <typename T> static T unpackThis(CallData)
    {
    return nullptr;
    }

  template <typename T> static bool canUnpackThis(CallData)
    {
    return false;
    }

  template <std::size_t I, typename Arg>
      static Arg unpackArgument(CallData args)
    {
    return detail::InjectorArgGetter<I, Arg, Fwd>::unpackArgument(args);
    }

  template <std::size_t I, typename Arg>
      static bool canUnpackArgument(CallData args)
    {
    return detail::InjectorArgGetter<I, Arg, Fwd>::canUnpackArgument(args);
    }

  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    Fwd::packReturn(data, result);
    }
  };

}
