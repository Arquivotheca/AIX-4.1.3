/* @(#)99	1.24  src/bos/kernel/sys/dma.h, sysios, bos41J, 9513A_all 3/25/95 11:57:15 */
#ifndef _H_DMA
#define _H_DMA
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: DMA external interface definition.
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sysdma.h>
#include <sys/types.h>
#include <sys/xmem.h>

/***********************  INITIALIZATION FLAGS ******************************/

/*	
 * These are the flag values common between the d_init and d_map_init services.
 */
#define		DMA_SLAVE	0x10		/* device is a DMA slave  */
#define		DMA_MASTER	0x00		/* device is a DMA master */

/*
* These are the flag values specific to the d_init service.
* These flags must each be unique with respect to each other and the common
* flags.
*/
#define		DMA_BUS_MASK	0x0F		/* bus type mask	  */
#define		PC_AT_DMA	0x01		/* PC AT BUS		  */
#define		MICRO_CHANNEL_DMA 0x02		/* Micro channel bus      */
#define		DMA_BUS_MAX	0x02		/* largest valid bus	  */
#define		REGION_DMA	0x20		/* region mode  	  */
#define		BUFFERED_DMA	0x40		/* buffer DMA  RT PC only */

/*
* These are the flag values specific to the d_map_init service.
* These flags must each be unique with respect to each other and the common
* flags.
*/
#define		DMA_ADDRESS_64_BIT	0x80	/* can drive 64-bit address */

	/* The DMA_MAXMIN_* encoded flag values represent the maximum
	 * possible minxfer value for any device attached to an adapter.
	 * The default is 64K so these flags should be used only if the
	 * device needs a guaranteed mapping for a single transfer that
	 * exceeds the default.
	 */
#define		DMA_MAXMIN_64K		0x0	/* 64K byte max minxfer size */
#define		DMA_MAXMIN_128K		0x1	/* 128K byte max minxfer size */
#define		DMA_MAXMIN_256K		0x2	/* 256K byte max minxfer size */
#define		DMA_MAXMIN_512K		0x3	/* 512K byte max minxfer size */
#define		DMA_MAXMIN_1M		0x4	/* 1M byte max minxfer size */
#define		DMA_MAXMIN_2M		0x5	/* 2M byte max minxfer size */
#define		DMA_MAXMIN_4M		0x6	/* 4M byte max minxfer size */
#define		DMA_MAXMIN_8M		0x7	/* 8M byte max minxfer size */

/****************************  SERVICE FLAGS ******************************/
/*	
* These are the flag values common between the d_master/d_slave/d_complete 
* and d_map_page/d_map_list/d_map_slave services
*/
#define		DMA_READ	0x80		/* xfer from device to buffer*/
#define		BUS_DMA		0x100		/* buffer in bus memory   */
/*	
* These are the flag values specific to the 
* d_master/d_slave/d_complete services.
*/
#define		DMA_VERIFY	0x200		/* don't write to memory  */
#define		DMA_WRITE_ONLY	0x400		/* will not write to memory */
#define         DMA_PG_PROTECT  0x800           /* set if access violaton */
#define         DMA_CONTINUE    0x1000          /* continue mapping tags  */
						/* where previos d_slave  */
						/* finished */
/* Use with DMA_READ to prevent the page from being hidden.  Caller had
 * better be sure the buffer will not be accessed (load/store) between 
 * d_master/d_slave and d_complete.  This provides faster DMA setup.
 */
#define		DMA_NOHIDE	0x2000
/*	
* These are the flag values specific to the 
* d_map_page/d_map_list/d_map_slave services.
*/
#define		DMA_BYPASS	0x4000		/* don't perform access check*/

/*
 * The following flags are specific to the d_map_slave() service, and
 * represent the definitions for the chan_flags parameter for the d_map_slave
 * service
 *   +---------------------------------------------------------------+
 *   |mod|I|A|Reserved |E|Tim|Adr|         R e s e r v e d           |
 *   |bit|n|u|         |O|Bit|Mod|                                   |
 *   |   |c|t|         |P|   |   |                                   |
 *   | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   +---------------------------------------------------------------+
 */
