#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/config/core.h>
#include <xtensa/core-macros.h>
#include <xtensa/hal.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/xtruntime.h>
#include <xtensa_api.h>

#include "FreeRTOS.h"
#include "platform.h"
#include "task.h"
#include "rpmsg.h" // Include the new RPMSG header

typedef void (*msgbox_rxcb)(uint32_t, uint32_t);

// Global RPMSG endpoint for "amp"
static rpmsg_endpoint_t *amp_endpoint = NULL;

// HiFI4 DSP-viewed address offset translate to host cpu viewwed @ua1arn
static ptrdiff_t xlate_dsp2mpu(ptrdiff_t a) {
    const ptrdiff_t BANKSIZE = 0x08000u;
    const ptrdiff_t CELLBASE = 0x10000u;
    const ptrdiff_t CELLSIZE = 16;
    const ptrdiff_t cellbank = (a - CELLBASE) / BANKSIZE;
    const ptrdiff_t cellrow = (a - CELLBASE) % BANKSIZE / CELLSIZE; // 16 bytes granularity
    const unsigned cellpos = (a % CELLSIZE);                       // 16 bytes granularity

    if (a < CELLBASE)
        return a; /* translation not needed. */

    return CELLBASE +
           cellbank * BANKSIZE +
           CELLSIZE * ((cellrow % 2) ? (cellrow / 2) + (BANKSIZE / CELLSIZE / 2) : cellrow / 2) +
           cellpos;
}

static void copy2dsp(uint8_t *pdspmap, const uint8_t *pcpu, unsigned offs, unsigned size) {
    for (; size--; ++offs) {
        pdspmap[xlate_dsp2mpu(offs)] = pcpu[offs];
    }
}

static void zero2dsp(uint8_t *pdspmap, unsigned offs, unsigned size) {
    for (; size--; ++offs) {
        pdspmap[xlate_dsp2mpu(offs)] = 0x00;
    }
    (void)zero2dsp; // Suppress unused function warning
}

// RPMSG implementations (minimal)
int rpmsg_init(void) {
    // Placeholder for actual RPMSG framework initialization
    printf("RPMSG: Initializing...\n");

    // Create the "amp" endpoint
    amp_endpoint = rpmsg_create_endpoint("sunxi,dsp-msgbox", 0x01, amp_rx_callback, NULL);

    // Send name service announce message
    rpmsg_ns_msg_t ns_msg;
    strncpy(ns_msg.name, amp_endpoint->name, RPMSG_NAME_MAX - 1);
    ns_msg.name[RPMSG_NAME_MAX - 1] = '\0';
    ns_msg.addr = amp_endpoint->addr;
    ns_msg.flags = RPMSG_NS_CREATE;

    // Send the NS message using the amp_endpoint as source, and RPMSG_NS_ADDR as destination
    rpmsg_send(amp_endpoint, RPMSG_NS_ADDR, &ns_msg, sizeof(rpmsg_ns_msg_t));

    return 0;
}

rpmsg_endpoint_t *rpmsg_create_endpoint(const char *name, uint32_t addr, void (*rx_cb)(void *payload, uint32_t len, void *priv), void *priv) {
    static rpmsg_endpoint_t new_endpoint; // Using static for simplicity, not thread-safe for multiple endpoints
    strncpy(new_endpoint.name, name, sizeof(new_endpoint.name) - 1);
    new_endpoint.name[sizeof(new_endpoint.name) - 1] = '\0';
    new_endpoint.rx_cb = rx_cb;
    new_endpoint.priv = priv;
    new_endpoint.addr = addr; // Store the address
    printf("RPMSG: Created endpoint '%s' with address 0x%08x\n", name, addr);
    return &new_endpoint;
}

