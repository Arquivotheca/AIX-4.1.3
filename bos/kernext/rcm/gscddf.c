static char sccsid[] = "@(#)20	1.14.2.7  src/bos/kernext/rcm/gscddf.c, rcm, bos411, 9433B411a 8/16/94 18:08:06";

/*
 *   COMPONENT_NAME: (rcm) Rendering Context Manager Dev Dep Functions
 *
 *   FUNCTIONS: dev_dep_fun_service
 *		dev_dep_fun_service_ext
 *		rcm_fun_service_ext
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989-1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                    /* includes for all lft related data */
#include <sys/adspace.h>
#include <sys/dma.h>                    /* direct memory addressing */
#include "rcmras.h"			/* error defines */
#include <sys/sysmacros.h>		/* Macros for MAJOR/MINOR */
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/user.h>			/* user structure */
#include <sys/syspest.h>
#include "rcm_mac.h"			/* macros for rcm */
#include "xmalloc_trace.h"

BUGVDEF(dbg_gscddf, 99);


/*------------
  Perform device dependent services for various graphics adapters.
  This is the device-independent code which calls the display-specific
  code to handle the requested service.
  ------------*/

dev_dep_fun_service(pd, user_ddf_arg)
struct phys_displays *pd;		/* physical display pointer */
struct _dev_dep_fun *user_ddf_arg;	/* argument passed by the user */
{
#	define DDI_LEN	64		/* sizeof stack-based ddi copy */
	int i;
	int rc; 			/* return code */
	char *ddi_ptr = NULL;		/* ddi struct copied to kernel */
	char ddi[DDI_LEN];		/* copy of ddi passed by user */
	struct _dev_dep_fun ddf_arg;	/* copy of arg passed by user */
	struct _rcmProc *pproc; 	/* pointer to rcm process structure */
	gscDev *pdev;			/* pointer to gsc device structure */

	BUGLPR(dbg_gscddf, BUGNFO,
	("entering dev_dep_fun_service(pd = 0x%x, arg = 0x%x)\n",
	pd,user_ddf_arg));
	gsctrace (DEV_DEP_FUN, PTID_ENTRY);

	RCM_ASSERT (pd != NULL, 0, 0, 0, 0, 0);

	SET_PDEV(pd, pdev);
	if (pdev == NULL)
		return (EINVAL);

	FIND_GP(pdev, pproc);
	if (pproc == NULL)
		return (EINVAL);

	RCM_ASSERT (pdev->devHead.display != NULL, 0, 0, 0, 0, 0);

	/*--------
	  Copy user argument structure to kernel space
	  --------*/
	rc = copyin(user_ddf_arg, &ddf_arg, sizeof(struct _dev_dep_fun));

	if (rc != 0) {
		BUGLPR(dbg_gscddf, BUGNTX, ("copyin of ddf_arg failed.\n"));
		rcmerr("GSC", "gscddf", "copyin", rc, 0, UNIQUE_1);
		return EFAULT;
	}

	BUGLDM(dbg_gscddf, BUGNTX, "\nddf_arg structure dump:\n",
	&ddf_arg, sizeof(struct _dev_dep_fun));

	/*--------
	  Copy device-dependent data to kernel space, if present
	  --------*/
	
	if (ddf_arg.ddi != NULL) {
		if (ddf_arg.ddi_len <= 0) {
			BUGLPR(dbg_gscddf, 1,
				("ddi_len=%d.\n",ddf_arg.ddi_len));
			return EINVAL;
		}
		if (ddf_arg.ddi_len <= DDI_LEN)
			ddi_ptr = ddi;
		else
			ddi_ptr = xmalloc(ddf_arg.ddi_len, 0, pinned_heap);
		if (ddi_ptr == NULL) {
			BUGLPR(dbg_gscddf,1,("xmalloc of ddi_ptr failed.\n"));
			rcmerr("GSC", "gscddf", "xmalloc", 0, 0, UNIQUE_2);
			return ENOMEM;
		}
	
		rc = copyin(ddf_arg.ddi, ddi_ptr, ddf_arg.ddi_len);
	
		if (rc != 0) {
			BUGLPR(dbg_gscddf, 1,
				("copyin of ddi_ptr failed.\n"));
			rcmerr("GSC", "gscddf", "copyin", rc, 0, UNIQUE_2);
			if (ddi_ptr != ddi)
				xmfree(ddi_ptr, pinned_heap);
			return EFAULT;
		}
	}

	/*--------
	  Call device driver function to perform the requested service.
	  --------*/

