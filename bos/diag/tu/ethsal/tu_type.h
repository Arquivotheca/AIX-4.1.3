/* @(#)02       1.2  src/bos/diag/tu/ethsal/tu_type.h, tu_ethsal, bos411, 9428A410j 10/22/93 09:50:38 */
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#define USE_DEFAULT_FRAME_SIZE      0
#define USE_DEFAULT_NUM_FRAMES      0

#define STATUS_TIMEOUT              15000        /* 15 sec */
#define READ_TIMEOUT                15000        /* 15 sec */

/* use next indexes with function get_fd()  */
#define MDD_FD                      0
#define ETHS_FD                     1

#define NET_ID                      0x5001       /* got this from B. Kovacs */
#define NET_ID_LENGTH               2
#define NETW_ADDR_LENGTH            6            /* in bytes */

#define MIN_FRAME_SIZE              60   /*bytes, 4-byte CRC will be appended*/
#define MAX_FRAME_SIZE              1510 /*bytes, 4-byte CRC will be appended*/

/* Define LOGMSGS if you want messages to be logged */

#ifdef LOGMSGS
extern void log_syserr(char *);
extern void logmsg(char *);
extern void logerror(char *);
#define LOG_SYSERR(msg)  log_syserr(msg)
#define LOG_MSG(msg)     logmsg(msg)
#define LOG_ERROR(msg)   logerror(msg)
#else
#define LOG_SYSERR(msg)
#define LOG_MSG(msg)
#define LOG_ERROR(msg)
#endif

#ifdef DEBUG_ETHER
#define DEBUG_MSG(msg)     logmsg(msg)
#else
#define DEBUG_MSG(msg)
#endif


#define MULTICAST_BIT 0x80

/* loopback types */
enum { LB_OFF = 0, /* no loopback                                             */
       LB_82596,   /* loopback at the 82596                                   */
       LB_EXT,     /* loopback at the DIX connector  (LPBK pin not activated) */
       LB_82501    /* loopback at the 82501 (LPBK pin activated)              */
     };



enum STATIST_TYPES { BAD_OTHERS = 0, BAD_READS, BAD_WRITES,
                     BYTES_READ, BYTES_WRITTEN, GOOD_OTHERS,
                     GOOD_READS, GOOD_WRITES
                    };


enum /* used in debugging */
{
  DUMP_VPD_TU = 90, /* dumps VPD data to standard output  */
  NADDR_TU          /* dumps network address to std. out. */
};


/* function prototypes */

extern int exectu(int, TUCB *);
extern int get_fd(int);
extern void set_tu_errno(void);
extern int get_machine_model(int);
extern char *get_tu_name(ulong_t);
extern int enable_eths(void);
extern int Get_byte(int, uchar_t *, ulong_t, int);
extern int put_byte(int, uchar_t, ulong_t, int);
extern int get_word(int, ulong_t *, ulong_t, int);
extern int put_word(int, ulong_t, ulong_t, int);
extern ushort_t crc_gen(uchar_t [], int);
extern int start_ether(void);
extern int halt_ether(void);
extern int config_ether(uchar_t);
extern BOOL set_frame_size(ulong_t);
extern void set_frame_num(ulong_t);
extern STATIST_STRUCT *get_stat(void);
extern void incr_stat(uchar_t, ulong_t);
extern void reset_stat(void);
extern int restore_eth_regs(void);
extern int op_result(int, int);
extern int wait_status (int, int, int, ushort);

extern int pos_tu(void);
extern int vpd_tu(void);
extern int io_tu(void);
extern int selftest_tu(void);
extern int lb_82596_tu(void);
extern int lb_82501_tu(void);
extern int lb_ext_tu(void);
extern int lb_ext_emc_tu(void);

extern int dump_vpd_tu(void);
extern int naddr_tu(void);


#define MAX_ERR 10000 /* no error codes should exceed this number */


/****************************** POWER/RSC ADDRESS SPACE *************/

#define CRESET_REG_ADDR_RSC     0x0040002c  /* RSC component reset */
#define CRESET_REG_ADDR_POWER   0x000100A0  /* POWER component reset */
  
ulong_t creset_reg_addr;  /* comp. reset reg. address global */

#define ETH_POS_BASE_ADDR 0x004e0000

/* Setup POS 0 & 1 defines for RSC & RBW */

#define POS0_ID_RSC 0xf3
#define POS1_ID_RSC 0x8e

#define POS0_ID_POWER 0x98
#define POS1_ID_POWER 0x8f


#define ETH_IO_BASE_ADDR_RSC  0x000000c0
#define ETH_IO_BASE_ADDR_POWER  0x000000f0

#define ETH_MODE_SETTING_MASK 0x00000004


/* Ethernet POS register addresses  */

enum { ID1_REG_ADDR = ETH_POS_BASE_ADDR, ID2_REG_ADDR,
       CONTROL_REG_ADDR, VPD_DATA_REG_ADDR, DMA_CONTROL_REG_ADDR,
       POS_STATUS_REG_ADDR, VPD_ADDR_REG_ADDR, RSV_POS7_ADDR
     };

/* Ethernet I/O registers address  */

ulong_t port_reg_addr;  /* ethernet I/O base addr global */

#define PORT_REG_ADDR       port_reg_addr
#define CA_REG_ADDR        (PORT_REG_ADDR + 4)
#define IO_STATUS_REG_ADDR (CA_REG_ADDR + 4)

/*****************************************************************/

