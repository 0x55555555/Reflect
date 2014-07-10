#include "ReflectTest.h"
#include "Crate/EmbeddedTypes.h"
#include "Reflect/MethodInjectorBuilder.h"
#include "Reflect/WrappedFunction.h"
#include "Reflect/FunctionSelector.h"
#include "../example/Default/Builder.h"
#include <tuple>

#define FLOAT_VAL 5.0f
#define DOUBLE_VAL 5.0
#define INT_VAL 2222
#define SELF_VAL 500

#define VERIFY_THROWS(lambda, type, equals) \
  { bool thrown = false; \
  try{ lambda(); } \
  catch(type t) { thrown = true; QVERIFY2(t == equals, "unequal exception"); } \
  catch(...) { QVERIFY2(false, "invalid exception type thrown"); } \
  QVERIFY2(thrown, "Nothing was thrown"); \
  }

#define VERIFY_NO_THROWS(lambda) \
  try{ lambda(); } \
  catch(...) { QVERIFY2(false, "invalid exception type thrown"); }

#define QCOMPARE_NO_RETURN(actual, expected) \
do {\
    if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__))\
      throw std::exception();\
} while (0)

class A
  {
public:
  void pork1(const float& in, double pork)
    {
    QCOMPARE_NO_RETURN(this->pork, SELF_VAL);
    QCOMPARE_NO_RETURN(in, FLOAT_VAL);
    QCOMPARE_NO_RETURN(pork, DOUBLE_VAL);
    }

  int pork2(A *a) const
    {
    QCOMPARE_NO_RETURN(pork, SELF_VAL);
    QCOMPARE_NO_RETURN(a->pork, SELF_VAL);
    return INT_VAL;
    }

  static A* pork3(const float& in)
    {
    QCOMPARE_NO_RETURN(in, FLOAT_VAL);
    static A a;
    a.pork = SELF_VAL;
    return &a;
    }

  void pork4(A *, float )
    {
    }

  std::tuple<int, float, double> multiReturn()
    {
    return std::make_tuple(5, 6.4f, 5.0);
    }

  A()
    {
    pork = -1;
    }

  int pork;
  };

void staticMethod(A &a, int i, float e)
  {
  QCOMPARE_NO_RETURN(&a != nullptr, true);
  QCOMPARE_NO_RETURN(i, INT_VAL);
  QCOMPARE_NO_RETURN(e, FLOAT_VAL);
  }

namespace Crate
{
namespace detail
{
template <> struct TypeResolver<A>
  {
  static const Type *find()
    {
    static Type t;
    t.initialise<A>("A", nullptr);
    return &t;
    }
  };

}
}

class InvocationBuilder : public Reflect::example::Builder
  {
public:
  struct Result
    {
    typedef void (*Signature)(Reflect::example::Boxer *, Arguments *);
    Signature fn;
    };
  struct CanCallResult
    {
    typedef bool (*Signature)(Reflect::example::Boxer *, Arguments *);
    Signature fn;
    };

  template <typename Function, typename Builder> static Result buildWrappedCall()
    {
    Result r = { call<Function, Builder> };
    return r;
    }

  template <typename Function> static Result buildCallSelf()
    {
    Result r = { call<Function, InvocationBuilder> };
    return r;
    }

  template <typename Function, typename Builder> static CanCallResult buildCanCall()
    {
    CanCallResult r = { canCall<Function, Builder> };
    return r;
    }

  template <typename Function, typename Builder> static void call(Reflect::example::Boxer *b, Arguments *data)
    {
    Call call = { data, b };
    Function::template call<Builder>(&call);
    }

  template <typename Function> static void callSelf(Reflect::example::Boxer *b, Arguments *data)
    {
    Call call = { data, b };
    Function::template call<InvocationBuilder>(&call);
    }

  template <typename Function, typename Builder> static bool canCall(Reflect::example::Boxer *b, Arguments *data)
    {
    Call call = { data, b };
    return Function::template canCall<Builder>(&call);
    }

  template <typename Function> static bool canCallSelf(Reflect::example::Boxer *b, Arguments *data)
    {
    Call call = { data, b };
    return Function::template canCall<InvocationBuilder>(&call);
    }
  };

