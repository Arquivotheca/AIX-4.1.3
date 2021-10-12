/* @(#)36	1.7.1.5  src/bos/kernext/disp/ped/inc/midhwa.h, peddd, bos411, 9428A410j 5/12/94 09:38:43 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: 
 *		FIFO_WR
 *		IPIO_EXC_OFF
 *		IPIO_EXC_ON
 *		PCB_WR
 *		PIO_EXC_OFF
 *		PIO_EXC_ON
 *		TRACE_DATA
 *		TRACE_TOKEN
 *		write__fifo
 *		write_fifo
 *		write_fifo1
 *		write_fifo10
 *		write_fifo11
 *		write_fifo12
 *		write_fifo13
 *		write_fifo14
 *		write_fifo15
 *		write_fifo16
 *		write_fifo2
 *		write_fifo3
 *		write_fifo4
 *		write_fifo5
 *		write_fifo6
 *		write_fifo7
 *		write_fifo8
 *		write_fifo9
 *		write_pcb
 *		write_pcb8
 *		write_pcbX
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_MIDHWA.H
#define _H_MIDHWA.H


#define LOADABLE

#include <fcntl.h>
#include "bishm.h"
#include "bidefs.h"
#include <hw_dd_model.h>
#include <hw_locks.h>

/*  VDD types.h has things like ushort ulong in it */
#include <sys/types.h>

#include <sys/adspace.h> /* for the BUSMEM_ATT() & BUSMEM_DET() macros        */

#define BUS_ID                          0x820C0020

#define HWPGDPSetup                                                     \
        label_t jmpbuf;                                                 \
        unsigned int save_bus_acc = 0;                                  \
        volatile unsigned long HWP = ((midddf_t *)                      \
                gdp->devHead.display->free_area)->HWP;                  \
        ASSERT(gdp != NULL && HWP != NULL);

#define HWPVPSetup                                                      \
        label_t jmpbuf;                                                 \
        unsigned int save_bus_acc = 0;                                  \
        volatile unsigned long HWP                                      \
        = ((midddf_t *)vp->display->free_area)->HWP;                    \
        ASSERT(vp != NULL && HWP != NULL);

#define HWPPPSetup                                                      \
        label_t jmpbuf;                                                 \
        unsigned int save_bus_acc = 0;                                  \
        volatile unsigned long HWP                                      \
        = ((midddf_t *)                                                 \
        pproc->pGSC->devHead.display->free_area)->HWP;                  \
        ASSERT(pproc != NULL && HWP != NULL);

#define HWPDVSetup                                                      \
        label_t jmpbuf;                                                 \
        unsigned int save_bus_acc = 0;                                  \
        volatile unsigned long HWP                                      \
        = ((midddf_t *)                                                 \
        pdev->devHead.display->free_area)->HWP;                         \
        ASSERT(pdev != NULL && HWP != NULL);

#define HWPPDSetup                                                      \
        label_t jmpbuf;                                                 \
        unsigned int save_bus_acc = 0;                                  \
        volatile unsigned long HWP                                      \
        = ((midddf_t *)pd->free_area)->HWP;                             \
        ASSERT(pd != NULL && HWP != NULL);

#define HWPDDFSetup                                                     \
        label_t jmpbuf;                                                 \
	unsigned int save_bus_acc = 0;					\
        volatile unsigned long MIDHWP = ddf->HWP;                       \
        ulong   _mid_lock_mode;            /* KSR vs Monitor Mode    */ \
        ulong   _mid_ind_save_adr;         /* Ind Addr Reg save area */ \
        ulong   _mid_ind_save_ctl;         /* Ind Ctrl Reg save area */ \
        ulong   _mid_ind_old_priority;     /* Intr Priority save area*/ \
        ulong   _mid_ind_access_is_locked; /* Ind access lock state  */ \
        ASSERT(ddf != NULL && ddf->bus_offset != NULL);



