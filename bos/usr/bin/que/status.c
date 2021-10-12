static char sccsid[] = "@(#)44        1.16  src/bos/usr/bin/que/status.c, cmdque, bos411, 9428A410j 11/18/91 10:58:42";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <IN/standard.h>
#include <sys/types.h>
#include <IN/backend.h>
#include "common.h"
#include <sys/access.h>
#include <sys/id.h>
#include <security.h>
#include <ctype.h>
#include <string.h>       /* D41806 */
#include "qadm/qcadm.h"   /* D41806 */

#include "qmain_msg.h"
#define MAXSTR 		10
#define MSGSTR(num,str)	catgets(catd,MS_QMAIN,num,str)
nl_catd	catd;


#define QMSGSOPN	"Error opening %s.  Errno = %d.\n"
#define QMSGSRED	"Error reading %s.  Errno = %d.\n"


/*
 * queueing daemon
 *      routines that manipulate status files
 */

extern int errno;

/*
 * get or make a status file for each device.
 * called on startup only.
 */
initstat(ql)
register struct q *ql;
{       register struct d *dl;

        for ( ; ql; ql = ql->q_next)
          for (dl = ql->q_devlist; dl; dl = dl->d_next)
           {
             /* D41806 - don't create "s.q_name.dummy" files */
             if ( strcmp( dl->d_name, DUMMY ) )
                inits1(dl);
           }
}


/*
 * set up status of device dev.  if stfile already there and status
 * is OFF, set device status to 'd' (down).  else set status
 * to 'u' (up), and write a stfile ourselves that says we're READY.
 */
inits1(dev)
register struct d *dev;
{       register struct q *q = dev->d_q;         /* queue this device. */
	register int status;

	/* if queue turned off, so are all devices */
	if (q->q_up == FALSE)
	{   setstat(stname(dev),OFF);
	    dev->d_up = FALSE;
	    return(0);
	}

	/* if device stfile says off or down, leave in this state */
	status = getstat(stname(dev));
	if (status == OFF)
	{   dev->d_up = FALSE;
	    return(0);
	}

	/* else device is up until someone says otherwise */
	dev->d_up = TRUE;
	setstat(stname(dev),READY);
}




/*
 * set status of stfile name to ststat
 * if name not there, just make one.
 */
setstat(name,ststat)
register char *name;
int ststat;
{       struct stfile stfile;
	register int fd;
	gid_t	effgid;		/* to use for chown() */

        /* D41806 - don't mess with "s.que_name.dummy" files */
        if ( strstr( name, DUMMY ) )
           return;
        /* end D41806 */

	if ((fd = open(name,2)) == -1)
	{
		/* create status file with 0000 permissions here
		 * and then acl_set() the file to owner rw and group rw	
		 */
		if ((fd = creat(name, 0000)) < 0)
			syserr((int)EXITFATAL,MSGSTR(MSGCREA,"Failure to create %s."),name);

		/* get the effective gid to chown() */
		if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
			syserr((int)EXITFATAL,MSGSTR(MSGEGID,"Cannot get effective gid. Errno = %d"),errno);
			
		/* set ownership to administrative user (root) and
		 * effective gid (printq)
		 */
		if (fchown(fd, OWNER_ADMDATA, effgid) == -1)
			syserr((int)EXITFATAL,MSGSTR(MSGCHWN,"Cannot chown %s. Errno = %d"),name,errno);

		/* set permissions to owner/group rw only	*/
		if (acl_fset(fd, R_ACC|W_ACC, R_ACC|W_ACC, NO_ACC) == -1)
			syserr((int)EXITFATAL,MSGSTR(MSGPERM,"Could not set permissions on %s. Errno = %d"),name,errno);

	    	stclean(&stfile,NULL);
	}
	else
	{ 
   	    read(fd,&stfile,sizeof(stfile));
	    lseek(fd,0L,0);
	}
	if (fd == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGSOPN,QMSGSOPN),name,errno);
	if (ststat == OFF)
		stfile.s_jobnum = 0;	/* for qstatus */
	stfile.s_status = ststat;
	write(fd,&stfile,sizeof(stfile));
	close(fd);
}


/*********************************************************
	GET STATUS FROM STATUS FILE NAMED gs_name
*********************************************************/
getstat(gs_name)
register char *gs_name;
{
	struct stfile gs_sblk;

	/*----Get the status record */
	if (!getstatblk(gs_name,&gs_sblk))
		return(0);
	else
		return(gs_sblk.s_status);
}

/***************************************************************
	GET STATUS BLOCK FROM STATUS FILE NAMED gsb_name
***************************************************************/
getstatblk(gsb_name,gsb_sblk)
char *gsb_name;
struct stfile *gsb_sblk;
{
        register int	gsb_fd;
	extern int	errno;

	/*----Try opening the file */
	if ((gsb_fd = open(gsb_name,0)) == -1)
	{
		/*----File does not exist */
		if (errno == ENOENT)
			return(0);

		/*----Some other error (bad) */
		else
			syserr((int)EXITFATAL,MSGSTR(MSGSOPN,QMSGSOPN),gsb_name,errno);
	}

	/*----Read the status block */
	if (read(gsb_fd,gsb_sblk,sizeof(struct stfile)) != sizeof(struct stfile))
		return(0);

	/*----Close status file and return */
	close(gsb_fd);
	return(1);
}
	

/* write out this status structure to the status file of dev */
stput(s,dev)
register struct stfile *s;
register struct d *dev;
{       register int fd;

	if ((fd = open(stname(dev),1)) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGSOPN,QMSGSOPN),stname(dev),errno);
	write(fd,s,sizeof(struct stfile));
	close(fd);
}
