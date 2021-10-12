static char sccsid[] = "@(#)20	1.10  src/bos/usr/bin/dosdir/semafore.c, cmdpcdos, bos41B, 412_41B_sync 1/4/95 16:12:18";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: ipc_init ipc_cleanup get_fat fat_alloc release_fat _DFsetlock 
 *            _DFunlock fat_addr fat_isvalid validate_fat invalidate_fat 
 *            shm_init shm_create shm_alloc shm_grab shm_free sem_init 
 *            sem_create semfunc Xerror flock_lock flock_unlock
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * Inter-process communications code for implementing DOS file sharing.
 */

/* 
 *     This file sharing implementation addresses two basic issues:
 * first, a way to share file system state information, and second,
 * a method for multiple concurrent processes to access and modify
 * a file system without corrupting the file system's structure.
 *     DOS file system state information is kept in the FAT.  To share
 * this information, the FAT is kept in a shared memory segment while
 * the file system is mounted.
 *     To maintain file system integrity in a multi-user environment,
 * all DOS library primitives that involve modifying the structure of
 * a DOS file system (e.g., file creation & deletion, cluster allocation)
 * are done atomically using the mutual exclusion primitives '_DFsetlock()'
 * and '_DFunlock()'.  These primitives use semaphores to restrict
 * access to a given DOS file system while it is being modified.
 */

#include "pcdos.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>

#include <nl_types.h>
#include "dosdir_msg.h" 
#define MSGSTR(N,S) catgets(catd,MS_DOSDIR,N,S) 
extern nl_catd catd;

/* IPC "keys" for DOS semaphores and shared memory.
 */
#define	SHMEM_KEY	0xABACAB	/* shared memory key */
#define	MASTERLOCK_KEY	0xABACAB	/* master lock semaphore key */
#define	MASTERCOUNT_KEY	0xABACAC	/* master count semaphore key */
#define	FATLOCK_KEY	0xABACAD	/* fat lock semaphore key */
#define	FATCOUNT_KEY	0xABACAE	/* fat count semaphore key */
#define	LOCKLIST_KEY	0xABACAF	/* lock list semaphore key */


/* Size constants.
 */
#define DOS_NFATS	MAXDCB	/* max number of FATS allowed: 1 per device */
#define	DOS_SHMSIZE	500000	/* size of shared memory segment */


/* Semaphores.
 * The master semaphore is used to lock out critical sections
 * in the IPC code.
 * The master count semaphore is used to keep a count of
 * the number of processes currently using any of the IPC facilities.
 * The lock semaphore is actually a list of semaphores, one per FAT,
 * and is used to restrict simultaneous access to a particular FAT.
 * The count semaphore is also a list (one per FAT),
 * and is used to keep a count of the number of processes
 * currently using a particular FAT.
 */
static int masterlock_sema = -1;	/* master semaphore */
static int mastercnt_sema = -1;		/* to keep track of access count */
static int fatlock_sema = -1;		/* to control access to FATs */
static int fatcnt_sema = -1;		/* to keep track of access count */
static int locklist_sema = -1;		/* to control access to lock list */

/* semaphore op codes */
#define S_LOCK		1
#define S_UNLOCK	2
#define S_INCR		3
#define S_DECR		4
#define S_GETVAL	5
#define S_ZERO		6

/* generic semaphore operations */
#define sem_lock(s,n)	semfunc(s,n,S_LOCK)	/* lock semaphore */
#define sem_unlock(s,n)	semfunc(s,n,S_UNLOCK)	/* unlock semaphore */
#define sem_incr(s,n)	semfunc(s,n,S_INCR)	/* increment access count */
#define sem_decr(s,n)	semfunc(s,n,S_DECR)	/* decrement access count */
#define sem_getval(s,n)	semfunc(s,n,S_GETVAL)	/* get current value */
#define sem_zero(s,n)	semfunc(s,n,S_ZERO)	/* set value to zero */

