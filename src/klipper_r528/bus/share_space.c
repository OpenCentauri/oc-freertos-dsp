#include "../../include/bus/share_space.h"
#include "../../include/printer/basecmd.h"
#include <platform.h>
#include "delay.h"
#include "../../include/bus/msgboxx.h"
#include "../../include/board/gpio.h"

#define SHARE_SPACE_HEAD_END 1

 #if SHARE_SPACE_HEAD_END
#define SHARE_SPACE_HEAD_OFFSET (4096-sizeof(struct msg_head_t))
#else
   #define SHARE_SPACE_HEAD_OFFSET 0
#endif



#if 1
uint16_t sharespace_arm_addr[2];
uint16_t sharespace_dsp_addr[2];
volatile struct msg_head_t arm_head;
volatile struct msg_head_t dsp_head;

struct dts_sharespace_t dts_sharespace;
struct dts_sharespace_t mmap_sharespace;
static void sharespace_init(struct dts_sharespace_t *p_dts_sharespace ) {
	volatile struct spare_rtos_head_t *pstr = platform_head;
	volatile struct dts_msg_t *pdts = &pstr->rtos_img_hdr.dts_msg;
	int val = 0;
	val = pdts->dts_sharespace.status;
	if(val == DTS_OPEN) {
		p_dts_sharespace->dsp_write_addr = pdts->dts_sharespace.dsp_write_addr;
		p_dts_sharespace->dsp_write_size = pdts->dts_sharespace.dsp_write_size;
		p_dts_sharespace->arm_write_addr = pdts->dts_sharespace.arm_write_addr;
		p_dts_sharespace->arm_write_size = pdts->dts_sharespace.arm_write_size;
		p_dts_sharespace->dsp_log_addr = pdts->dts_sharespace.dsp_log_addr;
		p_dts_sharespace->dsp_log_size = pdts->dts_sharespace.dsp_log_size;
	}
}

struct msg_head_t {
    uint32_t read_addr;
    uint32_t write_addr;
    uint32_t init_state;
};

#if SHARE_SPACE_SYNC

struct state_head_t {
    volatile uint32_t dsp_wr_addr_state;
    volatile uint32_t dsp_wr_addr_dsp_cmd;
    volatile uint32_t dsp_wr_addr_arm_cmd;
    volatile uint32_t arm_wr_addr_state;
    volatile uint32_t arm_wr_addr_dsp_cmd;
    volatile uint32_t arm_wr_addr_arm_cmd;
};
volatile static struct state_head_t *volatile  state_p;
#endif
volatile uint8_t *read_p;
volatile uint8_t *write_p;
void GAM_DEBUG_printf_sendbuf_HEX(  )
{
         char *readaddr = write_p; 
         printf("--write_buf-%d--%d---------\n", dsp_head.write_addr, msgbox_new_msg[1]);
        int i = 0;
        for (i = 0; i < 4096; i++)
        {
            printf("%x ", readaddr[i]);
            if(readaddr[i] == 0x7e)
            {
                printf("\n%d: ", i+1 );
            }
            if( i == arm_head.read_addr)
            {
                printf("--write_addr-%d----------\n", i);
            }
        }
        printf("\n");
}

void GAM_DEBUG_printf_receivebuf_HEX(  )
{
         char *readaddr = read_p; 
         printf("--read_buf-%d--%d---------\n", arm_head.write_addr, dsp_head.read_addr);
        int i = 0;
        for (i = 0; i < 4096; i++)
        {
            printf("%x ", readaddr[i]);
            if(readaddr[i] == 0x7e)
            {
                printf("\n%d: ", i+1 );
            }
            if( i == arm_head.read_addr)
            {
                printf("--read_addr-%d----------\n", i+1);
            }
        }
        printf("\n");
}

