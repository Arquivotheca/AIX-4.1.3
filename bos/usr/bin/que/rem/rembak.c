static char sccsid[] = "@(#)76	1.24.1.4  src/bos/usr/bin/que/rem/rembak.c, cmdque, bos411, 9430C411a 7/26/94 16:38:53";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: rembak
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 	This is the remote backend for the queueing system.
	It communicates with remote hosts using the BSD lpr/lpd 
	protocol.  See commonr.h for protocol description.
*/
#include "commonr.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <sys/id.h>
#include "outr.h"
#define STDSTAT 3
#include <ctype.h>

#include "rem_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_REM,num,str)

extern boolean backend;	  /* global control flag in common.c */

char *progname = "rembak";
char *senddatafile();
char *makecontrolfile();

int 		remsock; 		/* socket file descriptor */
char		**saveArgv;

#ifdef DEBUG
FILE *outfp;
#endif

main(argc,argv)
int argc;
char **argv;
{
	int i;
	struct job 	thisjob, *j;
	struct comline 	thiscomline, *c;
	int		firstfile;	/* array num of first file in argv*/

#ifdef DEBUG
	if((outfp = fopen("/tmp/rembak.out","w")) == NULL)
		outfp = stderr;
        if(getenv("ALLPARMS"))
        {
                fprintf(outfp,"rembak: ");
                for(i = 0; i < argc; i++)
                        fprintf(outfp,"[%s]",argv[i]);
                fprintf(outfp,"\n");
		fflush(outfp);
        }
#endif

	/*
	 * Save arguments so that they can be printed when
	 * rembak dies.
	 */
	saveArgv = Qalloc((size_t) ( (argc+1) * sizeof(char *) ));
	for ( i = 0; i < argc; i++) {
		saveArgv[i] = Qalloc((size_t) (strlen(argv[i]) + 1));
		strcpy(saveArgv[i], argv[i]);
	}
	saveArgv[i] = NULL;

        (void)setlocale(LC_ALL,"");
        catd = catopen(MF_REM, NL_CAT_LOCALE);

	firstfile = 0;
	j = &thisjob;
	c = &thiscomline;

        /* D31739, we will run as user's UID, but with privileges from root */
        setuidx(ID_EFFECTIVE, getuidx(ID_REAL));

	/* R we being run by the Queueing system? */
	backend = get_backend();

#ifdef DEBUG
	if(getenv("REMBAK"))
	{
		fprintf(outfp,"rbmain: backend = %d\n",backend);
		fflush(outfp);
	}
#endif

	if (backend) 
		if ((i=log_init())<0)
			syserr((int)EXITFATAL,MSGSTR(MSGLBQB,"Error: libqb's log_init returned %d."),i);

	bzero((char *) j,sizeof(struct job));

	/* look at command line */
	definejob(argc,argv,c,j,&firstfile);	


	/* hookup to Server */
	if ( -1 == ( remsock = hookup(c->c_Srem)) ) {
		printf("%s\n",MSGSTR(MSGDWN2,"HOST_DOWN"));
		qexit((int)EXITFATAL);
		/* rsyserr(MSGSTR(MSGSERV,"Failure to hook up to remote server.")); */
	}

	if (backend) log_status(SENDING);

	if ( c->c_xflg )
	{ 	rcancel(remsock,c,j);
		goto done;
	}

	if ( c->c_qflg || c->c_Lflg )
	{ 	rstatus(remsock,c,j);
		goto done;
	}
	
	if ( c->c_Rflg )
	{ 	restart(remsock,c);
		goto done;
	}
	
	sendjob(remsock,j,c,argv,firstfile);

	done:
		return(0);
}

 /* 	Sends a job request and all datafiles(using senddatafile) and the 
 	control file(again using senddatafile) see commonr.h for a protocol 
 	description.
 */
