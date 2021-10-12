static char sccsid[] = "@(#)42	1.6  src/bos/usr/lib/pios/piomsg.c, cmdpios, bos411, 9428A410j 11/9/93 11:05:02";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main, msgout, getmsg, parse_msg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/*** piomsg.c - send a message to user(s) using the sysnot() function ***/
/*		Syntax:		piomsg [ -u user_list ] \		*/
/*				       [ -c message_catalog \		*/
/*					 [ -s message_setno ] \		*/
/*					 -n message_no ] \		*/
/*				       [ message_text ] \		*/
/*				       [ -a message_arg ... ]		*/
/*	"piomsg" program extracts a specified message (if -c, -s and -n	*/
/*	are specified).  While extracting, first the given catalog is   */
/*	searched.  If there is a failure in extracting the message from */
/*	this catalog, an attempt is made to extract the same from the	*/
/*	catalog in the default directory.  If it still fails, the	*/
/*	message text is used provided that it is supplied.  If not, an	*/
/*	error message is used.  However, if these options are not	*/
/*	specified, the message text (if specified) is used.		*/
/*	Also, the message that was extracted or taken from the command	*/
/*	line is parsed for "%s" or "%n$s" sequences to fill in supplied	*/
/*	(if any) message arguments (maximum of 10).			*/
/*	After extracting the message, it is sent to the users specified */
/*	(if any, with -u flag) or to the job submitter (if -u is not	*/
/*	specified).							*/
/************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/lockf.h>
#include <locale.h>
#include <piobe_msg.h>
#include <time.h>
#include <IN/backend.h>
#include <string.h>
#include <memory.h>
#include <nl_types.h>
#include "pioformat.h"


extern int	lockf(int fd, int request, off_t size);
						/* should be in sys/lockf.h */
extern int	sysnot(char *user, char *host, char *msg, unsigned int pref);
						/* available in libqb.a */

static char 	*getmsg(const char *CatName, int set, int num, int *msgextflag);
static int	msgout(const char *msgstr, const char *user, const char *node,
		       char *userlistp);
static char	*parse_msg(const char *rawmsg);

extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;

#define OPTLIST		":u:c:s:n:a:"	/* user, catalog, set, no, msgarg */
#define DOMALLOC(memptr, size) \
   {if ((memptr = malloc(size)) == NULL) { \
	(void) fprintf(stderr,(getmsg(MF_PIOBE,1,MSG_MALLOC, &meflag))); \
	exit(1); \
    }}
#define MAXMSGARGS	(sizeof (msgargv) / sizeof (msgargv[0]))
#define EXPMSGBUFSIZ	(5 * 1024 - 1)			/* 5K - 1 */

static char sub_user[500];
static char *sub_node;
static char *qname;
static char *qdname;
static const char *msgtext;
static char *basedir;
static int mailonly;
static struct irqelem *irqp1, *irqp2;
static char nullstring[] = "";
static char defbasedir[] = DEFVARDIR;
static char sepline[] = "\
-----------------------------------------------------------------------------\
--";

/* info for chain of users to received message */
struct irqelem {                /* element in chain of users to receive msgs */
    struct irqelem *next;
    char *user;
    char *node;
};
static struct irqelem irquser_beg;     /* dummy element at beginning of chain */
static struct irqelem *irquser_end = &irquser_beg;
					/* ptr to last element in chain */
static struct irqelem *irquserwp;      /* work pointer */

static union {           /* used to convert back and forth between  */
    char *charptr;       /* pointer types without upsetting lint    */
    struct irqelem *irqptr;
    struct element *elemptr;
} mp;


