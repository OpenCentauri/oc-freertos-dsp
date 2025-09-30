#ifndef __SHARE_SPACE__
#define __SHARE_SPACE__

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <console.h>
#include <xtensa/tie/xt_hifi2.h>
#include <xtensa/config/core.h>
#include <xtensa/tie/xt_timer.h>
#include <imgdts.h>
#include <components/aw/linux_debug/debug_common.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "../../include/generic/usb_cdc.h" // usb_notify_ep0
#define SHARE_SPACE_SYNC 0


#define SYS_IDLE_STATE              0xcbda0001
#define DSP_READ_STATE           0xcbda0002
#define DSP_WRITE_STATE         0xcbda0003
#define ARM_READ_STATE           0xcbda0004
#define ARM_WRITE_STATE         0xcbda0005


#define SHARESPACE_READ 0
#define SHARESPACE_WRITE 1
extern uint16_t sharespace_arm_addr[2];
extern uint16_t sharespace_dsp_addr[2];

#define MESSAGE_MAX 64
#define MAX_PENDING_BLOCKS 12
struct receive_msg_block{
    int msg_len;
    char buf[MESSAGE_MAX];
    struct receive_msg_block *next;
};

struct receive_msg_queue {
    struct receive_msg_block *front, *rear;
};

void share_space_init(void);
int share_space_write(uint8_t*pstr, uint32_t size);
int share_space_read(uint8_t*pstr, uint32_t size);
void share_space_find_data_pack();

void receive_msg_queue_alloc();
void receive_msg_queue_free();
void receive_msg_queue_pop(struct receive_msg_queue* receive_msg, char* buf, int* len);
void receive_msg_queue_push(struct receive_msg_queue* receive_msg, char* buf, int* len);
int receive_msg_queue_isEmpty(struct receive_msg_queue* receive_msg);

#endif