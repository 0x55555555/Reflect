#pragma once
#include <tuple>

namespace Reflect
{

namespace detail
{

template <std::size_t Remaining, typename Tuple> struct StaticCombinerHelper
  {
  typedef std::integral_constant<size_t, Remaining-1> Index;
  typedef typename std::tuple_element<Index::value, Tuple>::type This;
  typedef StaticCombinerHelper<Remaining-1, Tuple> Next;

  typedef std::integral_constant<bool, Next::Value::value && This::Helper::Static::value> Value;
  };

template <typename Visitor> struct StaticCombinerHelper<0, Visitor>
  {
  typedef std::integral_constant<bool, true> Value;
  };

}

template <typename Tuple> struct StaticCombiner
  {
  typedef typename detail::StaticCombinerHelper<std::tuple_size<Tuple>::value, Tuple>::Value Value;
  };

}
