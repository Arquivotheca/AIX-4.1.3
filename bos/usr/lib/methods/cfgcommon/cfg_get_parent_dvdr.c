static char sccsid[] = "@(#)06  1.1  src/bos/usr/lib/methods/cfgcommon/cfg_get_parent_dvdr.c, cfgmethods, bos411, 9428A410j 6/28/94 07:10:36";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfg_get_parent_dvdr.c - used to obtain parent's
 *						        driver name
 *
 * FUNCTIONS: get_dvdr_name
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>

#include "cfgcommon.h"


int
get_dvdr_name()

{
	struct PdDv p_pddv;
	char	sstr[128];
	int	rc;

	sprintf(sstr,"uniquetype=%s",pcudv.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstr,&p_pddv);
	if (rc == -1)
		return(E_ODMGET);
	else if (rc == 0)
		return(E_NOPdDv);

	strcpy(dvdr, p_pddv.DvDr);
	return(0);
}
