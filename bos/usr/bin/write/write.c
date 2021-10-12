static char sccsid[] = "@(#)14	1.16.1.7  src/bos/usr/bin/write/write.c, cmdwrite, bos411, 9433B411a 8/16/94 16:47:57";
/*
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: write
 *
 * ORIGINS: 3, 26, 27
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
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#define _ILS_MACROS
#include <errno.h>
#include <stdio.h>
#include <locale.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <utmp.h>
#include <pwd.h>
#include <sys/limits.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include "write.h"
#include "write_msg.h" 
nl_catd  catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(Num,Str) catgets(catd,MS_write,Num,Str)

extern errno;

#define USAGE MSGSTR(M_MSG_35 \
   ,"write [-r|-q|-h handle,%s|%s|%s] [-n host] [user][@host] [line]\n")

#define BUF_LEN 5*MAX_INPUT		/* a 5-byte octal sequence is
					   displayed for non-printable
					   chars so the buffer must be able
					   to handle the expansion */
					   

struct flag {	
  short int nflag;       /* remote node flag */
  short int rflag;       /* wait for rely flag */
  short int qflag;       /* query flag */
  short int hflag;       /* handle flag */
  long int handle;       /* handle  */
};

struct userinfo {
  char host[MAX_HOST_LEN];
  char user[MAX_USERID_LEN+1]; 
  char tty[PATH_MAX];
  int fldes;
};

void	getoptions(),validoptions(),gettargetinfo(),openlink(),exshell(),
	sendreply(),srvinfo(),getuserinfo(),myread(),mywrite(),uzero(),
	timeread();

int sigpipe(void);
int timeout(void);
struct utmp *getutent();

/*
 * NAME: write [-r] [-q] [-h handle,reply] [-n host] [user][@host] [line]
 * FUNCTION: Sends messages to other users on the system.
 * RETURN VALUE DESCRIPTION: exits with 0 if everything is ok
 *                           exits with 1 if reply = "cancel"
 *                           exits with -1 if error.
 */  

        /*      
        *       change from old style signals (call to signal) to
        *       call sigaction. From ksh/sh/fault.c
        */

void (*new_signal( int sig, void (*func)(int))) (int)
{
        struct sigaction        act, oact;

        sigaction(sig, (struct sigaction *)NULL, &act); /* get current signal */
        act.sa_handler = func;                  /* set new handler */
        switch (sig) {
        case SIGCONT:
        case SIGTTIN:
        case SIGTTOU:
        case SIGTSTP:
        case SIGSTOP:
                /* system calls are restartable after a job control signal */
                act.sa_flags |= SA_RESTART;
                break;
        }
        sigaction(sig, &act, &oact);
        return(oact.sa_handler);                /* return old signal handler */
}


