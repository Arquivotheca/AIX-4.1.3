static char sccsid[] = "@(#)77 1.24  src/bos/usr/bin/confer/confer.c, cmdcomm, bos41B, 9504A 12/23/94 12:51:23";
/*
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: confer, joinconf
 *
 * ORIGINS: 27,10
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
/*
 * Confer/joinconf - conferencing program
 *
 * confer [[-]~] [-v] [-nname] {user | @ttyname | user@ttyname } ...
 * joinconf conference
 *
 * ~ means "off the record";  -v  means verbose (char at a time)
 * The shell may require that ~ be escaped.
 */

/* This module contains the source code for the confer and joinconf commands.
 * Users who are all logged into a single machine may participate in a 
 * conference in which the issuer of confer is the leader and the others,
 * issuers of joinconf, are the invitees.  In order to receive the
 * invitation, either the leader must be root, or the invitees must have 
 * writing enabled by others to their ttys.
 *
 * Each participant records his own contributions to the conference in a 
 * transcript file which is created in /tmp/confer (usually).  When the 
 * conference is completed, signaled by each participant terminating his
 * participation with a ^D, a child process forked by the leader reads
 * and merges the transcript files into a common transcript and mails it
 * to each participant requesting it.
 *
 * For each conference, there is a master file to which is linked sync files,
 * one for each participant.  The leader's child detects when the conference
 * is completed by watching for the last link on this master file.
 *
 * Only a very simple version of raw mode (allowing one character at a time
 * conferencing) is implemented.  A tab is converted to a single blank.
 * Backspace is handled correctly, but kill is not implemented.
 */

/* Define TEST if joining oneself is to be allowed */

#include <stdio.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <locale.h>
#include <langinfo.h>
#include <limits.h>
#include <IN/standard.h>
#include <signal.h>
#include <unistd.h>
#include <macros.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <utmp.h>
#include <termio.h>
#include <string.h>

#include "confer_msg.h" /* symbolic identifiers */
static nl_catd  scmc_catd;   /* message catalog file descriptor */
#define MSGSTR(Num,Str) catgets(scmc_catd,MS_confer,Num,Str)

#define TSSIZE 128	/* size of time string buffer in bytes */

static char USAGE1[]	= "usage: %s [[-]~] [-v] [-n<name>] {user | @ttyname | user@ttyname} ...\n";
static char TAKEFLOORMSG[] = "Type RETURN to take the floor and a blank line to relinquish it.\n\n";
static char TMPDIR[]   =       "/tmp/confer/";
static char mailcom[]  =       "rmail";        /* OK absolute. (?)  */

#define INTERRUPT       (EOF-1)         /* Returned by getch */

struct header   /* Header of record in transcript file */
{       time_t timestamp;
	int  nbytes;
	int  shflag; /* used for d54822 */
};
#define RDHEADER        1
#define RDDATA          2

/* max # bytes in user name excluding NULL.  We are limited to the length
 * of the user field in utmp.
 */
static struct utmp utmp;
#define USRNAMMAX	sizeof(utmp.ut_user)
#define TTYNAMMAX	sizeof(utmp.ut_line)

static struct person   /* user structure */
{       struct person *p_next;

	/* For now, the status only indicates whether the tty has been opened
	 * and successfully written to.
	 */
	int	p_status;

	char    p_name[USRNAMMAX+1]; /* usernameNULL (obtained from logname() */
	char    p_id[USRNAMMAX+TTYNAMMAX+5];      /* [username@ttyNumber]NULL */

	union
	{       int     p_fd;		/* file descriptor for /dev/ttyNumber */
		FILE    *p_fp;		/* file descriptor for transcript */
	} p_file;

	/* The name of the tty as specified by the conference leader.  After
	 * the person list has been initialized and the tty's opened, we will
	 * change the usage of this entry to mean the name of the transcript
	 * file (minus the directory).
	 */
	char    p_ttyspec[TTYNAMMAX+1];

	/* The name of the tty which is opened and written to and which may be 
	 * different than the specified tty.  In particular, one user may
	 * participate from several tty's.
	 */
	char    p_ttyname[sizeof "/dev/" + TTYNAMMAX+1];

	struct  header  p_header;
} me;

/* p_status defines */
#define P_OPEN    1	/* tty opened but not yet successfully written to */
#define P_WRITTEN 2	/* tty successfully written to */

static FILE *transcript;	/* pointer to transcript file */
static char joincom;		/* 0 ==> confer command.    1 ==> joinconf command */
static char confercom;		/* 0 ==> joinconf command.  1 ==> confer command */
static char recording = TRUE;
static char rec2 = TRUE;
static char intflag;		/* # of interrupts since last subshell */
static char raw = FALSE;	/* TRUE ==> one character read and written at a time */

/* ptr to buffer loaded from stdin by readline().  It is malloc'd when it
 * is first used and re-alloc'd when it needs to grow.  The pointer is set 
 * to NULL initially to indicate that no buffer has yet been malloc'd.
 */
static char *lineptr = NULL;

/* ptr to buffer loaded from the transcript file by readcnf().  It is malloc'd
 * when it is first used and re-alloc'd when it needs to grow.  The pointer
 * is set to NULL initially to indicate that no buffer has yet been malloc'd.
 */
static char *tscriptptr = NULL;

static size_t mb_cur_max;
static time_t now;
static char nullstr[] = "";
static char blank[] = " ";
static char atchar[] = "@";
static char mlist[]  = ".mls";	/* transcript mailing list suffix */
static char suffix[] = ".cnf";

/* The name of the transcript file (aka conference file).  Each participant 
 * maintains his own transcript file.  The name is derived from the 
 * conference name which has .cnf appended, in the case of the confer issuer,
 * or the conference name with the user@ttyNumber (with no suffix), in the
 * case of the joinconf issuer.
 */
static char conffile[PATH_MAX];

/* Same as conffile except that "@" is replaced with "&".  The purpose of the
 * syncfile is to permit the count of links on the master to be properly 
 * maintained so maketran can know when to send the mail to the participants.
 */
static char syncfile[PATH_MAX];

static char master[PATH_MAX];

/* The name of the conference.  By default, it is the login name of the 
 * user that issued confer command.  If -nconfname flag is appended, up
 * to USRNAMMAX characters of that name is used instead.  It is the simple
 * file name (without the suffix) of the transcript file.
 */
static char confname[USRNAMMAX+4]; /* -n may be added.  File has ".cnf". */

#define LINELEN 80

/* generic buffer used for storing lines of input, file names, etc. */
static char linebuf[LINELEN];

/* buffer used to store the name of the mail list file */
static char mlistfile[LINELEN];

static struct stat Statbuf;
static struct termio ttmodes, ottmodes;
static time_t readcnf();
static FILE *creatcnf();
static char *_mb_backup();

#define IDENTIFY 2
#define NOIDENTIFY (-1)
static int  idstate = NOIDENTIFY;
		/* < 0 => don't identify, otherwise consecutive NL count */
		/* == IDENTIFY => identify on next char and reset to 0   */

/* anystr(s1, s2) (from libPW) gets index of first position
 * (starting from zero) of any character of s2 in s1.
 */

static struct person *insert();
#define joinname MSGSTR(M_MSG_26,"joinconf")
extern char *logname();
extern char *substr();
extern char *CSsname();
static void interrupt(int);

