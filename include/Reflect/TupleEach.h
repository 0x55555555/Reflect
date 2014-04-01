#pragma once
#include <tuple>

namespace Reflect
{

namespace detail
{

template <std::size_t Remaining, typename Visitor> class TupleVisitor
  {
public:
  static void visit(Visitor &d)
    {
    TupleVisitor<Remaining-1, Visitor>::visit(d);

    typedef std::integral_constant<size_t, Remaining-1> Index;

    d.template visit<Index::value>();
    }
  };

template <typename Visitor> class TupleVisitor<0, Visitor>
  {
public:
  static void visit(Visitor &)
    {
    }
  };

}

template <typename Tuple, typename Visitor> void tupleEach(Visitor &d)
  {
  detail::TupleVisitor<std::tuple_size<Tuple>::value, Visitor>::visit(d);
  }

}
