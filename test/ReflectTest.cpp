#include "ReflectTest.h"
#include "Reflect/EmbeddedTypes.h"
#include "Reflect/MethodInjectorBuilder.h"
#include "Reflect/FunctionBuilder.h"
#include "../example/Default/Builder.h"
#include <QtTest>
#include <tuple>

#define FLOAT_VAL 5.0f
#define DOUBLE_VAL 5.0
#define INT_VAL 2222
#define SELF_VAL 500

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

  std::tuple<int, float, double> multiReturn()
    {
    return std::make_tuple(5, 6.4f, 5.0);
    }

  int pork;
  };

void staticMethod(A *a, int i, float e)
  {
  QCOMPARE_NO_RETURN(a == nullptr, true);
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
    typedef void (*Signature)(CallData);
    Signature fn;
    };

  template <typename Builder> static Result build()
    {
    Result r = { call<Builder> };
    return r;
    }

  template <typename Builder> static void call(CallData data)
    {
    Builder::call(data);
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

  A *a = nullptr;
  int b = INT_VAL;
  float c = FLOAT_VAL;

  auto inv1 = fn.buildInvocation<InvocationBuilder>();

  Reflect::example::Object argVals[3];
  Reflect::example::Object *args1[3];
  Reflect::example::initArgs(argVals, args1, a, b, c);
  InvocationBuilder::Arguments data1 = { args1, 3, nullptr, std::vector<Reflect::example::Object>() };

  try
    {
    inv1.fn(&data1);
    }
  catch(...)
    {
    }
  }

void ReflectTest::methodInjectionTest()
  {
  using namespace Reflect;
  auto fn = REFLECT_FUNCTION(staticMethod);

  int b = INT_VAL;
  float c = FLOAT_VAL;

  auto inv1 = fn.buildInvocation<MethodInjectorBuilder<InvocationBuilder>>();

  Reflect::example::Object argVals[2];
  Reflect::example::Caster<int>::pack(&argVals[0], b);
  Reflect::example::Caster<float>::pack(&argVals[1], c);
  Reflect::example::Object *args1[3] = { &argVals[0], &argVals[1] };
  InvocationBuilder::Arguments data1 = { args1, 3, nullptr, std::vector<Reflect::example::Object>() };

  try
    {
    inv1.fn(&data1);
    }
  catch(...)
    {
    }
  }

void ReflectTest::functionInvokeTest()
  {
  using namespace Reflect;
  typedef A ReflectClass;

  auto method1 = REFLECT_METHOD(pork1);
  auto method2 = REFLECT_METHOD(pork2);
  auto method3 = REFLECT_METHOD(pork3);

  auto inv1 = method1.buildInvocation<InvocationBuilder>();
  auto inv2 = method2.buildInvocation<InvocationBuilder>();
  auto inv3 = method3.buildInvocation<InvocationBuilder>();

  A a;
  a.pork = SELF_VAL;
  double dbl = DOUBLE_VAL;

  float flt = FLOAT_VAL;

  const float &arg11 = flt;
  double *arg12 = &dbl;
  A* arg21 = &a;
  const float &arg31 = flt;

  Reflect::example::Object argVals1[2];
  Reflect::example::Object *args1[2];
  Reflect::example::initArgs(argVals1, args1, arg11, arg12);

  Reflect::example::Object argVals2[1];
  Reflect::example::Object *args2[1];
  Reflect::example::initArgs(argVals2, args2, arg21);

  Reflect::example::Object argVals3[1];
  Reflect::example::Object *args3[1];
  Reflect::example::initArgs(argVals3, args3, arg31);

  A ths;
  ths.pork = SELF_VAL;

  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(thsVal, thsValPtr, &ths);

  InvocationBuilder::Arguments data1 = { args1, 2, thsValPtr, std::vector<Reflect::example::Object>() };
  InvocationBuilder::Arguments data2 = { args2, 1, thsValPtr, std::vector<Reflect::example::Object>() };
  InvocationBuilder::Arguments data3 = { args3, 1, thsValPtr, std::vector<Reflect::example::Object>() };

  try
    {
    inv1.fn(&data1);
    inv2.fn(&data2);
    inv3.fn(&data3);
    }
  catch(const Crate::TypeException &t)
    {
    qDebug() << t.what();
    QVERIFY(false);
    }
  catch(...)
    {
    QVERIFY(false);
    }

  QVERIFY(data2.results.size() == 1);
  QVERIFY(data3.results.size() == 1);
  QCOMPARE(Reflect::example::Caster<int>::cast(&data2.results[0]), INT_VAL);
  QCOMPARE(Reflect::example::Caster<A *>::cast(&data3.results[0])->pork, SELF_VAL);
  }

void ReflectTest::multipleReturnTest()
  {
  //
  using namespace Reflect;
  typedef A ReflectClass;

  auto method = REFLECT_METHOD(multiReturn);

  auto inv = method.buildInvocation<InvocationBuilder>();

  A ths;
  Reflect::example::Object thsVal;
  Reflect::example::Object *thsValPtr;
  Reflect::example::initArg(thsVal, thsValPtr, &ths);
  InvocationBuilder::Arguments data1 = { 0, 0, thsValPtr, std::vector<Reflect::example::Object>() };

  try
    {
    inv.fn(&data1);
    }
  catch(...)
    {
    }

  QVERIFY(data1.results.size() == 3);
  QVERIFY(data1.results[0].type == Reflect::findType<int>());
  QVERIFY(data1.results[1].type == Reflect::findType<float>());
  QVERIFY(data1.results[2].type == Reflect::findType<double>());
  QVERIFY(data1.results[0].i == 5);
  QVERIFY(data1.results[1].f == 6.4f);
  QVERIFY(data1.results[2].db == 5.0);
  }

QTEST_APPLESS_MAIN(ReflectTest)
