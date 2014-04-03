#include "ReflectTest.h"
#include "Reflect/EmbeddedTypes.h"
#include "Reflect/MethodInjectorBuilder.h"
#include "Reflect/FunctionBuilder.h"
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
  void pork1(const float& in, double* pork)
    {
    QCOMPARE_NO_RETURN(this->pork, SELF_VAL);
    QCOMPARE_NO_RETURN(in, FLOAT_VAL);
    QCOMPARE_NO_RETURN(*pork, DOUBLE_VAL);
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

namespace Reflect
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

  template <typename Builder> static Result build()
    {
    Result r = { call<Builder> };
    return r;
    }

  template <typename Builder> static CanCallResult buildCanCall()
    {
    CanCallResult r = { canCall<Builder> };
    return r;
    }

  template <typename Builder> static void call(Reflect::example::Boxer *b, Arguments *data)
    {
    Call call = { data, b };
    Builder::call(&call);
    }

  template <typename Builder> static bool canCall(Reflect::example::Boxer *b, Arguments *data)
    {
    Call call = { data, b };
    return Builder::canCall(&call);
    }
  };

void ReflectTest::methodWrapTest()
  {
  using namespace Reflect;
  typedef A ReflectClass;

  auto method1 = REFLECT_METHOD(pork1);
  auto method2 = REFLECT_METHOD(pork2);
  auto method3 = REFLECT_METHOD(pork3);

  QCOMPARE(method1.returnType(), findType<void>());
  QCOMPARE(method2.returnType(), findType<int>());
  QCOMPARE(method3.returnType(), findType<A*>());

  QCOMPARE((int)method1.argumentCount(), 2);
  QCOMPARE((int)method2.argumentCount(), 1);
  QCOMPARE((int)method3.argumentCount(), 1);

  QCOMPARE(method1.argumentType<0>(), findType<const float&>());
  QCOMPARE(method1.argumentType<1>(), findType<double*>());
  QCOMPARE(method2.argumentType<0>(), findType<A *>());
  QCOMPARE(method3.argumentType<0>(), findType<const float &>());

  typedef decltype(method1) Method1;
  typedef decltype(method2) Method2;
  typedef decltype(method3) Method3;

  QCOMPARE(Method1::Helper::Const::value, false);
  QCOMPARE(Method2::Helper::Const::value, true);
  QCOMPARE(Method3::Helper::Const::value, false);

  QCOMPARE(Method1::Helper::Static::value, false);
  QCOMPARE(Method2::Helper::Static::value, false);
  QCOMPARE(Method3::Helper::Static::value, true);

  QCOMPARE(findType<Method1::Helper::Class>(), findType<A>());
  QCOMPARE(findType<Method2::Helper::Class>(), findType<A>());
  QCOMPARE(findType<Method3::Helper::Class>(), findType<void>());
  }

void ReflectTest::functionWrapTest()
  {
  using namespace Reflect;
  auto fn = REFLECT_FUNCTION(staticMethod);

  A a;
  int b = INT_VAL;
  float c = FLOAT_VAL;

  auto inv1 = fn.buildCall<InvocationBuilder>();

  Reflect::example::Boxer boxer;

  Reflect::example::Object argVals[3];
  Reflect::example::Object *args1[3];
  Reflect::example::initArgs(&boxer, argVals, args1, a, b, c);
  InvocationBuilder::Arguments data1 = { args1, 3, nullptr, std::vector<Reflect::example::Object>() };

  VERIFY_NO_THROWS([&]() { inv1.fn(&boxer, &data1); });
  }

void ReflectTest::methodInjectionTest()
  {
  using namespace Reflect;
  auto fn = REFLECT_FUNCTION(staticMethod);

  int b = INT_VAL;
  float c = FLOAT_VAL;

  auto inv1 = fn.buildCall<MethodInjectorBuilder<InvocationBuilder>>();

  Reflect::example::Boxer boxer;

  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, A());

  Reflect::example::Object argVals[2];
  Reflect::example::Object *args1[2];
  Reflect::example::initArgs(&boxer, argVals, args1, b, c);
  InvocationBuilder::Arguments data1 = { args1, 2, &thsVal, std::vector<Reflect::example::Object>() };

  VERIFY_NO_THROWS([&]() { inv1.fn(&boxer, &data1); });
  }

