static char sccsid[] = "@(#)77	1.40.3.11  src/bos/usr/bin/que/rem/lpd.c, cmdque, bos411, 9436C411a 9/7/94 15:18:39";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1985 by the Massachusetts Institute of Technology 
 * Copyright (c) 1983 Regents of the University of California. 
 */



 
 
/*
 * lpd -- line printer daemon.
 *
 * Listen for a connection and perform the requested operation.
 * Operations are:
 *	\1printer\n
 *		check the queue for jobs and print any found.
 *	\2printer\n
 *		receive a job from another machine and queue it.
 *	\3printer [users ...] [jobs ...]\n
 *		return the current state of the queue (short form).
 *	\4printer [users ...] [jobs ...]\n
 *		return the current state of the queue (long form).
 *	\5printer person [users ...] [jobs ...]\n
 *		remove jobs from the queue.
 */

/*
	Adapted for AIX by W. Burger
	using the AIX/MIT code as basis.
	Most of the AIX/MIT flaws are still present,
	such as cleanup in case of errors and 'AIX' way
	of handling \3, \4, and \5 options.
*/	

#define _ILS_MACROS
#include <IN/standard.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#undef STATUS
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include "commonr.h"
#undef NORMAL 
#include <sys/syslog.h> 
#include <usersec.h>

/*
 * System Resource Controller (SRC) includes and variables
 */
#include <sys/time.h>
#include <spc.h>
#define SRCSTATNDX 	6
#define SRC_FD 		13
#define SRCMSG     	(sizeof(srcpacket))
static 	struct srcreq srcpacket;
struct	statrep *srcstatus;
short	srcstatsize;
int 	srcreplen;
int 	cont;
struct 	srchdr *srchdr;

#define PNAM 20

#undef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	MAXDNAME

#include <ctype.h>

#include "lpd_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_LPD,num,str)
 
char	cfilename[PATH_MAX]; 	/* control file name */
char	dfilename[PATH_MAX];
char	fhost[MAXHOSTNAMELEN];  /* D42772 needed host name for kill_print() */

char	pqueue[PNAM];
char	printer[PNAM];
 
int	lflag;				/* log requests flag */

extern	boolean qkillfile;		/* Global control flags in common.c */
extern	boolean lpd;
char	*progname = "lpd";

int	lpdID;

#ifdef DEBUG
FILE	*dd;
FILE	*outfp;
#endif

char	cbuf[BUFSIZ];	/* command line buffer */
char	*cmdnames[] = {
	"null",
	"printjob",
	"recvjob",
	"displayq short",
	"displayq long",
	"rmjob"
};

int	death(void);
int	config(void);
char	**saveArgv = NULL;

main(argc, argv)
	int argc;
	char **argv;
{
	int f, finet, options, fromlen;
	struct sockaddr_in sin, frominet;
	int omask;
	int on = 1;
	/* extern char *index(); */

/*
 * SRC local variable and code startup.
 */
	int rc, i, addrsz, debug = 0;
	struct sockaddr srcaddr;
        int src_exists = TRUE;

#ifdef DEBUG
	outfp = stderr; 
#endif

	omask = umask ((mode_t) 027);		/* set default umask for lpd */

        {
            /* Get lpd's ID for to set ownership of the files   */
            /* created by lpd daemon.                           */

            int ret;

            ret = getuserattr("lpd", S_ID, &lpdID, SEC_INT);
            if ( ret == -1 )
                perror("getuserattr lpd ");
        }

	lpd = TRUE;	/* this is lpd daemon. (error messages services) */
	openlog("lpd", LOG_PID, LOG_LPR);

        (void)setlocale(LC_ALL,"");
        catd = catopen(MF_LPD, NL_CAT_LOCALE);


	addrsz = sizeof(srcaddr);
	if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
                syswarn(MSGSTR(MSGSRC1,"getsockname(), SRC not found, continuing without SRC support. Errno = %d."),errno);
		src_exists = FALSE;
	}
	if (src_exists)
		(void) dup2(0, SRC_FD);


	while (--argc > 0) {
		argv++;
		if (argv[0][0] == '-')
			switch (argv[0][1]) {
			case 'd':
				debug++;
				options |= SO_DEBUG;
				break;
			case 'l':
				lflag++;
				break;
			default:
				sysuse( TRUE,
					MSGSTR(MSGUSE1,"[-d] [-l]"),
					(char *)0
				      );
			}
	}
	
	if (chdir(LPD_DIRECTORY)) {
		syslog(LOG_ERR, "%s: %m", (MSGSTR(MSGSPOO,
                        "cannot access spool directory, %s, exiting."),
                        LPD_DIRECTORY));
		syserr((int)EXITFATAL,MSGSTR(MSGSPOO,
			"cannot access spool directory, %s, exiting."),
			LPD_DIRECTORY );
		}

