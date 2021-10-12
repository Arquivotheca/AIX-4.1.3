static char sccsid[] = "@(#)42	1.27.1.9  src/bos/kernext/rcm/rcmwin.c, rcm, bos41J, 9519B_all 5/10/95 16:34:10";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Geometry/Attribute Mgmt.
 *
 * FUNCTIONS:
 *    create_win_geom	- creates a RCM window geometry structure
 *    delete_win_geom	- deletes a RCM window geometry structure
 *    update_win_geom	- updates a RCM window geometry structure
 *    create_win_attr	- creates a RCM window attribute structure
 *    delete_win_attr	- deletes a RCM window attribute structure
 *    update_win_attr	- updates a RCM window attribute structure
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                    /* includes for all lft related data */
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/user.h>			/* user structure */
#include <sys/lockl.h>
#include <sys/sleep.h>
#include <sys/ioacc.h>
#include <sys/syspest.h>
#include <gai/gai.h>
#include "gscsubr.h"			/* functions */
#include "rcm_mac.h"
#include "xmalloc_trace.h"

BUGVDEF(dbg_rcmwin,0);

/******************************************************************/
/******************************************************************
THIS MODULE CONTAINS FUNCTIONS THAT DISABLE AND ENABLE INTERRUPTS
THEREFORE IT MUST BE PINNED (OR SOME OF IT MUST BE)

    The reason the routines disable interrupts is to ensure that
    the fault list and other structures do not get traversed or
    modified when in a unlinked state.

 ******************************************************************/
/******************************************************************/


/* ============================================================= */
/* FUNCTION: gsc_create_win_geom
*/
/* PURPOSE: creates a RCM window geometry structure
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_create_win_geom (pd, parg)
	struct phys_displays 	*pd;
	create_win_geom 	*parg;
*/
/* CALLS:
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	handle to window geometry
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_create_win_geom (pd, parg)

    struct phys_displays	*pd;
    create_win_geom	*parg;
{
    create_win_geom	a;
    rcmProcPtr		pproc;
    int 		i, free;
    rcmWGPtr		pwg;
    ulong		old_int;
    gscDevPtr		pdev;
    rcm_wg_hash_t       *wg_hash ;

    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter create_win_geom\n"));
    gsctrace (CREATE_WIN_GEOM, PTID_ENTRY);


    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### create_win_geom ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== create_win_geom... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### create_win_geom ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== create_win_geom... pWG=0x%x\n", a.pWG));

    /* create wg  */
    if ((pwg = xmalloc (sizeof (struct _rcmWG), 3, pinned_heap)) == NULL) {
	BUGPR(("###### create_win_geom ERROR xmalloc wg\n"));
	return (ENOMEM);
    }
    /* copy in the caller's wg */
    if (copyin (a.pWG, &pwg->wg, sizeof (struct _gWinGeomAttributes))) {
	xmfree ((caddr_t) pwg, pinned_heap);
	BUGPR(("###### create_win_geom ERROR copyin wg \n"));
	return (EFAULT);
    }

    RCM_TRACE(0x400,getpid(),pwg,0);

    /* initialize wg */
    pwg->pHead = NULL;
    pwg->pProc = pproc;
    pwg->pPriv = NULL;
    pwg->flags = 0;
    pwg->wg.cm_handle = NULL;
    pwg->pLastWA = NULL;

    /* return handle */
    if (suword (&parg->wg, pwg)) {
	BUGPR(("###### create_win_geom ERROR suword returning wg handle \n"));
	xmfree ((caddr_t) pwg, pinned_heap);
	return (EFAULT);
    }
	
    free = FREE_WG_WG;

    /* if needed, create region/mask and copy, then link */
    if (i = get_region (&pwg->wg.pClip)) {
	BUGPR(("###### create_win_geom ERROR get_region, return=%d \n", i));
	free_win_geom (pwg, free);
	return (i);
    }

    free |= FREE_WG_REGION;

    /*
     *  Since older gai software does not fill in visibilityList (or
     *  even some of the other things in the structure), we cannot
     *  trust this pointer.  This can be rectified if "version" info
     *  can be passed in in some compatible way to indicate which parts
     *  of the structure are filled in.  Right now, we just NULL out
     *  this pointer.  The other indeterminate data doesn't matter.
     *
     *  wg.visibilityList can be filled in by performing an update on
     *  the window geometry.  The update has a changemask which can be
     *  used to indicate the validity of the structure member.
     */
    pwg->wg.visibilityList = NULL;

    /* call the device specific code to do whatever it wants */
    /************** WARNING ***********************************/
    /* it is assumed that device specific create functions do */
    /* not have to be guarded, because they will not cause a  */
    /* context switch					      */
    /************** WARNING ***********************************/
    RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
    if (a.error = (pd->create_win_geom) (pdev, pwg, &a)) {
        RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
	BUGPR(("###### create_win_geom ERROR dd create_win_geom=%d \n",
	       a.error));
	free_win_geom (pwg, free);
	suword (&parg->error, a.error);
	return (EIO);
    }

    RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );


    /* link wg to dev */
    old_int = i_disable (INTMAX);


    RCM_WG_HASH_TRACE (pdev, pwg, 1) ;


    /*-------------------------------------------------------------------
        Now add the window geometry onto the hash table
     *------------------------------------------------------------------- */

    wg_hash = &( (pdev->devHead.wg_hash_table->entry[RCM_WG_HASH(pwg)]) ) ;


    /*----------------------------------------
     Add current pWG to hash table linked-list.
    ----------------------------------------*/

    pwg->pNext = (rcmWGPtr) wg_hash->pWG ;
#ifdef WG_DEBUG
    if (wg_hash->pWG != NULL)
    {
        /* brkpoint(pdev, pwg, RCM_WG_HASH(pwg), wg_hash, wg_hash->pWG) ; */
    }
#endif
    wg_hash->pWG = (ulong*) pwg ;

#ifdef WG_DEBUG
    pdev->devHead.window_count ++ ;
