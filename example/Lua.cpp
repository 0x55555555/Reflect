#include "Reflect/FunctionBuilder.h"
#include "lua.h"
#include <cstdlib>

/// \brief This is a little helper (doesnt work!) which casts types to the correct value.
struct TypeHelper
  {
  template <typename T> static T getStackArg(lua_State *L, int idx)
    {
    // Type checking is not implemented here...
    void *data = lua_touserdata(L, idx);
    return *static_cast<T*>(data);
    }

  template <typename T> static void pushReturn(lua_State *L, T &&p)
    {
    // Type checking is not implemented here...
    void *data = lua_newuserdata(L, 4);
    void **result = static_cast<void**>(data);

    *result = &p;
    }
  };

/// \brief Builder for method invocation
class LuaInvocationBuilder
  {
public:
  /// \brief Data passed around during a call operation.
  struct CallData
    {
    lua_State *state;
    uint8_t argOffset;
    uint8_t returnCount;
    };

  typedef lua_CFunction Result;

  /// \brief called to build the function itself, Builder is a Reflect::detail::CallHelper class
  template <typename Builder> static Result build()
    {
    return call<Builder>;
    }

  /// \brief Lua entrypoint for the call.
  template <typename Builder> static int call(lua_State *L)
    {
    // set up the call data
    CallData data = {
      L,
      Builder::FunctionHelper::Static::value ? 0 : 1,
      0
    };

    // Find out the correct number of arguments (lua passes this as [1] ( and is 1 indexed... :( ))
    typedef std::tuple_size<typename Builder::FunctionHelper::Arguments> ArgCount;
    const std::size_t expectedArgCount =
      ArgCount::value +
      data.argOffset;

    // if we get the wrong number of values then bail out
    // (this long jumps, so be careful of classes created above...)
    if (lua_gettop(L) != (int)expectedArgCount)
      {
      lua_pushstring(L, "Incorrect argument count");
      lua_error(L);
      }

    // Invoke the method (uses getThis, unpackArgument, and packReturn)
    Builder::call(data);
    return data.returnCount;
    }

  /// \brief Get the this parameter for the function
  template <typename T> static T getThis(CallData args)
    {
    return TypeHelper::getStackArg<T>(args.state, 1);
    }

  /// \brief Get the argument for the parameter [I]th parameter.
  template <std::size_t I, typename Tuple>
      static typename std::tuple_element<I, Tuple>::type unpackArgument(CallData args)
    {
    typedef typename std::tuple_element<I, Tuple>::type Arg;
    typedef typename std::remove_reference<Arg>::type NoRef;
    return TypeHelper::getStackArg<NoRef>(args.state, args.argOffset + I + 1);
    }

  /// \brief Pack a return value for the function.
  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    TypeHelper::pushReturn(data.state, result);
    ++data.returnCount;
    }
  };

class Test
  {
public:
  void test(int, float);
  };

int main()
  {
  // create a lua state.
  lua_State* L = lua_newstate(
    [](void *, void *ptr, size_t, size_t nsize) { return realloc(ptr, nsize); },
    nullptr);

  // Typedefing this class for REFLECT_METHOD
  typedef Test ReflectClass;

  // Create a wraping helper for the method
  auto methodHelper = REFLECT_METHOD(test);
  // Create a static lua_CFunction for the method
  lua_CFunction method = methodHelper.buildInvocation<LuaInvocationBuilder>();

  // Invoke the method (expects the lua_State to be set up...)
  method(L);
  }