#define CH_DEFAULT	   0x00000000	/* run channel with default settings*/

#define CH_DEMAND          0x00000000   /* xfer until TC or device releases */
#define CH_SINGLE          0x40000000   /* single byte or word at a time    */
#define CH_BLOCK           0x80000000   /* lock out other devices until done*/
#define CH_CASCADE         0xC0000000   /* used for cascading ISA masters   */

#define CH_ADDR_INC        0x00000000   /* increment address during DMA     */

#define CH_AUTOINIT        0x10000000   /* at TC, reset address/count to orig*/

#define CH_EOP_OUTPUT      0x00000000   /* specifies EOP signal is output   */
#define CH_EOP_INPUT       0x00400000   /* specifies EOP signal as input    */

#define CH_COMPAT          0x00000000   /* compatible timing, 8 sysclks     */
#define CH_TYPE_A          0x00100000   /* shorter cycles, 6 sysclks        */
#define CH_TYPE_B          0x00200000   /* faster i/o timing, 5 sysclks     */
#define CH_TYPE_F          0x00300000   /* high performance, 3 sysclks      */

#define CH_8BIT_BYTES      0x00000000   /* 8 bit I/O device, count by bytes */
#define CH_16BIT_WORDS     0x00040000   /* 16 bit I/O device, count by words*/
#define CH_16BIT_BYTES     0x000C0000   /* 16 bit I/O device, count by bytes*/


/***************************  DATA STRUCTURES ****************************/

/*
 * This is the d_handle structure for the d_map_* services
 */
struct d_handle {
    uint        id;                     /* identifier for this device       */
    uint        flags;                  /* device capabilities              */
    int         (*d_map_page)();        /* pointer to d_map_page routine    */
    void        (*d_unmap_page)();      /* pointer to d_unmap_page routine  */
    int         (*d_map_list)();        /* pointer to d_map_list routine    */
    void        (*d_unmap_list)();      /* pointer to d_unmap_list routine  */
    int         (*d_map_slave)();       /* pointer to d_map_slave routine   */
    int         (*d_unmap_slave)();     /* pointer to d_unmap_slave routine */
    int         (*d_map_disable)();     /* pointer to d_map_disable routine */
    int         (*d_map_enable)();      /* pointer to d_map_enable routine  */
    void        (*d_map_clear)();       /* pointer to d_map_clear routine   */
    uint        reserved1;              /* padding                          */
    uint        reserved2;              /* padding                          */
    uint        reserved3;              /* padding                          */
    uint        reserved4;              /* padding                          */
    uint        reserved5;              /* padding                          */
};
typedef struct d_handle *	d_handle_t;

/*
 * This is the dio and d_iovec structures for the d_map_* services
 */ 
struct d_iovec {
        caddr_t iov_base;       /* base memory address                  */
        int     iov_len;        /* length of transfer for this area     */
        struct xmem *xmp;       /* cross memory pointer for this address*/
};
typedef struct d_iovec * d_iovec_t;

struct dio {
        int     total_iovecs;           /* total available iovec entries    */
        int     used_iovecs;            /* number of used iovecs            */
        int     bytes_done;             /* count of bytes processed         */
        int	resid_iov;              /* number of iovec that couldn't be */
					/* fully mapped (for NORES, DIOFULL)*/
        d_iovec_t	dvec;           /* pointer to list of d_iovecs      */
};
typedef struct dio *	dio_t;

extern struct xmem *xmem_global;	/* exported GLOBAL xmem pointer     */

/****************************  MACROS ************************************/
/*
 * The following macros are specific to the d_map_* services 
 * (non-microchannel).  
 */

/*
 * Initialize a DIO structure
 *		DIO_INIT(struct dio *d,		- pointer to dio struct 
 *			 int n_iovecs);         - number of iovecs 
 *		(Must only be used from process level)
 */
