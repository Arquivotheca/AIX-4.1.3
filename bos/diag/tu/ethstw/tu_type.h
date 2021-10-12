/* @(#)49       1.4  src/bos/diag/tu/ethstw/tu_type.h, tu_ethi_stw, bos411, 9428A410j 10/5/92 14:50:08 *
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw
 *
 * FUNCTIONS: Ethernet Test Unit Header File
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
 *****************************************************************************/

/*****************************************************************************
Header HTX/Mfg. Ethernet Adapter Definitions File

Module Name :  tu_type.h

Header file contains basic definitions needed by three applications for
testing the Integrated Ethernet. 

	1)  Hardware exerciser invoked by HTX,
	2)  Manufacturing application, and
	3)  Diagnostic application.

*****************************************************************************/

/****** variable type *********/
#define uchar	unsigned char
#define ulong 	unsigned long
#define ushort	unsigned short
#define uint	unsigned int

/***** PRINT macro **********/
#ifdef 	TU_DEBUG_MSG
#define PRINT(args)	fprintf args
#else
#define PRINT(args)
#endif

/******************************************************************
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 ******************************************************************/

#define INVOKED_BY_HTX   2

/******************** SYSTEM POS ADDRESS *****************************/

#define SPOS3    0x4f0003
#define SPOS6    0x4f0006
#define SPOS7    0x4f0007

/******************** POS ADDRESS ************************************/

#define POS0	0x4e0000
#define POS1    0x4e0001
#define POS2    0x4e0002 
#define POS3    0x4e0003
#define POS4    0x4e0004
#define POS5    0x4e0005
#define POS6    0x4e0006
#define POS7    0x4e0007

/******************* IO ADDRESS ***************************************/
#define IO0     0x1158
#define IO1     0x1159
#define IO2     0x115a
#define IO3     0x115b
#define IO4     0x115c
#define IO5     0x115d
#define IO6     0x115e
#define IO7	0x115f

/*****************************************************/
/*   Integrated Ethernet Test Units Header File   */
/*****************************************************/
/*         initial values for pos reg                          */
#define  INTPOS0         0xf2    /* adapter id                 */
#define  INTPOS1         0x8e    /* adapter id                 */

#define INTERNAL	0x01	/* Internal loopback */ 
#define EXT_DISABLE	0x02	/* External loopback.. /LPBK pin inactivated */
#define EXT_ENABLE	0X03	/* External loopback.. /LPBK pin activated */

#define TK_GDFUSE	0x01	/* riser card type */
#define TK_BDFUSE	0x03 
#define TN_CARD    	0x02
#define TP_CARD   	0x00

#define DEVBUS0		"/dev/bus0" /* machine device driver device */
/**************************************
 *      The Configure command.
 *      note: csc and next_cb set to zero
 **************************************/
struct cfg {
        ulong csc;                              /* cmd,status,control fields */
        ulong next_cb;                          /* link to next cmd block */
        uchar byte_count;                       /* # of CB bytes to configure*/
        uchar fifo_limit;                       /* pt in FIFO to request bus */
        uchar save_bf;                          /* save bad frames */
        uchar loopback;                         /* loopback and addr length */
        uchar linear_pri;                       /* linear priority */
        uchar spacing;                          /* interframe spacing */
        uchar slot_time_low;                    /* slot time for the network */
        uchar slot_time_up;                     /* upper nibble of slot time */
        uchar promiscuous;                      /* accept all frames */
        uchar carrier_sense;                    /* carrier sense filter */
        uchar frame_len;                        /* minimum frame length */
        uchar preamble;                         /* preamble until carrier sns*/
        uchar dcr_slot;                         /* DCR slot number */
        uchar dcr_num;                          /* # of stations in DCR mode */
};

/******************************************************
 * standard structure used by manufacturing diagnostics.
 ******************************************************/

struct tucb_t
   {
	long tu,	/* test unit number   */
	     loop,	/* loop test of tu    */
	     mfg;	/* mfg = 1 if running mfg. diagnostics, else 0  */
			/* mfg = 2 if running HTX */

	long r1,	/* reserved */
	     r2;	/* reserved */
   };

/*************************************************
 * error types and error codes for tca test units.
 *************************************************/

#define SYS_ERR     0x00
#define LOG_ERR     0x02

/**************************************************
 * adapter specific definitions for test units.
 **************************************************/

