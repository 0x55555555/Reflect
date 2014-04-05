#include "ReflectTest.h"
#include "Crate/EmbeddedTypes.h"
#include "Reflect/MethodInjectorBuilder.h"
#include "Reflect/WrappedFunction.h"
#include "Reflect/FunctionSelector.h"
#include "../example/Default/Builder.h"
#include <QtTest>
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
    static Type t("A");
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

  template <typename Function, typename Builder=InvocationBuilder> static Result buildCall()
    {
    Result r = { call<Function, Builder> };
    return r;
    }

  template <typename Function, typename Builder=InvocationBuilder> static CanCallResult buildCanCall()
    {
    CanCallResult r = { canCall<Function, Builder> };
    return r;
    }

  template <typename Function, typename Builder=InvocationBuilder> static void call(Reflect::example::Boxer *b, Arguments *data)
    {
    Call call = { data, b };
    Function::template call<Builder>(&call);
    }

  template <typename Function, typename Builder=InvocationBuilder> static bool canCall(Reflect::example::Boxer *b, Arguments *data)
    {
    Call call = { data, b };
    return Function::template canCall<Builder>(&call);
    }
  };

void ReflectTest::methodWrapTest()
  {
  using namespace Reflect;
  typedef A ReflectClass;

  typedef REFLECT_METHOD(pork1) Method1;
  typedef REFLECT_METHOD(pork2) Method2;
  typedef REFLECT_METHOD(pork3) Method3;

  QCOMPARE(Crate::findType<Method1::ReturnType>(), Crate::findType<void>());
  QCOMPARE(Crate::findType<Method2::ReturnType>(), Crate::findType<int>());
  QCOMPARE(Crate::findType<Method3::ReturnType>(), Crate::findType<A*>());

  QCOMPARE((int)Method1::argumentCount(), 2);
  QCOMPARE((int)Method2::argumentCount(), 1);
  QCOMPARE((int)Method3::argumentCount(), 1);

  typedef typename std::tuple_element<0, typename Method1::Arguments>::type Method1Arg0;
  typedef typename std::tuple_element<1, typename Method1::Arguments>::type Method1Arg1;
  typedef typename std::tuple_element<0, typename Method2::Arguments>::type Method2Arg0;
  typedef typename std::tuple_element<0, typename Method3::Arguments>::type Method3Arg0;

  QCOMPARE(Crate::findType<Method1Arg0>(), Crate::findType<const float&>());
  QCOMPARE(Crate::findType<Method1Arg1>(), Crate::findType<double*>());
  QCOMPARE(Crate::findType<Method2Arg0>(), Crate::findType<A *>());
  QCOMPARE(Crate::findType<Method3Arg0>(), Crate::findType<const float &>());

  QCOMPARE(Method1::Helper::Const::value, false);
  QCOMPARE(Method2::Helper::Const::value, true);
  QCOMPARE(Method3::Helper::Const::value, false);

  QCOMPARE(Method1::Helper::Static::value, false);
  QCOMPARE(Method2::Helper::Static::value, false);
  QCOMPARE(Method3::Helper::Static::value, true);

  QCOMPARE(Crate::findType<Method1::Helper::Class>(), Crate::findType<A>());
  QCOMPARE(Crate::findType<Method2::Helper::Class>(), Crate::findType<A>());
  QCOMPARE(Crate::findType<Method3::Helper::Class>(), Crate::findType<void>());
  }

