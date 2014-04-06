
Reflect [![Build Status](https://travis-ci.org/jorj1988/Reflect.png?branch=master)](https://travis-ci.org/jorj1988/Reflect)
=====================

Reflect is a function binding toolkit for C++. Using Reflect, methods and functions in C++ can be wrapped as a function taking any new argument format required.

Basic Usage
---------------------
Wrapping a function using reflect is very simple! For example, the class:
```cpp
class Test
{
  void testFunction(float param);
};
```

Can have the `testFunction` wrapped with:
```cpp
  // Specify which class is being wrapped
  //
  REFLECT_FUNCTION_HELPER(Test);
  
  // Generate a wrapper for the testFunction
  //
  typedef REFLECT_METHOD(testFunction) Method1;
  
  // Create a new function to invoke testFunction
  // See 'InvocationBuilder' section
  auto inv1 = InvocationBuilder::buildCall<Method1::Builder>();
  
  // Call the function!
  //
  inv1(...);
```


Overloaded Methods
---------------------
Reflect can wrap multiple methods into a single call. On calling, a method that matches the passed arguments is searched for, and called if available (exceptions are thrown if not callable).

> Note there is a runtime overhead to using overloaded methods.

An overloaded function can be build using:
```cpp
  // Method1 and Method2 are defined using REFLECT_METHOD, as above.
  typedef Reflect::FunctionArgumentTypeSelector<Method1::Builder, Method2::Builder> Overload1;
```
`Overload1` can now be treated as the `Method1` definition above, and built into a new callable function.

While the above system will accept functions taking different argument counts, it is more efficient to first switch between function argument counts, then switch on type.

Multiple Overloaded functions can be built with:
```cpp
  typedef Reflect::FunctionArgumentCountSelector<
    Reflect::FunctionArgCountSelectorBlock<2, Overload1>, // 2 specifies the argument count
    Reflect::FunctionArgCountSelectorBlock<1, Overload2>  // 1 is the argument count
    > AllMethods;
```
`AllMethods` can now be called using the methods above to generate callable functions. It will first search for a block taking te correct number of arguments, then call that block.


Fake Methods
---------------------
Methods don't always exist on the class that is being wrapped, but it is convenient to present a function as a class method.

For example, this function:
```cpp
void someFakeMethod(SomeClass &thsValue, float arg1, int arg2);
```

For example, can be *faked* as a method using:
```cpp
  // Specify which class is being wrapped
  //
  REFLECT_FUNCTION_HELPER(Test);
  
  // Generate a wrapper for the testFunction
  //
  typedef REFLECT_METHOD(testFunction) Method1;
  
  // Create a new function to invoke testFunction, using the MethodInjectorBuilder
  //
  auto inv1 = MethodInjectorBuilder<InvocationBuilder>::buildCall<Method1::Builder>();
  
  // Call the function!
  //
  inv1(...);
```

The method injector builder passes the *this* parameter of the arguments as the first parameter of the function (it is not possible to inject a method as a method!). This allows a function which is not part of a class to look as if it is.


Return Packers
---------------------
By default, Reflect support two types of returns: single return values:
```cpp
float floatReturner();
bool boolReturner();
```
And multiple return values:
```cpp
std::tuple<float, int, double> multiReturner();
```
Both types of functions call `InvocationBuilder::packReturn` once per return value.

It is possible to implement more specialised return packers by specialising `ReturnPacker`
```cpp
template <typename T, typename InvHelper> class ReturnPacker;
```
For example, you could wrap `std::optional<T>` to return a null value or a typed argument depending on if it is specified.


Using Crate to box and unbox classes
---------------------
Reflect has its own typing system - Crate. Crate is not used by Reflect itself, but an Invocation builder could utilise Crate to return correctly Typed arguments. Crate allows custom classes to be specified:
```cpp
namespace Crate
{
namespace detail
{
// Specialising this class causes Crate to find this type when calling Crate::findType<A>()
template <> struct TypeResolver<A>
  {
  static const Type *find()
    {
    // The string here is used for error messages
    static Type t("A"); 
    return &t;
    }
  };
}
}
```

This enables Crate to find the type, and Crates methods for boxing and unboxing classes could now be used on it.

Classes can be further customised in Crate by specifying how a class should be copied/referenced by the system. By default all classes are copied in and out of the class system.
```cpp
namespace Crate
{
// NonCopyable is doesn't have a copy constructor, so a pointer to the class is held by the system.
// Note the system will try to delete the pointer when asked.
template <> class Traits<NonCopyable> : public ReferenceTraits<NonCopyable>
  {
  };
  
// NonCopyable is doesn't have a copy constructor, so a pointer to the class is held by the system.
// Note the system will NOT try to delete the pointer when asked, it will just
// zero its own reference to the memory.
template <> class Traits<NonCopyable> : public ReferenceNonCleanedTraits<NonCopyable>
  {
  };
}
```

Crate expects users to implement a boxing interface, to help it deal with boxing memory
```cpp

class Boxer
  {
public:
  // A struct passed around representing a box
  struct BoxedData;

  // Find the type of boxed data
  const Crate::Type *getType(BoxedData) const;

  // Get the memory for the box (should comply with the Traits of the box!)
  void *getMemory(BoxedData);

  // Get the const memory of the box.
  const void *getMemory(BoxedData) const;

  // Setup a box with a type, and a cleanup function, to be called when the box is no longer required
  void initialise(BoxedData, const Crate::Type *t, Object::Cleanup c);
  };
```

There is some further Glue required to put Crate and Reflect together. This Glue may need to consider POD types differently, and enable transforming references and pointers for the InvocationBuilder. An example of this can be found in `example/Default/Builder.h` which provides boxing for the Reflect and Crate unit tests.

Writing an InvocationBuilder
---------------------
An invocation builder provides Reflect with functions to pack and unpack arguments for functions. 

An InvocationBuilder needs to provide some basic functions
```cpp
class CustomInvocationBuilder
  {
public:
  ////////////////////////////////////////////////////////////////////////////////
  // Required structures
  ////////////////////////////////////////////////////////////////////////////////
  // Defines a struct which is returned from buildCall
  struct Result;
  
  // Defines a struct which is returned from buildCalCall
  struct CanCallResult;
  
  // A structure which is passed around when calling a function.
  // See getArgumentCount for an example of use.
  struct CallData;

  ////////////////////////////////////////////////////////////////////////////////
  // Callback building
  ////////////////////////////////////////////////////////////////////////////////
  
  // Build a call to `Function`, using `the InvocationBuilder `Builder`.
  // The builder will generally be this class, but in some cases will be a forwarding builder
  // (see Fake Methods)
  template <typename Function, typename Builder=InvocationBuilder> 
    static Result buildCall();

  // Build a call to check if `Function` can be called, using `the InvocationBuilder `Builder`.
  // The builder will generally be this class, but in some cases will be a forwarding builder
  // (see Fake Methods)
  template <typename Function, typename Builder=InvocationBuilder> 
    static CanCallResult buildCanCall();

  ////////////////////////////////////////////////////////////////////////////////
  // Argument extraction and packing
  ////////////////////////////////////////////////////////////////////////////////

  // Get the argument count for `args`
  static std::size_t getArgumentCount(CallData args);

  // Get the `this` value as `T` from `args` (not called for static methods)
  // This method should verify (as canUnpackThis does), and throw if invalid.
  template <typename T> static T unpackThis(CallData args);

  // Find if `this` can be unpacked as `T`.
  // This method should not throw if `T` is invalid, it should return false.
  template <typename T> static bool canUnpackThis(CallData args);

  // Get the Ith argument as `Arg` from `args`
  // This method should verify (as canUnpackArgument does), and throw if invalid.
  template <std::size_t I, typename Arg>
    static typename Caster<Arg>::Result unpackArgument(CallData data);

  // Find if the Ith argument can be unpacked as `Arg`.
  // This method should not throw if `Arg` is invalid, it should return false.
  template <std::size_t I, typename Arg>
    static bool canUnpackArgument(CallData data);

  // Pack `result` as a return value for the function.
  // This function is called multiple times if the funtion returns
  // mutliple values (see Return Packers)
  template <typename Return, typename T>
    static void packReturn(CallData data, T &&result);
  
  ////////////////////////////////////////////////////////////////////////////////
  // Error Handling  
  ////////////////////////////////////////////////////////////////////////////////
  
  // Form a string describing the arguments in `args`
  // Used to create error messages.
  static std::string describeArguments(CallData args);

  // Describe Fn as a string.
  // Used to create error messages.
  template <typename Fn> 
    static std::string describeFunction();
    
  // Describe Class and Arguments as a string
  // Used to create error messages
  template <typename Class, typename Arguments> 
    static std::string describeFunction(size_t argStart);
  };
```

> Written with [StackEdit](https://stackedit.io/).