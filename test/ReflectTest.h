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
  void functionWrapTest();
  void functionInvokeTest();
  void typeCheckTest();
  void copyableTyperTest();
  void nonCopyableTyperTest();
  void nonCopyableNonCleanedTyperTest();
  };
