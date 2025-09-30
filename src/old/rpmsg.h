#ifndef __RPMSG_H__
#define __RPMSG_H__

#include <stdint.h>
#include <string.h>

// A very basic RPMSG endpoint structure
typedef struct rpmsg_endpoint {
    char name[32];
    void (*rx_cb)(void *payload, uint32_t len, void *priv);
    void *priv;
    uint32_t addr; // RPMSG address for this endpoint
} rpmsg_endpoint_t;

// Simplified RPMSG header structure
typedef struct rpmsg_hdr {
    uint32_t src;  // Source address
    uint32_t dst;  // Destination address
    uint32_t len;  // Length of payload
    uint16_t flags; // Flags
    uint16_t reserved; // Reserved
} rpmsg_hdr_t;

// Simplified RPMSG Name Service message
#define RPMSG_NAME_MAX 32
typedef struct rpmsg_ns_msg {
    char name[RPMSG_NAME_MAX]; // Service name
    uint32_t addr;             // Endpoint address
    uint32_t flags;            // Flags (e.g., RPMSG_NS_CREATE)
} rpmsg_ns_msg_t;

#define RPMSG_NS_ADDR 53
#define RPMSG_NS_CREATE 0 // Flag for service creation

static void amp_rx_callback(void *payload, uint32_t len, void *priv);

// Function prototypes for a minimal RPMSG-like API
int rpmsg_init(void);
rpmsg_endpoint_t *rpmsg_create_endpoint(const char *name, uint32_t addr, void (*rx_cb)(void *payload, uint32_t len, void *priv), void *priv);
void rpmsg_rx_data(uint32_t data); // To be called by the low-level driver
int rpmsg_send(rpmsg_endpoint_t *ept, uint32_t dst_addr, void *data, uint32_t len);

#endif // __RPMSG_H__