/* operations on FAT-related semaphores */
#define fat_lock(fd)	sem_lock(fatlock_sema, fd)
#define fat_unlock(fd)	sem_unlock(fatlock_sema, fd)
#define fat_incr(fd)	sem_incr(fatcnt_sema, fd)
#define fat_decr(fd)	sem_decr(fatcnt_sema, fd)
#define fat_getval(fd)	sem_getval(fatcnt_sema, fd)
#define fat_zero(fd)	sem_zero(fatcnt_sema, fd)
#define fat_inuse(fd)	(fat_getval(fd) > 0)

/* operations on master semaphores */
#define master_lock()	sem_lock(masterlock_sema, 0)
#define master_unlock()	sem_unlock(masterlock_sema, 0)
#define master_incr()	sem_incr(mastercnt_sema, 0)
#define master_decr()	sem_decr(mastercnt_sema, 0)
#define master_getval()	sem_getval(mastercnt_sema, 0)
#define master_zero()	sem_zero(mastercnt_sema, 0)

/* operations on lock list semaphores */
#define FLOCK_lock()	sem_lock(locklist_sema, 0)
#define FLOCK_unlock()	sem_unlock(locklist_sema, 0)

/* Shared memory info */
static int shm_id;			/* shared memory segment descriptor */
static char *shm_start;			/* location of shared memory segment */
static int shm_size = DOS_SHMSIZE;	/* size of shared memory segment */

/* File locking info */
extern int *llast;
extern struct lockent *lockarr;
int dos_pid;

/* Debugging, error handling defs. */
#define xerror()	Xerror(__FILE__,__LINE__)
#define TPERROR(msg)	TRACE(("*** %s: %s\n",msg,sys_errlist[errno]))
#define DEV(fd)		fatlist[fd].dev
#define setdev(fd,dev)	(strncpy(DEV(fd),dev,DEVLEN), DEV(fd)[DEVLEN]='\0')
extern int errno;
extern char *sys_errlist[];


/* FAT key macros.
 * Each FAT has a unique key derived from the
 * UNIX device on which the FAT resides.
 * These keys are used to locate a particular
 * FAT within the shared memory segment.
 */
#define mkkey(dev)	ftok(dev,'D')
#define getkey(fd)	(fatlist[fd].key)
#define setkey(fd,key)	(fatlist[fd].key = (key))


/* Shared-memory FAT descriptor.
 */
typedef struct
{
    key_t key;		/* unique key for identifying FAT */
    int size;		/* how big is this FAT? */
    int valid;		/* are the contents of this FAT valid? */
    char dev[DEVLEN+1];	/* UNIX device name: for debugging */
    int offset;		/* offset of FAT into shared memory segment */
} FATD;

/* List of FAT descriptors: one per possible FAT.
 * This list is shared among all processes that want to access FATS.
 * It goes at the head of the shared memory segment.
 */
static FATD *fatlist = NULL;	/* list of shared-memory FATs */

#define	valid_desc(f)	((f)>=0&&(f)<DOS_NFATS)	/* is FAT descriptor valid? */


/* keep track of whether ipc code has been initialized */
static int inited = FALSE;



/***************************************
   IPC Routines
 ***************************************/



/*
 * IPC startup code.
 * Initialize semaphores and shared memory segment.
 * Returns 0 if successful, else -1.
 */
