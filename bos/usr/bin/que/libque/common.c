static char sccsid[] = "@(#)61	1.52  src/bos/usr/bin/que/libque/common.c, cmdque, bos41B, 9504A 12/19/94 15:12:35";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: checkjobnum, dqstr, gettmp, terminate, stclean, cleanlist, scopy,
 *            rindex, sconcat, getln, getqn, issame,
 *            tochar, stname, qexit, redigest, remember, Qalloc, cd,
 *            renamefile, cmppri, qentry, syserr, syswarn, sysmsg, systell, sysuse,
 *            modtime, badtmp, rightjob1, separate, thishost, getline,
 *            getarray, cons_open
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/param.h>
#include <IN/standard.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <IN/DRdefs.h>
#include <sys/stat.h>
#include <varargs.h>
#include "common.h"
#include "enq.h"
#include <sys/dir.h>
#include <IN/backend.h>
/*#include <sys/vmount.h>*/
#include <ctype.h>
#include <string.h>
#include "libq_msg.h"
extern char *getmesg();

FILE *console;
char consnam[] = CONSOLE;


char gotint = FALSE;
char msgbuf[LINE_MAX];

boolean palladium_inst = FALSE;	/* Is Palladium installed on system? */

extern char *ruser;
extern char *progname;

char *rindex();
char *strchr();

/*
 * Common code to print and qdaemon
 *      read config file
 *      read qdir
 *      some auxiliary subroutines
 *
 * Many of the routines in this module act slightly different
 * for qdaemon than for enq.  This is controlled with a 
 * GLOBAL CONTROL FLAG. (sorry.)  The flag (qdaemon) should only be 
 * set in the main routines of respective executables.
 */
boolean	qdaemon = FALSE; 	/* GLOBAL CONTROL FLAG */
boolean qkillfile = FALSE;	/* GLOBAL CONTROL FLAG */
boolean backend = FALSE; 
boolean lpd = FALSE;
char	**saveArgv;

	
int checkjobnum(inpnum)
char	*inpnum;
{
	int num = 0;

	num = atoi(inpnum);	/* MINJOB and MAXJOB must be set so that atoi()
				   returns a value outside of (MINJOB,MAXJOB)
			           for invalid number strings. (i.e. 123w45) */

	if (((num < MINJOB) || (num > MAXJOB)) && (!palladium_inst))
	        return(-1);
	else
		return(num);
}
			
/* return string showing date queued for file name fil */
char *dqstr(fil)
char *fil;
{
	time_t mtime;
	static char dq_timbuf[LINE_MAX];

	if ((mtime = modtime(fil)) < 0)
		return(GETMESG(MSGUNKN,"Unknown"));
	(void) strftime(dq_timbuf,50,"%a %h %d %T %Y",localtime(&mtime));
	return(dq_timbuf);
}

/*
* Create a tmp file and place the name in str.  If
* dir is !NULL, then create in that directory.
*/

gettmp(dir,str)
register char *dir, *str;
{
	int 		retries, fd;

	/* prime the random number pump */

	for( retries = 0; /* null */; retries++ )
	{
		/* generate a psuedo-unique name */

		if (dir != NULL)
			sprintf(str,"%s/tXXXXXX",dir);
		else
			sprintf(str,"tXXXXXX" );

		mktemp(str);

		/* 
		* Try to create it exclusive access, this fails if 
		* the file already exits as opposed to creat().
		*/

		/* AIX security enhancement				*/
		/* create file with no initial permissions.		*/
		/* the caller should set the necessary permissions.	*/
		if( (fd = open(str,O_RDWR|O_CREAT|O_EXCL,0000)) < 0 )
		{
			if( retries < 15 )
			{
				/* not unique enough, try again */
				errno = 0;
				continue;
			}
			return(-1);	/* give up */
		}
		break;
	}
	close(fd);
	remember(str);
	return(0);	/* success */
}

/* called on interrupt or hangup */
terminate()
{
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT,SIG_IGN);
#ifdef DEBUG
	if( getenv("TRMNATE") )
		sysraw("aborted\n");
#endif
	remember(DELETE);
	qexit((int)EXITSIGNAL);
}


