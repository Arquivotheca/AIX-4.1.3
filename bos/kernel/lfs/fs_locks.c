static char sccsid[] = "@(#)67	1.2  src/bos/kernel/lfs/fs_locks.c, syslfs, bos411, 9428A410j 9/20/93 07:22:39";
/*
 *   COMPONENT_NAME: SYSLFS
 *
 *   FUNCTIONS: fs_complex_unlock
 *		fs_lock_exit
 *		fs_lock_fork
 *		fs_lock_scin
 *		fs_lock_scout
 *		fs_lockl
 *		fs_lockx
 *		fs_read_lock
 *		fs_simple_lock
 *		fs_simple_unlock
 *		fs_unlockl
 *		fs_unlockx
 *		fs_write_lock
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef _FSDEBUG

#include <sys/user.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/sysinfo.h>
#include <sys/fs_locks.h>

/* map of structure that describes lock activity */
#define MAXLTRC	12
struct locktrace
{
	ushort 	count;
	ushort	maxlock;
	ulong	totlock;
	int	pid;
	lock_t 	curlock;
	int	syscaddr;
	int	arg1;
	int	arg2;
	int	arg3;
	struct  lockinfo {
		caddr_t	*lockaddr;
		void    *calladdr;
	}	addrs[MAXLTRC];
};


/* map of the beginning of a stack frame */
struct stkframe 
{
	struct stkframe *next;
	int		 pad;
	void            *retaddr;
};
struct stkframe *get_stkp();

struct fs_lock_data
{
	uint	fsl_count;	/* total number of times the lock has been
				 * taken, including current waiters          */
	uint	fsl_wcount;	/* number of times complex lock has been
				 * taken in write mode                       */
	uint	fsl_contention; /* number of times there was contention 
				 * for this lock                             */
	union
	{   /* this is always treated as an int in the code */
	    struct clockstat {
		ushort	fsl_readlock;	/* lock currently held in read mode  */
		ushort	fsl_writelock;	/* lock currently held in write mode */
	    } fsl_clocked;
	    uint	fsl_locked;     /* lock currently held               */
	} lockheld;
#define fsl_lock lockheld.fsl_locked
};

/* statistics on file system lock use */
struct fs_lock_data fs_lock_stat[LTCOUNT];

void fs_simple_lock(simple_lock_t lp, int type)
{
	int id, lockword;
	
	lockword = LOADWCS((int *)lp);
	id = (int)curthread->t_tid;
	if (lockword & LOCKBIT)
	{
		assert((lockword & OWNER_MASK) != id)
		fetch_and_add(&fs_lock_stat[type].fsl_contention, 1);
	}
	fetch_and_add(&fs_lock_stat[type].fsl_count, 1);
	simple_lock(lp);
	fetch_and_add(&fs_lock_stat[type].fsl_lock, 1);
}

void fs_simple_unlock(simple_lock_t lp, int type)
{
	assert(lock_mine(lp));
	fetch_and_add(&fs_lock_stat[type].fsl_lock, -1);
	simple_unlock(lp);
}

void fs_read_lock(complex_lock_t lp, int type)
{
	int id, lockword;
	
	lockword = LOADWCS((int *)lp);
	id = (int)curthread->t_tid;
	if (lockword & WANT_WRITE)
		fetch_and_add(&fs_lock_stat[type].fsl_contention, 1);
	fetch_and_add(&fs_lock_stat[type].fsl_count, 1);
	lock_read(lp);
	fetch_and_add(&fs_lock_stat[type].fsl_lock, 0x10000);
}

void fs_write_lock(complex_lock_t lp, int type)
{
	int id, lockword;
	
	lockword = LOADWCS((int *)lp);
	id = (int)curthread->t_tid;
	if (lockword & (READ_MODE|WANT_WRITE))
		fetch_and_add(&fs_lock_stat[type].fsl_contention, 1);
	fetch_and_add(&fs_lock_stat[type].fsl_count, 1);
	fetch_and_add(&fs_lock_stat[type].fsl_wcount, 1);
	lock_write(lp);
	fetch_and_add(&fs_lock_stat[type].fsl_lock, 1);
}

void fs_complex_unlock(complex_lock_t lp, int type)
{
	int lockword;

	lockword = LOADWCS(&lp->_status);
	if (lockword & READ_MODE)
		fetch_and_add(&fs_lock_stat[type].fsl_lock, -0x10000);
	else
		fetch_and_add(&fs_lock_stat[type].fsl_lock, -1);
	lock_done(lp);
}

#ifdef notdef
/* The following debug code needs to be modified for use in a
 * multi-threaded environment.
 */

/* called from kfork in child process */
void fs_lock_fork(void)
{
	struct locktrace *ltp;

	U.U_vfs = malloc(sizeof(struct locktrace));
	ltp = (struct locktrace *)U.U_vfs;
	assert(ltp);
	bzero(ltp, sizeof(struct locktrace));
	ltp->pid = U.U_procp->p_pid;
}


/* called from fs_exit */
void fs_lock_exit(void)
{
	struct locktrace *ltp;

	if (U.U_vfs == NULL)
		return;
	
	ltp = (struct locktrace *)U.U_vfs;
	if (ltp->count != 0)
	{
		printf("process exit: count = %d, trc = %x\n",
			ltp->count, ltp);
		assert(0);
	}

	free(ltp);
	U.U_vfs = NULL;
}