#ifdef  NO_PIO_EXC                      /* disable PIO exception handling */

#define PIO_EXC_ON()							\
	save_bus_acc = ddf->HWP;					\
        MIDHWP = ddf->HWP = BUSMEM_ATT(BUS_ID, ddf->bus_offset);			

#define PIO_EXC_OFF()							\
	BUSMEM_DET( MIDHWP );						\
	ddf->HWP = save_bus_acc;

#define IPIO_EXC_ON()							\
	save_bus_acc = ddf->HWP;					\
        MIDHWP = ddf->HWP = BUSMEM_ATT(BUS_ID, ddf->bus_offset);                     

#define IPIO_EXC_OFF()							\
	BUSMEM_DET( MIDHWP );						\
	ddf->HWP = save_bus_acc;

#else                                   /* enable PIO exception handling */


#define PIO_EXC_ON()                                                    \
{                                                                       \
        int rc;                                                         \
        BUGLPR(dbg_middd,99,("PIO_EXC_ON\n"));                          \
	ASSERT(!save_bus_acc);						\
        if((rc=setjmpx(&jmpbuf)) != 0) {                                \
                char msgbuf[sizeof(__FILE__)+6];                        \
                pio_exc_on (rc, &jmpbuf); 		                \
		BUSMEM_DET( MIDHWP );					\
		ddf->HWP = save_bus_acc;				\
                sprintf(msgbuf, "%s,%d",__FILE__,__LINE__);             \
                miderr(NULL,NULL,ddf->component,"BUSMEM_ATT","setjmpx", \
                        EXCEPT_IO,NULL,msgbuf);                         \
                BUGLPR(dbg_middd,0,("PIO_EXC\n"));                      \
                return -1;                                              \
        }                                                               \
	save_bus_acc = ddf->HWP;					\
        MIDHWP = ddf->HWP = BUSMEM_ATT(BUS_ID, ddf->bus_offset);	\
}


#define PIO_EXC_OFF()							\
	BUGLPR(dbg_middd,99,("PIO_EXC_OFF\n")); 			\
	ASSERT( MIDHWP );						\
	clrjmpx( &jmpbuf );						\
	BUSMEM_DET( MIDHWP );						\
	ddf->HWP = save_bus_acc;


#define IPIO_EXC_ON()                                                   \
        if(setjmpx(&ipio_jmpbuf) == EXCEPT_IO) {                        \
                char msgbuf[sizeof(__FILE__)+6];                        \
		BUSMEM_DET( MIDHWP );					\
		ddf->HWP = save_bus_acc;				\
                sprintf(msgbuf, "%s,%d",__FILE__,__LINE__);             \
                miderr(NULL,NULL,ddf->component,"BUSMEM_ATT","setjmpx",	\
                        EXCEPT_IO,NULL,msgbuf);                         \
                return -1;                                              \
        }                                                               \
	save_bus_acc = ddf->HWP;					\
        MIDHWP = ddf->HWP = BUSMEM_ATT(BUS_ID, ddf->bus_offset);                     

#define IPIO_EXC_OFF()                                                  \
        clrjmpx( &ipio_jmpbuf );                                        \
	BUSMEM_DET( MIDHWP );						\
	ddf->HWP = save_bus_acc;	
	

#endif                          /* PIO exception handling */





#ifdef FIFOTRACE

#define TOKEN_TRACE_SIZE        256

struct fifo_tr {
        int token;
        int data[7];
};

extern int token_trace_index;
extern struct fifo_tr token_trace[];

#define TRACE_TOKEN(cmd) \
        token_trace_index = (++token_trace_index)&(TOKEN_TRACE_SIZE-1); \
        token_trace[token_trace_index].data[0] = 0;                     \
        token_trace[token_trace_index].data[1] = 0;                     \
        token_trace[token_trace_index].data[2] = 0;                     \
        token_trace[token_trace_index].data[3] = 0;                     \
        token_trace[token_trace_index].data[4] = 0;                     \
        token_trace[token_trace_index].data[5] = 0;                     \
        token_trace[token_trace_index].data[6] = 0;                     \
        token_trace[token_trace_index].token = cmd;

