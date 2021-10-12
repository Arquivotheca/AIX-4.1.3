static char sccsid[] = "@(#)45	1.1  src/bos/usr/lib/methods/common/cfg_lfterr.c, lftdd, bos411, 9428A410j 10/25/93 14:49:35";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: cfg_lfterr
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


/*------------
 * FUNCTIONAL DESCRIPTION:
 *
  GRAPHICS error logging routine.
	struct vtmstruc *vp  - pointer to the vtmstruc structure in vt.h.
	char   *res_name     - Failing sotfware component name. 
	char   *dmodule	     - Detecting module (the module that called the
                              	failing module) 
	char   *fmodule      - Failing module (the module that returned the 
				bad return code)
	int    return_code   - Return code from failing module
	int    err_indicator - Error indicator number from lftras.h
	char   *ras_unique   - Unique code used to identitify specific error 
				locations for error logging. 
  ------------*/

#include <sys/types.h>
#include <sys/errids.h>
#include <sys/syspest.h>
#include <vt.h>
#include "cfgdebug.h"

#ifndef ERRID_GRAPHICS
#define ERRID_GRAPHICS   0xf10d0eff                 /* Temp fix only */
#endif /* ERRID_GRAPHICS */


ERR_REC(256) ER;

int
cfg_lfterr(vp,res_name,dmodule,fmodule,return_code,err_indicator,ras_unique)
struct vtmstruc *vp;
char   *res_name;
char   *dmodule;
char   *fmodule;
int    return_code; 
int    err_indicator; 
char   *ras_unique; 

{
/* For debugging purposes only  */
       DEBUG_0("Entering cfg_lfterr \n");
if( res_name != NULL )
       DEBUG_1("Failing component %s\n",res_name);
if( dmodule != NULL )
       DEBUG_1("Detecting module %s \n",dmodule);
if( fmodule != NULL )
       DEBUG_1("Failing module %s\n",fmodule);
       DEBUG_1("Return code %d\n",return_code);
if( err_indicator != NULL )
       DEBUG_1("Ras error code %d\n",err_indicator);
if( ras_unique != NULL )
       DEBUG_1("Ras_unique %s\n",ras_unique);


ER.error_id = ERRID_GRAPHICS;        		/* Template Id # */

sprintf(ER.resource_name,"%8s",res_name);

sprintf(ER.detail_data,"%8s  %8s  %4d  %4d  %s",
	dmodule,fmodule,return_code,err_indicator,ras_unique);

/* Call system error logging routine */
errlog(&ER, ERR_REC_SIZE + (strlen(ER.detail_data)+1));

/* For debug only */
DEBUG_0("Leaving lfterr \n");

return(0);
}
