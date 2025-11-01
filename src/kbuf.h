#ifndef KBUF_H
#define KBUF_H

#include <stdint.h>

// Total size of one-way buffer region.
#define BUFFER_SIZE 4096

// Define the structure for the message head.
typedef struct __attribute__((packed)) {
    volatile uint32_t init_state;
    volatile uint32_t read_addr;
    volatile uint32_t write_addr;
} MsgHead;

// The data buffer starts at offset 0.
#define MIN_ADDR 0

// The data buffer ends just before the MsgHead.
// MAX_ADDR is the offset of the MsgHead.
#define MAX_ADDR (BUFFER_SIZE - sizeof(MsgHead))

// DTS Sharespace definitions
#define DTS_OPEN 1 // Assuming DTS_OPEN value

typedef struct {
    uint32_t status;
    uint32_t dsp_write_addr;
    uint32_t dsp_write_size;
    uint32_t arm_write_addr;
    uint32_t arm_write_size;
    uint32_t dsp_log_addr;
    uint32_t dsp_log_size;
} dts_sharespace_t;

typedef struct {
    // Placeholder for other fields if any, based on actual definition
    dts_sharespace_t dts_sharespace;
    // Placeholder for other fields if any
} dts_msg_t;

typedef struct {
    // Placeholder for other fields if any, based on actual definition
    struct { // rtos_img_hdr
        dts_msg_t dts_msg;
    } rtos_img_hdr;
    // Placeholder for other fields if any
} spare_rtos_head_t;

extern volatile spare_rtos_head_t *platform_head;


// Function prototypes
void kbuf_init(void* shared_mem_base);
void kbuf_wait_for_host_init(void);
int kbuf_read_from_host(void* out_buffer, int max_len);
int kbuf_write_to_host(const void* data, int len);

// New function prototype for system-level initialization
void kbuf_system_init(void);

#endif // KBUF_H