#endif

    RCM_WG_HASH_TRACE (pdev, pwg, 1) ;


    i_enable (old_int);


    BUGLPR(dbg_rcmwin,BUGNFO,
           ("==== Exit create_win_geom... pwg=0x%x\n\n", pwg));

    gsctrace (CREATE_WIN_GEOM, PTID_EXIT);
    return (0);
}



/* ============================================================= */
/* FUNCTION: gsc_delete_win_geom
*/
/* PURPOSE: deletes a RCM window geometry structure
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_delete_win_geom (pd, parg)
	struct phys_displays 	*pd;
	delete_win_geom 	*parg;
*/
/* CALLS:
	rcm_delete_win_geom	- to do the real work
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	Unless the hardware is locked by the caller, anomolous
	results may occur because rcx switching may occur

	To prevent some other gp from switching to a rcx using
	the wg being updated, the hardware should be locked.
	The function rcm locks the device to prevent another gp
	from using it until the update finished.

	THIS MAY BE AN INDICATION THAT NEED FINER GRANULARITY
	OF LOCKING.
*/

int gsc_delete_win_geom (pd, parg)

    struct phys_displays	*pd;
    delete_win_geom	*parg;

{
    delete_win_geom	a;
    rcmProcPtr		pproc;
    rcmWGPtr		pwg;
    int 		errno;
    gscDevPtr		pdev;
    int			old_int;

    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter delete_win_geom\n"));
    gsctrace (DELETE_WIN_GEOM, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### delete_win_geom ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== delete_win_geom... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### delete_win_geom ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== delete_win_geom... wg=0x%x\n", a.wg));

    /* rcm lock device so that the list of rcx using the wg can't change */
    rcm_lock_pdev (pdev, pproc, 0);

    errno = 0;

    /* find the window geom, note that a null window geom is NOT ok */
    FIND_WG(pdev,a.wg,pwg);
    if ((a.wg == NULL) || (pwg != (struct _rcmWG *) a.wg))
    {
#ifdef  WG_DEBUG
        printf ("\n *****   Delete WG:  WG = 0x%8X not found !! \n", a.wg) ;
        brkpoint (pdev, pdev->devHead.wg_hash_table, pwg, a.wg) ;
#endif
        errno = EINVAL;
    }

    else
        BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== delete_win_geom... found pwg=0x%x\n", pwg));

    /* if wg owned by the calling process */
    if (!errno && pwg->pProc == pproc)
    {
	/* unlink the wg, fix rcx references and delete the window geometry */
	RCM_TRACE(0x410,getpid(),pwg,0);

	if (pwg->pHead)			/* if any rcx bound to it */
	    pwg->flags |= WG_DELETED;	/* just set the flag and leave it */
	else
	{
	    errno = rcm_delete_win_geom (pdev, pwg, &a.error);
	    if (errno)
	    {
		BUGPR(("###### delete_win_geom ERROR delete_wg = %d \n",
		       a.error));
		suword (&parg->error, a.error);
	    }
	}
    }
    else if (!errno)		/* else => errno || pwg->pProc != pproc */
	errno = EINVAL;		/* we didn't own it */

    /* rcm unlock device */
    rcm_unlock_pdev (pdev, pproc, 0);

    BUGLPR(dbg_rcmwin,BUGNFO, ("==== Exit delete_win_geom...errno %d \n\n",
			errno));

    gsctrace (DELETE_WIN_GEOM, PTID_EXIT);
    return (errno);
}


/* ============================================================= */
/* FUNCTION: gsc_update_win_geom
*/
/* PURPOSE: updates a RCM window geometry structure
*/
/* DESCRIPTION:
	It replaces the window geometry description with that
	passed in.
*/
/* INVOCATION:
    int gsc_update_win_geom (pd, parg)
	struct phys_displays 	*pd;
	update_win_geom 	*parg;
*/
/* CALLS:
	device specific update_win_geom
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	To prevent some other gp from switching to a rcx using
	the wg being updated, the hardware should be locked.
	The function rcm locks the device to prevent another gp
	from using it until the update finished.

	THIS MAY BE AN INDICATION THAT NEED FINER GRANULARITY
	OF LOCKING (e.g. on a window geometry basis)
*/

int gsc_update_win_geom (pd, parg)

    struct phys_displays	*pd;
    update_win_geom	*parg;
{
    update_win_geom	a;
    rcmProcPtr		pproc;
    ulong		old_int;
    int 		i, free;
    rcmWGPtr		pwg, pwg_new;
    rcmWAPtr		pwa;
    gscDevPtr		pdev;
    rcxPtr		prcx;
    rcmCm		*pcm;

    BUGLPR(dbg_rcmwin,BUGNFO,("==== Enter update_win_geom\n"));
    gsctrace (UPDATE_WIN_GEOM, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### update_win_geom ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== update_win_geom... found pproc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### update_win_geom ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== update_win_geom... wg=0x%x, pWG=0x%x, changes=0x%x\n",
	    a.wg, a.pWG, a.changes));

    /* create wg to hold user parms  */
    if ((pwg_new = xmalloc (sizeof (struct _rcmWG), 3, pinned_heap)) == NULL) {
	BUGPR(("###### update_win_geom ERROR xmalloc for ga\n"));
	return (ENOMEM);
    }
    bzero(pwg_new, sizeof (struct _rcmWG));
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== update_win_geom... pwg_new=0x%x\n", pwg_new));

    /* copy in the caller's ga */
    if (copyin (a.pWG, &pwg_new->wg, sizeof (struct _gWinGeomAttributes))) {
	BUGPR(("###### update_win_geom ERROR copyin of ga \n"));
	xmfree ((caddr_t) pwg_new, pinned_heap);
	return (EFAULT);
    }

    free = FREE_WG_WG;

    /* if change region, get new region */
    if ((i = get_region (&pwg_new->wg.pClip))) {
	  BUGPR(("###### update_win_geom ERROR from get_region=%d \n", i));
	  free_win_geom (pwg_new, free);
	  return (i);
    }
    free |= FREE_WG_REGION;

    /* if change region, get new region */
    if (a.changes & gCWvisibility) {
      if ((i = get_region (&pwg_new->wg.visibilityList))) {
	  BUGPR(("###### update_win_geom ERROR from get_region=%d \n", i));
	  free_win_geom (pwg_new, free);
	  return (i);
      }
      free |= FREE_WG_VIS;
    }

    /* rcm lock device to prevent relationships from changing */
    rcm_lock_pdev(pdev, pproc, 0);

    /* check parms */
    FIND_WG(pdev,a.wg,pwg);
    if (pwg == NULL) {
	BUGPR(("###### update_win_geom ERROR no find wg handle\n"));

        BUGLPR(dbg_rcmwin, 0,
                ("\n *****   Update WG:  WG = 0x%8X not found !! \n", a.wg)) ;
#ifdef WG_DEBUG
        printf
                ("\n *****   Update WG:  WG = 0x%8X not found !! \n", a.wg) ;
        brkpoint (pdev, pdev->devHead.wg_hash_table, pwg) ;
#endif

	/* rcm unlock device */
	rcm_unlock_pdev (pdev, pproc, 0);

	free_win_geom (pwg_new, free);

	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,("====== update_win_geom... found pwg=0x%x\n",
			      pwg));
    if (pwg->pProc != pproc) {
	/* rcm unlock device */
	rcm_unlock_pdev (pdev, pproc, 0);
	free_win_geom (pwg_new, free);
	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,("====== update_win_geom... owned by process\n"));

    /*
     *  Find the color map handle on the device color map list.
     */
    pcm = pdev->devHead.pCm;
    while (pcm != NULL)
    {
	if (pwg_new->wg.cm_handle  == (CM_Handle) pcm)
	  break;
	/* If not equal then step one link in the list */
	pcm = pcm->nxtCm;
    }

    /*
     *  If we reach here and pcm is null, an invalid or NULL handle was passed;
     *  or, there were no color maps on the device chain.  Force colormap
     *  on new geometry to NULL and proceed.
     */
    if (pcm == NULL)
    {
#ifdef RCMDEBUG
	if (pwg_new->wg.cm_handle != NULL)
	  printf (
		  "gsc_update_win_geom:  Invalid color map handle 0x%x from X\n",
		  pwg_new->wg.cm_handle);
#endif
        pwg_new->wg.cm_handle = NULL;
    }

    RCM_TRACE(0x420,getpid(),pwg,0);

    /* give the device specific code a chance to do something
       about the change for every rcx using this wg */
#ifdef RCMDEBUG
	if (a.changes == 0)
	    printf (
		"gsc_update_win_geom:  Zero changemask\n");
#endif
    /*
     *  If the geometry is not bound to any context, call the DD level
     *  with NULL context pointer.  If the geometry is bound to any
     *  context(s), call the DD level with EACH context pointer bound
     *  to the geometry.
     *
     *  In either case the 'new' geometry structure is the one given
     *  by the user, and the 'old' one is pwg.  It is true that the
     *  bound case shows the old geometry as prcx->pWG.  But that is
     *  just pwg, since prcx is bound to pwg or it wouldn't be on the
     *  pwg->pHead context list.
     */
    if(!pwg->pHead) {
        RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
	a.error = (pd->update_win_geom)
		(pdev, NULL, pwg_new, a.changes, pwg);
        RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
	if (a.error) {
	    BUGPR(("###### update_win_geom ERROR dd update_win_geom=%d \n",
		   a.error));
	    suword (&parg->error, a.error);
	    /* rcm unlock device */
    	    rcm_unlock_pdev (pdev, pproc, 0);
	    free_win_geom (pwg_new, free);
	    return (EIO);
	}
    } else {
	    for (prcx = pwg->pHead; prcx != NULL; prcx = prcx->pLinkWG) {
		GUARD_DOM (pproc->pDomainCur[0]->pDomain, pproc, 0, GUARD_ONLY);
    		RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
		a.error = (pd->update_win_geom)
			(pdev, prcx, pwg_new, a.changes, prcx->pWG);
    		RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
		UNGUARD_DOM (pproc->pDomainCur[0]->pDomain, UNGUARD_ONLY);
		if (a.error) {
		    BUGPR(("#### update_win_geom ERR dd update_win_geom=%d\n",
			   a.error));
		    suword (&parg->error, a.error);
	    	    /* rcm unlock device */
    	    	    rcm_unlock_pdev (pdev, pproc, 0);
		    free_win_geom (pwg_new, free);
		    return (EIO);
		}
	    }
    }

    /* change origin */
    BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change origin, x=%d, y=%d\n",
	      pwg_new->wg.winOrg.x, pwg_new->wg.winOrg.y));
    pwg->wg.winOrg = pwg_new->wg.winOrg;

    /* change extent */
    BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change width, w=%d\n",
	      pwg_new->wg.width));
    pwg->wg.width = pwg_new->wg.width;

    BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change height, h=%d\n",
	      pwg_new->wg.height));
    pwg->wg.height = pwg_new->wg.height;

    /* change colormap handle */
    BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change cmap, new=%x wg=%x\n",
	      pwg_new->wg.cm_handle,pwg ));
    pwg->wg.cm_handle = pwg_new->wg.cm_handle;

    /* change clip region */
    BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change clip region\n"));
    /* free old region */
    if (pwg->wg.pClip != NULL) free_win_geom (pwg, FREE_WG_REGION);
    /* link new region */
    pwg->wg.pClip = pwg_new->wg.pClip;

    free &= ~FREE_WG_REGION;

    /* change visibility region */
    if (a.changes & gCWvisibility)
    {
      BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change visibility region\n"));
      /* free old region */
      if (pwg->wg.visibilityList != NULL) free_win_geom (pwg, FREE_WG_VIS);
      /* link new region */
      pwg->wg.visibilityList = pwg_new->wg.visibilityList;

      free &= ~FREE_WG_VIS;
    }

    if (a.changes & gCWdepth)
    {
      BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change depth, d=%d\n",
	      pwg_new->wg.depth));
      pwg->wg.depth = pwg_new->wg.depth;
    }

    /* change color class */
    if (a.changes & gCWcolorClass)
    {
      BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change color class, cc=%d\n",
	      pwg_new->wg.colorClass));
      pwg->wg.colorClass = pwg_new->wg.colorClass;
    }

    /* change layer */
    if (a.changes & gCWlayer)
    {
      BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change layer, l=%d\n",
	      pwg_new->wg.layer));
      pwg->wg.layer = pwg_new->wg.layer;
    }

    /* change color map ID */
    if (a.changes & gCWcmapID)
    {
      BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change color map ID, c=%d\n",
	      pwg_new->wg.cmapID));
      pwg->wg.cmapID = pwg_new->wg.cmapID;
    }

    /* change CurrDispBuff (getImage) user space pointer */
    if (a.changes & gCWcurrDispBuff)
    {
      BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change CurrDispBuff, p=%d\n",
	      pwg_new->wg.pCurrDispBuff));
      pwg->wg.pCurrDispBuff = pwg_new->wg.pCurrDispBuff;
    }

    /* change transparent flag */
    if (a.changes & gCWtransparent)
    {
      BUGLPR(dbg_rcmwin,BUGACT,
	     ("====== update_win_geom... change transparent flag, t=%d\n",
	      pwg_new->wg.transparent));
      pwg->wg.transparent = pwg_new->wg.transparent;
    }

    /* rcm unlock device */
    rcm_unlock_pdev (pdev, pproc, 0);

    /* clean up */
    free_win_geom (pwg_new, free);

    BUGLPR(dbg_rcmwin,BUGNFO, ("==== Exit update_win_geom...\n\n"));

    gsctrace (UPDATE_WIN_GEOM, PTID_EXIT);
    return (0);
}


