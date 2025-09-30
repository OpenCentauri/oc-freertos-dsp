#ifndef __MYDEBUG_H
#define __MYDEBUG_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t



#define uart_dbg(fmt,...) printf(fmt, ##__VA_ARGS__)     //--G-G-2022-06-07-----
#define	DisTraceMsg() 	uart_dbg("###### File:%s Line:%d\r\n",__FILE__,__LINE__)
#define	DisErrorMsg() 	uart_dbg("###### Error!!!!!!File:%s Line:%d\r\n",__FILE__,__LINE__)
#define time_debug(a, b) printf("file :%s func :%s line :%d a :%d b :%s\n", __FILE__, __func__, __LINE__, a, b)
#define share_space_debug(a, b) printf("file :%s func :%s line :%d a :%d b :%s\n", __FILE__, __func__, __LINE__, a, b)
#define adc_debug(a, b) printf("file :%s func :%s line :%d  %s : %d\n", __FILE__, __func__, __LINE__, a, b)
#define GAM_DEBUG_printf(fmt,...) printf(fmt, ##__VA_ARGS__)
#define GAM_DEBUG_errer(fmt,...) printf(fmt, ##__VA_ARGS__)
#define share_space_printf(fmt,...) printf(fmt, ##__VA_ARGS__)
#define timer_printf(fmt,...) printf(fmt, ##__VA_ARGS__)
#define msgbox_printf(fmt,...) printf(fmt, ##__VA_ARGS__)


#endif // basecmd.h