#ifndef DEBUG
	/*
	 * Set up standard environment by detaching from the parent.
	 */
/*
 * fork only if SRC is not in effect.
 */
                if (!src_exists)
                        if (fork())
                                qexit(0);


/*	for (f = 0; f < 5; f++)*/
/*		(void) close(f);*/
/*	(void) open("/dev/null", O_RDONLY);*/
/*	(void) open("/dev/null", O_WRONLY);*/
/*	(void) dup(1);*/
	f = open("/dev/tty", O_RDWR);
	if (f > 0) {
/* 
 * setpgrp call removed to support SRC
*/
		if (!src_exists && !debug)
			(void)setpgrp();
		(void) close(f);
	}
#else
/*	dd = fopen("/dev/console","a+");*/
#endif
	
	if (!creat_lock (LPD_LOCKNAME)) 
                {
		syslog(LOG_ERR, "%s: %m", (MSGSTR(MSGLOCK,"lock file, %s, or duplicate daemon."), LPD_LOCKNAME)); 
		syserr(1,MSGSTR(MSGLOCK,"lock file, %s, or duplicate daemon."), LPD_LOCKNAME);
		}

	qkillfile = TRUE; /* Lock file now exists (error messages services) */
	
	signal(SIGINT, (void (*)(int))config);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, (void (*)(int))death);
	signal(SIGCLD, SIG_IGN);
	
	finet = socket(AF_INET, SOCK_STREAM, 0);
	if (finet >= 0) 
	{
		struct servent *sp;

		if (options & SO_DEBUG)
			if (setsockopt(finet, SOL_SOCKET, SO_DEBUG, &on,
			   sizeof(on)) < 0) 
                                {
				syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m"); 
				syserr((int)EXITFATAL,"setsockopt (SO_DEBUG): ");	/* sanity */
				}
		sp = getservbyname("printer", "tcp");
		if (sp == NULL) 
                        {
			syslog(LOG_ERR, MSGSTR(MSGPTCP,"printer/tcp is an unknown service."));
			syserr((int)EXITFATAL,MSGSTR(MSGPTCP,"printer/tcp is an unknown service."));
			}
		sin.sin_family = AF_INET;
		sin.sin_port = sp->s_port;
		sin.sin_addr.s_addr = INADDR_ANY;
 

		if (bind(finet, (struct sockaddr *) &sin, sizeof(sin)) < 0) 
                        {
			syslog(LOG_ERR, "%s %m", MSGSTR(MSGBIND,"unable to bind socket.")); 
			syserr((int)EXITFATAL,MSGSTR(MSGBIND,"unable to bind socket."));
			}
		listen(finet, 5);
	} 
	else 
		{
		syslog(LOG_ERR, "%s %m", MSGSTR(MSGSOCK,"unable to create socket.")); 
		syserr((int)EXITFATAL,MSGSTR(MSGSOCK,"unable to create socket."));
		}
	
	/*
	 * Main loop: accept, do a request, continue.
	 */
	for (;;) 
	{
		int s;
		int ibits;
		int n;
		int onoff;
		int x;
		FILE *hostf;
		char ahost[MAXHOSTNAMELEN];
		register int p;
		boolean readflg;
		char *hostp;
		int src_cont;

		errno = 0;
		cont=END;
		addrsz=sizeof(srcaddr);
		do {
                	ibits = 1 << finet;
			if (src_exists)
                		ibits |= (1 << SRC_FD);
                	if ((n = select(20,&ibits,0,0,(struct timeval *)NULL)) <= 0) {
                        	if (n < 0 && errno != EINTR)
                                        {
					syslog(LOG_WARNING, "select: %m"); 
                                    	syswarn("select: %m\n");
					}
					continue;
                	}
                	if (ibits & (1 << finet)) {

				fromlen = sizeof(frominet);
				s = accept(finet, &frominet, &fromlen);

				if (s < 0) {
					if (errno != EINTR)
                                                {
						syslog(LOG_WARNING, "%s %m", MSGSTR(MSGACPT, "failure to accept a socket connection.")); 
						syswarn(MSGSTR(MSGACPT,
						"failure to accept a socket connection."));
						}
					continue;
				}
		
				if (fork() == 0) {
					qkillfile = FALSE;
					signal(SIGCLD, SIG_DFL);
					signal(SIGHUP, SIG_IGN);
					signal(SIGINT, SIG_IGN);
					signal(SIGQUIT, SIG_IGN);
					signal(SIGTERM, SIG_IGN);
					(void) close(finet);
					dup2(s, 1);
					(void) close(s);
					
					if (frominet.sin_family != AF_INET ||
			    		frominet.sin_port >= IPPORT_RESERVED)
						byebye(MSGSTR(MSGILLF,"ill-formed FROM address."));

					if (chk_fhost(frominet.sin_addr.s_addr))
						byebye(MSGSTR(MSGLINE,
							"your host does not have line printer access."));
					doreq();
					(void) close( fileno(stdout) );
					qexit(0);
				}
				(void) close(s);
			}

                	if (src_exists && (ibits & (1 << SRC_FD)))
                        	rc = recvfrom(SRC_FD, &srcpacket, SRCMSG, 0, &srcaddr, &addrsz);
                	else {
                        	rc = -1;
                        	errno = EINTR;
                	}

 		} while (rc == -1 && errno == EINTR);
		if (rc < 0)
			syserr(1,MSGSTR(MSGSRC2, "Error in recvfrom().  Errno = %d"),errno);

		/* process the request packet */
		if (src_exists) {
			switch(srcpacket.subreq.action) {
			case START:
				dosrcpacket(SRC_SUBMSG,progname,MSGSTR(MSGSRC3,"NOTICE: lpd does not support this option.\n"),sizeof(struct srcrep));
				break;
			case STOP:
				if (srcpacket.subreq.object == SUBSYSTEM) {
					dosrcpacket(SRC_OK,progname,NULL,sizeof(struct srcrep));
					if (srcpacket.subreq.parm1 == NORMAL)
						death();
					qexit(EXITOK);
				} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(MSGSRC3,"NOTICE: lpd does not support this option.\n"),sizeof(struct srcrep));
				break;
		    	case STATUS:
			    	if (srcpacket.subreq.object == SUBSYSTEM) {
			    		srcreplen = srcstatsize = sizeof(struct srchdr) + (sizeof(struct statcode)*SRCSTATNDX);
			    		srcstatus = (struct statrep *)Qalloc((size_t)srcstatsize);
			    		memset(srcstatus,(int)'\0',(size_t)srcreplen);
					src_cont=NEWREQUEST;
			    		srcstat("","",getpid(),&srcstatsize,srcstatus, &src_cont);
			    		srchdr = srcrrqs(&srcpacket);
					sprintf(srcstatus->statcode[3].objname,
						"%-12s",MSGSTR(MSGSRC4,"Trace"));
					if (lflag) 
						sprintf(srcstatus->statcode[3].objtext,
							" %s",MSGSTR(MSGSRC5, "Active"));
					else
						sprintf(srcstatus->statcode[3].objtext,
							" %s",MSGSTR(MSGSRC6, "Inactive"));
					sprintf(srcstatus->statcode[4].objname,
						"%-12s",MSGSTR(MSGSRC7,"Debug"));
					if (debug) 
						sprintf(srcstatus->statcode[4].objtext,
							" %s",MSGSTR(MSGSRC5, "Active"));
					else
						sprintf(srcstatus->statcode[4].objtext,
							" %s",MSGSTR(MSGSRC6, "Inactive"));
			    		srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
			    		memset(srcstatus,(int)'\0',(size_t)srcreplen);
					sprintf(srcstatus->statcode[0].objname,
						"%-12s",MSGSTR(MSGSRC8,"Signal"));
					sprintf(srcstatus->statcode[0].objtext,
						" %s",MSGSTR(MSGSRC9,"Purpose"));
					sprintf(srcstatus->statcode[1].objname,
						" %-12s",MSGSTR(MSGSRC12,"SIGTERM"));
					sprintf(srcstatus->statcode[1].objtext,
						"%s",MSGSTR(MSGSRC13, "Terminates lpd"));
					sprintf(srcstatus->statcode[2].objname,
						" %-12s",MSGSTR(MSGSRC14,"SIGHUP"));
					sprintf(srcstatus->statcode[2].objtext,
						"%s",MSGSTR(MSGSRC15, "Signal ingored"));
					sprintf(srcstatus->statcode[3].objname,
						" %-12s",MSGSTR(MSGSRC16,"SIGQUIT"));
					sprintf(srcstatus->statcode[3].objtext,
						"%s",MSGSTR(MSGSRC15, "Signal ingored"));
					sprintf(srcstatus->statcode[4].objname,
						" %-12s",MSGSTR(MSGSRC17,"SIGCLD"));
					sprintf(srcstatus->statcode[4].objtext,
						"%s",MSGSTR(MSGSRC15, "Signal ingored"));
			    		srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
					x = 0;
			    		memset(srcstatus,(int)'\0',(size_t)srcreplen);
					if (hostf = fopen(LPD_HOSTS, "r"))  {
						while (fgets(ahost, (int)sizeof (ahost), hostf))
						{
						     readflg = TRUE;
						     hostp = ahost;

						     /* REMOVE white spaces before text */
						     while ((*hostp != '\n') && isspace(*hostp))
							     hostp++;

                                                     if (index(hostp, (int)'\n'))
							   readflg = FALSE;
						      /* PARSE hostname and read the rest of the line. */
						      p = parse_hostname(hostp,readflg,hostf);
                                                     if (*hostp == '\n' || *hostp == '#')
                                                              continue;

                                                      if (x >= SRCSTATNDX) {
                                                              srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
                                                              x = 0;
                                                              memset(srcstatus,(int)'\0',(size_t)srcreplen);
                                                      }
                                                      sprintf(srcstatus->statcode[x].objname,"%.20s",MSGSTR(MSGSRC18,"hosts.lpd Hostname"));
						      if (!p)
                                                       		sprintf(srcstatus->statcode[x].objtext,"%s",hostp);
						       else sprintf(srcstatus->statcode[x].objtext,MSGSTR(MSGBADH,"WARNING:  %.20s: host name too long."),hostp);
                                                       x++;

						}
						(void) fclose(hostf);
					}
					if (x > 0)
			    			srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
			    		srcsrpy(srchdr,srcstatus,sizeof(struct srchdr),END);
			    		free((void *)srcstatus);
			    	} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(MSGSRC3,"NOTICE: lpd does not support this option.\n"),sizeof(struct srcrep));
	    		    	break;
			case REFRESH:
				config();
				dosrcpacket(SRC_OK,progname,NULL,sizeof(struct srcrep));
				break;
			case TRACE:
				if (srcpacket.subreq.object == SUBSYSTEM) { 
					onoff = (srcpacket.subreq.parm2 == TRACEON) ? 1 : 0;
					debug = onoff;
					if (setsockopt(finet, SOL_SOCKET, SO_DEBUG, &onoff, sizeof(onoff)) < 0)
						syserr((int)EXITFATAL,"setsockopt (SO_DEBUG): ");
				} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(MSGSRC3,"NOTICE: lpd does not support this option.\n"),sizeof(struct srcrep)); 
				break;
			default:
				dosrcpacket(SRC_SUBICMD,progname,NULL,sizeof(struct srcrep));
				break;

			}
		}
		
	}
}