/* called from svc_flih before system call is called */
void fs_lock_scin(int arg1, int arg2, int arg3, void *svcaddr)
{
	struct locktrace *ltp;
	int count;
	

	if (U.U_vfs == NULL)
		return;
	
	ltp = (struct locktrace *)U.U_vfs;
	ltp->syscaddr = svcaddr;
	ltp->arg1 = arg1;
	ltp->arg2 = arg2;
	ltp->arg3 = arg3;
}


/* called from svc_flih after system call is called */
void fs_lock_scout(void)
{
	struct locktrace *ltp;
	int	fd;

	for (fd = 0;fd < U.U_maxofile; fd++)
	{
		assert(U.U_ufd[fd].count == 0);
	}

	if (U.U_vfs == NULL)
		return;
	
	ltp = (struct locktrace *)U.U_vfs;
	if (ltp->count != 0)
	{
		printf("syscall exit: count = %d, trc = %x\n",
			ltp->count, ltp);
		assert(0);
	}
}


int fs_lockx(caddr_t *lock, int flag, int type)
{
	struct locktrace *ltp;
	struct lockinfo  *lip;
	int rc, count = 0;

	
	ltp = (struct locktrace *)U.U_vfs;
	if (ltp != NULL)
	{
		ltp->totlock++;
		ltp->curlock = lock;
	}

	sysinfo.rcread++;

	switch(type)
	{
	case (FS_VERS3) : 
			rc = lockl(lock,flag);
			if (rc == LOCK_NEST)
				return(rc);
			break;

	case (FS_SIMPLE) :
			simple_lock((simple_lock_t)(lock));
			break;
	
	case (FS_COMPLEX) :
			lock_write((complex_lock_t)(lock));
			break;
	default:
		printf("Invalid lock type\n");
		assert(0);
		break;
	}

	if (ltp)
	{
		ltp->count++;
		if (ltp->count > ltp->maxlock)
			ltp->maxlock = ltp->count;

		for (lip = &ltp->addrs[0]; lip->lockaddr != NULL; lip++,count++)
			if (count == MAXLTRC)
			{
				printf("max lock count exceeded, trc = %x\n", ltp);
				assert(0);
			}

		lip->lockaddr = lock;
		lip->calladdr = get_stkp()->next->retaddr;
	}
	return(rc);
}


void fs_unlockx(caddr_t *lock, int type)
{
	struct locktrace *ltp;
	struct lockinfo  *lip;
	int count = 0;

	ltp = (struct locktrace *)U.U_vfs;
	if (ltp != NULL)
	{
		ltp->curlock = lock;

		for (lip = &ltp->addrs[0]; lip->lockaddr != lock; lip++,count++)
			if (count == MAXLTRC)
			{
				printf("unlocking unregistered lock, trc = %x\n", ltp);
				assert(0);
			}

	}

	switch(type)
	{
	case (FS_VERS3) : 
			unlockl(lock);
			break;

	case (FS_SIMPLE) :
			simple_unlock((simple_lock_t)(lock));
			break;
	
	case (FS_COMPLEX) :
			lock_done((complex_lock_t)(lock));
			break;
	default:
		assert(0);
		printf("Invalid lock type\n");
	}

	if (ltp)
	{
		ltp->count--;
		lip->lockaddr = NULL;
		lip->calladdr = NULL;
	}
}

int fs_lockl(caddr_t *lock, int flag)
{
	struct locktrace *ltp;
	struct lockinfo  *lip;
	int rc, count = 0;

	
	ltp = (struct locktrace *)U.U_vfs;
	if (ltp)
	{
		ltp->totlock++;
		ltp->curlock = lock;
	}

	sysinfo.rcread++;

	rc = lockl(lock,flag);
	if (rc == LOCK_NEST)
		return(rc);

	if (ltp)
	{
		ltp->count++;
		if (ltp->count > ltp->maxlock)
			ltp->maxlock = ltp->count;

		for (lip = &ltp->addrs[0]; lip->lockaddr != NULL; lip++,count++)
			if (count == MAXLTRC)
			{
				printf("max lock count exceeded, trc = %x\n", ltp);
				assert(0);
			}

		lip->lockaddr = lock;
		lip->calladdr = get_stkp()->next->retaddr;
	}
	return(rc);
}


void fs_unlockl(caddr_t *lock)
{
	struct locktrace *ltp;
	struct lockinfo  *lip;
	int count = 0;

	ltp = (struct locktrace *)U.U_vfs;
	if (ltp == NULL)
	{
		unlockl(lock);
	}
	else
	{
		ltp->curlock = lock;

		for (lip = &ltp->addrs[0]; lip->lockaddr != lock; lip++,count++)
			if (count == MAXLTRC)
			{
				printf("unlocking error, trc = %x\n", ltp);
				assert(0);
			}

		unlockl(lock);

		ltp->count--;
		lip->lockaddr = NULL;
		lip->calladdr = NULL;
	}
}

#endif /* notdef */
#endif /* _FSDEBUG */