void GAM_DEBUG_printf_HEX(uint8_t write ,uint8_t *buf, uint32_t buf_len  )
{    
}
void GAM_DEBUG_printf_HEX1(uint8_t write ,uint8_t *buf, uint32_t buf_len  )
{       //-----gg-2-----------
        int m = 0;
        if(write)
        {
            // printf("write to arm %3d:",buf_len );
            printf("DSP_ARM :%4d=%4d:%4d-%4d:%4d  %4d:  ", buf_len,dsp_head.write_addr,sharespace_dsp_addr[1],arm_head.read_addr,msgbox_new_msg[0], arm_head.init_state);
        }
        else{
            // printf("read fr arm %d:",buf_len );
            printf("ARM_DSP:%4d=%4d:%4d-%4d:%4d  %4d:  ", buf_len, arm_head.write_addr,msgbox_new_msg[1], dsp_head.read_addr,sharespace_dsp_addr[0], arm_head.init_state);
        }
       for(; m < buf_len; m++)
        {
            printf(" %x",buf[m]);
            // if (m%16 == 0 && m != 0)
            //     printf("\n");
        }
        printf("\n");

        GAM_DEBUG_printf_receivebuf_HEX();
        GAM_DEBUG_printf_sendbuf_HEX();

}
void sharespace_reinit(void)
{
    while(1)  
    {
        memcpy(&arm_head, (dts_sharespace.arm_write_addr + SHARE_SPACE_HEAD_OFFSET), sizeof(struct msg_head_t));
        if (arm_head.init_state == 1 && arm_head.write_addr != 0Xa5a5a5a5 && arm_head.read_addr != 0Xa5a5a5a5)
        {
            mmap_sharespace.arm_write_addr =   arm_head.write_addr;
            mmap_sharespace.dsp_write_addr =    arm_head.read_addr;
            break;
        }
        else if (arm_head.init_state == 2 && arm_head.write_addr != 0Xa5a5a5a5 && arm_head.read_addr != 0Xa5a5a5a5)
        {
            mmap_sharespace.arm_write_addr =   arm_head.write_addr;
            mmap_sharespace.dsp_write_addr =    arm_head.read_addr;
            arm_head.init_state == 1;           //---G-G-2022-08-03----
            memcpy((dts_sharespace.arm_write_addr + SHARE_SPACE_HEAD_OFFSET ), &arm_head, sizeof(struct msg_head_t));
            break;
        }
        // GAM_DEBUG_printf("init_state  %x\n",arm_head.init_state );
    }
    // mmap_sharespace.dsp_write_size = 4096;
    // mmap_sharespace.arm_write_size = 4096;
    // GAM_DEBUG_printf("arm_write_addr  %x\n",mmap_sharespace.arm_write_addr );
    // GAM_DEBUG_printf("dsp_write_addr  %x\n",mmap_sharespace.dsp_write_addr );
}
void share_space_clear(void)
{
    sharespace_init(&dts_sharespace);
    memset((dts_sharespace.arm_write_addr + SHARE_SPACE_HEAD_OFFSET  ),0xa5, sizeof(struct msg_head_t));
    GAM_DEBUG_printf("share_space_clear init_state  %x\n",0XA5 );
}
void share_space_init(void)
{
    sharespace_init(&dts_sharespace);
//   GAM_DEBUG_printf("arm_write_addr  %x  %x\n",dts_sharespace.arm_write_addr,dts_sharespace.arm_write_size );
//   GAM_DEBUG_printf("dsp_write_addr  %x  %x\n",dts_sharespace.dsp_write_addr,dts_sharespace.dsp_write_size );
    sharespace_reinit();  //-CS-G-G-2022-07-28-----
    // memset((char*)mmap_sharespace.arm_write_addr, 0, mmap_sharespace.arm_write_size);
    // memset((char*)mmap_sharespace.dsp_write_addr, 0, mmap_sharespace.dsp_write_size);
    read_p = (volatile uint8_t *)(mmap_sharespace.arm_write_addr  );
    write_p = (volatile uint8_t *)(mmap_sharespace.dsp_write_addr );
    #if SHARE_SPACE_SYNC
    state_p = (volatile struct state_head_t *)(dts_sharespace.dsp_log_addr );
   state_p->dsp_wr_addr_state = SYS_IDLE_STATE;
   state_p->dsp_wr_addr_dsp_cmd = SYS_IDLE_STATE;
   state_p->dsp_wr_addr_arm_cmd = SYS_IDLE_STATE;
   state_p->arm_wr_addr_state = SYS_IDLE_STATE;
   state_p->arm_wr_addr_dsp_cmd = SYS_IDLE_STATE;
   state_p->arm_wr_addr_arm_cmd = SYS_IDLE_STATE;
    GAM_DEBUG_printf("state_p  %p\n",state_p );
    #endif
    // GAM_DEBUG_printf("read_p  %p\n",read_p );
    // GAM_DEBUG_printf("write_p  %p\n",write_p );


 #if SHARE_SPACE_HEAD_END
    dsp_head.read_addr = sizeof(struct msg_head_t);
    dsp_head.write_addr = sizeof(struct msg_head_t);// + sizeof(struct debug_img_t);
    dsp_head.init_state = 1;
#else
    dsp_head.read_addr = sizeof(struct msg_head_t);
    dsp_head.write_addr = sizeof(struct msg_head_t);// + sizeof(struct debug_img_t);
    dsp_head.init_state = 1;
#endif

//    GAM_DEBUG_printf("dsp_head.read_addr",dsp_head.read_addr);
    // GAM_DEBUG_printf("dsp_head.write_addr",dsp_head.write_addr);
    sharespace_dsp_addr[SHARESPACE_READ] = dsp_head.read_addr;
    sharespace_dsp_addr[SHARESPACE_WRITE] = dsp_head.write_addr;
    memcpy((mmap_sharespace.dsp_write_addr + SHARE_SPACE_HEAD_OFFSET ), &dsp_head, sizeof(struct msg_head_t));
     #if MSGBOX_EN_SYNC || MSGBOX_EN_WR_OK
    msgbox_send_signal((dsp_head.write_addr<<16) + dsp_head.read_addr);
    #endif
    while(1)    //-CS-G-G-2022-07-28-----
    {
        memcpy(&arm_head, (mmap_sharespace.arm_write_addr + SHARE_SPACE_HEAD_OFFSET), sizeof(struct msg_head_t));
        if (arm_head.init_state == 1)
        {
            sharespace_arm_addr[SHARESPACE_READ] = arm_head.read_addr; 
            sharespace_arm_addr[SHARESPACE_WRITE] = arm_head.write_addr;     
            printf("arm share space finish init! %d %d  \n",arm_head.read_addr,arm_head.write_addr);
            return;
        } 
    }  
}