#define RULE_LEN      8
#define RETRIES_LEN   3
#define YES_NO_LEN    3
#define CONFIG_LEN   15
#define NETADD_LEN    6
#define PARTNO_LEN    8

#define DISABLE       0
#define ENABLE        1

struct wrap_test
   {
   	int wrap_type;
	int fairness;
	int parity;
	int card_type;
   };
#define WRAPTEST	struct wrap_test

struct ethi_tu 
   {
	ushort netid;
	uchar net_addr[NETADD_LEN];
	char rule_id[RULE_LEN+1];
        char retries[RETRIES_LEN+1];
        char show[YES_NO_LEN+1];

	int riser_card;
        long packet_size;
	long num_packet;
	uchar *pattern;  /* reserve */
	int pat_size;    /* reserve */
	ushort default_pk;
	long loop_remain;
   };

struct device_counter 
   {
   int good_other;
   int bad_other;
   int good_write;
   int bad_write;
   int good_read;
   int bad_read;
   int byte_read;
   int byte_write;
   };

/******************************************************************
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 ******************************************************************/

struct ethi_tucb
   {
	struct tucb_t header;
	FILE *msg_file;		  
	int mdd_fd;		/* machine device driver file descriptor */
	struct ethi_tu ethi;
	struct device_counter counter;
   };

#define TUTYPE struct ethi_tucb
#define SESBLK struct session_blk

#define BOOLEAN	unsigned char
#define BYTE	unsigned char
#define HALFWD	unsigned short
#define WORD	unsigned long

/***** riser card return code *****/
#define TWISTED_PAIR	0x8880
#define THICK           0x8881
#define THIN            0x8882

/*********************************************************************
 * Error codes. Please reserve 0x8880 to 0x8889 for good return code
 * and 0x9xxx for errno .      
 *********************************************************************/

/************ POS Register Error Codes (tu001) **************************/
#define POS0_RD_ERR	0x0100	/* Error reading POS register 0 */
#define POS1_RD_ERR	0x0101	/* Error reading POS register 1 */
#define POS2_RD_ERR	0x0102	/* Error reading POS register 2 */
#define POS4_RD_ERR	0x0104	/* Error reading POS register 4 */
#define POS5_RD_ERR	0x0105	/* Error reading POS register 5 */
#define POS6_RD_ERR	0x0106	/* Error reading POS register 6 */

#define POS2_WR_ERR	0x0112	/* Error writing POS register 2 */
#define POS4_WR_ERR	0x0114	/* Error writing POS register 4 */
#define POS5_WR_ERR	0x0115	/* Error writing POS register 5 */
#define POS6_WR_ERR	0x0116	/* Error writing POS register 6 */

#define POS0_CMP_ERR	0x0120	/* POS 0 has invalid value */
#define POS1_CMP_ERR	0x0121	/* POS 1 has invalid value */
#define POS2_CMP_ERR	0x0122	/* POS 2 Write/Read did not compare */
#define POS4_CMP_ERR	0x0124	/* POS 4 Write/Read did not compare */
#define POS5_CMP_ERR	0x0125	/* POS 5 Write/Read did not compare */
#define POS6_CMP_ERR	0x0126	/* POS 6 Write/Read did not compare */

/******************* I/O Register Error Codes (tu002) ***********************/
#define IO0_RD_ERR	0x0200	/* Error reading Addr Lo Register */
#define IO1_RD_ERR	0x0201	/* Error reading Addr Hi Register */
#define IO2_RD_ERR	0x0202	/* Error reading Port Data Lo Register */
#define IO3_RD_ERR	0x0203	/* Error reading Port Data Hi Register */
#define IO4_RD_ERR 	0x0204	/* Error reading Interrupt Status Register */
#define IO5_RD_ERR	0x0205	/* Error reading Check Status Register */
#define IO6_RD_ERR	0x0206	/* Error reading CA Control Register	*/

#define IO0_WR_ERR	0x0210	/* Error writing Addr Lo Register */
#define IO1_WR_ERR	0x0211	/* Error writing Addr Hi Register */
#define IO2_WR_ERR	0x0212	/* Error writing Port Data Lo Register */
#define IO3_WR_ERR	0x0213	/* Error writing Port Data Hi Register */
#define IO4_WR_ERR	0x0214	/* Error writing Interrupt Status Register */
#define IO5_WR_ERR	0x0215	/* Error writing Check Status Register */
#define IO7_WR_ERR      0x0217  /* Error writing Port Control Register  */