main(argc,argv)
int argc;
char *argv[];
{
	struct flag flags;
	struct userinfo userinfo,targetinfo;	
	char message[BUF_LEN];
	char msg2[BUF_LEN + 1];
  	char *host;
	int reply = 0, length;
	char *eof = NULL;
	char mbchar[10];		/* to store partial multibyte
					   char from fgets if necessary */
	int mberror = 0, offset = 0;	/* always 0 if MB_CUR_MAX == 1 */
        struct hostent *hp;
        char user_name[MAX_HOST_LEN];
        char target_name[MAX_HOST_LEN];

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_WRITE, NL_CAT_LOCALE);

	new_signal(SIGCONT, SIG_IGN);	/*
					 * This keeps read from failing
					 * it may not be necessary in the future
					 */
	new_signal(SIGPIPE,(void (*)(int))sigpipe);
	getoptions(argc,argv,&flags,&targetinfo,message);
	validoptions(flags,targetinfo);
	getuserinfo(&userinfo);	
	if (targetinfo.host[0] != '\0')
		host = targetinfo.host;
	else
		host = userinfo.host;
	if (flags.hflag) {
		sendreply(flags.handle,encoder(message),host);
		exit(0);
	}
	if (flags.qflag) {
		srvinfo(host,userinfo.host);
		exit(0);
	}
	/* This section of code is added to prevent "write -n localhost user"
           go thru writesrv, this enable local root write to local user
	   regardless of what local user mesg is set to.  D48768 */
        if (flags.nflag) {
                if ((hp = gethostbyname(userinfo.host)) == NULL) {
                    fprintf(stderr,MSGSTR(M_MSG_26,"write: unknown host\n"));
                    exit(-1);
                }
                strncpy(user_name, hp->h_name, sizeof(user_name));
                if ((hp = gethostbyname(targetinfo.host)) == NULL) {
                    fprintf(stderr,MSGSTR(M_MSG_26,"write: unknown host\n"));
                    exit(-1);
                }
                strncpy(target_name, hp->h_name, sizeof(target_name));
                if (!strcmp(user_name, target_name))
                        flags.nflag--;
        }
	gettargetinfo(&targetinfo,&flags);
	if (targetinfo.host[0] == '\0')
		strcpy(targetinfo.host,userinfo.host);
	openlink(&userinfo,&targetinfo,&flags);
	printf("");
	do {
		bzero(message,sizeof(message));
		bzero (msg2, sizeof(msg2));
		if (mberror)		/* previous fgets split a multibyte char */
			strcpy (message, mbchar);
		if ((eof = fgets((message+offset),(MAX_INPUT - offset),stdin)) == NULL)
			strcpy(message,MSGSTR(M_MSG_39,"<EOT>\n"));
		if (message[0] == '!' && flags.rflag == 0) {
			exshell(message);
			continue;
		}
		mberror = expand_control ( message, strlen(message), msg2 );  /* expand control characters */
		offset = 0;
		if ( mberror ) {
			/* fgets returns at most MAX_INPUT - 1 chars */
			offset = MAX_INPUT - mberror - 1;
			strncpy (mbchar, (message+mberror), (size_t)offset);
		}
		length = strlen (msg2);
		mywrite (targetinfo.fldes,msg2,((length==0)?1:length));
	 } while (eof != NULL);
	if (flags.nflag || flags.rflag ) {
		bzero(message,sizeof(message));
		message[0] ='\0';
		mywrite (targetinfo.fldes,message,sizeof(message));
	}
	if (flags.rflag)
		reply = getreply(targetinfo.fldes);
	close(targetinfo.fldes);
	exit(reply);	
}

/*
 * NAME: getoptions
 * FUNCTION: get the options from the command line and set the proper
 *           flags and target information.  calls Usage if an error is 
 *           detected.
 */
void getoptions(argc,argv,flags,tarinfo,message)
int argc;
char *argv[];
struct flag *flags;
struct userinfo *tarinfo;
char *message;
{
	int j,k;
	extern int optind;
	extern char *optarg;
	int c;
	char *h;

	if (argc < 2) {
		Usage();
	}
	flags->nflag = 0;    /* initialize flags */
	flags->rflag = 0;
	flags->qflag = 0;
	flags->hflag = 0;
	flags->handle = 0;
	tarinfo->user[0] = '\0';
	tarinfo->host[0] = '\0';
	tarinfo->tty[0] = '\0';
	tarinfo->fldes = 0;
	message[0] = '\0';

	while ((c = getopt(argc,argv,"n:rqh:@:")) != EOF) {
		switch (c) {
			case 'n':
				strcpy(tarinfo->host,optarg);
				flags->nflag++;
				break;
			case 'r':
				flags->rflag++;
				break;
			case 'q':
				flags->qflag++;
				break;
			case 'h':
				flags->hflag++;
				j = 0;
				h = optarg;
	    			while (h[j] != '\0'){
				  	if (h[j] == ',') { 
			            		message[j] = '\0';
				    		flags->handle = atoi(message);
				    		j++;
						break;
				  	}
				  	else {
				    		message[j] = h[j];
				    		j++;
				  	}
				}
				strcpy(message,h+j);
				break;
			case '@':
				tarinfo->user[0] = '-';
				strcpy(tarinfo->host,optarg);
				flags->nflag++;
				break;
			default:
				Usage();
			break;
		}
	}		

	for (; optind < argc; optind++){
		if (tarinfo->user[0] != '\0') {    /* get tty */
			if (tarinfo->tty[0] != '\0') Usage();
	    		strcpy(tarinfo->tty,argv[optind]);
	   		continue;
	  	}
	  	if (argv[optind][0] == '@') {
			if (argv[optind][1] == '\0') Usage();
			flags->nflag++;
			if (tarinfo->host[0] != '\0') Usage();
			strcpy(tarinfo->host,argv[optind] + 1);
			continue;
	  	}
	  	k=0;
	  	while (argv[optind][k] != '\0'){
	    		if (argv[optind][k] == '@') {  /* remote host */
	      			flags->nflag++;
	      			if (tarinfo->host[0] != '\0') Usage();
		  		strcpy(tarinfo->host,argv[optind] + k + 1);
		  		argv[optind][k] = '\0';
		  		break;
	      		}
	      		k++;
	  	} /* end of while loop */
	  	if (k > 0) {
	    		if (tarinfo->user[0] != '\0') Usage();
			strcpy(tarinfo->user,argv[optind]);
	    	}
	} /*end of for loop */
} /* end of getoptions */

