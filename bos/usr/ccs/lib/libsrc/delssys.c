static char sccsid[] = "@(#)38	1.5  src/bos/usr/ccs/lib/libsrc/delssys.c, libsrc, bos411, 9428A410j 2/26/91 14:54:23";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	delssys
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <odmi.h>
#include "src.h"
#include "srcodm.h"
#include "srcaudit.h"
#include "srcobj.h"

int delssys(subsysname)
char *subsysname;	/* subsystem to delete */
{
	int rc;
	char criteria[256];

	/* init the odm environment */
	if(src_odm_init() < 0)
		return(-1);

	sprintf(criteria,"subsysname = '%s'",subsysname);
	rc=odm_rm_obj(SRCSYSTEM,criteria);
	if(rc <= 0)
	{
		src_odm_terminate(FALSE);
		return(rc);
	}
	auditlog(AUDIT_SRC_DELSSYS,0,subsysname,strlen(subsysname));

	/* tell srcmstr that we have killed one of his siblings */
	tellsrc(DELSUBSYS,subsysname);

	/* delete subserver entries */
	odm_rm_obj(SRCSUBSVR,criteria);

	/* delete notify entries */
	sprintf(criteria,"notifyname = '%s'",subsysname);
	odm_rm_obj(SRCNOTIFY,criteria);

	src_odm_terminate(FALSE);
	return(rc);
}
