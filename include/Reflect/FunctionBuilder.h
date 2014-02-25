#pragma once
#include <tuple>
#include "Reflect/Type.h"

namespace Reflect
{
namespace detail
{

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
    typedef ReturnDispatch<typename FunctionHelper::ReturnType> Dispatch;

    // call the function.
    Dispatch::call(data, IndicesForFunction());
    }

private:
  /// \brief Holder for the indices
  template <std::size_t... Is> struct Indices { };
  /// \brief Build indices for a given size N.
  template <std::size_t N, std::size_t... Is> struct BuildIndices : BuildIndices<N-1, N-1, Is...> { };
  /// \overload
  /// \brief Specialistion of BuildIndices for 0.
  template <std::size_t... Is> struct BuildIndices<0, Is...> : Indices<Is...> { };

  /// \brief Call a function with a return type and pack the return.
  template <typename T> struct ReturnDispatch
    {
    template <typename std::size_t... Idx> static void call(CallerData data, Indices<Idx...>)
      {
      // Get this for the class
      auto ths = InvHelper::getThis<FunctionHelper::Class*>(data);

      // Call the function, unpacking arguments, collect the return.
      auto result = FunctionHelper::call<Fn>(ths, unpack<Idx>(data)...);

      // Pack the return into data.
      InvHelper::packReturn<typename FunctionHelper::ReturnType>(data, std::move(result));
      }
    };

  /// \brief Call a function with no return type and pack the return.
  template <> struct ReturnDispatch<void>
    {
    template <typename std::size_t... Idx> static void call(CallerData data, Indices<Idx...>)
      {
      // Get this for the class
      auto ths = InvHelper::getThis<FunctionHelper::Class*>(data);

      // Call the function, unpacking arguments.
      FunctionHelper::call<Fn>(ths, unpack<Idx>(data)...);
      }
    };

  /// \brief unpack an argument from data and return it as the correct type.
  template <std::size_t Index>
      static typename std::tuple_element<Index, typename FunctionHelper::Arguments>::type
      unpack(CallerData data)
    {
    return InvHelper::unpackArgument<Index, FunctionHelper::Arguments>(data);
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

  template <Signature Fn, typename... Args> static ReturnType call(Class* cls, Args... args)
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

  template <Signature Fn, typename... Args> static ReturnType call(Class* cls, Args... args)
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

  template <Signature Fn, typename... Args> static ReturnType call(Class*, Args... args)
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
    return T::build<Builder>();
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