/*
 * NAME: Usage
 * FUNCTION:  prints the usage statement to stderr and then exits.
 */
Usage()
{
	fprintf(stderr,USAGE,"ok","cancel","query");
	exit(-1);
}

/*
 * NAME: validoptions
 * FUNCTION: Checks for incompatible flags.  If incompatible options are
 *           found then validoptions will issue an error message and exit
 * 	     the program.
 */
void validoptions(flags,targetinfo)
struct flag flags;
struct userinfo targetinfo;
{
	if (flags.rflag + flags.qflag + flags.hflag > 1)
		Usage();
	if (flags.hflag + flags.qflag > 0 && targetinfo.user[0] != '\0')
		Usage();
	if (targetinfo.user[0] == '-' && targetinfo.tty[0] == '\0')
		Usage();
	if (flags.rflag > 0 && targetinfo.user[0] == '\0')
		Usage();
	if (flags.rflag + flags.qflag + flags.hflag == 0 
				&& targetinfo.user[0] == '\0')
		Usage();
	if (flags.nflag > 0 && targetinfo.host[0] == '\0')
		Usage();
	if (flags.hflag > 0 && flags.handle == 0)
		Usage();
	if(flags.hflag > 0 && targetinfo.tty[0] != '\0')
		Usage();
}

/*
 * NAME: getuserinfo
 * FUNCTION:  This function will get the user information (i.e. user id,
 *   host name and tty).
 */
void getuserinfo(userinfo)
struct userinfo *userinfo;
{
	struct passwd *pbuf;
	char *tty;

	pbuf = getpwuid(getuid());    /* get user id */
	if (pbuf == NULL) {
		fprintf (stderr, MSGSTR(M_MSG_2, 
			"write: unable to find you login ID\n") );
		exit(-1);
	}
	strcpy(userinfo->user,pbuf->pw_name);	
	tty = ttyname((int)fileno(stdin));
	if (tty == NULL)
		tty = ttyname((int)fileno(stdout));
	if (tty == NULL)
		tty = ttyname((int)fileno(stderr));
	if (tty == NULL) {
		fprintf(stdout, MSGSTR(M_MSG_3, 
			"write: unable to determine your tty\n") );
		strcpy(userinfo->tty,"/dev/");
		strncat(userinfo->tty,MSGSTR(UNKNOWN,"UNKNOWN"),(size_t)(MAX_TTY_LEN-5));
	}
	else
		strncpy(userinfo->tty,tty,(size_t)MAX_TTY_LEN);
	if (gethostname(userinfo->host,sizeof(userinfo->host)) < 0) {
		fprintf(stderr, MSGSTR(M_MSG_4, 
			"write: unable to determine your host name\n") );
		exit(-1);
	}
	userinfo->fldes = fileno(stdin);
}

/*
 * NAME: encoder
 * FUNCTION:  encode the message into 0 for 'ok'
 *                                    1 for 'cancel'
 *				      2 for 'query'
 *            exit if anything else
 */
