#pragma once
#include <tuple>
#include "Reflect/ReturnPacker.h"

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
    typedef typename FunctionHelper::Class *Cls;
    typedef typename FunctionHelper::Arguments Args;

    // Get this for the class
    auto ths = InvHelper::template getThis<Cls>(data);

    // Call the function, unpacking arguments, collect the return.
    auto result = FunctionHelper::template call<Fn>(
      ths,
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
    typedef typename FunctionHelper::Class *Cls;
    typedef typename FunctionHelper::Arguments Args;

    // Get this for the class
    auto ths = InvHelper::template getThis<Cls>(data);

    // Call the function, unpacking arguments.
    FunctionHelper::template call<Fn>(
      ths,
      InvHelper::template unpackArgument<Idx, typename std::tuple_element<Idx, Args>::type>(data)...
      );
    }
  };

/// \brief Helper class to create calls to functions.
/// \param InvHelper      A user defined helper which knows how to pack and unpack arguments.
/// \param FunctionHelper The type of the function to be wrapped - a specialised FunctionHelper<...>
/// \param Fn             The function to call.
template <typename InvHelper, typename _FunctionHelper, typename _FunctionHelper::Signature Fn>
    struct CallHelper
  {
  /// \brief The CallerData required by InvHelper to call the function.
  typedef typename InvHelper::CallData CallerData;
  typedef _FunctionHelper FunctionHelper;

  /// \brief Call to invoke the function.
  /// \param data The data containing the arguments which are passed to the function.
  static void call(CallerData data)
    {
    // The size of the tuple.
    typedef std::tuple_size<typename FunctionHelper::Arguments> TupleSize;
    // Indices for the arguments.
    typedef BuildIndices<TupleSize::value> IndicesForFunction;
    // The correct dispatcher - based on the ReturnType.
    typedef ReturnDispatch<InvHelper, FunctionHelper, Fn, typename FunctionHelper::ReturnType> Dispatch;

    // call the function.
    Dispatch::call(data, IndicesForFunction());
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

  template <Signature Fn> static ReturnType call(Class* cls, Args... args)
    {
    return (cls->*Fn)(std::forward<Args>(args)...);
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

  template <Signature Fn> static ReturnType call(Class* cls, Args... args)
    {
    return (cls->*Fn)(std::forward<Args>(args)...);
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

  template <Signature Fn> static ReturnType call(Class*, Args... args)
    {
    return Fn(std::forward<Args>(args)...);
    }
  };

}
}
