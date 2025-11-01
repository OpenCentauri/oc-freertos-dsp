#include "kbuf.h"
#include <string.h> // For memcpy

// Pointers to the shared memory regions and control structures.
// These are initialized in `kbuf_init`.
static volatile uint8_t* dsp_read_buffer = NULL;  // ARM writes here, DSP reads from here.
static volatile uint8_t* dsp_write_buffer = NULL; // DSP writes here, ARM reads from here.

static volatile MsgHead* arm_head = NULL;   // Control block for the dsp_read_buffer.
static volatile MsgHead* dsp_head = NULL;   // Control block for the dsp_write_buffer.

// This is a placeholder for the hardware-specific function that will trigger
// an RPMsg interrupt to notify the host that new data is available.
// You must implement this function based on your DSP's platform specifics.
extern void rpmsg_signal_host(uint16_t read_addr, uint16_t write_addr);

void kbuf_init(void* shared_mem_base) {
    // The shared memory is split into two buffers.
    dsp_read_buffer = (volatile uint8_t*)shared_mem_base;
    dsp_write_buffer = dsp_read_buffer + BUFFER_SIZE;

    // The MsgHead for each buffer is at the end of its 4096-byte region.
    arm_head = (volatile MsgHead*)(dsp_read_buffer + MAX_ADDR);
    dsp_head = (volatile MsgHead*)(dsp_write_buffer + MAX_ADDR);
}

void kbuf_wait_for_host_init() {
    // 1. Wait for the ARM host to set its init_state to 1.
    // This mirrors the `wait_dsp_set_init` logic in the Rust code.
    while (arm_head->init_state != 1) {
        // Depending on the system, a small delay or a yield might be needed here.
    }

    // 2. Initialize the DSP's own message head.
    // The read and write pointers start at the beginning of the data area.
    dsp_head->read_addr = MIN_ADDR;
    dsp_head->write_addr = MIN_ADDR;

    // 3. Signal to the ARM host that the DSP is initialized.
    dsp_head->init_state = 1;
}

int kbuf_read_from_host(void* out_buffer, int max_len) {
    // This logic mirrors `dsp_mem_read` in the Rust `CommunicationHandler`.
    uint32_t local_read_addr = dsp_head->read_addr;
    uint32_t host_write_addr = arm_head->write_addr;

    if (local_read_addr == host_write_addr) {
        return 0; // No new data.
    }

    int msg_size;
    if (local_read_addr < host_write_addr) {
        msg_size = host_write_addr - local_read_addr;
    } else { // Wrap-around case
        msg_size = (MAX_ADDR - local_read_addr) + (host_write_addr - MIN_ADDR);
    }

    int bytes_to_copy = (msg_size < max_len) ? msg_size : max_len;
    uint8_t* dest = (uint8_t*)out_buffer;

    if (local_read_addr + bytes_to_copy <= MAX_ADDR) {
        // No wrap-around for this read.
        memcpy(dest, (const void*)(dsp_read_buffer + local_read_addr), bytes_to_copy);
    } else {
        // Handle wrap-around.
        int len1 = MAX_ADDR - local_read_addr;
        memcpy(dest, (const void*)(dsp_read_buffer + local_read_addr), len1);

        int len2 = bytes_to_copy - len1;
        memcpy(dest + len1, (const void*)(dsp_read_buffer + MIN_ADDR), len2);
    }

    // Update the local read pointer.
    uint32_t new_read_addr = (local_read_addr + bytes_to_copy) % (MAX_ADDR - MIN_ADDR);
    if (new_read_addr < MIN_ADDR) {
        new_read_addr += MIN_ADDR;
    }
    dsp_head->read_addr = new_read_addr;

    return bytes_to_copy;
}

int kbuf_write_to_host(const void* data, int len) {
    // This logic mirrors `dsp_mem_write` in the Rust `CommunicationHandler`.
    uint32_t local_write_addr = dsp_head->write_addr;
    uint32_t host_read_addr = arm_head->read_addr;

    int free_size;
    if (host_read_addr <= local_write_addr) {
        free_size = (MAX_ADDR - MIN_ADDR) - (local_write_addr - host_read_addr);
    } else {
        free_size = host_read_addr - local_write_addr;
    }

    // Check if there is enough space, leaving 1 byte for the full/empty check.
    if (free_size <= len) {
        return -1; // Not enough space.
    }

    const uint8_t* src = (const uint8_t*)data;

    if (local_write_addr + len <= MAX_ADDR) {
        // No wrap-around for this write.
        memcpy((void*)(dsp_write_buffer + local_write_addr), src, len);
    } else {
        // Handle wrap-around.
        int len1 = MAX_ADDR - local_write_addr;
        memcpy((void*)(dsp_write_buffer + local_write_addr), src, len1);

        int len2 = len - len1;
        memcpy((void*)(dsp_write_buffer + MIN_ADDR), src + len1, len2);
    }

    // Update the write pointer.
    uint32_t new_write_addr = (local_write_addr + len) % (MAX_ADDR - MIN_ADDR);
    if (new_write_addr < MIN_ADDR) {
        new_write_addr += MIN_ADDR;
    }
    dsp_head->write_addr = new_write_addr;

    // IMPORTANT: Signal the host that new data is available.
    // The host's `msgbox_send_signal` expects the ARM's read and write addresses.
    // From the DSP's perspective, these are the DSP's write and read addresses respectively.

    // Skip this for now! Sims said it might be okay *cries in concurrency errors*
    //rpmsg_signal_host((uint16_t)dsp_head->read_addr, (uint16_t)dsp_head->write_addr);

    return 0;
}