int encoder(message)
char *message;
{
	if (message[0] == 'o' || message[0] == 'O')
		return(OK);
	else if (message[0] == 'c' || message[0] == 'C')
		return(CANCEL);
	else if (message[0] == 'q' || message[0] == 'Q')
		return(MQUERY);
	else {
		fprintf(stderr,MSGSTR(M_MSG_5,
			"Invalid reply, must be %s, %s or %s\n"),
			SOK,SCANCEL,SQUERY);
		exit(-1);
	}
}

/*
 * NAME: sendreply
 * FUNCTION:  This function will send the handle and the encoded reply to 
 *    writesrv.  
 */
void sendreply(handle,code,host)
int handle, code;
char *host;
{
	char service[MAX_INPUT];
	char serv2[MAX_INPUT + 1];
	int sock,r;
	int status;

	bzero(serv2,sizeof(serv2));
	sock = setupsock(host);                /* set up link to host */
	bzero(service,sizeof(service));
	sprintf(service,"%c%s",HWRITE,host);
        mywrite(sock,service,sizeof(service));    /* send service request */
	bzero(service,sizeof(service));
	sprintf(service,"%d %d",handle,code);
        mywrite(sock,service,sizeof(service));    /* send handle and code */
	bzero(service,sizeof(service));
        timeread(sock,service,sizeof(service));     /* get status */
	status = atoi(service);
	if (status == 2) {        /* get original message */
		mywrite(sock,service,sizeof(service));   /* sync */
		do {     /* get original message */
			bzero(service,sizeof(service));
			myread(sock,service,sizeof(service));

			/* If buffer is full, copy to another buffer to
			   ensure there is always a terminating NULL. */
			if (strlen(service) >= MAX_INPUT)
			{
				strncpy (serv2, service, MAX_INPUT);
				printf ("%s",serv2);
			}
			else
				if (service[0] != '\0') printf("%s",service);
		} while (service[0] != '\0');			
		bzero(service,sizeof(service));
	        myread(sock,service,sizeof(service));     /* get status */
		status = atoi(service);
	}
	if (status < 0) error(status,"",host,0);
	mywrite(sock,service,sizeof(service));  	/* use as sync */
	close(sock);
	return;
}

/* 
 * NAME: srvinfo
 * FUNCTION:  This function gets a list of all messages a waiting a reply,
 *    from the writesrv and then displays them.
 * RETURN:  returns -1 if error
 *          else  0
 */
void srvinfo(tohost,fromhost)
char *tohost;
char *fromhost;
{
	int sock;
	char service[MAX_INPUT];
	
	bzero(service,sizeof(service));
	sock = setupsock(tohost);      /* set up link to host */
	service[0] = QUERY;
	strcat(service,fromhost);
	mywrite (sock,service,sizeof(service));  /* send service request */
	bzero(service,sizeof(service));
	timeread(sock,service,sizeof(service));
	do {               /* get list */
		if (service[0] != '\0') printf("%s",service);
		bzero(service,sizeof(service));
		myread(sock,service,sizeof(service));
	} while (service[0] != '\0');
	mywrite(sock,service,sizeof(service));   /* use as sync */
	close(sock);
}

/*
 * NAME: gettargetinfo
 * FUNCTION:  This function will verify all target information that
 *    was recieved from the command line and then it will gather any 
 *    remaining information it needs.  
 */
void gettargetinfo(targetinfo,flags)
struct userinfo *targetinfo;
struct flag *flags;
{
	struct utmp *ubuf;
	int found = 0;
	char tty[PATH_MAX];
	struct ttys *head = NULL,*current = NULL,*test = NULL;