#define DIO_INIT(d, n_iovecs)	\
	{	((struct dio *)(d))->dvec = (struct d_iovec *)xmalloc( \
			(sizeof(struct d_iovec) * (n_iovecs)), 2, pinned_heap);\
		if (((struct dio *)(d))->dvec == NULL)			\
			((struct dio *)(d))->total_iovecs = 0; 		\
		else							\
			((struct dio *)(d))->total_iovecs = n_iovecs; 	\
		((struct dio *)(d))->used_iovecs = 0;			\
		((struct dio *)(d))->bytes_done = 0;   			\
		((struct dio *)(d))->resid_iov = 0;   			\
	}

/*
 * Free elements of a DIO structure
 *		DIO_FREE(struct dio *d)		- pointer to dio struct 
 *		(Must only be used from process level)
 */
#define DIO_FREE(d)	\
	xmfree(((struct dio *)(d))->dvec, pinned_heap)


/*
 * Invoke the d_map_init service 
 */
#define D_MAP_INIT(bid, flags, bus_flags, channel)	\
                d_map_init(bid, flags, bus_flags, channel)

/*
 * Invoke the d_map_clear service for a specific bus type 
 */
#define D_MAP_CLEAR(handle)					\
                (handle->d_map_clear)(handle)

/*
 * Invoke the d_map_page service for a specific bus type 
 */
#define D_MAP_PAGE(handle, flags, baddr, busaddr, xmp)		\
                (handle->d_map_page)(handle, flags, baddr, busaddr, xmp)

/*
 * Invoke the d_unmap_page service for a specific bus type 
 */
#define D_UNMAP_PAGE(handle, bus_addr)				\
                if (handle->d_unmap_page != NULL)		\
                        (handle->d_unmap_page)(handle, bus_addr)

/*
 * Invoke the d_map_list service for a specific bus type 
 */
#define D_MAP_LIST(handle, flags, minxfer, virt_list, bus_list)		\
           (handle->d_map_list)(handle, flags, minxfer, virt_list, bus_list)

/*
 * Invoke the d_unmap_list service for a specific bus type 
 */
#define D_UNMAP_LIST(handle, bus_list)				\
                if (handle->d_unmap_list != NULL)		\
                        (handle->d_unmap_list)(handle, bus_list)

/*
 * Invoke the d_map_slave service for a specific bus type 
 */
#define D_MAP_SLAVE(handle, flags, minxfer, vlist, chan_flags)		\
           (handle->d_map_slave)(handle, flags, minxfer, vlist, chan_flags)

/*
 * Invoke the d_unmap_slave service for a specific bus type 
 */
#define D_UNMAP_SLAVE(handle)					\
                (handle->d_unmap_slave != NULL) ?		\
                        (handle->d_unmap_slave)(handle) : DMA_SUCC

/*
 * Invoke the d_map_disable service for a specific bus type 
 */
#define D_MAP_DISABLE(handle)					\
                (handle->d_map_disable)(handle)

/*
 * Invoke the d_map_enable service for a specific bus type 
 */
#define D_MAP_ENABLE(handle)					\
                (handle->d_map_enable)(handle)


/****************************  RETURN CODES ******************************/
/*
* These are common between the d_* and d_map_* services
*/
#define         DMA_FAIL        -1              /* error		*/
#define         DMA_SUCC        0               /* succesful completion */
/*
 * These are specific to the d_* services
 */
#define         DMA_CONFLICT    1               /* inconsistent flag val*/
#define         DMA_LIMIT       3               /* limit check		*/
#define         DMA_PAGE_FAULT  4               /* page fault		*/
#define         DMA_BAD_ADDR    5               /* invalid bus address	*/
#define         DMA_AUTHORITY   6               /* protection exception	*/
#define         DMA_INVALID     7               /* invalid operation    */
#define         DMA_EXTRA     	8               /* extra slave request  */
/*
*	All errors that have a return value greater than
*	DMA_RETRY are re-tryable.
*/
#define         DMA_RETRY       9
#define         DMA_DATA        10              /* data parity error	*/
#define         DMA_ADDRESS     11              /* address parity error */
#define         DMA_SYSTEM      12              /* system error		*/
#define         DMA_CHECK       13              /* channel check	*/
#define         DMA_MULTIPLE    14              /* multiple DMA errors  */
#define         DMA_NO_RESPONSE 15              /* no response		*/

