/* @(#)31	1.15  src/bos/kernel/sys/POWER/io3270.h, sysxc327, bos411, 9428A410j 4/3/91 16:45:54 */
#ifndef	_H_IO3270
#define	_H_IO3270

/*
 * COMPONENT_NAME: (SYSXC327) c327 tca ioctl definitions
 *
 * FUNCTIONS:    N/A
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
** Common C327 device driver interface
*/

#define DEVICE_BASE_NAME "/dev/3270c"

/*
** CUT mode emulator interface
*/

#define EMMISC   ('j'<<8)      /* ORed with various values for other ioctl's */
#define EMWAIT  (EMMISC | 1)   /* Do wait for BMC or LIA interrupts (default)*/
#define EMNWAIT (EMMISC | 2)   /* Do not wait for BMC or LIA interrupts */
#define EMXPOR  (EMMISC | 3)   /* execute power-on reset */
#define EMIMASK (EMMISC | 4)   /* Mask interrupts (ignore to cut overhead) */
#define EMCPOS  (EMMISC | 5)   /* get cursor position */
#define EMVISND (EMMISC | 6)   /* Get most recent vis/snd reg value */
#define EMSEEK  (EMMISC | 7)   /* Special seek for CUT file transfer */
#define EMKEY    ('k'<<8)      /* send key scancode by ORing with EMKEY */

/*
** diagnostic interface
*/

#define C327M                 ('m'<<8)
#define C327DIAG_TCA_SIZE     (C327M |  1) /* get size of ram */
#define C327DIAG_TCA_READ     (C327M |  2) /* ram read */
#define C327DIAG_TCA_TEST     (C327M |  3) /* ram test */
#define C327DIAG_REG_READ     (C327M |  5) /* register read */
#define C327DIAG_REG_TEST     (C327M |  6) /* register test */
#define C327DIAG_SIM_RD_CMD   (C327M |  7) /* simulate a read command */
#define C327DIAG_SIM_WR_CMD   (C327M |  8) /* simulate a write command */
#define C327DIAG_GET_RCV_CMD  (C327M |  9) /* read the receive or cmd reg */
#define C327DIAG_I_STAT_RESET (C327M | 10) /* reset saved interrupt stat reg */
#define C327DIAG_I_STAT_READ  (C327M | 11) /* read saved interrupt stat reg */
#define C327DIAG_SIM_RESET    (C327M | 12) /* simulate a power-on reset */
#define C327DIAG_DISCONNECT   (C327M | 13) /* reset + record tim of discon */
#define C327DIAG_CONN_TEST    (C327M | 14) /* connection test */

/* data structure passed with diagnostic ioctl */
typedef struct {
   long          address;      /* register number 0-F or TCA offset 0-FFFF */
   unsigned char send_byte;    /* char to output or write */
   unsigned char recv_byte;    /* char input or read */
} C327DIAG_DATA;

/*
** adapter I/O registers
*/
#define adapter_intr_stat_reg      0x0 /* 0 interrupt status */
#define adapter_vis_snd_reg        0x1 /* 1 visual sound */
#define adapter_lsb_cur_reg        0x2 /* 2 cursor addr lsh */
#define adapter_msb_cur_reg        0x3 /* 3 cursor addr msh */
#define adapter_conn_ctrl_reg      0x4 /* 4 3270 connection control */
#define adapter_scan_code_reg      0x5 /* 5 scan code */
#define adapter_term_id_reg        0x6 /* 6 terminal id */
#define adapter_old_segment_reg    0x7 /* 7 reserved (old segment) */
#define adapter_87e_stat_reg       0xA /* A 87E status */
#define adapter_io_ctrl_reg        0xF /* F register bank selection */

/*
** adapter I/O registers - literals
*/
#define CONN_CTRL_ENABLE_COAX      0x01 /* enable coax */
#define CONN_CTRL_DFT_MODE         0x02 /* dft mode */
#define CONN_CTRL_87E_MODE         0x04 /* 3287 mode */
#define CONN_CTRL_KEY_AVAIL        0x08 /* keystroke available */
#define CONN_CTRL_POLL_REQ         0x10 /* poll request */
#define CONN_CTRL_TEST             0x20 /* test */
#define CONN_CTRL_RST_CUR          0x40 /* reset cursor (87E mode) */
#define CONN_CTRL_CON_INT_INH      0x40 /* conditional interrupt inhibit */
#define CONN_CTRL_INT_INH          0x80 /* interrupt inhibit */

