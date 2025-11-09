
/*
 * Copyright (c) 2001-2016 Cadence Design Systems, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef XTUTIL_H
#define XTUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>

/* parasoft-begin-suppress ALL "This file not MISRA checked." */

/*
 ** xt_putchar - writes a character to stdout. Equivalent to the C library
 ** putchar() call.
 **
 ** \param      c       Character to be written.
 **
 ** \return     Returns the character written, cast to int.
 */
extern int  xt_putchar(int c);

/*
 ** xt_puts - writes a string to stdout, appending a newline to the end.
 ** Equivalent to the C library puts() call.
 **
 ** \param      s       String to be written.
 **
 ** \return     Returns the number of characters written, not including the
 **             trailing newline.
 */
extern int  xt_puts(const char * s);

/*
 ** xt_putn - writes an unsigned integer to stdout (in decimal form).
 **
 ** \param      n       Number to be written.
 **
 ** \return     Returns nothing.
 */
extern void xt_putn(unsigned n);

/*
 ** xt_atoi - converts the input string into an integer. Does not check for
 ** overflow or sign.
 **
 ** \param      s       String to be converted.
 **
 ** \return     Returns the converted integer.
 */
extern int  xt_atoi(const char * s);

/*
 ** xt_printf - formatted output to stdout. Subset of C library printf()
 ** call, designed primarily for small size (secondarily for efficiency).
 ** Does not support any of the floating point format options.
 ** Only a common subset of printf formats and options are handled:
 **
 **     %[-+ ][0][width]i       decimal signed integer
 **     %[-+ ][0][width]d       decimal signed integer
 **     %[-][0][width]u         decimal unsigned integer
 **     %[-][0][width]x         hex unsigned integer
 **     %[-][0][width]X         hex unsigned integer (uppercase)
 **     %[-][0][width]p         hex unsigned integer with 0x prefix ("pointer")
 **     %[-][width]c            single character
 **     %[-][width]s            string
 **
 ** These modifiers are ignored (legally on 32-bit Xtensa):
 **     #           (alternate format)
 **     h           (short)                     expands to int on 32-bit Xtensa
 **     l           (long)                      same as int on 32-bit Xtensa
 **     j           (intmax_t or uintmax_t)     same as int on 32-bit Xtensa
 **     z           (size_t or ssize_t)         same as int on 32-bit Xtensa
 **     t           (ptrdiff_t)                 same as int on 32-bit Xtensa
 **
 ** Does NOT support:
 **     width.prec  (precision modifier)
 **     %o          (octal)
 **     %[L][feEgG] (floating point formats)
 **     %a %A       (hex floating point formats, C99)
 **     %C          (multibyte character)
 **     %S          (multibyte character string)
 **     %n          (returning count of character written)
 **     ll          (long long)
 **     q j z t     (other size modifiers, eg. see glibc)
 **
 **
 ** \param      format      Format string, subject to rules above.
 **
 ** \return     Returns the number of characters output.
 */
extern int  xt_printf(const char *format, ...);

/*
 ** xt_sprintf - formatted output to a string. Equivalent of C library
 ** sprintf() call. See xt_printf() description for format restrictions.
 **
 ** \param      str         Buffer for output.
 **
 ** \param      format      Format string, subject to rules above.
 **
 ** \return     Returns the number of characters output, not counting the
 **             trailing \0 used to terminate the output string.
 */
extern int  xt_sprintf(char * str, const char * format, ...);

/*
 ** xt_vprintf - formatted output to stdout, using va_list. Equivalent of
 ** C library vprintf() call. See xt_printf() description for format
 ** restrictions.
 **
 ** \param      format      Format string, subject to rules above.
 **
 ** \param      ap          List of arguments.
 **
 ** \return     Returns the number of characters output.
 */
extern int  xt_vprintf(const char *format, va_list ap);

/*
 ** xt_vsprintf - formatted output to a string, using va_list. Equivalent
 ** of C library vsprintf() call. See xt_printf() description for format
 ** restrictions.
 **
 ** \param      str         Buffer for output.
 **
 ** \param      format      Format string, subject to rules above.
 **
 ** \param      ap          List of arguments.
 **
 ** \return     Returns the number of characters output, not counting the
 **             trailing \0 used to terminate the output string.
 */
extern int  xt_vsprintf(char *str, const char *format, va_list ap);

/*
 ** xt_snprintf - formatted output to a string, limited by buffer size.
 ** Equivalent of C library snprintf() call. See xt_printf() description
 ** for format restrictions.
 **
 ** \param      str         Buffer for output.
 **
 ** \param      size        Maximum number of characters to write, including
 **                         the trailing \0 character.
 **
 ** \param      format      Format string, subject to rules above.
 **
 ** \return     Returns the number of characters output, not counting the
 **             trailing \0 used to terminate the output string.
 */
extern int  xt_snprintf(char *str, size_t size, const char *format, ...);

/*
 ** xt_vsnprintf - formatted output to a string, limited by buffer size.
 ** Equivalent of C library vsnprintf() call. See xt_printf() description
 ** for format restrictions.
 **
 ** \param      str         Buffer for output.
 **
 ** \param      size        Maximum number of characters to write, including
 **                         the trailing \0 character.
 **
 ** \param      format      Format string, subject to rules above.
 **
 ** \param      ap          List of arguments.
 **
 ** \return     Returns the number of characters output, not counting the
 **             trailing \0 used to terminate the output string.
 */
extern int  xt_vsnprintf(char *str, size_t size, const char *format, va_list ap);

typedef int xt_output_fn(int *, int, const void *, int);

/*
 ** xt_set_output_fn - sets the character output function used by libxtutil.
 ** The output function must be of type xt_output_fn, see the implementation
 ** of the default version in the libxtutil sources for an example.
 **
 ** \param      fn      Pointer to new output function.
 **
 ** \return     Returns a pointer to the previous output function.
 */
extern xt_output_fn * xt_set_output_fn(xt_output_fn * fn);

#ifdef XTUTIL_LIB

// Only defined if building library

typedef void (xt_outbuf_fn)(void *, char *, int);

extern int  _xt_vprint(xt_outbuf_fn * out, void * outarg, const char * fmt, va_list ap, int size);
extern void _xt_output(void *closure, char *buf, int cnt);

#else

// Only defined if building application and overriding

#ifndef XTUTIL_NO_OVERRIDE

#define putchar     xt_putchar
#define puts        xt_puts
#define putn        xt_putn
#define atoi        xt_atoi
#define printf      xt_printf
#define sprintf     xt_sprintf
#define vprintf     xt_vprintf
#define vsprintf    xt_vsprintf
#define snprintf    xt_snprintf
#define vsnprintf   xt_vsnprintf

#endif // XTUTIL_NO_OVERRIDE

#endif // XTUTIL_LIB

/* parasoft-end-suppress ALL "This file not MISRA checked." */

#ifdef __cplusplus
}
#endif

#endif // XTUTIL_H