/* wipe a status file structure clean to avoid confusion */
stclean(s,dev)
register struct stfile *s;
register struct d *dev;
{       
	static char unknown[] = "Unknown";

	bzero(s,sizeof(struct stfile));

	s->s_jobnum	= 0;
	s->s_status	= READY;
	s->s_align	= FALSE;
	s->s_feed	= NOFEED;
	s->s_head	= (dev? dev->d_head: NEVER);
	s->s_trail	= (dev? dev->d_trail: NEVER);
	s->s_copies	= 1;
	s->s_mailonly	= FALSE;
	s->s_was_idle	= FALSE;
	s->s_percent	= 0;
	s->s_pages	= 0;
	s->s_charge	= 0;
	strncpy (s->s_qdate,		unknown,S_DATE);
	strncpy (s->s_to,		unknown,S_TO);
	strncpy (s->s_from,		unknown,S_FROM);
	strncpy (s->s_title,		unknown,S_TITLE);
	strncpy (s->s_device_name,	unknown,S_DNAME);
	strncpy (s->s_queue_name,	unknown,S_QNAME);
	s->s_uid	= 0;
	strncpy (s->s_cmdline,		unknown,S_CMDLEN);
}

/**********************************************************************/
/* cleanlist                                                          */
/* This functions runs through a list of transformed filenames and    */
/* unmounts and frees them. Then frees the allocated string list      */
/* structure.                                                         */
/**********************************************************************/
void cleanlist(head)
register struct str_list *head;  /* head of the list of filenames */
{
	 register struct str_list *fl, *prev_p ;

			for (prev_p=NULL,fl=head; fl; fl = fl->s_next)
			{
				/* free file */
				if (fl->s_name) free((void *)fl->s_name);
				/* if not at the top of the list */
				if (prev_p != NULL)
					free((void *)prev_p);
				prev_p = fl;
			}
			/* free last one */
			if (prev_p)
				free((void *)prev_p);
}


/*====Allocate and Copy string of size to destination */
char *			/*----Returns a character pointer to alloced mem */
sncopy(str,size)
char	*str;		/*----String to copy */
int	size;		/*----How moch of it to copy */
{
	char	*new;

	new = Qalloc(size + 1);
	strncpy(new,str,size);
	new[size] = '\0';
	return(new);
}


/* copy a string into allocated space. */
char *scopy(str)
register char *str;
{       register char *new;

	new = Qalloc(strlen(str)+1);
	strcpy(new,str);
	return(new);
}


/*
	Concatanate the input strings together and return a pointer to new 
	memory which contains the new string.  
	Last arg must be NULL.
*/
char * sconcat(arg)
char *arg;
{
	int i,len=0;
	char *retval,*to,*from;
	char **argv;
	
	argv=&arg;

	for (i=0; argv[i]; i++)
		len += strlen(argv[i]);
	len++;			/* for null terminator */
		
	to = retval = Qalloc(len);

	for (i=0; argv[i]; i++)
		for (from=argv[i]; *from; *(to++) = *(from++))
			;
	*to=0;		/* null terminator */

	return(retval);
}
		

/* routines to fetch various parts out of an entry name */
/* if the entry name format is changed, these have to be changed, */
/* as well as outent(), which cooks up the name in the first place */

/* get queue name -- right after dot.  on unix, last char is sequence info */
char *getqn(enam)
char *enam;
{       static char buf[QNAME +1];
	register char  *q;

	char r[E_NAME];

	strcpy (r, enam);
	if( (q = strrchr (r, ':')) == NULL )	/* funny name in qdir */
		return("???????????????");
	q++;                /* skip over . */
	TRUNC_NAME( q, QNAME );
	strcpy (buf,q );
#ifdef DEBUG
	if(getenv("GETQN"))
		sysraw("getqn returning |%s|\n",buf);
#endif
	return(buf);
}

/* find the login name (truncated) */
char *getln(enam)
char *enam;
{       static char buf[MAXNAMLEN+1];
        register char *from, *p;
        register int numbytes;

        from = enam + 2;
        p = strrchr(from,':');
        if (p == NULL)
            syserr((int)EXITFATAL,GETMESG(MSGNOLG,"No login name found."));
        numbytes = p - from;
        strncpy(buf,from,numbytes);
        buf[numbytes] = '\0';
        return(buf);
}