void ReflectTest::functionWrapTest()
  {
  using namespace Reflect;
  typedef REFLECT_FUNCTION(staticMethod) Fn;

  A a;
  int b = INT_VAL;
  float c = FLOAT_VAL;

  auto inv1 = InvocationBuilder::buildCall<Fn::Builder>();

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
  typedef REFLECT_FUNCTION(staticMethod) Fn;

  int b = INT_VAL;
  float c = FLOAT_VAL;

  auto inv1 = MethodInjectorBuilder<InvocationBuilder>::buildCall<Fn::Builder>();

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
  typedef A ReflectClass;

  typedef REFLECT_METHOD(pork1) Method1;
  typedef REFLECT_METHOD(pork2) Method2;
  typedef REFLECT_METHOD(pork3) Method3;

  auto inv1 = InvocationBuilder::buildCall<Method1::Builder>();
  auto inv2 = InvocationBuilder::buildCall<Method2::Builder>();
  auto inv3 = InvocationBuilder::buildCall<Method3::Builder>();

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
  InvocationBuilder::Arguments data3(args3, 1, thsValPtr);

  VERIFY_NO_THROWS([&]() {
    inv1.fn(&boxer, &data1);
    inv2.fn(&boxer, &data2);
    inv3.fn(&boxer, &data3);
    });

  auto inv1WithArg2 = [&]()
    {
    inv1.fn(&boxer, &data2);
    };
  VERIFY_THROWS(inv1WithArg2, const Reflect::ArgCountException &, Reflect::ArgCountException(2, 1));

  auto inv2WithArg1 = [&]()
    {
    inv2.fn(&boxer, &data1);
    };
  VERIFY_THROWS(inv2WithArg1,
    const Crate::ArgException &,
    Crate::ArgException(Crate::TypeException(Crate::findType<A>(), Crate::findType<float>()), 0));

  auto inv2WithNoThis = [&]()
    {
    inv2.fn(&boxer, &data2_1);
    };
  VERIFY_THROWS(inv2WithNoThis,
    const Crate::ThisException &,
    Crate::ThisException(Crate::TypeException(Crate::findType<A>(), Crate::findType<void>())));

  QVERIFY(data2.resultCount == 1);
  QVERIFY(data3.resultCount == 1);
  QCOMPARE(Reflect::example::Caster<int>::cast(&boxer, &data2.results[0]), INT_VAL);
  QCOMPARE(Reflect::example::Caster<A *>::cast(&boxer, &data3.results[0])->pork, SELF_VAL);
  }

