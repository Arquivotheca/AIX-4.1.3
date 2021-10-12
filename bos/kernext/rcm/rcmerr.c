static char sccsid[] = "@(#)59  1.1.3.1  src/bos/kernext/rcm/rcmerr.c, rcm, bos41J, 9520A_all 5/3/95 14:02:10";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Error Handler
 *
 * FUNCTIONS: rcmerr
 *
 * ORIGINS: 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*------------
 * FUNCTIONAL DESCRIPTION:
 *
 * RCMERR error logging routine.
  ------------*/

#include <lft.h>                  /* includes for all lft related data */
#include <sys/errids.h>
#include <sys/syspest.h>
#include <rcmras.h>
#include "xmalloc_trace.h"

#ifndef ERRID_RCMERR
#define ERRID_RCMERR   0x8abb68ae                 /* Temp fix only */
#endif /* ERRID_RCMERR */

BUGVDEF(dbg_rcmerr,99);                          /* debug flag for rcm        */

#define REC_ERR _uerr_rec._err_rec        /* Error record structure    */
	                                  /* in errids.h.              */
#ifndef NULL
#define NULL 0
#endif

int
rcmerr(res_name,dmodule,fmodule,return_code,err_indicator,ras_unique)
char   *res_name;       /* Failing sotfware component name. options are: */
	                /* HFT, MDD, TDD, KDD, VDD1, VDD2, VDD3, VDD4, GADD, */
	                /* SDD.  This are defined in RAS.h.                  */

char   *dmodule;        /* Detecting module is the module that called the    */
	                /* failing module Exp: if you are in VTMOUT and call  */
	                /* VTMUPD() and VTMUPD returns a error. VTMOUT is the */
	                /* detecting module and VTMUPD is the failing module. */

char   *fmodule;        /* Failing module is the module that returned the bad */
	                /* return code.                                       */

int    return_code;     /* Return code from failing module */

int    err_indicator;   /* Error indicator number reside in RAS.h.            */

char   *ras_unique;     /* Unique RAS code used to identitify specific error  */
	                /* locations for error logging. Nine RAS unique codes */
	                /* reside in RAS.h.                                   */
{
	union uerr_rec {                /* This is a union of the error */
	         struct err_rec _err_rec;/* record in sys/errids.h, it  */
	         char   tbuf[256];      /* expands the detail data area.*/
	        }_uerr_rec;
	int rindex;                             /* Index for detail_data   */
	                                        /* detail_data with blanks */
	
	/* For debugging purposes only  */
	       BUGLPR(dbg_rcmerr, BUGNFO, ("Entering rcmerr \n"));
	if( res_name != NULL )
	       BUGLPR(dbg_rcmerr, BUGNFO, ("Failing component %s\n",res_name));
	if( dmodule != NULL )
	       BUGLPR(dbg_rcmerr, BUGNFO, ("Detecting module %s \n",dmodule));
	if( fmodule != NULL )
	       BUGLPR(dbg_rcmerr, BUGNFO, ("Failing module %s\n",fmodule));
	       BUGLPR(dbg_rcmerr, BUGNFO, ("Return code %d\n",return_code));
	if( err_indicator != NULL )
	       BUGLPR(dbg_rcmerr, BUGNFO,("Ras err code %d\n",err_indicator));
	if( ras_unique != NULL )
	       BUGLPR(dbg_rcmerr, BUGNFO, ("Ras_unique %s\n",ras_unique));
	
	REC_ERR.error_id = ERRID_RCMERR;/* Template Id #defined in rcmerr.h */
	
	bcopy("        ", REC_ERR.resource_name,8);/* Remove leftover junk */
	bcopy(res_name, REC_ERR.resource_name,strlen(res_name));
	/* Fail SW comp name */
	
	/* Build detail data line */
	sprintf(REC_ERR.detail_data,"%8s  %8s  %4d  %4d  %s",
	        dmodule,fmodule,return_code,err_indicator,ras_unique);
	
	/* Call system error logging routine */
	errsave(&REC_ERR, ERR_REC_SIZE + (strlen(REC_ERR.detail_data)+1));
	
	/* For debug only */
	BUGLPR(dbg_rcmerr, BUGNFO, ("Leaving rcmerr \n"));
	return return_code;
}
