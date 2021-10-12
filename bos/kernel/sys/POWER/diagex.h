/* @(#)02       1.6  src/bos/kernel/sys/POWER/diagex.h, sysxdiag, bos41J, 9514A_all 3/31/95 09:39:39 */
/*
 *
 * COMPONENT_NAME: (DIAGEX) Diagnostic Kernel Extension
 *
 * FUNCTIONS: Diagnostic Kernel Extension Interface definition
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/intr.h>
#include <sys/dma.h>
#include <sys/timer.h>
#include <sys/watchdog.h>

/*********************************************************************/
/* New Macro Constants Return Codes				     */
/*********************************************************************/
#define DGX_OK                  0      /* Successful Operation 			*/
#define DGX_INVALID_HANDLE  0x80000000 /* Invalid Handle Supplied 		*/
#define DGX_DEVBUSY_FAIL    0x80000001 /* Resource Already Reserved 		*/
#define DGX_DINIT_FAIL      0x80000002 /* Bad DMA Channel/Unable to Init Chan	*/
#define DGX_DCLEAR_FAIL     0x80000003 /* Unable to Free DMA Chan		*/
#define DGX_IINIT_FAIL      0x80000004 /* Unable Initialized Interrupt Chan 	*/
#define DGX_ICLEAR_FAIL     0x80000005 /* Unable to Free Interrupt Chan		*/
#define DGX_COPYDDS_FAIL    0x80000006 /* Unable to copy DDS to Handle 		*/
#define DGX_COPY_FAIL       0x80000007 /* Failcopy to/from buffer/kernal addr	*/
#define DGX_KMOD_FAIL       0x80000008 /* Unable Find Function Entry Point 	*/
#define DGX_PIN_FAIL        0x80000009 /* Unable Pin Kernel Space Memory 	*/
#define DGX_PINU_FAIL       0x8000000A /* Unable Pin User Space Memory 		*/
#define DGX_PINCODE_FAIL    0x8000000B /* Unable Pin User Space Memory 		*/
#define DGX_UNPIN_FAIL      0x8000000C /* Unable Unpin User Space Memory 	*/
#define DGX_UNPINU_FAIL     0x8000000D /* Unable Unpin User Space Memory 	*/
#define DGX_UNPINCODE_FAIL  0x8000000E /* Unable Unpin User Space Memory 	*/
#define DGX_XMALLOC_FAIL    0x8000000F /* Unable XMemAttach UserSpace Memory 	*/
#define DGX_XMFREE_FAIL     0x80000010 /* Unable Free Memory 			*/
#define DGX_XMATTACH_FAIL   0x80000011 /* Unable XMemAttach UserSpace Memory 	*/
#define DGX_XMDETACH_FAIL   0x80000012 /* Unable XMemDetach UserSpace Memory 	*/
#define DGX_BOUND_FAIL      0x80000013 /* Invalid Tranfer Size Received 	*/
#define DGX_DCOMPLETE_FAIL  0x80000014 /* d_complete Failed 			*/
#define DGX_OUTSTANDINGDMA_FAIL 0x80000015  /* Closure Fail: DMA XFER in progress */
#define DGX_NO_INTRPT_FAIL  0x80000016 /* nonexistant interrupt function 	*/
#define DGX_BADVAL_FAIL     0x80000018 /* Illegal value specified 		*/
#define DGX_UKCOPY_FAIL     0x80000019 /* Unable to copy Buffer 		*/
#define DGX_ENABLE_FAIL     0x8000001A /* Unable to copy Buffer 		*/
#define DGX_DISABLE_FAIL    0x8000001B /* Unable to copy Buffer 		*/
#define DGX_UMAP_SLAVE_FAIL 0x8000001C /* D_UNMAP_SLAVE failed */
#define DGX_MAP_SLAVE_FAIL 0x8000001D /* D_MAP_SLAVE failed */
#define DGX_RMFREE_FAIL 0x8000001E /* rmfree failed */
#define DGX_RMALLOC_FAIL 0x8000001F /* rmalloc failed */
#define DGX_EVENT_SIG 	0x80000020 /* e_sleep event termintated with a signal */
#define DGX_FAIL            0xFFFFFFFF /* diagex function failed 		*/

#define DGX_LISTFAIL        0xDEADCAFE /* Linked List Failure 			*/
#define DGX_INTERNAL_FAIL   0xDEADBABA /* Diagex Internal Program Error 	*/

/*********************************************************************/
/* PIO Constants						     */
/*********************************************************************/
#define IOCHAR               1  /* PIO TYPEs (size values) 			*/
#define IOSHORT              2
#define IOLONG               4

#define PROCLEV              1  /* PIO called from User Process Level 		*/
#define INTRKMEM             2  /* PIO called from IntrptLev,Buf inKmem		*/
#define INTRPMEM             3  /* PIO called from IntrptLev,Buf XMAttc		*/