void ReflectTest::multipleReturnTest()
  {
  //
  using namespace Reflect;
  typedef A ReflectClass;

  typedef REFLECT_METHOD(multiReturn) Method;

  auto inv = InvocationBuilder::buildCall<Method::Builder>();

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

template <typename Builder> class CanCallTestHelper
  {
public:
  template <typename T> static bool canCall(Reflect::example::Boxer *b, typename T::Arguments &data)
    {
    auto result = T::template buildCanCall<typename Builder::Builder>();
    return result.fn(b, &data);
    }
  };

void ReflectTest::canCallTest()
  {
  REFLECT_FUNCTION_HELPER(A);
  typedef REFLECT_METHOD(pork1) Method1; // 2 args
  typedef REFLECT_METHOD(pork2) Method2; // 1 arg
  typedef REFLECT_METHOD(pork3) Method3; // 1 arg
  typedef REFLECT_METHOD(pork4) Method4; // 2 args

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

  QVERIFY(CanCallTestHelper<Method1>::canCall<InvocationBuilder>(&boxer, args1));
  QVERIFY(!CanCallTestHelper<Method2>::canCall<InvocationBuilder>(&boxer, args1));
  QVERIFY(CanCallTestHelper<Method3>::canCall<InvocationBuilder>(&boxer, args1));
  QVERIFY(!CanCallTestHelper<Method4>::canCall<InvocationBuilder>(&boxer, args1));

  QVERIFY(!CanCallTestHelper<Method1>::canCall<InvocationBuilder>(&boxer, args2));
  QVERIFY(CanCallTestHelper<Method2>::canCall<InvocationBuilder>(&boxer, args2));
  QVERIFY(!CanCallTestHelper<Method3>::canCall<InvocationBuilder>(&boxer, args2));
  QVERIFY(!CanCallTestHelper<Method4>::canCall<InvocationBuilder>(&boxer, args2));

  QVERIFY(!CanCallTestHelper<Method1>::canCall<InvocationBuilder>(&boxer, args3));
  QVERIFY(!CanCallTestHelper<Method2>::canCall<InvocationBuilder>(&boxer, args3));
  QVERIFY(CanCallTestHelper<Method3>::canCall<InvocationBuilder>(&boxer, args3));
  QVERIFY(!CanCallTestHelper<Method4>::canCall<InvocationBuilder>(&boxer, args3));

  QVERIFY(!CanCallTestHelper<Method1>::canCall<InvocationBuilder>(&boxer, args4));
  QVERIFY(CanCallTestHelper<Method2>::canCall<InvocationBuilder>(&boxer, args4));
  QVERIFY(!CanCallTestHelper<Method3>::canCall<InvocationBuilder>(&boxer, args4));
  QVERIFY(CanCallTestHelper<Method4>::canCall<InvocationBuilder>(&boxer, args4));
  }

void ReflectTest::overloadingTest()
  {
  REFLECT_FUNCTION_HELPER(A);
  typedef REFLECT_METHOD(pork1) Method1; // 2 args
  typedef REFLECT_METHOD(pork2) Method2; // 1 arg
  typedef REFLECT_METHOD(pork3) Method3; // 1 arg
  typedef REFLECT_METHOD(pork4) Method4; // 2 args

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

  typedef Reflect::FunctionArgumentTypeSelector<Method1::Builder, Method4::Builder> Overload1; // 2 args
  typedef Reflect::FunctionArgumentTypeSelector<Method2::Builder, Method3::Builder> Overload2; // 1 arg

  QVERIFY(InvocationBuilder::canCall<Overload1>(&boxer, &args1));
  QVERIFY(!InvocationBuilder::canCall<Overload1>(&boxer, &args2));
  QVERIFY(!InvocationBuilder::canCall<Overload1>(&boxer, &args3));
  QVERIFY(InvocationBuilder::canCall<Overload1>(&boxer, &args4));

  QVERIFY(InvocationBuilder::canCall<Overload2>(&boxer, &args1));
  QVERIFY(InvocationBuilder::canCall<Overload2>(&boxer, &args2));
  QVERIFY(InvocationBuilder::canCall<Overload2>(&boxer, &args3));
  QVERIFY(InvocationBuilder::canCall<Overload2>(&boxer, &args4));
  QVERIFY(!InvocationBuilder::canCall<Overload2>(&boxer, &args5));

  InvocationBuilder::Call args2Call = { &args2, &boxer };
  InvocationBuilder::Call args3Call = { &args3, &boxer };
  InvocationBuilder::Call args5Call = { &args5, &boxer };

  auto args2Excep = Reflect::OverloadException::build<InvocationBuilder, Overload1>(&args2Call);
  auto args3Excep = Reflect::OverloadException::build<InvocationBuilder, Overload1>(&args3Call);
  auto args5Excep = Reflect::OverloadException::build<InvocationBuilder, Overload1>(&args5Call);

  VERIFY_NO_THROWS([&]() { InvocationBuilder::call<Overload1>(&boxer, &args1); });
  VERIFY_THROWS([&]() { InvocationBuilder::call<Overload1>(&boxer, &args2); },
    const Reflect::OverloadException &,
    args2Excep);
  VERIFY_THROWS([&]() { InvocationBuilder::call<Overload1>(&boxer, &args3); },
    const Reflect::OverloadException &,
    args3Excep);
  VERIFY_NO_THROWS([&]() { InvocationBuilder::call<Overload1>(&boxer, &args4); });
  VERIFY_THROWS([&]() { InvocationBuilder::call<Overload1>(&boxer, &args5); },
    const Reflect::OverloadException &,
    args5Excep);

  auto args5Excep2 = Reflect::OverloadException::build<InvocationBuilder, Overload2>(&args5Call);

  VERIFY_NO_THROWS([&]() { InvocationBuilder::call<Overload2>(&boxer, &args1); });
  VERIFY_NO_THROWS([&]() { InvocationBuilder::call<Overload2>(&boxer, &args2); });
  VERIFY_NO_THROWS([&]() { InvocationBuilder::call<Overload2>(&boxer, &args3); });
  VERIFY_NO_THROWS([&]() { InvocationBuilder::call<Overload2>(&boxer, &args4); });
  VERIFY_THROWS([&]() { InvocationBuilder::call<Overload2>(&boxer, &args5); },
    const Reflect::OverloadException &,
  args5Excep2);

  QCOMPARE(args5Excep2.what(),
"Unable to find overload matching passed arguments 'A ->( bool )'\n"
"Possibilities are: A ->( A )\n"
"void ->( float )\n");


  typedef Reflect::FunctionArgumentCountSelector<
    Reflect::FunctionArgCountSelectorBlock<2, Overload1>,
    Reflect::FunctionArgCountSelectorBlock<1, Overload2> > AllMethods;

  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args1));
  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args2));
  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args3));
  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args4));
  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args5)); // length matches, types dont - throws in call.

  VERIFY_NO_THROWS([&](){ InvocationBuilder::call<AllMethods>(&boxer, &args1); });
  VERIFY_NO_THROWS([&](){ InvocationBuilder::call<AllMethods>(&boxer, &args2); });
  VERIFY_NO_THROWS([&](){ InvocationBuilder::call<AllMethods>(&boxer, &args3); });
  VERIFY_NO_THROWS([&](){ InvocationBuilder::call<AllMethods>(&boxer, &args4); });
  VERIFY_THROWS([&](){ InvocationBuilder::call<AllMethods>(&boxer, &args5); },
    const Reflect::OverloadException &,
    args5Excep2);

  }

QTEST_APPLESS_MAIN(ReflectTest)
