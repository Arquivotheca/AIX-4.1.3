static char sccsid[] = "@(#)35	1.4  src/bos/usr/ccs/lib/libsrc/defssys.c, libsrc, bos411, 9428A410j 2/26/91 14:54:21";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	defssys
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
#include "src.h"
#include "srcobj.h"
int defssys(subsys)
struct SRCsubsys *subsys;
{
	memset((void *)subsys,0,(size_t)sizeof(struct SRCsubsys));
	subsys->priority=20;
	subsys->multi=SRCNO;
	subsys->display=SRCYES;
	subsys->action=ONCE;
	subsys->waittime=TIMELIMIT;
	subsys->contact=SRCSOCKET;
	strcpy(subsys->standin,"/dev/console");
	strcpy(subsys->standout,"/dev/console");
	strcpy(subsys->standerr,"/dev/console");
}
