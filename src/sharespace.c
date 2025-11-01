#include "platform.h"
#include "sharespace.h"
#include <string.h> // For memcpy and memset
#include <stdio.h>

// Pointers to the shared memory regions.
static volatile uint8_t* dsp_reads_from_arm = NULL; // ARM writes here, DSP reads from here.
static volatile uint8_t* dsp_writes_to_arm = NULL;  // DSP writes here, ARM reads from here.

// Head pointers for the two buffers.
static volatile MsgHead* arm_head_ptr = NULL;   // Head for the ARM->DSP buffer.
static volatile MsgHead* dsp_head_ptr = NULL;   // Head for the DSP->ARM buffer.

// Global instance to store the discovered shared space parameters
static struct dts_sharespace_t dts_sharespace;

// platform_head is defined as a macro in platform.h

// This is a placeholder for the hardware-specific function that will trigger
// an RPMsg interrupt to notify the host that new data is available.
extern void rpmsg_signal_host(uint32_t msg);

// Gets the initial shared memory configuration from the platform header.
static void sharespace_get_config(struct dts_sharespace_t *p_dts_sharespace) {
    volatile struct spare_rtos_head_t *pstr = platform_head;
    volatile struct dts_msg_t *pdts = &pstr->rtos_img_hdr.dts_msg;
    if (pdts->dts_sharespace.status == DTS_OPEN) {
        p_dts_sharespace->dsp_write_addr = pdts->dts_sharespace.dsp_write_addr;
        p_dts_sharespace->dsp_write_size = pdts->dts_sharespace.dsp_write_size;
        p_dts_sharespace->arm_write_addr = pdts->dts_sharespace.arm_write_addr;
        p_dts_sharespace->arm_write_size = pdts->dts_sharespace.arm_write_size;
        p_dts_sharespace->dsp_log_addr = pdts->dts_sharespace.dsp_log_addr;
        p_dts_sharespace->dsp_log_size = pdts->dts_sharespace.dsp_log_size;
    }
}

// Fake init function to call from DSP boot-up to check on some stuff
void sharespace_fake_init(void) {
    sharespace_get_config(&dts_sharespace);
    printf("DTS Sharespace Configuration:\n");
    printf("  DSP Write Address: 0x%x\n", dts_sharespace.dsp_write_addr);
    printf("  DSP Write Size:    0x%x\n", dts_sharespace.dsp_write_size);
    printf("  ARM Write Address: 0x%x\n", dts_sharespace.arm_write_addr);
    printf("  ARM Write Size:    0x%x\n", dts_sharespace.arm_write_size);
    printf("  DSP Log Address:   0x%x\n", dts_sharespace.dsp_log_addr);
    printf("  DSP Log Size:      0x%x\n", dts_sharespace.dsp_log_size);
    dsp_reads_from_arm = (volatile uint8_t*)dts_sharespace.arm_write_addr;
    dsp_writes_to_arm = (volatile uint8_t*)dts_sharespace.dsp_write_addr;
    printf("Initialized Shared Memory Pointers:\n");
    printf("  DSP Reads from ARM: %p\n", (void*)dsp_reads_from_arm);
    printf("  DSP Writes to ARM:  %p\n", (void*)dsp_writes_to_arm);
    arm_head_ptr = (volatile MsgHead*)(dsp_reads_from_arm + SHARE_SPACE_HEAD_OFFSET);
    dsp_head_ptr = (volatile MsgHead*)(dsp_writes_to_arm + SHARE_SPACE_HEAD_OFFSET);
    printf("Initialized Message Head Pointers:\n");
    printf("  ARM Head Pointer: %p\n", (void*)arm_head_ptr);
    printf("  DSP Head Pointer: %p\n", (void*)dsp_head_ptr);
    printf("DONE DSP INIT!\n");
    return;
}

// Waits for the ARM core to initialize its side of the shared memory.
static void sharespace_reinit(void) {
    MsgHead temp_arm_head;
    while (1) {
        memcpy(&temp_arm_head, (const void*)arm_head_ptr, sizeof(MsgHead));

        if ((temp_arm_head.init_state == 1 || temp_arm_head.init_state == 2) &&
            temp_arm_head.write_addr != 0xa5a5a5a5 &&
            temp_arm_head.read_addr != 0xa5a5a5a5) {

            if (temp_arm_head.init_state == 2) {
                temp_arm_head.init_state = 1;
                memcpy((void*)arm_head_ptr, &temp_arm_head, sizeof(MsgHead));
            }
            break; // Sync complete
        }
    }
}

