#ifndef KBUF_DSP_H
#define KBUF_DSP_H

#include <stdint.h>
#include <stddef.h>
#include <platform.h>

// This struct must match the layout of the `MsgHead` struct in the Rust code.
// It is marked `#[repr(C)]` in Rust, so a direct translation is safe.
typedef struct {
    uint32_t read_addr;
    uint32_t write_addr;
    uint32_t init_state;
} __attribute__((packed)) MsgHead;

#define BUFFER_SIZE 4096
#define MSG_HEAD_SIZE sizeof(MsgHead)

// The actual data payload area in the ring buffer.
#define MIN_ADDR MSG_HEAD_SIZE
#define MAX_ADDR (BUFFER_SIZE - MSG_HEAD_SIZE)

/**
 * @brief Initializes the DSP communication module.
 *
 * This function must be called once at startup. It takes the base address of the
 * shared memory region allocated by the host.
 *
 * @param shared_mem_base A pointer to the beginning of the 8192-byte shared memory region.
 */
void kbuf_init(void* shared_mem_base);

/**
 * @brief Waits for the host (ARM) to complete its initialization.
 *
 * This function implements the DSP side of the initialization handshake.
 * It blocks until the host signals it is ready.
 */
void kbuf_wait_for_host_init();

/**
 * @brief Reads data sent from the host into the provided buffer.
 *
 * This function checks the ring buffer for new data from the host and copies
 * it into the `out_buffer`.
 *
 * @param out_buffer A buffer to store the read data.
 * @param max_len The maximum number of bytes to read (size of `out_buffer`).
 * @return The number of bytes read. Returns 0 if no new data is available.
 */
int kbuf_read_from_host(void* out_buffer, int max_len);

/**
 * @brief Writes data from the DSP to be read by the host.
 *
 * This function copies data into the DSP-to-Host ring buffer and signals the
 * host that new data is available.
 *
 * @param data A pointer to the data to be written.
 * @param len The number of bytes to write.
 * @return 0 on success, -1 on failure (e.g., not enough space in the buffer).
 */
int kbuf_write_to_host(const void* data, int len);

#endif // KBUF_DSP_H