main( argc, argv )
int argc;
char **argv;
{
	register int i;
	register char *cp;
	register struct person *p;
	struct person *q;
	int j;
	int pid;
	char tty[sizeof "/dev/" + TTYNAMMAX+1];

	(void ) setlocale(LC_ALL,"");
	scmc_catd = catopen(MF_CONFER, NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;
	if (argc == 1) {
		if (**argv == 'c')
			printf(MSGSTR(M_MSG_0, USAGE1), *argv);
		else
			printf(MSGSTR(M_MSG_1,"usage: %s conference\n"),*argv);
		exit (1);
	}

	/* get full path of tty for this process */
	if (!isatty(0) || !isatty(1))
	{
		fprintf(stderr, MSGSTR(M_MSG_47,
			"Illegal redirection of stdin or stdout.\n"));
		exit(1);
	}
	strcpy( tty, ttyname(1) );	/* get tty associated with stdout */

	/* By definition, p_ttyname is the tty to be written to.  For me, this
	 * is the value returned by ttyname().  We will agree that, for me,
	 * the specified tty is the same as the tty to be written to.
	 */
	strcpy( me.p_ttyspec, tty + 5);
	strcpy( me.p_ttyname, me.p_ttyspec );

	strcpy( me.p_name, logname() );
	makeid( &me );
	me.p_status = P_WRITTEN; /* tty has been successfully written to */
	time( &now );

	/* Make sure the tty is writable by everyone.  This is necessary since
	 * root may write to any tty, even those for whom mesg -n were issued.
	 * Therefore, root may have issued an invitation, and successfully
	 * written to this tty, but other participants may still not be able
	 * to write to this tty.
	 */
	if (system("mesg -y"))
	{
		fprintf(stderr, MSGSTR(M_MSG_46, "cannot run mesg -y\n")); 
		exit(1);
	}
		
	/* Get the command name.  If this invocation is by confer, then use
	 * the login name of the user as the default conference name.  Set
	 * the flags indicating how this executable was invoked.  Set up
	 * argument list for subsequent processing in the case of joinconf.
	 */
	cp = CSsname( argv[0] ); /* get command name */
	if (strcmp( cp, "joinconf") == 0)
	{       joincom++;
		argc = getconf( &argv );
	}
	else
	{       confercom++;	/* indicate confer command invocation */
		strcpy( confname, me.p_name );
	}

	/* Process the arguments (flags and invited users).  For a joinconf
	 * invocation, the invited users were obtained in getconf() above.
	 * For each invited user, allocate and load a person structure and
	 * link it to the list of conference participants.  Each process
	 * maintains his own list of participants.
	 */
	for(i = 1; i < argc; i++)
	{       cp = argv[i];
		if (strcmp( cp, "~" ) == 0)
		{       recording = FALSE;
			continue;
		}
		if (*cp == '-')
		{  switch(*(cp+1))
		   { case 'n':  /* Conference name */
			if (confercom)
				/* get conf name from -nName argument */
				if (cp[2] != '\0')
					substr( cp, confname, 2, 8 );
				else
				{
					printf(MSGSTR(M_MSG_48,
						"Illegal space after -n\n"));
					exit(1);
				}
			continue;
		     case '~':
			recording = FALSE;
			continue;
		     case 'v':
			raw = TRUE;
			ioctl(0, TCGETA, &ttmodes);  /* get old line mode */
			ottmodes = ttmodes;	     /* copy it */
			ottmodes.c_lflag &= ~ICANON; /* kill line processing */
			ottmodes.c_lflag &= ~ECHO;   /* kill echo processing */
			ottmodes.c_cc[VMIN] = 1;     /* 1 character at a time */
			ottmodes.c_cc[VTIME] = 0;    /* no time limit */
			ioctl(0, TCSETA, &ottmodes); /* set new line mode */
			continue;
		     default:
			printf(MSGSTR(M_MSG_4,
					"Invalid flag (%s) ignored\n"),cp);
			continue;
		    }
		}

		/* At this point, we have a user or user@ttyNumber */

		p = (struct person *) malloc((size_t)sizeof(struct person));

		/* If the tty was specified by the conference leader, save it */
		if ((j = anystr( cp, atchar )) != -1)
		{       cp[j] = '\0';
			strcpy( p->p_ttyspec, cp + j + 1 );
		}
		else *p->p_ttyspec = 0;

		/* At this point, we don't know which tty will be used for
		 * writing.
		 */
		*p->p_ttyname = 0;

		if (strlen( cp ) > USRNAMMAX)
		{       
			printf(MSGSTR(M_MSG_5,
				"User name (%s) too long. Ignored.\n"),cp);
			free( (void *)p );
			continue;
		}
		strcpy( p->p_name, cp );
		p->p_next = me.p_next;
		me.p_next = p;
	}

	/* At this point, the only person on the person list having his
	 * p_ttyname set is me.
	 */

	if (joincom)
	{       /* Make sure I'm on list */
		for (p = &me; p = p->p_next ; )
		{
			/* If I match the tty, then I was invited.  Confer
			 * won't write an invitation to my screen if the user
			 * was specified and it doesn't match me.
			 */
			if (strcmp( me.p_ttyname, p->p_ttyspec ) == 0)
				break;
			/* If the tty was not specified and I match the user,
			 * then I was invited, no matter what tty I respond
			 * from.
			 */
			if (*p->p_ttyspec == 0 &&
				strcmp( me.p_name, p->p_name ) == 0 )
				break;
		}
		if ( p == 0 )
		{       fprintf(stderr,
			  MSGSTR(M_MSG_6,"Sorry, but you weren't invited.\n"));
			done( EXITBAD );
		}
	}

	/* Find out where everyone is, and drop out those that are nowhere.
	 * Set the p_ttyname of every person's structure except for me.
	 * The p_ttyspec is also set.
	 */
	gettys(&me);

	/* Try opening tty's and remove from list those that can't be opened. */
	for ( q = &me; p = q->p_next; q = p )
	{       cp = tty;
		CScat( cp, "/dev/", p->p_ttyname, (char *) NULL );
		/* In POSIX, opens of block structured devices can block and
		 * the O_NONBLOCK flag can be set, but there is no way to 
		 * detect that it would have blocked.  We have to wait until
		 * we try to write to the device to discover that there is an
		 * error.
		 */
		if ( (p->p_file.p_fd = open( cp, O_WRONLY | O_NONBLOCK )) < 0 )
		{       printf(MSGSTR(M_MSG_7, "Can't write to %s on %s\n"),
				p->p_name, p->p_ttyname );
			q->p_next = p->p_next;
			free( (void *)p );
			p = q->p_next;
		}
		else
		{
			makeid( p );
			/* Indicate that we've opened the tty but that we
			 * have not yet successfully written to it.  There is
			 * the possibility that the open would have blocked, 
			 * but there is no way to detect that fact without
			 * first trying to write to it.
			 */
			p->p_status = P_OPEN;
		}
	}

	rec2 = recording;

	/* At this point, the p_ttyspec will refer to the transcript file
	 * name.  Since there is a possibility that the tty (or pts) is of 
	 * the form pts/xx, we need to remove the '/' so that we can create
	 * the transcript files with these names in TMPDIR.  Only the 
	 * conference leader needs to have them all converted in this way.
	 * The issuer of joinconf only needs to have his one transcript
	 * file purged of spurious '/'s.  But it's simpler to do them all here.
	 */
	for ( p = &me; p; p = p->p_next )
		while ((j = anystr( p->p_ttyspec, "/" )) != -1)
			p->p_ttyspec[j] = '.';

	/* Create temporary directory if it doesn't exist.  There is probably
	 * no need to remove it within the program.
	 */
	if (!exists( TMPDIR ))
	{
		j = umask( (mode_t)0 );
		i = mkdir( TMPDIR, 0777 );
		umask( (mode_t)j );
	}

	/* Create transcript file in temporary directory and link master
	 * to the sync file for this user.
	 */
	if (confercom)
	{
		if ((transcript = creatcnf()) == NULL )
			done( EXITBAD );
	}
	else
	{	/* make my private transcript file */
		CScat(conffile,TMPDIR,confname,atchar,me.p_ttyspec,
								(char *)NULL);
		CScat(syncfile,TMPDIR,confname,"&",me.p_ttyspec,(char *)NULL);
		j = umask( (mode_t)0 );
		i = creat( conffile, 0666 );
		umask( (mode_t)j );
		if (i < 0 || (transcript = fdopen( i, "w" )) == NULL)
		{       fprintf(stderr,MSGSTR(M_MSG_8,"Can't create %s\n"),
			    conffile);
			done(EXITBAD);
		}
		link(master, syncfile);
	}

	signal( SIGHUP, interrupt );
	signal( SIGQUIT,interrupt );
	signal( SIGINT, interrupt );

	if (confercom)
		setupconf();
	else
	{       sprintf( linebuf,MSGSTR(M_MSG_31,"** %s%s%s has joined\n"),
			 me.p_name, atchar, me.p_ttyname);
		writeall( linebuf, (int) rec2, TRUE );
	}

	printf(MSGSTR(M_MSG_9, TAKEFLOORMSG));

	/* reset lastchar for subsequent readline() calls */
	writeall("\n", FALSE, TRUE);
	idstate = IDENTIFY;
	/* no calls to writeall() must exist between here and the while below */

	/* This is the main loop where the program reads lines of input.
	 * The readline() function sends the lines to everyone else in the
	 * conference.  The loop continues until EOF is encountered or an
	 * interrupt occurs.
	 */
	while (!intflag && readline( &lineptr ) != EOF)
	{
		switch( *lineptr )
		{ case '!':
			subshell( lineptr+1, FALSE );
			/* Fall through */
		  case '~':
			continue;
		  case '|':
			if (rec2) writecnf( lineptr, FALSE );
			subshell( lineptr+1, TRUE );
			continue;
		}
		if (rec2) writecnf( lineptr, FALSE );
	}

	/* Turn off raw mode since it is not necessary and because it will
	 * interfere with the prompting for transcript and possibly other
	 * termination activities.
	 */
	if (raw) ioctl(0, TCSETAF, &ttmodes);
	raw = FALSE;

	idstate = IDENTIFY;

	writeall( MSGSTR(M_MSG_33,"BYE\n"),recording, TRUE );

	/* close all tty's */
	for (p = me.p_next; p; p = p->p_next)
	{       if (*p->p_ttyname == '\0') continue;
		close( p->p_file.p_fd );
		p->p_file.p_fd = 0;
	}

	fclose( transcript );

	/* Create mailing list, if necessary.  Make sure I'm on the mailing
	 * list.  If this is a confer invocation, mail the merged transcript
	 * to everyone on the mailing list.
	 */
	if (recording)
	{
		printf("\n");	/* not part of the dialog */
		printf(MSGSTR(M_MSG_10,"Transcript? (n) "));
		fflush (stdout);
		if (getresponse() == 1)
		{	CScat( linebuf, TMPDIR, confname, mlist, (char *)NULL );
			/* open or create transcript mailing list file */
			if ((i = open( linebuf, O_WRONLY )) < 0 )
			{	j = umask( (mode_t)0 );
				i =  creat( linebuf, 0666 );
				umask( (mode_t)j );
			}
			if (i < 0 )
				fprintf(stderr,
				  MSGSTR(M_MSG_11,"Can't make %s\n"), linebuf);
			else
			{	/* write my name in mailing list file */
				CScat(linebuf, me.p_name, blank, (char *)NULL);
				j = strlen( linebuf );
				lseek( i, (long) 0, 2 );
				/*  Vulnerable here to other writers */
				write( i, linebuf, (unsigned)j );
				close( i );
			}
		}
		if (confercom)
		{
			while ((pid = fork()) == -1) sleep( (unsigned)5 );
			if (pid == 0)
				/* child */
				maketran();
		}
	}
	done( EXITOK );
}


static void interrupt(int ignored)
{       signal( SIGINT, interrupt );
	signal( SIGQUIT,interrupt );
	signal( SIGHUP, interrupt );
	if (++intflag > 10) {
		deletefiles();
		done( EXITSIGNAL );
	}
}

static done( code )
{       if (raw) ioctl(0, TCSETAF, &ttmodes);
	if (joincom) unlink( syncfile );
	exit( code );
}

static makeid( p )
register struct person *p;
{
	CScat(p->p_id, "[", p->p_name,atchar,p->p_ttyname, "] ", (char *)NULL);
}


/* When this routine is called, we have a linked list of person structures,
 * starting with me, that were invited.  All we know is whether the user
 * and/or the tty was specified.  We will scan the utmp file to see who is
 * actually logged in and what tty(s) they are using.  It is possible to
 * determine whether I was invited without looking at utmp and this is done
 * before this routine is called.  The difficulty is in determining who to
 * write output to.
 *
 * The following comments are nearly exact quotes from the man pages:
 *
 * "A user who is logged in to more than one workstation is normally written to
 * on all of them, unless the conference leader specifies one of the
 * workstations with the @ttyNumber flag when the confer command is issued."
 * 
 * "When inviting user@ttyNumber, if no user is specified, any user logged in
 * to the specified workstation may participate."
 * 
 * "Example:
 * confer jeanne@tty5 ron @tty10  -  [Note well: There is a blank space
 * between ron and @tty10.]  If user jeanne is logged in at tty3, tty4,
 * and tty5, she can join the conference from tty5 only.  If ron is logged in
 * at tty7 or tty8, he can join the conference from either tty7 or tty8, or
 * from both.  The user at tty10 is also invited to join the conference."
 *
 * "If ron decides to join the conference from tty7, the conference dialog is
 * also displayed at his other workstation tty8, unless everyone (including
 * ron), enters !excuse ron@tty8."
 *
 * When this routine is called, the only person on the person list having
 * p_ttyname set is me.
 */
static gettys(list)
struct person *list;
{
	register struct person *p, *q;
	FILE *userfile;
	char tty[TTYNAMMAX+1];
	char name[USRNAMMAX+1];

	/* open UTMP_FILE which contains info on logged in users */
	if ((userfile = fopen( UTMP_FILE, "r" )) == NULL)
	{       fprintf(stderr,MSGSTR(M_MSG_12, "Can't read %s\n"), UTMP_FILE);
		done(EXITFILE);
	}

	/* read UTMP_FILE one user record at a time */
	while ( fread( (void *)&utmp, (size_t)sizeof( struct utmp), (size_t)1,
							userfile) != 0 )
	{
		/* only running user processes should be examined */
		if (utmp.ut_type != USER_PROCESS)
			continue;

		/* get device name */
		strncpy(tty, utmp.ut_line, (size_t)TTYNAMMAX);
		tty[TTYNAMMAX] = '\0';

		/* get user name */
		strncpy(name, utmp.ut_name, (size_t)USRNAMMAX);
		name[USRNAMMAX] = '\0';

		/* At this point we have a utmp entry.  Find out if he is on
		 * our list of invitees and get his ttyname if he is.
		 */
		for (p = list; q = p->p_next; p = q)
		{
			/* if user specified and doesn't match, ignore him */
			if (*q->p_name && strcmp( name, q->p_name ))
				continue;

			/* if tty specified and doesn't match, ignore him */
			if (*q->p_ttyspec && strcmp( tty, q->p_ttyspec ))
				continue;

			/* If user not specified and tty matches, print a
			 * message to that effect and set his user name.
			 * (He was already included as a result of the above
			 * test.)
			 */
			if (strcmp(tty,q->p_ttyspec) == 0 && *q->p_name == '\0')
			{       strcpy( q->p_name, name );
				printf(MSGSTR(M_MSG_13, "%s on %s\n"),
					q->p_name, tty );
			}

			/* We have a utmp entry which matches an invitee.  If
			 * the tty of the invitee was not specified, then it
			 * is possible that other utmp entries will also be
			 * invited.  The first of these causes this utmp
			 * entry's ttyname to be assigned to the p_ttyname
			 * field of the person struct.  After that, we need a
			 * new person struct for each matching utmp entry.
			 */
			if (*q->p_ttyspec == '\0')
			{
			   if (*q->p_ttyname == '\0')
				strcpy( q->p_ttyname, tty );
			   else /* if this is another tty with same user */
				if (strcmp(q->p_ttyname, tty))
				{
					p->p_next = (struct person *)
						malloc( (size_t)sizeof *p );
					p = p->p_next;
					p->p_next = q;
					strcpy( p->p_name, q->p_name );
					*p->p_ttyspec = '\0';
					strcpy( p->p_ttyname, tty );
				}
			}
			else
				strcpy( q->p_ttyname, tty );

			break; /* utmp entry can only be used once */
		}
	}
	fclose( userfile );

	/* At this point, if a person structure is a participant, then
	 * both p_name and p_ttyname will have been set.  p_ttyspec may not
	 * yet have been set in some cases.
	 */

	/* Remove from the list those persons whose p_name or p_ttyname
	 * entries have not yet been set.
	 */
	for (p = list; q = p->p_next; p = q)
	{
		if (*q->p_ttyspec == '\0')
			strcpy( q->p_ttyspec, q->p_ttyname );
		if (*q->p_name && *q->p_ttyname)
			continue;
		/* At this point, either p_name or p_ttyname are not set */
		if (*q->p_name)
		{
		   if (*q->p_ttyspec)
			printf(MSGSTR(M_MSG_50,"%s is not logged in on %s\n"),
						q->p_name, q->p_ttyspec);
		   else
			printf(MSGSTR(M_MSG_14,"%s is not logged in\n"),
							q->p_name );
		}
		else
			printf(MSGSTR(M_MSG_15, "Nobody logged in on %s\n"),
				q->p_ttyspec );
		p->p_next = q->p_next;
		free( (void *)q );
		q = p;
	}

#ifndef TEST
	/* I'm not testing.  Therefore, the person list should
	 * contain me only as the head of the list.  If another copy
	 * exists, remove it.
	 */
	for (p = list; q = p->p_next; p = q)
		if (strcmp( q->p_ttyname, list->p_ttyname ) == 0 &&
		   strcmp( q->p_name, list->p_name ) == 0)
		{
			p->p_next = q->p_next;
			free( (void *)q );
			q = p;
		}
#endif
}


/* Create conference (transcript) file.  The name of the transcript file
 * (conffile) is composed here by appending the suffix to the conference name
 * (confname).  If necessary, select new conference name by appending -0, -1,
 * etc., to the original name.  Up to nine conferences with the same original
 * name may be active at one time.
 */
static FILE *creatcnf()
{       register int fd, i, j;

	for (i = 0; ; i++)
	{
		CScat( conffile, TMPDIR, confname, suffix, (char *) NULL);
		/* create succeeds if the file already exists as long as 
		 * the process has write permission on the file.
		 */
		if (!exists( conffile ))
		{
			j = umask( (mode_t)0 );
			fd = creat( conffile, 0666 );
			umask( (mode_t)j );
			if (fd >= 0)
				break;
		}
		if (i == 0)
			CScat( confname, confname, "-0", (char *) 0 );
		else if ( i >= 9 )
		{       fprintf(stderr,MSGSTR(M_MSG_16,
				"Can't create conference (%s)\n"),confname);
			return(NULL);
		}
		confname[ strlen( confname ) - 1 ] ++;
	}

	/* create master file (name without suffix) used for linking to
	 * syncfiles.
	 */
	CScat( master, TMPDIR, confname, (char *) NULL);

	/* We guaranteed that conffile was unique.  If its corresponding
	 * syncfile already exists, we are going to commandeer it.
	 */
	j = umask( (mode_t)0 );
	i = creat( master, 0666 );
	umask( (mode_t)j );
	if ( i < 0 )
	{
		fprintf(stderr,MSGSTR(M_MSG_16,
			"Can't create conference (%s)\n"),confname);
		return(NULL);
	}

	return( fdopen( fd, "w") );
}


/* Write confer command in first line of transcript file and send 
 * invitation to others and tell them to use joinconf to join the conference.
 * This routine does not wait to see who joins.
 */
static setupconf()
{       register struct person *p;
	char user[20];
	char *bp;	/* ptr to allocated buffer containing invitation */
	size_t bsize;	/* size of allocated buffer containing invitation */

	/* set p_status of tty */
	for(p=&me; p; p=p->p_next)
		writeall( "\n", FALSE, TRUE );

	if (!me.p_next)
		done( EXITOK );

	/* Form confer command for inclusion in transcript file. */
	bsize = LINELEN;
	if ((bp = malloc(bsize)) == (void *)NULL)
	{
		printf(MSGSTR(M_MSG_49,"Too many participants.\n"));
		exit(1);
	}
	sprintf( bp, "confer %s -n%s %s",
		(raw ? "-v" : nullstr), confname, (rec2 ? nullstr : "~"));

	/* Concatenate users to end of command */
	for (p = &me; p; p = p->p_next)
	{
		sprintf( user, " %s%s%s", p->p_name, atchar, p->p_ttyname );
		if (strlen(bp)+strlen(user)+2 > bsize)
		{
			if ((bp = realloc(bp, bsize+LINELEN)) == (void *)NULL)
			{
				printf(MSGSTR(M_MSG_49,
						"Too many participants.\n"));
				exit(1);
			}
			bsize += LINELEN;
		}
		CScat( bp, bp, user, "\n", (char *) NULL );
	}

	/* write full confer command to transcript file */
	writecnf( bp, FALSE );
	fflush( transcript );

	/* Form invitation to be sent to invited participants */
	sprintf( bp, MSGSTR(M_MSG_34,
		"\007Conference %s%srequested with %s%s%s"),
		(rec2 ? nullstr : MSGSTR(M_MSG_44,"(off the record) ")),
		(raw    ? MSGSTR(M_MSG_45,"(verbose) ") : nullstr),
		me.p_name, atchar, me.p_ttyname);
	for (p = &me; p = p->p_next; )
	{
		sprintf( user, ", %s%s%s", p->p_name, atchar, p->p_ttyname );
		if (strlen(bp)+strlen(user)+2 > bsize)
		{
			if ((bp = realloc(bp, bsize+LINELEN)) == (void *)NULL)
			{
				printf(MSGSTR(M_MSG_49,
						"Too many participants.\n"));
				exit(1);
			}
			bsize += LINELEN;
		}
		CScat( bp, bp, user, (char *) NULL );
	}

	writeall( bp, FALSE, TRUE ); /* send invitation */
	free(bp);

	/* form message instructing the invitee on how to join the conference */
	sprintf( linebuf, MSGSTR(M_MSG_35,"\n\007Please %s %s\n"),
		joinname, confname );

	writeall( linebuf, FALSE, TRUE ); /* send instruction */
}


/* Only called for joinconf command.  Fix up a new argv, and return a new
 * argc.
 */
static getconf( argv )
register char ***argv;
{       char **oldargv;
	char leadtranscript[PATH_MAX];
	register FILE *mfile;
	register time_t mastertime;

	oldargv = *argv;

	CScat(master,TMPDIR,substr(oldargv[1], confname, 0, 8), (char *) NULL );

	/* form name of transcript file for conference leader */
	CScat( leadtranscript, master, (char *) NULL );
	CScat( leadtranscript, leadtranscript, suffix, (char *) NULL );

	/* See if the transcript file, written by the issuer of confer, can be
	 * read.  If so, get the first record which contains a list of the 
	 * invited users.  Look at the time that the lead transcript file
	 * was created and ensure that it was create within the last hour.
	 */
	if ((mfile = fopen( leadtranscript, "r") ) == NULL ||
		(mastertime =
		   readcnf( mfile, &me.p_header, &tscriptptr, RDHEADER|RDDATA ))
			< now - (60*60) || mastertime > now)
	{	fprintf(stderr,MSGSTR(M_MSG_18,
			"No current `%s' conference\n"), confname );
		done( EXITBAD );
	}

	fclose( mfile );

	/* Parse the list of invited users and build a new argv which looks
	 * like the argv that would have existed if this had been a confer
	 * invocation.  This is done so that issuers of joinconf can 
	 * construct the list of persons participating in the conference.
	 * Doing it this way implies that users may not join a conference
	 * to which they were not initially invited, even if they know the
	 * name of the conference.
	 */
	return(parse(tscriptptr, argv)); /* return argc. construct new argv */
}


/* Write buf to all tty's.  Record if rec is true.  idstate determines 
 * whether the leading identifier, [user@ttyNumber], is prepended to the
 * text which is written to the participant ttys.  In general, if
 * idstate == -1, no identifier is prepended.  If idstate == 2, then
 * the identifier is prepended and idstate is then reset to 1 if the 
 * current line is blank and 0 if non-blank.  If idstate == 0 or 1, then the 
 * next blank line will cause idstate to be bumped.  This method has the
 * desireable property that identifiers are prepended only when the person
 * who has the floor changes, assuming that the protocol outlined in the man 
 * pages is followed.
 */
static writeall( buf, rec, echostatus )
char *buf;
int echostatus;	/* TRUE ==> don't echo to myself even if raw mode */
{
	static char lastchar = '\n';	/* 1st character from the last buffer */

	if ( rec ) writecnf( buf, FALSE );

	switch( idstate )
	{
		case -1:
			dowrite( buf, strlen( buf ), echostatus );
			break;
		case 0:
		case 1:
			dowrite( buf, strlen( buf ), echostatus );

			/* We're really trying to see if this is a blank line */
			if ((raw && lastchar == '\n' && *buf == '\n')
						|| (!raw && *buf == '\n'))
				idstate += 1;
			else
				idstate = 1;

			break;
		case 2:
			if ((raw && lastchar == '\n' && *buf == '\n')
						|| (!raw && *buf == '\n'))
			{
				/* write line before id */
				dowrite( buf, strlen( buf ), echostatus );

				/* if raw, then dowrite writes to everyone,
				 * including me.  Otherwise, I have to
				 * explicitly write the p_id to myself.
				 */
				dowrite(me.p_id, strlen (me.p_id), echostatus);
				if (!raw)
					write( 1, me.p_id, strlen (me.p_id) );

				idstate = 1;
			}
			else
			{
				dowrite(me.p_id, strlen (me.p_id), echostatus);
				if (!raw)
					write( 1, me.p_id, strlen (me.p_id) );

				/* write line after id */
				dowrite( buf, strlen( buf ), echostatus );

				idstate = 0;
			}

			break;
	}
	lastchar = *buf;
}


/* Write cnt bytes of buf to the tty of each person in the linked list of
 * persons.  If error during write, print message indicating that the
 * corresponding person is gone and remove him from my list.
 */
static dowrite( buf, cnt, noecho )
register char *buf;
register int cnt;
register int noecho;	/* TRUE ==> don't echo to myself even if raw */
{	register struct person *p, *q;

	for (p = &me; p; q = p, p = p->p_next)
	{
		if (*p->p_ttyname == '\0' || (!raw && p == &me)) continue;
		if (noecho && p == &me) continue;
		if (write( p->p_file.p_fd, buf, (unsigned)cnt ) < 0 )
		{
			if (p->p_status == P_WRITTEN)
			{
				printf(MSGSTR(M_MSG_19,"\n** %s%s%s is gone\n"),
					p->p_name, atchar, p->p_ttyname);
				/* We don't want to remove from list since
				 * he might want a transcript.
				 */
				close( p->p_file.p_fd );
				*p->p_ttyname = '\0';
			}
			else
			{
				/* we were never able to write to him */
				printf(MSGSTR(M_MSG_7,
					"Can't write to %s on %s\n"),
					p->p_name, p->p_ttyname );
				q->p_next = p->p_next;
				free( (void *)p );
				p = q;
			}
			continue;
		}
        /*  Output is being sent faster than tty or pts can display it.
            Added a 0.1 second break (i.e., usleep(100000);) so that tty
            or pts can display output completely before sending the next
            buffer of data.  NOTE:  This delay does not guarantee that
            the timing problem cannot occur again.  d54821 */
		usleep(100000);
		p->p_status = P_WRITTEN; /* indicate tty successfully written */
	}
}


/* All transcript file I/O is done in the following two routines.
 * When encryption is added, the changes go here.  It will be necessary
 * to save some state information on the crypto machine in the header
 * structure (expanded for that purpose), and to initialize it.
 */

/* Write info in buf into transcript file */
static writecnf( buf, shcmd )
register char *buf;
int shcmd;
{       register struct header *hdr;

	hdr = &me.p_header;
	hdr->nbytes = strlen( buf );
	time( &hdr->timestamp );
	hdr->shflag = shcmd;
	fwrite((void *)hdr,(size_t)sizeof(struct header),(size_t)1,transcript);
	fwrite((void *)buf,(size_t)hdr->nbytes,(size_t)1,transcript );
}


/* Read next available hdr and/or buf (determined by flags) from the specified 
 * conference file.  If not reading the hdr, then the header must have 
 * already been read using this function (so the realloc could have been done,
 * if required) and contain valid information.  If error while reading
 * hdr, set hdr timestamp to zero.  In case of any error, return NULL.
 * Otherwise, return the hdr timestamp.  The buf argument points at a pointer
 * to the buffer to contain the data to be read.  This design allows very 
 * large buffers to be read from transcript files that might have resulted 
 * from subshells.
 */
time_t readcnf( file, hdr, buf, flags )
register FILE *file;
register struct header *hdr;
char **buf;	/* buf points at static storage for ptr to malloc'd buffer */
register int flags;
{
	static int maxcount;	/* current size of malloc'd buffer in bytes */

	/* If this is the first call to this routine, malloc initial buffer
	 * which may be realloc'd if necessary.
	 */
	if (*buf == NULL)
	{
		maxcount = 0; /* size before first allocation in case of fail */
		if ((*buf = malloc(LINELEN)) == (char *)NULL)
			return(0);
		maxcount = LINELEN;
	}

	if (flags & RDHEADER)
	{
		if (fread((void *)hdr,(size_t)sizeof(struct header),
				(size_t)1, file) != 1)
		{
			hdr->nbytes = 0;
			return( hdr->timestamp = 0 );
		}
		/* the realloc needs to be done here (immediately after
		 * reading the header) so any error will be returned when
		 * the header is being read, the only time the error is 
		 * looked at.
		 */
		if (hdr->nbytes > maxcount)
		{
			if ((*buf = realloc(*buf, hdr->nbytes)) == NULL)
				return(0);
			maxcount = hdr->nbytes; /* update AFTER realloc */
		}
	}

	if ((flags & RDDATA) && fread((void *)*buf, (size_t)hdr->nbytes,
				(size_t)1, file ) != 1)
		return(0);
	else
		*(*buf + hdr->nbytes) = (char)NULL; /* null terminate RDDATA */

	return( hdr->timestamp );
}


/* Read the next line of input from my stdin (tty) up to and including the
 * newline character into a malloc'd buffer and NULL terminate.  If raw mode,
 * the read will complete after each character.  When calling, set *buf to 
 * be the pointer to the malloc'd buffer.  Return the number of bytes actually
 * read not including NULL.  If raw, return EOF on INT, QUIT, or EOT.
 * Return EOF if intflag is set on entry.  At any time, if EOF is returned,
 * *buf, the returned pointer to the malloc'd buffer, is undefined.  The
 * return buffer is malloc'd to ensure that the entire line available from the
 * tty is read and that no partial characters are written to stdout and the
 * others' ttys.  If send is TRUE and EOF is not returned, a writeall() of
 * the characters read is performed.  If raw mode, this writeall is performed
 * on every character.
 */
static readline( buf )
char **buf;	/* buf points at static storage for ptr to malloc'd buffer */
{
	register int ch;	/* current byte from stdin */
	register char *cp;	/* current pointer into malloc'd buffer */
	register char *rawptr;	/* ptr to str to be written in raw mode */
	register int count = 0;	/* current number of bytes in malloc'd buffer */
	static int maxcount;	/* current size of malloc'd buffer in bytes */
	int send = TRUE;	/* TRUE ==> writeall(). */
	char blstr[20];		/* buffer to contain blanking string */
	int n,nt;		/* # display columns */
	wchar_t wc;		/* current character being deleted */

	if (intflag) return( EOF );

	/* If this is the first call to this routine, malloc initial buffer
	 * which may be realloc'd if necessary.
	 */
	if (*buf == NULL)
	{
		maxcount = 0; /* size before first allocation in case of fail */
		if ((*buf = malloc(LINELEN)) == (char *)NULL)
			return(EOF);
		maxcount = LINELEN;
	}
	cp = *buf;

	/* Treat first character in line separately */
	ch = getch();
	switch (ch)
	{  case '!':
		send = FALSE;
		break;
	   case INTERRUPT:
		interrupt(0);
		/* Fall through */
	   case EOF:
		return( EOF );
	}

	while (!intflag)
	{
	   rawptr = cp;
	   switch( ch )
	   {
		case INTERRUPT:
			interrupt(0);
			/* Fall through */
		case EOF:
			*cp = '\0';
			return( count );
		case '\b':
			/* can only get this character if we are in raw mode */
			if (cp > *buf)
			{
				/* backup one multibyte character */
#ifdef COMMENT
				/* single byte code sets only */
				--cp;
				if (send)
					writeall("\b \b", FALSE, FALSE);
#endif
				/* multibyte code sets */
				cp = _mb_backup(cp, *buf);
				count -= mbtowc(&wc, cp, mb_cur_max);
				n = wcwidth(wc);
				if (n < 1) break;
				blstr[0] = '\0';
				for (nt=n; nt--; ) strcat(blstr, "\b");
				for (nt=n; nt--; ) strcat(blstr, " ");
				for (nt=n; nt--; ) strcat(blstr, "\b");
				if (send)
					writeall(blstr, FALSE, FALSE);
			}
			break;
		case '\n':
			*cp++ = ch;
			++count;
			*cp = '\0';
			if (send)
			{
				if (raw)
				        writeall( rawptr, FALSE, FALSE );
				else
				        writeall( *buf, FALSE, FALSE );
			}
			return( count );
		case '\t':
			if (raw)
				/* for now ... */
				ch = ' ';
			/* fall through */
		default:
			*cp++ = ch;
			if (send)
			{
				if (raw)
				{
					*cp = '\0';
				        writeall( rawptr, FALSE, FALSE );
				}
			}
			if (++count <= maxcount-2) /* room for '\n' and '\0' */
				break;
			if ((*buf = realloc(*buf, maxcount+LINELEN)) == NULL)
				return(EOF);
			maxcount += LINELEN; /* update AFTER realloc */
			cp = *buf + count;   /* new ptr in new buffer */
	   }
	   ch = getch();
	}
	*cp = '\0';
	return( count );
}


/* Return next byte in input buffer from stdin (which must be a tty).  If the
 * buffer is empty, do a read from stdin.  A partial multibyte character may
 * be returned.  If no data is available from the tty, the read will block.
 */
static getch()
{	static char buf[LINELEN];
	static char *next= buf;
	static int nleft = 0;
	register int ch;

	if (nleft <= 0)
	{
		nleft = read( 0, buf, sizeof buf );
		if (nleft <= 0)
			return( EOF );
		next = buf;
	}

	ch = *next++;	/* 8th bit no longer ANDed off */
	nleft--;
	if (raw)
	{	switch( ch )
		{ case CEOT:
			return( EOF );
		  case CINTR:
		  case CQUIT:
			return( INTERRUPT );
		}
	}
	return( ch );
}


/* Spawn subshell to execute argument string.  If filter is TRUE, write
 * output to all terminals and, if record, pipe and writeall the results.
 * Checks for special commands, like "excuse".
 */
#define SMALL 128
static subshell( buf, filter )
register char *buf;
{       static char shdone[] = "!\n";	/* indicates subshell is finished */
	register int i;
	char **argv;
	int pipefd[2], pid;
	register int saveid;
	int (*output)(char *);
	int metoo(char *);
	char smallbuf[ SMALL+1 ];       /* Small to allow overlap of */
					/* output to multiple terminals */
					/* (So ttyhiwat doesn't catch us */
	char *shellnam;

	shellnam = "/usr/bin/ksh"; /* d70416 */
	output = ( filter ? (int (*)(char *))metoo : (int (*)(char *))printf );
	if ((i = parse( buf, &argv )) > 0)
	{
		if (strcmp( *argv, "excuse") == 0)
		{       excuse( i, argv, output );
			freeargs( argv );
			return;
		}
		else
			if (((*argv)[0] == '~') != 0)
			{
				i = ( (*argv)[1] == '~' );
				rec2 = recording;
				sprintf( buf, "** %s %s\n", me.p_name,
					(i ? MSGSTR(M_MSG_38,"recording"):
					   MSGSTR(M_MSG_39,"off the record")));
				metoo( buf );
				rec2 = i;
				return;
			}
		/* else if ... for other special commands */
	}

	if (filter)
		if (pipe( pipefd ) < 0)
		{       metoo( MSGSTR(M_MSG_40,"Not enough files for '!'\n"));
			return;
		}

	while ((pid = fork()) == -1) sleep( (unsigned)5 );

	if (pid == 0)
	{       if (filter)
		{       close( 1 );
			dup( pipefd[1] );
			close( 2 );
			dup( 1 );
		}
		for ( i = 3; i < 30; i++ ) close( i );
		execlp(shellnam, CSsname(shellnam), "-c", buf, NULL);

		/* if the exec fails... */
		write(2,MSGSTR(M_MSG_41,"No shell\n"),
                   strlen(MSGSTR(M_MSG_41,"No shell"))+1);
		exit(1);
	}

	signal( SIGQUIT, SIG_IGN );
	signal( SIGINT, SIG_IGN );
	signal( SIGHUP, SIG_IGN );
	if (filter)
	{       close( pipefd[1] );
		saveid = idstate;
		idstate = NOIDENTIFY;
		while ((i =  read( pipefd[0], smallbuf, (unsigned)SMALL )) > 0)
		{       smallbuf[ i ] = '\0';
			metoo( smallbuf );
		}
		close( pipefd[0] );
		wait((int *) 0); /* wait for child to finish */
		metoo( shdone ); /* indicating subshell is finished */
		if (saveid != NOIDENTIFY) idstate = 1;
	}
	else
	{       wait((int *) 0);
		printf( shdone );
	}
	signal( SIGQUIT, interrupt );
	signal( SIGINT,  interrupt );
	signal( SIGHUP,  interrupt );
	intflag = FALSE;
}


/* Write buf to my standard out, to the other conferees, and to the
 * transcript file.  Only called by subshell().
 */
static metoo(char *buf)
{
	printf( buf );

	/* don't record since we don't want to affect the idstate */
	writeall( buf, FALSE, FALSE );

	if (rec2) writecnf( buf, TRUE );
}


/* Make transcript.  Close terminals and fopen transcripts. */
#define SLEEP1  10      /* Time to sleep the first time in geometric prog */
#define NSLEEPS 8       /* Time to give up after SLEEP1 * (2^NSLEEPS -1)  */
			/* As set, 10*(255) secs = 42.5 minutes */
static maketran( )
{       register struct person *p, *q;
	register int i;
	struct stat mlstat;	/* mail list stat buffer */
	char *bp;		/* ptr to buffer containing mail command */
	struct person *r;
	char **recipients;
	int j;
	int pid;
	int nrecips;
	unsigned int sleeptime;
	int mailpipe[ 2 ];
	char tfile[PATH_MAX];	/* transcript file name */
	char tsbuf[TSSIZE];	/* buffer to hold time string */

	/* This procedure unlinks files immediately after opening them, before
	 * doing I/O. This saves keeping their names around for unlinking later,
	 * and is a defense against leaving the files around in the event of a
	 * failure.  The POSIX specification of unlink() makes this portable.
	 */

	signal( SIGINT, SIG_IGN);
	signal( SIGQUIT, SIG_IGN);
	signal( SIGHUP, SIG_IGN);
	close( 0 );
	close( 1 );
	close( 2 );

	/* Loop until the master file no longer exists or the number
	 * of links on it are 0 or 1.
	 */
	for (sleeptime = SLEEP1, i = NSLEEPS+1; --i; )
	{       /* exists (defined in macros.h) sets Statbuf */
		if (!exists( master ) || Statbuf.st_nlink <= 1)
			break;
		sleep( sleeptime );
		sleeptime += sleeptime;
	}
	unlink( master ); /* remove, in case the number of links was 1 */

	/* Open the transcript file for each participant */
	me.p_file.p_fp = fopen( conffile, "r" );
	for ( p = &me; p = p->p_next; )
	{       if (p->p_file.p_fd > 0) close( p->p_file.p_fd );
		CScat(tfile,TMPDIR,confname,atchar,p->p_ttyspec,(char *)NULL);
		p->p_file.p_fp = fopen( tfile, "r" );
	}

	/* Get name of mailing list file and try to open it.  If it doesn't
	 * exist, assume no one wants a copy of the transcript and return.
	 */
	CScat( mlistfile, TMPDIR, confname, mlist, (char *) NULL );
	if ((i = open( mlistfile, O_RDONLY )) < 0)
	{
		/* nobody wants transcript */
		deletefiles();
		return;
	}

	/* get size of file in bytes and malloc buffer for mail command */
	stat(mlistfile, &mlstat);
	if ((bp = malloc(mlstat.st_size + strlen(mailcom) + 4)) == (void *)NULL)
	{
		deletefiles();
		return;
	}

	CScat( bp, CSsname( mailcom ), blank, (char *) NULL );
	j = strlen( bp );
	j += read( i, bp+j, mlstat.st_size);
	*(bp+j) = '\0';
	if (( nrecips = parse( bp, &recipients )) <= 1 )
	{
		deletefiles();
		return;
	}
	close( i );

	/* Fork a process to run the mail command.  Before the fork(), open
	 * a pipe for talking to mail.  When the child is born, he makes the
	 * pipe into his stdin and execs mail with the list of recipients as
	 * its argument list.  Everything written to the mail pipe becomes
	 * part of the transcript sent to each of the recipients.
	 */
	pipe( mailpipe );

	while ((pid = fork()) == -1) sleep( (unsigned)5 );

	if (pid == 0)
	{
		/* child */
		char *av[40];		/* only 40 recipients allowed */

		close( 0 );
		dup( mailpipe[ 0 ] );	/* force pipe reads from stdin */
		while( ++i < 30 ) close( i );

		av[0] = CSsname(mailcom);
		for (i=1; i < nrecips; i++)
			av[i] = recipients[i];
		av[i] = NULL;

		execvp(mailcom, av);
		exit(0);
	}

	transcript = fdopen( mailpipe[1], "w" );

	strftime(tsbuf, TSSIZE, nl_langinfo(D_T_FMT), localtime(&now));

	/* write transcript header to mail pipe */
	fprintf( transcript,MSGSTR(M_MSG_20,
		"Subject: Conference (%s) of %s\nTo: ") ,
		confname, tsbuf );

	/* write list of recipients to mail pipe */
	for (i = 1; i < nrecips - 1; i++)
		fprintf( transcript, "    %s,", recipients[ i ] );
	fprintf(transcript, "    %s", recipients[ i ]);
	fprintf(transcript, "\n--------\n");

	/* The rest of this routine is devoted to creating the transcript
	 * and writing it to the mail pipe to be sent to the recipients.
	 * Each conference participant maintained his own transcript file,
	 * consisting of (hdr,line) pairs.  The hdr contains the timestamp and 
	 * the length in bytes corresponding to the line.  The line is the
	 * data that the participant typed (if it was on the record).  Here,
	 * we are going to compose a merged transcript by building a linked
	 * list of person structures, each pointing to one (hdr,line) pair,
	 * which is the next transaction from that person,
	 * sorted by time from earliest to latest, and we are going to write
	 * the earliest transaction to the mail pipe transcript when we are
	 * sure that there aren't any that are earlier.  Each participant
	 * may have only one entry in the transaction list at a time.
	 * After the transaction has been written to the mail pipe, it will
	 * be removed from the head of the list.
	 */

	/* Run through participant list once.  For each participant, check
	 * to see if he has a transcript file and if not, print a message
	 * to that effect.  Then read the first transaction hdr from his 
	 * transcript file and insert it in the transcript list in sort.
	 * The corresponding line will be read in the next while loop.
	 */
	q = NULL;       /* q is the transcript linked list header */
	for (p = &me; p; p = r)
	{	r = p->p_next; /* get ptr, in case this gets inserted */
		if (p->p_file.p_fp == NULL && p->p_status == P_WRITTEN)
			fprintf(transcript,
				MSGSTR(M_MSG_24, "** No transcript for %s\n"),
				p->p_name);
		else if (readcnf( p->p_file.p_fp, &p->p_header, &tscriptptr,
				RDHEADER ))
			q = insert( p, q );
	}

	/* While there are still transactions in the transcript list ...
	 * print name of participant for this transaction, read the line
	 * he typed from his transcript file, write the line to the mail
	 * pipe, read his next header, and insert him on the list if the
	 * header exists.
	 */
	while ( q )
	{	p = q;              /* Remove head from list */
		q = p->p_next;
		readcnf( p->p_file.p_fp, &p->p_header, &tscriptptr, RDDATA );
		/* d54822, no prompts are needed in the output of shell, prompts
                   will cause multibyte characters wrongly displayed */
		if (!p->p_header.shflag)
			fprintf( transcript, "%s", p->p_id );
		fwrite((void *)tscriptptr, (size_t) p->p_header.nbytes,
				(size_t)1, transcript );
		if (readcnf(p->p_file.p_fp,&p->p_header,&tscriptptr,RDHEADER))
			q = insert( p, q );
	}
	fclose( transcript );
	deletefiles();
}



/* Since unlink() cannot be relied upon to wait until the files are closed
 * before removing them, we must wait until we are ready to exit before
 * deleting them.
 */
static deletefiles()
{
	register struct person *p;
	char tfile[PATH_MAX];	/* transcript file name */

	unlink( conffile );
	for ( p = &me; p = p->p_next; )
	{
		CScat(tfile,TMPDIR,confname,atchar,p->p_ttyspec,(char *)NULL);
		unlink( tfile );
	}
	unlink( mlistfile );
}


/* Insert structure pointed to by THIS into the linked list whose head is 
 * pointed to by LIST in sort from earliest to latest according to timestamp
 * of headers and return (potentially new) pointer to beginning of list.
 * This routine is used only for constructing the final transcript.
 */
static struct person *insert( this, list )
register struct person *this;	/* ptr to structure being inserted */
struct person *list;		/* ptr to head of list */
{	register struct person *p;

	if (list == NULL || this->p_header.timestamp < list->p_header.timestamp)
	{       this->p_next = list;
		return( this );
	}

	/* Locate the structure after which THIS is to be inserted.  Be
	 * careful to ensure that it is possible to insert at the end.
	 * It isn't possible for p to end up NULL unless it's timestamp is
	 * less than every one in the list, which is handled by the 
	 * previous if statement.
	 */
	for (p = list; p; p = p->p_next)
		if (this->p_header.timestamp >= p->p_header.timestamp
		   && (p->p_next == NULL ||
		   this->p_header.timestamp < p->p_next->p_header.timestamp))
			break;

	this->p_next = p->p_next;
	p->p_next = this;
	return( list );
}


/* Excuse the specified list of users from the conference.  If a specified 
 * user is not currently participating, print a message to that effect using
 * the specified output routine.  At least two output routines may be used.
 * metoo() does a printf() to myself and writes the buffer to everyone else's
 * tty.  printf() only write the message to myself.  For each excused
 * participant, close his tty and set ttyname = '\0'.  Return nothing.
 */
static excuse( n, args, output )
int n;			/* number of name@ttyNumber entries to excuse */
char *args[];		/* ptr to user[@ttyNumber] list */
int (*output)();	/* ptr to output routine, either metoo() or printf() */
{       int i, j;
	register struct person *p, *q;
	register char *cp;
	char term[8];
	char found;
	char buf[ LINELEN ];

	for (i = 1; i < n; i++)
	{       cp = args[i];	/* get pointer to argument */
		if ((j = anystr( cp, atchar )) >= 0)
		{       cp[j] = '\0';
			strcpy( term, cp + j + 1 );
		}
		else *term = '\0';
		for (found = FALSE, p = &me; q = p->p_next; p = q)
		{       if (*q->p_ttyname && strcmp( q->p_name, cp ) == 0 &&
					( *term == '\0' ||
					strcmp(term, q->p_ttyname) == 0 ))
			{       sprintf( buf, MSGSTR(M_MSG_42,
					"** %s has excused %s%s%s\n"),
					me.p_name, q->p_name, atchar,
					q->p_ttyname );
				(*output)( buf );
				close( q->p_file.p_fd );
				*q->p_ttyname = '\0';
				found = TRUE;
			}
		}
		if ( *term ) cp[j] = atchar[0];
		if ( ! found )
		{       sprintf(buf, MSGSTR(M_MSG_43, "** %s not found\n"), cp);
			(*output)( buf );
		}
	}
}


/* Parse the string in line into blank-separated strings.
 * Return argc and an "argv" type char ** in argv.
 * Doesn't look for quoted strings or escapes - simple internal use only.
 */
static parse( line, argv )
char *line;
char ***argv;
{       register char *cp, *cpsave;
	char **argptr;
	char buf[ LINELEN ];
	register int n = 0;
	int i;

	strcpy( buf, line );
	cp = buf;

	/* First count the args and null out whitespace */
	while (*cp)
	{       while (isspace( (int)*cp )) *cp++ = '\0';
		cpsave = cp;
		while (!isspace( (int)*cp ) && *cp) cp++;
		if ( cp == cpsave ) break;
		n++;
	}

	*argv = argptr = (char **) malloc( (size_t) ((n+1) * sizeof (char *)) );
	for (cp = buf, i = 0; i < n; i++)
	{       while (*cp == '\0') cp++;
		cpsave = cp;
		while (*cp != (char) 0) cp++;
		*argptr = malloc( (size_t) (cp - cpsave + 1) );
		strcpy( *argptr++, cpsave );
	}
	*argptr = 0;
	return( n );
}

static freeargs( argp )
register char **argp;
{       register char **p = argp;

	while (*p)
		free( (void *)*p++ );
	free( (void *)argp );
}


/* Reads a response from stdin, removes the first newline character if present,
 * and returns 1 if the response is affirmative, 0 if the response is negative,
 * and -1 if neither.  The default response is negative.
 */
static getresponse()
{
	char *p;

	if (getaline(linebuf, sizeof linebuf) == EOF)
		return(-1);
	if (linebuf[0] == '\n')
		return(0);
	if ((p = strchr( linebuf, (int)'\n' )) != NULL)
		*p = '\0';
	return(rpmatch(linebuf));
}


/* simple routine to read line of input from keyboard.  It's assumed that 
 * raw mode has been turned off and the tty has been returned to the cooked
 * line discipline.  The contents of the returned buffer are NOT sent to the
 * other participants.
 */
static getaline(buf, n)
char *buf;	/* pointer to destination buffer */
int n;		/* largest number of bytes read */
{
	register int count = 0;	/* # bytes read so far */

	while (!intflag && (*buf++ = getch()) != '\n' && count++ < n)
		;
	*buf = '\0';
}


/* Backup 1 multibyte character in a buffer whose beginning address is buf
 * from the character pointed to by cp.  Returns new character pointer.
 */
static char *_mb_backup(cp, buf)
char *cp;	/* ptr to current character */
char *buf;	/* pointer to beginning of buffer */
{
	int count;

	if (buf == cp) return(buf);

	while ((buf += (count = mblen(buf, mb_cur_max))) < cp)
		;
	return(buf -= count);
}