void ReflectTest::methodWrapTest()
  {
  using namespace Reflect;
  typedef A ReflectClass;

  typedef FunctionSignature<decltype(&A::pork1)> Method1;
  typedef FunctionSignature<decltype(&A::pork2)> Method2;
  typedef FunctionSignature<decltype(&A::pork3)> Method3;

  QCOMPARE(Crate::findType<Method1::ReturnType>(), Crate::findType<void>());
  QCOMPARE(Crate::findType<Method2::ReturnType>(), Crate::findType<int>());
  QCOMPARE(Crate::findType<Method3::ReturnType>(), Crate::findType<A*>());

  QCOMPARE((int)std::tuple_size<Method1::Arguments>::value, 2);
  QCOMPARE((int)std::tuple_size<Method2::Arguments>::value, 1);
  QCOMPARE((int)std::tuple_size<Method3::Arguments>::value, 1);

  typedef std::tuple_element<0, Method1::Arguments>::type Method1Arg0;
  typedef std::tuple_element<1, Method1::Arguments>::type Method1Arg1;
  typedef std::tuple_element<0, Method2::Arguments>::type Method2Arg0;
  typedef std::tuple_element<0, Method3::Arguments>::type Method3Arg0;

  QCOMPARE(Crate::findType<Method1Arg0>(), Crate::findType<const float&>());
  QCOMPARE(Crate::findType<Method1Arg1>(), Crate::findType<double*>());
  QCOMPARE(Crate::findType<Method2Arg0>(), Crate::findType<A *>());
  QCOMPARE(Crate::findType<Method3Arg0>(), Crate::findType<const float &>());

  QCOMPARE(Method1::Const::value, false);
  QCOMPARE(Method2::Const::value, true);
  QCOMPARE(Method3::Const::value, false);

  QCOMPARE(Method1::Static::value, false);
  QCOMPARE(Method2::Static::value, false);
  QCOMPARE(Method3::Static::value, true);

  QCOMPARE(Crate::findType<Method1::Class>(), Crate::findType<A>());
  QCOMPARE(Crate::findType<Method2::Class>(), Crate::findType<A>());
  QCOMPARE(Crate::findType<Method3::Class>(), Crate::findType<void>());
  }

void ReflectTest::functionWrapTest()
  {
  using namespace Reflect;
  typedef FunctionSignature<decltype(&staticMethod)> Fn;

  A a;
  int b = INT_VAL;
  float c = FLOAT_VAL;

  auto inv1 = InvocationBuilder::buildCallSelf<FunctionCall<Fn, &staticMethod, InvocationBuilder>>();

  Reflect::example::Boxer boxer;

  Reflect::example::Object argVals[3];
  Reflect::example::Object *args1[3];
  Reflect::example::initArgs(&boxer, argVals, args1, a, b, c);
  InvocationBuilder::Arguments data1(args1, 3, nullptr);

  VERIFY_NO_THROWS([&]() { inv1.fn(&boxer, &data1); });
  }

void ReflectTest::methodInjectionTest()
  {
  using namespace Reflect;
  typedef FunctionSignature<decltype(&staticMethod)> Fn;

  int b = INT_VAL;
  float c = FLOAT_VAL;

  auto inv1 = MethodInjectorBuilder<InvocationBuilder>::buildWrappedCall<
    FunctionCall<Fn, &staticMethod, InvocationBuilder>, MethodInjectorBuilder<InvocationBuilder>>();

  Reflect::example::Boxer boxer;

  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, A());

  Reflect::example::Object argVals[2];
  Reflect::example::Object *args1[2];
  Reflect::example::initArgs(&boxer, argVals, args1, b, c);
  InvocationBuilder::Arguments data1(args1, 2, &thsVal);

  VERIFY_NO_THROWS([&]() { inv1.fn(&boxer, &data1); });
  }

