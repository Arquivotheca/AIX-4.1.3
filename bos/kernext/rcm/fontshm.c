static char sccsid[] = "@(#)45	1.5.1.1  src/bos/kernext/rcm/fontshm.c, rcm, bos41J, 9520A_all 5/3/95 14:02:11";
/*
 *
 *   COMPONENT_NAME: (rcm) Rendering Context Manager Fonts
 *
 * FUNCTIONS: - make_shm():  enqueue command to fkproc() to attach shm seg. 
 *            - unmake_shm(): enqueue command to fkproc() to detach shm seg 
 *            - fkproc_destroy_shm(): destroy shared mememory segment for fonts 
 *
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* #define DEBUG */
#include <lft.h>                    /* includes for all lft related data */
#include <sys/syspest.h>                /* debugging flags */
#include "xmalloc_trace.h"


BUGVDEF(db_fontshm,99);



/*------------------------------------------------------------------------- 

        Font faults occur because the adapter has limited address space
        for font data.  Thus, often times the adapter will not have the
        font data stored locally when a X-client or lft issues command to 
        use a particular font for some text data.  When this occurs, the 
        adapter raises an interrupt to the CPU called the font request.
	The font request is routed through the 1st interrupt handler to
        the 2nd level interrupt handler (SLIH - see midintr.c) which is 
        the adapter specific interrupt handler.  It would be nice if SLIH 
        could handle all the processing necessary to send the font data
        to the adapter.  However, it cannot for the reason explained in 
        the next paragraph.  Therefore, for a font request SLIH merely
        schedules (wakes up) a separate special process.  It is this
        process which actually prepares the font for the adapter.  For
        clarity, let's call this process, the font kernel process or fkproc 

	A quick recap tells us that we are trying to get the font data
	to the adapter.  This is accomplished by putting the font data
        in known spot and notify the adapter when it is there.  The
	adapter is responsible for transferring (via DMA) the font data
        as it is required.  As it turns out, the "known spot" will
        be a shared memory segment being created by X server.  When the 
        first graphics process (happens
        to be X server) calls gsc_make_gp we will attach this segment
        to the font kernel process so that it has access (reading)
        to this segment.  This is how the shared memroy segment will be 
        used by X-server and SLIH in order to make fonts accessible for both

	With this background in mind, make_shm and unmake_shm are 
        responsible for attaching and detaching the shared memory
        segment to the font kernel process.

    unmake_shm:                                                 

	Enqueue a command to the font kernel process, fkproc, requesting it to
       detach the shared memory from itself.

   Invoke: fsp_enq 

   Called by: gsc_unmake_gp indirectly (cproc_term calls unmake_shm directly)  

---------------------------------------------------------------------------*/
unmake_shm(vtm_ptr)
struct vtmstruc *vtm_ptr;
{
    int             flag, rc = 0;
    fkprocQE        qe;               /* queue a command element to kproc */

    BUGLPR(db_fontshm, BUGNFO , ("-> entering unmake_shm, vtmstruc ptr=%x\n",vtm_ptr));

    /* request fkproc to detach and release shared memory segment */
    qe.command = FKPROC_COM_DETACH | FKPROC_COM_WAIT;

    if ( (* vtm_ptr->fsp_enq) (&qe, vtm_ptr->display->lftanchor)) 
    {
	    BUGPR(("###### unmake_gp ERROR bad nq \n"));
   	    rc = -1;  
    }

    BUGLPR(db_fontshm, BUGNFO , ("exiting unmake_shm\n"));

    return(rc);
}



/*---------------------------------------------------------------------------
    Make_shm:

 	Enqueue a commond to the font kernel process, fkproc, requesting it to
        attach the shared memory created by X server to itself 

   Invoke: fsp_enq 

   Called by:  gsc_make_gp 

---------------------------------------------------------------------------*/
make_shm(vtm_ptr)
struct vtmstruc *vtm_ptr;
{
    int             flag, rc = 0;
    fkprocQE        qe;               /* queue a command element to kproc */

    BUGLPR(db_fontshm, BUGNFO, ("-> entering make_shm vtmstruc ptr=%x\n",vtm_ptr));

    /* request fkproc to create shared memory segment */

    qe.command = FKPROC_COM_ATTACH | FKPROC_COM_WAIT;

    /* queue function ptr is contained in the virtual terminal structure */
    if (rc = (*vtm_ptr->fsp_enq) (&qe, vtm_ptr->display->lftanchor)) {
	    BUGPR(("###### make_gp ERROR bad nq \n"));
    }
    BUGLPR(db_fontshm, BUGNFO , ("exiting make_shm\n"));
    return (rc);
}

