#include <FreeRTOS.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <awlog.h>
#include <hal_msgbox.h>

#include <console.h>

#include "../../include/bus/msgboxx.h"
#include "../../include/printer/basecmd.h"
#include "../../include/bus/share_space.h"
#include "../../include/board/gpio.h"

#define RECEIVE_QUEUE_LENGTH 1024
#define RECEIVE_QUEUE_WAIT_MS 100

struct msgbox_demo {
	int remote_id;
	int read_ch;
	int write_ch;
};

#if MSGBOX_EN_IRQ
#endif
uint16_t msgbox_new_msg[2];

 int msgboxx_recv_callback(unsigned long data, void *private_data)         //---msgbox-G-G-2022-07-08---
{
    msgbox_new_msg[0] = (uint16_t)((data << 16) >> 16);
	msgbox_new_msg[1] = (uint16_t)(data >> 16);
    // gpio_out_toggle_noirq(&g_GAM_DBG_gpio2);
	usb_notify_bulk_out();
    // usb_notify_bulk_in();
    return 0;
}

struct msgbox_demo demo= {
		.remote_id = 0,
		.read_ch = 3,
		.write_ch = 3,
};
struct msg_endpoint ept;

int msgboxx_init()
{
    int ret = 0;
	printf("remote id: %d, write channel: %d, read channel: %d\n",demo.remote_id, demo.write_ch, demo.read_ch);

    ept.rec = (void *)msgboxx_recv_callback;            //---msgbox-G-G-2022-07-08---
    ept.private = &demo;
	// hal_msgbox_free_all_channel();
	ret = hal_msgbox_alloc_channel(&ept, demo.remote_id, demo.read_ch, demo.write_ch);
    printf("allocate msgbox channel ret %d\n", ret);
	if (ret != 0) {
		printf("Failed to allocate msgbox channel\n");
        return -1;
	}
}
#if MSGBOX_EN_IRQ
#else
int msgboxx_check()
{
    check_msgbox_handler(&ept);
}
#endif
FINSH_FUNCTION_EXPORT_CMD(msgboxx_init, msgboxx_init, msgboxx init);


void msgbox_send_signal(uint32_t data)
{
   
   
    int ret = hal_msgbox_channel_send(&ept, (unsigned char *)&data, 4);
    if (ret != 0) {
        printf("Failed to send message %d\n", ret);
    }
}

void msgbox_receive_signal_cb()
{
}