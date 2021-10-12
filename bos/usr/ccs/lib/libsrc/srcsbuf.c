static char sccsid[] = "@(#)24	1.10  src/bos/usr/ccs/lib/libsrc/srcsbuf.c, libsrc, bos411, 9428A410j 2/26/91 14:54:47";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcsbuf
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


/*
** IDENTIFICATION:
**    Name:	srcsbuf
**    Title:	SRC Subsystem/Subserver Status handler
** PURPOSE:
**	Srcsbufs will allocate memory for the status reply from SRC or
**	subsystem send a status request to SRC for short subsystem status
**	or a sends a packet requesting status to the subsystem for
**	all other status
** 
** SYNTAX:
**    srcsbuf(host,type,name,subname,svr_pid,stattype,where,shellptr,cont)
**    Parameters:
**	i char *host - host that we wish status on subsystem/subserver
**	i short type - SUBSYSTEM or subserver code point
**	i char *name - subsystem name status is requested for
**	i char *subname - subserver object status is requested for
**	i int  svr_pid - subsystem pid that status is desired on
**		on long subsystem status/or subserver status
**		needed when multipule instances
**		of the subsystem is running
**	i short stattype - long or short status
**	i int where - are error msgs to be displayed or logged
**	o char *shellptr[1] - return pointer to storage for formated status
**	u int  *cont - continuation indicator
**		when NEWREQUEST will send new request to src
**		otherwise will just get the next responce packet
**		
**
** INPUT/OUTPUT SECTION:
**	UDP Socket
**
** PROCESSING:
**	Send new status request to (SRC/subsystem) when this is a new request.
**	Wait for a return packet from (SRC/subsystem)
**	Format the status packet received.
**
** PROGRAMS/FUNCTIONS CALLED:
**	srcsrqt
**	srcstat
**	srcstattxt
**	srcerr
**
** OTHER:
**	Three responces to contionuation can be returned by a subsystem
**		1. Reply continuation packet, another packet will follow
**		2. End packet, no more packets follow
**	There are three vaild responces from SRC or the subsystem
**		Short error packet (SRC error code)
**		Srvreply error packet (text message included)
**		Srvreply response packet
**
** RETURNS:
**	size of printable text
**	Error code on failure
**
**/

#include "src.h"
#include "src10.h"
/*--------------------------*/
/* External Functions Used  */
/*--------------------------*/
extern int  srcstat();                     /* src status subsys routine*/
extern int  srcsrqt();                     /* src send request         */
extern char *srcstattxt();