boolean sendjob(s,j,c,argv,firstfile)
int		s;
struct job 	*j;
struct comline 	*c;
char		**argv;
int		firstfile;
{
	int i,k ;
	char *cfname;
	char *datafn[MAXFILES];
	char *makecontrolfile();
	
	for (i = 0; i < MAXFILES; datafn[i++] = 0);
		
	short_sendreq(s,c->c_Prem,'\2');

	i = firstfile;
	while (argv[i])	   /* i is the counter in argv, k everywhere else*/
	{ 	k = i - firstfile;
		if (!(datafn[k] = senddatafile(s,argv[i],k,c->c_Nrem)))
			syserr((int)EXITFATAL,MSGSTR(MSGDATA,"Could not send datafile %s."),argv[i]);
		i++;
	}
	if (backend == TRUE)
		j->j_jobname = scopy(get_title());
	else
		j->j_jobname = scopy(argv[firstfile]);
		
	if (!(cfname = makecontrolfile(j,c,argv,firstfile,datafn)))
			syserr((int)EXITFATAL,MSGSTR(MSGMAKE,"Could not make control file"));

	if (!sendcontrolfile(s,cfname,datafn[0]))
			syserr((int)EXITFATAL,MSGSTR(MSGSEND,"Could not send control file."));

	for (i=k; i; free((void *)datafn[i--]));

	unlink(cfname);

	return (TRUE);
}


 /* send a restart request to the lpd daemon*/
boolean restart(s,c)
int		s;
struct comline 	*c;
{
	if (!short_sendreq(s,c->c_Prem,'\1'))
		syserr((int)EXITFATAL,MSGSTR(MSGREST,"Could not send restart."));
	else
		return (TRUE);
}

 /* send a status request to the lpd daemon*/
boolean rstatus(s,c,j)
int		s;
struct comline 	*c;
struct job 	*j;
{
	char stattype;
	int numread;

	if (c->c_Lflg)
		stattype = '\4';
	else
		stattype = '\3';

	long_sendreq(s,j->j_u,j->j_jn,c->c_Prem,stattype);

	numread=readsock(s,fileno(stdout));

	if (numread)	/* if any data came back, assume success.
			   It will be the qstatus output from remote.
			   Who cares anyway?  The caller of this doesn't! */
		return (TRUE);
	else
		return (FALSE);
}

/* send a cancel request to the lpd daemon*/
boolean rcancel(s,c,j)
int		s;
struct job 	*j;
struct comline 	*c;
{
	int numread;

	long_sendreq(s,j->j_u,j->j_jn,c->c_Prem,'\5');
	numread = readsock(s,(backend ? STDSTAT : fileno(stderr)));
	if (numread)	/* if any data came back, assume failure because
			   it will be an error message.
			   Who cares anyway?  The caller of this doesn't! */
		return (FALSE);
	else
		return (TRUE);
}

 /* read the command line argv and parse it*/
definejob(argc,argv,c,j,firstfile)
int		argc;
char 		**argv;
struct comline 	*c;		/* the main idea of this routine*/
int 		*firstfile;	/* how far did we read thru the argv*/
struct job	*j;
{
	rrdopts(argc,argv,c,j,firstfile);
	validate(c,argv[*firstfile]);
}


/*
 * Create a connection to the remote printer server.
 */
int hookup(rhost)
	char *rhost;
{
	struct hostent *hp;
	struct servent *sp;
	struct sockaddr_in sin;
	char *tmpaddr;
	int s, timo = 1, lport = IPPORT_RESERVED - 1;
	int err;

	/*
	 * Get the host address and port number to connect to.
	 */
	if (rhost == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGNORM,"No remote host to connect to."));

	if (backend) log_status(GETHOST);
	if (!(isinet_addr(rhost)))
		hp = gethostbyname(rhost);
	else {
		tmpaddr = inet_addr(rhost);
		hp = gethostbyaddr(&tmpaddr,sizeof(tmpaddr),AF_INET);
	}
	
	if (hp == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGUNKN,"Unknown host %s."), rhost);
	sp = getservbyname("printer", "tcp");
	if (sp == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGPTCP,"Unknown service for printer/tcp."));
	bzero((char *)&sin, sizeof(sin));
	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = sp->s_port;

	/*
	 * Try connecting to the server.
	 */
	if (backend) log_status(CONNECT);
