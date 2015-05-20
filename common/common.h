#ifndef PS2AUTOTESTS_COMMON_H
#define PS2AUTOTESTS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

// Defines for MSVC highlighting.  Not intended to actually compile with msc.
#ifdef _MSC_VER
#define __STDC__
#define _EE
#define __attribute__(x)
#endif

#include <stdio.h>
#include <tamtypes.h>

void schedf(const char *format, ...);
void flushschedf();

#ifdef __cplusplus
}
#endif

#endif