int dosrcpacket(msgno, subsys, txt, len)
	int msgno;
	char *subsys;
	char *txt;
	int len;
{
	struct srcrep reply;

	reply.svrreply.rtncode = msgno;
	strcpy(reply.svrreply.objname, subsys);
	strcpy(reply.svrreply.rtnmsg, txt);
	srchdr = srcrrqs(&srcpacket);
	srcsrpy(srchdr, &reply, len, cont);
}

doreq()
{
	register char *cp,*pp;
	register int n;


	cp = cbuf;
	do {
		if (cp >= &cbuf[sizeof(cbuf) - 1])
			byebye(MSGSTR(MSGTLNG,"command line too long."));
		if ((n = read(1, cp, 1)) != 1) {
			if (n < 0) {
				syslog(LOG_ERR, MSGSTR(MSGLOST,"Lost Connection."));
				syserr(1,MSGSTR(MSGLOST,"Lost Connection."));
			}
			return(0);
		}
	} while (*(cp++) != '\n');
	*(--cp) = '\0';

#ifdef DEBUG
	if (getenv("LPD"))
		fprintf(outfp,"doreq: command is [\\%d%s]\n",cbuf[0],&cbuf[1]);
#endif

	cp = cbuf;
	if (*cp >= '\1' && *cp <= '\5') {
		if (lflag)
                        {
			syslog(LOG_INFO, MSGSTR(MSGRQST,"request %s %s."),
                                cmdnames[*cp], cp+1);
			errno=0;
			systell(MSGSTR(MSGRQST,"request %s %s."),
				cmdnames[*cp], cp+1);
			}
	} else {
		if (lflag)
                        {
			syslog(LOG_INFO, MSGSTR(MSGBRQS,"bad request (%d)."), *cp); 
			errno=0;
			syswarn(MSGSTR(MSGBRQS,"bad request (%d)."), *cp);
			}
		byebye(MSGSTR(MSGILLG,"illegal service request."));
	}
	
	strip(&cbuf[1],&cbuf[1]);  /* normalize request string */

 	cp++; pp = pqueue; n = 0;
	while (*cp && *cp != ' ' && n < PNAM-1) {
		*pp++ = *cp++; n++;
	}	
	*pp = '\0';
	if ((n==PNAM-1 && *cp && *cp != ' ') || chk_printer(pqueue)) {
		syslog(LOG_ERR, MSGSTR(MSGUNKN,"unknown printer %s."),pqueue);
 		byebye(MSGSTR(MSGUNKN,"unknown printer %s."),pqueue);
		}
	
	switch (cbuf[0]) {
	case '\1':	/* check the queue and print any jobs there */
		byebye(MSGSTR(MSGNOPT,"option not supported."));
		break;
	case '\2':	/* receive files to be queued */
		if (readjob() == 0) {
			write(1,"",1);   /* D57383 */
		}
		break;
	case '\3':	/* display the queue (short form) */
	case '\4':	/* display the queue (long form) */
		send_status(cbuf);
		break;
	case '\5':	/* remove a job from the queue */
		kill_print(cbuf);
		break;
	}
	return(0);
}

