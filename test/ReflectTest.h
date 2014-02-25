#ifndef XREFLEXTEXT_H
#define XREFLEXTEXT_H

#include "QObject"

class EksReflexTest : public QObject
  {
  Q_OBJECT

public:
  EksReflexTest()
    {
    }

  ~EksReflexTest()
    {
    }

private Q_SLOTS:
  void functionWrapTest();
  void functionInvokeTest();
  void classWrapTest();
  };

#endif // XREFLEXTEXT_H