int srcsbuf(host, type, name, subname, svr_pid, stattype, where, shellptr, cont)
char *host;
short type;
char *name;
char *subname;
int  svr_pid;
short  stattype;
int where;
char *shellptr[1];
int  *cont;
{
	int rc;                                 /* return code              */
	short  replen;                          /* length of status reply   */
	short  reqlen;                          /* length of request to RTL */
	unsigned osize;                         /* length total text buffer */
	unsigned msgsize;                       /* length total text buffer */

	char *malloc();

	struct statcode *statptr;

	static struct srcrep *reqptr;           /* pointer to request buffer*/
	char *textptr;
	struct subreq  subreq;                  /* subserver request to RTL */

	/* when a new status request arrives (non continued request)
	** we must allocate buffer space for the returned status response
	**/
	if (*cont == NEWREQUEST)
	{
		/* allocate maximum buffer size for a packet */
		reqptr = (struct srcrep *) malloc ((unsigned) SRCPKTMAX);
		if (reqptr  == NULL) {
			if (where == SSHELL)
				srcerr(SRC_BASE, SRC_MMRY, where, 0, 0, 0, 0);
			return(SRC_MMRY);
		}
	}

	replen = (short) SRCPKTMAX;

	/* initialize return packet buffer to zero */
	memset((void *)reqptr, 0, (size_t)SRCPKTMAX);

	/* get short status of for subsystem
	**	allways goes to SRC for processing
	**/
	if (type == SUBSYSTEM && stattype == SHORTSTAT)
	{
		/* send status request to SRC to get status short by:
		**	subsystem name, subsystem pid, group name, all subsys
		**/
		rc = srcstat(host, name, svr_pid, &replen, reqptr, cont);
	}
	else 
	/* status long and subserver status will be passed through SRC
	** to the subsystem, status returns directly from the subsystem
	** with no SRC envolvment after the pass
	*/
	{
		/* tell SRC that this is a status request and what type
		** short or long status 
		**/
		subreq.action=STATUS;

		/* place SUBSYSTEM object id or code point for a subserver
		** so the subsystem knows what the status is for
		**/
		subreq.object=type;
		subreq.parm1=stattype;
		subreq.parm2=replen;

		/* status of a subsystem needs to have objname set to the
		** name of the subsystem
		**/
		if (type == SUBSYSTEM) 
			(void) strcpy((char *) subreq.objname, name);
		else 
		/* status of a subserver needs to have objname set to
		** subserver object or subserver pid (subname is either
		** subserver object or subserver pid)
		**/
			(void) strcpy((char *) subreq.objname, subname);

		/* size of the packet to be send to the subsystem by SRC */
		reqlen = (short) sizeof(struct subreq);

		/* set up our status continuation factor */
		reqptr->srchdr.cont = *cont;

		/* pass request to SRC for forwarding to the subsystem
		**   indicate that the subsystem is not to be started if
		**   the subsystem has not already been started
		*/
		rc = srcsrqt(host,name,svr_pid,reqlen,&subreq,&replen,reqptr,SRCNO,cont);
	}

	/* did we successfuly receive our status packet? */
	if (rc == SRC_OK)
	{
		/* have we finished receiving packets of status? */
		if (*cont == END) {
			/* free space that was allocated for status */
			(void) free ((char *) reqptr);
			/* indicate to caller that nothing is to be printed */
			shellptr[0] = 0;
			return(rc);
		}

		/* remove the srchdr size from the size of the reply length */
		replen=replen-(sizeof(struct srchdr));
	
		/* setup index into arrary of status structures returned by
		**    SRC or the subsystem itself
		**/
		statptr = (struct statcode *) &reqptr->svrreply;

		/* how much mem do we need to format the status response */
		osize = (unsigned) ((replen/sizeof(struct statcode)) * TEXTSIZE);
		/* allocate buffer for formated status text */
		textptr = malloc (osize);
		if (textptr == NULL)
		{
			if (where == SSHELL)
				srcerr(SRC_BASE, SRC_MMRY, where, 0, 0, 0, 0);
			return(SRC_MMRY);
		}

		/* let caller know about the formated text */
		shellptr[0] = (char *) textptr;

		/* format each element in the array that was sent to us */
		for(;osize > (unsigned) 0;osize=osize-TEXTSIZE)
		{
			textptr=textptr+sprintf(textptr,"%s %s %s\n", statptr->objname, statptr->objtext, srcstattxt(statptr->status));
			statptr++;
		}

		/* return size of the formatted buffer */
		msgsize = textptr-shellptr[0];
	}
	else 
	{
		/* error code was retunred to us */
		if (where != SSHELL)
		{
			/* error on status returning from SRC has no error
			** message associated with it
			**/
			if(type == SUBSYSTEM && stattype==SHORTSTAT)
			{
				free((char *)reqptr);
				shellptr[0]=0;
				return(rc);
			}
			/* error message came back from the subsystem */
			memcpy((void *)reqptr,(void *)reqptr->svrreply.rtnmsg,(size_t)sizeof(*reqptr->svrreply.rtnmsg));
			shellptr[0] = (char *) reqptr;
			return(rc);
		}

		srcerr(SUBSYS_BASE, rc, where, whattoken(rc,SRC_UHOST,host,reqptr->svrreply.objname), 0, 0, reqptr->svrreply.rtnmsg);
		free((char *)reqptr);
		shellptr[0]=0;
		return(rc);
	}

	return((int)msgsize);
}