/*
 * The following are specific to the d_map_* services
 */
#define         DMA_NOACC       	-2      /* page access violation   */
#define         DMA_NORES       	-3      /* resources exhausted     */
#define         DMA_DIOFULL     	-4      /* dio structure exhausted */
#define         DMA_TC_NOTREACHED       -5      /* terminal count ! reached*/
#define         DMA_BAD_MODE  		-6      /* unsupported mode setting*/
 

/*************************  FUNCTION PROTOTYPES ******************************/
#ifndef _NO_PROTO

/*
 * Initialize a session with the DMA mapping services (non-microchannel)
 */
d_handle_t d_map_init(int bid, int flags, int bus_flags, uint channel);	
/* arguments:
 *	int bid;			bus type/number identifier
 *	int flags;			device capabilities
 *	int bus_flags;			flags specific to the target bus
 *	uint channel;			channel assignment specific to dev/bus
 */


void d_mask(int channel);		/* disable a DMA channel	*/
/* arguments:
 *	int channel;			DMA channel to be disabled
 */

void d_unmask(int channel);		/* enable a DMA channel 	*/
/* arguments:
 *	int channel;			DMA channel to be enabled
 */

/* 
 *  allocate a DMA channel
 */ 
int d_init(int channel, int flags, size_t bid);
/* arguments:
 *      int channel;                    DMA channel to be allocated
 *      int flags;                      describe channel's use
 *	ulong bid;			BUSACC parameter
 * returns:
 *      int rc;                         success/failure
 */

void d_clear(int channel);              /* deallocate a DMA channel     */
/* arguments:
 *      int channel;                    DMA channel to be deallocated
 */

/*
 * initialize for a DMA block mode master transfer.
 */

void d_master(int channel_id, int flags, char *baddr,
	      size_t count, struct xmem *dp, char *daddr); 
/* arguments:
 *      int channel;                    DMA channel number
 *      int flags;                      flags that control the DMA transfer
 *      char *baddr;                    address of the memory buffer
 *      int count;                      transfer length in bytes
 *      struct xmem *dp;                addr of the cross memory descriptor
 *      char *daddr;                    addr used to program the DMA master
 */

/*
 * init block mode DMA transfer for a DMA slave.
 */

void d_slave(int channel_id, int flags, char *baddr,
	     size_t count, struct xmem *dp);
/* arguments:
 *      int channel;                    DMA channel number
 *      int flags;                      flags that control the DMA transfer
 *      char *baddr;                    address of the memory buffer
 *      int count;                      transfer length in bytes
 *      struct xmem *dp;                addr of the cross memory descriptor
 */

/*
 * clean up after a DMA transfer
 */
int d_complete(int channel_id, int flags, char *baddr, 
	       size_t count, struct xmem *dp, char *daddr);
/* arguments:
 *      int channel;                    DMA channel number
 *      int flags;                      describe the DMA transfer
 *      int count;                      transfer length in bytes
 *      char *daddr;                    addr used to program the DMA master
 *                                      (NULL specified for DMA slaves)
 * returns:
 *      int rc;                         DMA transfer errors (if any)
 */

int d_move(int channel_id, int flags, char *baddr,
	   size_t count, struct xmem *dp, char *daddr);
/* arguments:
 *	register int    channel_id;     channel to use
 *	register int    flags;          control flags
 *	register char   *baddr;         buffer address
 *	register int    count;          length of transfer
 *	struct xmem     *dp;            cross mem descrip
 *	register char   *daddr;         bus address
 */

#else 

d_handle_t d_map_init();		/* Initialize session (non-MCA) */
void d_mask();				/* disable a DMA channel	*/
void d_unmask();			/* enable a DMA channel 	*/
int d_init();                           /* allocate a DMA channel       */
void d_clear();                         /* deallocate a DMA channel     */
void d_master();                        /* init block mode DMA transfer */
void d_slave();                         /* init block mode DMA transfer */
int d_complete();                       /* clean up after a DMA transfer*/
int d_move();

#endif /* not _NO_PROTO */

#endif /* _H_DMA	*/
