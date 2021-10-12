static char sccsid[] = "@(#)74	1.19.1.1  src/bos/usr/bin/src/cmds/srcmstr.c, cmdsrc, bos411, 9428A410j 2/2/94 12:36:42";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	main,sinit,dsrcsendpkt,dmsgsnd,dsrcrecvpkt,src_invalid_version,
 *	srcstrtfind,srcfind,srcpassfind,bldsubsystable,addonesubsys,
 *	delvalidsubsys,count_by_group,dostop,isgroup,isall,issubsys,
 *	startsome,addinetsocket,close_src_sockets,term_src_sockets
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


/*
** IDENTIFICATION:
**    Name:	srcmstr
**    Title:	SRC Master Daemon
** PURPOSE:
**	To spawn the SRC Master Daemon to control defined subsystems.
**	The SRC Master Daemon understands the following requests:
**		Start Subststem
**		Stop Subststem
**		Short Status of Subststem(s)
**		Subsystem Requests
**	The following SRC internal requests are also understood:
**		Subsystem Termination
**		ODM updates
** 
** SYNTAX:
**	srcmstr
**
** INPUT/OUTPUT SECTION:
**	ODM Object: SRCsystem
**
** PROCESSING:
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**
** RETURNS:
**
**/

#include "src.h"
#include <netinet/in.h>
#include <sys/ipc.h>
#include <signal.h>                      /* signal set up               */
#include <fcntl.h>                       /* for "open" statement        */

#include "src11.h"                       /* subsys profile              */
#include "src10.h"                       /* RTL request/reply   structur*/

#include "srcmstr.h"
#include "srcsocket.h"
#include "odmi.h"
#include <sys/syslog.h>

/* external function calls */
extern int srcdsrt() ;                /* deamon start routine         */
extern int srcdstp() ;                /* deamon stop  routine         */
extern void dsrcrecvpkt();
extern void exit();
extern void srcalrm(int);         /* alarm signal catcher           */
extern void srcdrqt() ;                /* deamon request routine       */
extern void srcelog();                 /* deamon cleanup routine   */
extern void srchevn(int);         /* child death signal catcher     */
extern void srcsvrs();                 /* deamon status routine        */
extern void srcterm(int);         /* terminate signal catcher       */
static void addinetsocket();
extern int src_invalid_socket_address();

/* internal function calls */
extern int dmsgsnd();
extern int dsrcsendpkt();
extern struct activsvr *srcfind();    /* finds subsys by PID or name  */
extern void bldsubsystable();
extern void delvalidsubsys();
extern void sinit();
extern void startsome();
extern void dostop();

/* SRC stop reply buffer */
static struct srcrep rtlreply;


/*   global variables   */

/* socket to receive requests on */
static struct src_socket src_socket[2];
/* address of our local afunix socket to receive requests on */
struct sockaddr_un src_sock_addr;
/* address to send replies to */
struct sockaddr_un retaddr;

/* indicator whether we have an alarm set or not */
int alarmset=0;
long nextalarm=0;

union ibuf ibuf             ;   /* ipc queue input buffer   */
union obuf obuf             ;   /* ipc queue output buffer  */
struct activsvr *curraste   ;   /* Ptr to current entry           */
struct activsvr *frstaste   ;   /* Ptr to 1st  actve svr tbl entry*/
struct activsvr *lastaste   ;   /* Ptr to last actve svr tbl entry*/

/* extended signals */
struct sigvec vecarray[]=
{
	{(void (*)(int))srcalrm,(int)0,(int)0},	/* alarm clock */
	{(void (*)(int))srchevn,(int)0,(int)0},	/* child death */
	{(void (*)(int))srcterm,(int)0,(int)0},	/* termination signal */
	{SIG_IGN,(int)0,(int)0},
	{SIG_DFL,(int)0,(int)0}
};	
/* note: child death does not use extended signals because not all zombie
** childern are recongnized when the signal handler is log
**/

/* vaild subsystem pointers */
struct validsubsys *frstsubsys;
struct validsubsys *lastsubsys;