/*
 * Check for valid printer request.
 * Make sure that printer queue specified exists, that is, check
 * /etc/qconfig for argname = printer.
 */
chk_printer(printer)
char	*printer;
{
	char	execbuf[MAXPATHLEN];
	int	pid,stat,rv,high,low;

	if ((pid=fork())==(-1))
                {
		syslog(LOG_WARNING, MSGSTR(MSGFORK,"unable to fork process.")); 
		syserr((int)EXITFATAL,MSGSTR(MSGFORK,"unable to fork process."));
		}
	else
		if (pid)	/* if parent */
		{
			rv=wait(&stat);
			low = stat & 0xFF;
			high = stat >> 8;
			/*  if exited corrrectly then low=0 and high =value*/
			if ((low==0 ) && (high==0))
					return(0);
				else
					return(1);
		}
		else
		{
			sprintf(execbuf,"-P%s",printer);
			/* the Y flag to enq with the P flag and printername
			   tells us if this is a valid printer or not */
			execl("/bin/enq","/bin/enq",execbuf,"-Y",0);
			syserr((int)EXITFATAL,MSGSTR(MSGEXEC,"could not exec /bin/enq."));
		}

}


readjob()
{
	char *cp;
	int n, size;
	int ck;

	write(1,"",1);  /* ack command */
	ck = 0;
	cfilename[0] = '\0';
	dfilename[0] = '\0';

	for (;;) {
		/* get a command line */
		cp = cbuf;
		do {
			if ((n = read(1, cp, 1)) != 1) 
			{
				if (ck == 1 || ck == 2)
				{
					ck = 0;
					rcleanup();
				}	
#ifdef DEBUG
				if(getenv("LPD"))
					fprintf(outfp,"readjob: returning %d [%s]\n",ck,cbuf);
#endif	
				return(ck);
			}
		} while (*(cp++) != '\n');
		*(--cp) = '\0';
#ifdef DEBUG
		if(getenv("LPD"))
			fprintf(outfp,"readjob: command is [\\%d%s]\n",cbuf[0],&cbuf[1]);
#endif	
		cp = cbuf + 1;
		switch (*cbuf) {
		case '\1':	/* cleanup because data sent was bad */
			rcleanup();
			continue;

		case '\2':	/* read cf file */
			if (chk_request(cp,&size,cfilename))
				continue;
			if (!readfile(cfilename, size)) {
				rcleanup();
				continue;
			}
			ck |= 1;
			if (ck == 3) {    
				doit(cfilename);
				unlink(cfilename);
				ck=0;
			}
			continue;

		case '\3':	/* read df file */
			if (chk_request(cp,&size,dfilename))
				continue;
			if (readfile(dfilename, size))
				ck |= 2;
			if (ck == 3) {    
				doit(cfilename);
				unlink(cfilename);
				ck=0;
			}
			continue;
		}
		frecverr(MSGSTR(MSGPROT,"protocol error."));
		syserr(1,MSGSTR(MSGPROT,"protocol error."));
	}
}

