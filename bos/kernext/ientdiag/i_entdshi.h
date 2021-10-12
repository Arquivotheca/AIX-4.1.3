/* @(#)54  1.10.1.1  src/bos/kernext/ientdiag/i_entdshi.h, diagddient, bos411, 9428A410j 11/10/93 13:47:17 */
/*
 * COMPONENT_NAME: (SYSXIENT) Device Driver for the integrated Etherent controller
 *
 * FUNCTIONS:  Ethernet device dependent data structures
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define DD_NAME_STR      "ethdd" 
#define TRACE_HOOKWORD   HKWD_DD_ETHDD      /* in <sys/trchkid.h>  */
#define MAX_OPENS        ent_MAX_OPENS      /* max opens per adapter */
#define MAX_ADAPTERS     1 		    /* # of native ethernet's sup'ted */
#define MAX_CDT          0                  /* Component Dump table entry */
#define ROS_LEVEL_SIZE   4        	    /* bytes in ROS Level            */

/* these defines effect the DDS size */
#define	MAX_ADPT_NETID	 32       	    /* maximum adapter netid's */

typedef enum {  			/* Who set the watchdog timer */
	WDT_INACTIVE,			/* WDT is inactive */
	WDT_CONNECT,			/* dsact in progress */
	WDT_XMIT,			/* xmit in progress */
	WDT_ACT,			/* action cmd in progress */
	WDT_CLOSE,			/* Adapter close in progress */
	WDT_CFG,			/* configure action command */
	WDT_IAS				/* Individual Address setup act cmd */
} wdt_setter;

typedef enum { 				/* Who set the timeout timer */
	TO_RX_MBUF			/* Receive mbuf get wait in progress */
} to_setter;


#ifdef DEBUG
#define TRACE_TABLE_SIZE 	3000             /* SAVETRACE table (ulongs) */
#else
#define TRACE_TABLE_SIZE 	1000               /* SAVETRACE table (ulongs) */
#endif /* DEBUG */


/*****************************************************************************/
/* these types are used to define the dds                                    */
/*****************************************************************************/
/* State variable for first start state machine sequencing */
/* Used with state machine variable WRK.adpt_start_state */
typedef enum {
	NOT_STARTED = 0,     /* initial state after last halt */
	STARTED,             /* adapter started & acknowledged */
        STARTING	     /* while starting up in xxx-act */
} START_STATE;

/*****************************************************************************/
/*                  Work section of DDS                                      */
/*****************************************************************************/