/* faterr will log an error msg and exit the program */
#define  faterr(rc,base,err0,str1,str2,str3,str4) \
{\
   if(rc < 0)\
   {\
      openlog("srcmstr",LOG_CONS|LOG_NDELAY|LOG_NOWAIT,LOG_DAEMON); \
      srcerr(base, err0, SDAEMON, str1, str2, str3, str4); \
      closelog(); \
      exit(1); \
   }\
}

main(argc, argv)
int argc;
char *argv[];
{
	int rc;

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* initialize our daemon
	**	1. create orphan child
	**	2. create our communication socket
	**	3. setup signal handlers
	**	4. read our odm database for valid subsystems
	**/
	(void) sinit();

	/* for ever (or until we are terminated which ever comes first)
	** 	get next SRC request packet and process it 
	**/
	for(;;)
	{
		/* receive command request packet */
		dsrcrecvpkt();

		/* Process the command request received. Will be one of:
          	**  start subsys request
          	**  stop subsys request
          	**  status request
          	**  forward subsys request packet
          	**  subsys death packet (internal packet)
          	**  add and delete valid subsystems
          	**/

		switch (ibuf.demnreq.action)
		{
		case START:
			/* start a subsystem */
			startsome();
			break;

		case STOP:
			/* stop a subsystem */

			/* no active subsystems? */
			if(frstaste == NULL)
			{
				shortrep(&retaddr,SRC_SVND);
			}
			else
			/* there are some active subsystem. try to the stop */
				dostop();
			break;

		case STATUS:
			/* subsys status request */
			(void) srcsvrs();
			break;

		case REQUEST:
			/* pass subsystem a request packet */
			(void) srcdrqt();
			break;

		case REQUESTANDSTART:
			/* pid of subsystem was specified so we won't try
			** and start the subsystem if it is inactive
	       		**/
			if(ibuf.demnreq.pid == 0 && srcfind(ibuf.demnreq.subsysname,ibuf.demnreq.pid,frstaste) == NULL)
				srcdsrt(ibuf.demnreq.subsysname,(char*)0,0,"",0,"",RESP_USE);

			/* pass subsystem a request packet */
			(void) srcdrqt();
			break;

		case NEWHVN:
			/* subsystem death packet */
			(void) newhevn(ibuf.hvn.svrpid,ibuf.hvn.stat);
			break;

		case ADDSUBSYS:
			/* new valid subsystem has been added */
			rc=opensubsys();
			logiferr(rc,0,0,SRC_ODMERR,odmerrno);
			rc=addonesubsys(SRCFIRST,ibuf.demnreq.subsysname);
			logiferr(rc,0,0,SRC_ODMERR,odmerrno);
			closeclass();
			break;

		case DELSUBSYS:
			/* valid subsystem has been delete
	       		** however valid subsystem will remain as long as the
	       		** subsystem is active as soon as it dies then it will
	       		** be removed from the valid subsystem table
	       		**/
			{
				struct validsubsys *ptr;

				/* find the subsystem to delete */
				for(ptr=frstsubsys;ptr!=(struct validsubsys *) 0 && (ptr->deleted==(char)TRUE || strcmp(ptr->subsys.subsysname,ibuf.demnreq.subsysname) != 0);ptr=ptr->nsubsys) ;

				/* the subsystem was not found lets not do anything */
				if(ptr == (struct validsubsys *) 0)
					break;

				/* subsystem found markit to be deleted and attempt 
				** to remove it from the active server table 
				**/
				ptr->deleted=(char)TRUE;
				delvalidsubsys(ptr);
			}
			break;

			/* NOTE: a change valid subsystem is implimented
			** by doing a delete subsystem then an add subsystem
	  		**/

		/* start listining for remote requests */
		case ADDINETSOCKET:
			addinetsocket();
			break;

		/* stop listining for remote requests */
		case DELINETSOCKET:
			src_close_socket(&src_socket[1]);
			break;

		default:
			/* invalid command request */
			shortrep(&retaddr,SRC_ICMD);
			break;
		}
	}
}


