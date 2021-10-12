/* @(#)91	1.5.1.1  src/bos/diag/da/scsi/dscsia.h, dascsi, bos411, 9428A410j 7/20/92 10:48:11 */

/*
 * COMPONENT: DAMEDIA - SCSI adapter source code.
 *
 * FUNCTIONS: List
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_DSCSIA
#define _H_DSCSIA

#define CRD_DIAG_TST 0          /*  card diagnostics failure           */
#define FUSE_FAILURE 1          /*  adapter fuse failure               */
#define CRD_WRAP_TST 2          /*  execute card SCSI wrap tests       */
#define VPDTEST_FAIL 2		/*  VPD test failure. Same SRN as WRAP */
				/*  test failure		       */
#define BCR_REGS_TST 3          /*  BCR register failure               */
#define POS_REGS_TST 4          /*  POS register failure               */
#define BUS_RESET    5          /*  Adapt failed to complete bus reset */
#define CRD_TIMEOUT  6          /*  adapter cmd timed out              */
#define DD_CONFIG    7          /*  DD/CONFIG error                    */
#define ERROR_LOG1   8          /*  Error log analysis error           */
#define ERROR_LOG2   9          /*  Error log analysis error           */
#define ERROR_LOG3   10         /*  Error log analysis error           */
#define ROM_CRC_ERR  11         /*  ROM CRC error                      */
#define ADAP_RAM_ERR 12         /*  Adapter RAM error                  */
#define SCRP_RAM_ERR 13         /*  SCORPION chip RAM error            */
#define SCRP_LOGIC_ERR 14       /*  SCORPION chip logic error          */
#define SCSI_POR_ERR 15         /*  SCSI control chip POR error        */
#define SCSI_REG_ERR 16         /*  SCSI control chip register error   */
#define PREV_ERR     17         /*  Diagnose complete--previous errors */
#define	DATA_LOSS    18		/*  Down level card with data loss problem */
#define	CONFIG_FAILED 20
#define	CONFIG_FAILED_PTC 21

#define MAX_BUF 1024

#define ONE "1"
#define TWO "2"
#define ADAP_TIMEOUT 30         /* Timeout(secs) to wait for cmd completion */
#define bad_da_error -1
#define srchstr "%s -N %s"
int	    failing_function_code;
int         fdes;                            /* file descriptor for device */
int         fd_dname;             /* drive type...determined from ODM data */
long        damode;                /* Selected Diagnostic Application mode */
int         menu_return;               /* return value from menu selection */
int         errno_rc;           /* errno received after openx & ioctl op's */

#endif /* _H_DSCSIA */