/* are 2 entries the same? */
issame(e1,e2)
register struct e *e1, *e2;
{
	return( e1 != NULL &&
		e2 != NULL &&
		strcmp(e1->e_name, e2->e_name) == 0 &&
		e1->e_pri2 == e2->e_pri2 &&
		e1->e_time == e2->e_time
		);
}


/* return a character that might reasonably be in a file name */
/* must be one-to-one. */
tochar(n,bomb)
register n;
{       static char map[] =
"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,:+-=_~%$#!";

	if (n < sizeof(map))
	    return(map[n]);

	if (bomb)           /* number is too big */
	    syserr((int)EXITFATAL,"out of file names");
	else
	    return('\0');           /* impossible char */
}


/* return name of status file for device dev.  name is sD.QQQ */
/* in unix, assume we're in qdir, which makes name "../stat/sD.QQQ" */
/* finch32988, D means Device number (usually 0) since a queue can have*/
/* more than one device on it. All the data to construct this name is*/
/* stored in the d struct.*/
char *
stname(dev)
struct d *dev;
{
	static char name[MAXPATHLEN];

	sprintf(name,"/var/spool/lpd/stat/s.%s.%s",dev->d_q->q_name,dev->d_name);
	return(name);
}



/*==== Funeral Services (required by law) */
qexit(exitcode)
int exitcode;
{
	if (qkillfile == TRUE)
	{
		if (qdaemon == TRUE)
			unlink(QDPID);
		if (lpd == TRUE)
			unlink(LPD_LOCKNAME);
	}
	exit(exitcode);
}



/*==== Send a message to the correct place */
sysmsg(message) 
char	*message;	/* the message to print */
{
	char	buf[LINE_MAX];
	int	saverrno;
	int	sfd;

	buf[0] = '\0';
	saverrno = errno;

	/*----Open the console device if a daemon */
	if (qdaemon == TRUE || lpd == TRUE)
	{
		sfd = open(consnam,O_NOCTTY | O_WRONLY | O_NDELAY,0);
		if (sfd == -1)
		{
			sprintf(buf,GETMESG(MSGNOCO,
			        "%s: %s could not be opened for error message output.\n%s"),
			        progname,consnam,progname);
			perror(buf);
			return(0);
		}
	}

	/*----Combine the error message and the perror messages */
	if (saverrno)
        	sprintf(buf + strlen(buf),"%s%s: errno = %d: %s\n",message,progname,saverrno,strerror(saverrno));
	else
		strcpy(buf + strlen(buf),message);

	/*----Print the message itself */
	if (qdaemon == TRUE || lpd == TRUE)
	{
		write(sfd,buf,strlen(buf));
		close(sfd);
	}
	else
		fprintf(stderr,buf);

	/*----Backend error messages go to sysnot */
	if (backend == TRUE)
	{
		sysnot(get_from(),NULL,buf,get_mail_only() ? DOMAIL : DOWRITE);
		buf[0] = '\0';
		while ( NULL != *saveArgv )
		{
			strcat(buf, *saveArgv++);
			strcat(buf, " ");
		}
		sysnot(get_from(),NULL,buf,get_mail_only() ? DOMAIL : DOWRITE);
	}

	/*----Reset errno and return */
	errno = 0;
	return(0);
}


/*==== Fatal error message, and die                                          */
/*     Call:  syserr(<exitcode>, <message_format>, <thing1>, <thing2>, ...); */
syserr(va_alist)
va_dcl
{
        va_list args;
        char    buf[LINE_MAX];
	char	message[LINE_MAX];
        char    *fmt;
	int	exitcode;

        va_start(args);
	exitcode = va_arg(args,int);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	if(exitcode != EXITOK)
		sprintf(message,"%s: (%s): %s\n", progname, GETMESG(MSGFATA,"FATAL ERROR"), buf);
	else
		sprintf(message,"%s: %s\n",progname,buf);
        va_end(args);
	sysmsg(message);
	qexit(exitcode);
}