noresponse()
{
	char resp;

	if (read(1, &resp, 1) != 1)
	{
		frecverr(MSGSTR(MSGLOST,"Lost Connection"));
		syserr(1,MSGSTR(MSGLOST,"Lost Connection"));
	}
	if (resp == '\0')
		return(0);
	return(1);
}


/*
 * Read files sent and copy them to the spooling directory.
 */
readfile(file, size)
	char *file;
	int size;
{
	register char *cp;
	char buf[BUFSIZ];
	register int i, j, amt;
	int fd, err;
	mode_t mode=S_IRUSR|S_IWUSR|S_IRGRP;

	fd = open(file, O_WRONLY|O_CREAT, mode);
	if (fd < 0)
	{
		frecverr("%s: ",file);
		syserr(1,"%s: ",file);
	}
	err = 0;
	for (i = 0; i < size; i += BUFSIZ) {
		amt = BUFSIZ;
		cp = buf;
		if (i + amt > size)
			amt = size - i;
		do {
			j = read(1, cp, amt);
			if (j <= 0)
			{
				frecverr(MSGSTR(MSGLOST,"lost connection"));
				syserr(1,MSGSTR(MSGLOST,"lost connection"));
			}
			amt -= j;
			cp += j;
		} while (amt > 0);
		amt = BUFSIZ;
		if (i + amt > size)
			amt = size - i;
		if (write(fd, buf, amt) != amt) {
			err++;
			break;
		}
	}

        if ( (i = fchown(fd, lpdID, -1)) == -1 )
            systell(MSGSTR(MSGOWN,"Failed to change ownership of %s to %d\n"),
                    buf, lpdID);

	close(fd);
	if (err)
	{
		frecverr(MSGSTR(MSGEWRT,"error writing to %s."), file);
		syserr(1,MSGSTR(MSGEWRT,"error writing to %s."), file);
	}
	if (noresponse()) {		/* file sent had bad data in it */
		(void) unlink(file);
		return(0);
	}
	write(1,"",1);
	return(1);
}


