#pragma once
#include <tuple>

namespace Reflect
{
namespace detail
{

template <typename T, typename InvHelper> class ReturnPacker
  {
public:
  static void pack(typename InvHelper::CallData data, T &&result)
    {
    InvHelper::template packReturn<T>(data, std::move(result));
    }
  };


template <size_t Remaining, typename T, typename InvHelper> class TuplePacker
  {
public:
  static void pack(typename InvHelper::CallData d, T &&tup)
    {
    TuplePacker<Remaining-1, T, InvHelper>::pack(d, std::move(tup));

    typedef std::integral_constant<size_t, Remaining-1> Index;
    typedef typename std::tuple_element<Index::value, T>::type ElementType;

    ReturnPacker<ElementType, InvHelper>::pack(d, std::move(std::get<Index::value>(tup)));
    }
  };

template <typename T, typename InvHelper> class TuplePacker<0, T, InvHelper>
  {
public:
  static void pack(typename InvHelper::CallData, T &&)
    {
    }
  };

#if defined(_MSC_VER) && _MSC_VER < 1800

template <typename InvHelper, typename A, typename B, typename C, typename D> class ReturnPacker<std::tuple<A, B, C, D>, InvHelper>
  {
public:
  typedef std::tuple<A, B, C, D> Tuple;

  static void pack(typename InvHelper::CallData data, Tuple &&result)
    {
    TuplePacker<std::tuple_size<Tuple>::value, Tuple, InvHelper>::pack(data, std::move(result));
    }
  };

#else

template <typename InvHelper, typename... Args> class ReturnPacker<std::tuple<Args...>, InvHelper>
  {
public:
  typedef std::tuple<Args...> Tuple;
  static void pack(typename InvHelper::CallData data, std::tuple<Args...> &&result)
    {
    TuplePacker<std::tuple_size<Tuple>::value, Tuple, InvHelper>::pack(data, std::move(result));
    }
  };

#endif


}
}