	targetinfo->fldes = 0;
	if (flags->nflag == 0 && flags->rflag == 0) { /* for local only */
		if (targetinfo->user[0] == '-' && targetinfo->tty[0] != '\0') {
			if (targetinfo->tty[0] != '/') {
				strcpy(tty,targetinfo->tty);
				strcpy(targetinfo->tty,"/dev/");
				strcat(targetinfo->tty,tty);
			} /* end of if */
			return;
		} /* end of if */
		if ((ubuf = getutent()) == NULL) { /* open utmp file */
			fprintf (stderr, MSGSTR(M_MSG_8, 
				"write: Can't open %s\n") ,UTMP_FILE);
			if (targetinfo->tty[0] == '\0'){
				fprintf (stderr, MSGSTR(M_MSG_9, 
					"write: can not continue\n") );
				exit(-1);
			} /* end of if */
		}  /* end of if */
		else {                            /* check entry in utmp file */
		  while (ubuf != NULL) {
                    if (
        ubuf->ut_type == USER_PROCESS &&
        strncmp(ubuf->ut_user,targetinfo->user,(size_t)MAX_USERID_LEN) == 0 
                    ) {
			if (targetinfo->tty[0] == '\0') {
			    errno = 0;
			    test = (struct ttys *) malloc (sizeof(struct ttys));
			    if (test == NULL) {
			        if (errno == 0) error(MALLOC,"","",0);
				perror (MSGSTR(M_MSG_38,"malloc"));
				exit(-1);
			    }
			    if (current == NULL) 
				current = test;
			    else {
				current->next = test;
			    	current = current->next;
			    } /* end of else */
		            current->next = NULL;
			    if (head == NULL) head = current;	
			    strncpy(current->tty,ubuf->ut_line,(size_t)MAX_TTY_LEN);
			    found++;
			} /* end of if */
			else if (strncmp(targetinfo->tty, ubuf->ut_line,
					(size_t)MAX_TTY_LEN) == 0) {
				found++;
				break;
			} /* end of else if */
		      } /* end of if */
		      ubuf = getutent();
		    } /* end of while */
		    endutent();
		    if (found == 0){
			if(targetinfo->tty[0])
				fprintf(stderr,MSGSTR(M_MSG_44, 
				"%s is not logged in on %s.\n"),targetinfo->user,targetinfo->tty);
			else
				fprintf(stderr,MSGSTR(M_MSG_10, 
				"%s is not logged on.\n"),targetinfo->user);
			exit(-1);
		    } /* end of if */
		  } /* end of else */
		  if (targetinfo->tty[0] == '\0') 
			strcpy(targetinfo->tty,head->tty);
		  if (targetinfo->tty[0] != '/' && targetinfo->tty[0] != '\0') {
			strcpy(tty,targetinfo->tty);
			strcpy(targetinfo->tty,"/dev/");
			strcat(targetinfo->tty,tty);
		  } /* end of if */
		  if (head != NULL && head->next != NULL) {
			fprintf(stdout, MSGSTR(M_MSG_11, 
				"%s is logged on more than one place.\n") ,
					targetinfo->user);
			fprintf(stdout, MSGSTR(M_MSG_12, 
					"You are connected to %s.\n") ,
					targetinfo->tty);
			fprintf(stdout, MSGSTR(M_MSG_13, 
					"Other locations are:\n") );
			head = head->next;
			found--;
			while(head != NULL) {
				fprintf(stdout,"%s\n",head->tty);
				head = head->next;
			} /* end of while */
		  } /* end of if */
	} /* end of if */
} /* end of gettargetinfo */
	
/*
 * NAME: openlink
 * FUNCTION:  This function opens the target tty or sends a service repuest to
 *   the remote writesrv to open it's target tty.  In either case after it is
 *   opened, the header informing the target user of a incoming message will 
 *   be displayed.  The file descriptor of the target will be set.
 */
void openlink (userinfo,targetinfo,flags)
struct userinfo *userinfo,*targetinfo;
struct flag *flags;
{
	time_t timet;
	char date[MB_DATE_LEN],errs[MAX_INPUT],nums[MAX_INPUT];
	int err,num,i;
	int sock;
	char service[MAX_INPUT];
	struct tm *tm_time;

