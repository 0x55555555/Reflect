#pragma once
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdeprecated-register"
#endif
#include <QObject>
#include <qtestcase.h>
#ifdef __clang__
# pragma clang diagnostic pop
#endif

class ReflectTest : public QObject
  {
  Q_OBJECT

public:
  ReflectTest()
    {
    }

  ~ReflectTest()
    {
    }

private Q_SLOTS:
  void methodWrapTest();
  void multipleReturnTest();
  void functionWrapTest();
  void methodInjectionTest();
  void functionInvokeTest();
  void typeCheckTest();
  void copyableTyperTest();
  void nonCopyableTyperTest();
  void nonCopyableNonCleanedTyperTest();
  void canCallTest();
  void overloadingTest();
  void parentingTest();
  };
