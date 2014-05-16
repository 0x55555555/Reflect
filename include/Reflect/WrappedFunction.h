#pragma once

#include "Reflect.h"

#ifdef REFLECT_MACRO_IMPL
# include "Reflect/Macro/FunctionBuilderImpl.h"
#else
# include "Reflect/Cpp11/FunctionBuilderImpl.h"
#endif

namespace Reflect
{

/// \brief Wrap a function of a given type. Allows extracting reflected
///        calls to the function.
///        Call REFLECT_METHOD(function) after using REFLECT_CLASS, or
///        typedefing ReflectClass to generate a FunctionWrap.
///        Call buildInvocation on this function to build a reflected call
///        to the function.
template <typename FnType, FnType Fn> class WrappedFunction
  {
public:
  typedef detail::FunctionHelper<FnType> Helper;
  typedef typename Helper::ReturnType ReturnType;
  typedef typename Helper::Arguments Arguments;
  typedef detail::CallHelper<Helper, Fn> Builder;

  /// \brief The number of arguments to be passed to the function
  static size_t argumentCount() { return std::tuple_size<Arguments>::value; }
  };

#define REFLECT_FUNCTION_HELPER(cls) typedef cls ReflectClass
#define REFLECT_FUNCTION_PTR(name) &name
#define REFLECT_FUNCTION(name) Reflect::WrappedFunction<decltype(REFLECT_FUNCTION_PTR(name)), REFLECT_FUNCTION_PTR(name)>
#define REFLECT_METHOD_PTR(name) & ReflectClass::name
#define REFLECT_METHOD(name) Reflect::WrappedFunction<decltype(REFLECT_METHOD_PTR(name)), REFLECT_METHOD_PTR(name)>

}
