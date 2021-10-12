/* @(#)54       1.1.1.1  src/bos/diag/tu/siosl/tableta/misc.h, tu_siosl, bos411, 9428A410j 11/4/93 16:08:05 */
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

/* Define error codes for Salmon system I/O and adapter TU's */

#define	IO_ERROR	(-1)	/* returned by ioctl()	*/

#define TABLET_RESET_ERROR      12
 
#define	WRONG_TU_NUMBER        256
#define	FUSE_BAD_ERROR          20
#define	TAB_ENABLE_ERROR        30
#define	SET_WRAP_ERROR          31
#define	BAD_WRAP_DATA           32
#define TABLET_WRAP_PLUG_ERROR  40

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

/* Set up structure for testing */

typedef struct TUCB {
  struct tucb_t header;
  int mach_fd;    /* File descriptor for machine device driver */
};