/* ============================================================= */
/* FUNCTION: gsc_create_win_attr
*/
/* PURPOSE: creates a RCM window attribute structure
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_create_win_attr (pd, parg)
	struct phys_displays 	*pd;
	create_win_attr 	*parg;
*/
/* CALLS:
	vddcreate_win_attr - to do device dependent stuff
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	handle to window attribute
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	The function must rcm lock the device if a wg is
	associated to prevent it from changing while using
	it.
*/

int gsc_create_win_attr (pd, parg)

    struct phys_displays	*pd;
    create_win_attr	*parg;
{
    create_win_attr	a;
    rcmProcPtr		pproc;
    int 		i, free;
    rcmWAPtr		pwa;
    rcmWGPtr		pwg;
    gscDevPtr		pdev;

    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter create_win_attr\n"));
    gsctrace (CREATE_WIN_ATTR, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### create_win_attr ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== create_win_attr... found pproc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### create_win_attr ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== create_win_attr... pWA=0x%x\n", a.pWA));

    /* alloc a wa */
    if ((pwa = xmalloc (sizeof (struct _rcmWA), 3, pinned_heap)) == NULL) {
	BUGPR(("###### create_win_attr ERROR malloc wa\n"));
	return (ENOMEM);
    }
    /* copy in the caller's wa */
    if (copyin (a.pWA, &pwa->wa, sizeof (struct _gWindowAttributes))) {
	BUGPR(("###### create_win_attr ERROR copyin caller wa \n"));
	xmfree ((caddr_t) pwa, pinned_heap);
	return (EFAULT);
    }
    /* initialize wa */
    pwa->pHead = NULL;
    pwa->pPriv = NULL;
    pwa->flags = 0;

    /* return handle */
    if (suword (&parg->wa, pwa)) {
	BUGPR(("###### create_win_attr ERROR suword on handle \n"));
	xmfree ((caddr_t) pwa, pinned_heap);
	return (EFAULT);
    }

    RCM_TRACE(0x430,getpid(),pwa,0);

    free = FREE_WA_WA;

    /* if needed, alloc a region/mask and copy and link */
    if (i = get_region (&pwa->wa.pRegion)) {
	BUGPR(("###### create_win_attr ERROR from get_region=%d \n", i));
	free_win_attr (pwa, free);
	return (i);
    }

    free |= FREE_WA_REGION;

    if (i = get_pixmap (&pwa->wa.pMask)) {
	BUGPR(("###### create_win_attr ERROR from get_pixmap=%d \n", i));
	free_win_attr (pwa, free);
	return (i);
    }

    free |= FREE_WA_PIXMAP;

    /* call the device specific code to do whatever it wants */
    /************** WARNING ***********************************/
    /* it is assumed that device specific create functions do */
    /* not have to be guarded, because they will not cause a  */
    /* context switch					      */
    /************** WARNING ***********************************/
    RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
    if (a.error = (pd->create_win_attr) (pdev, pwa, &a)) {
        RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
	BUGPR(("###### create_win_attr ERROR dd create_win_attr=%d \n",
	       a.error));
	suword (&parg->error, a.error);
	free_win_attr (pwa, free);
	return (EIO);
    }
    RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );

    /* link the wa to list for gp */
    pwa->pNext = pproc->procHead.pWA;
    pproc->procHead.pWA = pwa;

    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("==== Exit create_win_attr... pwa=0x%x\n\n", pwa));

    gsctrace (CREATE_WIN_ATTR, PTID_EXIT);
    return(0);
}