#define TRACE_DATA(t_data, data_item) \
{                                                                       \
        volatile int tr_dat_in_1;                                       \
        for(tr_dat_in_1=0;tr_dat_in_1<1000;tr_dat_in_1++)               \
                tr_dat_in_1--;                                          \
        token_trace[token_trace_index].data[data_item] = t_data;        \
}
#else
#define TRACE_TOKEN(cmd)
#define TRACE_DATA(data, data_item)
#endif

typedef struct s1 {
        long x1[1];
} s1_t;

typedef struct s2 {
        long x2[2];
} s2_t;

typedef struct s3 {
        long x3[3];
} s3_t;

typedef struct s4 {
        long x4[4];
} s4_t;

typedef struct s5 {
        long x5[5];
} s5_t;

typedef struct s6 {
        long x6[6];
} s6_t;

typedef struct s7 {
        long x7[7];
} s7_t;

typedef struct s8 {
        long x8[8];
} s8_t;

typedef struct s9 {
        long x9[9];
} s9_t;

typedef struct s10 {
        long x10[10];
} s10_t;

typedef struct s11 {
        long x11[11];
} s11_t;

typedef struct s12 {
        long x12[12];
} s12_t;

typedef struct s13 {
        long x13[13];
} s13_t;

typedef struct s14 {
        long x14[14];
} s14_t;

typedef struct s15 {
        long x15[15];
} s15_t;

typedef struct s16 {
        long x16[16];
} s16_t;


#if 0
#define BLOCK_SIZE                      8       /* size of pcb */
#define FIFO_SIZE                       16      /* size of fifo 'mouth' */
#define MID_DMA_TIMEDOUT                0xFFFFFFFF
#endif



/* write a long into the pcb */
#define FIFO_WR(x)                                                      \
        *(volatile ulong *)(HWP|H_PIO_IN_PORT_0) = x;

/* write a long into the pcb */
#define PCB_WR(x)                                                       \
        *(volatile ulong *)(HWP|H_PCB_IN_PORT_0) = x;