/*
 * Remove all the files associated with the current job being transfered.
 */
rcleanup()
{

  	if ((strchr(cfilename,'/'))!=NULL) return;
  	if ((strchr(dfilename,'/'))!=NULL) return;
 
	if (cfilename[0])
		unlink(cfilename);

	if (dfilename[0])
		do {
			do {
				(void) unlink(dfilename);
			} while (dfilename[2]-- != 'A');
		} while (dfilename[0]-- != 'd');
	dfilename[0] = '\0';
	cfilename[0] = '\0';
}

frecverr(msg, a1, a2)
        char * msg;
{
        rcleanup();
	syslog(LOG_ERR, msg, a1, a2);
        write(1,"\1",1);                /* return error code */
}

/*
 * Check for valid request
 * Parse request into file size and file name
 * Verify, that there is enough space on disk to store the file.
 */
chk_request(rq_str, psize, filename)
register char	*rq_str;
register int	*psize;
register char	*filename;
{
	FILE	*fp;

	if (sscanf(rq_str, "%d %s", psize, filename) != 2) {
		filename[0] = '\0';
		write(1,"\2",1);
		return(-1);
	}
  	if ((strchr(filename,'/')) != NULL)
  	{
  		byebye(MSGSTR(MSGBADF,"Invalid filename sent."));
	}
	if ((fp = fopen(filename, "w")) == NULL) {
		write(1,"\2",1);
		return(-1);
	}
	if (fclear(fileno(fp), *psize) != (*psize)) {
		write(1,"\1",1);
		fclose(fp);
		return(-1);
	}
	fclose(fp);
	write(1,"",1);
	return(0);
}
 
#define	LOCKMODE	0444			/* mode for lock file	*/
 
/* Try to create a lock file with the specified name and write our 
 * process id out there.  Returns 1 on success, 0 on failure.
 */ 
creat_lock (name) 
register char	*name; 
{ 
	register FILE	*fptr;			/* for stdio		*/
	int pid;				/* process ID		*/
	 
	if ((fptr = fopen(name,"r")) != NULL)	/* try to open		*/
	{
		if (fscanf(fptr,PID_FMT,&pid) == 1)
			if (kill(pid,0) == 0)	/* does process exist?	*/
				return(0);
		unlink(name);			/* remove bad lock file	*/
	}
	fptr = fopen(name,"w");
	fprintf(fptr, "%d\n", getpid());
	fclose(fptr);
	return(1);
}
 
