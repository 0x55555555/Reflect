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

}
}

#define EMBEDDED_TYPE(type) \
  namespace Crate { namespace detail { template <> struct TypeResolver<type> { \
  static const Type *find() { static Type t; t.initialise<type>(#type, nullptr); return &t; } }; } }

EMBEDDED_TYPE(void)
EMBEDDED_TYPE(bool)
EMBEDDED_TYPE(int)
EMBEDDED_TYPE(unsigned int)
EMBEDDED_TYPE(long)
EMBEDDED_TYPE(unsigned long)
EMBEDDED_TYPE(long long)
EMBEDDED_TYPE(unsigned long long)
EMBEDDED_TYPE(float)
EMBEDDED_TYPE(double)
EMBEDDED_TYPE(char)
EMBEDDED_TYPE(unsigned char)
EMBEDDED_TYPE(char *)
EMBEDDED_TYPE(wchar_t)
EMBEDDED_TYPE(wchar_t *)

#undef EMBEDDED_TYPE