/* write 1 integer into the fifo */
#define write_fifo1(data)                                               \
{                                                                       \
        s1_t *s1 = (s1_t *) data;                                       \
        s1_t *d1 = (s1_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d1 = *s1;                                                      \
}

/* write 2 integers into the fifo */
#define write_fifo2(data)                                               \
{                                                                       \
        s2_t *s2 = (s2_t *) data;                                       \
        s2_t *d2 = (s2_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d2 = *s2;                                                      \
}

/* write 3 integers into the fifo */
#define write_fifo3(data)                                               \
{                                                                       \
        s3_t *s3 = (s3_t *) data;                                       \
        s3_t *d3 = (s3_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d3 = *s3;                                                      \
}

/* write 4 integers into the fifo */
#define write_fifo4(data)                                               \
{                                                                       \
        s4_t *s4 = (s4_t *) data;                                       \
        s4_t *d4 = (s4_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d4 = *s4;                                                      \
}

/* write 5 integers into the fifo */
#define write_fifo5(data)                                               \
{                                                                       \
        s5_t *s5 = (s5_t *) data;                                       \
        s5_t *d5 = (s5_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d5 = *s5;                                                      \
}

/* write 6 integers into the fifo */
#define write_fifo6(data)                                               \
{                                                                       \
        s6_t *s6 = (s6_t *) data;                                       \
        s6_t *d6 = (s6_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d6 = *s6;                                                      \
}

/* write 7 integers into the fifo */
#define write_fifo7(data)                                               \
{                                                                       \
        s7_t *s7 = (s7_t *) data;                                       \
        s7_t *d7 = (s7_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d7 = *s7;                                                      \
}

/* write 8 integers into the fifo */
#define write_fifo8(data)                                               \
{                                                                       \
        s8_t *s8 = (s8_t *) data;                                       \
        s8_t *d8 = (s8_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d8 = *s8;                                                      \
}

/* write 9 integers into the fifo */
#define write_fifo9(data)                                               \
{                                                                       \
        s9_t *s9 = (s9_t *) data;                                       \
        s9_t *d9 = (s9_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);     \
        *d9 = *s9;                                                      \
}

/* write 10 integers into the fifo */
#define write_fifo10(data)                                              \
{                                                                       \
        s10_t *s10 = (s10_t *) data;                                    \
        s10_t *d10 = (s10_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);  \
        *d10 = *s10;                                                    \
}

/* write 11 integers into the fifo */
#define write_fifo11(data)                                              \
{                                                                       \
        s11_t *s11 = (s11_t *) data;                                    \
        s11_t *d11 = (s11_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);  \
        *d11 = *s11;                                                    \
}

/* write 12 integers into the fifo */
#define write_fifo12(data)                                              \
{                                                                       \
        s12_t *s12 = (s12_t *) data;                                    \
        s12_t *d12 = (s12_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);  \
        *d12 = *s12;                                                    \
}

/* write 13 integers into the fifo */
#define write_fifo13(data)                                              \
{                                                                       \
        s13_t *s13 = (s13_t *) data;                                    \
        s13_t *d13 = (s13_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);  \
        *d13 = *s13;                                                    \
}

/* write 14 integers into the fifo */
#define write_fifo14(data)                                              \
{                                                                       \
        s14_t *s14 = (s14_t *) data;                                    \
        s14_t *d14 = (s14_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);  \
        *d14 = *s14;                                                    \
}

/* write 15 integers into the fifo */
#define write_fifo15(data)                                              \
{                                                                       \
        s15_t *s15 = (s15_t *) data;                                    \
        s15_t *d15 = (s15_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);  \
        *d15 = *s15;                                                    \
}

/* write 16 integers into the fifo */
#define write_fifo16(data)                                              \
{                                                                       \
        s16_t *s16 = (s16_t *) data;                                    \
        s16_t *d16 = (s16_t *)(volatile ulong *)(HWP|H_PIO_IN_PORT_0);  \
        *d16 = *s16;                                                    \
}

/* write integers into the fifo */
#define write_fifo(data,size)                                           \
{                                                                       \
        long    tsize = size;                                           \
        long    i;                                                      \
        tsize -= FIFO_SIZE;                                             \
        BUGLPR(dbg_middd, BUGNTA, ("size = 0x%x\n", size));             \
        for (i = 0; i <= tsize; i+=FIFO_SIZE)                           \
        {                                                               \
                /*----------------------------*/                        \
                /* Send 16 words to the fifo. */                        \
                /*----------------------------*/                        \
                                                                        \
                write_fifo16(&data[i]);                                 \
        }                                                               \
        tsize += FIFO_SIZE - i;                                         \
                                                                        \
        /*--------------------------------------------*/                \
        /* Send the last 0 - 15 words to the fifo.    */                \
        /*--------------------------------------------*/                \
                                                                        \
        switch (size) {                                                 \
                case 0:                                                 \
                        /* nothing to do */                             \
                break;                                                  \
                case 1:                                                 \
                        write_fifo1(&data[i]);                          \
                break;                                                  \
                case 2:                                                 \
                        write_fifo2(&data[i]);                          \
                break;                                                  \
                case 3:                                                 \
                        write_fifo3(&data[i]);                          \
                break;                                                  \
                case 4:                                                 \
                        write_fifo4(&data[i]);                          \
                break;                                                  \
                case 5:                                                 \
                        write_fifo5(&data[i]);                          \
                break;                                                  \
                case 6:                                                 \
                        write_fifo6(&data[i]);                          \
                break;                                                  \
                case 7:                                                 \
                        write_fifo7(&data[i]);                          \
                break;                                                  \
                case 8:                                                 \
                        write_fifo8(&data[i]);                          \
                break;                                                  \
                case 9:                                                 \
                        write_fifo9(&data[i]);                          \
                break;                                                  \
                case 10:                                                \
                        write_fifo10(&data[i]);                         \
                break;                                                  \
                case 11:                                                \
                        write_fifo11(&data[i]);                         \
                break;                                                  \
                case 12:                                                \
                        write_fifo12(&data[i]);                         \
                break;                                                  \
                case 13:                                                \
                        write_fifo13(&data[i]);                         \
                break;                                                  \
                case 14:                                                \
                        write_fifo14(&data[i]);                         \
                break;                                                  \
                case 15:                                                \
                        write_fifo15(&data[i]);                         \
                break;                                                  \
                default:                                                \
                BUGLPR(dbg_middd, 0, ("bad token count 0x%x\n",i));     \
                break;                                                  \
        }                                                               \
}

/* write integers into the fifo */
#define write__fifo(data,size)                                          \
{                                                                       \
        ulong _write_fifo_loop_ctr;                                     \
        ulong *dataptr;                                                 \
        for (_write_fifo_loop_ctr=0, dataptr=(data);                    \
                _write_fifo_loop_ctr < size;                            \
                _write_fifo_loop_ctr++,dataptr++)                       \
        {                                                               \
                BUGLPR(dbg_middd, BUGNTA, ("data  = 0x%x\n", *dataptr));\
                FIFO_WR(*dataptr);                                      \
        }                                                               \
}

/* write integers into the priority command buffer */
#define write_pcbX(data,xsize)                          \
{                                                       \
        ulong _write_pcb_loop_ctr;                      \
        ulong *dataptr;                                 \
        for (_write_pcb_loop_ctr=0, dataptr=(data);     \
                _write_pcb_loop_ctr < xsize;            \
                _write_pcb_loop_ctr++,dataptr++)        \
        {                                               \
                BUGLPR(dbg_middd, BUGNTA, ("data  = 0x%x\n", *dataptr)); \
                PCB_WR(*dataptr);                       \
        }                                               \
}

/* write integers into the priority command buffer */
#define write_pcb8(data)                                                \
{                                                                       \
        s8_t *s8 = (s8_t *) data;                                       \
        s8_t *d8 = (s8_t *)(volatile ulong *)(HWP|H_PCB_IN_PORT_0);     \
        *d8 = *s8;                                                      \
}

/* write integers into the priority command buffer */
#define write_pcb(data,pcb_size)                                        \
{                                                                       \
        long i;                                                         \
        long _pcb_size = pcb_size;                                      \
        _pcb_size -= BLOCK_SIZE;                                        \
        BUGLPR(dbg_middma, BUGNTA, ("size = 0x%x\n", _pcb_size));       \
        for (i = 0; i < _pcb_size; i+=BLOCK_SIZE)                       \
        {                                                               \
                /*----------------------------------------*/            \
                /* Wait for the pcb to be free.           */            \
                /*----------------------------------------*/            \
                                                                        \
                wait_on_pcb();                                          \
                                                                        \
                /*----------------------------------------*/            \
                /* Send down the data to the pcb.         */            \
                /*----------------------------------------*/            \
                                                                        \
                write_pcb8(&data[i]);                                   \
                                                                        \
        }                                                               \
        _pcb_size += BLOCK_SIZE - i;                                    \
                                                                        \
        /*----------------------------------------*/                    \
        /* Wait for the pcb to be free.           */                    \
        /*----------------------------------------*/                    \
                                                                        \
        wait_on_pcb();                                                  \
                                                                        \
        /*----------------------------------------*/                    \
        /* Send down the data to the pcb.         */                    \
        /*----------------------------------------*/                    \
                                                                        \
        write_pcbX(&data[i],_pcb_size);                                 \
                                                                        \
}


#endif  /* end of midhwa.h */
