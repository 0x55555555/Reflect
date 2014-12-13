#include "ReflectTest.h"
#include "Crate/Traits.h"
#include <memory>
#include "../example/Default/Builder.h"

static int ctorCount = 0;
static int copyCount = 0;
static int dtorCount = 0;

class Vector3
  {
public:
  Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
    {
    ++ctorCount;
    }

  Vector3(const Vector3 &copy) : x(copy.x), y(copy.y), z(copy.z)
    {
    ++copyCount;
    }

  ~Vector3()
    {
    ++dtorCount;
    }

  bool operator==(const Vector3 &v) const
    {
    return x == v.x && y == v.y && z == v.z;
    }

  float x;
  float y;
  float z;
  };


class Vector3Better : public Vector3
  {
public:
  Vector3Better() : Vector3(0,0,0) { }
  };

class NonCopyable
  {
public:
  NonCopyable(const Vector3 &v)
    {
    ++ctorCount;
    m_ptr = std::unique_ptr<Vector3>(new Vector3(v));
    }

  ~NonCopyable()
    {
    ++dtorCount;
    }

  std::unique_ptr<Vector3> m_ptr;
  };

class NonCopyableReferencable
  {
public:
  NonCopyableReferencable(const Vector3 &v)
    {
    ++ctorCount;
    m_ptr = std::unique_ptr<Vector3>(new Vector3(v));
    }

  ~NonCopyableReferencable()
    {
    ++dtorCount;
    }

  std::unique_ptr<Vector3> m_ptr;
  };

class PrivateConstructorDestructor
  {
  int data;

private:
  PrivateConstructorDestructor() { }
  PrivateConstructorDestructor(const PrivateConstructorDestructor &) { }
  PrivateConstructorDestructor &operator=(const PrivateConstructorDestructor &) { return *this; }
  ~PrivateConstructorDestructor() { }

  friend class ReflectTest;
  };

namespace Crate
{
namespace detail
{

template <> struct TypeResolver<Vector3>
  {
  static const Type *find()
    {
    static Type t;
    t.initialise<Vector3>("Vector3", nullptr);
    return &t;
    }
  };


template <> struct TypeResolver<Vector3Better>
  {
  static const Type *find()
    {
    static Type t;
    t.initialise<Vector3Better>("Vector3Better", findType<Vector3>());
    return &t;
    }
  };

template <> struct TypeResolver<NonCopyable>
  {
  static const Type *find()
    {
    static Type t;
    t.initialise<NonCopyable>("NonCopyable", nullptr);
    return &t;
    }
  };

template <> struct TypeResolver<NonCopyableReferencable>
  {
  static const Type *find()
    {
    static Type t;
    t.initialise<NonCopyableReferencable>("NonCopyableReferencable", nullptr);
    return &t;
    }
  };

template <> struct TypeResolver<PrivateConstructorDestructor>
  {
  static const Type *find()
    {
    static Type t;
    t.initialise<PrivateConstructorDestructor>("PrivateConstructorDestructor", nullptr);
    return &t;
    }
  };

}
}

namespace Crate
{
template <> class Traits<NonCopyable> : public ReferenceTraits<NonCopyable>
  {
  };
template <> class Traits<NonCopyableReferencable> : public ReferenceNonCleanedTraits<NonCopyableReferencable>
  {
  };
template <> class Traits<PrivateConstructorDestructor> : public ReferenceNonCleanedTraits<PrivateConstructorDestructor>
  {
  };
}

typedef Crate::Traits<Vector3> Vector3Traits;
typedef Crate::Traits<Vector3Better> Vector3BetterTraits;
typedef Crate::Traits<NonCopyable> NonCopyableTraits;
typedef Crate::Traits<NonCopyableReferencable> NonCopyableReferencableTraits;
typedef Crate::Traits<PrivateConstructorDestructor> PrivateConstructorDestructorTraits;

void ReflectTest::typeCheckTest()
  {
  ctorCount = 0;
  dtorCount = 0;

  Reflect::example::Boxer boxer;
    {
    auto vec3 = boxer.create<Vector3Traits>();

    Vector3 vec3Data(1, 2, 3);

    QCOMPARE(ctorCount, 1);
    QCOMPARE(dtorCount, 0);
    Vector3Traits::box(&boxer, vec3.get(), &vec3Data);

    Vector3 *v = Vector3Traits::unbox(&boxer, vec3.get());
    QVERIFY(v != &vec3Data);
    QCOMPARE(*v, vec3Data);

    bool caught = false;
    try
      {
      auto empty = NonCopyableTraits::unbox(&boxer, vec3.get());
      QVERIFY(!empty);
      }
    catch(...)
      {
      caught = true;
      }
    QVERIFY(caught);

    caught = false;
    try
      {
      auto empty = NonCopyableReferencableTraits::unbox(&boxer, vec3.get());
      QVERIFY(!empty);
      }
    catch(...)
      {
      caught = true;
      }
    QVERIFY(caught);

    vec3 = nullptr;

    QCOMPARE(ctorCount, 1);
    QCOMPARE(dtorCount, 1);
    }
  }

void ReflectTest::copyableTyperTest()
  {
  ctorCount = 0;
  dtorCount = 0;

  Reflect::example::Boxer boxer;
    {
    auto vec3 = boxer.create<Vector3Traits>();

    Vector3 vec3Data(1, 2, 3);

    QCOMPARE(ctorCount, 1);
    QCOMPARE(dtorCount, 0);
    Vector3Traits::box(&boxer, vec3.get(), &vec3Data);

    Vector3 *v = Vector3Traits::unbox(&boxer, vec3.get());
    QVERIFY(v != &vec3Data);
    QCOMPARE(*v, vec3Data);

    bool caught = false;
    try
      {
      auto empty = NonCopyableTraits::unbox(&boxer, vec3.get());
      QVERIFY(!empty);
      }
    catch(...)
      {
      caught = true;
      }
    QVERIFY(caught);

    caught = false;
    try
      {
      auto empty = NonCopyableReferencableTraits::unbox(&boxer, vec3.get());
      QVERIFY(!empty);
      }
    catch(...)
      {
      caught = true;
      }
    QVERIFY(caught);

    vec3 = nullptr;

    QCOMPARE(ctorCount, 1);
    QCOMPARE(dtorCount, 1);
    }
  }