retry:
	s = rresvport(&lport);		
	if (s < 0)
		syserr((int)EXITFATAL,"rresvport");	/* insanity */

	if (connect(s, (caddr_t)&sin, sizeof(sin)) < 0) 
	{
		
		err = errno;
		(void) close(s);
		errno = err;
		if (errno == EADDRINUSE) 
		{
			lport--;
			goto retry;
		}
		if (errno == ECONNREFUSED && timo <= 16) 
		{
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		return(-1);
	}
	return(s);
}


char	*userstr();
char	*jobnumstr();

/* short requests never have an acknowledgement 
	long ones do.
*/
long_sendreq(sockfd,users,jobnums,printername,reqtype )
int 		sockfd;
struct users	*users;
struct jobnum	*jobnums;
char 		* printername;
char 		reqtype;
{
	char * reqstr;
	char reqtypestr[] = " ";

	reqtypestr[0]=reqtype;

	reqstr=sconcat( reqtypestr,
			printername,
			" ",
			users ? userstr(users) : "",	
			" ",
	 		jobnums ? jobnumstr(jobnums) : "",
			"\n",
			0);

	sendreq(sockfd,reqstr);
	free((void *)reqstr);
}

/* short requests always have an acknowledgement and a printername*/
boolean short_sendreq(sockfd,printername,reqtype )
int 		sockfd;
char 		*printername;
char 		reqtype;
{
	char * reqstr;
	char reqtypestr[] = " ";

	reqtypestr[0]=reqtype;

	reqstr=sconcat( reqtypestr,
			printername,
			"\n",
			0);

	sendreq(sockfd,reqstr);
	free((void *)reqstr);
	if (!gotack(sockfd))
		return(FALSE);
	else
		return(TRUE);
}

sendreq(sockfd,reqstring)
int sockfd;
char * reqstring;
{
	int size;

#ifdef DEBUG
        if(getenv("REMBAK"))
	{
                fprintf(outfp,"sendreq: \\%d%s",reqstring[0],&reqstring[1]);
		fflush(outfp);
	}
#endif
	
	size=strlen(reqstring);
	if (write(sockfd,reqstring,size) != size)
		syserr((int)EXITFATAL,MSGSTR(MSGSOCK,"Send request write error on socket %d %s."),sockfd,reqstring);
	return(0);
}


	/* read from a sock, write to a filedes ( use
	   log_message to do the write if it's the
	   statusfil. 
	*/
int readsock(sock,where)
int sock;
int where;
{
	char *current,buf[BUFSIZ*10];
	int size ;
	int bigsize =0;

	current=buf;

	while (1)
	{
		errno=0;
		size = read(sock, current, BUFSIZ);

		/* pretend like a broken pipe or reset is EOF */
		if ((size < 0) && (errno == EPIPE || errno == ECONNRESET))
			size=0;

		if (size < 0) 
			syserr((int)EXITFATAL,MSGSTR(MSGLOST,"Lost connection reading socket."));

		if (size == 0) 	break;
		current += size;
		bigsize += size;
	}

	if (!bigsize) 
		goto bottom;

	buf[bigsize++] = 0;		/* add null byte */

	if ((where==STDSTAT) && backend)
		syswarn(buf);
	else
	{
		errno=0;
		if ((size=write(where, buf, bigsize)) != bigsize) 
			syserr((int)EXITFATAL,MSGSTR(MSGERWT,"Error writing to fd %d."),where);
	}

bottom:
	(void) close(sock);
	return (bigsize);
}

