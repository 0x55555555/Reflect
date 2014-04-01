#pragma once
#include "QObject"

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
  void overloadingTest();
  };