void ReflectTest::nonCopyableTyperTest()
  {
  ctorCount = 0;
  copyCount = 0;
  dtorCount = 0;

  Reflect::example::Boxer boxer;
    {
    auto nonCopyable = boxer.create<NonCopyableTraits>();

    Vector3 vec(1, 2, 3);

    QCOMPARE(ctorCount, 1);
    QCOMPARE(copyCount, 0);
    QCOMPARE(dtorCount, 0);
    auto nonCopyData = new NonCopyable(vec);

    QCOMPARE(ctorCount, 2);
    QCOMPARE(copyCount, 1);
    QCOMPARE(dtorCount, 0);
    NonCopyableTraits::box(&boxer, nonCopyable.get(), nonCopyData);

    QCOMPARE(ctorCount, 2);
    QCOMPARE(copyCount, 1);
    QCOMPARE(dtorCount, 0);

    auto v = NonCopyableTraits::unbox(&boxer, nonCopyable.get());
    QCOMPARE(v, nonCopyData);

    bool caught = false;
    try
      {
      auto empty = Vector3Traits::unbox(&boxer, nonCopyable.get());
      QVERIFY(!empty);
      }
    catch(...)
      {
      caught = true;
      }
    QVERIFY(caught);

    caught = false;
    try
      {
      auto empty = NonCopyableReferencableTraits::unbox(&boxer, nonCopyable.get());
      QVERIFY(!empty);
      }
    catch(...)
      {
      caught = true;
      }
    QVERIFY(caught);

    QCOMPARE(ctorCount, 2);
    QCOMPARE(copyCount, 1);
    QCOMPARE(dtorCount, 0);

    nonCopyable = nullptr;
    QCOMPARE(dtorCount, 2);
    }

  QCOMPARE(ctorCount, 2);
  QCOMPARE(copyCount, 1);
  QCOMPARE(dtorCount, 3);
  }


void ReflectTest::nonCopyableNonCleanedTyperTest()
  {
  ctorCount = 0;
  copyCount = 0;
  dtorCount = 0;

  Reflect::example::Boxer boxer;
    {
    auto nonCopyable = boxer.create<NonCopyableReferencableTraits>();

    Vector3 vec(1, 2, 3);

    QCOMPARE(ctorCount, 1);
    QCOMPARE(copyCount, 0);
    QCOMPARE(dtorCount, 0);
    NonCopyableReferencable nonCopyData(vec);

    QCOMPARE(ctorCount, 2);
    QCOMPARE(copyCount, 1);
    QCOMPARE(dtorCount, 0);
    NonCopyableReferencableTraits::box(&boxer, nonCopyable.get(), &nonCopyData);

    auto v = NonCopyableReferencableTraits::unbox(&boxer, nonCopyable.get());
    QCOMPARE(v, &nonCopyData);

    bool caught = false;
    try
      {
      auto empty = Vector3Traits::unbox(&boxer, nonCopyable.get());
      QVERIFY(!empty);
      }
    catch(...)
      {
      caught = true;
      }
    QVERIFY(caught);

    caught = false;
    try
      {
      auto empty = NonCopyableTraits::unbox(&boxer, nonCopyable.get());
      QVERIFY(!empty);
      }
    catch(...)
      {
      caught = true;
      }
    QVERIFY(caught);


    QCOMPARE(ctorCount, 2);
    QCOMPARE(copyCount, 1);
    QCOMPARE(dtorCount, 0);

    nonCopyable = nullptr;
    }

  QCOMPARE(ctorCount, 2);
  QCOMPARE(copyCount, 1);
  QCOMPARE(dtorCount, 3);
  }

void ReflectTest::parentingTest()
  {
  auto vec3 = Crate::findType<Vector3>();
  auto vec3Better = Crate::findType<Vector3Better>();

  QVERIFY(vec3);
  QVERIFY(vec3Better);

  QVERIFY(!vec3->parent());
  QVERIFY(vec3Better->parent() == vec3);

  Vector3Better vec;

  Reflect::example::Boxer boxer;
  auto vec3Obj = boxer.create<Vector3Traits>();
  Vector3Traits::box(&boxer, vec3Obj.get(), &vec);
  auto vec3BetterObj = boxer.create<Vector3BetterTraits>();
  Vector3BetterTraits::box(&boxer, vec3BetterObj.get(), &vec);

  QVERIFY(Vector3Traits::unbox(&boxer, vec3Obj.get()));

  bool caught = false;
  try
    {
    Vector3BetterTraits::unbox(&boxer, vec3Obj.get());
    }
  catch(...)
    {
    caught = true;
    }
  QVERIFY(caught);

  QVERIFY(Vector3Traits::unbox(&boxer, vec3BetterObj.get()));
  QVERIFY(Vector3BetterTraits::unbox(&boxer, vec3BetterObj.get()));
  }

void ReflectTest::privateConstructorClassTest()
  {
  Reflect::example::Boxer boxer;
  auto obj = boxer.create<PrivateConstructorDestructorTraits>();

  PrivateConstructorDestructor test;

  PrivateConstructorDestructorTraits::box(&boxer, obj.get(), &test);
  }
