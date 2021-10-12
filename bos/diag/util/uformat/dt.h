/* @(#)79       1.1  src/bos/diag/util/uformat/dt.h, dsauformat, bos41J, 9512A_all 3/17/95 09:51:55 */
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <memory.h>
#include <errno.h>
#include <sys/buf.h>
#include <sys/signal.h>

#include <sys/scsi.h>
#include <sys/scdisk.h>

#include "diag/scsi_atu.h"
#include "diag/diag.h"
#include "diag/da.h"            /* FRU Bucket Database */
#include "diag/da_rc.h"
#include "diag/dcda_msg.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"     /* return codes for DC */
#include "diag/tmdefs.h"        /* diagnostic modes and variables     */
#include "diag/diagodm.h"
#include  "sys/cfgodm.h"

#define DT_TMI			1	/* Init dt and print some TMI stuff. */
#define DT_SCSI_TUCB		2	/* Print SCSI TUCB CDB, and data out */
					/* if flags are set to B_WRITE.      */
#define DT_SCSI_TUCB_SD 	3	/* Print SCSI TUCB Sense Data.	     */
#define DT_DEC			10	/* Print a decimal variable.	     */
#define DT_MDEC 		11	/* Print multiple decimal variables. */
#define DT_HEX			16	/* Print a hex variable.	     */
#define DT_MHEX 		17	/* Print multiple hex variables.     */
#define DT_MSTR 		20	/* Print multiple string variables.  */
#define DT_MSG			25	/* Simple message like "hello".      */
#define DT_BUFF 		30	/* Print a data buffer. 	     */
#define DT_END			999	/* Print "end of trace" msg.         */

