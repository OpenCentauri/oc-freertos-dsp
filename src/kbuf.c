#include "kbuf.h"
#include <string.h> // For memcpy

// Pointers to the shared memory regions and control structures.
// These are initialized in `kbuf_init`.
static volatile uint8_t* dsp_read_buffer = NULL;  // ARM writes here, DSP reads from here.
static volatile uint8_t* dsp_write_buffer = NULL; // DSP writes here, ARM reads from here.

static volatile MsgHead* arm_head = NULL;   // Control block for the dsp_read_buffer.
static volatile MsgHead* dsp_head = NULL;   // Control block for the dsp_write_buffer.

// Global instance to store the discovered shared space parameters
static dts_sharespace_t dts_sharespace_params;

// Declare platform_head as extern, as it's defined elsewhere
extern volatile spare_rtos_head_t *platform_head;

static void sharespace_init(dts_sharespace_t *p_dts_sharespace) {
    volatile spare_rtos_head_t *pstr = platform_head;
    volatile dts_msg_t *pdts = &pstr->rtos_img_hdr.dts_msg;
    int val = 0;
    val = pdts->dts_sharespace.status;
    if (val == DTS_OPEN) {
        p_dts_sharespace->dsp_write_addr = pdts->dts_sharespace.dsp_write_addr;
        p_dts_sharespace->dsp_write_size = pdts->dts_sharespace.dsp_write_size;
        p_dts_sharespace->arm_write_addr = pdts->dts_sharespace.arm_write_addr;
        p_dts_sharespace->arm_write_size = pdts->dts_sharespace.arm_write_size;
        p_dts_sharespace->dsp_log_addr = pdts->dts_sharespace.dsp_log_addr;
        p_dts_sharespace->dsp_log_size = pdts->dts_sharespace.dsp_log_size;
    }
}

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
    // This function now handles initial synchronization and re-initialization.
    while (1) {
        // Check if the host has initialized its side of the shared memory.
        // We check for init_state 1 (normal init) or 2 (re-init),
        // and make sure the addresses are not the magic uninitialized value.
        if ((arm_head->init_state == 1 || arm_head->init_state == 2) &&
            arm_head->write_addr != 0xa5a5a5a5 &&
            arm_head->read_addr != 0xa5a5a5a5) {

            // If host is in re-init state (2), the DSP should acknowledge this
            // by setting the host's state back to 1.
            // Note: This involves the DSP writing to the ARM's control block,
            // which can be risky if not handled carefully in the ARM code.
            if (arm_head->init_state == 2) {
                arm_head->init_state = 1;
            }

            // Initialize the DSP's own message head.
            // The read and write pointers start at the beginning of the data area.
            dsp_head->read_addr = MIN_ADDR;
            dsp_head->write_addr = MIN_ADDR;

            // Signal to the ARM host that the DSP is initialized.
            dsp_head->init_state = 1;

            break; // Exit the loop as we are initialized.
        }

        // Depending on the system, a small delay or a task yield might be
        // needed here to prevent busy-spinning too fast.
    }
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

    // IMPORTANT: Signal the host that new data is available.
    // The arguments are the DSP's current read and write addresses,
    // which the host will interpret as its own write and read addresses, respectively.

    // PD: Skip this for now! Sims said it might be okay *cries in concurrency errors*
    //rpmsg_signal_host((uint16_t)dsp_head->read_addr, (uint16_t)dsp_head->write_addr);

    return 0;
}

void kbuf_system_init(void) {
    // First, initialize the shared space parameters from the platform head.
    sharespace_init(&dts_sharespace_params);

    // Then, initialize the kbuf buffers using the discovered ARM write address
    // as the base for the shared memory.
    kbuf_init((void*)dts_sharespace_params.arm_write_addr);
}
