#include "FreeRTOS.h"
#include "msgboxx.h"

struct messagebox *msgbox_dsp;
struct messagebox *msgbox_cpu;

uint32_t hal_msgbox_init(void) {
    return;
}

uint32_t hal_msgbox_alloc_channel(struct msg_endpoint *edp, uint32_t remote,
                              uint32_t read, uint32_t write) {
    return 1;
}

uint32_t hal_msgbox_channel_send(struct msg_endpoint *edp, uint8_t *bf,
                             uint32_t len) {
    return 1;
}

void hal_msgbox_free_channel(struct msg_endpoint *edp) {
    return;
}
