#include <stdio.h>
#include <string.h> // For memcpy and memset
#include <stdint.h>
#include <xtensa/hal.h>

#include "platform.h"
#include "sharespace.h"
#include "log.h"

// Pointers to the shared memory regions.
static volatile uint8_t* dsp_reads_from_arm = NULL; // ARM writes here, DSP reads from here.
static volatile uint8_t* dsp_writes_to_arm = NULL;  // DSP writes here, ARM reads from here.

// Head pointers for the two buffers.
static volatile MsgHead* arm_head_ptr = NULL;   // Head for the ARM->DSP buffer.
static volatile MsgHead* dsp_head_ptr = NULL;   // Head for the DSP->ARM buffer.

// Address to track the arm and dsp read/write location
uint16_t sharespace_arm_addr[2];
uint16_t sharespace_dsp_addr[2];
uint16_t sharespace_log_addr[2];

// Global instance to store the discovered shared space parameters
static struct dts_sharespace_t dts_sharespace;
static struct dts_sharespace_t mmap_sharespace;

// platform_head is defined as a macro in platform.h

// This is a placeholder for the hardware-specific function that will trigger
// an RPMsg interrupt to notify the host that new data is available.
extern void rpmsg_signal_host(uint32_t msg);

// Janky HW usleep() function using clock ticks!
void hw_usleep(uint32_t usec) {
    volatile uint32_t count = 5 * (configCPU_CLOCK_HZ / 1000000) * usec;
    while (count--) {
        __asm__ volatile ("nop");
    }
}

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
    /*char[] msg="This is a test of the emergency buffer initialization system. This is only a test!";
    memcpy((void*)(dsp_writes_to_arm + MIN_ADDR), msg, strlen(msg)+1);
    printf("Initializing DSP head buffer (%p) to string (%d bytes):\n%s\n", (void*)dsp_head_ptr, strlen(msg)+1, msg);*/
    printf("DONE DSP FAKEINIT!\n\n");
    return;
}

// Waits for the ARM core to initialize its side of the shared memory.
static void sharespace_reinit(MsgHead *p_arm_head) {
    lprintf("sharespace_reinit: Waiting for ARM initialization...\n");
    hw_usleep(10000); // sleep 10 seconds before looping
    while (1) {
        lprintf("sharespace_reinit: Reading ARM head from 0x%08x\n", (uint32_t)arm_head_ptr);
        memcpy(p_arm_head, (const void*)arm_head_ptr, sizeof(MsgHead));
        lprintf("sharespace_reinit: arm_head.init_state = %d\n", p_arm_head->init_state);
        lprintf("sharespace_reinit: arm_head.write_addr = 0x%08x\n", p_arm_head->write_addr);
        lprintf("sharespace_reinit: arm_head.read_addr = 0x%08x\n", p_arm_head->read_addr);

        if (p_arm_head->init_state == 2 &&
            p_arm_head->write_addr != 0xa5a5a5a5 &&
            p_arm_head->read_addr != 0xa5a5a5a5) {

            lprintf("sharespace_reinit: ARM is initialized (state %d).\n", p_arm_head->init_state);
            lprintf("sharespace_reinit: Sync complete.\n");
            return;
        }
        hw_usleep(10000); // sleep 2 seconds between iterations
    }
}

