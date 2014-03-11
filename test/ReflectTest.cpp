#include "ReflectTest.h"
#include "Reflect/EmbeddedTypes.h"
#include "Reflect/MethodInjectorBuilder.h"
#include "Reflect/FunctionBuilder.h"
#include <QtTest>

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

class InvocationBuilder
  {
public:
  class Arguments
    {
  public:
    void **_args;
    void *_this;
    void *_result;
    };
  typedef Arguments *CallData;

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

  template <typename T> static T getThis(CallData args)
    {
    return (T)args->_this;
    }

  template <std::size_t I, typename Arg>
      static Arg unpackArgument(CallData args)
    {
    typedef typename std::remove_reference<Arg>::type NoRef;
    return *(NoRef*)args->_args[I];
    }

  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    *(Return*)data->_result = result;
    }
  };


class MultiReturnInvocationBuilder : public InvocationBuilder
  {
public:
  class Arguments : public InvocationBuilder::Arguments
    {
  public:
    Arguments(void *ths)
      {
      _args = nullptr;
      _this = ths;
      resultIndex = 0;
      }

    QVariant results[3];
    size_t resultIndex;
    };
  typedef Arguments *CallData;

  struct Result
    {
    typedef void (*Signature)(CallData);
    Signature fn;
    };

  template <typename Return, typename T> static void packReturn(CallData data, T &&result)
    {
    data->results[data->resultIndex++] = QVariant::fromValue(result);
    }

  template <typename Builder> static Result build()
    {
    Result r = { MultiReturnInvocationBuilder::call<Builder> };
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

  void *args1[] = { (void*)&a, &b, &c };
  InvocationBuilder::Arguments data1 = { args1, nullptr, nullptr };

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

  void *args1[] = { &b, &c };
  InvocationBuilder::Arguments data1 = { args1, nullptr, nullptr };

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
  void *args1[] = { (void*)&arg11, &arg12 };
  void *args2[] = { &arg21 };
  void *args3[] = { (void*)&arg31 };

  int result2;
  A* result3;

  A ths;
  ths.pork = SELF_VAL;

  InvocationBuilder::Arguments data1 = { args1, &ths, nullptr };
  InvocationBuilder::Arguments data2 = { args2, &ths, &result2 };
  InvocationBuilder::Arguments data3 = { args3, &ths, &result3 };

  try
    {
    inv1.fn(&data1);
    inv2.fn(&data2);
    inv3.fn(&data3);
    }
  catch(...)
    {
    }

  QCOMPARE(result2, INT_VAL);
  QCOMPARE(result3->pork, SELF_VAL);
  }

void ReflectTest::multipleReturnTest()
  {
  //
  using namespace Reflect;
  typedef A ReflectClass;

  auto method = REFLECT_METHOD(multiReturn);

  auto inv = method.buildInvocation<MultiReturnInvocationBuilder>();

  A ths;
  MultiReturnInvocationBuilder::Arguments data1(&ths);

  try
    {
    inv.fn(&data1);
    }
  catch(...)
    {
    }

  QVERIFY(data1.resultIndex == 3);
  QVERIFY(data1.results[0].type() == QVariant::Int);
  QVERIFY(data1.results[1].type() == qMetaTypeId<float>());
  QVERIFY(data1.results[2].type() == QVariant::Double);
  QVERIFY(data1.results[0].toInt() == 5);
  QVERIFY(data1.results[1].toFloat() == 6.4f);
  QVERIFY(data1.results[2].toDouble() == 5.0);
  }

QTEST_APPLESS_MAIN(ReflectTest)
