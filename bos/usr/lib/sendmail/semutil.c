static char sccsid[] = "@(#)23	1.9  src/bos/usr/lib/sendmail/semutil.c, cmdsend, bos411, 9428A410j 1/14/94 17:43:58";
/* 
 * COMPONENT_NAME: CMDSEND semutil.c
 * 
 * FUNCTIONS: MSGSTR, Msemutil 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
**  Semutil - for use in semaphore maintenance for the sendmail program.
**
*/


#include <nl_types.h>
#include "semutil_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SEMUTIL,n,s) 

#include <locale.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>

key_t  ftok ();
char  *malloc ();

#define LOCKFILE	"Lockfile"
#define USAGE		"usage: semutil {value | remove} [<queue_directory_path>]\n"
#define DEFAULT_DIR	"/usr/spool/mqueue"

main (argc, argv)
int  argc;
char *argv[];
{
	key_t  key;
	int  sid;
	int  val;
	char  *dpath;
	char  *path;

        setlocale(LC_ALL,"");
	catd = catopen(MF_SEMUTIL,NL_CAT_LOCALE);

	if (argc < 2 || argc > 3)
	{
		fprintf (stderr, MSGSTR(ARG, "argument count error\n")); /*MSG*/
		fprintf (stderr, MSGSTR(SM_USAGE, USAGE)); /*MSG*/
		exit (-1);
	}

	if (argc == 2)
		dpath = DEFAULT_DIR;
	else
		dpath = argv[2];

	if ((path = malloc ((unsigned) (strlen (dpath) + strlen ("/") + strlen (LOCKFILE) + 1))) == NULL)
	{
		fprintf (stderr, MSGSTR(MEM, "memory allocation error\n")); /*MSG*/
		exit (-2);
	}
	(void) strcpy (path, dpath);
	(void) strcat (path, "/");
	(void) strcat (path, LOCKFILE);
	fprintf (stderr, MSGSTR(LOCK, "Lockfile path: %s\n"), path); /*MSG*/

	if ((key = ftok (path, 'M')) == (key_t) -1)
	{
		fprintf (stderr, "ftok: errno = %d", errno);
		perror (" ");
		exit (-3);
	}
	fprintf (stderr, MSGSTR(KEY, "key = 0x%lx\n"), key); /*MSG*/

	if ((sid = semget (key, 1, IPC_CREAT)) < 0)
	{
		fprintf (stderr, "semget: errno = %d", errno);
		perror (" ");
		exit (-4);
	}
	fprintf (stderr, "sid = %d\n", sid);

	switch (argv[1][0])
	{
	    case 'R':
	    case 'r':
		if (semctl (sid, (unsigned) 0, IPC_RMID, 0) < 0)
		{
			fprintf (stderr, "semctl: errno = %d", errno);
			perror (" ");
			exit (-5);
		}
		fprintf (stderr, MSGSTR(SEM, "semaphore removed\n")); /*MSG*/
		break;

	    case 'V':
	    case 'v':
		if ((val = semctl (sid, (unsigned) 0, GETVAL, 0)) < 0)
		{
			fprintf (stderr, "semctl: errno = %d", errno);
			perror (" ");
			exit (-5);
		}
		fprintf (stderr, "semval = %d\n", val);
		break;

	    default:
		fprintf (stderr, MSGSTR(UNKNOWN, "unknown command code %s\n"), argv[1]); /*MSG*/
		exit (-1);
	}

	exit (0);

/*NOTREACHED*/
}