/* send a datafile  across the net, which means:
	send req string
	get ack
	send the data file
	send ack
	get ack */
	
char * senddatafile(s,realfn,k,filtstr)
int s;			/* socket file descriptor.   */
char * realfn;	/* filename of file to send. */
int k;			/* this is the (k+1)th file in this job*/
char *filtstr;
{

	char		fakefn[MAXPATHLEN];        /* spooler filename */
	struct stat 	sbuf;
	char		reqtype = '\3';
        time_t 		curtime;
	int		jobnum,filefd;

        if (backend) 
	{ 	if (( jobnum = get_job_number())  < 0)
			syserr((int)EXITFATAL,MSGSTR(MSGGJOB,"Error getting job number %d."),jobnum);
	}
	else
		time(&curtime); 
        sprintf(fakefn, "df%c%03ld%s", 
			numtochar(k),
			(backend ? jobnum : curtime % 1000),  
			myhost() );

	/*----Insure a reasonable file size */
	if ((strlen(fakefn) > 10) && strstr(filtstr,V2FILT))
		fakefn[10] = '\0';

	if (stat(realfn,&sbuf) == -1)
		syserr((int)EXITWARN,MSGSTR(MSGSTAT,"Status on %s failed: Errno = %d."),realfn,errno);
	if (!send_file_req(s,sbuf.st_size,realfn,fakefn,reqtype))
		return(NULL);
	if ((filefd = open(realfn,O_RDONLY,0)) < 0 )
		syserr((int)EXITWARN,MSGSTR(MSGCOPN,"Cannot open %s."),realfn);
	if (!send_file(s,filefd,sbuf.st_size))
		return(NULL);
	return (scopy(fakefn));
}

/* send a file across the net and ack and then wait for ack.
   it can be either a data file or a control file*/
send_file(sokfd,f, count)
register int	sokfd;		/* socket file descriptor */
register int	f;		/* open file descriptor of file to send*/
register int	count;
{
	static char 	buf[BUFSIZ];
	register int	amt, i, sizerr;

	sizerr = 0;
	for (i = 0; i < count; i += BUFSIZ) 
	{
		amt = ((i + BUFSIZ > count) ? count - i : BUFSIZ);
		if (sizerr == 0 &&  read(f, buf, amt) != amt)
		{ 	write(sokfd,"\1\n",2);
		 	syserr((int)EXITWARN,MSGSTR(MSGESIZ,"Error reading file or file changed size."));	
			break;
		}
		if (write(sokfd, buf, amt) != amt) 
                	syserr((int)EXITFATAL,MSGSTR(MSGWSOK,"Could not write to socket while sending file."));
	}
	close(f);
#ifdef DEBUG
	if(getenv("REMBAK"))
	{
		fprintf(outfp,"send_file: file sent\n");
		fflush(outfp);
	}
#endif
	if (!ack(sokfd))
		syserr((int)EXITFATAL,MSGSTR(MSGNACK,"Could not send acknowledgement while sending file."));
	if (!gotack(sokfd ))
		syserr((int)EXITFATAL,MSGSTR(MSGACKN,"Send file acknowledgement not recieved."));
        return(TRUE);
}
 
/* send the file request string across the net and wait for ack*/
boolean send_file_req(sockfd,size,realfn,fakefn,reqtype )
int 		sockfd;
int		size;
char 		*realfn;
char 		*fakefn;
char 		reqtype;
{
	int retval = TRUE;
	char * reqstr;
	char reqtypestr[] = " ";
	char sizestr[50];

	reqtypestr[0] = reqtype;
	sprintf(sizestr,"%d",size);
	reqstr=sconcat( reqtypestr,
			sizestr,
			" ",
			fakefn,
			"\n",
			0);
	sendreq(sockfd,reqstr);
	if (!gotack(sockfd))
	  	retval = FALSE;
	free((void *)reqstr);
	return(retval);
}
