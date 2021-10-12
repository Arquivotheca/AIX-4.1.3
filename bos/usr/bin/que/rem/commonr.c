static char sccsid[] = "@(#)73	1.22.1.2  src/bos/usr/bin/que/rem/commonr.c, cmdque, bos41J, 9520A_a 2/23/95 12:52:46";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	See commonr.h for BSD lpr/lpd protocol description.*/
	
#include "commonr.h"
#include <fcntl.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <stdio.h>
#include <time.h>
#include <varargs.h>
#include "outr.h"
#include "lpd.h"
#include <ctype.h>

#include "rem_msg.h"
#define MSGSTR(num,str)	catgets(catd,MS_REM,num,str)
nl_catd	catd;

#ifdef DEBUG
extern FILE *outfp;
#endif


int timeout_ack=0;		/* This variable is exported in paramr.c
				 * and set when the T flag is specified.
				 */

char * myhost()
{
	static int gotten = FALSE;
	static char host[HOST_SZ];

	if (!gotten)
		if ( gethostname(host,HOST_SZ)!=0)
			syserr((int)EXITFATAL,"gethostname");		/* sanity */

	host[HOST_SZ-1] = '\0';	/* This null termination is required if there */
				/* was not enough space allocated for the */
				/* gethostname function. The documentation */
				/* for gethostname says that if there is */
				/* enough space the gethostname function will */
				/* null terminate. If there is not enough */
				/* space, it will simply truncate it and it */
				/* will not null terminate the string. */
	return(host);
}

/* return a character that might reasonably be in a file name */
/* must be one-to-one.  fromchar() is reverse of this; beware of changing */
/* one without the other */
numtochar(n)
unsigned int n;
{       static char map[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,:+-=_~%$#!";

	if (n < sizeof(map))
	    return(map[n]);
	else
	    return('A');           
}

/*
* Create a tmp file and place the name in str.  If
* dir is !NULL, then create in that directory.
*/
int gettmpr(dir,str)
register char *dir, *str;
{
	int 		retries, fd;

	/* prime the random number pump */

	for( retries = 0; /* null */; retries++ )
	{
		/* generate a psuedo-unique name */

		if (dir != NULL)
			sprintf(str,"%s/rembakXXXXXX",dir);
		else
			sprintf(str,"tXXXXXX" );

		mktemp(str);

		/* 
		* Try to create it exclusive access, this fails if 
		* the file already exits as opposed to creat().
		*/

		if( (fd = open(str,O_RDWR|O_CREAT|O_EXCL,0640)) < 0 )
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
	return(fd);	/* success */
}

int gettmpfile(where,fn)
char * where;
char  **fn;
{
	
	char fname[MAXPATHLEN];
	int fd;

	if ((fd=gettmpr(where,fname))<0)
		syserr((int)EXITFATAL,MSGSTR(MSGTFIL,"Could not create tempfile in directory %s name %s."),
			where,fname);
	else
	{ 	*fn=scopy(fname);
		return(fd);
	}
}
		
ack(skfd)
int skfd;
{
	if (write(skfd,"",1)==1)
			return(1);
		else
			return(0);
}


/*
 * catch alarm.  just want read() to fail to avoid waiting indefinately
 * for a response from the server.
 */
int
gotalarm(void)
{
        signal(SIGALRM,(void (*)(int))gotalarm);
}


int gotack(skfd)
int skfd;
{
        char ack;

#ifdef DEBUG
	if(getenv("GOTACK"))
	{
		fprintf(outfp,"gotack: timeout = %d\n",timeout_ack);
		fflush(outfp);
	}
#endif  

	if ( timeout_ack ) {
	    signal(SIGALRM,(void (*)(int))gotalarm);	/* handle alarms */
	    alarm(timeout_ack);
	}
        if (read(skfd,&ack,1) == 1)
        {
#ifdef DEBUG
                if(getenv("GOTACK"))
		{
                        fprintf(outfp,"gotack: ack = %d\n",ack);
			fflush(outfp);
		}
#endif  
		if ( timeout_ack ) {
		    alarm(0);
		    signal(SIGALRM,SIG_DFL);	/* disable handler */
		}
                if (ack == '\0')
                        return(1);
                else
                        return(0);
        }
        else
	{
		if ( timeout_ack ) {
		    alarm(0);
		    signal(SIGALRM,SIG_DFL);	/* disable handler */
		}
#ifdef DEBUG
               	if(getenv("GOTACK"))
			{
               		        fprintf(outfp,"gotack: bad gotack");
				fflush(outfp);
			}
#endif  
                return(0);
	}
}

/* returns current username,  caches it for multiple calls since we all
   know that the SECURITY software is SO UNBELIEVABLY SLOW!
*/
char * getusername()
{
	static struct passwd *pp;
	static boolean alreadygotten=FALSE;
	int gu;

	if (alreadygotten)
		return(pp->pw_name);

	gu=getuid();
	errno=0;
	if ((pp=getpwuid((uid_t)gu))==NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGUNAM,"Cannot determine username for uid = %d."),gu);
	
	alreadygotten=TRUE;
	return(pp->pw_name);
}

char *getclass(c)
struct comline 	*c;
{
	if (c->c_Hflg)
		return(c->c_Hrem);
	return(myhost());
}

/* Returns TRUE if user used the -B option to override default burst options */
int user_burst(cmdline)
char *cmdline;
{
	int i;
	
	for (i=0; cmdline[i]; i++)
		if ( (cmdline[i] == '-') && (cmdline[i+1] == 'B') )
			return (TRUE);
	return(FALSE);
}