/*
** non-SNA DFT mode emulator interface
*/

/*
** definitions for ioctl
*/
#define WDC		('w' << 8)
#define WDC_INQ		(WDC | 1)	       /* inquire io status */
#define WDC_SSTAT	(WDC | 2)	       /* send status */
#define WDC_POR		(WDC | 3)	       /* power on reset */
#define WDC_AUTO	(WDC | 4)	       /* enable/disable auto ack for API */
#define WDC_WAITCLEAR	(WDC | 5)	   /* wait for broadcast clear */

/*
** link address information structure
*/
struct nsddinfo {
	int	dev_addr;		               /* link_address */
	int	length;			               /* length of buffer for DDS */
	char	*ddsptr;		           /* pointer to DDS buffer */
	char	type;			           /* device type */
};

typedef struct nsddinfo	tcadinfo;

typedef struct iocinfo {
	int	chan;			     /* logical terminal number */
	int	controller;		     /* ebcdic rep. of controller type */
	int	attach_protocol;	 /* controller attachment protocol */
} iocinfo;

/*
** io3270/iotca status
*/
struct io3270 {
	unsigned int 	io_flags;	/* information flags */
	unsigned int	io_status;	/* 5xx, 6xx, 7xx msg codes */
    unsigned int    io_extra;   /* extra information field */
};

typedef struct io3270	iotca;

/*
** defines for use with io_flags
*/
#define WDI_DAVAIL	0x02  		/* data available */
#define WDI_CCHAIN	0x04		/* data chaining */
#define WDI_LOCKED	0x08		/* device locked */
#define WDI_FATAL	0x10		/* fatal err. on card */
#define WDI_COMM   	0x20		/* communication check */
#define WDI_PROG   	0x40 		/* program check */
#define WDI_MACH   	0x80 		/* machine check */
#define WDI_CU     	0x100 		/* ACTLU/DACTLU received */
#define WDI_WCUS_30	0x200 		/* WCUS 30 Received */
#define WDI_WCUS_31 0x400 	  	/* WCUS 31 received */
#define WDI_RUSIZE  0x800       /* transmit max. RU size allowed. */
#define WDI_ALL_CHECK	(WDI_FATAL | WDI_COMM | WDI_PROG | WDI_MACH | WDI_CU \
                         | WDI_WCUS_30 | WDI_WCUS_31)
#define WDI_TRANSP_MODE	0x01		/* transparency write option  */

/*
** Communication Check Status Codes
*/
#define WEC_501		501	/* 5088 is operating but is set to offline */
#define WEC_503		503	/* selective reset seq. on chan. detected */
#define WEC_505		505	/* sys. reset seq. on chan. detected by 5088 */
#define WEC_513		513	/* link address is busy */
#define	WEC_551		551	/* bus out check detected by 5088 */
#define WEC_583		583	/* halt I/O sequence detected by 5088 */
#define WEC_584		584	/* streaming overrun detected by 5088 */
#define WEC_585		585	/* process has data to be read. */
				        /* driver cannot issue current command */
				        /* until data is read */

/*
** Machine Check Status Codes
*/
#define	WEM_TIMEOUT	    670	/* hardware timed out */
#define WEM_BAD_INIT    671	/* device initialization error */
#define WEM_HARDWARE    674	/* hardware error */
#define	WEP_BAD_NET	    672	/* netID not in table */
#define	WEP_CMD_OUT	    673	/* command already outstanding */
#define	WEP_INV_Q	    675	/* invalid queue element type */
#define	WEP_INV_CMD	    676	/* invalid command */
#define	WEP_LA_ENB	    677	/* link address already enabled	*/
#define	WEP_LA_AENB	    678	/* link address not enabled */
#define	WEP_UNDER_SLIH	679	/* SLIH ring que underflow */
#define	WEP_BUFF_DEPL	680	/* buffer pool depleted */
#define	WEP_OVER_DEV	681	/* device ring que overflow */
#define	WEP_SSLA_BAD	682	/* SSLA rejected commands */
#define	WEP_SEL_RESET	683	/* selective reset issued */
#define	WEP_POWER_ON_RST  684   /* power on reset in progress */
#define	WEP_5088_CMD_REJECT 685 /* command reject generated by 5088 for */
                                /* data received on channel */
