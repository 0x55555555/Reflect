#pragma once
#include <tuple>

namespace Reflect
{

namespace detail
{

template <std::size_t Remaining, typename Visitor> class TupleVisitor
  {
public:
  static void visit(Visitor d)
    {
    TupleVisitor<Remaining-1, T, Visitor>::pack(d);

    typedef std::integral_constant<size_t, Remaining-1> Index;

    d.visit<Index>();
    }
  };

template <std::size_t Remaining, typename Visitor> class TupleVisitor<0, Visitor>
  {
public:
  static void visit(Visitor)
    {
    }
  };

}

template <typename Tuple, typename Visitor> class TupleEach
  {
  template <typename T> static operator()(Visitor &d)
    {
    detail::TupleVisitor<std::tuple_size<Tuple>::value>::visit(d);
    }
  };

}
