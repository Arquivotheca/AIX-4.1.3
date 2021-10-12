static char sccsid[] = "@(#)15	1.8  src/bos/usr/ccs/lib/libsrc/readsubsys.c, libsrc, bos411, 9428A410j 11/9/93 16:23:39";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	opensubsys,closeclass,readclass
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
#include "srcobj.h"
#include "src11.h"

static char criteria[256];
static struct SRCsubsys SRCsubsys;

static struct fieldview fvsubsys[]=
{
	{0,SRCsubsys.subsysname,sizeof(struct subsys)},
	{0}
};

static struct objview vsubsys[]=
{
	{(char *)&SRCsubsys,fvsubsys}
};

int opensubsys()
{
	int rc;
	void *addr_rc;

	rc=src_odm_init();
	if(rc<0)
		return(rc);
	addr_rc = odm_open_class(SRCSYSTEM);
	if(addr_rc == -1) {
		return(-1);
	}
	return(1);
}

int closeclass()
{
	int rc;

	rc=odm_close_class(SRCSYSTEM);
	src_odm_terminate(TRUE);
	return(rc);
}

int readclass(typeread,key,buff)
int typeread;
char *key;
char *buff;
{
	int rc;

	fvsubsys[0].c_addr=buff;
	/* do we want to read the next record? */
	if(typeread==SRCNEXT)
	{
		rc=readrec(SRCSYSTEM,vsubsys,0,0);
	        return(rc);
	}
	/* setup to get all? */
	else if(key==(char *) 0)
	{
		rc=readrec(SRCSYSTEM,vsubsys,"",TRUE);
		return(rc);
	}
	/* we want to read a specific subsystem */
	sprintf(criteria,"subsysname = %s",key);
	rc=readrec(SRCSYSTEM,vsubsys,criteria,TRUE);
	return(rc);
}
