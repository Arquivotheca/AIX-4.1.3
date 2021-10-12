/* @(#)29       1.1.1.2  src/bos/diag/tu/siosl/kbda/misc.h, tu_siosl, bos41J, 9515A_all 4/7/95 11:02:12 */
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


#define FIREB_MODEL                 0xA6   /* FIREB_MODEL (rosinfo.h)*/

/* Define error codes for Salmon system I/O and adapter TU's */

#define	IO_ERROR	(-1)	/* returned by ioctl()	*/

#define	KBD_RESET_LOGIC_ERROR   10
#define	KBD_LOGIC_ERROR         11
#define	KBD_TX_ERROR            12
#define	KBD_RX_ERROR            13
#define	KBD_DATA_COMPARED_ERROR 15
#define	KBD_CMD_NOT_ACK_ERROR   40
#define	KBD_EXTERNAL_BAT_ERROR  41
#define	KBD_RESET_CMD_ERROR     19
#define	SPK_LOGIC_ERROR         30

#define	WRONG_TU_NUMBER        256
#define	FUSE_BAD_ERROR          20

#define	FOREVER	while(1)

#define	BLANK	' '
#define	TAB		'\t'
#define EOS		'\0'

#define SUCCESS          0
#define FAIL             1

/* Define print macros for HTX */ 

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
  int mach_fd;  /* File descriptor for machine device driver */
};

/* Global flag for checking if PowerPC or RSC product */
int power_flag;