/* ============================================================= */
/* FUNCTION: gsc_delete_win_attr
*/
/* PURPOSE: deletes a RCM window attribute structure
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_delete_win_attr (pd, parg)
	struct phys_displays  *pd;
	delete_win_attr  *parg;
*/
/* CALLS:
	rcm_delete_win_attr - to do real work
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_delete_win_attr (pd, parg)
    struct phys_displays  *pd;
    delete_win_attr  *parg;

{
    delete_win_attr	a;
    rcmProcPtr		pproc;
    rcmWAPtr		pwa;
    int 		errno;
    gscDevPtr		pdev;

    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter delete_win_attr\n"));
    gsctrace (DELETE_WIN_ATTR, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### delete_win_attr ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== delete_win_attr... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### delete_win_attr ERROR copyin arg \n"));
	return (EFAULT);
    }
    BUGLPR(dbg_rcmwin,BUGNFO,("====== delete_win_attr... wa=0x%x\n", a.wa));

    /* find the window attr, note that a null window attr is NOT ok */
    FIND_WA(pproc,a.wa,pwa);
    if ((pwa != (struct _rcmWA *) a.wa) || (a.wa == NULL)) return (EINVAL);
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== delete_win_attr... found pwa=0x%x\n", pwa));

    RCM_TRACE(0x440,getpid(),pwa,0);

    /* if wa not bound to any rcx, delete */
    if (pwa->pHead == NULL) {
	/* unlink wa, fix rcx references to the wa and free the wa */
	if ((errno = rcm_delete_win_attr (pdev, pproc, pwa, &a.error)) == EIO) {
	    BUGPR(("###### delete_win_attr ERROR delete_wa = %d \n", a.error));
	    suword (&parg->error, a.error);
	}
    } else { /* for now can't do anything */
	pwa->flags |= WA_DELETED;
	errno = 0;
    }

    BUGLPR(dbg_rcmwin,BUGNFO, ("==== Exit delete_win_attr...errno %d\n\n",
				errno));

    gsctrace (DELETE_WIN_ATTR, PTID_EXIT);
    return (errno);
}