/*==== Warning error message                                      */
/*     Call:  syswarn(<message_format>, <thing1>, <thing2>, ...); */
syswarn(va_alist)
va_dcl
{
        va_list args;
        char    buf[LINE_MAX];
	char	message[LINE_MAX];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	sprintf(message,"%s: (%s): %s\n", progname, GETMESG(MSGWARN,"WARNING"), buf);
        va_end(args);
	sysmsg(message);
	return(0);
}



/*==== Usage message                                                            */
/*     Call:  sysuse(<TRUE or FALSE>,<usage_messageline1>, <usage_messageline2>, ..., (char *)0 ); */
sysuse(va_alist)
va_dcl
{
        va_list args;
        char    message[LINE_MAX];
	char	spaces[64];
        char    *uline;
	boolean exitbad;

        va_start(args);
	errno = 0;
	memset(spaces,' ',strlen(progname));
	spaces[strlen(progname)] = '\0';
	exitbad = va_arg(args,boolean);
	uline = va_arg(args,char *);
	sprintf(message,"%s: %s %s\n", GETMESG(MSGUSAGE,"usage"), progname, uline);
	while ((uline = va_arg(args,char *)) != (char *) 0)
	{
		/*----Same line, no progname */
                sprintf(message + strlen(message),"       %s %s\n", spaces, uline);
	}
        va_end(args);
	sysmsg(message);
	if(exitbad == TRUE)
        	qexit((int)EXITBAD);
	return(0);
}




/*==== Information message                                        */
/*     Call:  systell(<message_format>, <thing1>, <thing2>, ...); */
systell(va_alist)
va_dcl
{
        va_list args;
        char    buf[LINE_MAX];
	char	message[LINE_MAX];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	sprintf(message,"%s: %s\n", progname, buf);
        va_end(args);
	sysmsg(message);
	return(0);
}



/*==== Raw, unformatted information message (for debugging statements) */
/*     Call:  sysraw(<message_format>, <thing1>, <thing2>, ...);       */
/*     NOTE: no line feed added                                        */
sysraw(va_alist)
va_dcl
{
        va_list args;
        char    buf[LINE_MAX];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
        va_end(args);
	errno = 0;
	sysmsg(buf);
	return(0);
}



/* return last mod time this file -- -2 if not there or -1 if stat failed due to some other error */
time_t modtime(str)
char *str;
{       struct stat statb;
        extern int errno;

        if (stat(str,&statb) == -1)
            if ( errno == ENOENT )
                return(-2);
            else return(-1);
        return(statb.st_mtime);
}


/* remember the name of a file so can delete it later.  some might */
/* already have been deleted; that's ok -- our process id is in the name, */
/* so no one else will have made a file by that name anyway.  */
/* if called with the arg DELETE, we */
/* delete all the files we have remembered. */
/* this may cause us to delete copies of files for which a queue request */
/* has already been grabbed -- this is ok, since if backend can't find */
/* a file, it just dies anyway (its presence and readability having been */
/* established by us before we made the request). */
remember(tnam)
register char *tnam;
{
	static struct tnlist
	{
		struct tnlist *tn_next;
		char tn_name[MAXPATHLEN];
	} *tnlist;

	if (tnam == DELETE)
	{   /* unlink everything we've got */
	    for (; tnlist; tnlist = tnlist->tn_next)
	    {
#ifdef DEBUG
		if( getenv("REMEMB") )
			sysraw("unlink(%s)\n",tnlist->tn_name);
#endif
		unlink(tnlist->tn_name);
	    }
	}
	else
	{   struct tnlist *this;
	    this = (struct tnlist *)calloc((size_t)1,(size_t)sizeof(struct tnlist));
	    if (this == NULL)               /* calloc failure */
	    {   
		/*
		 * Qalloc insufficient since Qalloc may exit on failure
		 * and tnam wouldn't get deleted by remember(DELETE)
		 */
		if (gotint == TRUE)
		{   unlink(tnam);
		    terminate();
		}
		else
		    syserr((int)EXITFATAL,GETMESG(MSGOUTO,"Insufficient space for file retention."));
	    }
#ifdef DEBUG
		if( getenv("REMEMB") )
			sysraw("remember(%s)\n",tnam);
#endif
	    strcpy(this->tn_name,tnam);
	    this->tn_next = tnlist;
	    tnlist = this;
	}
}


