#ifndef XREFLECTTEXT_H
#define XREFLECTTEXT_H

#include "QObject"

class EksReflectTest : public QObject
  {
  Q_OBJECT

public:
  EksReflectTest()
    {
    }

  ~EksReflectTest()
    {
    }

private Q_SLOTS:
  void functionWrapTest();
  void functionInvokeTest();
  };

#endif // XREFLECTTEXT_H