/* ============================================================= */
/* FUNCTION: gsc_update_win_attr
*/
/* PURPOSE: updates a RCM window attribute structure
*/
/* DESCRIPTION:
	It replaces the window attribute description with that
	passed in.
*/
/* INVOCATION:
    int gsc_update_win_attr (pd, parg)
	struct phys_displays 	*pd;
	update_win_attr 	*parg;
*/
/* CALLS:
	vddupdate_win_attr - to do device dependent stuff
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int gsc_update_win_attr (pd, parg)

    struct phys_displays	*pd;
    update_win_attr	*parg;
{
    update_win_attr	a;
    rcmProcPtr		pproc;
    int 		i, free;
    rcmWGPtr		pwg;
    rcmWAPtr		pwa, pwap;
    gscDevPtr		pdev;
    rcxPtr		prcx;

    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter update_win_attr\n"));
    gsctrace (UPDATE_WIN_ATTR, PTID_ENTRY);

    /* create device pointer */
    SET_PDEV(pd,pdev);

    /* check if caller is gp, return if not */
    FIND_GP(pdev,pproc);
    if (pproc == NULL) {
	BUGPR(("###### update_win_attr ERROR not a gp \n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== update_win_attr... found proc=0x%x\n", pproc));

    /* copy the argument */
    if (copyin (parg, &a, sizeof (a))) {
	BUGPR(("###### update_win_attr ERROR copyin arg \n"));
	return (EFAULT);
    }

    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== update_win_attr... wa=0x%x, pWA=0x%x, changes=0x%x\n",
	    a.wa, a.pWA, a.changes));

    /* check parms */
    FIND_WA(pproc,a.wa,pwa);
    if (pwa == NULL) {
	BUGPR(("###### update_win_attr ERROR no find wa handle\n"));
	return (EINVAL);
    }
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== update_win_attr... found pwa=0x%x\n", pwa));

    /* create wa to hold user parms  */
    if ((pwap = xmalloc (sizeof (struct _rcmWA), 3, pinned_heap)) == NULL) {
	BUGPR(("###### update_win_attr ERROR xmalloc wa parm\n"));
	return (ENOMEM);
    }
    bzero(pwap, sizeof (struct _rcmWA));
    BUGLPR(dbg_rcmwin,BUGACT,("====== update_win_attr... pwap=0x%x\n", pwap));

    /* copy in the caller's win attr */
    if (copyin (a.pWA, &pwap->wa, sizeof (struct _gWindowAttributes))) {
	BUGPR(("###### update_win_attr ERROR copyin user wa \n"));
	xmfree ((caddr_t) pwap, pinned_heap);
	return (EFAULT);
    }

    free = FREE_WA_WA;

    /* if need region, get new region */
    if (i = get_region (&pwap->wa.pRegion)) {
	BUGPR(("###### update_win_attr ERROR get_region=%d \n", i));
	free_win_attr (pwap, free);
	return (i);
    }

    free |= FREE_WA_REGION;

    /* if need pixmap, get new pixmap */
    if ((i = get_pixmap (&pwap->wa.pMask))) {
	BUGPR(("###### update_win_attr ERROR get_pixmap=%d \n", i));
	free_win_attr (pwap, free);
	return (i);
    }

    free |= FREE_WA_PIXMAP;

    /* rcm lock device so wg and wg/wa relations can't change */
    rcm_lock_pdev (pdev, pproc, 0);

    RCM_TRACE(0x450,getpid(),pwa,0);

    /* give the device specific code a chance to do something
       about the change for each rcx it is bound to */
#ifdef RCMDEBUG
    if (a.changes == 0)
	printf (
	    "gsc_update_win_attr:  Zero changemask\n");
