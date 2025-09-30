#include "../../include/bus/share_space.h"
#include "../../include/printer/basecmd.h"
#include <platform.h>
#include "delay.h"
#include "../../include/bus/msgboxx.h"

struct receive_msg_queue* receive_msg;
int read_pos = 0;
// uint8_t read_buf[4096];
//  char receive_msg_buf[MESSAGE_MAX];
 static uint8_t buf[4096];
 static char receive_msg_buf[4096];     //MESSAGE_MAX

void share_space_find_data_pack()
{
    while (1)
    {
        int len = 0;
#if SHARE_SPACE_SYNC
        if (!share_space_set_read_state())
        {
        }
        else
#endif
        {
            len = do_read_space(buf, 4096);         //do_read_uart do_read_space
             if (len <= 0)
             {
                  return;
             }
#if SHARE_SPACE_SYNC
            while (!share_space_set_read_idle_state())
            {
            }
#endif  
        }
        if (len > 0)
        {
            int i = 0, j = 0,length = 0;
            for (; i < len; i++)
            {
                receive_msg_buf[j] = buf[i];
                 j++;
                 if(j==2)
                 {
                    if (receive_msg_buf[0] != 0x7e)
                    {
                        length =  receive_msg_buf[0];
                    }
                    else{
                        length =    1 + receive_msg_buf[1];
                    }
                    if(length > 64)
                    {
                        length = 0;
                    }
                 }
                if (buf[i] == 0x7e && (length <= j) && ( j>1 ))
                {
                        if ((j > 64)|| (j > length))
                        {
                            GAM_DEBUG_errer("-ERR length:%d-%d-%d-\n",length,j,i );
                            GAM_DEBUG_printf_HEX1(0,buf,len);
                        }
                    int msg_len = j;
                    receive_msg_queue_push(receive_msg, receive_msg_buf, &msg_len);
                    j = 0;
                }
            }
        }
        if (!receive_msg_queue_isEmpty(receive_msg))
        {
            usb_notify_bulk_out();
            usb_notify_bulk_in();
            return;
        }
    }
}

//=====================================================================================================

void receive_msg_queue_alloc()
{
    receive_msg = (struct receive_msg_queue*)malloc(sizeof(struct receive_msg_queue));
//    if(receive_msg == NULL)
//     {
//         GAM_DEBUG_errer("-ERR malloc:%d--\n",len);
//         return;
//     }

    receive_msg->front = NULL;
    receive_msg->rear = NULL;
}

void receive_msg_queue_free()
{
    free(receive_msg);
}

int receive_msg_queue_isEmpty(struct receive_msg_queue* receive_msg)
{
    if(receive_msg->front == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void receive_msg_queue_pop(struct receive_msg_queue* receive_msg, char* buf, int* len)
{
    struct receive_msg_block *q ;//= (struct receive_msg_block *)malloc(sizeof(struct receive_msg_block));

	if(!receive_msg->front){
		printf("queue empty!\n");
        return;
	}
	q = receive_msg->front;
    int j = 0;
    j = 0;
    memcpy(buf, q->buf, q->msg_len);
    *len = q->msg_len;

	receive_msg->front = q->next;
	free(q);
	if(receive_msg->front == NULL)
        receive_msg->rear = NULL;
}

void receive_msg_queue_push(struct receive_msg_queue* receive_msg, char *buf, int* len)
{
    struct receive_msg_block *p = (struct receive_msg_block *)malloc(sizeof(struct receive_msg_block));
    if(p == NULL)
    {
        GAM_DEBUG_errer("-ERR malloc:%d--\n",len);
        return;
    }
    int j = 0;
    j = 0;
    memcpy(p->buf ,buf, *len);
    p->msg_len = *len;
	p->next=NULL;
	if(receive_msg->front == NULL){
		receive_msg->front = receive_msg->rear = p;
	}
	else{
		receive_msg->rear->next=p;
		receive_msg->rear=p;
	}
}

//=============================================================================

int_fast8_t usb_read_bulk_out(void *data, uint_fast8_t max_len)
{
    char buf[MESSAGE_MAX];
    int len = -1;
    if (!receive_msg_queue_isEmpty(receive_msg))
    {
        receive_msg_queue_pop(receive_msg, buf, &len);
        memcpy((char*)data, buf, len);
    }
    return len;
}

int_fast8_t usb_send_bulk_in(void *data, uint_fast8_t len)
{
    int_fast8_t ret_len;
    uint8_t *udata = data;
    ret_len = share_space_write(udata, len);
    if(ret_len != len )
    {
        GAM_DEBUG_errer("write space err %d %d!\n", ret_len,len );
    }
    return ret_len;
}

//=================================================================================================


