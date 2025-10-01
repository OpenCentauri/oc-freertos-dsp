#ifndef __AW_LOG_H
#define __AW_LOG_H

#include <xtensa/xtutil.h>

/*
 * When XOS debugging features are enabled, it uses the libxtutil
 * library for formatted output. Thus, this library must also be
 * linked into the application. This library provides functions
 * compatible with the C standard library (such as xt_printf()
 * instead of printf()), although not all features are supported.
 * In particular, xt_printf() does not support floating point
 * formats and some other format options. The libxtutil functions
 * are lightweight versions with much smaller code and data memory
 * requirements and they are also thread-safe. For more details on
 * included functions refer to the file xtensa/xtutil.h.
 */
#define printfFromISR	xt_printf

#endif  /* __AW_LOG_H */