void ReflectTest::functionInvokeTest()
  {
  using namespace Reflect;

  typedef FunctionSignature<decltype(&A::pork1)> Method1;
  typedef FunctionSignature<decltype(&A::pork2)> Method2;
  typedef FunctionSignature<decltype(&A::pork3)> Method3;

  auto inv1 = InvocationBuilder::buildCallSelf<FunctionCall<Method1, &A::pork1, InvocationBuilder>>();
  auto inv2 = InvocationBuilder::buildCallSelf<FunctionCall<Method2, &A::pork2, InvocationBuilder>>();
  auto inv3 = InvocationBuilder::buildCallSelf<FunctionCall<Method3, &A::pork3, InvocationBuilder>>();

  A a;
  a.pork = SELF_VAL;
  double dbl = DOUBLE_VAL;

  float flt = FLOAT_VAL;

  const float &arg11 = flt;
  double arg12 = dbl;
  A* arg21 = &a;
  const float &arg31 = flt;

  Reflect::example::Boxer boxer;

  Reflect::example::Object argVals1[2];
  Reflect::example::Object *args1[2];
  Reflect::example::initArgs(&boxer, argVals1, args1, arg11, arg12);

  Reflect::example::Object argVals2[1];
  Reflect::example::Object *args2[1];
  Reflect::example::initArgs(&boxer, argVals2, args2, arg21);

  Reflect::example::Object argVals3[1];
  Reflect::example::Object *args3[1];
  Reflect::example::initArgs(&boxer, argVals3, args3, arg31);

  A ths;
  ths.pork = SELF_VAL;

  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, &ths);

  InvocationBuilder::Arguments data1(args1, 2, thsValPtr);
  InvocationBuilder::Arguments data2(args2, 1, thsValPtr);
  InvocationBuilder::Arguments data2_1(args2, 1, 0);
  InvocationBuilder::Arguments data3(args3, 1, 0);

  VERIFY_NO_THROWS([&]() {
    inv1.fn(&boxer, &data1);
    inv2.fn(&boxer, &data2);
    inv3.fn(&boxer, &data3);
    });

  auto inv1WithArg2 = [&]()
    {
    inv1.fn(&boxer, &data2);
    };
  VERIFY_THROWS(inv1WithArg2, const Reflect::ArgCountException &, Reflect::ArgCountException(3, 2));

  auto inv2WithArg1 = [&]()
    {
    inv2.fn(&boxer, &data1);
    };
  VERIFY_THROWS(inv2WithArg1,
    const Reflect::ArgCountException &,
    Reflect::ArgCountException(2, 3));

  auto inv2WithNoThis = [&]()
    {
    inv2.fn(&boxer, &data2_1);
    };
  VERIFY_THROWS(inv2WithNoThis,
    const Reflect::ArgCountException &,
    Reflect::ArgCountException(2, 1));

  QVERIFY(data2.resultCount == 1);
  QVERIFY(data3.resultCount == 1);
  QCOMPARE(Reflect::example::Caster<int>::cast(&boxer, &data2.results[0]), INT_VAL);
  QCOMPARE(Reflect::example::Caster<A *>::cast(&boxer, &data3.results[0])->pork, SELF_VAL);
  }

void ReflectTest::multipleReturnTest()
  {
  //
  using namespace Reflect;

  typedef FunctionSignature<decltype(&A::multiReturn)> Fn;

  auto inv = InvocationBuilder::buildCallSelf<FunctionCall<Fn, &A::multiReturn, InvocationBuilder>>();

  Reflect::example::Boxer boxer;

  A ths;
  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, &ths);
  InvocationBuilder::Arguments data1(0, 0, thsValPtr);

  VERIFY_NO_THROWS([&]() { inv.fn(&boxer, &data1); });

  QVERIFY(data1.resultCount == 3);
  QVERIFY(data1.results[0].type == Crate::findType<int>());
  QVERIFY(data1.results[1].type == Crate::findType<float>());
  QVERIFY(data1.results[2].type == Crate::findType<double>());
  QVERIFY(data1.results[0].i == 5);
  QVERIFY(data1.results[1].f == 6.4f);
  QVERIFY(data1.results[2].db == 5.0);
  }

