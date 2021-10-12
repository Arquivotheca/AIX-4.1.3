static char sccsid[] = "@(#)79	1.9.1.1  src/bos/usr/ccs/lib/libsrc/chssys.c, libsrc, bos411, 9428A410j 11/9/93 16:25:36";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	chssys,finishupdate
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
#include <odmi.h>
#include "src.h"
#include "srcobj.h"
#include "srcodm.h"
#include "srcaudit.h"


void finishupdate();

static struct SRCsubsys Usubsys;

static char criteria[256];

int chssys(subsysname,subsys)
char *subsysname;
struct SRCsubsys *subsys;
{
	int sverrno;
	int rc;
	void *getrc;

	rc=checkssys(subsys);
	if(rc<0)
		return(rc);

	if(src_odm_init() < 0)
		return(-1);

	/* subsystem must already exist */
	sprintf(criteria,"subsysname = '%s'",subsysname);
	getrc = odm_get_first(SRCSYSTEM,criteria,&Usubsys);
	if(getrc == NULL || getrc == -1)
	{
		src_odm_terminate(FALSE);
		rc = (int)getrc;
		return(odmerrmap(rc,SRC_SSME));
	}

	/* new subsys name must not exist */
	if(strcmp(subsysname,subsys->subsysname) != 0)
	{
		sprintf(criteria,"subsysname = '%s'",subsys->subsysname);
		getrc=odm_get_first(SRCSYSTEM,criteria,(char *)0);
		if((getrc != NULL) && (getrc != -1))
		{
			free((void *)getrc);
			src_odm_terminate(FALSE);
			return(SRC_SUBEXIST);
		}
	}

	/* new synonym entered  must not exist*/
	if(*subsys->synonym != '\0' && 
		strcmp(Usubsys.synonym,subsys->synonym) != 0)
	{
		sprintf(criteria,"synonym = '%s'",subsys->synonym);
		getrc=odm_get_first(SRCSYSTEM,criteria,(char *)0);
		if((getrc != NULL) && (getrc != -1))
		{
			free((void *)getrc);
			src_odm_terminate(FALSE);
			return(SRC_SYNEXIST);
		}
	}

	sprintf(criteria,"subsysname = '%s'",subsysname);
	subsys->auditid=Usubsys.auditid;
	rc=odm_change_obj(SRCSYSTEM,subsys);

	/* we can only update a record if one existed */
	if(rc < 0)
	{
		src_odm_terminate(FALSE);
		return(-1);
	}

	src_odm_auditlog(AUDIT_SRC_CHSSYS,0,Usubsys.subsysname,
	    SRCSYSTEM,subsys,&Usubsys);
	finishupdate(subsysname,subsys);
	src_odm_terminate(FALSE);

	return(0);
}

static struct SRCsubsvr subsvr;
static struct SRCnotify notify;
static struct fieldview subfields[]=
{
	{0,subsvr.subsysname,sizeof(subsvr.subsysname)},
	{0}
};
static struct fieldview notfields[]=
{
	{0,notify.notifyname,sizeof(notify.notifyname)},
	{0}
};
static struct objview notview[]=
{
	{(char *)&notify,notfields}
};
static struct objview subview[]=
{
	{(char *)&subsvr,subfields}
};

void finishupdate(subsysname,subsys)
struct SRCsubsys *subsys;
char *subsysname;
{
	int rc;

	/* delete srcmstr's old version of the subsystem */
	if(tellsrc(DELSUBSYS,subsysname) != -1)
	{
		/* add srcmstr's new version of the subsystsem  but only 
		 * if srcmstr was active at the delete will we send the add
	 	 * this is a timing consideration if srcmstr becomes 
		 * active after we have updated the subsystem and 
		 * after the delete failed srcmstr
	 	 * will have two versions of the subsystem 
		 * in its valid subsystsem table
	 	 */
		if(*subsys->subsysname != '\0' &&
		   strcmp(subsysname,subsys->subsysname) != 0)
			tellsrc(ADDSUBSYS,subsys->subsysname);
		else
			tellsrc(ADDSUBSYS,subsysname);
	}

	/* when contact method is updated to signals we can no longer have
	 * subservers so delete them
	 */
	if(subsys->contact==SRCSIGNAL)
	{
		sprintf(criteria,"subsysname = '%s'",subsysname);
		odm_rm_obj(SRCSUBSVR,criteria);
	}

	/* when we change the name of the subsystem we must also then change
	 * the subssystem name in the subserver classe and the notify class
	 */
	if(*subsys->subsysname != '\0' &&
	   strcmp(subsysname,subsys->subsysname) != 0)
	{
		/* update subserver object class */
		if(subsys->contact!=SRCSIGNAL)
		{
			subfields[0].c_addr=subsys->subsysname;
			sprintf(criteria,"subsysname = '%s'",subsysname);
			update_obj(SRCSUBSVR,subview,criteria);
		}

		notfields[0].c_addr=subsys->subsysname;
		/* update notify object class */
		sprintf(criteria,"notifyname = '%s'",subsysname);
		update_obj(SRCNOTIFY,notview,criteria);
	}
}