#define IO0_CMP_ERR	0x0220	/* Addr Lo Register Write/Read did not compare*/
#define IO1_CMP_ERR	0x0221	/* Addr Hi Register Write/Read did not compare*/
#define IO2_CMP_ERR	0x0222	/* Port Data Lo Reg Write/Read did not compare*/
#define IO3_CMP_ERR	0x0223	/* Port Data Hi Reg Write/Read did not compare*/
#define IO4_CMP_ERR	0x0224	/* Inter Status Reg Write/Read did not compare*/
#define IO5_CMP_ERR	0x0225	/* Chect Status Reg Write/Read did not compare*/

/*************** LOCAL RAM Error Codes (tu003) *******************************/
#define MEM_RD_ERR	0x0300	/* Error reading shared memory */
#define MEM_WR_ERR	0x0301	/* Error writing shared memory */
#define MEM_CMP_ERR	0x0302	/* Memory Write/Read did not compare */

/******************* VPD error codes (tu004) *****************************/
#define VPD_HDR_ERR     0x0400  /* VPD header data VPD invalid */
#define VPD_CRC_ERR     0x0401  /* VPD CRC's do not match */
#define VPD_LEN_ERR     0x0402  /* VPD maximum length exceeded */
#define NAF_MULT_ON     0x0403  /* Multicast of Network Address is ON */
#define NO_ETH_VPD	0x0404	/* No Int. Ethernet VPD */

#define SPOS3_RD_ERR    0x0410  /* Reading sys. POS reg 3 error */
#define SPOS6_WR_ERR    0x0411  /* Writing sys. POS reg 6 error */
#define SPOS7_WR_ERR    0x0412  /* Writing sys. POS reg 7 error */

/**************** SELF TEST Error Codes (tu005) ******************************/
#define SELFTEST_FAIL   0x0500  /* Self-test failed */
#define ENT_SELFTEST_ERR 0x0501  /* ioctl of ENT_SELFTEST failed */

/**************** WRAP (tu006-tu015) ***************************************/
#define WRAP_WR_ERR	0x0900	/* Error writing to filedescriptor */ 
#define WRAP_RD_ERR	0x0901	/* Error reading from filedescriptor */ 
#define WRAP_CMP_ERR 	0x0902  /* Packet data did not compare */
#define BAD_PCK_S_ERR   0x0904  /* Bad Packet size - internal error */
#define ENT_CFG_ERR	0x0905  /* Unable to send loop back configuration */
#define ENT_POS_ERR 	0x0906	/* Unable to send POS data to device driver */
#define WRAP_NODATA	0x0907  /* No data available for reading */
#define CIO_NOT_OK	0x0908	/* Transmit complete but unsuccessful */
#define TX_TIMEOUT	0x0910  /* Transmit timeout */
#define RX_TIMEOUT	0x0911	/* Receive timeout */
#define IOCTL_TIMEOUT	0x0912	/* ioctl timeout */
#define UNDERRUN_ERR	0x0913	/* Underrun exceed number of retry */
#define OVERRUN_ERR	0x0914	/* Overrun exceed number of retry */

/******************* TP: Collision Test error codes (tu016) **************/
#define CIO_QUERY_ERR   0x1600  /* ioctl: Error querying ent_query_stats_t */
#define CONNECTOR_ERR   0x1601  /* Wrap: Connector does not work properly */
#define STAT_NOT_AVAIL	0x1602  /* No status available for CIO_GET_STAT */
#define TX_INCOMPLETE	0x1603	/* Transmission is incomplete */
#define NO_COLLISION	0x1604	/* No collision is detected */

/********************* RISER CARD Error Codes (tu017) ***********************/
#define RISERCARD_ERR   0x1700  /* Invalid riser card */
#define BADFUSE   	0x1701  /* Thick Riser Card has a blown fuse */

/*********  st_eth.c error codes *********************************/
#define START_ERR	0x2000	/* START failed */
#define GET_STAT_ERR	0x2001	/* START status completion error */
#define START_TIME_ERR	0x2002	/* START status timed out - no completion */
#define START_BSTAT_ERR	0x2003	/* Bad START completion received */
#define HALT_ERR	0x2004	/* HALT failed */
#define HALT_TIME_ERR	0x2005  /* HALT status timed out - no completion */
#define HALT_BSTAT_ERR	0x2006	/* halt Bad HALT completion received */
#define ILLEGAL_TU_ERR  0x7777  /* Illegal TU value - internal error */

