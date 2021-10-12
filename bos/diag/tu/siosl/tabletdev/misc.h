/* @(#)58       1.3  src/bos/diag/tu/siosl/tabletdev/misc.h, tu_siosl, bos411, 9428A410j 12/17/93 11:12:51 */
/*
 * COMPONENT_NAME: TU_SIOSL (misc.h include for tablet device)
 *
 * FUNCTIONS: none
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
 */

/* Define error codes for Salmon system I/O and adapter TU's */

#define	IO_ERROR	(-1)	/* returned by ioctl()	*/

#define TABLET_RESET_ERROR      12
 
#define	WRONG_TU_NUMBER        256
#define	FUSE_BAD_ERROR          20

#define	FOREVER	while(1)

#define	BLANK	' '
#define	TAB		'\t'
#define EOS		'\0'

#define SUCCESS          0
#define FAIL             1

/* The following ifdef is for unit testing purposes */

#ifdef SUZTESTING
#define PRINT		printf
#define PRINTERR	printf
#else
#define PRINT	
#define PRINTERR	
#endif
