/* @(#)27       1.1  src/bos/usr/include/diag/diag_trace.h, cmddiag, bos41J, 9514A_all 4/4/95 16:21:01 */
/*
 *   COMPONENT_NAME: libdiag
 *
 *   FUNCTIONS: Diagnostic Trace 
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

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

