#pragma once
#include <tuple>
#include "Reflect/Utils/Exceptions.h"
#include "Reflect/Utils/ReturnPacker.h"
#include "Reflect/Utils/CanCallHelper.h"

namespace Reflect
{
namespace detail
{

/// \brief Holder for the indices
template <std::size_t... Is> struct Indices { };
/// \brief Build indices for a given size N.
template <std::size_t N, std::size_t... Is> struct BuildIndices : BuildIndices<N-1, N-1, Is...> { };
/// \overload
/// \brief Specialistion of BuildIndices for 0.
template <std::size_t... Is> struct BuildIndices<0, Is...> : Indices<Is...> { };

/// \brief Call a function with a return type and pack the return.
template <typename InvHelper, typename FunctionHelper, typename FunctionHelper::Signature Fn, typename T> struct ReturnDispatch
  {
  /// \brief The CallerData required by InvHelper to call the function.
  typedef typename InvHelper::CallData CallerData;

  template <typename std::size_t... Idx> static void call(CallerData data, Indices<Idx...>)
    {
    typedef typename FunctionHelper::Arguments Args;

    // Call the function, unpacking arguments, collect the return, and pack it.
    ReturnPacker<T, InvHelper>::pack(data,
      FunctionHelper::template call<Fn, InvHelper>(
        data,
        InvHelper::template unpackArgument<typename std::tuple_element<Idx, Args>::type>(data, FunctionHelper::Static::value == false, Idx)...
        )
      );
    }
  };

/// \brief Call a function with no return type and pack the return.
template <typename InvHelper, typename FunctionHelper, typename FunctionHelper::Signature Fn> struct ReturnDispatch<InvHelper, FunctionHelper, Fn, void>
  {
  /// \brief The CallerData required by InvHelper to call the function.
  typedef typename InvHelper::CallData CallerData;

  template <typename std::size_t... Idx> static void call(CallerData data, Indices<Idx...>)
    {
    typedef typename FunctionHelper::Arguments Args;

    // Call the function, unpacking arguments.
    FunctionHelper::template call<Fn, InvHelper>(
      data,
      InvHelper::template unpackArgument<typename std::tuple_element<Idx, Args>::type>(data, FunctionHelper::Static::value == false, Idx)...
      );
    }
  };

}

/// \brief Helper class to create calls to functions.
/// \param InvHelper      A user defined helper which knows how to pack and unpack arguments.
/// \param FunctionHelper The type of the function to be wrapped - a specialised FunctionHelper<...>
/// \param Fn             The function to call.
template <typename _FunctionHelper, typename _FunctionHelper::Signature Fn, typename _Caller>
    struct FunctionCall
  {
  typedef _FunctionHelper Helper;
  typedef _Caller Caller;

  /// \brief Call to invoke the function.
  /// \param data The data containing the arguments which are passed to the function.
  template <typename InvHelper> static void call(typename InvHelper::CallData data)
    {
    // The size of the tuple.
    typedef std::tuple_size<typename Helper::Arguments> ArgCount;
    // Indices for the arguments.
    typedef detail::BuildIndices<ArgCount::value> IndicesForFunction;
    // The correct dispatcher - based on the ReturnType.
    typedef detail::ReturnDispatch<InvHelper, Helper, Fn, typename Helper::ReturnType> Dispatch;

    typedef typename Helper::Static Static;
    typedef typename Caller::ForceMember ForceMember;
    std::size_t expectedThisArgs = (!ForceMember::value && Helper::Static::value) ? 0 : 1;
    std::size_t expectedArgs = (std::size_t)ArgCount::value + expectedThisArgs;

    if (InvHelper::getArgumentCountWithThis(data) != expectedArgs)
      {
      throw ArgCountException(
        expectedArgs,
        InvHelper::getArgumentCountWithThis(data));
      }

    // call the function.
    Dispatch::call(data, IndicesForFunction());
    }

  template <typename InvHelper> static bool canCall(typename InvHelper::CallData data)
    {
    typedef typename Helper::Static Static;
    return detail::CanCallHelper<InvHelper>::template canCast<typename Helper::Arguments, Static::value>(data) &&
        Helper::template canCastThis<InvHelper>(data);
    }
  };

/// \brief Class definition for the FunctionSignature. Specialised further below.
///
///        Expects several members:
///          Const      An integral constant which defines if the function is Const
///          Static     An integral constant which defines if the function takes a this parameter.
///          Class      A typedef for the Class that owns the function.
///          ReturnType A typedef for the return value of the function.
///          Arguments  A typedef for a tuple that contains the argument types for the function.
///          Signature  A typedef for the signature of the function.
///          call       A function which calls the function, accepting a pointer to Class
///                     and arguments for the call.
template <typename FnType> class FunctionSignature;

/// \overload
/// \brief FunctionSignature definition for a non-const member function.
template <typename Rt, typename Cls, typename... Args>
    class FunctionSignature<Rt(Cls::*)(Args...)>
  {
public:
  typedef std::integral_constant<bool, false> Const;
  typedef std::integral_constant<bool, false> Static;

  typedef Cls Class;
  typedef Rt ReturnType;
  typedef std::tuple<Args...> Arguments;
  typedef Rt(Class::*Signature)(Args...);

  template <Signature Fn, typename InvHelper> static ReturnType call(typename InvHelper::CallData data, Args... args)
    {
    // Get this for the class
    auto cls = InvHelper::template unpackThis<Cls*>(data);

    return (cls->*Fn)(std::forward<Args>(args)...);
    }

  template <typename InvHelper> static bool canCastThis(typename InvHelper::CallData data)
    {
    return detail::CanCallHelper<InvHelper>::template canCastThis<Cls*>(data);
    }
  };

/// \overload
/// \brief FunctionSignature definition for a const member function.
template <typename Rt, typename Cls, typename... Args>
    class FunctionSignature<Rt(Cls::*)(Args...) const>
  {
  public:
  typedef std::integral_constant<bool, true> Const;
  typedef std::integral_constant<bool, false> Static;

  typedef Cls Class;
  typedef Rt ReturnType;
  typedef std::tuple<Args...> Arguments;
  typedef Rt(Class::*Signature)(Args...) const;

  template <Signature Fn, typename InvHelper> static ReturnType call(typename InvHelper::CallData data, Args... args)
    {
    // Get this for the class
    auto cls = InvHelper::template unpackThis<Cls*>(data);

    return (cls->*Fn)(std::forward<Args>(args)...);
    }

  template <typename InvHelper> static bool canCastThis(typename InvHelper::CallData data)
    {
    return detail::CanCallHelper<InvHelper>::template canCastThis<Cls*>(data);
    }
  };

/// \overload
/// \brief FunctionSignature definition for a static function.
template <typename Rt, typename... Args>
    class FunctionSignature<Rt (*)(Args...)>
  {
  public:
  typedef std::integral_constant<bool, false> Const;
  typedef std::integral_constant<bool, true> Static;

  typedef void Class;
  typedef Rt ReturnType;
  typedef std::tuple<Args...> Arguments;
  typedef ReturnType (*Signature)(Args...);

  template <Signature Fn, typename InvHelper> static ReturnType call(typename InvHelper::CallData, Args... args)
    {
    return Fn(std::forward<Args>(args)...);
    }

  template <typename InvHelper> static bool canCastThis(typename InvHelper::CallData)
    {
    return true;
    }
  };

}