	BUGLPR(dbg_gscddf, BUGNTX,
	("About to call vttddf (desc @ 0x%x) with arguments:\n",
		pdev->devHead.display->vttddf));
	BUGLPR(dbg_gscddf, BUGNTX,
		("(pdev 0x%x,cmd 0x%x,ddi_ptr 0x%x,len 0x%x)\n",
		pdev, ddf_arg.cmd, ddi_ptr, ddf_arg.ddi_len));
	RCM_ASSERT (pdev->devHead.display != NULL, 0, 0, 0, 0, 0);
	RCM_ASSERT (pdev->devHead.display->vttddf != NULL, 0, 0, 0, 0, 0);
	rc = (*pdev->devHead.display->vttddf)(pdev, ddf_arg.cmd,
		ddi_ptr, ddf_arg.ddi_len, ddf_arg.ddi);

	if (ddi_ptr)
		if (ddi_ptr != ddi)
			xmfree(ddi_ptr, pinned_heap);

	gsctrace (DEV_DEP_FUN, PTID_EXIT);
	if (rc != 0)
	{
		BUGLPR(dbg_gscddf, 1,
		("vtt ddf returned error = %d.\n", rc));
		return rc;
	}

	BUGLPR(dbg_gscddf, BUGNTX, ("successful call to vttddf.\n"));
	return 0;
}


dev_dep_fun_service_fast (pd, arg, parm1, parm2, parm3, parm4, parm5)
struct phys_displays *pd;		/* physical display pointer */
int  arg;
int  parm1, parm2, parm3, parm4, parm5;
{
	int i;
	int rc; 			/* return code */
	struct _rcmProc *pproc; 	/* pointer to rcm process structure */
	gscDev *pdev;			/* pointer to gsc device structure */

	BUGLPR(dbg_gscddf, BUGNFO,
	    ("entering dev_dep_fun_service_fast (pd = 0x%x, arg = 0x%x)\n",
	    pd,arg));

	gsctrace (GSC_DEVICE_REQ, PTID_ENTRY);

	RCM_ASSERT (pd != NULL, 0, 0, 0, 0, 0);

	SET_PDEV(pd, pdev);
	if (pdev == NULL)
		return (EINVAL);

	FIND_GP(pdev, pproc);
	if (pproc == NULL)
		return (EINVAL);

	RCM_ASSERT (pdev->devHead.display != NULL, 0, 0, 0, 0, 0);

	/*--------
	  Call device driver function to perform the requested service.
	  --------*/

	BUGLPR(dbg_gscddf, BUGNTX,
	("About to call vttddf_fast (desc @ 0x%x) with arguments:\n",
		pdev->devHead.display->vttddf_fast));
	BUGLPR(dbg_gscddf, BUGNTX,
		("(pdev 0x%x arg 0x%x \
		parm1 0x%x parm2 0x%x parm3 0x%x parm4 0x%x parm5 0x%x)\n",
		pdev, arg, parm1, parm2, parm3, parm4, parm5));
	RCM_ASSERT (pdev->devHead.display != NULL, 0, 0, 0, 0, 0);
	RCM_ASSERT (pdev->devHead.display->vttddf_fast != NULL, 0, 0, 0, 0, 0);
	rc = (*pdev->devHead.display->vttddf_fast)(pdev, arg,
					parm1, parm2, parm3, parm4, parm5);

	gsctrace (GSC_DEVICE_REQ, PTID_EXIT);

	BUGLPR(dbg_gscddf, 1, ("vttddf_fast returned = %d.\n", rc));

	return  rc;
}


rcm_fun_service_fast (pd, arg, parm1, parm2, parm3, parm4, parm5)
struct phys_displays *pd;		/* physical display pointer */
int  arg;
int  parm1, parm2, parm3, parm4, parm5;
{
	int i;
	int rc; 			/* return code */
	struct _rcmProc *pproc; 	/* pointer to rcm process structure */
	gscDev *pdev;			/* pointer to gsc device structure */

	BUGLPR(dbg_gscddf, BUGNFO,
	    ("entering rcm_fun_service_fast (pd = 0x%x, arg = 0x%x)\n",
	    pd,arg));

	gsctrace (GSC_RCM_REQ, PTID_ENTRY);

	RCM_ASSERT (pd != NULL, 0, 0, 0, 0, 0);

	SET_PDEV(pd, pdev);
	if (pdev == NULL)
		return (EINVAL);

	FIND_GP(pdev, pproc);
	if (pproc == NULL)
		return (EINVAL);

	switch (arg)
	{
		/*
		 *  Shared memory attachment to permit access from
		 *  interrupt code.
		 */
		case gsc_SHM_INIT:
		    rc = rcm_usr_buffer_init (pproc, parm1, (caddr_t ) parm2,
									parm3);

		    break;

		case gsc_SHM_EXTEND:
		    rc = rcm_usr_buffer_extend (pproc, parm1, (caddr_t) parm2,
									parm3);

		    break;

		case gsc_SHM_DELETE:
		    rc = rcm_usr_buffer_delete_set (pproc, parm1);

		    break;

		default:
		    rc = EINVAL;
	}

	gsctrace (GSC_RCM_REQ, PTID_EXIT);

	BUGLPR(dbg_gscddf, 1, ("rcm_fun_service_fast returned = %d.\n", rc));

	return  rc;
}