ipc_init()
{
    int shmrc;
    char *shm_grab();

    if (inited)
	return(0);

    if ((masterlock_sema = sem_init(MASTERLOCK_KEY, 1, "master lock", 1))<0)
	return(-1);

    /* Lock out upcoming code to avoid race conditions.
     */
    master_lock();

    /* Initialize "lock" and "count" semaphores.
     */
    if((mastercnt_sema = sem_init(MASTERCOUNT_KEY, 1, "master count", 0))<0)
	exit(-1);
    if((fatlock_sema   = sem_init(FATLOCK_KEY, DOS_NFATS, "fat lock", 1))<0)
	exit(-1);
    if((fatcnt_sema    = sem_init(FATCOUNT_KEY, DOS_NFATS, "fat count", 0))<0)
	exit(-1);

    /* Initialize shared memory segment.
     */
    shmrc = shm_init();

    if (fatlock_sema < 0||fatcnt_sema < 0||mastercnt_sema < 0||shmrc != 0)
    {
	master_unlock();
	return(-1);
    }

    /* The list of FAT descriptors is kept at the beginning
     * of the shared memory segment segment.
     */

    fatlist = (FATD *) shm_grab(DOS_NFATS * sizeof(FATD));
    lockarr = (struct lockent *) shm_grab(DOS_NLOCKS * sizeof(struct lockent));
    llast = (int *) shm_grab(sizeof (int));
    dos_pid = getpid();
    if ((locklist_sema = sem_init(LOCKLIST_KEY, 1, "lock list", 1)) < 0)
    {
	master_unlock();
	return(-1);
    }

    /* Bump the ipc access count.
     */
    master_incr();

    master_unlock();
    inited = TRUE;
    return(0);
}



/*
 * IPC shutdown code.
 * Delete all semaphores and the shared memory segment.
 * Executed only if no other processes are using the ipc facilities.
 */
ipc_cleanup()
{
    if ( ! inited)
	return;

    master_lock();

    /* See if there are other processes using the ipc facilities.
     */
    if (master_decr()  >  0)
    {
	master_unlock();
	return;
    }

    /* Delete the semaphores.
     * Don't bother checking to see if deletion
     * was successful because its not that big of a deal.
     */
    semctl(fatlock_sema, 0, IPC_RMID, 0);
    semctl(fatcnt_sema, 0, IPC_RMID, 0);
    semctl(mastercnt_sema, 0, IPC_RMID, 0);
    semctl(locklist_sema, 0, IPC_RMID, 0);

    /* Delete the shared memory segment.
     */
    shmctl(shm_id, IPC_RMID, (struct shmid_ds *)0);

    /* If we fail to delete the master semaphore,
     * we *MUST* unlock it because other processes
     * may be blocked on it.
     */
    if (semctl(masterlock_sema, 0, IPC_RMID, 0)  !=  0)
	master_unlock();
    
    inited = FALSE;
    return;
}



/***************************************
   Shared-Memory FAT Routines
 ***************************************/



/*
 * Get the shared-memory FAT for a particular DOS device.
 */
int get_fat(dev, fatsize)
char *dev;	/* name of device whose FAT we're getting */
int fatsize;	/* size of FAT */
{
    int fd;		/* FAT descriptor */
    key_t key;		/* FAT key */

    /* Lock out upcoming code to avoid race conditions.
     */
    master_lock();

    TRACE(("*** getting shared-memory FAT for '%s'\n", dev));

    /* If fat is already present in shared memory segment,
     * just return its descriptor.
     */
    key = mkkey(dev);
    for (fd=0; fd<DOS_NFATS; fd++)
	if (fat_inuse(fd)  &&  getkey(fd) == key)
	    break;
    
    /* If the FAT doesn't already reside in the shared memory
     * segment, allocate a new partition for it.
     */
    if (fd >= DOS_NFATS)
	if ((fd = fat_alloc(key,fatsize,dev))  <  0)
	    return(-1);

    fat_incr(fd);	/* bump access count */

    TRACE(("*** shmem FAT for '%s' is in partition %d; user count = %d\n",
      dev, fd, fat_getval(fd)));

    master_unlock();
    return(fd);
}



/*
 * Allocate a new shared-memory FAT partition for a DOS device.
 */
fat_alloc(key, size, dev)
key_t key;
int size;
char *dev;
{
    int fd;

    TRACE(("*** allocating NEW partition for '%s'\n", dev));

    /* Look for an unused slot in the fat list.
     */
    for (fd=0; fd<DOS_NFATS; fd++)
	if ( ! fat_inuse(fd) )
	    break;

    if (fd >= DOS_NFATS)
    {
	TRACE(("*** fat_alloc: out of slots\n"));
	doserrno = ENOMEM;
	return(-1);
    }

    if ((fatlist[fd].offset = shm_alloc(size))  <  0)
	return(-1);

    fatlist[fd].size = size;
    fat_zero(fd);
    setkey(fd, key);
    setdev(fd, dev);	/* save device name for debugging info */
    invalidate_fat(fd);	/* this is a new FAT, so its contents aren't valid */

    return(fd);
}