void *Qalloc(size)
size_t size;
{       
	register char *ans;

	ans = malloc(size);
	if ( NULL != ans ) {
		bzero(ans,size);
	}
	else {
		if (!qdaemon)	/*GLOBAL CONROL FLAG*/
		{
			if (gotint == TRUE)     /*int caused sbrk failure?*/
				terminate();
			else
			    	syserr((int)EXITFATAL,GETMESG(MSGOUTS,"Insufficient memory."));
		}
		else
			syswarn(GETMESG(MSGOUTS,"Insufficient memory."));
	}
	return(ans);
}


/* like chdir, but gives error message and dies */
cd(dir)
register char *dir;
{
	if (chdir(dir) == -1)
		    syserr((int)EXITFATAL,GETMESG(MSGERCD,"Cannot cd to %s."),dir);
}


/* rename; return FALSE if new name already in use */
renamefile(old,new)
register char *old, *new;
{
	if (link(old,new) == -1)
	    return(FALSE);
	unlink(old);
	return(TRUE);
}


/* compare priorities of 2 entries.  -1 means first is higher, */
/* 0 means they're equal, +1 means second is higher */
/* pri1 is highest first, pri2 is lowest first */
cmppri(e1, e2)
register struct e *e1, *e2;
{
	if (e1->e_pri1 > e2->e_pri1)
	    return(-1);
	if (e1->e_pri1 < e2->e_pri1)
	    return(1);

	/* now pri1 is equal in both entries */
	if (e1->e_pri2 < e2->e_pri2)
	    return(-1);
	if (e1->e_pri2 > e2->e_pri2)
	    return(1);


	/*
	 * This is redundant for FCFS queues, but it makes
	 * SJN queues FCFS when job sizes are the same.
	 */
	if (e1->e_time < e2->e_time)
	    return(-1);
	if (e1->e_time > e2->e_time)
	    return(1);
	
	/*
	 * if times are the same sort by their queued order
	 */
	if (e1->e_qorder < e2->e_qorder)
	    return(-1);
	if (e1->e_qorder > e2->e_qorder)
	    return(1);
	return(0);
}


/*
 * rename the file tnam to realnam.  this is hard because there might
 * already be one, so we use sequence letters to avoid duplicates.
 * return the name we finally succeeded with, or NULL on failure.
 */
char *qentry(tnam, realnam)
char *tnam, *realnam;
{       register char mi, ni;   /* sequence numbers */
	register char mc, nc;   /* chars to try */
	register int m, n;      /* positions of 2 seq chars */
	extern int errno;

	/* find what chars are the sequence chars */
	if (realnam[0] == 'r')          /* special request */
	{   m = 1;
	    n = 2;
	}
	else
	{   m = 0;
	    n = 1;
	}

	/* now try everything */
	for (mi = 0; mc = tochar(mi,TRUE); mi++)
	{   if (mc == 'r')
		continue;        /* looks like spec req */
	    realnam[m] = mc;
	    for (ni = 0; nc = tochar(ni,FALSE); ni++)
	    {   realnam[n] = nc;
		if (renamefile(tnam,realnam))
		    return(realnam);
		if (errno == EEXIST)        /* else failure */
		    continue;
		return(NULL);       /* some other error */
	    }
	}
}


/* get first len-1 bytes of next line, null-terminate */
getline(linep,len,filep)
register char  *linep;
register FILE  *filep;
{       register int  c;
	register char *eline;

	eline = linep + len - 1;

	while ((c = fgetc(filep)) != EOF)
	{   if (c == '\n')
	    {   *linep = 0;
		return(TRUE);
	    }
	    if (linep < eline)
		*linep++ = c;
	}
	return(FALSE);
}