	if (flags->nflag > 0 || flags->rflag > 0) {    /* remote */
		bzero(service,sizeof(service));
		sock = setupsock(targetinfo->host);
		if (flags->rflag > 0) service[0] = RWRITE;
		else service[0] = RELAY;
		strcat(service,userinfo->host);
		mywrite(sock,service,sizeof(service));/* send service request */
		targetinfo->fldes = sock;
		bzero(service,sizeof(service));
		strcpy(service,targetinfo->user);
		mywrite(sock,service,sizeof(service));  /* send user  */
		bzero(service,sizeof(service));
		if (targetinfo->tty[0] == '\0') strcpy(service,".");
		else strcpy(service,targetinfo->tty);
		mywrite(sock,service,sizeof(service));  /* send tty */
		bzero(service,sizeof(service));
		timeread(sock,service,sizeof(service));    /* get status */
		sscanf(service,"%s%s",errs,nums);
		err = atoi(errs);
		num = atoi(nums);
		if (err <= 0) 
		  error(err,targetinfo->user,targetinfo->host,flags->rflag);
		if (num > 1) {             /* get list of ttys user is using */
			bzero(service,sizeof(service));
			myread(sock,service,sizeof(service));
			fprintf(stderr, MSGSTR(M_MSG_15, 
			    "%s on %s is logged on more than one place.\n"),
				targetinfo->user,targetinfo->host);
			fprintf(stderr, MSGSTR(M_MSG_16, 
				"You are connected to %s.\n"),service);
			fprintf(stderr, MSGSTR(M_MSG_17,
				"Other locations are:\n"));
			for (i=1;i<num;i++) {
				bzero(service,sizeof(service));
				myread(sock,service,sizeof(service));
				printf("%s\n",service);
			}
		}
	}
	else {     /* open local tty */
		if (access(targetinfo->tty,F_OK) < 0) {
			fprintf(stderr,MSGSTR(M_MSG_19, 
				"write: No such tty\n"));
			exit(-1);
		}
		targetinfo->fldes = open (targetinfo->tty,O_WRONLY,0);
		if (targetinfo->fldes < 0) {
			fprintf(stderr, MSGSTR(M_MSG_21, 
				"write: Permission denied\n"));
			exit(-1);
  		}
	}
	timet = time((long *) 0);
	tm_time = localtime(&timet);
	(void) strftime(date,MB_DATE_LEN,"%c",tm_time);
	trunc_str (date, DATE_LEN);
	bzero(service,sizeof(service));
	service[0] = '\n';
	mywrite(targetinfo->fldes,service,sizeof(service)); /* send blank line */
	bzero(service,sizeof(service));
	sprintf(service,
		MSGSTR(M_MSG_1,
		"   Message from %s on %s (%s) [%s] ...\n")
		,userinfo->user,userinfo->host,userinfo->tty+5,date);
	mywrite(targetinfo->fldes,service,sizeof(service));     /* send header */
}


/*
 * NAME: trunc_str
 * FUNCTION:  This function will truncate a string to a specified
 *    number of characters.  It checks for multibyte characters and
 *    truncates to the last multibyte character in the string such that
 *    the total number of bytes is <= length.  Assumes that the buffer
 *    size of str is > length, though the string length itself may
 *    be < length.
 */ 
trunc_str ( str, length)
char *str;
int length;
{
	int count=0, i=0, wclen;
	wchar_t wc;

	if ( MB_CUR_MAX != 1 )
	{
		do {
			if ( (wclen =  mbtowc(&wc, &str[count], MB_CUR_MAX)) == -1 ) {
				fprintf(stderr,MSGSTR(M_MSG_43, "Error in multibyte character conversion.\n"));
				exit(-1);
			}
			else {
				if ( (count + wclen) <= length )
					count += wclen;
				else {
					str[count] = '\0';
					break;
				}
			}
		}
		while ( (count <= length) && (wclen != 0) );
	}
	else str[length] = '\0';
}


/*
 * NAME: exshell
 * FUNCTION:  This function will execute the rest of the message line as
 *    a shell command.  
 */ 
void exshell(message)
char *message;
{
	int pid;

	pid = fork();
	if (pid < 0) {
		perror(MSGSTR(M_MSG_22,"execute shell command"));
		return;
	}
	if (pid == 0) {    /* inside of child */
		execl(getenv("SHELL") ?
		    getenv("SHELL") : "/usr/bin/sh", "sh", "-c", ++message, 0);
		exit(0);
	}
	while (wait((int *)NULL) != pid)   /* in parent */
		;
	printf("!\n");
}

