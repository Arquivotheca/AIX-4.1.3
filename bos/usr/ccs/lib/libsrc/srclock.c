static char sccsid[] = "@(#)03	1.7  src/bos/usr/ccs/lib/libsrc/srclock.c, libsrc, bos411, 9428A410j 9/16/93 14:09:46";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	active_srcmstr,lock_srcmstr,unlock_srcmstr,is_active_srcmstr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/un.h>
#include <signal.h>


#define ACTIVE_SRCMSTR "/usr/sbin/srcmstr"
int active_srcmstr()
{
	struct sigaction N_action, O_action;

	/* active srcmstr and we are done */
	if(is_active_srcmstr())
		return(TRUE);

	/* sleep 10 and try again */
	bzero(&N_action,sizeof(N_action));
	N_action.sa_handler=SIG_DFL;
	sigaction(SIGALRM,&N_action,&O_action);
	sleep(10);
	sigaction(SIGALRM,&O_action,(struct sigaction *)0);

	/* srcmstr is active? */
	return(is_active_srcmstr());
}

int is_active_srcmstr()
{
	int SemID;
	key_t Key;

	Key = ftok(ACTIVE_SRCMSTR, 'b');
	SemID = semget(Key, 1, IPC_EXCL);
	if(SemID == -1 || semctl(SemID,0,GETVAL,0) != 2)
		return(FALSE);

	/* srcmstr is active */
	return(TRUE);
}

int lock_srcmstr(int new_lock)
{
	key_t Key;
	static int SemID;
	static int old_SemID;
	static struct sembuf sembuf[2];
	char sun_path[sizeof(struct sockaddr_un)];

	if(new_lock)
	{
		semop(SemID,&sembuf[1],(unsigned int) 1);
		return(0);
	}

	sembuf[0].sem_num=0;
	sembuf[0].sem_op=0;
	sembuf[0].sem_flg=IPC_NOWAIT;
	sembuf[1].sem_num=0;
	sembuf[1].sem_op=1;
	sembuf[1].sem_flg=SEM_UNDO;

	Key = ftok(ACTIVE_SRCMSTR, 'a');
	old_SemID = semget(Key, 1, IPC_EXCL|IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	Key = ftok(ACTIVE_SRCMSTR, 'b');
	SemID = semget(Key, 1, IPC_EXCL|IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (SemID == -1) 
	{
		SemID = semget(Key, 1, IPC_EXCL);
		if (SemID == -1 || (semop(SemID,sembuf,(unsigned int) 2) == -1))
			return(-1);
	}
	else if(semop(SemID,sembuf,(unsigned int) 2)==-1)
		return(-1);

	/* remove the old socket file */
	src_get_sun_path(sun_path,0);
	unlink(sun_path);
	/* set the lock file to close on exec */
	if(old_SemID == -1)
		return(0);
	semctl(old_SemID,0,IPC_RMID,0);
	return(0);
}

void unlock_srcmstr()
{
	key_t Key;
	int SemID;

	Key = ftok(ACTIVE_SRCMSTR, 'b');
	SemID = semget(Key, 1, 0);
	if (SemID == -1)
		return;
	semctl(SemID,0,IPC_RMID,0);
}