/* init or back ground daemon */
void sinit()
{
	int   rc;
	char  rmpath[200];

	/* disassociate this process from the original terminal */
	kleenup(0, 0, 0);
	open("/dev/null",O_RDONLY);
	open("/dev/null",O_WRONLY);
	open("/dev/null",O_WRONLY);

	/* Become head of own process group so that SIGHUP from terminal
	** does not kill spawned daemon.
	**/
	(void) setpgrp();

	faterr(lock_srcmstr(0),SRC_BASE,SRC_ACTV,0,0,0,0);

	system(SRC_DESTROY_TMP_SOCKETS);
	umask(07);
	mkdir(SRC_BASE_DIR_AF_UNIX,0770);
	chown(SRC_BASE_DIR_AF_UNIX,0,0);

	bzero(src_socket,sizeof(src_socket));
	srcafunixsockaddr(&src_sock_addr,0);
	src_socket[0].sock_id=src_setup_socket(&src_sock_addr,SOCK_DGRAM,MAXSOCKBUFSIZE,SRCPKTMAX);
	faterr(src_socket[0].sock_id,SRC_BASE,src_socket[0].sock_id,0,0,0,0);
	memcpy(&src_socket[0].sun,&src_sock_addr,sizeof(struct sockaddr_un));
	src_socket[0].open=1;
	lock_srcmstr(1);

	addinetsocket();

	/* init active subsystem table pointers */
	frstaste = (struct activsvr *)0;
	lastaste = (struct activsvr *)0;
	curraste = (struct activsvr *)0;

	/* get the list of valid subsystems */
	bldsubsystable();

	/* catch alarmclock signals (stop cancel actions) */
	sigvec(SIGALRM, &vecarray[0],(struct sigvec *)0);

	/* catch subsystems death (normal and abnormal) */
	sigvec(SIGCLD, &vecarray[1],(struct sigvec *)0);

	/* catch termination signal from real world */
	sigvec(SIGTERM, &vecarray[2], (struct sigvec *)0);

	/* ignore these signals
    	**    interupt
    	**    hangup
    	**    quit
    	**/
	sigvec(SIGINT, &vecarray[3], (struct sigvec *)0);
	sigvec(SIGQUIT, &vecarray[3], (struct sigvec *)0);
	sigvec(SIGHUP, &vecarray[3], (struct sigvec *)0);

	return ;
}

/* only used in dsrcsendpkt */
#define whatsocket(sockaddr) \
	((sockaddr->sun_family == AF_UNIX) ? (src_socket[0].sock_id) : (src_socket[1].sock_id))

/* send packet by socket */
int dsrcsendpkt(send_addr,pkt,pktsz)
char *pkt;
struct sockaddr_un *send_addr;
int pktsz;
{
	int rc;

	/* send packet */
	rc=srcsendpkt(whatsocket(send_addr),pkt,pktsz,0,send_addr,src_what_sockaddr_size(send_addr));

	/* log an error ? */
	logiferr(rc,0,0,SRC_SOCK,errno);
	if(rc<0)
		return(SRC_BADSOCK);
	return(rc);
}
#undef whatsocket

/* send packet to subsystem by message queue */
int dmsgsnd(qid,mptr,size)
char *mptr;
int qid,size;
{
	int rc;

	/* keep sending that message wile we are interupted */
	do {
		/* IPC SEND, no wait on receive */
		rc = msgsnd(qid,mptr,size,IPC_NOWAIT);
	}while (rc < 0 && errno == EINTR);

	/* log error sending message to subsystem */
	if(rc < 0)
	{
		logerr(0,0,SRC_MSGQ,errno);
		return(SRC_MSGQ);
	}
	else
		return(SRC_OK);
}