/*
 * Release shared-memory FAT.
 * Decrement FAT's access count and if no other
 * processes are accessing it, free up its resources.
 */
release_fat(fd)
int fd;	/* FAT descriptor */
{
    if ( ! valid_desc(fd) ||  ! fat_inuse(fd))
	return;

    TRACE(("*** releasing shmem FAT for '%s': partition=%d; user count = %d\n",
      DEV(fd), fd, fat_getval(fd)-1));

    /* Decrement access count.
     * and invalidate the partition if nobody else
     * is currently accessing it.
     */
    if (fat_decr(fd)  <=  0)
    {
	TRACE(("*** freeing shmem FAT for '%s'\n", DEV(fd)));
	shm_free(fd);
	invalidate_fat(fd);
    }
}



/*
 * Lock out a shared-memory FAT.
 * Allow only one process at a time to access
 * the FAT in order to avoid scrambling it.
 */
_DFsetlock(disk)
DCB *disk;
{
    if (disk->lock++ == 0)
    {
	TRACE(("*** locking device '%s'\n", disk->dev_name));
	fat_lock(disk->fat_desc);
    }
}



/*
 * Unlock a locked FAT.
 */
_DFunlock(disk)
DCB *disk;
{
    if (disk->lock <= 0)
	return;

    if (--disk->lock == 0)
    {
	TRACE(("*** unlocking device '%s'\n", disk->dev_name));
	fat_unlock(disk->fat_desc);
    }
}



/*
 * Get the address of the shared-memory FAT.
 */
char *fat_addr(fd)
int fd;	/* FAT descriptor */
{
    if ( ! valid_desc(fd) ||  ! fat_inuse(fd))
	xerror();

    return( shm_start + fatlist[fd].offset );
}



/*
 * Are the contents of the shared-memory FAT valid?
 */
fat_isvalid(fd)
int fd;	/* FAT descriptor */
{
    if ( ! valid_desc(fd) ||  ! fat_inuse(fd))
	xerror();

    TRACE(("*** shared-memory FAT for '%s' (ptn %d) %s valid\n",
      DEV(fd), fd, fatlist[fd].valid ? "is" : "NOT"));

    return( fatlist[fd].valid );
}



/*
 * Set the "valid" flag for a shared-memory FAT.
 * Whoever initializes the shared-memory FAT from the
 * on-disk version should invoke this routine when
 * they're though.
 */
validate_fat(fd)
int fd;	/* FAT descriptor */
{
    if ( ! valid_desc(fd) ||  ! fat_inuse(fd))
	xerror();

    fatlist[fd].valid = TRUE;
}



/*
 * Invalidate the shared-memory FAT.
 * This routine should only be called once the
 * access count for the FAT drops to zero.
 */
invalidate_fat(fd)
int fd;	/* FAT descriptor */
{
    if ( ! valid_desc(fd))
	xerror();

    fatlist[fd].valid = FALSE;
}



/***************************************
   Shared Memory Routines
 ***************************************/



/*
 * Per-process initialization of shared memory stuff.
 * Returns 0 if successful, else -1.
 */
shm_init()
{


    TRACE(("*** initializing shared memory segment\n"));

    /* Get shared memory segment.
     * If it doesn't already exist, create it.
     */
    if ((shm_id = shmget(SHMEM_KEY, shm_size, 0))  <  0)
    {
	if (errno == ENOENT)
	{
	    if ((shm_id = shm_create())  <  0)
		return(-1);
	}
	else
	{
	    TPERROR("shm_init (shmget)");
	    doserrno = errno;
	    return(-1);
	}
    }

    /* Attach shared memory segment.
     */
    shm_start = shmat(shm_id, (char *)0, 0);
    if ( (int)shm_start  ==  -1)
    {
	TPERROR("shm_init (shmat)");
	doserrno = errno;
	return(-1);
    }

    return(0);
}