#endif
    if(!pwa->pHead) {
        RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
	a.error = (pd->update_win_attr)
	    (pdev, NULL, pwap, a.changes);
        RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
	if (a.error) {
	    BUGPR(("###### update_win_geom ERROR dd update_win_attr=%d \n",
		   a.error));
	    suword (&parg->error, a.error);
	    /* rcm unlock device */
    	    rcm_unlock_pdev (pdev, pproc, 0);
	    free_win_attr (pwap, free);
	    return (EIO);
	}
    } else {
	    for (prcx = pwa->pHead; prcx != NULL; prcx = prcx->pLinkWA) {
		GUARD_DOM (prcx->pDomain, pproc, 0, GUARD_ONLY);
    		RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
		a.error = (pd->update_win_attr)
		    (pdev, prcx, pwap, a.changes);
    		RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
		UNGUARD_DOM (prcx->pDomain, UNGUARD_ONLY);
		if (a.error) {
		    BUGPR(("###### update_win_geom ERROR dd update_win_attr=%d \n",
			   a.error));
		    suword (&parg->error, a.error);
		    /* rcm unlock device */
    		    rcm_unlock_pdev (pdev, pproc, 0);
		    free_win_attr (pwap, free);
		    return (EIO);
		}
	    }
    }

    /* change origin */
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== update_win_attr... change origin, x=%d, y=%d\n",
	    pwap->wa.maskOrg.x, pwap->wa.maskOrg.y));
    pwa->wa.maskOrg.x = pwap->wa.maskOrg.x;
    pwa->wa.maskOrg.y = pwap->wa.maskOrg.y;
    pwa->wa.RegOrigin = pwap->wa.RegOrigin;

    /* change region */
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== update_win_attr... change region, old=0x%x, new=0x%x\n",
	    pwa->wa.pRegion, pwap->wa.pRegion));
    /* free old region */
    if (pwa->wa.pRegion != NULL) free_win_attr (pwa, FREE_WA_REGION);
    /* link new region */
    pwa->wa.pRegion = pwap->wa.pRegion;

    free &= ~FREE_WA_REGION;

    /* change pixmap */
    BUGLPR(dbg_rcmwin,BUGACT,
	   ("====== update_win_attr... change pixmap, old=0x%x, new=0x%x\n",
	    pwa->wa.pMask, pwap->wa.pMask));
    /* free old pixmap */
    if (pwa->wa.pMask != NULL) free_win_attr (pwa, FREE_WA_PIXMAP);
    /* link new pixmap */
    pwa->wa.pMask = pwap->wa.pMask;

    free &= ~FREE_WA_PIXMAP;

    /* clean up */
    free_win_attr (pwap, free);

    BUGLPR(dbg_rcmwin,BUGNFO, ("==== Exit update_win_attr...\n\n"));

    /* rcm unlock device */
    rcm_unlock_pdev (pdev, pproc, 0);


    gsctrace (UPDATE_WIN_ATTR, PTID_EXIT);
    return (0);
}


/* =============================================================

		INTERNAL FUNCTIONS

   ============================================================= */


/* ============================================================= */
/* FUNCTION: get_region
*/
/* PURPOSE: gets a clipping region
*/
/* DESCRIPTION:
	allocates region and box structures in kernel space and
	copies in the data from user space
*/
/* INVOCATION:
	get_region (ppReg)
	gRegionPtr *ppReg
*/
/* CALLS:
	NONE
*/
/* DATA:
	No global data
*/
/* RETURNS:
	none-zero if problem with allocation or copy (errno)
*/
/* OUTPUT:
	returns pointer to kernel structures in location of
	pointer to user structures; creates some data structures
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	This function assumes that a region must be described by
	boxes; thus, it requires that a region
	have a box or boxes associated.
*/

int get_region (ppReg)

    gRegionPtr	*ppReg;

{
    int 	i;
    gRegionPtr	pkreg;
    gBoxPtr	pbox;

    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter get_region\n"));
    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== get_region... ppReg=0x%x, pReg=0x%x\n", ppReg, *ppReg));

    /* make sure really need to get a region */
    if (*ppReg == NULL) {
	return (0);
    }

    /* allocate region and copy in user data */
    if ((pkreg = xmalloc (sizeof (struct _gRegion), 3, pinned_heap)) == NULL) {
	BUGPR(("###### get_region ERROR malloc region\n"));
	return (ENOMEM);
    }
    if (copyin (*ppReg, pkreg, sizeof (struct _gRegion))) {
	BUGPR(("###### get_region ERROR copyin region \n"));
	xmfree ((caddr_t) pkreg, pinned_heap);
	return (EFAULT);
    }

    /* if have box description */
    if (pkreg->numBoxes) {
	
	/* allocate box area and copy in user data */
	i = pkreg->numBoxes * sizeof (struct _gBox);
	BUGLPR(dbg_rcmwin,BUGACT,
	       ("====== get_region, pBox=0x%x, numBoxes=%d, size=%d\n",
		pkreg->pBox, pkreg->numBoxes, i));
	if ((pbox = xmalloc (i, 3, pinned_heap)) == NULL) {
	    BUGPR(("###### get_region ERROR malloc box area, size=%d\n", i));
	    xmfree ((caddr_t) pkreg, pinned_heap);
	    return (ENOMEM);
	}
	if (copyin (pkreg->pBox, pbox, i)) {
	    BUGPR(("###### get_region ERROR copyin box \n"));
	    xmfree ((caddr_t) pkreg, pinned_heap);
	    xmfree ((caddr_t) pbox,  pinned_heap);
	    return (EFAULT);
	}
	
	/* link box to region */
	pkreg->pBox = pbox;
    }
    else
	pkreg->pBox = NULL;
	

    /* return region pointer */
    *ppReg = pkreg;

    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("==== Exit get_region... pkreg=0x%x, pbox=0x%x\n\n", pkreg, pbox));

    return (0);
}