#if SHARE_SPACE_SYNC
uint32_t share_space_set_read_idle_state()
{
    // DisTraceMsg();
    if( (state_p->arm_wr_addr_dsp_cmd) == SYS_IDLE_STATE )
    {   
        return 2;
    }
    while( (state_p->arm_wr_addr_dsp_cmd) != SYS_IDLE_STATE )
    {   
        if((state_p->arm_wr_addr_dsp_cmd) == DSP_READ_STATE )
        {
             if((state_p->arm_wr_addr_state) == DSP_READ_STATE ) 
             {
                // DisTraceMsg();
                while((state_p->arm_wr_addr_state) != SYS_IDLE_STATE ) 
                {
                    state_p->arm_wr_addr_state = SYS_IDLE_STATE;
                }
                 while((state_p->arm_wr_addr_dsp_cmd) != SYS_IDLE_STATE ) 
                {
                    state_p->arm_wr_addr_dsp_cmd = SYS_IDLE_STATE;
                } 
            //    GAM_DEBUG_printf("share_space_SDA %d:  %x %x %x \n",__LINE__,state_p->arm_wr_addr_state ,state_p->arm_wr_addr_dsp_cmd,state_p->arm_wr_addr_arm_cmd );
 
                return 1;
             }
            else if((state_p->arm_wr_addr_state) == SYS_IDLE_STATE ) 
            {
            //   DisTraceMsg();
               while((state_p->arm_wr_addr_dsp_cmd) != SYS_IDLE_STATE ) 
                {
                    state_p->arm_wr_addr_dsp_cmd = SYS_IDLE_STATE;
                    // DisTraceMsg();
                }
                return 1;
            }
             else{
            //    GAM_DEBUG_printf("share_space_SDA %d:  %x %x %x \n",__LINE__,state_p->arm_wr_addr_state ,state_p->arm_wr_addr_dsp_cmd,state_p->arm_wr_addr_arm_cmd );
 
             }
        } 
    }
    // GAM_DEBUG_printf("share_space_SDA %d:  %x %x %x \n",__LINE__,state_p->arm_wr_addr_state ,state_p->arm_wr_addr_dsp_cmd,state_p->arm_wr_addr_arm_cmd );
 
    return 1;
}
uint32_t share_space_set_read_state()
{
    if(((state_p->arm_wr_addr_state) == DSP_READ_STATE )&& ( state_p->arm_wr_addr_dsp_cmd == DSP_READ_STATE))
    {
         return 1;
    }
    if((state_p->arm_wr_addr_arm_cmd) != SYS_IDLE_STATE ) 
    {
         return 0;
    }
    while( (state_p->arm_wr_addr_state) != DSP_READ_STATE )
    {   
       state_p->arm_wr_addr_dsp_cmd = DSP_READ_STATE;
        if((state_p->arm_wr_addr_dsp_cmd) == DSP_READ_STATE )
        {
             if((state_p->arm_wr_addr_arm_cmd) == SYS_IDLE_STATE ) 
             {
                state_p->arm_wr_addr_state = DSP_READ_STATE;
                while((state_p->arm_wr_addr_state) != DSP_READ_STATE ) 
                {
                    state_p->arm_wr_addr_state = DSP_READ_STATE;
                }
                return 1;
             }
             else{
                    state_p->arm_wr_addr_dsp_cmd = SYS_IDLE_STATE;
                    while((state_p->arm_wr_addr_dsp_cmd) != SYS_IDLE_STATE ) 
                    {
                        GAM_DEBUG_printf("share_space_SDA %d: %x %x %x\n",__LINE__,state_p->arm_wr_addr_state ,state_p->arm_wr_addr_dsp_cmd,state_p->arm_wr_addr_arm_cmd );
                        state_p->arm_wr_addr_dsp_cmd = SYS_IDLE_STATE;
                    }
                    return 0;
             }
        } 
        else{
            state_p->arm_wr_addr_dsp_cmd = SYS_IDLE_STATE;
          while((state_p->arm_wr_addr_dsp_cmd) != SYS_IDLE_STATE ) 
            {
                GAM_DEBUG_printf("share_space_SDA %d: %x %x %x\n",__LINE__,state_p->arm_wr_addr_state ,state_p->arm_wr_addr_dsp_cmd,state_p->arm_wr_addr_arm_cmd );
                state_p->arm_wr_addr_dsp_cmd = SYS_IDLE_STATE;
            }
              return 0;
        }
    }
}
uint32_t share_space_set_write_idle_state()
{
    while( (state_p->dsp_wr_addr_dsp_cmd) != SYS_IDLE_STATE )
    {   
        if((state_p->dsp_wr_addr_dsp_cmd) == DSP_WRITE_STATE )
        {
             if((state_p->dsp_wr_addr_state) == DSP_WRITE_STATE ) 
             {
                while((state_p->dsp_wr_addr_state) != SYS_IDLE_STATE ) 
                {
                    state_p->dsp_wr_addr_state = SYS_IDLE_STATE;
                }
               while((state_p->dsp_wr_addr_dsp_cmd) != SYS_IDLE_STATE ) 
                {
                    state_p->dsp_wr_addr_dsp_cmd = SYS_IDLE_STATE;
                }
                return 1;
             }
            else if((state_p->dsp_wr_addr_state) == SYS_IDLE_STATE ) 
            {
               while((state_p->dsp_wr_addr_dsp_cmd) != SYS_IDLE_STATE ) 
                {
                    state_p->dsp_wr_addr_dsp_cmd = SYS_IDLE_STATE;
                }
                return 1;
            }
             else{
                //  GAM_DEBUG_printf("share_space_SDA %d: %x %x %x\n",__LINE__,state_p->dsp_wr_addr_state ,state_p->dsp_wr_addr_dsp_cmd,state_p->dsp_wr_addr_arm_cmd );
             }
        } 
    }
    return 1;
}
uint32_t share_space_set_write_state()
{
   
    while((state_p->dsp_wr_addr_arm_cmd) != SYS_IDLE_STATE ) 
    {
                    // GAM_DEBUG_printf("share_space_set_write_state 1  SDA %x %x %x\n",state_p->dsp_wr_addr_state ,state_p->dsp_wr_addr_dsp_cmd,state_p->dsp_wr_addr_arm_cmd );
    }
    while( (state_p->dsp_wr_addr_state) != DSP_WRITE_STATE )
    {   
       state_p->dsp_wr_addr_dsp_cmd = DSP_WRITE_STATE;
        if((state_p->dsp_wr_addr_dsp_cmd) == DSP_WRITE_STATE )
        {
             if((state_p->dsp_wr_addr_arm_cmd) == SYS_IDLE_STATE ) 
             {
                while((state_p->dsp_wr_addr_state) != DSP_WRITE_STATE ) 
                {
                    state_p->dsp_wr_addr_state = DSP_WRITE_STATE;
                }
                return 1;
             }
             else{
                 GAM_DEBUG_printf("share_space_SDA %d: %x %x %x\n",__LINE__,state_p->dsp_wr_addr_state ,state_p->dsp_wr_addr_dsp_cmd,state_p->dsp_wr_addr_arm_cmd );
                //     while((state_p->dsp_wr_addr_dsp_cmd) != SYS_IDLE_STATE ) 
                //     {
                //         state_p->dsp_wr_addr_dsp_cmd = SYS_IDLE_STATE;
                //     }
                //    return 0;
             }
        } 
        else{
                GAM_DEBUG_printf("share_space_SDA %d: %x %x %x\n",__LINE__,state_p->dsp_wr_addr_state ,state_p->dsp_wr_addr_dsp_cmd,state_p->dsp_wr_addr_arm_cmd );
            // while((state_p->dsp_wr_addr_dsp_cmd) != SYS_IDLE_STATE ) 
            // {
            //     state_p->dsp_wr_addr_dsp_cmd = SYS_IDLE_STATE;
            // }
            // return 0;
        }
    }
}
#endif

