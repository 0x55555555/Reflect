#pragma once
#include <tuple>

namespace Reflect
{
namespace detail
{
template <size_t ArgCount, typename InvHelper, typename FunctionHelper, typename FunctionHelper::Signature Fn, typename T> struct ReturnDispatch;
}
}

#define REFLEX_TEMPLATE_COUNT 0
#define REFLEX_TEMPLATE_UNPACK(MACRO)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

#define REFLEX_TEMPLATE_COUNT 1
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

#define REFLEX_TEMPLATE_COUNT 2
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0) MACRO(1)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0), MACRO(1)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA


#define REFLEX_TEMPLATE_COUNT 3
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0) MACRO(1) MACRO(2)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0), MACRO(1), MACRO(2)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

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
    typedef std::tuple_size<typename FunctionHelper::Arguments> ArgCount;
    // The correct dispatcher - based on the ReturnType.
    typedef ReturnDispatch<
      ArgCount::value,
      InvHelper,
      FunctionHelper,
      Fn,
      typename FunctionHelper::ReturnType> CallDispatch;

    // call the function.
    CallDispatch::call(data);
    }
  };

}
}