/* receive src packet request */
void dsrcrecvpkt()
{
	int    rc;
	int	addr_sz;

	/* receive packet */
	do {
		addr_sz=sizeof(retaddr);
		rc=srcrecvpkt(src_local_or_remote(src_socket),&ibuf,sizeof(ibuf),0,&retaddr,&addr_sz,0);
		/* log if there was an error */
		if(rc == -1 && errno != EINTR)
			logerr(0,0,rc,errno);
		
	} while(rc <= 0 || src_invalid_version() || 
		src_invalid_socket_address(&retaddr,ibuf.demnreq.action));

	return;
}
int src_invalid_version()
{

	/* packet received is of a version we know about */
	if(ibuf.demnreq.dversion == SRCMSGBASE)
		return(FALSE);

	/* send version number back to the client */
	bzero(&obuf,sizeof(struct srcdver));
	obuf.srcdver.rtncode= SRC_VERSION;
	obuf.srcdver.dversion= SRCMSGBASE;
	dsrcsendpkt(&retaddr,&obuf,sizeof(struct srcdver));

	return(TRUE);
}

/* srcstrtfind gives an answer to the imponderable question
**	is the requested subsystem already spawned or is the
**	message queue used by the subsystem already in use
**/
int srcstrtfind()
{
	struct activsvr *currsys;
	int found;

	sigblock(BLOCKMASK);
	for(currsys=frstaste,found=FALSE;currsys!=(struct activsvr *)0 && found == FALSE;currsys=currsys->next)
	{
		if(strcmp(curraste->subsys->subsysname,currsys->subsys->subsysname) == 0
		   || (curraste->subsys->contact == SRCIPC
		       && currsys->subsys->svrkey == curraste->subsys->svrkey
		       && (currsys->subsys->multi == SRCOFF || curraste->subsys->multi == SRCOFF)))
			found= TRUE;
	}
	sigsetmask(0);
	return(found);
}

/* srcfind returns a pointer to the first/next active subsystem that matches
** either the subsystem name or pid
**/
struct activsvr *srcfind(subsys,subsys_pid,startsys)
char *subsys;
int subsys_pid;
struct activsvr *startsys;
{
	struct activsvr *currsys;
	int found;

	sigblock(BLOCKMASK);
	for(currsys=startsys,found=FALSE;currsys!=(struct activsvr *)0 && found == FALSE;)
	{
		if(strcmp(subsys,currsys->subsys->subsysname) == 0 || subsys_pid == currsys->svr_pid)
			found= TRUE;
		else
			currsys=currsys->next;
	}
	sigsetmask(0);
	return(currsys);
}

/* srcpassfind finds the proper subsystem instance to pass a packet to */
struct activsvr *srcpassfind(subsys,subsys_pid,startsys)
char *subsys;
int subsys_pid;
struct activsvr *startsys;
{
	struct activsvr *currsys;
	int found;

	sigblock(BLOCKMASK);
	for(currsys=startsys,found=FALSE;currsys!=(struct activsvr *)0 && found == FALSE;)
	{
		if((strcmp(subsys,currsys->subsys->subsysname) == 0 && (subsys_pid == currsys->svr_pid || subsys_pid == 0))
		    || (*subsys=='\0' && subsys_pid == currsys->svr_pid))
			found= TRUE;
		else
			currsys=currsys->next;
	}
	sigsetmask(0);
	return(currsys);
}

/* build the valid subsystem table */
void bldsubsystable()
{
	int rc;
	int typeread;

	frstsubsys=(struct validsubsys *)0;
	lastsubsys=(struct validsubsys *)0;
	typeread=SRCFIRST;

	/* open subsystem object class */
	rc=opensubsys();
	faterr(rc,ODM_BASE,odmerrno,0,0,0,0);

	/* read all subsystem records
	**   build validsubsystem element for each
	**/
	do {
		rc=addonesubsys(typeread,0L);
		typeread=SRCNEXT;
		/* last subsystem alread read ? */
	} while(rc > 0);

	/* close the subsystem object class */
	closeclass();

}