void ReflectTest::functionInvokeTest()
  {
  using namespace Reflect;
  typedef A ReflectClass;

  auto method1 = REFLECT_METHOD(pork1);
  auto method2 = REFLECT_METHOD(pork2);
  auto method3 = REFLECT_METHOD(pork3);

  auto inv1 = method1.buildCall<InvocationBuilder>();
  auto inv2 = method2.buildCall<InvocationBuilder>();
  auto inv3 = method3.buildCall<InvocationBuilder>();

  A a;
  a.pork = SELF_VAL;
  double dbl = DOUBLE_VAL;

  float flt = FLOAT_VAL;

  const float &arg11 = flt;
  double *arg12 = &dbl;
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

  InvocationBuilder::Arguments data1 = { args1, 2, thsValPtr, std::vector<Reflect::example::Object>() };
  InvocationBuilder::Arguments data2 = { args2, 1, thsValPtr, std::vector<Reflect::example::Object>() };
  InvocationBuilder::Arguments data2_1 = { args2, 1, 0, std::vector<Reflect::example::Object>() };
  InvocationBuilder::Arguments data3 = { args3, 1, thsValPtr, std::vector<Reflect::example::Object>() };

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
    Crate::ArgException(Crate::TypeException(Reflect::findType<A>(), Reflect::findType<float>()), 0));

  auto inv2WithNoThis = [&]()
    {
    inv2.fn(&boxer, &data2_1);
    };
  VERIFY_THROWS(inv2WithNoThis,
    const Crate::ThisException &,
    Crate::ThisException(Crate::TypeException(Reflect::findType<A>(), Reflect::findType<void>())));

  QVERIFY(data2.results.size() == 1);
  QVERIFY(data3.results.size() == 1);
  QCOMPARE(Reflect::example::Caster<int>::cast(&boxer, &data2.results[0]), INT_VAL);
  QCOMPARE(Reflect::example::Caster<A *>::cast(&boxer, &data3.results[0])->pork, SELF_VAL);
  }

void ReflectTest::multipleReturnTest()
  {
  //
  using namespace Reflect;
  typedef A ReflectClass;

  auto method = REFLECT_METHOD(multiReturn);

  auto inv = method.buildCall<InvocationBuilder>();

  Reflect::example::Boxer boxer;

  A ths;
  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, &ths);
  InvocationBuilder::Arguments data1 = { 0, 0, thsValPtr, std::vector<Reflect::example::Object>() };

  VERIFY_NO_THROWS([&]() { inv.fn(&boxer, &data1); });

  QVERIFY(data1.results.size() == 3);
  QVERIFY(data1.results[0].type == Reflect::findType<int>());
  QVERIFY(data1.results[1].type == Reflect::findType<float>());
  QVERIFY(data1.results[2].type == Reflect::findType<double>());
  QVERIFY(data1.results[0].i == 5);
  QVERIFY(data1.results[1].f == 6.4f);
  QVERIFY(data1.results[2].db == 5.0);
  }

template <typename Builder> class CanCallTestHelper
  {
public:
  template <typename T> static bool canCall(Reflect::example::Boxer *b, typename T::Arguments data)
    {
    auto result = Builder().template buildCanCall<T>();
    return result.fn(b, &data);
    }
  };