death(void)
{
	/* Defect 10454:
	 * Errno is completely invalid here so we set it to 0 so as
	 * to avoid irrelevant error messages.
	 */
	errno = 0;
	syserr((int)EXITOK,MSGSTR(MSGDETH,"Terminating."));
}

 
send_status(req)
register char *req;
{
	register FILE	*pp;
	char		*buf;
	int		rc;
	char		enqstr[PATH_MAX];	/* string for enq call */
	char		*p;			/* Temp pointer */
	char		tmpreq[PATH_MAX];

	strcpy(tmpreq,req);	/* Copy The request line into temp array */
	if (req[0] == '\4')	/* If long request */
		strcpy(enqstr,"IFS=' \t\n'; export IFS; /bin/enq -L -P");
	else			/* Else it is a short request */
		strcpy(enqstr,"IFS=' \t\n'; export IFS; /bin/enq -q -P");
	strcat(enqstr,pqueue);	/* Put the queue name in the string */
	strcat(enqstr," ");	/* separate with a blank */
	p = strtok(tmpreq," \t\n");	/* Look for a blank, tab, or newline */
	while((p=strtok(NULL," \t\n")) != NULL) {	/* Get first token */
		if(atoi(p))		/* Check if it is a job number */
			strcat(enqstr,"-#");
		else			/* Must be a user name */
			strcat(enqstr,"-u");
		strcat(enqstr,p);	/* append on the string */
		strcat(enqstr," ");	/* separate each with a  blank */
		if((strlen(enqstr) >= PATH_MAX))
			break;
	}
	/*----Invoke the proper status request and read output into socket */
	buf = (char *)Qalloc(BUFSIZ);
#ifdef DEBUG
        if(getenv("LPD"))
                fprintf(outfp,"send_status: enqstr = [%s]\n",enqstr);
#endif

 	if ((pp = popen(enqstr,"r")) != NULL)
	{
		while ((rc = read(fileno(pp), buf, BUFSIZ)) > 0)
			write(1,buf,rc);
		pclose(pp);
	}

	/*----Get outta here */
	free((void *)buf);
	return(0);
}
 
/*----Issue an enq cancel job request according to the request string from socket */
kill_print(req)
register char	*req;
{
	char	cmd[128];
	char	*begin,
		*end;
	char	usr_nam[USERNAMELEN];
	int 	tfd;			/*temp file descriptor*/
	int 	num;
	char	*tmp_lpd, *lpd_buf;
	register int i=0;

	/*----Assumed format is "\5queuename ### ### ### ###\n" */
	/*----Skip over the queue name */
        begin = req + 1;
	end = begin;
        end = strpbrk(begin," \t\n");
	while (!isalpha((int)*end))
		end++;	/* find user name */
	while ((*end != '\t') && (*end != '\n') && (*end != ' '))
	{
		usr_nam[i++] = *end;
		end++;
	}
	/* D42772 - add hostname if it is not already there */
	if (strchr(usr_nam,'@') ==NULL) {
		usr_nam[i++]='@';
		usr_nam[i] = '\0';
		strcat(usr_nam,fhost);
	}
	else {
		usr_nam[i] = '\0';
	}
	/* D42772 - end */
	
	/*----Process each job number */
	while(1)
	{
		/*----Find next job number */
		begin = strpbrk(end,"-0123456789");
		if (begin == NULL)
			break;
		end = strpbrk(begin," \t\n");
                if (!end)
                {
                        end = begin;
                        while (*end) end++;
                }
		if ((*(begin - 1) != ' ') && (*(begin - 1) != '\t'))
			continue;
		*end = '\0';

		tfd = gettmpfile(LPD_DIRECTORY,&tmp_lpd);

#ifdef NO_CANCEL_ALL
		/*
		 * Currently we don't support remote cancel all.
		 * If it's ever added, this code needs to be consistent
		 * with the following code for error message handling.
		 */
		/*----Check for cancel all */
		if (!strncmp(begin,REM_ALL_STR,(size_t)4))
		{ 
			sprintf(cmd,"enq -P%s -X > %s 2>&1",pqueue, tmp_lpd);
			system(cmd);
			close(tfd);	/* close and remove temporary files */
			unlink(tmp_lpd);
			break;
		}
#endif

		/*----Otherwise cancel a regular job */
		sprintf(cmd,"enq -P%s -x%s -u%s > %s 2>&1",pqueue,begin,usr_nam,tmp_lpd);
		if ( -1 == system(cmd))
			syswarn("system(), kill_print()");
		lpd_buf = (char *)Qalloc(BUFSIZ);
		while ((num = read(tfd,lpd_buf,BUFSIZ)) > 0) {
			if ( write(fileno(stdout), lpd_buf,num) != num )
				syswarn("write(), kill_print():");
		}
		close(tfd);	/* close and remove temporary files */
		unlink(tmp_lpd);
		if ( -1 == num )
			syswarn("read(), kill_print():");
		*end = ' ';
	} /* while(1) */

	/*----Get outta here (error responses on local stderr) */
	return(0);
}
 

