#ifndef SHARE_SPACE_H
#define SHARE_SPACE_H

#include <stdint.h>
#include <imgdts.h>

#define SHARE_SPACE_SYNC 0
#define MSGBOX_EN_SYNC 0
#define MSGBOX_EN_WR_OK 1
#define MSGBOX_EN_RD_OK 1
#define MSGBOX_EN_IRQ 0

#define SHARESPACE_READ 0
#define SHARESPACE_WRITE 1

void share_space_init(void);
void share_space_clear(void);
int share_space_write(uint8_t *pstr, uint32_t size);
int do_read_space(uint8_t* buf, uint32_t size);

#endif /* SHARE_SPACE_H */
