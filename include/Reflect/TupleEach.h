#pragma once
#include <tuple>

namespace Reflect
{

namespace detail
{

template <std::size_t Remaining, typename Visitor> class TupleVisitor
  {
public:
  static bool visit(Visitor &d)
    {
    typedef std::integral_constant<size_t, Remaining-1> Index;

    return TupleVisitor<Remaining-1, Visitor>::visit(d) ||
        d.template visit<Index::value>();
    }
  };

template <typename Visitor> class TupleVisitor<0, Visitor>
  {
public:
  static bool visit(Visitor &)
    {
    return false;
    }
  };

}

template <typename Tuple, typename Visitor> void tupleEach(Visitor &d)
  {
  detail::TupleVisitor<std::tuple_size<Tuple>::value, Visitor>::visit(d);
  }

}