byebye(msg, a1, a2, a3)
	char *msg;
{	

#ifdef DEBUG
	if(getenv("LPD"))
	{
		fprintf(outfp,"Terminate: ");
		fprintf(outfp,msg,a1,a2,a3);
		fprintf(outfp,"\n");
	}
#endif
	printf(msg,a1,a2,a3);
	fflush(stdout);
	close(1);
	qexit(1);
}	
		
 
/*
 * strip(st);
 *  Takes a ptr to a null-terminated character string,
 *  transforms the string without leading and trailing white space, with blanks
 *  substituted for tabs and with multiple blanks compressed to one between
 *  each token.
 */
strip(st,s)
	char *st, *s;
{
	register int bl;

	if ((!st) || (! *st))  /* NULL or empty string */
		return(0);
	bl = 0;
	while (*st == ' ' || *st == '\t') st++;
	while (*st) {
  		if (*st == ' ' || *st == '\t' || *st == '\n') {
      			if (!bl) {
      				bl = 1; *s++ = ' ';
      			}
      		} else {
      			bl = 0; *s++ = *st;
      		}	
      		st++;
      	}	
      	if (!bl) *s = '\0';
      	else *(s-1) = '\0';
	return(0);
}

config(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, (void (*)(int))config);
	return(0);
}


/*
 * Check to see if the from host has access to the line printer.
 * NB: this calls _validuser() from libc/net/rcmd.c so that we can
 * use the same logic to do + and - hostname or NIS netgroup.
 */
chk_fhost(addr)
long	addr;
{
	FILE *hostf;
	struct hostent *hp;
	static char dummy[] = ":nobody::";
	int i;
	int baselen = -1;
	char *cp;


	if( (hp = gethostbyaddr( &addr, sizeof(addr), AF_INET ))
		== (struct hostent *)0 )
		return(-1);
	errno=0;                    /* D12467 */
	strcpy(fhost, hp->h_name);

	/* mark position of domain name, if any */
	for (cp = fhost; *cp; ++cp) {
		if (*cp == '.') {
			baselen = cp - fhost;
			break;
		}
	}

	if (hostf = fopen(EQUIV_HOSTS, "r"))  {
		if (i = _validuser(hostf, fhost, dummy, dummy, baselen)) {
			(void) fclose(hostf);
			return(i == 1 ? 0 : i);
		}
		(void) fclose(hostf);
	}
	if (hostf = fopen(LPD_HOSTS, "r"))  {
		if (i = _validuser(hostf, fhost, dummy, dummy, baselen)) {
			(void) fclose(hostf);
			return(i == 1 ? 0 : i);
		}
		(void) fclose(hostf);
	}
	return (-1);
}

/* 
 * Examine the string from the hosts.lpd file and return only the host
 * name portion of it.  Place a newline at the end of the name, separating
 * blanks, tabs, comments from the name.  Read the rest of the line from
 * hosts.lpd if applicable.
 * Return error code if the host name is too long.
*/
parse_hostname(host_ptr,rdflag,hostf)
char *host_ptr;
boolean rdflag;
FILE *hostf;
{
	char *hindex;
	char rdbuf[MAXHOSTNAMELEN];
	char nextch;
	boolean name2long=FALSE;
	boolean iscmnt=FALSE;

	/* PARSE hostname, removing comments, etc. */
	if ((*host_ptr != '\n') && (*host_ptr != '#'))
	{
		if ((hindex = index(host_ptr,'#'))  ||
	    	(hindex = index(host_ptr,'\t')) ||
	    	(hindex = index(host_ptr,' ')) )
			*hindex = '\0';
		else
			if ( hindex = index(host_ptr,'\n') )
				*hindex = '\0';
	}
	else
		iscmnt = TRUE;

	/* READ the rest of the line from hosts.lpd. */
	if ( rdflag )
	{
		if ((nextch = fgetc(hostf)) && (nextch != '\n'))
		{
			if (!iscmnt && !isspace(nextch) && (strlen(host_ptr) >= MAXHOSTNAMELEN-1))
				name2long = TRUE;	/* hostname too long */
			while (fgets(rdbuf, (int)sizeof (rdbuf), hostf))
				if (index(rdbuf,'\n'))
					break;
		}
	}

	if ( name2long )
		return(1);
	return(0);
}
