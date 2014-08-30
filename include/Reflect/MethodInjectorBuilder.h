#pragma once

namespace Reflect
{

/// \brief Diverts the this argument of a call to the first parameter for
/// the function, then passes all the other arguments second.
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

  static std::size_t getArgumentCountWithThis(CallData data)
    {
    std::size_t count = Fwd::getArgumentCountWithThis(data);
    return count;
    }

  template <typename T> static T unpackThis(CallData)
    {
    return nullptr;
    }

  template <typename T> static bool canUnpackThis(CallData)
    {
    return false;
    }

  template <typename Arg>
  static Arg unpackArgument(CallData args, bool t, std::size_t i)
    {
    if (i == 0)
      {
      return Fwd::template unpackThis<Arg>(args);
      }
    else
      {
      return Fwd::template unpackArgument<Arg>(args, t, i-1);
      }
    }

  template <typename Arg>
  static bool canUnpackArgument(CallData args, bool t, std::size_t i)
    {
    if (i == 0)
      {
      return Fwd::template canUnpackThis<Arg>(args, i);
      }
    else
      {
      return Fwd::template canUnpackArgument<Arg>(args, t, i);
      }
    }

  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    Fwd::packReturn(data, result);
    }
  };

}
