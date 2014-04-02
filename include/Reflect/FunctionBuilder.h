#pragma once

#ifdef REFLECT_MACRO_IMPL
# include "Reflect/Macro/FunctionBuilderImpl.h"
#else
# include "Reflect/Cpp11/FunctionBuilderImpl.h"
#endif

#include "Reflect/Type.h"

namespace Reflect
{

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
  static const Type *returnType() { return findType<ReturnType>(); }

  /// \brief The number of arguments to be passed to the function
  static size_t argumentCount() { return std::tuple_size<Arguments>::value; }

  /// \brief The type of the argument at N.
  template <size_t N> static const Type *argumentType()
    {
    typedef typename std::tuple_element<N, Arguments>::type Type;
    return findType<Type>();
    }

  /// \brief Build a call to this function of type T::Signature.
  ///        Builder must be a class containing required functions
  ///        to build calls.
  /// \sa    BasicBuilder.h
  template <typename T> static typename T::Result buildCall()
    {
    typedef detail::CallHelper<T, Helper, Fn> Builder;
    return T::template build<Builder>();
    }

  template <typename T> static typename T::CanCallResult buildCanCall()
    {
    typedef detail::CallHelper<T, Helper, Fn> Builder;
    return T::template buildCanCall<Builder>();
    }

  /// \brief Call this function, passing the given argument data.
  template <typename T> static void call(typename T::CallData data)
    {
    typedef detail::CallHelper<T, Helper, Fn> Builder;
    return Builder::call(data);
    }

  template <typename T> static bool canCall(typename T::CallData data)
    {
    typedef detail::CallHelper<T, Helper, Fn> Builder;
    return Builder::canCall(data);
    }
  };

#define REFLECT_FUNCTION_HELPER(cls) typedef cls ReflectClass
#define REFLECT_FUNCTION_PTR(name) &name
#define REFLECT_FUNCTION(name) Reflect::FunctionBuilder<decltype(REFLECT_FUNCTION_PTR(name)), REFLECT_FUNCTION_PTR(name)>()
#define REFLECT_METHOD_PTR(name) & ReflectClass::name
#define REFLECT_METHOD(name) Reflect::FunctionBuilder<decltype(REFLECT_METHOD_PTR(name)), REFLECT_METHOD_PTR(name)>()

}