/* get a null separated, double-null terminated array */
/* used for pcred and penv lists */
char **
getarray(filep)
register FILE  *filep;
{       
#define NONULL	0
#define ONENULL 1
#define TWONULL 2
	register int  c;
	register char  **listp,**retp;
	char	*linep,*orgp;
	int	count;
	int	qsize;
	int 	state;
	boolean	done;
	int	argcount;

	count = 0;
	argcount = 0;
	state = NONULL;
	done = FALSE;
	qsize = sizeof(char) * QELINE;
	linep = (char *) Qalloc(qsize);
	do
	{
		/*----Get the character */
		c = fgetc(filep);

		/*----Handle according to state */
		switch(state)
		{
		case NONULL:
			switch(c)
			{
			case EOF:
				done = TRUE;
				break;
			case '\0':
				state = ONENULL;
				argcount++;	/* update count for malloc below */
			default:
				if (count == (qsize - 1))
				{
					qsize += sizeof(char) * QELINE;
					linep  = (char *)realloc((char *)linep, qsize); 
				}
				linep[count] = c;	/* store the character, and go on */
				count++;
			}
			break;

		case ONENULL:
			switch(c)
			{
			case EOF:
				done = TRUE;
				break;
			case '\0':
				state = TWONULL;
				linep[count++] = c;	/* store the null character */
				break;
			default:
				state = NONULL;
				if (count == (qsize - 1))
				{
					qsize += sizeof(char) * QELINE;
					linep  = (char *)realloc((char *)linep, qsize); 
				}
				linep[count] = c;	/* store the character, and go on */
				count++;
			}
			break;

		case TWONULL:
			done = TRUE;
			break;
		}
	} while (done == FALSE);

	/*----Error if anything but a newline character after two NULLs */
	if ('\n' != c || TWONULL != state)
		return(NULL);

	/* allocate space for list of char pointers */
	/* argcount + 1 for the QUEUE_BACKEND env var + null at end */
	if ( (listp = (char **)Qalloc((size_t)((argcount + 2) * sizeof(char *)))) == NULL)
		return(NULL);

	retp = listp;

	/* reset pointer to the list (in string form)*/
	orgp = linep;

	/* loop until double-null */
	while ( '\0' != *linep )
	{
		*listp = (char *)Qalloc((size_t)(strlen(linep)+1));
		strcpy(*listp++,linep); /* copy the next string */
		while (*linep++); 	/* skip to next string (null) */
	}

	*listp = NULL;	/* terminate the pointer array */

	free((void *)orgp);

	return(retp);
}


char *rindex(cp, c)
	register char *cp;
	int c;
{
	register char *ret;

	ret = 0;
	do {
	    if (*cp == c) ret = cp;
	} while (*cp++);
	return(ret);
}

redigest()
{
	char *bconfig, *aconfig;
	char bc[MAXPATH], ac[MAXPATH];
	char error_msg[LINE_MAX];
	int pid, status=0, got, offset ;
	int p[2];

	msgbuf[0] = '\0';

	memset(error_msg, 0, LINE_MAX);

	bconfig = bc;
	aconfig = ac;

	bconfig = BCONFIG;      /* qconfig.bin */
	aconfig = CONFIG;       /* qconfig */

	if (-1==pipe(p))
		syserr((int)EXITFATAL,GETMESG(MSGPIPE,"Digest pipe error."));

	switch( (pid = fork()) )
	{
	case -1:
		syserr((int)EXITFATAL,GETMESG(MSGFORK,"Cannot fork for %s."),DIGEST);

	case 0:                          /* child */
		close(2);                /* close stderr */
		dup(p[1]);
		close(p[0]);
		close(p[1]);

		execl(DIGEST,"digest",aconfig,bconfig,0);
		syserr((int)EXITFATAL,GETMESG(MSGEXEC,"Cannot exec digester %s."),DIGEST);

	default:                        /* parent */
		close(p[1]);
		offset = 0 ;
		error_msg[ LINE_MAX -1 ] = '\0' ;
		while ( got = read(p[0], error_msg + offset, LINE_MAX - 1 - offset ) )
			if ( got == -1 )
				if ( errno == EINTR )
					continue ;
				else
					break ;
			else
				offset += got ;

		errno = 0;

		while( (got = waitpid(pid,&status,0)) != pid )
		{
			if( got == -1 && errno == EINTR )
			{
				/*
				* Wait was interrupted by a signal, probably
				* a new incoming request.  Ignore it.
				*/
				errno = 0;
				continue;
			}
			syserr((int)EXITFATAL,GETMESG(MSGWPID,"Redigest wait: wrong pid."));
		}
		close(p[0]);

	}

	if( status )
	{
		sysraw(error_msg);
		syserr((int)EXITFATAL,GETMESG(MSGDIGE,
			"Error from digester %s, status = %d, rv = %d."),
			DIGEST,status,got);
	}
	return(0);
}