/*
 * One-time shared memory creation and initialization code.
 * This stuff only runs if the shared memory segment doesn't
 * already exist.
 * Returns shared memory id if successful, else -1.
 */
shm_create()
{
    int shmid;
    int shmflags;

    TRACE(("*** creating shared memory segment\n"));

    /* Shared memory flags:
     *   IPC_CREAT|IPC_EXCL: create segment, but fail if
     *     someone else beats us to the punch.
     *   SHM_CLEAR: zero out segment.
     *   0666: give everybody read/write permissions on segment.
     */
    shmflags = IPC_CREAT | IPC_EXCL | SHM_CLEAR | 0666;

    /* Create shared memory segment.
     */
    if ((shmid = shmget(SHMEM_KEY, shm_size, shmflags))  <  0)
    {
	TPERROR("shm_create (shmget)");
	doserrno = errno;
	return(-1);
    }

    return(shmid);
}



/* Shared memory allocation defs.
 */
#define BLKSIZE		2048
#define NBLKS		(shm_size / BLKSIZE)
#define MAXBLKS		(DOS_SHMSIZE / BLKSIZE)
#define num_blks(n)	((n + BLKSIZE -1) / BLKSIZE)



/*
 * Allocate a new partition within the shared memory segment.
 * Memory is allocated in BLKSIZE increments to make memory
 * managment easier.
 * Returns offset of partition from beginning of shared memory segment.
 */
shm_alloc(size)
int size;	/* how much memory do we need? */
{
    char blkmap[MAXBLKS];	/* shared memory allocation map */
    int nblks;			/* number of blocks in partition */
    int blknum;
    int count;
    int firstblk;
    int fd, i;

    /* Generate allocation map.
     * This map contains an entry for each (allocatable) block
     * of the shared memory segment.  Each entry is marked
     * as to whether that block is currently allocated.
     */
    for (i=0; i<NBLKS; i++)	/* initialize all map entries to zero */
	blkmap[i] = 0;

    /* Mark the blocks allocated to each FAT.
     */
    for (fd=0; fd<DOS_NFATS; fd++)
    {
	if ( ! fat_inuse(fd) )
	    continue;

	blknum = fatlist[fd].offset / BLKSIZE;
	nblks = num_blks(fatlist[fd].size);

	for (i=blknum; i<blknum+nblks; i++)
	{
	    if (i >= NBLKS  ||  blkmap[i] != 0)	/* sanity checks */
		xerror();
	    blkmap[i] = 1;
	}
    }

    /* Now try to find enough contiguous blocks
     * to make a large enough partition.
     * Just do a simple first-fit search through
     * the block list.
     */
    nblks = num_blks(size);

    for (i=count=0; i<NBLKS; i++)
    {
	if (blkmap[i] != 0)	/* block already allocated: start over */
	    count = 0;
	else
	{
	    if (count == 0)	/* remember first blk in the new partition */
		firstblk = i;
	    if (++count >= nblks)	/* do we have enough? */
		break;
	}
    }

    /* Not enough memory.
     */
    if (i >= NBLKS)
    {
	doserrno = ENOMEM;
	return(-1);
    }

    return(firstblk * BLKSIZE);
}


/*
 * grab a piece of shared memory from an already allocated buffer
 */
char *
shm_grab(size)
int size;
{
    char *addr;

    addr = shm_start;
    shm_start += size;
    shm_size  -= size;
    return(addr);
}


/*
 * Free a partition within the shared memory segment.
 */
int shm_free(fd)
int fd;		/* FAT descriptor */
{
    /* This is a no-op.
     * All memory reclamation is handled in 'shm_alloc()'.
     */
}



/***************************************
   Semaphore Routines
 ***************************************/



/*
 * Per-process initialization of semaphore stuff.
 * Returns semaphore id if successful, else -1.
 */