/* ============================================================= */
/* FUNCTION: get_pixmap
*/
/* PURPOSE: gets a clipping pixmap
*/
/* DESCRIPTION:
	allocates pixmap and pixmap data structures in kernel space
	and copies in the data from user space
*/
/* INVOCATION:
	get_pixmap (ppPix)
	gPixmapPtr *ppPix
*/
/* CALLS:
	NONE
*/
/* DATA:
	No global data
*/
/* RETURNS:
	returns pointer to kernel structures in location of
	pointer to user structures; creates some data structures
*/
/* OUTPUT:
	returns pointer to kernel structures in location of
	pointer to user structures; creates some data structures
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int get_pixmap (ppPix)

    gPixmapPtr	*ppPix;

{
    int 	i;
    gPixmapPtr	pkpix;
    char	*pdata;

    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter get_pixmap\n"));
    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== get_pixmap... ppPix=0x%x, pPix=0x%x\n", ppPix, *ppPix));

    /* make sure really need to get a region */
    if (*ppPix == NULL) {
	return (0);
    }

    /* allocate pixmap and copy in user data */
    if ((pkpix = malloc (sizeof (struct _gPixmap))) == NULL) {
	BUGPR(("###### get_pixmap ERROR malloc pixmap\n"));
	return (ENOMEM);
    }
    if (copyin (*ppPix, pkpix, sizeof (struct _gPixmap))) {
	BUGPR(("###### get_pixmap ERROR copyin pixmap \n"));
	free ((caddr_t) pkpix);
	return (EFAULT);
    }

    /* allocate data area and copy in user data */
    if (pkpix->pData == NULL) {
	BUGPR(("###### get_pixmap ERROR bad data pointer\n"));
	free ((caddr_t) pkpix);
	return (EINVAL);
    }
    i = (((pkpix->width * pkpix->fmt.info.bitsPerPixel) +
	  pkpix->fmt.info.scanlinePad - 1)
	 / pkpix->fmt.info.scanlinePad) * (pkpix->fmt.info.scanlinePad/8);
    BUGLPR(dbg_rcmwin,BUGACT, ("====== get_pixmap, width=%d, bpp=%d\n",
	    pkpix->width, pkpix->fmt.info.bitsPerPixel));
    BUGLPR(dbg_rcmwin,BUGACT, ("====== get_pixmap, slp=%d, size=%d\n",
	    pkpix->fmt.info.scanlinePad, i));
    if ((pdata = malloc (i)) == NULL) {
	BUGPR(("###### get_pixmap ERROR malloc data area, size=%d\n", i));
	free ((caddr_t) pkpix);
	return (ENOMEM);
    }
    if (copyin (pkpix->pData, pdata, i)) {
	BUGPR(("###### get_pixmap ERROR copyin data \n"));
	free ((caddr_t) pkpix);
	free ((caddr_t) pdata);
	return (EFAULT);
    }

    /* link data to pixmap and return pixmap pointer */
    pkpix->pData = pdata;
    *ppPix = pkpix;

    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("==== Exit get_pixmap... pkpix=0x%x, pdata=0x%x\n\n",
	    pkpix, pdata));

    return (0);
}


/* ============================================================= */
/* FUNCTION: free_win_geom
*/
/* PURPOSE: frees a window geometry structure
*/
/* DESCRIPTION:
	frees window geometry, region, and box structures in
	kernel space; only frees wg if indicated by second
	parameter
*/
/* INVOCATION:
	free_win_geom (pWG, flags)
	rcmWGPtr     pWG;
	int	     flags;
*/
/* CALLS:
	NONE
*/
/* DATA:
	No global data
*/
/* RETURNS:
*/
/* OUTPUT:
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int free_win_geom (pWG, flags)

    rcmWGPtr	pWG;
    int 	flags;

{
    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter free_win_geom\n"));
    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== free_win_geom... pWG=0x%x, pClip=0x%x\n",
	    pWG, pWG->wg.pClip));
    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== free_win_geom... pBox=0x%x, flags=0x%x\n",
	    pWG->wg.pClip->pBox, flags));

    /* if wg has region(s), free it/them */

    if ((flags & FREE_WG_REGION) && (pWG->wg.pClip != NULL)) {

	if (pWG->wg.pClip->pBox != NULL)	/* free box list */
	    xmfree ((caddr_t) pWG->wg.pClip->pBox, pinned_heap);

	xmfree ((caddr_t) pWG->wg.pClip, pinned_heap);	/* free region struct */
    }

    if ((flags & FREE_WG_VIS) && (pWG->wg.visibilityList != NULL)) {

	if (pWG->wg.visibilityList->pBox != NULL)	/* free box list */
	    xmfree ((caddr_t) pWG->wg.visibilityList->pBox, pinned_heap);

	xmfree ((caddr_t) pWG->wg.visibilityList, pinned_heap);	/* free region*/
    }

    /* if flag is set, free wg struct */
    if (flags & FREE_WG_WG)
	xmfree ((caddr_t) pWG, pinned_heap);

    BUGLPR(dbg_rcmwin,BUGNFO, ("==== Exit free_win_geom...\n\n"));

    return (0);
}


/* ============================================================= */
/* FUNCTION: free_win_attr
*/
/* PURPOSE: frees a window attribute structure
*/
/* DESCRIPTION:
	frees window attribute, and region and box, or pixmap and
	data structures in kernel space; only frees as indicated
*/
/* INVOCATION:
	free_win_attr (pWA, flags)
	rcmWAPtr    pWA;
	int	    flags;
*/
/* CALLS:
	NONE
*/
/* DATA:
	No global data
*/
/* RETURNS:
*/
/* OUTPUT:
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int free_win_attr (pWA, flags)

    rcmWAPtr	pWA;
    int 	flags;

{
    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter free_win_attr\n"));
    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== free_win_attr... pWA=0x%x, pMask=0x%x\n",
	    pWA, pWA->wa.pMask));
    BUGLPR(dbg_rcmwin,BUGNFO,
	   ("====== free_win_attr... pData=0x%x, flags=0x%x\n",
	    pWA->wa.pMask->pData, flags));

    RCM_ASSERT( (pWA), 0, 0, 0, 0, 0 );

    /* if wa has region, free it */
    if ((flags & FREE_WA_REGION) && (pWA->wa.pRegion != NULL)) {
	/* free box */
	if(pWA->wa.pRegion->pBox)
		xmfree ((caddr_t) pWA->wa.pRegion->pBox, pinned_heap);
	/* free region */
	xmfree ((caddr_t) pWA->wa.pRegion, pinned_heap);
    }

    /* if wa has pixmap, free it */
    if ((flags & FREE_WA_PIXMAP) && (pWA->wa.pMask != NULL)) {
	/* free data */
	if (pWA->wa.pMask->pData)
		free (pWA->wa.pMask->pData);
	/* free pixmap */
	free ((caddr_t) pWA->wa.pMask);
    }
    /* free wa */
    if (flags & FREE_WA_WA)
    	xmfree ((caddr_t) pWA, pinned_heap);

    BUGLPR(dbg_rcmwin,BUGNFO, ("==== Exit free_win_attr...\n\n"));

    return (0);
}


/* ============================================================= */
/* FUNCTION: rcm_delete_win_geom
*/
/* PURPOSE: deletes a RCM window geometry structure
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int rcm_delete_win_geom (pdev, pwg, perror)
	gscDevPtr	pdev;
	rcmWGPtr	*pwg;
	int		perror;
*/
/* CALLS:
	vdd update_win_geom	- to update device clipping
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
	Assumes the device (or at least the window geometry
	list for the device) is rcm locked.

	Unless the hardware is locked by the caller, anomolous
	results may occur because rcx switching may occur

	To prevent some other gp from switching to a rcx using
	the wg being updated, the hardware should be locked.

*/