/* 
 * NAME: getreply
 * FUNCTION: This function will wait for writesrv to send an encoded reply.
 *    This encoded reply will then be returned. 
 */
int getreply(fldes)
int fldes;
{
	char buf[MAX_INPUT];
	int reply;

	bzero(buf,sizeof(buf));
	myread(fldes,buf,sizeof(buf));
	reply=atoi(buf);	
	if (reply < 0)
		error(reply,"","",0);
	mywrite(fldes,buf,sizeof(buf));  /* send acknowledgement */
	return(reply);
}

/*
 * NAME: setupsock:
 * FUCNTION: sets up the connection to proper socket.
 */
int setupsock(host)
char *host;
{
	struct servent *sp;
	struct sockaddr_in server;
	struct hostent *hp;
	int sock;
	
	sp = getservbyname("writesrv","tcp");
	if (sp == NULL) {
		fprintf(stderr,MSGSTR(M_MSG_24,"writesrv: unknown service\n"));
		exit(-1);
	}
	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0) {
		perror( MSGSTR(E_MSG_25, "opening stream socket") );
		exit(-1);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(host);  /* if internet address */
	if ( server.sin_addr.s_addr == -1) {               /* if hostname */
		hp = gethostbyname(host); 
		if (hp == 0) { 
		    fprintf(stderr,MSGSTR(M_MSG_26,"write: unknown host\n"));
		    exit(-1);
		}
		bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	}
	server.sin_port = sp->s_port;
	if (connect(sock,&server,sizeof(server)) < 0) {
		perror ( MSGSTR(E_MSG_27, "connecting stream socket") );
		exit(-1);
	}
	return(sock);
}

/*
 * NAME: error
 * FUNCTION:  Displays the proper error message based on the status 
 *     returned by a remote host.
 */
error(err,user,host,rflag)
int err;
char *user,*host;
int rflag;
{
	if ( err == NOTLOG && rflag > 0) 
		exit(2);
	else if ( err == NOTLOG) fprintf(stderr,MSGSTR(M_MSG_28, 
			"%s not logged in on %s\n") ,user,host);
	else if (err == NOTTY) 
		fprintf(stderr,MSGSTR(M_MSG_29,"No such tty\n") );
	else if (err == NOPERM ) fprintf(stderr,MSGSTR(M_MSG_30,
					"Permission denied\n"));	
	else if (err == NOOPEN) fprintf(stderr,MSGSTR(M_MSG_31,
			"write: Can't open %s on %s\n"),UTMP_FILE,host);	
	else if (err == MALLOC) fprintf(stderr, MSGSTR(M_MSG_32, 
					"malloc: FATAL ERROR\n") );
	else if (err == BADHAND) fprintf(stderr,MSGSTR(M_MSG_33,
					"Invalid handle on %s\n"),host);
	else if (err == SNDRPLY) fprintf(stderr,MSGSTR(M_MSG_34, 
						"Could not send reply\n"));
	else if (err == GETRPLY) fprintf(stderr,MSGSTR(M_MSG_36, 
						"Could not get reply\n"));
	else if (err == NOSERVICE) {
		fprintf(stderr,MSGSTR(M_MSG_41,
			"writesrv can not provide this service right now.\n")); 
		fprintf(stderr,MSGSTR(M_MSG_42,
			"writesrv is shutting down\n"));
	}
	exit(-1);

}

/* 
 * NAME: myread
 * FUNCTION:  perform the read and checks the return code.
 */
void myread (fldes,buf,len)
int fldes;
char *buf;
int len;
{
	int nchar;
	
	nchar = read(fldes,buf,(unsigned)len);
	if ((nchar == -1) && (errno != EINTR)) {
		fprintf(stderr,MSGSTR(M_MSG_37,
		"Can not communicate with the daemon writesrv\n"));
		perror("read() in myread()");
		exit(-1);
	}
}

