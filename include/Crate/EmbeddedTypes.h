#pragma once
#include "Crate/Type.h"

namespace Crate
{
namespace detail
{

enum EmbeddedType
  {
  embedded_void,
  embedded_bool,
  embedded_int,
  embedded_float,
  embedded_double,

  EmbeddedTypeCount
  };

#define EMBEDDED_TYPE(type) \
  template <> struct TypeResolver<type> { \
  static const Type *find() { static Type t(#type); return &t; } };

EMBEDDED_TYPE(void)
EMBEDDED_TYPE(bool)
EMBEDDED_TYPE(int)
EMBEDDED_TYPE(float)
EMBEDDED_TYPE(double)
EMBEDDED_TYPE(char)
EMBEDDED_TYPE(char *)

#undef EMBEDDED_TYPE


}
}
