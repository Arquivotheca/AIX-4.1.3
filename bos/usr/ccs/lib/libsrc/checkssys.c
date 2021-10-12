static char sccsid[] = "@(#)59	1.6  src/bos/usr/ccs/lib/libsrc/checkssys.c, libsrc, bos411, 9428A410j 2/26/91 14:54:12";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	ckcontact,checkssys,required
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


#include <sys/signal.h>
#include "src.h"
#include "srcobj.h"

static int required();

int checkssys(subsys)
struct SRCsubsys *subsys;
{
	int rc;

	rc=required(subsys);
	if(rc!=0)
		return(rc);

	rc=ckcontact(subsys);
	if(rc!=0)
		return(rc);
	
	/* check that only the allowed number of characters are entered */
	if(strlen(subsys->subsysname) >= SRCNAMESZ)
		return(SRC_SUBSYS2BIG);
	if(strlen(subsys->synonym) >= SRCNAMESZ)
		return(SRC_SYN2BIG);
	if(strlen(subsys->grpname) >= SRCNAMESZ)
		return(SRC_GRPNAM2BIG);
	if(strlen(subsys->cmdargs) >= SRCPATHSZ)
		return(SRC_CMDARG2BIG);
	if(strlen(subsys->path) >= SRCPATHSZ)
		return(SRC_PATH2BIG);
	if(strlen(subsys->standin) >= SRCPATHSZ)
		return(SRC_STDIN2BIG);
	if(strlen(subsys->standout) >= SRCPATHSZ)
		return(SRC_STDOUT2BIG);
	if(strlen(subsys->standerr) >= SRCPATHSZ)
		return(SRC_STDERR2BIG);

	return(0);
}
static int required(subsys)
struct SRCsubsys *subsys;
{
	if(*subsys->subsysname=='\0')
		return(SRC_NONAME);
	if(*subsys->path=='\0')
		return(SRC_NOPATH);
	return(0);
}

int ckcontact(subsys)
struct SRCsubsys *subsys;
{
	/* contact by IPC queue? */
	if(subsys->contact==SRCIPC)
	{
		subsys->signorm=0;
		subsys->sigforce=0;
	}
	/* contact by signals? */
	else if(subsys->contact==SRCSIGNAL)
	{

		/* is the signorm signal a valid signal? */
		if(sigvec((int)subsys->signorm,NULL,NULL)==-1)
			return(SRC_BADNSIG);
	
		/* is the sigforce signal a valid signal? */
		if(sigvec((int)subsys->sigforce,NULL,NULL)==-1)
			return(SRC_BADFSIG);
		subsys->svrkey=0;
		subsys->svrmtype=0;

	}
	/* contact by sockets? */
	else if(subsys->contact==SRCSOCKET)
	{
		subsys->signorm=0;
		subsys->sigforce=0;
		subsys->svrkey=0;
		subsys->svrmtype=0;
	}
	else
		return(SRC_NOCONTACT);

	/* success */
	return(0);
}
