/* @(#)76       1.2  src/bos/diag/tu/siosl/mousea/tu_type.h, cmddiag, bos41J, 9515A_all 4/7/95 11:05:32 */
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: ERR_DESC
 *              PRINT
 *              PRINTERR
 *              PRINTSYS
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Mouse adapter register addresses                             */

#define MOUSE_DATA_TX_REG               0x0048
#define MOUSE_CMD_REG                   0x0049
#define MOUSE_STAT_REG                  0x004a
#define MOUSE_RX_STAT_REG               0x004c
#define MOUSE_RX_1_REG                  0x004d
#define MOUSE_RX_2_REG                  0x004e
#define MOUSE_RX_3_REG                  0x004f
#define IOR_PB_8255              	0x0055  /* Diagnostic register in KTS
						   adapter */

/* Values set for mouse command register  */
#define MOUSE_CMD_TX_DATA               0x00
#define MOUSE_CMD_DIAG_WRAP             0x01
#define MOUSE_CMD_ENABLE_INTERRUPT      0x02
#define MOUSE_CMD_DISABLE_INTERRUPT     0x03
#define MOUSE_CMD_ENABLE_BLK_MODE       0x04
#define MOUSE_CMD_DISABLE_BLK_MODE      0x05

/* Bits mapped for mouse status register  */
#define MOUSE_STAT_INTERRUPT_ENABLED    0x01
#define MOUSE_STAT_BLK_MODE_ENABLED     0x02
#define MOUSE_STAT_BLK_MODE_DISABLED    0x04

/* Bits mapped for mouse receive status register */
#define MOUSE_STAT_RX_INTERRUPT         0x01
#define MOUSE_STAT_RX_DATA_ERROR        0x02
#define MOUSE_STAT_RX_INTF_ERROR        0x04
#define MOUSE_STAT_RX_REG1_FULL         0x08
#define MOUSE_STAT_RX_REG2_FULL         0x10
#define MOUSE_STAT_RX_REG3_FULL         0x20


/* Machine types */

#define FIREB_MODEL                 0xA6   /* FIREB_MODEL (rosinfo.h)*/

/* The following define is intended for use in code for RS1/RS2 products */
#define MOU_FUSE_BAD                    0x04

/* The following define is for tu20, since the fuse status for RSC/RBW is 
   contained within the tablet modem status register. */

#define TABLET_MODEM_ST_REG             0x0076

#define SIO_REG_RESET_MOUSE     0x80
#define SIO_REG_RESET_MOUSE_MSK 0x7F

#define SIO_ID0_REG             0x004f0000
#define SIO_ID1_REG             0x004f0001
#define SIO_CONTROL_REG         0x004f0002


/* Register definitios for G30 */

#define SIO_ID0_G30REG		0x00460000
#define SIO_ID1_G30REG		0x00460001
#define SIO_CTL_G30REG		0x00460000


/* System registers :            */
/* Below is system register define for RSC */

#define COMP_RESET_REG_RSC      0x0040002c      /* Component Reset Register */

/* Below is system register define for POWER */

#define COMP_RESET_REG_POWER    0x000100A0      /* Component Reset Register */


#define REPEAT_COUNT            100       /* Number of times to retry     */


/* Define error codes for Salmon system I/O and adapter TU's */

#define	IO_ERROR	(-1)	/* returned by ioctl()	*/

#define	MOUSE_LOGIC_ERROR       12
#define MOUSE_NON_BLOCK_ERROR   25
#define MOUSE_BLOCK_ERROR       35
#define MOUSE_WRAP_ERROR        22

#define	WRONG_TU_NUMBER        256
#define	FUSE_BAD_ERROR          20

#define	FOREVER	while(1)

#define	BLANK	' '
#define	TAB		'\t'
#define EOS		'\0'

#define SUCCESS          0
#define FAIL             1

/* Define print macros */ 

#ifdef LOGMSG
#define PRINT(msg)       logmsg(msg)
#define PRINTERR(msg)    logerror(msg)
#define PRINTSYS(msg)    log_syserr(msg)
#define ERR_DESC(rc)     get_err_desc(rc)
#else
#define PRINT(msg)    
#define PRINTERR(msg)
#define PRINTSYS(msg)
#define ERR_DESC(rc)   
#endif

/* Set up structure for planar testing */

typedef struct TUCB {
  struct tucb_t header;
  int mach_fd;   /* File descriptor for machine driver */
};

/* Declare global variables for system registers */
unsigned int comp_reset_reg;

/* This structure will be used to store the machine type, and mse/kbd/tab
   polyswitch mask which will be used by tu20 for RSC/RBW products */

struct info_struct {
  uchar switch_stat_mask;
  uchar mach_type;
  int machine;
  uchar mach_model;
} mach_info;

/* Initialize 'POWER' and 'RSC' flags */
enum machines { RSC = 1, POWER };

/* Function prototypes */

#ifdef DIAGNOSTICS
extern int mexectu(long, struct TUCB *);
#else
extern int exectu(long, struct TUCB *);
#endif

extern int get_machine_model_tum(int);
extern int set_semm(int);
extern int rel_semm(void);
extern int setup_mouse(int);
extern int rd_bytem(int, uchar *, uint);
extern int rd_posm(int, uchar *, uint);
extern int rd_wordm(int, uint *, uint);
extern int wr_2bytem(int, ushort *, uint);
extern int wr_bytem(int, uchar *, uint);
extern int wr_posm(int, uchar *, uint);
extern int wr_wordm(int, uint *, uint);
extern int tu10m(long, struct TUCB *);
extern int tu15m(long, struct TUCB *);
extern int tu20m(long, struct TUCB *);
extern int tu25m(long, struct TUCB *);
extern int tu30m(long, struct TUCB *);


