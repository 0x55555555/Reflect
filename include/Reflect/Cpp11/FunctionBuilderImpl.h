#pragma once
#include <tuple>
#include "Reflect/Exceptions.h"
#include "Reflect/ReturnPacker.h"
#include "Reflect/CanCallHelper.h"

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

    // Call the function, unpacking arguments, collect the return.
    auto result = FunctionHelper::template call<Fn, InvHelper>(
      data,
      InvHelper::template unpackArgument<Idx, typename std::tuple_element<Idx, Args>::type>(data)...
      );

    // Pack the return into data.
    ReturnPacker<T, InvHelper>::pack(data, std::move(result));
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
      InvHelper::template unpackArgument<Idx, typename std::tuple_element<Idx, Args>::type>(data)...
      );
    }
  };

/// \brief Helper class to create calls to functions.
/// \param InvHelper      A user defined helper which knows how to pack and unpack arguments.
/// \param FunctionHelper The type of the function to be wrapped - a specialised FunctionHelper<...>
/// \param Fn             The function to call.
template <typename _FunctionHelper, typename _FunctionHelper::Signature Fn>
    struct CallHelper
  {
  typedef _FunctionHelper Helper;

  /// \brief Call to invoke the function.
  /// \param data The data containing the arguments which are passed to the function.
  template <typename InvHelper> static void call(typename InvHelper::CallData data)
    {
    // The size of the tuple.
    typedef std::tuple_size<typename Helper::Arguments> TupleSize;
    // Indices for the arguments.
    typedef BuildIndices<TupleSize::value> IndicesForFunction;
    // The correct dispatcher - based on the ReturnType.
    typedef ReturnDispatch<InvHelper, Helper, Fn, typename Helper::ReturnType> Dispatch;

    if (InvHelper::getArgumentCount(data) < std::tuple_size<typename Helper::Arguments>::value)
      {
      throw ArgCountException(
        std::tuple_size<typename Helper::Arguments>::value,
        InvHelper::getArgumentCount(data));
      }

    // call the function.
    Dispatch::call(data, IndicesForFunction());
    }

  template <typename InvHelper> static bool canCall(typename InvHelper::CallData data)
    {
    return detail::CanCallHelper<InvHelper>::template canCast<typename Helper::Arguments>(data) &&
        Helper::template canCastThis<InvHelper>(data);
    }
  };

/// \brief Class definition for the FunctionHelper. Specialised further below.
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
template <typename FnType> class FunctionHelper;

/// \overload
/// \brief FunctionHelper definition for a non-const member function.
template <typename Rt, typename Cls, typename... Args>
    class FunctionHelper<Rt(Cls::*)(Args...)>
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
/// \brief FunctionHelper definition for a const member function.
template <typename Rt, typename Cls, typename... Args>
    class FunctionHelper<Rt(Cls::*)(Args...) const>
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
/// \brief FunctionHelper definition for a static function.
template <typename Rt, typename... Args>
    class FunctionHelper<Rt (*)(Args...)>
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
}
