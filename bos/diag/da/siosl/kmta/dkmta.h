/* @(#)23	1.3  src/bos/diag/da/siosl/kmta/dkmta.h, dasiosl, bos411, 9428A410j 1/27/94 15:27:06 */
/*
 * COMPONENT_NAME: dkmta.h
 *
 * FUNCTIONS: list
 *                                                                    
 * ORIGIN: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*      used by all 3 DAs                                                 */

#define YES     1
#define NO      2

#define OPEN_MODE (O_RDWR)      /* device mode: read/write                */

#define OPEN_DD (open(devname,OPEN_MODE))  /* System call to open mouse,  */
                                           /* keyboard, or tablet device  */
                                           /* if HFT is not there.        */
#define TIMEOUT	40
#define	ERR_FILE_OPEN	-1
#define	NO_ERROR	0
#define	KTM_TYPE	2
#define INVALID_RETURN	-1
#define KMT_FUSE_CHK    20   /* Fuse test. TU #20 for all 3 adapter tests.  */

/*	keyboard adapter specific defines				     */

#define KBD_ADP		0    /* arbitrary numerical value for keyboard adap. */
#define KDB_RESET	10   /* NIO Keyboard Adapter Register test.        */
#define KDB_SPKR_REG	30   /* NIO Keyboard Adapter Speaker reg.test.     */
 
#define	PTR_RESET_TST	0    /* pointer to test #1 execute flag.             */ 
#define PTR_KFUSE_TST   1    /* pointer to test #2 execute flag.             */
#define	PTR_SPKR_REG	2    /* pointer to test #3 execute flag.             */

/*	mouse adapter specific defines				             */
	
#define MSE_ADP		1     /* arbitrary numerical value for mouse adapt.  */ 
#define MSE_READ_STATUS 10    /* NIO Mouse Adapter Register test.          */
#define MSE_POS_REG	15    /* NIO Mouse Adapter POS Register test.      */	
#define MSE_NON_BLOCK   25    /* NIO Mouse Non-Block test                  */
#define MSE_WRAP_DATA	30    /* NIO Mouse Port wrap test.                 */
 
#define PTR_READ_TST	3     /* pointer to test # 1 execute flag.           */
#define PTR_POS_TST	4     /* pointer to test # 2 execute flag.	     */	
#define PTR_MFUSE_TST   5     /* pointer to test # 3 execute flag.           */
#define PTR_NBLK_TST    6     /* pointer to test # 4 execute flag.           */
#define PTR_MWRP_TST	7     /* pointer to test # 5 execute flag.           */

#define ASK_MOUSE	1
#define REMOVE_CABLE	2
#define CONNECT_CABLE	3

/*      tablet adapter specific defines                                      */

#define	TAB_ADP		2       /* arbitrary numerical value for tablet adapt*/ 
#define TBL_INT_WRAP	10      /* NIO Tablet Disable/Enable UART test     */

#define PTR_TWRP_TST 	8	/* pointer to test # 1 execute flag.         */
#define PTR_TFUSE_TST   9       /* pointer to test # 2 execute flag.         */

#define	KBD_CLN		99	/* NIO Keyboard clean test		    */

#define	PTR_KCLN_TST	10

struct salio_tucb
   {
        struct tucb_t header;
        int mach_fd;             /* machine device driver file descriptor */
   };

#define TUTYPE struct salio_tucb

