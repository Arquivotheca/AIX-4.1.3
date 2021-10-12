/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/
#ifndef ADAPTERS
#define ADAPTERS

#define ADAPTER_COMMAND COMMAND
#define ADAPTER_RESPONSE char*
typedef unsigned long int ADAPTER_HANDLE;

#define ADAPTER_TIMEOUT_ERROR -1

/********************/
/*   Header Files   */
/********************/
#include <sys/xmem.h>
#include <stddef.h>
#include <sys/mbuf.h>

#include <sys/comio.h>
#include <sys/ciouser.h>

/********************************
 *     Constant Definitions
 ********************************/

/* these defines effect the DDS size                                        */
#define RAM_SIZE         (0x4000)  /* Adapter RAM size - 16 K               */
#define MAX_TX_LIST      (2)       /* Maximum Transmit frame list           */
#define MAX_RX_LIST      (2)       /* Maximum Receive  buffer list          */
#define RX_LIST_SIZE     (16)      /* size of receive elem                  */
#define TX_LIST_SIZE     (20)      /* size of transmit elem                 */

/**********************************
 *  Watchdog Timer Definitions
 *********************************/
typedef enum {                   /* Who set the watchdog timer            */
   WDT_INACTIVE,                 /* WDT is inactive                       */
   WDT_INIT = 0xD00,             /* Initialization is progress            */
   WDT_OPEN,                     /* Open in progress                      */
   WDT_XMIT,                     /* xmit in progress                      */
   WDT_COMMAND,                  /* command in progress                   */
   WDT_RECV,                     /* recv in progress                      */
   WDT_CLOSE                     /* Adapter close in progress             */
} WDT_SETTER;

typedef struct {
     struct watchdog  wd_timer;       /* watchdog timer structure            */
     ulong            bus_id;         /* Bus id                              */
     ulong            bus_io_addr;    /* Bus io address                      */
     int              int_level;      /* Interrupt level for INTTIMER        */
 } WDT_STRUCT;

/*****************************************************************************/
/*                  Transmit List type definition                            */
/*****************************************************************************/
 typedef struct TX_LIST {
  ulong       fw_pointer;       /* transmit forward pointer          */
  ulong       xmit_status;      /* transmit status                   */
  ushort      frame_len;        /* frame length                      */
  ushort      buf_count;        /* data buffer length                */
  ulong       data_pointer;     /* data buffer pointer               */
  ushort      buf_len;          /* frame length                      */
  ushort      reserved;         /* reserved                          */
} tx_list_t;

/*****************************************************************************/
/*                  Receive List type definition                             */
/*****************************************************************************/
typedef struct RX_LIST {
  ulong       fw_pointer;         /* receive forward pointer           */
  ulong       recv_status;        /* receive status                    */
  ulong       data_pointer;       /* data buffer pointer               */
  ushort      data_len;           /* data buffer length                */
  ushort      fr_len;             /* frame length                      */
} rx_list_t;


/**************************************************************/
/*               Main Structure For Adapter                   */
/**************************************************************/

typedef struct {
     diag_struc_t  *handle;           /* handle for adapter used by diagex   */
     mps_dds       dds;               /* device structure used by diagex     */
     char          *vpd_info;         /* vendor product data for adapter     */
     uint          card_id;           /* card id for adapter                 */
     int           error_log[100];    /* contains errors on adapter          */
     int           num_errors;        /* contains number of errors           */
     int           int_type[16];      /* interrupt type                      */
     int           num_interrupts;    /* number of interrupts                */
     ulong         int_pending;       /* bit list of types of intr pending   */
     ushort        cmd_reg;           /* command from bus master interrupt   */
     ushort        source_address[3]; /* source address for token ring       */
     ushort        dest_address[3];   /* dest address for token ring         */
     char          transmit_string[4096];  /* pointer to transmit string     */
     char          receive_string[4096];   /* pointer to receive string      */
     uint	   opened;		/* flag indicated open complete      */
     int	   best_orw;
     ulong	   buf_addr;

     WDT_SETTER       wd_setter;      /* watchdog timer setter               */
     int   dma_channel;               /* for use with DMA services d_xxxx    */
     uchar   channel_alocd;           /* State of DMA_CHANNEL                */

     /* Address of LAP registers                                             */
     ushort  asb_address;                 /* address of ARB                  */
     ushort  srb_address;                 /* address of SRB                  */
     ushort  arb_address;                 /* address of ARB                  */
     ushort  trb_address;                 /* address of TRB                  */

     /* for transmit                                                         */
     uchar      *xmit_usr_buf[MAX_TX_LIST]; /* returned from malloc         */
     uchar      *xmit_buf[MAX_TX_LIST];   /* pinned buffer of transmit data  */
					  /* user buffer after page allign   */
     tx_list_t  *xmit_usr_desc[MAX_TX_LIST]; /* returned from malloc         */
     tx_list_t  *xmit_list[MAX_TX_LIST];  /* transmit memory descriptor list */
					  /* user desc after page allign   */
     ulong xmit_dma_addr[MAX_TX_LIST];	 /* dma address */
     ulong xmit_desc_addr[MAX_TX_LIST];  /* DMA address for transmit desc    */

     /* Information for receive list                                         */
     uchar      *recv_usr_buf[MAX_RX_LIST]; /* returned from malloc         */
     char       *recv_buf[MAX_RX_LIST];   /* pinned buffer for receive data */
					  /* user desc after page allign   */
     rx_list_t  *recv_usr_desc[MAX_RX_LIST]; /* returned from malloc         */
     rx_list_t  *recv_list[MAX_RX_LIST]; /* Receive buffer descriptor list  */
					  /* user desc after page allign   */
     ulong recv_dma_addr[MAX_RX_LIST];   /* dma address for receive buffer   */
     ulong recv_desc_addr[MAX_RX_LIST];  /* DMA address for receive  desc    */

} ADAPTER_STRUCT;


/*  Return Codes  */
#define XMFREE_FAIL      20
#define IINIT_FAIL       30
#define XMATTACH_FAIL    40
#define BOUND_FAIL       50
#define XMALLOC_FAIL     60
#define INTR_INIT_FAIL   70
#define KMOD_FAIL        80
#define PIN_FAIL         90
#define LENGTH_FAIL      100
#define TYPE_FAIL        110

#define OPEN_FAILED       120
#define DMA_FAILED        130
#define PINU_FAILED       140
#define PINCODE_FAILED    150
#define COPYIN_FAILED     160

#endif