/*******************************************************************************
*                                                                              *
* NAME:           piomsg                                                       *
*                                                                              *
* DESCRIPTION:    Send the specified message to the specified (or default)     *
*                 users using the sysnot() function                            *
*                                                                              *
* PARAMETERS:     argc - argument count;                                       *
*                 argv - argument vector;                                      *
*                                                                              *
*                                                                              *
* GLOBALS:                                                                     *
*     MODIFIED:                                                                *
*                                                                              *
*                                                                              *
* RETURN VALUES:  0 - successful                                               *
*                 1 - error detected                                           *
*                                                                              *
*                                                                              *
*******************************************************************************/
int
main(int argc, char **argv)
{
int ch, done, duplicate, fd, cnt;
long clock_tm;
char wk_user[500];
char *cp1, *cp2, *cp3, *p1, *p2;
char userlist[5000];
char fname[200];
char pdate[50];
struct tm *tdate;
FILE *fp;
int c_flag = FALSE;			/* c flag */
int s_flag = FALSE;			/* s flag */
int n_flag = FALSE;			/* n flag */
const char *msg_catalog_name;		/* message catalog name */
int msg_set_no = 1;			/* message set no */
int msg_no;				/* message no */
const char *supmsgtext;			/* supplied message text */
int meflag = TRUE;			/* message extracted successfully? */
char const *msgargv[]			/* message argument array */
	   = { nullstring,		/* (maximum of 10 arguments + */
	       nullstring,		/*  1 for standard argv convention) */
	       nullstring,
	       nullstring,
	       nullstring,
	       nullstring,
	       nullstring,
	       nullstring,
	       nullstring,
	       nullstring,
	       nullstring
	     };
char const **msgargvp = msgargv;	/* temp pointer to message arguments */
char expmsgbuf[EXPMSGBUFSIZ + 1];	/* buffer (5K) for expanded message */


setlocale(LC_ALL, "");

*userlist = '\0';

/* Determine If We're Running Under the Spooler */
if (getenv("PIOSTATUSFILE") != NULL) {
    (void) strncpy(sub_user, getenv("PIOFROM"), sizeof(sub_user) -1);
    qname    = getenv("PIOQNAME");
    qdname   = getenv("PIOQDNAME");
    mailonly = atoi(getenv("PIOMAILONLY"));
} else {
    (void) fprintf
	(stderr, getmsg(MF_PIOBE, 1, MSG_PIOMSGCMD, &meflag));
    exit(1);
}

/* Determine Base Directory */
if ((basedir = getenv("PIOVARDIR")) == NULL)
    basedir = defbasedir;                    /* default base directory */

/* Process Flags */
opterr = 0;
while ((ch = getopt(argc, argv, OPTLIST)) != EOF) {
    switch (ch) {
    case 'u':  /* User(s) to send message to */
	       /* User IDs separated by ','; null string represents submitter*/
	done = FALSE;
	p1 = p2 = optarg;
	do {
	    if (*p2 == ',' || *p2 == '\0') {
		if (p2 == p1)
		    cp1 = strcpy(wk_user, sub_user);
		else
		    cp1 = p1;
		if (*p2 == '\0')
		    done = TRUE;
		else
		    *p2 = '\0';
		p1 = p2 + 1;
		DOMALLOC(mp.charptr, sizeof(struct irqelem));
		irquserwp = mp.irqptr; /* convert char * to struct irqelem * */
		irquserwp->next = NULL;
		irquserwp->user = cp1;
		irquserwp->node = nullstring;
		for (cp2 = cp1; *cp2; cp2++)
		    if (*cp2 == '@') {
			*cp2 = '\0';
			irquserwp->node = ++cp2;
			break;
		    }
		irquser_end->next = irquserwp; /* add element to end of chain*/
		irquser_end = irquserwp;
	    }
	    p2++;
	} while (!done);
	break;

    case 'c':		/* message catalog name */
	c_flag++;
	msg_catalog_name = optarg;
	break;

    case 's':		/* message set no */
	s_flag++;
	msg_set_no = atoi(optarg);
	break;

    case 'n':		/* message no */
	n_flag++;
	msg_no = atoi(optarg);
	break;

    case 'a':		/* message argument(s) */
	/* Save the message arguments in an array upto a maximum of 10. */
	if (msgargvp - msgargv < MAXMSGARGS - 1)
	    *msgargvp++ = optarg;
	break;

    case ':':		/* missing option argument */
	(void) fprintf(stderr, getmsg(MF_PIOBE, 1, MSG_PIOMSG_MARG, &meflag),
		       (char) optopt);
	exit(1);

    case '?':		/* illegal option */
	(void) fprintf(stderr, getmsg(MF_PIOBE, 1, MSG_PIOMSG_ILOPT, &meflag),
		       (char) optopt);
	exit(1);
    } /*switch*/
} /*for*/

/* If any of the flags '-c', '-s' or '-n' are specified, make sure that all
   of them are specified (Exception: set no is optional). */
if ((c_flag || s_flag || n_flag)  &&  !(c_flag && n_flag))
{
    (void) fprintf(stderr, getmsg(MF_PIOBE, 1, MSG_PIOMSG_IPUSG, &meflag));
    exit(1);
}

/* Get the Supplied Message Text */
supmsgtext = optind < argc ?		/* if MsgText supplied */
	     argv[optind]
	     :				/* if MsgText not supplied */
	     NULL;

/* If message catalog options were given, fetch the corresponding message. */
/* Else, use the supplied message text. */
if (c_flag || s_flag || n_flag)
{
    /* Fetch the messag from the specified catalog.  If unsuccessful,
       use the supplied message (if any). */
    if ((!(msgtext = getmsg(msg_catalog_name, msg_set_no, msg_no, &meflag)) ||
	!meflag)  &&  supmsgtext)
	msgtext = supmsgtext;
}
else
    msgtext = supmsgtext;
if (!msgtext)
    msgtext = nullstring;

/* Expand the extracted/supplied message text parsing and filling it with
   the supplied message argument(s), if any.  Use all the ten message
   arguments (even if supplied or not).  If error, send the error message. */
msgtext = sprintf(expmsgbuf, msgtext, *msgargv, *(msgargv + 1), *(msgargv + 2),
	    *(msgargv + 3), *(msgargv + 4), *(msgargv + 5), *(msgargv + 6),
	    *(msgargv + 7), *(msgargv + 8), *(msgargv + 9)) < 0 ?  /* failure */
          /* strerror(errno) */
          msgtext
          :							   /* success */
          expmsgbuf;

/* Parse the message for any special characters like new line, form feed, etc.
   Note: Octal and Hexadecimal characters can be translated by "piodigest"
   at the time of digesting.  Hence they need not be processed here. */
msgtext = parse_msg(msgtext);

/* Send the Message to the Target User(s) */
(void) freopen("/dev/null", "a", stderr); /* don't want stderr from sysnot() */
if (irquser_beg.next == NULL) {
    /* -u Flag Was NOT Specified, So Assume Print Job Submitter */
    sub_node = nullstring;
    for (cp1 = sub_user; *cp1 != '\0'; cp1++)
	if (*cp1 == '@') {
	    *cp1 = '\0';
	    sub_node = ++cp1;
	    break;
	}
    (void) msgout(msgtext, (const char *) sub_user, (const char *) sub_node,
		  userlist);
} else {
    /* -u Flag Was Specified, So Process List of users/nodes */
    for (irqp1 = irquser_beg.next; irqp1; irqp1 = irqp1->next) {
	/* Check For Duplicate user/node Combinations */
	duplicate = FALSE;
	for (irqp2 = irqp1->next; irqp2; irqp2 = irqp2->next)
	    if (!strcmp(irqp1->user,irqp2->user)
		  && !strcmp(irqp1->node,irqp2->node)) {
		duplicate = TRUE;
		break;
	    }
	if (!duplicate)
	    (void) msgout(msgtext, (const char *) irqp1->user,
			  (const char *) irqp1->node, userlist);
    }
}

/* keep a copy of the message in /var/spool/lpd/pio/@local for autopsies */
clock_tm = time ((long *) 0);  /* get the time */
tdate = localtime(&clock_tm);
(void) strftime(pdate,350,"%a %h %d %T %Y",tdate);
(void) sprintf(fname, "%s/msg1.%s:%s", basedir, qname, qdname);
(void) lockf(3, F_LOCK, 0);
(void) unlink(fname);
(void) umask (0777 ^ (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
fd = open(fname, O_CREAT + O_WRONLY, 0664);
if (fd > 0) {
    fp = fdopen(fd, "w");
    cp3 = getmsg(MF_PIOBE, 1, MSG_MAILONLY, &meflag);
    if (cp1 = malloc(1001))
	strncpy(cp1, cp3, 1000);
    cp3 = getmsg(MF_PIOBE, 1, MSG_MSG_TO, &meflag);
    if (cp2 = malloc(1001))
	strncpy(cp2, cp3, 1000);
    if (!mailonly)
        cp1 = nullstring;
    (void) fprintf(fp,"%s\n",sepline); 
    cnt = fprintf(fp,"%s  %s  ",pdate,cp1); 
    if ((cnt + strlen(cp2) + strlen(userlist)) > 78)
        (void) fprintf(fp,"\n");
    (void) fprintf(fp,"%s %s\n%s\n\n%s",cp2,userlist,sepline,msgtext); 
    (void) fclose(fp);
}
(void) lockf(3, F_ULOCK, 0);

return(0);
}


/*******************************************************************************
*                                                                              *
* NAME:           msgout                                                       *
*                                                                              *
* DESCRIPTION:    Send the specified message to the specified user at the      *
*                 (optional) specified node                                    *
*                                                                              *
* PARAMETERS:     msgstr - message text                                        *
*                 user   - target user                                         *
*                 node   - target user's node (if any)                         *
*                 target - 1 submitter                                         *
*                 userlistp - pointer to list of user/node names               *
* GLOBALS:                                                                     *
*     REFERENCED:                                                              *
*                                                                              *
*                                                                              *
* RETURN VALUES:  void                                                         *
*                                                                              *
*******************************************************************************/

static int
msgout(const char *msgstr, const char *user, const char *node, char *userlistp)
{
char wkbuf[1000];

/* Send the Message */
(void) lockf(3, F_LOCK, 0);
(void) sysnot((char *)user, (char *)node, (char *)msgstr,
	      mailonly?DOMAIL:DOWRITE);
(void) lockf(3, F_ULOCK, 0);

/* Update the string list of users/nodes */
*wkbuf = '\0';
if (*userlistp)
    (void) strcat(wkbuf, ", ");
(void) strcat(wkbuf, user);
if (*node) {
    (void) strcat(wkbuf, "@");
    (void) strcat(wkbuf, node);
}
(void) strcat(userlistp, wkbuf);

return(0);
}

/*
*******************************************************************************
*******************************************************************************
** NAME:        getmsg()
**
** DESCRIPTION: Replaces the NLgetamsg routine used in 3.1 code  If the catalog
**              is not found in the NLSPATH, it will look for a default in
**              /usr/lib/lpd/pio/etc.
**
** ROUTINES
**   CALLED:    catopen() - gets catalog descriptor
**
**              catgets() - gets message
**              catclose  - closes catalog
**
** PARAMETERS:  catalog name, set number, message number, flag to denote whether
**		the specified message has been successfully extracted or not
**
**
*******************************************************************************
*******************************************************************************
*/

#define DUMMYSTR	"dummy"
#define MAXMSGBUF	4000
#define DEFERRMSG	"Cannot access message catalog %s.\n"

static char *msgbuffer = NULL;
static nl_catd catd;
static nl_catd def_catd;
static char save_name[100];

static char *
getmsg(const char *CatName, int set, int num, int *msgextflag)
{
	char *ptr;
	char defpath[200];
	char default_msg[100];
	register size_t msglen;

	if (strcmp(CatName,save_name) != 0)  /* is it a different catalog */
	{
	catclose(catd);  /* close /usr/lpp message catalog */
	catclose(def_catd);  /* close default message catalog */
	catd = def_catd = (nl_catd) 0;  /* set catalog descriptors to NULL */
	strcpy(save_name,CatName);
	}

	if (catd != (nl_catd) -1)  
		if (catd == 0) /* if it hasn't been open before */
			catd = catopen((char *) CatName,NL_CAT_LOCALE);
	
	if (catd != (nl_catd) -1)
		{
		ptr = catgets(catd,set,num,DUMMYSTR);
		if (!msgbuffer)
			msgbuffer = malloc(MAXMSGBUF + 1);
		if (msgbuffer)
		{
		    (void) strncpy(msgbuffer, ptr,
				   msglen = strlen (ptr) < MAXMSGBUF ?
				   strlen (ptr) : MAXMSGBUF);
		    msgbuffer[msglen] = 0;
		}
		if (strcmp(ptr,DUMMYSTR) != 0)  /* did catgets fail? */
			return(msgbuffer);
		}
	
	if (def_catd == 0)
		{
		sprintf(defpath,"/usr/lib/lpd/pio/etc/%s",CatName);
		def_catd = catopen(defpath,NL_CAT_LOCALE);
		}
	
	(void) sprintf(default_msg, DEFERRMSG, CatName);
	ptr = catgets(def_catd,set,num, DUMMYSTR);
	*msgextflag = !strcmp(ptr, DUMMYSTR) ? 	/* if catgets failed */
		      (ptr = default_msg), FALSE
		      :				/* catgets did not fail */
		      TRUE;
	if (!msgbuffer)
		msgbuffer = malloc(MAXMSGBUF + 1);
	if (msgbuffer)
	{
	    (void) strncpy(msgbuffer, ptr,
			   msglen = strlen (ptr) < MAXMSGBUF ?
			   strlen (ptr) : MAXMSGBUF);
	    msgbuffer[msglen] = 0;
	}

	return(msgbuffer);
}

#undef	DEFERRMSG
#undef	MAXMSGBUF
#undef	DUMMYSTR


/*******************************************************************************
*                                                                              *
* NAME:           parse_msg                                                    *
*                                                                              *
* DESCRIPTION:    Parse the given raw message and process any escape           *
*                 characters like line feed, form feed, etc.                   *
*                 Note that characters with octal and hexadecimal notation     *
*                 are not processed here, since these can be processed by      *
*                 "piodigest" at the time of digesting.                        *
*                                                                              *
* PARAMETERS:     rawmsg - intput raw message text                             *
*                                                                              *
* GLOBALS:                                                                     *
*     REFERENCED:                                                              *
*                                                                              *
*                                                                              *
* RETURN VALUES:  postmsg (processed message text)                             *
*                                                                              *
*******************************************************************************/

static char *
parse_msg(register const char *rawmsg)
{
   static char		postmsg[EXPMSGBUFSIZ + 1];
   register char	*postmsgp 	= postmsg;
   register int		bkslshflag 	= FALSE;

   for ( ;*rawmsg && postmsgp < postmsg + sizeof (postmsg); rawmsg++)
   {
      if (*rawmsg == '\\')		/* if a back slash char */
      {
	 /* If previous character is not a back slash, skip this. */
	 if (bkslshflag = !bkslshflag)
	    continue;			/* continue to process next character */
      }
      else				/* if not a back slash char */
      {
	 /* If previous character is a back slash, process this as a
	    special character.  Else, fall through and copy this
	    character. */
	 if (bkslshflag)
	 {
	    bkslshflag = FALSE;
	    switch (*rawmsg)
	    {
	       case 'a':		/* alert (bell) character */
		  *postmsgp++ = '\a';
		  break;

	       case 'b':		/* backspace */
		  *postmsgp++ = '\b';
		  break;

	       case 'f':		/* formfeed */
		  *postmsgp++ = '\f';
		  break;

	       case 'n':		/* newline */
		  *postmsgp++ = '\n';
		  break;

	       case 'r':		/* carriage return */
		  *postmsgp++ = '\r';
		  break;

	       case 't':		/* horizontal tab */
		  *postmsgp++ = '\t';
		  break;

	       case 'v':		/* vertical tab */
		  *postmsgp++ = '\v';
		  break;

	       case '\?':		/* question mark */
		  *postmsgp++ = '\?';
		  break;

	       case '\'':		/* single quote */
		  *postmsgp++ = '\'';
		  break;

	       case '\"':		/* double quote */
		  *postmsgp++ = '\"';
		  break;

	       default:			/* non-escape character */
		  /* This is a (supposedly) non-escape character.
		     However, there is a way even an escape character
		     like octal or hexadecimal character (with "\ooo" or
		     "\xhh" notation) might creep in here after evading
		     the all-too-mighty sweep of "piodigest".
		     Hence, to handle this kind of situations, we'll
		     copy the original character sequence including
		     the backslash, which is otherwise not necessary. */
		  *postmsgp++ = '\\';	/* normally unnecessary */
		  *postmsgp++ = *rawmsg;
		  break;
	    }		/* end - switch *rawmsg */
	    continue;			/* continue to process next character */
	 }		/* end - if bkslshflag */
      }			/* end - if *rawmsg != '\\' */
      *postmsgp++ = *rawmsg;
   }			/* end - for *rawmsg */
   *postmsgp = 0;

   return postmsg;
}			/* end of function 	-	parse_msg */
