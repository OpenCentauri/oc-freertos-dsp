#ifndef __MSGBOXX_H__
#define __MSGBOXX_H__
#include "../../include/generic/usb_cdc.h" // usb_notify_ep0

int msgboxx_init();
int_fast8_t msgbox_read_bulk_out(void *data, uint_fast8_t max_len);
int_fast8_t msgbox_send_bulk_in(void *data, uint_fast8_t len);
extern uint16_t msgbox_new_msg[2];

#define MSGBOX_EN_IRQ 1
#define MSGBOX_EN 1
#define MSGBOX_EN_WR_OK 1
#define MSGBOX_EN_RD_OK 1
#define MSGBOX_EN_SYNC 0
void msgbox_receive_signal_cb();
void msgbox_send_signal(uint32_t data);
#endif