/* @(#)82       1.2  src/bos/kernext/lft/inc/fkproc.h, lftdd, bos411, 9428A410j 6/1/94 17:18:51 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993-1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* --------------------------------------------------------------------	*
 * Included in this header file are the structures required to support	*
 * fonts and the font kernel process.  					*
 *									*
 * This information was contained in the hftss.h file in the hft	*
 * environment.								*
 * --------------------------------------------------------------------	*/

#include <sys/intr.h>
#include <fkprocFont.h>

typedef struct {
	ushort		setup_shm;	/* 0 = shm is not attached	*/
					/* 1 = shm is attached		*/
	ushort		unused_short;	/* explicitly mention space     */
	char		*addr_shm_seg;	/* start addr of shared seg	*/
	int		segID;
	int		unused[10];	/* Pad to avoid future size chg */
	int		(*fsp_enq)();	/* Pointer to the enqueue func	*/
	int		(*fsp_deq)();	/* Pointer to the dequeue func	*/
	int		(*unused0)();
	int		(*unused1)();
	fkprocState	fsq;		/* command queue for fonts	*/
} lft_fkp_t;
