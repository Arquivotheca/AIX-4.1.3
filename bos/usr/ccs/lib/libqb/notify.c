static char sccsid[] = "@(#)63	1.16  src/bos/usr/ccs/lib/libqb/notify.c, libqb, bos411, 9428A410j 2/3/93 15:50:43";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: sysnot, csysnot, telluser
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <IN/standard.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <IN/backend.h>
#include "notify.h"
#include <ctype.h>
#include <nl_types.h>
#include "libqb_msg.h"
#define MAXSTR 		10

#define	WRITPATH	"/usr/bin/write"
#define MAILPATH	"/usr/bin/mail"
#define NULLPATH	"/dev/null"

char * get_from();

/*----SEND A MESSAGE TO THE USER WHO ORIGINATED THE JOB */
/*
sysnot(sn_user,sn_host,sn_msg,sn_pref)
        Sends a sn_msg to sn_user@sn_host using the method of your preference.
        DOMAIL always uses mail.  DOWRITE tries to use the write command for
        more immediate notification, and if the write command fails, (with a
        nonzero return code,) sysnot will try the mail command also.  In future
        releases of AIX, other communication programs may also be used.
        If sn_host is null, the current host is used.   This routine allows the
        backend to determine for itself which messages should go to the user,
        which to the system administrator and which to both.
*/
sysnot(sn_user,sn_host,sn_msg,sn_pref)
register char	*sn_user;
char		*sn_host;
char		*sn_msg;
unsigned	sn_pref;
{
	int	sn_fresult;		/* result of fork call */
	int	sn_waitstat = 0;	/* wait termination status of child */
	    
#ifdef DEBUG
	if(getenv("WRT"))
	{
		fprintf(stderr, "sysnot: user    = [%s]\n", sn_user);
		fprintf(stderr, "        host    = [%s]\n", sn_host);
		fprintf(stderr, "        pref    = [%d]\n", sn_pref);
		fprintf(stderr, "        message = [%s]\n", sn_msg);
	}
#endif

	fflush(stdout);		/* Flush to avoid duplication of output */
	fflush(stderr);
	sn_fresult = kfork();
	switch(sn_fresult) {
	case 0:
		/*----Child: Send message different ways until it gets there */
		switch(sn_pref) {
		case DOWRITE:
			csysnot(DOWRITE,sn_user,sn_host,sn_msg);
		case DOMAIL:
			csysnot(DOMAIL,sn_user,sn_host,sn_msg);
		}
		/*----Error, print message, then return */
		fprintf(stderr,MSGSTR(MSGEXEC,QMSGEXEC));
		_exit(EXITOK);

	case -1:
		/*----Error, print message, then return */
		fprintf(stderr,MSGSTR(MSGFORK,QMSGFORK));
		return(0);

	default:
		/*----Parent, just wait and return, regardless of what happened */
                waitpid(sn_fresult,&sn_waitstat,0);
		return(0);
	}
}

/*----SEND A MESSAGE USING SELECTED SYSTEM (CHILD OF SYSNOT) */
csysnot(cs_action,cs_user,cs_host,cs_msg)
unsigned	cs_action;
char		*cs_user;
char		*cs_host;
char		*cs_msg;
{
	int	cs_pipe[2];
	char	cs_node[30];	/*???????????????????*/
	int	cs_waitstat = 0;
	int	cs_fresult;
	char	*cs_hdr;

	/*----Set up user node id for proper parameters for write and mail */
	if ((cs_host ) && (cs_host[0]))
		sprintf(cs_node,"%s@%s",cs_user,cs_host);
	else
		sprintf(cs_node,"%s",cs_user);

	/*----Set up pipe and send message */
	pipe(cs_pipe);
	fflush(stdout);		/* Flush to avoid duplication of output */
	fflush(stderr);
	cs_fresult = kfork();
	switch(cs_fresult) {
	case -1:
		/*----Error, print message */
			fprintf(stderr,MSGSTR(MSGFORK,QMSGFORK));
			return(0);

	case 0:
	        /*----Child: Try write with redirected stdin */
        	close(0);
        	dup(cs_pipe[0]);
        	close(cs_pipe[0]);
        	close(cs_pipe[1]);

        	/*----Redirect sdtout and stderr to the land of nothing
        	      so that write command does not send error msgs to printer */
        	close(1);
        	open(NULLPATH,O_WRONLY);
        	close(2);
        	open(NULLPATH,O_WRONLY);

                /* /bin/write may hang if utmp file is corrupted. set a timer
                 * so if it is hanging, timer will send it a signal which will
                 * kill the /bin/write or mail!
                 */
                #define TIMEOUT_L 10 /* 10 seconds */
                alarm(TIMEOUT_L);

        	/*----Do what is chosen */
        	switch (cs_action) {
        	case DOWRITE:
        	        execlp(WRITPATH,WRITPATH,cs_node,0);
        	case DOMAIL:
        	        execlp(MAILPATH,MAILPATH,cs_node,0);
			/*----All failed if we get here */
			_exit(EXITEXEC);
		}

	default:
		/*----Parent: Pipe message to child */
		cs_hdr = MSGSTR(MSGMHDR,QMSGMHDR);
		close(cs_pipe[0]);
		write(cs_pipe[1],cs_hdr,strlen(cs_hdr));
		write(cs_pipe[1],cs_msg,strlen(cs_msg));
		close(cs_pipe[1]);
		wait(&cs_waitstat);
		if(cs_waitstat == 0)
			exit(EXITOK);
		return(0);
	}
}


/*
telluser(msg)
        sends a message to the user@host as shown in the get_from() subroutine.
        uses the value of get_mail_only() to determine how to send it.
*/
telluser(msg)
char * msg;
{
	char from[40000];
	int pref;
	char *c,*host;

	if (get_backend())
	{
		strcpy(from,get_from());
		for (c=from, host=NULL; *c ; c++)
		{
			if (*c=='@')
			{
				host=c+1;
				*c=0;
				break;
			}
		}
		if (get_mail_only())
			pref=DOMAIL;
		else
			pref=DOWRITE;

		sysnot(from,host,msg,pref);
	}
	else
		fprintf(stderr,MSGSTR(MSGTELL,
			"0781-266  Cannot use telluser() unless you are a backend."));

	return(0);
}