template <typename Builder, typename Sig, Sig Fn> class CanCallTestHelper
  {
public:
  template <typename T> static bool canCall(Reflect::example::Boxer *b, typename T::Arguments &data)
    {
    auto result = T::template buildCanCall<Reflect::FunctionCall<Builder, Fn, InvocationBuilder>, T>();
    return result.fn(b, &data);
    }
  };

void ReflectTest::canCallTest()
  {
  using namespace Reflect;
  typedef FunctionSignature<decltype(&A::pork1)> Method1;
  typedef FunctionSignature<decltype(&A::pork2)> Method2;
  typedef FunctionSignature<decltype(&A::pork3)> Method3;
  typedef FunctionSignature<decltype(&A::pork4)> Method4;


  Reflect::example::Boxer boxer;

  A a;

  A ths;
  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, &ths);

  Reflect::example::Object argVals1[2];
  Reflect::example::Object *argsP1[2];
  Reflect::example::initArgs(&boxer, argVals1, argsP1, 5.0f, 4.0);
  InvocationBuilder::Arguments args1(argsP1, 2, thsValPtr);

  Reflect::example::Object argVals2[1];
  Reflect::example::Object *argsP2[1];
  Reflect::example::initArgs(&boxer, argVals2, argsP2, &a);
  InvocationBuilder::Arguments args2(argsP2, 1, thsValPtr);

  Reflect::example::Object argVals3[1];
  Reflect::example::Object *argsP3[1];
  Reflect::example::initArgs(&boxer, argVals3, argsP3, 5.0f);
  InvocationBuilder::Arguments args3(argsP3, 1, nullptr);

  Reflect::example::Object argVals4[2];
  Reflect::example::Object *argsP4[2];
  Reflect::example::initArgs(&boxer, argVals4, argsP4, &a, 5.0f);
  InvocationBuilder::Arguments args4(argsP4, 2, thsValPtr);

  typedef CanCallTestHelper<Method1, Method1::Signature, &A::pork1> Helper1;
  typedef CanCallTestHelper<Method2, Method2::Signature, &A::pork2> Helper2;
  typedef CanCallTestHelper<Method3, Method3::Signature, &A::pork3> Helper3;
  typedef CanCallTestHelper<Method4, Method4::Signature, &A::pork4> Helper4;

  QVERIFY(Helper1::canCall<InvocationBuilder>(&boxer, args1));
  QVERIFY(!Helper2::canCall<InvocationBuilder>(&boxer, args1));
  QVERIFY(!Helper3::canCall<InvocationBuilder>(&boxer, args1));
  QVERIFY(!Helper4::canCall<InvocationBuilder>(&boxer, args1));

  QVERIFY(!Helper1::canCall<InvocationBuilder>(&boxer, args2));
  QVERIFY(Helper2::canCall<InvocationBuilder>(&boxer, args2));
  QVERIFY(!Helper3::canCall<InvocationBuilder>(&boxer, args2));
  QVERIFY(!Helper4::canCall<InvocationBuilder>(&boxer, args2));

  QVERIFY(!Helper1::canCall<InvocationBuilder>(&boxer, args3));
  QVERIFY(!Helper2::canCall<InvocationBuilder>(&boxer, args3));
  QVERIFY(Helper3::canCall<InvocationBuilder>(&boxer, args3));
  QVERIFY(!Helper4::canCall<InvocationBuilder>(&boxer, args3));

  QVERIFY(!Helper1::canCall<InvocationBuilder>(&boxer, args4));
  QVERIFY(!Helper2::canCall<InvocationBuilder>(&boxer, args4));
  QVERIFY(!Helper3::canCall<InvocationBuilder>(&boxer, args4));
  QVERIFY(Helper4::canCall<InvocationBuilder>(&boxer, args4));
  }