int rcm_delete_win_geom (pdev, pwg, perror)

    gscDevPtr	pdev;
    rcmWGPtr	pwg;
    int 	*perror;

{ 
    rcmWGPtr    pwgt, wg_prev ;
    int         old_int; 
    rcm_wg_hash_t       *wg_hash ;

#ifdef WG_DEBUG  /* track window geometry hash depth for debugging */
    ulong depth;
#endif

    RCM_ASSERT( (pdev && pwg), 0, 0, 0, 0, 0 );
    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter delete_wg, pwg=0x%x\n", pwg));

    RCM_ASSERT ((pdev->devHead.flags & DEV_GP_LOCKED), 0, 0, 0, 0, 0);

    /*
     *  It is an error if there are any rendering contexts bound to
     *  this geom.
     */
    if (pwg->pHead != NULL)
	return (EINVAL);

    /* unbind all rcx and give device code a chance to do something */
    RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
    *perror = (pdev->devHead.display->delete_win_geom)
	(pdev, pwg);
    RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
    if (*perror) {
	BUGPR(("###### delete_wg ERROR dd delete_win_geom=%d \n",
	   *perror));
	return (EIO);
    }

    /* ---------------------------------------------------------------------
       prevent intr, since there may be list scanners that don't
       use the structure lock
     * --------------------------------------------------------------------- */

    old_int = i_disable (INTMAX);
	
    DEPTH_INIT(depth);

    /* ---------------------------------------------------------------------
       Unlink the wg from the list for the device.  If WG ptr matches the
       first in the linked list that this WG ptr hashed to, re-initialize
       to the next pointer in the linked list (typical case NULL).  If not
       the first, then search the link list until match is found.
     * --------------------------------------------------------------------- */

    wg_hash = &( (pdev->devHead.wg_hash_table->entry[RCM_WG_HASH(pwg)]) ) ;

    RCM_WG_HASH_TRACE (pdev, pwg, 3) ;

    pwgt = (rcmWGPtr) wg_hash-> pWG ;
    if (pwg == pwgt)  /* first in list */
    {
        wg_hash->pWG = (ulong*) pwg->pNext ;
    }
    else   /* not first in list, search the rest */
    {
        wg_prev = pwgt ;
        pwgt = pwgt->pNext ;
        while (pwgt != NULL)
        {

	    DEPTH_INC(depth);

            BUGLPR(dbg_rcmwin,2,("\n delete WG loop, hash = %x,   WG = 0x%8X",
                                        RCM_WG_HASH(pwg), pwgt)) ;
            if (pwgt == pwg)
            {
                BUGLPR(dbg_rcmwin, 2,
                        ("\n --- found it, prev next = 0x%8X, next = 0x%8X",
                                wg_prev->pNext, wg_prev->pNext->pNext )) ;

                wg_prev->pNext = wg_prev->pNext->pNext;

                BUGLPR(dbg_rcmwin, 2,
                        ("\n ------   new prev next = 0x%8X\n",wg_prev->pNext));
                break;
            }

            wg_prev = pwgt ;
            pwgt = pwgt->pNext ;
        }
    }

    CHECK_FOR_LARGE_DEPTH(pdev, depth);


#   ifdef WG_DEBUG
    pdev->devHead.window_count -- ;
    RCM_WG_HASH_TRACE (pdev, pwg, 3) ;
#   endif

    i_enable (old_int);

    /* free the region/mask, wg */
    free_win_geom (pwg, FREE_WG_WG | FREE_WG_REGION | FREE_WG_VIS);

    BUGLPR(dbg_rcmwin,BUGNFO, ("==== Exit delete_wg... \n\n"));

    return (0);
}


/* ============================================================= */
/* FUNCTION: rcm_delete_win_attr
*/
/* PURPOSE: deletes a RCM window attribute structure
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int rcm_delete_win_attr (pdev, pproc, pwa, perror)
	gscDevPtr	 pdev;
	rcmWAPtr	 pwa;
	int		 *perror;
*/
/* CALLS:
	vdddelete_win_attr - to do device dependent stuff
*/
/* DATA:
	virtual terminal structure and RCM/DDH structure
*/
/* RETURNS:
	0 if successful
	ERRNO if unsuccessful
*/
/* OUTPUT:
	error code
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/

int rcm_delete_win_attr (pdev, pproc, pwa, perror)
    gscDevPtr		pdev;
    rcmProcPtr		pproc;
    rcmWAPtr		pwa;
    int 		*perror;

{
    rcmWAPtr		pwat;

    BUGLPR(dbg_rcmwin,BUGNFO,("\n==== Enter delete_wa\n"));

    /* unbind all rcx and give device code a chance to do something */
    RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
    *perror = (pdev->devHead.display->delete_win_attr)
	(pdev, pwa);
    RCM_ASSERT ( (i_disable (INTBASE) == INTBASE), 0, 0, 0, 0, 0 );
    if (*perror) {
	BUGPR(("###### delete_wa ERROR dd delete_win_attr=%d \n",
	   *perror));
	return (EIO);
    }

    /* unlink the wa from the list for the process */
    pwat = pproc->procHead.pWA;
    if (pwa == pwat) { /* first in list */
	pproc->procHead.pWA = pwa->pNext;
    } else { /* not first in list, search the rest */
	for (; pwat->pNext != NULL; pwat = pwat->pNext)
	    if (pwat->pNext == pwa) break;
	RCM_ASSERT ( (pwat->pNext != NULL), 0, 0, 0, 0, 0 );	 
	pwat->pNext = pwat->pNext->pNext;
    }

    /* free region/mask and wa */
    free_win_attr (pwa, FREE_WA_WA | FREE_WA_REGION | FREE_WA_PIXMAP);

    BUGLPR(dbg_rcmwin,BUGNFO, ("==== Exit delete_wa...\n\n"));

    return (0);
}

