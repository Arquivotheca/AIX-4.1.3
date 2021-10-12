static char sccsid[] = "@(#)46	1.2.2.5  src/bos/kernext/rcm/rcmcmap.c, rcm, bos411, 9437A411a 9/8/94 17:53:07";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Color Map Mgmt.
 *
 * FUNCTIONS:
 *    create_colormap	     - creates a colormap entry
 *    delete_colormap	     - deletes a colormap entry
 *    update_colormap	     - sets hardware colormap entry to value assigned
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989-1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*#define DBGCMAP
*/

#include <lft.h>                    /* includes for all lft related data */
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/user.h>			/* user structure */
#include <sys/ioacc.h>
#include <sys/lockl.h>
#include <sys/syspest.h>
#include "gscsubr.h"			/* functions */
#include "rcm_mac.h"
#include "xmalloc_trace.h"


BUGVDEF(dbg_rcmcmap,0); 	 /* Set to 1 turns on all prints */


/* ============================================================= */
/* FUNCTION: gsc_create_colormap
*/
/* PURPOSE: creates a colormap entry for a graphics process
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_create_colormap (pd, parg)
	struct phys_displays 	*pd;
	create_colormap 	*parg;
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
	handle to requesting graphics process
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/


int gsc_create_colormap (pd, parg)

struct phys_displays *pd;
create_colormap *parg;
{
   rcmCm *pcm;
   gscDevPtr   pdev;

   gsctrace (CREATE_COLORMAP, PTID_ENTRY);
   /* Allocate colormap entry structure and attach it to the */
   /* list of colormap structures.			     */

   if((pcm = xmalloc (sizeof (struct _rcmCm), 3, pinned_heap)) == NULL) {
       /* Log an error */
       return (ENOMEM);
   }

   pcm->hwd_map = 0;
   BUGLPR(dbg_rcmcmap, 1,
	  ("==== Malloc of cmap successful %x--- CREATE\n", pcm));

   /* return handle */
   if (suword (&parg->cm_handle, pcm)) {
       BUGPR(("###### create_rcx ERROR suword returning handle \n"));
       xmfree ((caddr_t) pcm, pinned_heap);
       return (EFAULT);
   }

   /* Now attach to front of cmap list */

   /* create device pointer */
   SET_PDEV(pd,pdev);

   pcm->nxtCm = pdev->devHead.pCm;
   pdev->devHead.pCm = pcm;

#ifdef DBGCMAP
   pcm = pdev->devHead.pCm;
   while (pcm != NULL)
   {
       BUGLPR(dbg_rcmcmap, 0,
	  ("CREATE cmap entry at %x has hwd id %d\n",pcm,pcm->hwd_map));
       pcm = pcm->nxtCm;
   }
#endif
   gsctrace (CREATE_COLORMAP, PTID_EXIT);
   return  0;
}




/* ============================================================= */
/* FUNCTION: gsc_delete_colormap
*/
/* PURPOSE: deletes a colormap entry for a graphics process
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_delete_colormap (pd, parg)
	struct phys_displays 	*pd;
	delete_colormap 	*parg;
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
	handle to requesting graphics process
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/


int gsc_delete_colormap (pd, parg)

struct phys_displays *pd;
delete_colormap *parg;
{
   delete_colormap arg;
   rcmCm *pcm,*lastcm;
   gscDevPtr   pdev;

   gsctrace (DELETE_COLORMAP, PTID_ENTRY);
   /* This routine must validate handle and then unhook the colormap */
   /* entry from the list */

   /* copy the argument */
   if (copyin (parg, &arg, sizeof (arg))) {
       /* Log an error */
       return (EFAULT);
   }

   BUGLPR(dbg_rcmcmap, 1,
	  ("==== COPIED in argumant rcmhandle is %x--- DELETE\n", arg.cm_handle));

   /* Range check the colormap handle */
   SET_PDEV(pd,pdev);

   lastcm = (rcmCm *) &(pdev->devHead.pCm);
   pcm = pdev->devHead.pCm;
   while (pcm != NULL)
   {
       if (arg.cm_handle == (CM_Handle) pcm)
	   break;
       /* If not equal then step one link in the list */
       lastcm = pcm;
       pcm = pcm->nxtCm;
   }

   BUGLPR(dbg_rcmcmap, 1,
	  ("Out of loop pcm is %x--- DELETE\n", pcm));
   /* If we reach here and pcm is null an invalid handle was passed */
   if (pcm == NULL)
       return (EINVAL);

   /* pcm now points to the colormap to delete so unhook it and free */
   /* the memory						     */

   /* Check to see if this is first in list */
   if (lastcm == (rcmCm *) &(pdev->devHead.pCm))
       pdev->devHead.pCm = pcm->nxtCm;
   else
       lastcm->nxtCm = pcm->nxtCm;

   xmfree((caddr_t) pcm,pinned_heap);

#ifdef DBGCMAP
   pcm = pdev->devHead.pCm;
   while (pcm != NULL)
   {
       BUGLPR(dbg_rcmcmap, 0,
	  ("DELETE - cmap entry at %x has hwd id %d\n",pcm,pcm->hwd_map));
       pcm = pcm->nxtCm;
   }
#endif

   gsctrace (DELETE_COLORMAP, PTID_EXIT);
   return (0);

}



/* ============================================================= */
/* FUNCTION: gsc_update_colormap
*/
/* PURPOSE: updates a colormap entry for a graphics process
*/
/* DESCRIPTION:
*/
/* INVOCATION:
    int gsc_update_colormap (pd, parg)
	struct phys_displays 	*pd;
	update_colormap 	*parg;
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
	handle to requesting graphics process
*/
/* RELATED INFORMATION:
*/
/* NOTES:
*/


int gsc_update_colormap (pd, parg)

struct phys_displays *pd;
update_colormap *parg;
{
   update_colormap arg;
   rcmCm *pcm,*lastcm;
   gscDevPtr   pdev;

   gsctrace (UPDATE_COLORMAP, PTID_ENTRY);
   /* This routine must validate handle and then unhook the colormap */
   /* entry from the list */

   /* copy the argument */
   if (copyin (parg, &arg, sizeof (arg))) {
       /* Log an error */
       return (EFAULT);
   }

   BUGLPR(dbg_rcmcmap, 1,
	  ("==== COPIED in argumant rcmhandle is %x -- UPDATE\n", arg.cm_handle));
   BUGLPR(dbg_rcmcmap, 1,
	  ("new_hwdmap is %x -- UPDATE\n", arg.new_hwdmap));

   /* Range check the colormap handle */
   SET_PDEV(pd,pdev);

   lastcm = (rcmCm *) &(pdev->devHead.pCm);
   pcm = pdev->devHead.pCm;
   while (pcm != NULL)
   {
       if (arg.cm_handle == (CM_Handle) pcm)
	   break;
       /* If not equal then step one link in the list */
       lastcm = pcm;
       pcm = pcm->nxtCm;
   }

   /* If we reach here and pcm is null an invalid handle was passed */
   if (pcm == NULL)
       return (EINVAL);

   /* pcm now points to the colormap to update so plug in new colormap */
   /* id.							       */

   pcm->hwd_map = arg.new_hwdmap;
#ifdef DBGCMAP
   pcm = pdev->devHead.pCm;
   while (pcm != NULL)
   {
       BUGLPR(dbg_rcmcmap, 0,
	  ("UPDATE cmap entry at %x has hwd id %d\n",pcm,pcm->hwd_map));
       pcm = pcm->nxtCm;
   }
#endif
   gsctrace (UPDATE_COLORMAP, PTID_EXIT);
   return(0);

}