#define DGX_SING_LOC_ACC   123  /*R/W from 1 location to 1 loc, count times 	*/
#define DGX_SING_LOC_HW    124  /*buffer gets R/W count times,HW gets W/R once	*/
#define DGX_SING_LOC_BUF   125  /*buffer gets R/W once,HW gets W/R count times	*/
#define DGX_MULT_LOC_ACC   126  /*R/W from count locs to count locs,once each	*/

/************************************************************************/
/* DIAGEX Handle Structures						*/
/************************************************************************/
/*----------------------------------------------------------------------*/
/*  DGX_DDS 								*/
/*     This structure MUST be filled in by the Calling Application	*/
/*     This structure is passed to diagex in the diag_open() routine	*/
/*     Diagex copies this structure into the dds portion of it's handle */
/*----------------------------------------------------------------------*/
typedef struct {
   char        device_name[16]; /* device logical name */
   char        parent_name[16]; /* parent logical name */
   uint        slot_num;       /* slot number of adapter */
   uint        bus_intr_lvl;   /* interrupt level */
   uint        intr_priority;  /* interrupt priority */
   uint        intr_flags;     /* look at intr.h */
   uint        dma_lvl;        /* this is the bus arbitration level */
   uint        dma_chan_id;    /* dma channel ID set by diag_open() */
   ulong       bus_io_addr;    /* base of Bus I/O area for this */
   ulong       bus_io_length;
   ulong       bus_mem_addr;   /* base of Bus Memory "Shared" */
   ulong       bus_mem_length;
   ulong       dma_bus_mem;   /* base of Bus Memory DMA (ignored for BUS_60X) */
   ulong       dma_bus_length;/*  total dma area at dma_bus_mem (bytes)*/
   int         dma_flags;      /* used for d_init() */
   ulong       bus_id;         /* bus ID given to the device */
   ulong       bus_type;       /* BUS_MICRO_CHANNEL or BUS_60X or BUS_BID*/
   int         kmid;           /* kmid of interrupt handler */
   uchar       *data_ptr;      /* pointer for passing data to interrupt*/
   int         d_count;        /* count of bytes of data for interrupt*/
   int         maxmaster;      /* maxium number of concurrent diag_dma_masters*/
} diagex_dds_t;

/*------------------------------------------------------------------------------*/
/*  DMA_STRUCTURE : dma_info_t							*/
/*     This  structure is part of the diag_struc_t handle's dma_info_t array	*/
/*     For dds.bus_type == BUS_MICRO_CHANNEL,					*/
/*        There is dds.maxmaster+1 array elements of type dma_info_t		*/
/*        (the +1 hold information for the single slave dma)			*/
/*        firsttcw and last_tcw refer to the TCWs currently 'in_use'		*/
/*     For dds.bus_type == BUS_60X						*/
/*        There are dds.maxmaster+1 array elements of each type dma_free_t and	*/
/*        dma_info_t (the +1 hold information for the single slave dma)		*/
/*     This structure maintains active dma master/slave information.		*/
/*------------------------------------------------------------------------------*/

typedef struct dmast {
   struct dmast *next;
   int         firsttcw;     /* first TCW used (micro channel only) */
   int         last_tcw;     /* last  TCW used (micro channel only) */
   int         dma_flags;    /* see /usr/include/sys/dma.h */
   uchar       *baddr;       /* address of the host buffer to DMA to/from */
   uchar       *daddr;       /* Phys addr in DMAbus_mem-from diag_dma_master()*/
   uint        count;        /* size of the DMA data in bytes */
   struct      xmem dp;      /* Cross Memory descriptor of baddr */
   char        pinned;       /* NonZero if DMA buffer was pinned */
   char        xmattached;   /* NonZero if DMA buffer was CrossMemAttached */
   char        in_use;       /* TRUE if this linked list member is valid */
} dma_info_t;


/*----------------------------------------------------------------------*/
/*  AIOO_FLAGS								*/
/*     This structure specifies a set of status flags for 		*/
/*     allocations, initializations, and outstanding operations		*/
/*     in each active handle						*/
/*----------------------------------------------------------------------*/
typedef struct {
   char AllocIntrptDataMem;  /* NonZero if Alloc'd Intrpt Data Mem */
   char AllocDmaAreaMem;     /* NonZero if  Alloc'd DMA buffer Mem */
   char CopyDDS;             /* NonZero if  Copied DDS to handle   */
   char SetIntrptEntPt;      /* NonZero if  Intr Function in KMem  */
   char PinIntrptFunct;      /* NonZero if  Pinned Intrpt Function */
   char PinUIntrptData;      /* NonZero if  Pinned User Int DataMem*/
   char PinDiagExt;          /* NonZero if  Pinned Diagex Extension*/
   char InitIntrptChan;      /* NonZero if  Initialized InterptChan*/
   char InitDmaChan;         /* NonZero if  Initialized DMA Channel*/
   char XmatUIntrptData;     /* NonZero if  CrossMemAttached IntDat*/
} aioo_flags_t;

typedef struct dmadio{
	dio_t vlist;  /* NonZero if vlist */
	dio_t blist; /* NonZero if vlist */
	int dioinit; /* NonZero if DIO_INIT successful */
	int chan_flags; /* device/bus specific flags for transfer */
}dma_dio;

