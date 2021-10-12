/* @(#)34       1.11  src/bos/diag/da/cdrom/dcdrom.h, dacdrom, bos411, 9437B411a 9/14/94 13:07:14 */
/*
 *   COMPONENT_NAME: DACDROM
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef TRUE
#define TRUE		1      /* logical true.          */
#endif
#ifndef FALSE
#define FALSE		0      /* logical false.         */
#endif

#define srchstr         "%s -N %s"
#define	MAX_REQ_TRY	2

#define YES_ANS		1
#define NO_ANS		2

#define OTHER_SCSI	0x0723  /* OEM CD-ROM Drive */
#define ORIG_CDROM	0x0974  /* IBM CD-ROM. Uses a caddy, non-XA mode. */
#define ATL_CDROM	0x0987  /* IBM CD-ROM. Uses a caddy, XA mode. */
#define BAND_CDROM	0x89a   /* IBM CD-ROM. Has a tray, XA mode. */

#define CHK_ASL_READ	99
#ifndef CATD_ERR
#define CATD_ERR	-1
#endif