/* 
 * NAME: mywrite
 * FUNCTION:  perform the write and checks the return code.
 */
void mywrite (fldes,buf,len)
int fldes;
char *buf;
int len;
{
	int nchar;
	
	nchar = write(fldes,buf,(unsigned)len);
	if (nchar != len) {
		fprintf(stderr,MSGSTR(M_MSG_37,
		        "Can not communicate with the daemon writesrv\n"));
		perror("write() in mywrite()");
		exit(-1);
	}
}
/*
 * NAME: sigpipe
 * FUNCTION: catches SIGPIPE, issues error message and exits.
 */
sigpipe(void)
{
	fprintf(stderr,MSGSTR(M_MSG_37,
		"Can not communicate with the daemon writesrv\n"));
	exit(-1);
}

/*
 * NAME: timeout
 * FUNCTION: catches the alarm signal prints error message and exits
 */
int timeout(void)
{
	fprintf(stderr,MSGSTR(M_MSG_40,"%s must not be running\n"),"writsrv");
	exit(-1);
}
/*
 * NAME: timeread
 * FUNCTION: only wait 1 miniute for a response from writesrv
 *  this function is only used for the first read from writesrv for a
 * connection.
 */
void timeread(fldes,buf,len)
int fldes;
char *buf;
int len;
{
	new_signal (SIGALRM,(void (*)(int))timeout);
	alarm ((unsigned)60);
	myread(fldes,buf,len);
	alarm ((unsigned)0);
}

expand_control ( str, length, str2 )

char str[];
int length;
char str2[];
{
int count,wclen;
int i = 0;
int j = 0;
char c;
wchar_t wc;
char octstr[32];	/* will hold octal version of non-printable chars */
bzero ( octstr, 32 );
if ( MB_CUR_MAX == 1 )
{
	while (  ( i < length ) && ( j < BUF_LEN )  )
	{
        	c = str[i];
        	if (	( isprint ( c ) )  ||  ( c == WRT_BELL )  ||
               		( c == WRT_NEWLINE ) ||
			( c == '\t') ) 		/* p35972 */
                	str2[j++] = c;
        	else
                {
			/* Print octal sequence for non-printable chars */
                	if ( (c != '\0') && (BUF_LEN-j > 5) )
                	{
				sprintf (octstr,"\\o%3o",c);
				strcat(str2,octstr);
				j+=5;		/* for \o plus 3 octal chars */
                	}
                }
        	++i;
	} /* while */
}
else  {		/* multibyte characters */
	while (  ( i < length ) && ( j < BUF_LEN )  )
	{
		c = str[i];
		if ( (wclen =  mbtowc(&wc, &str[i], MB_CUR_MAX)) == -1 ) {
			/* length is a maximum of MAX_INPUT-1 from the fgets */
			if ( (length == (MAX_INPUT-1)) && ((MAX_INPUT-i-1) < MB_CUR_MAX) )	/* split a multibyte char */
				return (i);
			else {
				fprintf(stderr,MSGSTR(M_MSG_43, "Error in multibyte character conversion.\n"));
				exit(-1);
			}
		}
		else {
        		if (	( iswprint ( wc ) )  ||  ( wc == WRT_BELL )  ||
               			( wc == WRT_NEWLINE ) ||
				( wc == '\t') ) {			/* p35972 */
				if ( (j + wclen) < BUF_LEN ) {
					for (count = 0; count < wclen; count++)
					{
                				str2[j++] = c;
						c = str[++i];
					}
				}
				else break;
			}
        		else
                	{
                		if ( wc != '\0' )
                		{
					/* Print octal sequence for non-printable chars */
					for (count = 0; count < wclen; count++)
						if ( BUF_LEN-j >=5 ) {
							sprintf (octstr,"\\o%3o",c);
							strcat(str2,octstr);
							j+=5;		/* for \o plus 3 octal chars */
							c = str[++i];
                				}
				}
                	}
		}
	} /* while */
} /* else */

if ( length == (MAX_INPUT - 1) )
   str2[j] = WRT_NEWLINE;

return (0);
}
