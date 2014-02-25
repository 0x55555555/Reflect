#pragma once
#include <tuple>
#include "Reflect/Type.h"

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
      InvHelper::template unpackArgument<Idx, Args>(data)...
      );

    // Pack the return into data.
    InvHelper::template packReturn<T>(data, std::move(result));
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
      InvHelper::template unpackArgument<Idx, Args>(data)...
      );
    }
  };

/// \brief Helper class to create calls to functions.
/// \param InvHelper      A user defined helper which knows how to pack and unpack arguments.
/// \param FunctionHelper The type of the function to be wrapped - a specialised FunctionHelper<...>
/// \param Fn             The function to call.
template <typename InvHelper, typename FunctionHelper, typename FunctionHelper::Signature Fn>
    struct CallHelper
  {
  /// \brief The CallerData required by InvHelper to call the function.
  typedef typename InvHelper::CallData CallerData;

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

/// \brief Wrap a function of a given type. Allows extracting reflected
///        calls to the function.
///        Call REFLECT_METHOD(function) after using REFLECT_CLASS, or
///        typedefing ReflectClass to generate a FunctionWrap.
///        Call buildInvocation on this function to build a reflected call
///        to the function.
template <typename FnType, FnType Fn> class FunctionBuilder
  {
public:
  typedef detail::FunctionHelper<FnType> Helper;
  typedef typename Helper::ReturnType ReturnType;
  typedef typename Helper::Arguments Arguments;

  /// \brief Find the type of the return value of the function.
  const Type *returnType() { return findType<ReturnType>(); }

  /// \brief The number of arguments to be passed to the function
  size_t argumentCount() { return std::tuple_size<Arguments>::value; }

  /// \brief The type of the argument at N.
  template <size_t N> const Type *argumentType()
    {
    typedef typename std::tuple_element<N, Arguments>::type Type;
    return findType<Type>();
    }

  /// \brief Build a call to this function of type T::Signature.
  ///        Builder must be a class containing required functions
  ///        to build calls.
  /// \sa    BasicBuilder.h
  template <typename T> typename T::Result buildInvocation() const
    {
    typedef detail::CallHelper<T, Helper, Fn> Builder;
    return T::template build<Builder>();
    }

  /// \internal
  /// \brief Use REFLECT_METHOD and partners to create a FunctionWrap.
  FunctionBuilder(const char* name) : _name(name)
    {
    }

  const char* name() const { return _name; }

private:
  const char* _name;
  };

#define REFLECT_FUNCTION_HELPER(cls) typedef cls ReflectClass
#define REFLECT_METHOD_PTR(name) & ReflectClass::name
#define REFLECT_METHOD(name) Reflect::FunctionBuilder<decltype(REFLECT_METHOD_PTR(name)), REFLECT_METHOD_PTR(name)>(#name)

}
