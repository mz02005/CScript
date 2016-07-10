#pragma once
#include "config.h"

#if _MSC_VER <= 1600

NOTSTD_API unsigned long long strtoull(const char *nptr, char **endptr, int base);

#endif