// A very basic buffer to reassemble RPMSG messages
#define RPMSG_MAX_MSG_SIZE 512 // Max message size, including header
static uint8_t rpmsg_rx_buffer[RPMSG_MAX_MSG_SIZE];
static uint32_t rpmsg_rx_offset = 0;
static rpmsg_hdr_t current_rx_hdr;
static int receiving_header = 1; // State: 1 for header, 0 for payload

void rpmsg_rx_data(uint32_t data) {
    // This is a very simplified reassembly. A real RPMSG would use shared memory and more robust parsing.
    if (receiving_header) {
        // Assuming the first word is part of the header
        if (rpmsg_rx_offset < sizeof(rpmsg_hdr_t)) {
            memcpy(rpmsg_rx_buffer + rpmsg_rx_offset, &data, sizeof(uint32_t));
            rpmsg_rx_offset += sizeof(uint32_t);

            if (rpmsg_rx_offset == sizeof(rpmsg_hdr_t)) {
                memcpy(&current_rx_hdr, rpmsg_rx_buffer, sizeof(rpmsg_hdr_t));
                receiving_header = 0; // Switch to payload reception
                rpmsg_rx_offset = 0; // Reset offset for payload
                printf("RPMSG: Received header (src=0x%x, dst=0x%x, len=%lu)\n", current_rx_hdr.src, current_rx_hdr.dst, (unsigned long)current_rx_hdr.len);
                if (current_rx_hdr.len == 0) {
                    // Message is just a header, no payload
                    if (amp_endpoint && amp_endpoint->rx_cb) {
                        amp_endpoint->rx_cb(NULL, 0, amp_endpoint->priv);
                    }
                    receiving_header = 1; // Reset for next message
                }
            }
        }
    } else {
        // Receiving payload
        if (rpmsg_rx_offset < current_rx_hdr.len) {
            memcpy(rpmsg_rx_buffer + rpmsg_rx_offset, &data, sizeof(uint32_t));
            rpmsg_rx_offset += sizeof(uint32_t);

            if (rpmsg_rx_offset >= current_rx_hdr.len) {
                // Full message received
                if (amp_endpoint && amp_endpoint->rx_cb) {
                    amp_endpoint->rx_cb(rpmsg_rx_buffer, current_rx_hdr.len, amp_endpoint->priv);
                }
                receiving_header = 1; // Reset for next message
                rpmsg_rx_offset = 0; // Reset offset
            }
        }
    }
}

int rpmsg_send(rpmsg_endpoint_t *ept, uint32_t dst_addr, void *data, uint32_t len) {
    if (!ept) {
        printf("RPMSG: Error: Endpoint is NULL\n");
        return -1;
    }

    if (len > (RPMSG_MAX_MSG_SIZE - sizeof(rpmsg_hdr_t))) {
        printf("RPMSG: Error: Message too long (len=%lu)\n", (unsigned long)len);
        return -1;
    }

    rpmsg_hdr_t hdr;
    hdr.src = ept->addr;
    hdr.dst = dst_addr; // Use the provided destination address
    hdr.len = len;
    hdr.flags = 0;
    hdr.reserved = 0;

    // Send header first
    // Assuming channel 1 for RPMSG communication based on DTS
    dsp_msgbox_channel_send(0, (uint8_t *)&hdr, sizeof(rpmsg_hdr_t));

    // Send payload
    if (len > 0) {
        dsp_msgbox_channel_send(0, (uint8_t *)data, len);
    }

    printf("RPMSG: Sent message from 0x%08x to 0x%08x, len=%lu\n", hdr.src, hdr.dst, (unsigned long)hdr.len);
    return 0;
}

// Callback for the "amp" endpoint
static void amp_rx_callback(void *payload, uint32_t len, void *priv) {
    (void)priv; // Suppress unused parameter warning
    printf("RPMSG 'amp' endpoint received message (len: %lu)\n", (unsigned long)len);
    if (payload && len > 0) {
        // For demonstration, print as string if possible
        printf("  Payload: \"%.*s\"\n", (int)len, (char *)payload);
    }
    // Here you would process the actual RPMSG message payload
}

