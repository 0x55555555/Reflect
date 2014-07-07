#pragma once

#include "Reflect/Reflect.h"

#ifdef REFLECT_MACRO_IMPL
# include "Reflect/Macro/FunctionBuilderImpl.h"
#else
# include "Reflect/Cpp11/FunctionBuilderImpl.h"
#endif
