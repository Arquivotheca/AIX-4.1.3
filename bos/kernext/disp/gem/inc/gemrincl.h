/* @(#)01	1.5.1.6  src/bos/kernext/disp/gem/inc/gemrincl.h, sysxdispgem, bos411, 9428A410j 3/10/93 07:23:15 */

/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		rFree
 *		rMalloc
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*;************************************************************************/
/*;                                                                       */
/*; CHANGE HISTORY:                                                       */
/*;LW  08/29/89	Replace use of gem_rcm.h with gsgmstr.h and gsgmrcm.h	  */
/*;LW  08/31/89	Added gsgmmac.h						  */
/*;LW  09/01/89	Moved gem_ddstr.h to gemincl.h				  */
/*;LW  09/12/89	Added gem_ddstr.h              				  */
/*;LW  09/14/89	Added gem_err.h                				  */
/*;MC  09/15/89 Renamed gem_rcx to gem_rcmstr and added gem_rcmdef	  */
/*;JG  10/04/89 add #define for pid_t ifndef KERNAL                       */
/*;LW  10/04/89 undo   							  */
/*;MC  11/29/89 Changed KERNEL to _KERNEL and removed KGNHAK		  */
/*;LW   12/11/89   Move to new h-file structure                         @1*/ 
/*;************************************************************************/
/* @1 */

#include "gmcomm.h"
#include "gem_err.h"
#include "gem_rcmdef.h"
#include "gem_rcmstr.h"


#ifdef _KERNEL

#define rMalloc( NUM )	xmalloc( (NUM), 3, kernel_heap )
#define rFree( PTR )	xmfree( (PTR), kernel_heap )

#else

/*  struct vtmstruc {
    struct gemini_data	*vttld;
    }; */

#define rMalloc( NUM )	malloc( NUM )
#define rFree( PTR )	free( PTR )

#endif