/* add one element to the valid subsystem table */
int addonesubsys(typeread,key)
int typeread;
char *key;
{
	int rc;
	struct validsubsys *currsubsys;

	/* allocate memory for the new element and initialize it */
	currsubsys=(struct validsubsys *)malloc(sizeof(struct validsubsys));
	if(currsubsys==(struct validsubsys *) 0)
	{
		return(SRC_MMRY);
	}
	memset(currsubsys,0,sizeof(struct validsubsys));

	/* get the subsystem information */
	rc=readclass(typeread,key,&currsubsys->subsys);
	/* last subsystem alread read ? */
	if(rc<=0)
	{
		free(currsubsys);
		return(rc);
	}

	/* is this the first subsystem we have added? */
	if(frstsubsys==(struct validsubsys *) 0)
	{
		/* only time first and last equal
		**  prev and next subsystems will still be null
		**/
		lastsubsys=currsubsys;
		frstsubsys=currsubsys;
	}
	else
	{
		/* set pointers on the doubley linked list */
		lastsubsys->nsubsys=currsubsys;
		currsubsys->psubsys=lastsubsys;
		lastsubsys=currsubsys;
	}
	/* success */
	return(1);
}

void delvalidsubsys(ptr)
struct validsubsys *ptr;
{
	/* is this entry to be deleted?
	**    we will free it if it is marked to be deleted and
	**    there are no forked subsubsystems for this subsystem
	**/
	if(!ptr->deleted || ptr->forked)
		return;

	/* correctly assign the pointers around this node to be
	** removed from this doublely linked list
	**/

	if(ptr==frstsubsys)	/* prev */
		frstsubsys=ptr->nsubsys;
	else
		ptr->psubsys->nsubsys=ptr->nsubsys;

	if(ptr==lastsubsys)	/* next */
		lastsubsys=ptr->psubsys;
	else
		ptr->nsubsys->psubsys=ptr->psubsys;

	/* free up that old memory */
	free((char *) ptr);
	return;
}

/* for a start count all the members of a group that exist in the
**   valid subsystem table
*/
int count_by_group(grpname)
char *grpname;
{
	int rc;
	int count;
	struct validsubsys *ptr;

	for(count=0, ptr=frstsubsys; ptr != (struct validsubsys *)0; ptr=ptr->nsubsys)
	{
		if(strcmp(ptr->subsys.grpname,grpname)==0)
			count++;
	}
	return(count);
}

/* stop group or all subsystems */
void dostop()
{
	struct activsvr *ptr;
	int    count;
	int    rc;

	/* count the entries in the active subsys table
	** that are to be stoped
	**	1. by group
	**	2. all
	**	3. subsystem
	**/
	if(strncmp(ibuf.demnreq.subsysname,SRCGROUP,1) == 0 )
	{
		/* group stop */
		for(ptr=frstaste,count=0; ptr!=NULL; ptr=ptr->next)
			if(strcmp(ptr->subsys->grpname,&ibuf.demnreq.subsysname[1]) == 0)
				count++;
	}
	else if(strncmp(ibuf.demnreq.subsysname,SRCALLSUBSYS,1) == 0 )
	{
		/* all stop */
		for(ptr=frstaste,count=0; ptr!=NULL; ptr=ptr->next)
			count++;
	}
	else
	{
		/* one subsystem */
		ptr=srcfind(ibuf.demnreq.subsysname,ibuf.demnreq.pid, frstaste);
		for(count=0; ptr!=NULL;)
		{
			count++;
			ptr=srcfind(ibuf.demnreq.subsysname,ibuf.demnreq.pid, ptr->next);
		}

	}

	if(count > 0)
	{
		shortrep(&retaddr,count);
	}
	else
	{
		shortrep(&retaddr,SRC_SVND);
		return;
	}

	/* stop the subsyss */
	for(ptr=frstaste; ptr!=NULL; ptr=ptr->next)
	{
		if(!isgroup(ptr->subsys->grpname) && !isall() && !issubsys(ptr->subsys->subsysname,ptr->svr_pid))
			continue;
		rc = srcdstp(ptr);

		/* was there an error? or does SRC respond with the stop msg */
		if (rc <= 0)
		{
			strcpy(rtlreply.svrreply.objname,ptr->subsys->subsysname);
			rtlreply.srchdr.dversion=ibuf.demnreq.dversion;
			rtlreply.svrreply.rtncode = (short)rc;
			dsrcsendpkt(&retaddr,&rtlreply,sizeof(rtlreply));
		}
	}
}