int share_space_write(uint8_t *pstr, uint32_t size)
{
    #if MSGBOX_EN_SYNC
    while(msgbox_new_msg[0] >= 5000)
    {
            GAM_DEBUG_printf("share_space_write wait msg  %d %d\n",msgbox_new_msg[0],msgbox_new_msg[1] );
    }
    #elif SHARE_SPACE_SYNC
        while (!share_space_set_write_state())
        {
                GAM_DEBUG_printf("share_space_set_write_state  %d %d\n",msgbox_new_msg[0],msgbox_new_msg[1] );
        }
    #endif
    arm_head.read_addr = msgbox_new_msg[0];

    #if MSGBOX_EN_SYNC
    msgbox_send_signal((dsp_head.write_addr<<16) + dsp_head.read_addr);//dsp_head.write_addr;//5000;
    #endif

    uint32_t i = 0;
    uint32_t write_size = 0;
    int t = 0;

#if SHARE_SPACE_HEAD_END
     uint32_t max_addr = 4096-(sizeof(struct msg_head_t));
     uint32_t min_addr = (sizeof(struct msg_head_t)  ) ;
#else
     uint32_t max_addr = 4096;
     uint32_t min_addr = (sizeof(struct msg_head_t)  );
#endif


    if ((dsp_head.write_addr >= arm_head.read_addr) && ((max_addr - min_addr  - (dsp_head.write_addr - arm_head.read_addr)) < size) )
    {
        GAM_DEBUG_errer("write space not enough!\n");
        return -1;
    }
    if ((dsp_head.write_addr < arm_head.read_addr) && ( (arm_head.read_addr - dsp_head.write_addr) < size))
    {
        printf("write space not enough!\n");
        return -1;
    }
    // GAM_DEBUG_printf("dsp share_space_write start:--%d-%d: ", dsp_head.write_addr,arm_head.read_addr  );
    uint32_t end_addr = (dsp_head.write_addr + size  );
    if (end_addr >=  max_addr )
        end_addr = min_addr + (end_addr - max_addr);
    uint32_t  start_addr = dsp_head.write_addr   ;
   while (start_addr != end_addr)
    {
        write_p[start_addr++] = *pstr;
        pstr++;
        write_size++;
        if (start_addr >=  max_addr )
            start_addr = min_addr;
    }
    dsp_head.write_addr = start_addr;

    // GAM_DEBUG_printf("\ndsp share_space_write: end--%d:%d \n", dsp_head.write_addr,size   );
    memcpy((mmap_sharespace.dsp_write_addr  + SHARE_SPACE_HEAD_OFFSET ), &dsp_head, sizeof(struct msg_head_t));
    sharespace_arm_addr[SHARESPACE_READ] = arm_head.read_addr; 
    sharespace_dsp_addr[SHARESPACE_WRITE] = dsp_head.write_addr;

    #if SHARE_SPACE_SYNC
        while (!share_space_set_write_idle_state())
        {
                // GAM_DEBUG_printf("share_space_set_write_state  %d %d\n",msgbox_new_msg[0],msgbox_new_msg[1] );
        }
        //  GAM_DEBUG_printf("share_space_SDA %x %x %x\n",state_p->arm_wr_addr_state ,state_p->arm_wr_addr_dsp_cmd,state_p->arm_wr_addr_arm_cmd );
        // DisTraceMsg();
    #endif


    #if MSGBOX_EN_SYNC || MSGBOX_EN_WR_OK
    msgbox_send_signal((dsp_head.write_addr<<16) + dsp_head.read_addr);
    #endif
    GAM_DEBUG_printf_HEX(1,pstr-write_size,write_size);
    // gpio_out_toggle_noirq(&g_GAM_DBG_gpio3);
    return write_size;
}



