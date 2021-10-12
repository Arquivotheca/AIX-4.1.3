/* @(#)97	1.6.1.12  src/bos/kernext/disp/gem/inc/gem_rcmstr.h, sysxdispgem, bos411, 9428A410j 5/28/93 15:09:32 */

/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: 
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


/*;**********************************************************************/
/*;CHANGE HISTORY                                                       */
/*;LW 08/29/89 Fixed sccs tag to header file type                       */
/*;LW 09/11/89 Added some stuff for rcxp                                */
/*;LW 09/12/89 Merged changes from rcx development                      */
/*;LW 09/12/89 Changed ctx to cxt                                       */
/*;LW 09/13/89 Moved rGemrcx to gem_ddstr                               */
/*;MC 09/15/89 Moved rWAPriv and rWGPriv from gem_ddstr.h		*/
/*;DM 09/17/89 fixed syntax error inside #ifdef R2                    @7*/
/*;DM 09/21/89 added xmem struct to Gemproc                           @8*/
/*;MC 10/24/89 moved hwid field from rGemrcx to rWGPriv		      @9*/
/*;MC 11/21/89 moved wa and wg change masks from rcx priv to wa and wg	*/
/*;	       priv, respectively					*/
/*;MC 11/29/89 Changed KERNEL to _KERNEL and removed KGNHAK		*/
/*;LW 01/16/90 Change groups_used type                       	      @A*/
/*;MC 01/25/90 Added color table id to geometry private area	      @B*/
/*;LW 02/28/90 Added fifolockptr to gemproc                   	      @B*/
/*;LW 06/12/90 changed fault_addr to fault_pid                	        */
/*;                                                                     */
/*;**********************************************************************/

#include <sys/watchdog.h>

/* 
 * Gemini Specific Data pointed to by pData in struct rcmProc
 */
typedef	struct	_Gemproc {	/*structure pointed to by pData in rcmproc */
	shmFifoPtr	shmem;/* Pointer to user process<=>kernel fifo ptrs */
	FifoLockPtr	pLocks;/* Pointer to fifo locks                      */
	ulong		fault_pid;  /* PID of faulting process */
	struct	xmem	xs;        /* fifo ptrs cross memory descriptor  */
	struct	xmem	xs_fifolocks;	/* cross memory descriptor	    */
	struct  watchdog watch;	    /* watchdog timer structure	*/	
} rGemproc, *rGemprocPtr;


/****************************************************************************/
/*                    Window Attributes Private Data                        */
/****************************************************************************/
/* RJE:  Added pointer to structure used to improve client clip performance */
/* (specifically for graPHIGS rubberbanding).  Please refer to ORBIT defect */
/* 78706 for details.                                                       */
/****************************************************************************/

typedef struct _rWAPriv {
  int		active_group;       /* Index into groups_used of current group */
  int		num_groups;	    /* Number of group attributes */
  rDefGroupsPtr	groups_used;	    /* Array of group types */
  int          	gWAChangeMask;	    /* Change mask for differed update WA */
  GemWAClientClipPtr ClientClipWA;  /* For client clip performance */
} rWAPriv, *rWAPrivPtr ;


/****************************************************************************/
/*                      Window Geometry Private Data                        */
/****************************************************************************/
/****************************************************************************/
/* RPD added deferredClearOverlays:  When a new window is being brought     */
/*   up, we check to see whether it is 2D or 3D.  If it is 2D, we want to   */
/*   clear the overlay planes for that window, since X knows nothing        */
/*   about overlay planes but we don't want them to show up in an XWindow.  */
/*   Sometimes we have no clue who will be drawing into the window, though, */
/*   so we defer the clear until we can determine whether it is a 2D window */
/*   or a 3D window.                                                        */
/****************************************************************************/

typedef struct _rWGPriv {
    unsigned short	type;
    long 	hwid;		       /* hardware window id */
    ulong	imm_sync_cntr;	       /* imm fifo hwid sync counter */
    ulong	trv_sync_cntr;	       /* trav fifo hwid sync counter */
    ulong	gWGChangeMask;	       /* Change mask for differed update WG */
    Bool	zBufferWind;	       /* Does window use Z Buffer? */
    ulong       deferredClearOverlays;
} rWGPriv, *rWGPrivPtr ;





