void ReflectTest::overloadingTest()
  {
  using namespace Reflect;
  typedef FunctionSignature<decltype(&A::pork1)> Method1;
  typedef FunctionSignature<decltype(&A::pork2)> Method2;
  typedef FunctionSignature<decltype(&A::pork3)> Method3;
  typedef FunctionSignature<decltype(&A::pork4)> Method4;

  Reflect::example::Boxer boxer;

  A a;
  a.pork = SELF_VAL;

  A ths;
  ths.pork = SELF_VAL;

  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, &ths);

  Reflect::example::Object argVals1[2];
  Reflect::example::Object *argsP1[2];
  Reflect::example::initArgs(&boxer, argVals1, argsP1, FLOAT_VAL, DOUBLE_VAL);
  InvocationBuilder::Arguments args1(argsP1, 2, thsValPtr);

  Reflect::example::Object argVals2[1];
  Reflect::example::Object *argsP2[1];
  Reflect::example::initArgs(&boxer, argVals2, argsP2, &a);
  InvocationBuilder::Arguments args2(argsP2, 1, thsValPtr);

  Reflect::example::Object argVals3[1];
  Reflect::example::Object *argsP3[1];
  Reflect::example::initArgs(&boxer, argVals3, argsP3, FLOAT_VAL);
  InvocationBuilder::Arguments args3(argsP3, 1, nullptr);

  Reflect::example::Object argVals4[2];
  Reflect::example::Object *argsP4[2];
  Reflect::example::initArgs(&boxer, argVals4, argsP4, &a, FLOAT_VAL);
  InvocationBuilder::Arguments args4(argsP4, 2, thsValPtr);

  Reflect::example::Object argVals5[1];
  Reflect::example::Object *argsP5[1];
  Reflect::example::initArgs(&boxer, argVals5, argsP5, false);
  InvocationBuilder::Arguments args5(argsP5, 1, thsValPtr);

  InvocationBuilder::Arguments args6(nullptr, 0, thsValPtr);

  typedef Reflect::FunctionArgumentTypeSelector<
    Reflect::FunctionCall<Method1, &A::pork1, InvocationBuilder>,
    Reflect::FunctionCall<Method4, &A::pork4, InvocationBuilder>> Overload1; // 3 args
  typedef Reflect::FunctionArgumentTypeSelector<
    Reflect::FunctionCall<Method2, &A::pork2, InvocationBuilder>> Overload2; // 2 arg
  typedef Reflect::FunctionArgumentTypeSelector<
      Reflect::FunctionCall<Method3, &A::pork3, InvocationBuilder>> Overload3; // 1 arg

  QVERIFY(InvocationBuilder::canCallSelf<Overload1>(&boxer, &args1));
  QVERIFY(!InvocationBuilder::canCallSelf<Overload1>(&boxer, &args2));
  QVERIFY(!InvocationBuilder::canCallSelf<Overload1>(&boxer, &args3));
  QVERIFY(InvocationBuilder::canCallSelf<Overload1>(&boxer, &args4));

  QVERIFY(!InvocationBuilder::canCallSelf<Overload2>(&boxer, &args1));
  QVERIFY(InvocationBuilder::canCallSelf<Overload2>(&boxer, &args2));
  QVERIFY(!InvocationBuilder::canCallSelf<Overload2>(&boxer, &args3));
  QVERIFY(!InvocationBuilder::canCallSelf<Overload2>(&boxer, &args4));
  QVERIFY(!InvocationBuilder::canCallSelf<Overload2>(&boxer, &args5));

  InvocationBuilder::Call args1Call = { &args1, &boxer };
  InvocationBuilder::Call args2Call = { &args2, &boxer };
  InvocationBuilder::Call args3Call = { &args3, &boxer };
  InvocationBuilder::Call args4Call = { &args4, &boxer };
  InvocationBuilder::Call args5Call = { &args5, &boxer };
  InvocationBuilder::Call args6Call = { &args6, &boxer };

  auto args2Excep = Reflect::OverloadException::build<InvocationBuilder, Overload1>(&args2Call);
  auto args3Excep = Reflect::OverloadException::build<InvocationBuilder, Overload1>(&args3Call);
  auto args5Excep = Reflect::OverloadException::build<InvocationBuilder, Overload1>(&args5Call);

  VERIFY_NO_THROWS([&]() { InvocationBuilder::callSelf<Overload1>(&boxer, &args1); });
  VERIFY_THROWS([&]() { InvocationBuilder::callSelf<Overload1>(&boxer, &args2); },
    const Reflect::OverloadException &,
    args2Excep);
  VERIFY_THROWS([&]() { InvocationBuilder::callSelf<Overload1>(&boxer, &args3); },
    const Reflect::OverloadException &,
    args3Excep);
  VERIFY_NO_THROWS([&]() { InvocationBuilder::callSelf<Overload1>(&boxer, &args4); });
  VERIFY_THROWS([&]() { InvocationBuilder::callSelf<Overload1>(&boxer, &args5); },
    const Reflect::OverloadException &,
    args5Excep);

  auto args5Excep2 = Reflect::OverloadException::build<InvocationBuilder, Overload2>(&args5Call);
  auto args4Excep2 = Reflect::OverloadException::build<InvocationBuilder, Overload2>(&args4Call);

  auto args1Excep = Reflect::OverloadException::build<InvocationBuilder, Overload2>(&args1Call);
  VERIFY_THROWS([&]() { InvocationBuilder::callSelf<Overload2>(&boxer, &args1); },
    const Reflect::OverloadException,
    args1Excep);
  VERIFY_NO_THROWS([&]() { InvocationBuilder::callSelf<Overload2>(&boxer, &args2); });
  VERIFY_NO_THROWS([&]() { InvocationBuilder::callSelf<Overload3>(&boxer, &args3); });
  VERIFY_THROWS([&]() { InvocationBuilder::callSelf<Overload2>(&boxer, &args4); },
    const Reflect::OverloadException,
    args4Excep2);
  VERIFY_THROWS([&]() { InvocationBuilder::callSelf<Overload2>(&boxer, &args5); },
    const Reflect::OverloadException &,
    args5Excep2);

  QCOMPARE(args5Excep2.what(),
"Unable to find overload matching passed arguments 'A ->( bool )'\n"
"Possibilities are: A ->( A )\n");


  typedef Reflect::FunctionArgumentCountSelector<
    Reflect::FunctionArgCountSelectorBlock<3, Overload1>,
    Reflect::FunctionArgCountSelectorBlock<2, Overload2>,
    Reflect::FunctionArgCountSelectorBlock<1, Overload3> > AllMethods;

  QVERIFY(InvocationBuilder::canCallSelf<AllMethods>(&boxer, &args1));
  QVERIFY(InvocationBuilder::canCallSelf<AllMethods>(&boxer, &args2));
  QVERIFY(InvocationBuilder::canCallSelf<AllMethods>(&boxer, &args3));
  QVERIFY(InvocationBuilder::canCallSelf<AllMethods>(&boxer, &args4));
  QVERIFY(InvocationBuilder::canCallSelf<AllMethods>(&boxer, &args5)); // length matches, types dont - throws in call.
  QVERIFY(InvocationBuilder::canCallSelf<AllMethods>(&boxer, &args6));

  VERIFY_NO_THROWS([&](){ InvocationBuilder::callSelf<AllMethods>(&boxer, &args1); });
  VERIFY_NO_THROWS([&](){ InvocationBuilder::callSelf<AllMethods>(&boxer, &args2); });
  VERIFY_NO_THROWS([&](){ InvocationBuilder::callSelf<AllMethods>(&boxer, &args3); });
  VERIFY_NO_THROWS([&](){ InvocationBuilder::callSelf<AllMethods>(&boxer, &args4); });
  VERIFY_THROWS([&](){ InvocationBuilder::callSelf<AllMethods>(&boxer, &args5); },
    const Reflect::OverloadException &,
    args5Excep2);


  auto args6Excep = Reflect::OverloadException::build<InvocationBuilder, Overload3>(&args6Call);
  VERIFY_THROWS([&](){ InvocationBuilder::callSelf<AllMethods>(&boxer, &args6); },
    const Reflect::OverloadException &,
    args6Excep);

  QCOMPARE(args6Excep.what(),
"Unable to find overload matching passed arguments 'A ->(  )'\n"
"Possibilities are: void ->( float )\n");

  }

int main(int argc, char *argv[])
{
    ReflectTest tc;
    return QTest::qExec(&tc, argc, argv);
}