int share_space_read(uint8_t* pstr, uint32_t size)
{
#if SHARE_SPACE_HEAD_END
     uint32_t max_addr = 4096-(sizeof(struct msg_head_t));
     uint32_t min_addr = (sizeof(struct msg_head_t)  ) ;
#else
     uint32_t max_addr = 4096;
     uint32_t min_addr = (sizeof(struct msg_head_t)  );
#endif
    // memcpy(&arm_head,(dts_sharespace.arm_write_addr + SHARE_SPACE_HEAD_OFFSET ), sizeof(struct msg_head_t));
    // {
    //      if ( arm_head.init_state != 1)
    //      {
    //          command_reset(NULL);       //--G-G-2022-08-03---
    //      }
    // }
    memcpy(&arm_head,(mmap_sharespace.arm_write_addr + SHARE_SPACE_HEAD_OFFSET ), sizeof(struct msg_head_t));
    if ( (arm_head.init_state == 0) || (arm_head.init_state == 3) || (arm_head.write_addr >= max_addr) || (arm_head.write_addr < min_addr) )
    {
        GAM_DEBUG_printf("share_space_read  %d %d  %d\n",arm_head.init_state ,arm_head.read_addr,arm_head.write_addr);
        return -1;
    }
    if (dsp_head.read_addr == arm_head.write_addr)
    {
          return -1;
    }
    uint32_t read_size = 0;
    uint32_t i = 0;



    uint32_t end_addr = (arm_head.write_addr );
    uint32_t  start_addr = dsp_head.read_addr  ;

    if ((end_addr >= max_addr  ) ||  (end_addr < min_addr  ) )
    {
        return -1;
    }
   while (start_addr != end_addr)
    {
        *pstr = read_p[start_addr++];
        pstr++;
        read_size++;
        if (start_addr >=  max_addr )
            start_addr = min_addr;
    }
    // if (start_addr <= end_addr)
    // {
    //      read_size = end_addr-start_addr;
    //      start_addr = end_addr;
    //     memcpy(pstr,(read_p + start_addr ),read_size);
        
    // }
    // else{
    //     int len1 = max_addr-start_addr;
    //     memcpy(pstr, (read_p + start_addr ),len1);
    //     memcpy(pstr+len1, (read_p + min_addr ),end_addr-min_addr);
    //     read_size = len1 + end_addr-min_addr;
    //      start_addr = end_addr;
    // }


    sharespace_arm_addr[SHARESPACE_WRITE] = arm_head.write_addr; 
    sharespace_dsp_addr[SHARESPACE_READ] = start_addr;
// gpio_out_toggle_noirq(&g_GAM_DBG_gpio4);
    return read_size;
}