// Main initialization function for the shared memory communication.
void sharespace_init(void) {
    sharespace_get_config(&dts_sharespace);

    dsp_reads_from_arm = (volatile uint8_t*)dts_sharespace.arm_write_addr;
    dsp_writes_to_arm = (volatile uint8_t*)dts_sharespace.dsp_write_addr;

    arm_head_ptr = (volatile MsgHead*)(dsp_reads_from_arm + SHARE_SPACE_HEAD_OFFSET);
    dsp_head_ptr = (volatile MsgHead*)(dsp_writes_to_arm + SHARE_SPACE_HEAD_OFFSET);

    sharespace_reinit();

    MsgHead dsp_head = { .read_addr = MIN_ADDR, .write_addr = MIN_ADDR, .init_state = 1 };
    memcpy((void*)dsp_head_ptr, &dsp_head, sizeof(MsgHead));

    uint32_t signal_msg = (dsp_head.write_addr << 16) | dsp_head.read_addr;
    //rpmsg_signal_host(signal_msg);

    MsgHead temp_arm_head;
    while (1) {
        memcpy(&temp_arm_head, (const void*)arm_head_ptr, sizeof(MsgHead));
        if (temp_arm_head.init_state == 1) {
            break;
        }
    }
}

// Invalidates the ARM's message head in shared memory.
void sharespace_clear(void) {
    sharespace_get_config(&dts_sharespace);
    memset((void*)(dts_sharespace.arm_write_addr + SHARE_SPACE_HEAD_OFFSET), 0xa5, sizeof(MsgHead));
}

int sharespace_write(const void* data, int len) {
    MsgHead dsp_head, arm_head;
    memcpy(&dsp_head, (const void*)dsp_head_ptr, sizeof(MsgHead));
    memcpy(&arm_head, (const void*)arm_head_ptr, sizeof(MsgHead));

    uint32_t host_read_addr = dsp_head.read_addr; // ARM reads from DSP buffer
    uint32_t local_write_addr = dsp_head.write_addr;

    int free_size;
    if (host_read_addr <= local_write_addr) {
        free_size = (MAX_ADDR - local_write_addr) + (host_read_addr - MIN_ADDR);
    } else {
        free_size = host_read_addr - local_write_addr;
    }

    if (free_size <= len + 1) { // Leave 1 byte for full/empty check
        return -1;
    }

    const uint8_t* src = (const uint8_t*)data;
    if (local_write_addr + len <= MAX_ADDR) {
        memcpy((void*)(dsp_writes_to_arm + local_write_addr), src, len);
    } else {
        int len1 = MAX_ADDR - local_write_addr;
        memcpy((void*)(dsp_writes_to_arm + local_write_addr), src, len1);
        int len2 = len - len1;
        memcpy((void*)(dsp_writes_to_arm + MIN_ADDR), src + len1, len2);
    }

    dsp_head.write_addr = (local_write_addr + len) % (MAX_ADDR - MIN_ADDR) + MIN_ADDR;
    memcpy((void*)dsp_head_ptr, &dsp_head, sizeof(MsgHead));

    uint32_t signal_msg = (dsp_head.write_addr << 16) | dsp_head.read_addr;
    //rpmsg_signal_host(signal_msg);

    return len;
}

int sharespace_read(void* out_buffer, int max_len) {
    MsgHead arm_head;
    memcpy(&arm_head, (const void*)arm_head_ptr, sizeof(MsgHead));

    uint32_t host_write_addr = arm_head.write_addr;
    uint32_t local_read_addr = arm_head.read_addr;

    if (local_read_addr == host_write_addr) {
        return 0;
    }

    int msg_size;
    if (local_read_addr < host_write_addr) {
        msg_size = host_write_addr - local_read_addr;
    } else { 
        msg_size = (MAX_ADDR - local_read_addr) + (host_write_addr - MIN_ADDR);
    }

    int bytes_to_copy = (msg_size < max_len) ? msg_size : max_len;
    uint8_t* dest = (uint8_t*)out_buffer;

    if (local_read_addr + bytes_to_copy <= MAX_ADDR) {
        memcpy(dest, (const void*)(dsp_reads_from_arm + local_read_addr), bytes_to_copy);
    } else {
        int len1 = MAX_ADDR - local_read_addr;
        memcpy(dest, (const void*)(dsp_reads_from_arm + local_read_addr), len1);
        int len2 = bytes_to_copy - len1;
        memcpy(dest + len1, (const void*)(dsp_reads_from_arm + MIN_ADDR), len2);
    }

    arm_head.read_addr = (local_read_addr + bytes_to_copy) % (MAX_ADDR - MIN_ADDR) + MIN_ADDR;
    memcpy((void*)arm_head_ptr, &arm_head, sizeof(MsgHead));

    return bytes_to_copy;
}
