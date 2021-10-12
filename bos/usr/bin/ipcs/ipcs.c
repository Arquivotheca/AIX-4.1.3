static char sccsid[] = "@(#)48	1.12.1.6  src/bos/usr/bin/ipcs/ipcs.c, cmdipc, bos41J, 9516A_all 4/17/95 17:24:31";
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
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
#include	<a.out.h>
#include	<fcntl.h>
#include	<time.h>
#include	<grp.h>
#include	<pwd.h>
#include	<stdio.h>
#include	<locale.h>
#include <nl_types.h>
#include "ipcs_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_IPCS,Num,Str)


#define TIME	0
#define MSGM	1
#define SEM	2
#define SHM	3
#define MSGINFO 4
#define SEMINFO 5
#define SHMINFO 6
#define MSGMARK 7
#define SHMMARK 8
#define SEMMARK 9
#define TIME_SIZE 50

struct nlist nl[] = {		/* name list entries for IPC facilities */
	{"time"},
	{"msgque"},
	{"sema"},
	{"shmem"},
	{"msginfo"},
	{"seminfo"},
	{"shminfo"},
	{"msg_mark"},
	{"shm_mark"},
	{"sem_mark"},
	{NULL},
};
char	chdr[80],
	chdr2[80],
				/* c option header format */
	*name = "/unix",	/* name list file */
	*mem = "/dev/mem",	/* memory file */
	opts[] = "abcmopqstC:N:";/* allowable options for getopt */
extern char	*optarg;	/* arg pointer for getopt */
int		bflg,		/* biggest size:
					segsz on m; qbytes on q; nsems on s */
		cflg,		/* creator's login and group names */
		mflg,		/* shared memory status */
		oflg,		/* outstanding data:
					nattch on m; cbytes, qnum on q */
		pflg,		/* process id's: lrpid, lspid on q;
					cpid, lpid on m */
		qflg,		/* message queue status */
		sflg,		/* semaphore status */
		tflg,		/* times: atime, ctime, dtime on m;
					ctime, rtime, stime on q;
					ctime, otime on s */
		err;		/* option error count */
extern int	optind; 	/* option index for getopt */

extern long		lseek();
static char *get_time(time_t * thetime);

/*
 * NAME:	ipcs - IPC status
 * FUNCTION: 	Examine and print certain things about message queues, 
 *		semaphores, and shared memory.
 */