// Main initialization function for the shared memory communication.
void sharespace_init(void) {
    lprintf("sharespace_init: Starting initialization.\n");
    sharespace_get_config(&dts_sharespace);
    sharespace_clear();
    lprintf("sharespace_init: Got config from DTS.\n");
    lprintf("sharespace_init: dsp_write_addr=0x%08x, arm_write_addr=0x%08x\n",
            dts_sharespace.dsp_write_addr, dts_sharespace.arm_write_addr);

    // Temporarily point to the initial handshake location in the DTS-defined sharespace
    arm_head_ptr = (volatile MsgHead*)(dts_sharespace.arm_write_addr + SHARE_SPACE_HEAD_OFFSET);
    lprintf("sharespace_init: arm_head_ptr initially set to 0x%08x\n", (uint32_t)arm_head_ptr);

    lprintf("sharespace_init: Calling sharespace_reinit() to get kbuf addresses.\n");
    MsgHead initial_arm_head;
    sharespace_reinit(&initial_arm_head);
    lprintf("sharespace_init: sharespace_reinit() returned.\n");

    lprintf("sharespace_init: Got kbuf addresses from ARM: read_addr=0x%08x, write_addr=0x%08x\n",
            initial_arm_head.read_addr, initial_arm_head.write_addr);

    // Now that we have the kbuf addresses, update pointers to point to the kbuf memory regions.
    // ARM's write_addr is the buffer for ARM->DSP communication.
    // ARM's read_addr is the buffer for DSP->ARM communication.
    dsp_writes_to_arm = (volatile uint8_t*)initial_arm_head.read_addr;
    dsp_reads_from_arm = (volatile uint8_t*)initial_arm_head.write_addr;

    // Update head pointers to point to the new locations within kbuf
    arm_head_ptr = (volatile MsgHead*)(dsp_reads_from_arm + SHARE_SPACE_HEAD_OFFSET);
    dsp_head_ptr = (volatile MsgHead*)(dsp_writes_to_arm + SHARE_SPACE_HEAD_OFFSET);

    lprintf("sharespace_init: Pointers updated to kbuf regions.\n");
    lprintf("sharespace_init: dsp_reads_from_arm (ARM->DSP buffer) is now at 0x%08x\n", (uint32_t)dsp_reads_from_arm);
    lprintf("sharespace_init: dsp_writes_to_arm (DSP->ARM buffer) is now at 0x%08x\n", (uint32_t)dsp_writes_to_arm);
    lprintf("sharespace_init: arm_head_ptr is now at 0x%08x\n", (uint32_t)arm_head_ptr);
    lprintf("sharespace_init: dsp_head_ptr is now at 0x%08x\n", (uint32_t)dsp_head_ptr);

    MsgHead dsp_head = {
        .read_addr = MIN_ADDR,
        .write_addr = MIN_ADDR,
        .init_state = 1
    };
    lprintf("sharespace_init: Initializing DSP head: read_addr=0x%08x, write_addr=0x%08x, init_state=%d\n",
            dsp_head.read_addr, dsp_head.write_addr, dsp_head.init_state);

    memcpy((void*)dsp_head_ptr, &dsp_head, sizeof(MsgHead));
    lprintf("sharespace_init: Wrote DSP head to 0x%08x.\n", (uint32_t)dsp_head_ptr);
    xthal_dcache_region_writeback((void*)dsp_head_ptr, sizeof(MsgHead));
    lprintf("sharespace_init: Flushed cache for DSP head region at 0x%08x, size %d.\n", (uint32_t)dsp_head_ptr, sizeof(MsgHead));

    MsgHead arm_head;
    lprintf("sharespace_init: Waiting for ARM to acknowledge with init_state=1.\n");
    while (1) {
        memcpy(&arm_head, (const void*)arm_head_ptr, sizeof(MsgHead));
        lprintf("sharespace_init: Polling ARM head (from kbuf): init_state=%d\n", arm_head.init_state);
        if (arm_head.init_state == 1) {
            sharespace_arm_addr[SHARESPACE_READ] = arm_head.read_addr;
            sharespace_arm_addr[SHARESPACE_WRITE] = arm_head.write_addr;
            lprintf("sharespace_init: ARM acknowledged. read_addr=0x%04x, write_addr=0x%04x\n",
                    sharespace_arm_addr[SHARESPACE_READ], sharespace_arm_addr[SHARESPACE_WRITE]);
            break;
        }
    }
    lprintf("sharespace_init: Initialization complete.\n");
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
    //xthal_dcache_region_writeback((void*)dsp_head_ptr, sizeof(MsgHead));

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