static int sunxi_msgbox_interrupt(int dummy, void *args) {
    (void) dummy;
    (void)args; // Suppress unused parameter warning
    uint32_t dat;
    for (int i = 0; i < SUNXI_MSGBOX_MAX_CHANNEL; i++) {
        if (SUNXI_MSGBOX_RD_IRQ_IS_PENDING(i)) {
            while (readl(SUNXI_MSGBOX_DSP_MSG_STATUS_REG(i))) {
                dat = readl(SUNXI_MSGBOX_DSP_MSG_REG(i));
                // Pass received data to the RPMSG layer
                rpmsg_rx_data(dat);
            }
            SUNXI_MSGBOX_RD_IRQ_CLR_PENDING(i);
        }
    }
    return 0;
}

void dsp_msgbox_init(void (*rxcb)(uint32_t, uint32_t)) {
    // Initialize low-level message box interrupt
    xt_set_interrupt_handler(MSGBOX_IRQ, (xt_handler) sunxi_msgbox_interrupt,
                             (void *) rxcb);
    xt_ints_on(1 << MSGBOX_IRQ);
    SUNXI_MSGBOX_RD_IRQ_ENABLE(MSGBOX_CHANNELS);

    // Clear any pending write interrupts on CPU->DSP channel (channel 1)
    writel(SUNXI_MSGBOX_WR_IRQ_STATUS_REG, (1 << (2 * 1)));

    // Enable clock and deassert reset for both CPU and DSP message boxes
    // MSGBOX_BGR_REG is at CCU_BASE + 0x071C
    // Bits: 17 (MSGBOX1_RST), 1 (MSGBOX1_GATING), 16 (MSGBOX0_RST), 0 (MSGBOX0_GATING)
    #define CCU_BASE 0x02001000
    #define MSGBOX_BGR_REG (CCU_BASE + 0x071C)
    writel(MSGBOX_BGR_REG, readl(MSGBOX_BGR_REG) | ( (1 << 17) | (1 << 1) | (1 << 16) | (1 << 0) ) );

    // Add a small delay after enabling clock and deasserting reset
    volatile int i;
    for (i = 0; i < 100000; i++); // Busy-wait for a short period

    // Configure channel 0 as RX and TX for the DSP (coprocessor)
    // This corresponds to CTRL_REG(0), CTRL_RX(0) and CTRL_TX(0) in the Linux driver
    // Use SUNXI_MSGBOX_ARM_BASE as the Linux driver is looking at this base address
    writel(SUNXI_MSGBOX_ARM_BASE + 0x0000, readl(SUNXI_MSGBOX_ARM_BASE + 0x0000) | (0x01 | 0x10));

    // Initialize RPMSG framework
    rpmsg_init();

    
}

static void msgbox_channel_send_data(uint32_t ch, uint32_t data) {
    while (readl(SUNXI_MSGBOX_ARM_MSG_STATUS_REG(ch)) == SUNXI_MSGBOX_MAX_QUEUE)
        ;
    writel(SUNXI_MSGBOX_ARM_MSG_REG(ch), data);
}

void dsp_msgbox_channel_send(uint32_t ch, uint8_t *bf, uint32_t len) {
    uint32_t data = 0;
    uint32_t bytes_sent = 0;

    // Ensure len is a multiple of 4 for simplicity, or handle padding
    // For this minimal implementation, we'll send 4 bytes at a time.
    // If len is not a multiple of 4, the last word might contain garbage or be padded.
    for (uint32_t i = 0; i < len; i += 4) {
        data = 0;
        for (uint32_t j = 0; j < 4; j++) {
            if (bytes_sent < len) {
                data |= (uint32_t)bf[bytes_sent] << (j * 8);
                bytes_sent++;
            }
        }
        msgbox_channel_send_data(ch, data);
    }
}