badtmp(dir)
char *dir;
{
	systell(GETMESG(MSGEDIR,"Error %d writing directory %s."), errno, dir);
	syserr((int)EXITFATAL,GETMESG(MSGCTMP,"Can't create temporary file"));
}

char * grjobnum();

/* Look in the jdf at the jobnum rec and compare. Return with open file.  */
boolean rightjob1(looking4jnum,qefil,actual_jobnum)
int	looking4jnum;		/* <- */
FILE 	*qefil;			/* <- */
long	*actual_jobnum;		/* -> */ 
{
	struct e estruct, *e = &estruct;
	int rc;
	boolean answer=FALSE;

	bzero((char *)e,sizeof(struct e)); 	/* nuke e struct */

	if (grjobnum(e,qefil,&rc))
		answer = FALSE;

	if (looking4jnum == e->e_jobnum)
		answer = TRUE;
	else
		answer = FALSE;			/* wrong jobnumber */

	if (actual_jobnum )
		*actual_jobnum = e->e_qorder;  

	return(answer);
}

/*====Separate an argument string into argv array */
int			/*----Returns number of args found */
separate(outvec, instr)
char	**outvec;	/*----Vector to put args into (be sure to allocate mem) */
char	*instr;		/*----String to tear apart */
{

/*----States */
#define SFBEGIN 1	/*----Search For BEGINning of argv */
#define SFENDNQ	2	/*----Search For END, Not in Quotes */
#define SFENDIQ 3	/*----Search For END, In Quotes */
#define DONE	4	/*----End of string reached */

	int	index;		/*----Running count of args */
	int	pos;		/*----Position in input string */
	int	begin;		/*----Beginning of new argv */
	int	end;		/*----End of new argv */
	int	state;		/*----State we are in */

	/*----Init */
	index = 0;
	pos = 0;
	state = SFBEGIN;

	/*----Scan each character in input string and process */
	while(state != DONE)
	{
		switch(instr[pos])
		{
		case ' ':
		case '\t':
			if(state == SFENDNQ)
			{
				end = pos;
				outvec[index++] = sncopy(&instr[begin],end - begin);
				state = SFBEGIN;
			}
			break;
		case '\0':
		case '\n':
			switch(state)
			{
			case SFENDIQ:
			case SFENDNQ:
				end = pos;
				outvec[index++] = sncopy(&instr[begin],end - begin);
			case SFBEGIN:
				state = DONE;
			}
			break;
		case '\"':
		case '\'':
			switch(state)
			{
			case SFENDIQ:
				end = pos;
                                outvec[index++] = sncopy(&instr[begin],end - begin);
				state = SFBEGIN;
				break;
			case SFENDNQ:
				end = pos;
                                outvec[index++] = sncopy(&instr[begin],end - begin);
			case SFBEGIN:
				begin = pos + 1;
				state = SFENDIQ;
			}
			break;
		default:			/*----Regular characters */
			if(state == SFBEGIN)
			{
				begin = pos;
				state = SFENDNQ;
			}
		}
		++pos;
	}
	outvec[index] = NULL;
	return(index);
}
			
char * thishost()
{
	static int gotten = FALSE;
	static char host[HOST_SZ];

	if (!gotten)
		if ( gethostname(host,HOST_SZ)!=0){
			syserr((int)EXITFATAL,GETMESG(MSGUHST,"Unable to get host name."));
		}
	if ( 0 == strlen(host) )
		strcpy(host,"localhost");
	return(host);
}

char *getmesg(num,str)
int num;
char *str;
{
	char *p, *dest;
	nl_catd catd;

	catd = catopen(MF_LIBQ, NL_CAT_LOCALE);
	p = catgets(catd,MS_LIBQ,num,str);
	dest = Qalloc(strlen(p)+1);
	strcpy(dest,p);
	catclose(catd);
	return(dest);
		
}