sem_init(key, n, name, init)
int key;	/* semaphore key */
int n;		/* number of semaphores in list */
char *name;	/* semaphore name: for debugging */
int init;	/* initial value of semaphores */
{
    int sema;

    TRACE(("*** initializing %s semaphore\n", name));

    /* Get semaphore list.
     * If list doesn't already exist, create it.
     */
    if ((sema = semget((key_t) key, n, 0))  <  0)
    {
	if (errno == ENOENT)
	{
	    if ((sema = sem_create(key, n, name, init))  <  0)
		return(-1);
	}
	else
	{
	    TPERROR("sem_init (semget)");
	    doserrno = errno;
	    return(-1);
	}
    }

    return(sema);
}



/*
 * One-time semaphore creation and initialization code.
 * This stuff only runs if the semaphore doesn't
 * already exist.
 * Returns semaphore id if successful, else -1.
 */
sem_create(key, n, name, init)
int key;	/* semaphore key */
int n;		/* number of semaphores in list */
char *name;	/* semaphore name: for debugging */
int init;	/* initial value of semaphores */
{
    int sema;
    int semflags;
    ushort initial[DOS_NFATS];	/* initial semaphore values */
    int i;

    TRACE(("*** creating %s semaphore\n", name));

    /* Semaphore flags:
     *   IPC_CREAT|IPC_EXCL: create semaphores, but fail if
     *     someone else beats us to the punch.
     *   0666: give everybody read/write permissions on semaphores.
     */
    semflags = IPC_CREAT | IPC_EXCL | 0666;

    /* Create semaphore list.
     */
    if ((sema = semget((key_t) key, n, semflags))  <  0)
    {
	TPERROR("sem_create (semget)");
	doserrno = errno;
	return(-1);
    }

    /* Initialize semaphore list.
     */
    for (i=0; i<n; i++)
	initial[i] = init;

    if (semctl(sema, 0, SETALL, initial)  <  0)
    {
	TPERROR("sem_create (semctl)");
	doserrno = errno;
	return(-1);
    }

    return(sema);
}



/*
 * Generic semaphore operations.
 * Returns value of semaphore after operation.
 */
semfunc(sema, num, op)
int sema;	/* semaphore id */
int num;	/* semaphore number */
int op;		/* operation to perform */
{
    struct sembuf sop;
    int semval;

    switch (op)
    {
    case S_UNLOCK:			/* release semaphore */
	if (sem_getval(sema, num)  !=  0)	/* sema. better be locked! */
	    xerror();
	/* fall through... */
    case S_INCR:			/* increment value */
	sop.sem_num = num;
	sop.sem_op  = 1;
	sop.sem_flg = SEM_UNDO;
	if (semop(sema, &sop, (unsigned) 1)  <  0)
	    xerror();
	break;

    case S_DECR:			/* decrement value */
	if (sem_getval(sema, num)  ==  0)	/* don't decr if already 0 */
	    break;
	/* else fall through... */
    case S_LOCK:			/* lock semaphore */
	sop.sem_num = num;
	sop.sem_op  = -1;
	sop.sem_flg = SEM_UNDO;
	if (semop(sema, &sop, (unsigned)1)  <  0)
	    xerror();
	break;

    case S_GETVAL:			/* get current value */
	/* done below */
	break;

    case S_ZERO:			/* set value to zero */
	if(semctl(sema, num, SETVAL, 0)  !=  0)
	    xerror();
	break;

    default:
	xerror();
	break;
    }

    /* Return current sema value.
     */
    if((semval = semctl(sema, num, GETVAL, 0))  <  0)
	xerror();
    return(semval);
}



/***************************************
   Misc. Routines
 ***************************************/



/*
 * Abort on fatal error.
 */
Xerror(file, line)
char *file;
int line;
{
    fprintf(stderr, MSGSTR(FATALERR, "XXX FATAL ERROR: line %d, file %s: "), 
	line, file);
    perror("");
    exit(1);
}

flock_lock() { return(FLOCK_lock());}

flock_unlock() { return(FLOCK_unlock());}
