#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <QObject>
#include <QtTest>
#pragma clang diagnostic pop

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
  };