typedef struct {
	START_STATE	adapter_state;	/* state variable for start up */
	wdt_setter	wdt_setter;     /* starter of watch dog timer */
	to_setter	to_setter;      /* starter of timeout timer */
	ulong		connection_result;   /* start done status */

	/* configuration values */
	int	timr_priority;   	/* use with tstart system timer */
	int	dma_channel;     	/* for use with DMA services d_xxxx */
	int	rdto;            	/* mbuf data offset for read data */
	struct  xmem	xbuf_xd;	/* cross-memory descriptor */

	/*  shared memory structure(SMS) pointers */
	struct	scp	*scp_ptr;	/* pointer to the SCP */
	struct	iscp	*iscp_ptr;	/* pointer to the ISCP */
	struct	scb	*scb_ptr;	/* pointer to the SCB */
	struct	xcbl	*xcbl_ptr;	/* pointer to the xmit CBL */
	struct	acbl	*acbl_ptr;	/* pointer to the action CBL */

	/* bus or ASIC RAM addresses for the SMS */
	struct	scp	*scp_addr;	/* bus address of the SCP */
	struct	iscp	*iscp_addr;	/* bus address of the ISCP */
	struct	scb	*scb_addr;	/* bus address of the SCB */
	struct	xcbl	*xcbl_addr;	/* bus address of the xmit CBL */
	struct acbl	*acbl_addr;	/* bus address of the action CBL list */
	ulong	sysmem;			/* start of allocated system memory */
	ulong	sysmem_end;		/* end of allocated system memory */
	ulong	asicram;		/* start of allocated ASIC RAM */
	ulong	asicram_end;		/* end of allocated ASIC RAM */
	ulong	dma_base;    		/* beginning of our IO address space */
	ulong	aram_mask;		/* mask for ASIC RAM addresses */
	ulong	alloc_size;		/* size of allocated system memory */

	/* miscellaneous variables */
	ulong	control_pending;	/* a control command is pending */

	/* action command queue */
	ulong	action_que_active;	/* true when scb pts to action que */
	ulong	action_cmd_queued;	/* true when element on waitq */
	ulong	act_next_in;
	ulong	act_next_out;
	ulong	actq_in;		/* next free elem in action queue */
	ulong	current_elem;		/* action q elem currently exec'g */

	/* receive frame area (RFA) variables */
	struct	rfd	*rfd_ptr;	/* pointer to the RFD array */
	struct	rfd	*rfd_addr;	/* chip address of RFD array */
	struct	rbd	*rbd_ptr;	/* pointer to the RBD array */
	struct	rbd	*rbd_addr;	/* chip address of RFD array */
	ulong	save_bf;		/* save bad rcv frames configuration */
	ulong	begin_fbl;		/* 1st free recv buffer in the list */
        ulong   end_fbl;                /* last free recv buffer in the list */
        ulong   el_ptr;                 /* index of the RFD with the EL bit */
        struct rbd   *rbd_el_ptr;       /* addrsss of RBD with the EL bit set */
	ulong	recv_buffers;		/* number of recv buffers allocated */
	struct	recv_buffer	*recv_buf_ptr;	/* pointer to xmit buffers */
	struct	recv_buffer	*recv_buf_addr;	/* bus address of the RFA */

	/* transmit command queue and buffers */
	struct	xmit_buffer	*xmit_buf_ptr;	/* pointer to xmit buffers */
	struct	xmit_buffer	*xmit_buf_addr;	/* chip address, xmit buffers */
	ulong	xmit_que_active;	/* true when scb pts to xmit que */
	ulong	readyq_out;
	ulong	readyq_in;
	ulong	waitq_in;
	ulong	waitq_out;
	ulong	xmits_queued;		/* true when element on waitq */
	ulong	xmits_pending;		/* number of xmit elements pending */
	
	/* control variables for transmit buffers */
	ulong	xmit_buffers_allocd;	/* total number of transmit buffers */
	ulong	xmit_buffers_used;	/* transmit buffers in use */
	ulong	buffer_in;		/* next free xmit buffer */

	xmt_elem_t	*xmit_elem;	/* list of xmt elements */
	struct	tbd	*tbd_ptr;	/* ptr to TBD array */
	struct	tbd	*tbd_addr;	/* chip address of TBD array */

	/* POS and Ethernet addresses.  */
	uchar ent_addr[ent_NADR_LENGTH];	/* actual net addr in use */
	uchar ent_vpd_addr[ent_NADR_LENGTH]; 	/* net address from VPD */
	uchar pos_reg[8];                   	/* POS register values */
	ulong	pos;

	/* multicast list information */
	open_elem_t *multi_open[MAX_MULTI]; 		
	uchar   multi_list[MAX_MULTI][ent_NADR_LENGTH];   /* array of addr's */
	ulong   multi_count;                      /* number of valid entries */

	/* saved info on the last netid received */
	netid_t	prev_netid;                 	/* data type last net id */
	ushort  prev_netid_length;          	/* length of last netid */
	ulong   prev_netid_indice;           	/* last index into netid tbl */

	/* for state of resources used/maintained by driver */
	ulong   channel_allocated;             	/*  DMA channel allocated */
	ulong   machine;              		/*  HW platform */
	/* The following variable is used in the intr. handler, first bit
	   is "is_rainbow" next bit is "done_ca_already". */
	ulong	do_ca_on_intr;			/*  Rainbow hack. */
	ulong   pos_request;           		/*  POS values specified */

	/* information for sync close */
	int     close_event;

	/* count of the number of pages ( TCW's ) allocated */
	ushort	xmit_tcw_cnt;           /* # of TCW's used for xmit queue */
	ushort	recv_tcw_cnt;		/* # of TCW's used for recv queue */
	ushort	restart; 		/* TRUE ==> reinitialize the adapter */

	/* Current adapter configuration. */
	struct cfg	cur_cfg;	/* Holding place for current cfg. */
} dds_wrk_t;
