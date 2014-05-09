#define JOIN(A, B) JOIN_IMPL(A, B)
#define JOIN_IMPL(A, B) A##B
#define CALL_HELPER_NAME JOIN(CallHelper, REFLEX_TEMPLATE_COUNT)
#define UNPACK_HELPER(Idx) , InvHelper::template unpackArgument<Idx, typename std::tuple_element<Idx, Args>::type>(data)

namespace Reflect
{
namespace detail
{

/// \brief Call a function with a return type and pack the return.
template <typename InvHelper, typename FunctionHelper, typename FunctionHelper::Signature Fn, typename T>
    struct ReturnDispatch<REFLEX_TEMPLATE_COUNT, InvHelper, FunctionHelper, Fn, T>
  {
  /// \brief The CallerData required by InvHelper to call the function.
  typedef typename InvHelper::CallData CallerData;

  static void call(CallerData data)
    {
    typedef typename FunctionHelper::Arguments Args;

    // Call the function, unpacking arguments, collect the return.
    auto&& result = FunctionHelper::template call<Fn, InvHelper>(
      data
      REFLEX_TEMPLATE_UNPACK(UNPACK_HELPER)
      );

    // Pack the return into data.
    ReturnPacker<T, InvHelper>::pack(data, std::move(result));
    }
  };

/// \brief Call a function with no return type and pack the return.
template <typename InvHelper, typename FunctionHelper, typename FunctionHelper::Signature Fn>
    struct ReturnDispatch<REFLEX_TEMPLATE_COUNT, InvHelper, FunctionHelper, Fn, void>
  {
  /// \brief The CallerData required by InvHelper to call the function.
  typedef typename InvHelper::CallData CallerData;

  static void call(CallerData data)
    {
    typedef typename FunctionHelper::Arguments Args;

    // Call the function, unpacking arguments.
    FunctionHelper::template call<Fn, InvHelper>(
      data
      REFLEX_TEMPLATE_UNPACK(UNPACK_HELPER)
      );
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

#define TYPENAME_HELPER(Idx) , typename JOIN(Arg, Idx)
#define ARG_HELPER(Idx) JOIN(Arg, Idx)
#define PARAM_HELPER(Idx) , JOIN(Arg, Idx) JOIN(param, Idx)
#define PARAM_FORWARD_HELPER(Idx) JOIN(param, Idx)


/// \overload
/// \brief FunctionHelper definition for a non-const member function.
template <typename Rt, typename Cls REFLEX_TEMPLATE_UNPACK(TYPENAME_HELPER)>
    class FunctionHelper<Rt(Cls::*)(REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER))>
  {
public:
  typedef std::integral_constant<bool, false> Const;
  typedef std::integral_constant<bool, false> Static;

  typedef Cls Class;
  typedef Rt ReturnType;
  typedef std::tuple<REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER)> Arguments;
  typedef Rt(Class::*Signature)(REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER));

  template <Signature Fn, typename InvHelper> static ReturnType call(typename InvHelper::CallData data REFLEX_TEMPLATE_UNPACK(PARAM_HELPER))
    {
    // Get this for the class
    auto cls = InvHelper::template unpackThis<Cls*>(data);

    return (cls->*Fn)(REFLEX_TEMPLATE_UNPACK_COMMA(PARAM_FORWARD_HELPER));
    }

  template <typename InvHelper> static bool canCastThis(typename InvHelper::CallData data)
    {
    return detail::CanCallHelper<InvHelper>::template canCastThis<Cls*>(data);
    }
  };

/// \overload
/// \brief FunctionHelper definition for a const member function.
template <typename Rt, typename Cls REFLEX_TEMPLATE_UNPACK(TYPENAME_HELPER)>
    class FunctionHelper<Rt(Cls::*)(REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER)) const>
  {
  public:
  typedef std::integral_constant<bool, true> Const;
  typedef std::integral_constant<bool, false> Static;

  typedef Cls Class;
  typedef Rt ReturnType;
  typedef std::tuple<REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER)> Arguments;
  typedef Rt(Class::*Signature)(REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER)) const;

  template <Signature Fn, typename InvHelper> static ReturnType call(typename InvHelper::CallData data REFLEX_TEMPLATE_UNPACK(PARAM_HELPER))
    {
    // Get this for the class
    auto cls = InvHelper::template unpackThis<Cls*>(data);

    return (cls->*Fn)(REFLEX_TEMPLATE_UNPACK_COMMA(PARAM_FORWARD_HELPER));
    }

  template <typename InvHelper> static bool canCastThis(typename InvHelper::CallData data)
    {
    return detail::CanCallHelper<InvHelper>::template canCastThis<Cls*>(data);
    }
  };

/// \overload
/// \brief FunctionHelper definition for a static function.
template <typename Rt REFLEX_TEMPLATE_UNPACK(TYPENAME_HELPER)>
    class FunctionHelper<Rt (*)(REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER))>
  {
  public:
  typedef std::integral_constant<bool, false> Const;
  typedef std::integral_constant<bool, true> Static;

  typedef void Class;
  typedef Rt ReturnType;
  typedef std::tuple<REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER)> Arguments;
  typedef ReturnType (*Signature)(REFLEX_TEMPLATE_UNPACK_COMMA(ARG_HELPER));

  template <Signature Fn, typename InvHelper> static ReturnType call(typename InvHelper::CallData REFLEX_TEMPLATE_UNPACK(PARAM_HELPER))
    {
    return Fn(REFLEX_TEMPLATE_UNPACK_COMMA(PARAM_FORWARD_HELPER));
    }

  template <typename InvHelper> static bool canCastThis(typename InvHelper::CallData)
    {
    return true;
    }
  };

}
}

#undef TYPENAME_HELPER
#undef ARG_HELPER
#undef PARAM_HELPER
#undef PARAM_FORWARD_HELPER
#undef CALL_HELPER_NAME
#undef UNPACK_HELPER
#undef JOIN
#undef JOIN_IMPL
#undef REFLEX_TEMPLATE_UNPACK