main(argc, argv)
int	argc;	/* arg count */
char	**argv; /* arg vector */
{
	int		id;	/* message queue id */
	register int	i,	/* loop control */
			md,	/* memory file file descriptor */
			o;	/* option flag */
	time_t		time;	/* date in memory file */
	struct shmid_ds mds;	/* shared memory data structure */
	struct shminfo shminfo; /* shared memory information structure */
	struct msqid_ds qds;	/* message queue data structure */
	struct msginfo msginfo; /* message information structure */
	struct semid_ds sds;	/* semaphore data structure */
	struct seminfo seminfo; /* semaphore information structure */
	key_t local_key;	/* local key for a remote queue */
	paddr_t	msgmark;	/* High water mark for messages */
	paddr_t	shmmark;	/* High water mark for shared memory */
	paddr_t	semmark;	/* High water mark for semaphores */

	(void) setlocale (LC_ALL,"");

	catd = catopen(MF_IPCS, NL_CAT_LOCALE);

	strcpy(chdr, MSGSTR(CHDR, "T     ID     KEY        MODE       OWNER    GROUP"));	/*MSG*/
	strcpy(chdr2, MSGSTR(CHDR2, "  CREATOR   CGROUP")); /*MSG*/
	/* Go through the options and set flags. */
	while((o = getopt(argc, argv, opts)) != EOF)
		switch(o) {
		case 'a':
			bflg = cflg = oflg = pflg = tflg = 1;
			break;
		case 'b':
			bflg = 1;
			break;
		case 'c':
			cflg = 1;
			break;
		case 'C':
			mem = optarg;
			break;
		case 'm':
			mflg = 1;
			break;
		case 'N':
			name = optarg;
			break;
		case 'o':
			oflg = 1;
			break;
		case 'p':
			pflg = 1;
			break;
		case 'q':
			qflg = 1;
			break;
		case 's':
			sflg = 1;
			break;
		case 't':
			tflg = 1;
			break;
		case '?':
			err++;
			break;
		}
	if(err || (optind < argc)) {
		fprintf(stderr, MSGSTR(USAGE,
		    "usage:  ipcs [-abcmopqst] [-C corefile] [-N namelist]\n"));
		exit(1);
	}
	if((mflg + qflg + sflg) == 0)
		mflg = qflg = sflg = 1;

	/* Check out namelist and memory files. */
	nlist(name, nl);
	
	if(!nl[TIME].n_value) {
		fprintf(stderr, 
			MSGSTR(NAMELIST, "ipcs:	%s: no name list found\n"), name);
		exit(1);
	}
	if((md = open(mem, O_RDONLY)) < 0) {
		fprintf(stderr, 
			MSGSTR(MEMORY, "ipcs: %s: unable to open memory file\n"), mem);
		exit(1);
	}

	lseeke(md, (long)nl[TIME].n_value, SEEK_SET, nl[TIME].n_sclass);
	reade(md, &time, sizeof(time));
	printf(MSGSTR(STATUS,"IPC status from %s as of %s"), mem, get_time(&time));

	/* Print Message Queue status report. */
	if(qflg) {
		i = 0;
		if(nl[MSGM].n_value) {
		    lseeke(md, (long)nl[MSGINFO].n_value, SEEK_SET, 
				nl[MSGINFO].n_sclass);
		    reade(md, &msginfo, sizeof(msginfo));
		    lseeke(md, (long)nl[MSGMARK].n_value, SEEK_SET,
				nl[MSGMARK].n_sclass);
		    reade(md, &msgmark, sizeof(msgmark));
		    lseeke(md, (long)nl[MSGM].n_value, SEEK_SET, 
				nl[MSGM].n_sclass);
		    fputs(chdr, stdout);
		    if (cflg)
		        fputs(chdr2, stdout);
		    if (oflg)
		        fputs(MSGSTR(QOMSG, " CBYTES  QNUM"), stdout);
		    if (bflg)
		        fputs(MSGSTR(QBMSG, " QBYTES"), stdout);
		    if (pflg)
		        fputs(MSGSTR(QPMSG, " LSPID LRPID"), stdout);
		    if (tflg)
		        fputs(MSGSTR(QTMSG, "   STIME    RTIME    CTIME "), stdout);
		    printf("\n%s:\n",MSGSTR(QMSG,"Message Queues"));
		} else {
			msgmark = 0;
			printf(MSGSTR(MQNIS,"Message Queue facility not in system.\n"));
		}
		while(i < msgmark) {
			reade(md, &qds, sizeof(qds));
			if(!(qds.msg_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
			hp('q',"SRrw-rw-rw-",&qds.msg_perm,i++,msginfo.msgmni);
			if(oflg)
				printf("%7u%6u", qds.msg_cbytes, qds.msg_qnum);
			if(bflg)
				printf("%7u", qds.msg_qbytes);
			if(pflg)
				printf("%6u%6u", qds.msg_lspid, qds.msg_lrpid);
			if(tflg) {
				tp(qds.msg_stime);
				tp(qds.msg_rtime);
				tp(qds.msg_ctime);
			}
			printf("\n");
		}
	}

	/* Print Shared Memory status report. */
	if(mflg) {
		i = 0;
		if(nl[SHM].n_value) {
			lseeke(md, (long)nl[SHMINFO].n_value, SEEK_SET,
				nl[SHMINFO].n_sclass);
			reade(md, &shminfo, sizeof(shminfo));
			lseeke(md, (long)nl[SHMMARK].n_value, SEEK_SET,
				nl[SHMMARK].n_sclass);
			reade(md, &shmmark, sizeof(shmmark));
			lseeke(md, (long)nl[SHM].n_value, SEEK_SET,
				nl[SHM].n_sclass);
			if(oflg || bflg || tflg || !qflg || !nl[MSGM].n_value) {
                    	    fputs(chdr, stdout);
			    if (cflg)
			        fputs(chdr2, stdout);
			    if (oflg)
			        fputs(MSGSTR(MOMSG," NATTCH"),stdout);
			    if (bflg) {
			        fputs("   ", stdout);  /* allow extra width for SEGSZ */
			        fputs(MSGSTR(MBMSG,"  SEGSZ"), stdout);
			    }
			    if (pflg)
			        fputs(MSGSTR(MPMSG,"  CPID  LPID"), stdout);
			    if (tflg)
			        fputs(MSGSTR(MTMSG,"   ATIME    DTIME    CTIME "), stdout);
			    putchar('\n');
			}
			printf(MSGSTR(MMSG,"Shared Memory:\n")); /*MSG*/
		} else {
			shmmark = 0;
			printf(MSGSTR(NOSHMEM, "Shared Memory facility not in system.\n"));
		}
		while(i < shmmark) {
			reade(md, &mds, sizeof(mds));
			if(!(mds.shm_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
			hp('m',"DCrw-rw-rw-",&mds.shm_perm,i++,shminfo.shmmni);
			if(oflg)
				printf(" %6u", mds.shm_nattch);
			if(bflg)
				printf(" %9d", mds.shm_segsz);
			if(pflg)
				printf(" %5u %5u", mds.shm_cpid, mds.shm_lpid);
			if(tflg) {
				tp(mds.shm_atime);
				tp(mds.shm_dtime);
				tp(mds.shm_ctime);
			}
			printf("\n");
		}
	}

	/* Print Semaphore facility status. */
	if(sflg) {
		i = 0;
		if(nl[SEM].n_value) {
			lseeke(md, (long)nl[SEMINFO].n_value, SEEK_SET,
				nl[SEMINFO].n_sclass);
			reade(md, &seminfo, sizeof(seminfo));
			lseeke(md, (long)nl[SEMMARK].n_value, SEEK_SET,
				nl[SEMMARK].n_sclass);
			reade(md, &semmark, sizeof(semmark));
			lseeke(md, (long)nl[SEM].n_value, SEEK_SET,
				nl[SEM].n_sclass);
			if(bflg || tflg || (!qflg || !nl[MSGM].n_value) &&
			    (!mflg || !nl[SHM].n_value)) {
                    	    fputs(chdr, stdout);
			    if (cflg)
			        fputs(chdr2, stdout);
			    if (bflg)
			        fputs(MSGSTR(SBMSG," NSEMS"), stdout);
			    if (tflg)
			        fputs(MSGSTR(STMSG, "   OTIME    CTIME "), stdout);
			    putchar ('\n');
			}
			printf(MSGSTR(SMSG,"Semaphores:\n"));
		} else {
			semmark = 0;
			printf(MSGSTR(SFNIS,"Semaphore facility not in system.\n"));
		}
		while(i < semmark) {
			reade(md, &sds, sizeof(sds));
			if(!(sds.sem_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
			hp('s',"--ra-ra-ra-",&sds.sem_perm,i++,seminfo.semmni);
			if(bflg)
				printf("%6u", sds.sem_nsems);
			if(tflg) {
				tp(sds.sem_otime);
				tp(sds.sem_ctime);
			}
			printf("\n");
		}
	}
	exit(0);
}

/*
**	lseeke - lseek with error exit
*/

lseeke(f, o, w, s)
int	f,	/* fd */
	w,	/* whence */
	s;	/* storage class */
long	o;	/* offset */
{
	long	tmpaddr;	/* temporary address from TOC entry */

	if(lseek(f, o, w) < 0) {
		perror(MSGSTR(IPCSSE,"ipcs:  seek error")); /*MSG*/
		exit(1);
	}
	if (s == C_HIDEXT) {
		reade(f, &tmpaddr, sizeof(long));
		if (lseek(f, tmpaddr, 0) < 0) {
			perror(MSGSTR(IPCSSE,"ipcs:  seek error")); /*MSG*/
			exit(1);
		}
	}
}

/*
**	reade - read with error exit
*/

reade(f, b, s)
int	f,	/* fd */
	s;	/* size */
char	*b;	/* buffer address */
{
	if(read(f, b, s) != s) {
		perror(MSGSTR(IPCSRE,"ipcs:  read error")); /*MSG*/
		exit(1);
	}
}

/*
**	hp - common header print
*/

hp(type, modesp, permp, slot, mni)
char				type,	/* facility type */
				*modesp;/* ptr to mode replacement characters */
register struct ipc_perm	*permp; /* ptr to permission structure */
int				slot;	/* facility slot number */
int				mni;	/* xxxinfo.xxxmni field */
{
	register int		i,	/* loop control */
				j;	/* loop control */
	register struct group	*g;	/* ptr to group group entry */
	register struct passwd	*u;	/* ptr to user passwd entry */
	int	id;

	id = mni * permp->seq + slot;

	printf("%c%7d%#11.8x ", type, id, permp->key);
	for(i = 02000;i;modesp++, i >>= 1)
		printf("%c", (permp->mode & i) ? *modesp : '-');
	if((type == 'Q') || (u = getpwuid(permp->uid)) == NULL)
		printf("%9d", permp->uid);
	else
		printf("%9.8s", u->pw_name);
	if((type == 'Q') || (g = getgrgid(permp->gid)) == NULL)
		printf("%9d", permp->gid);
	else
		printf("%9.8s", g->gr_name);
	if(cflg) {
		if((type == 'Q')||(u = getpwuid(permp->cuid)) == NULL)
			printf("%9d", permp->cuid);
		else
			printf("%9.8s", u->pw_name);
		if((type == 'Q')||(g = getgrgid(permp->cgid)) == NULL)
			printf("%9d", permp->cgid);
		else
			printf("%9.8s", g->gr_name);
	}
}

/*
**	tp - time entry printer
*/

tp(time)
time_t	time;	/* time to be displayed */
{
	register struct tm	*t;	/* ptr to converted time */

	if(time) {
		t = localtime(&time);
		printf(" %2d:%2.2d:%2.2d", t->tm_hour, t->tm_min, t->tm_sec);
	} else
		printf(MSGSTR(NOENTRY," no-entry")); /*MSG*/
}

/*
 * NAME: get_time
 *
 * FUNCTION: This is to replace ctime() for international support language
 *
 * RETURNS: string of characters indicate the time
 */
static char *
get_time(thetime)
time_t * thetime;
{
	struct tm *tmp;
	static char buf[TIME_SIZE];
	
	if ((tmp = localtime(thetime)) == NULL)
		return((char *) NULL);
	/* strftime does the time in a NLS manner */
	strftime(buf, TIME_SIZE, "%c\n", tmp);
	return(buf);
}