void ReflectTest::canCallTest()
  {
  typedef Reflect::FunctionBuilder<decltype(&A::pork1), &A::pork1> Method1; // 2 args
  typedef Reflect::FunctionBuilder<decltype(&A::pork2), &A::pork2> Method2; // 1 arg
  typedef Reflect::FunctionBuilder<decltype(&A::pork3), &A::pork3> Method3; // 1 arg
  typedef Reflect::FunctionBuilder<decltype(&A::pork4), &A::pork4> Method4; // 2 args

  Reflect::example::Boxer boxer;

  A a;

  A ths;
  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, &ths);

  Reflect::example::Object argVals1[2];
  Reflect::example::Object *argsP1[2];
  Reflect::example::initArgs(&boxer, argVals1, argsP1, 5.0f, 4.0);
  InvocationBuilder::Arguments args1 = { argsP1, 2, thsValPtr, std::vector<Reflect::example::Object>() };

  Reflect::example::Object argVals2[1];
  Reflect::example::Object *argsP2[1];
  Reflect::example::initArgs(&boxer, argVals2, argsP2, &a);
  InvocationBuilder::Arguments args2 = { argsP2, 1, thsValPtr, std::vector<Reflect::example::Object>() };

  Reflect::example::Object argVals3[1];
  Reflect::example::Object *argsP3[1];
  Reflect::example::initArgs(&boxer, argVals3, argsP3, 5.0f);
  InvocationBuilder::Arguments args3 = { argsP3, 1, nullptr, std::vector<Reflect::example::Object>() };

  Reflect::example::Object argVals4[2];
  Reflect::example::Object *argsP4[2];
  Reflect::example::initArgs(&boxer, argVals4, argsP4, &a, 5.0f);
  InvocationBuilder::Arguments args4 = { argsP4, 2, thsValPtr, std::vector<Reflect::example::Object>() };

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
  typedef Reflect::FunctionBuilder<decltype(&A::pork1), &A::pork1> Method1; // 2 args
  typedef Reflect::FunctionBuilder<decltype(&A::pork2), &A::pork2> Method2; // 1 arg
  typedef Reflect::FunctionBuilder<decltype(&A::pork3), &A::pork3> Method3; // 1 arg
  typedef Reflect::FunctionBuilder<decltype(&A::pork4), &A::pork4> Method4; // 2 args

  Reflect::example::Boxer boxer;

  A a;
  a.pork = SELF_VAL;

  A ths;
  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(&boxer, thsVal, thsValPtr, &ths);

  Reflect::example::Object argVals1[2];
  Reflect::example::Object *argsP1[2];
  Reflect::example::initArgs(&boxer, argVals1, argsP1, FLOAT_VAL, DOUBLE_VAL);
  InvocationBuilder::Arguments args1 = { argsP1, 2, thsValPtr, std::vector<Reflect::example::Object>() };

  Reflect::example::Object argVals2[1];
  Reflect::example::Object *argsP2[1];
  Reflect::example::initArgs(&boxer, argVals2, argsP2, &a);
  InvocationBuilder::Arguments args2 = { argsP2, 1, thsValPtr, std::vector<Reflect::example::Object>() };

  Reflect::example::Object argVals3[1];
  Reflect::example::Object *argsP3[1];
  Reflect::example::initArgs(&boxer, argVals3, argsP3, FLOAT_VAL);
  InvocationBuilder::Arguments args3 = { argsP3, 1, nullptr, std::vector<Reflect::example::Object>() };

  Reflect::example::Object argVals4[2];
  Reflect::example::Object *argsP4[2];
  Reflect::example::initArgs(&boxer, argVals4, argsP4, &a, FLOAT_VAL);
  InvocationBuilder::Arguments args4 = { argsP4, 2, thsValPtr, std::vector<Reflect::example::Object>() };

  Reflect::example::Object argVals5[1];
  Reflect::example::Object *argsP5[1];
  Reflect::example::initArgs(&boxer, argVals5, argsP5, false);
  InvocationBuilder::Arguments args5 = { argsP5, 1, thsValPtr, std::vector<Reflect::example::Object>() };

  typedef Reflect::FunctionArgumentTypeSelector<InvocationBuilder, Method1, Method4> Overload1; // 2 args
  typedef Reflect::FunctionArgumentTypeSelector<InvocationBuilder, Method2, Method3> Overload2; // 1 arg

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


  /*typedef Reflect::FunctionArgCountSelector<InvocationBuilder,
    Reflect::FunctionArgCountBlock<2, Overload1>,
    Reflect::FunctionArgCountBlock<1, Overload2> > AllMethods;

  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args1));
  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args2));
  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args3));
  QVERIFY(InvocationBuilder::canCall<AllMethods>(&boxer, &args4));*/

  }

QTEST_APPLESS_MAIN(ReflectTest)
