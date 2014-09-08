#pragma once
#include <tuple>
#include "Reflect/Utils/ReturnPacker.h"
#include "Reflect/Utils/CanCallHelper.h"
#include "Reflect/Utils/Exceptions.h"

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

#define REFLEX_TEMPLATE_COUNT 4
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0) MACRO(1) MACRO(2) MACRO(3)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0), MACRO(1), MACRO(2), MACRO(3)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

#define REFLEX_TEMPLATE_COUNT 5
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0) MACRO(1) MACRO(2) MACRO(3) MACRO(4)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0), MACRO(1), MACRO(2), MACRO(3), MACRO(4)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

#define REFLEX_TEMPLATE_COUNT 6
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0) MACRO(1) MACRO(2) MACRO(3) MACRO(4) MACRO(5)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0), MACRO(1), MACRO(2), MACRO(3), MACRO(4), MACRO(5)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

#define REFLEX_TEMPLATE_COUNT 7
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0) MACRO(1) MACRO(2) MACRO(3) MACRO(4) MACRO(5) MACRO(6)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0), MACRO(1), MACRO(2), MACRO(3), MACRO(4), MACRO(5), MACRO(6)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

#define REFLEX_TEMPLATE_COUNT 8
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0) MACRO(1) MACRO(2) MACRO(3) MACRO(4) MACRO(5) MACRO(6) MACRO(7)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0), MACRO(1), MACRO(2), MACRO(3), MACRO(4), MACRO(5), MACRO(6), MACRO(7)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

#define REFLEX_TEMPLATE_COUNT 9
#define REFLEX_TEMPLATE_UNPACK(MACRO) MACRO(0) MACRO(1) MACRO(2) MACRO(3) MACRO(4) MACRO(5) MACRO(6) MACRO(7) MACRO(8)
#define REFLEX_TEMPLATE_UNPACK_COMMA(MACRO) MACRO(0), MACRO(1), MACRO(2), MACRO(3), MACRO(4), MACRO(5), MACRO(6), MACRO(7), MACRO(8)
#include "Reflect/Macro/FunctionBuilderImplTemplate.h"
#undef REFLEX_TEMPLATE_COUNT
#undef REFLEX_TEMPLATE_UNPACK
#undef REFLEX_TEMPLATE_UNPACK_COMMA

namespace Reflect
{

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
    typedef std::tuple_size<typename Helper::Arguments> ArgCount;
    // The correct dispatcher - based on the ReturnType.
    typedef detail::ReturnDispatch<
      ArgCount::value,
      Caller,
      Helper,
      Fn,
      typename Helper::ReturnType> CallDispatch;

    std::size_t expectedThisArgs = Helper::Static::value ? 0 : 1;
    std::size_t expectedArgs = (std::size_t)ArgCount::value + expectedThisArgs;

    typedef typename Helper::Static Static;
    if (Caller::getArgumentCountWithThis(data) != expectedArgs)
      {
      throw ArgCountException(
        expectedArgs,
        Caller::getArgumentCountWithThis(data));
      }

    // call the function.
    CallDispatch::call(data);
    }

  template <typename InvHelper> static bool canCall(typename InvHelper::CallData data)
    {
    return detail::CanCallHelper<InvHelper>::template canCast<typename Helper::Arguments, Helper::Static::value>(data) &&
        Helper::template canCastThis<InvHelper>(data);
    }
  };

}

