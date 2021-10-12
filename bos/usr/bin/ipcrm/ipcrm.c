static char sccsid[] = "@(#)47	1.9.1.3  src/bos/usr/bin/ipcrm/ipcrm.c, cmdipc, bos41B, 412_41B_sync 12/14/94 09:52:31";
/*
 * COMPONENT_NAME: (CMDIPC) ipc commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 3
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

#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#include	<sys/errno.h>
#include	<sys/syspest.h>
#include 	<grp.h>
#include	<stdio.h>
#include	<locale.h>
#include	<nl_types.h>
#include	"ipcrm_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_IPCRM,Num,Str)

char opts[] = "q:m:s:Q:M:S:";	/* allowable options for getopt */
extern char	*optarg;	/* arg pointer for getopt */
extern int	optind; 	/* option index for getopt */
extern int	errno;		/* error return */
int		rc;		/* return code */

/* define ipcrmdb large enough so the the BUG* debug messages 
 * will print out if code is compiled with -DDEBUG.  If DEBUG
 * is not defined these are no-ops.  (See sys/syspest.h) */
BUGVDEF(ipcrmdb,0x999)

#define DEC	0
#define HEX	1

/*
 * NAME:	ipcrm - IPC remove
 * FUNCTION:	Remove specified message queues, semaphore sets and shared 
 *		memory ids.
 */

main(argc, argv)
int	argc;	/* arg count */
char	**argv; /* arg vector */
{
	register int	o;	/* option flag */
	register int	err;	/* error count */
	register int	ipc_id; /* id to remove */
	register key_t	ipc_key;/* key to remove */
	extern	long	atol();
	extern	long	strtol();

	(void) setlocale (LC_ALL,"");

	catd = catopen(MF_IPCRM, NL_CAT_LOCALE);

	BUGLPR(ipcrmdb,BUGGID,("ruid %d\n",getuid()));

	/* Go through the options */
	err = 0;
	while ((o = getopt(argc, argv, opts)) != EOF)
		switch(o) {

		case 'q':	/* message queue */
			ipc_id = atoi(optarg);
			if (msgctl(ipc_id, IPC_RMID, 0) == -1)
				oops("msqid", (long)ipc_id, DEC);
			break;

		case 'm':	/* shared memory */
			ipc_id = atoi(optarg);
			if (shmctl(ipc_id, IPC_RMID, 0) == -1)
				oops("shmid", (long)ipc_id, DEC);
			break;

		case 's':	/* semaphores */
			ipc_id = atoi(optarg);
			if (semctl(ipc_id, 0, IPC_RMID, 0) == -1)
				oops("semid", (long)ipc_id, DEC);
			break;

		case 'Q':	/* message queue (by key) */
			ipc_key = (key_t)strtol(optarg,(char **)NULL,16);
			if ((ipc_id=msgget(ipc_key, 0)) == -1
				|| msgctl(ipc_id, IPC_RMID, 0) == -1)
				oops("msgkey", ipc_key, HEX);
			break;

		case 'M':	/* shared memory (by key) */
			ipc_key = (key_t)strtol(optarg,(char **)NULL,16);
			if ((ipc_id=shmget(ipc_key, 0, 0)) == -1
				|| shmctl(ipc_id, IPC_RMID, 0) == -1)
				oops("shmkey", ipc_key, HEX);
			break;

		case 'S':	/* semaphores (by key) */
			ipc_key = (key_t)strtol(optarg,(char **)NULL,16);
			if ((ipc_id=semget(ipc_key, 0, 0)) == -1
				|| semctl(ipc_id, 0, IPC_RMID, 0) == -1)
				oops("semkey", ipc_key, HEX);
			break;

		default:
		case '?':	/* anything else */
			err++;
			break;
		}
	if (err || (optind < argc)) {
		fprintf(stderr,
		MSGSTR(USAGE1, "usage: ipcrm [-q msqid] [-m shmid] [-s semid]\n%s\n"),
		MSGSTR(USAGE2, "	[-Q msgkey] [-M shmkey] [-S semkey]")); /*MSG*/
		exit(1);
	}
	exit (rc);  /* D35773 */
}

oops(s, i, b)
char *s;
long   i;
int	b;
{
	char str[BUFSIZ];

	rc = 1;
	if (b == DEC)
		sprintf(str, "%ld", i);
	else
		sprintf(str, "%lx", i);

	switch(errno) {
	case ENOENT:
	case EINVAL:
	case EIDRM:
		fprintf(stderr, MSGSTR(NOTFOUND, 
			"ipcrm: %s(%s): not found\n"), s, str);
		break;
	case EPERM:
		fprintf(stderr, MSGSTR(NOPERM, 
			"ipcrm: %s(%s): permission denied\n"), s, str);
		break;
	default:
		fprintf(stderr, MSGSTR(UNKNOWN,
			"ipcrm: %s(%s): %s\n"), s, str, strerror(errno));
		break;	
	}
}

/*
 * sysgrp
 *      return TRUE if (one of) our groups is the system group (0).
 *      on machines where each process has exactly one group,
 *      return !getgid();
 */
sysgrp()
{
        extern char *malloc();
        int *gidset = (int *)malloc(NGROUPS * sizeof(int));
        int ngr;

        if(gidset == NULL) {
                ngr = getgid();
                BUGLPR(ipcrmdb,BUGACT,
			("malloc failed, assuming group %d\n", ngr));
                return !ngr;
        }
        ngr = getgroups(NGROUPS, gidset);


        while (ngr-- > 0)
                if (gidset[ngr] == 0)
                        return TRUE;

	BUGLPR(ipcrmdb,BUGACT,("not in system group\n")); /*MSG*/
        return FALSE;
} /* sysgrp */