#define	WEP_5088_OP_CHK	686	/* 5088 detected op check */
#define	WEP_HIA_DOWN	687	/* link is down */
#define	WEP_DUP_NET	    688	/* duplicate netID in table */
#define	WEP_INV_PARMS	689	/* invalid parameters */
#define	WEP_FULL_NET	690	/* netID table full */
#define WEP_CARD_DOWN	WEP_HIA_DOWN	/* link is down */

/*
** Broadcast w/no COMM, PROG or MACH check
*/
#define WEB_610     610 /* Error code for FATAL ERROR. Coax is disconnected */
#define WEB_603		610	/* Same as WEB_610, but defined to remain compatable */
                        /* with earlier implementation.                      */

/*
** Status Codes
*/
#define	STAT_ACK	0	/* zero status */
#define STAT_READ	1	/* read successful, write will follow */
#define STAT_SEL	107	/* device not selected */
#define	STAT_BERR	109	/* buffer error */
#define	STAT_BADC	110	/* bad command */
#define	STAT_UNSUP	111	/* unsupported command */
#define STAT_RESET	2	/* reset command */
#define	STAT_REST	STAT_RESET	/* reset command */
#define	STAT_PRTCMP	3	/* print complete (reset command) */

/*
** Acknowlegement Codes
*/
#define	ACK_ON		0x01	/* turn on auto acknowledgement */
#define	ACK_OFF		0x00	/* turn auto acknowledgement off */

/*
** Write Options
*/
#define TRANSP_MODE	0x01	/* Transparent option on write, BSC mode */

/*
** DDS definition
*/
typedef struct
{
                                        /* bus information */
      unsigned long  bus_id;            /* bus ID */
      unsigned short bus_type;          /* type of bus */
                                        /* adapter information */
      int    slot_number;               /* slot number for this card */
      long   bus_mem_beg;               /* begin of adapter bus memory */
      long   bus_mem_size;              /* size of adapter bus memory */
      int    io_port;                   /* port base address (2D0,6D0,AD0,ED0*/
      int    bus_intr_lvl;              /* interrupt level (9) */
      int    intr_priority;             /* interrupt priority */
                                        /* session information */
      short  printer_addr[8];           /* printer addresses (-1 = no ptr) */
      int    num_sessions;              /* number of sessions (1-8) */
      long   buffer_size;               /* size of data buffers */
                                        /* machine information */
      char   machine_type_number[4];    /* machine type number */
      char   customer_id;               /* customer id */
      char   model_number[3];           /* model number */
      char   plant_manufactured[2];     /* plant manufactured */
      char   serial_no[7];              /* serial number */
      char   software_release_level[3]; /* software release level */
      char   ec_level[16];              /* eng change (EC) level */
      char   dev_name[16];              /* device name */
   } c327_dds;

/******************************************************/
/** MSLA microcode address information structure */
typedef volatile struct Micro_Info {
   int   type;
   int size;
   char *start;
} Micro_Info;



/****************************************************************************/
/*       Define Device Structure               */
/* this info comes from the device configuration routines for msla        */
/****************************************************************************/
typedef struct msla_dds
{
   ulong bus_id;        /* bus id */
   ushort   bus_type;      /* bus type usually BUS_MICRO_CHANNEL*/

   char  dev_name[16];     /* adapter name, msla */
   ulong slot_number;      /* slot adapter is in */
   ulong bus_mem_beg;      /* begin of adapter bus memory */
   int   bus_mem_size;     /* size of adapter memory */
   ulong io_port;    /* begin of adapter registers */
   ulong bus_intr_level;      /* interrupt level */
   ulong bus_intr_pri;     /* interrupt priority class */
   ulong dma_arb_level;    /* DMA arbitration level */

   short num_of_sessions;           /* number of session addresses */
   short int   transfer_buff_size;  /* transfer data buffers */
   short lower_la;                  /* lower bound session address (x 0)*/
   short upper_la;                  /* upper bound session address (x F)*/
   int   link_speed ;               /* link speed */
   short num_5080_sess;             /* number of session addresses */
   short lower_5080_la;             /* lower bound session address (x 0)*/
   short upper_5080_la;             /* upper bound session address (x F)*/
   short addr_5080_chan;            /* channel address for 5080 */
   Micro_Info *micro_info;          /* pointer to microcode */
   int      dma_base;               /* DMA bus memory base */
}Msla_Ddi;


#endif	/* _H_IO3270 */
