#ifndef KBUF_H
#define KBUF_H

#include <stdint.h>

// Total size of one-way buffer region.
#define BUFFER_SIZE 4096

// Define the structure for the message head, matching the ARM side.
// The order of fields is critical for memory layout compatibility.
typedef struct __attribute__((packed)) {
    volatile uint32_t read_addr;
    volatile uint32_t write_addr;
    volatile uint32_t init_state;
} MsgHead;

// The offset to the MsgHead at the end of the buffer region.
#define SHARE_SPACE_HEAD_OFFSET (BUFFER_SIZE - sizeof(MsgHead))

// The data buffer does not start at 0. It starts after where the head would be if it were at the beginning.
#define MIN_ADDR sizeof(MsgHead)

// The data buffer ends just before the actual MsgHead at the end of the region.
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
void sharespace_init(void);
void sharespace_clear(void);
int sharespace_write(const void* data, int len);
int sharespace_read(void* out_buffer, int max_len);

#endif // KBUF_H