/*----------------------------------------------------------------------*/
/*  DIAG_STRUC								*/
/*     This structure is the handle for diagex.				*/
/*----------------------------------------------------------------------*/
typedef struct handl {
   struct        intr intr;             /* see intr.h  - Needs to be first */
   struct handl  *next;                 /* points to nxt handle in linked list*/
   int           (*intr_func)();        /* pointer to interrupt handler */
   uchar         *intr_data;            /* Pointer to interrpt handler buffer */
   struct        xmem udata_dp;         /* intrpt data area cross mem descript*/
   diagex_dds_t  dds;                   /* copy of Application's DDS */
   struct        timestruc_t itime;     /* updated at interrupt or PIO        */
   struct        timestruc_t ntime;     /* updated at interrupt or PIO        */
   dma_info_t    *dma_info;             /* pointer to an array of structure */
   aioo_flags_t  aioo;                  /* allocated/init/outstandingops flags*/
   char          *scratch_pad;          /* PIO scratch pad for large transfers*/
   /* the following should only be used by the interrupt handler           */
   /* when the interrupt handler is supporting the diag_watch4intr() call   */
   /* (sleep_flag and sleep word should NOT be modified by the intr hdnlr) */
   /* (flag_word should be modified by the interrupt handler)              */
   uint          sleep_flag;    /* TRUE when diag_watch4intr is waiting 4 intr*/
   uint          sleep_word;    /* use in e_wakeup() to wake diag_watch4intr */
   uint          flag_word;     /* application defined interrupt flag */
   struct watchdog wdt;		/* watch dog timer for watch4intr() timeout */
   struct d_handle * dhandle;   /* Used for DMA for non-MCA devices */
   dma_dio  * dio_st;	/* ptr. to DIO struct used in DMA ops. */ 
} diag_struc_t;

/*******************************************************************/
/* Function Prototypes						   */
/*******************************************************************/
/*-------------diagex_load.c---------------------------------------*/
int diag_open (diagex_dds_t *dds_ptr, diag_struc_t **usershandle);
int diag_close(diag_struc_t *handle );
int diag_dma_master
  (diag_struc_t *handle,int dma_flags,caddr_t baddr,int count,int *users_daddr);
int diag_dma_slave(diag_struc_t *handle,int dma_flags,caddr_t baddr,int count);
int diag_read_handle(diag_struc_t *handle,diag_struc_t *hdlr);

/*-------------diagex_pin.c---------------------------------------*/
int diag_dma_complete(diag_struc_t *handle, int daddr);
int diag_intr_enable(diag_struc_t *handle, int prev_priority);
int diag_intr_disable
  (diag_struc_t *handle, int new_priority, int *prev_priority);
int diag_dma_flush(diag_struc_t *handle, int daddr);
int diag_dma_unmask(diag_struc_t *handle);
int diag_dma_mask(diag_struc_t *handle);
int diag_watch4intr(diag_struc_t *handle, int flag_mask, int timeout_sec);
void diag_watchdog(struct watchdog *wdt);
int diag_io_write(diag_struc_t *handle,int type,int offset,
                    ulong dataval, struct timestruc_t *times, int intlev);
int diag_io_read(diag_struc_t *handle,int type,int offset,
                    void *data, struct timestruc_t *times, int intlev);
int diag_mem_write(diag_struc_t *handle,int type,int offset,
                    ulong dataval, struct timestruc_t *times, int intlev);
int diag_mem_read(diag_struc_t *handle,int type,int offset,
                    void *data, struct timestruc_t *times, int intlev);
int diag_pos_write(diag_struc_t *handle,int offset,
                    char dataval, struct timestruc_t *times, int intlev);
int diag_pos_read(diag_struc_t *handle,int offset,
                  char *data, struct timestruc_t *times, int intlev);
int diag_io_wr_stream(diag_struc_t *handle,int type,int offset,
                   int count, int range_flag, char *data,
                   struct timestruc_t *times, int intlev);
int diag_io_rd_stream(diag_struc_t *handle,int type,int offset,
                   int count, int range_flag, char *data,
                   struct timestruc_t *times, int intlev);
int diag_mem_wr_stream(diag_struc_t *handle,int type,int offset,
                   int count, int range_flag, char *data,
                   struct timestruc_t *times, int intlev);
int diag_mem_rd_stream(diag_struc_t *handle,int type,int offset,
                   int count, int range_flag, char *data,
                   struct timestruc_t *times, int intlev);
int diag_trace(ulong hook, char *tag, ulong arg1, ulong arg2, ulong arg3);

int diag_special_io_write(diag_struc_t *handle,int type,int offset,
             ulong dataval, struct timestruc_t *times, int intlev);

int diag_special_io_read(diag_struc_t *handle,int type,int offset,
             void *data, struct timestruc_t *times, int intlev);

/* the following macro is provided to allow backward compatability */
#define wait4intr(a,b)	watch4intr((a),(b),0);