int do_read_space(uint8_t* buf, uint32_t size)
{
    int read_size = 0;
  #if MSGBOX_EN_SYNC
    while(msgbox_new_msg[1] >= 5000)
    {
            GAM_DEBUG_printf("share_space_read wait msg  %d %d\n",msgbox_new_msg[0],msgbox_new_msg[1] );
    }
#endif
   #if  MSGBOX_EN_SYNC
        msgbox_send_signal((dsp_head.write_addr<<16) + dsp_head.read_addr);//dsp_head.read_addr;//5000;
    #endif
    read_size = share_space_read(buf, size);
    if( read_size > 0 )
    {
      GAM_DEBUG_printf_HEX(0,buf,read_size);
        if( ( buf[0] == 0 ) || ( buf[0] == 0xa5 ) || ( buf[read_size-1] != 0x7e ))
        {
            read_size = 0;
        }
    }

    if( read_size > 0 )
    {
        dsp_head.read_addr = sharespace_dsp_addr[SHARESPACE_READ];
        #if MSGBOX_EN_SYNC || MSGBOX_EN_RD_OK
        msgbox_send_signal((dsp_head.write_addr<<16) + dsp_head.read_addr);//  
        #endif
    }
    else{
        sharespace_dsp_addr[SHARESPACE_READ] = dsp_head.read_addr;
    }

    return read_size;
}


//=====================================================================================================

#endif


