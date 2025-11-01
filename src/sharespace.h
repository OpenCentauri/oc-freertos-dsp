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



// Function prototypes
void sharespace_init(void);
void sharespace_clear(void);
int sharespace_write(const void* data, int len);
int sharespace_read(void* out_buffer, int max_len);

#endif // KBUF_H