/* is a subsystem group the object of a request */
isgroup(grpname)
char *grpname;
{
	if(strncmp(ibuf.demnreq.subsysname,SRCGROUP,1) == 0  && strcmp(grpname,&ibuf.demnreq.subsysname[1]) == 0)
		return(TRUE);
	else
		return(FALSE);
}

/* are all subsystem the object of a request */
isall()
{
	if(strncmp(ibuf.demnreq.subsysname,SRCALLSUBSYS,1) == 0)
		return(TRUE);
	else
		return(FALSE);
}

/* is a particular subsystem the object of a request */
issubsys(subsys,pid)
char *subsys;
int pid;
{
	if(strncmp(ibuf.demnreq.subsysname,SRCGROUP,1) != 0  
	   && strncmp(ibuf.demnreq.subsysname,SRCALLSUBSYS,1) != 0  
	   && (strcmp(subsys,ibuf.demnreq.subsysname) == 0 || (ibuf.demnreq.pid == pid && pid != 0)))
		return(TRUE);
	else
		return(FALSE);
}

#define strtmsg(rc, subsysname)\
{ \
	obuf.strtreply.pid = rc;\
	strcpy(obuf.strtreply.subsysname, subsysname);\
	dsrcsendpkt(&retaddr,&obuf,sizeof(struct strtreply));\
}
void startsome()
{
	int count;
	int rc;
	char *psubsysname;
	char subsysname[SRCNAMESZ];

	/* group start? */
	if(strncmp(ibuf.demnreq.subsysname,SRCGROUP,1) == 0)
	{
		/* count the number of processes to start */
		(void) sigblock(BLOCKMASK);
		count=count_by_group(&ibuf.demnreq.subsysname[1]);
		(void) sigsetmask(0);
		/* must have atleast one or we have an error */
		if(count==0)
		{
			shortrep(&retaddr,SRC_SVND);
			return;
		}
	}
	else
		count=1;

	psubsysname=ibuf.demnreq.subsysname;

	/* send a count back */
	shortrep(&retaddr,count);

	/* start the subsystem(s) */
	for(;count>0;count--)
	{
		/* start subsys      */
		rc=srcdsrt(psubsysname,subsysname,
		    ibuf.start.envlen,
		    ibuf.start.parm+ibuf.start.parmlen,
		    ibuf.start.parmlen,
		    ibuf.start.parm,
		    (unsigned int)ibuf.start.rstrt);
		/* send reply for each subsystem */
		strtmsg(rc,subsysname);
		psubsysname=(char *)0;
	}
	return;
}

static void addinetsocket()
{
	int rc;
	struct sockaddr_in src_sock_addr;

	/* if our inter net socket is already open don't open another */
	if(src_socket[1].open!=0)
		return;

	/* create socket address */
	bzero(&src_sock_addr,sizeof(src_sock_addr));
	src_sock_addr.sin_family=AF_INET;
	rc=srcgetport(&src_sock_addr);
	if(rc < 0)
		return;

	/* setup the SRC socket so we can use it */
	rc=src_setup_socket(&src_sock_addr,SOCK_DGRAM,MAXSOCKBUFSIZE,SRCPKTMAX);
	if(rc < 0)
		return;
	src_socket[1].sock_id=rc;
	src_socket[1].open=1;
}
void close_src_sockets()
{
	if(src_socket[0].open)
		close(src_socket[0].sock_id);
	if(src_socket[1].open)
		close(src_socket[1].sock_id);
	src_socket[0].open=0;
	src_socket[1].open=0;
}
void term_src_sockets()
{
	src_close_socket(&src_socket[0]);
	src_close_socket(&src_socket[1]);
}
