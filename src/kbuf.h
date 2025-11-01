#ifndef KBUF_H
#define KBUF_H

#include <stdint.h>

// Total size of one-way buffer region.
#define BUFFER_SIZE 4096

// Define the structure for the message head.
typedef struct __attribute__((packed)){
    volatile uint32_t read_addr;
    volatile uint32_t write_addr;
    volatile uint32_t init_state;
} MsgHead;

// The data buffer starts at offset 0.
#define MIN_ADDR 0

// The data buffer ends just before the MsgHead.
// MAX_ADDR is the offset of the MsgHead.
#define MAX_ADDR (BUFFER_SIZE - sizeof(MsgHead))

// Function prototypes
void kbuf_init(void* shared_mem_base);
void kbuf_wait_for_host_init(void);
int kbuf_read_from_host(void* out_buffer, int max_len);
int kbuf_write_to_host(const void* data, int len);

#endif // KBUF_H
