static char sccsid[] = "@(#)17 1.13.1.6 src/bos/kernel/ipc/ipc.c, sysipc, bos411, 9430C411a 7/11/94 15:01:33";
/*
 * COMPONENT_NAME: (SYSIPC) common services for SYSIPC
 *
 * FUNCTIONS: ipcaccess ipcget ipc_lock_init()
 *
 * ORIGINS: 27, 3, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */
#include <sys/errno.h>
#ifdef _POWER_MP
#include <sys/atomic_op.h>
#endif
#include <sys/user.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/priv.h>
#include <sys/syspest.h>
#include <sys/id.h>
#include "ipcspace.h"


struct msqid_ds	msgque[MSGMNI];
struct msginfo	msginfo = {
	MSGMAX,
	MSGMNB,
	MSGMNI,
	MSGMNM
};

struct semid_ds	sema[SEMMNI];
#define	SEMUSZ	(sizeof(struct sem_undo)+sizeof(struct undo)*SEMUME)

struct	seminfo seminfo = {
	SEMMNI,
	SEMMSL,
	SEMOPM,
	SEMUME,
	SEMUSZ,
	SEMVMX,
	SEMAEM
};

struct shmid_ds	shmem[SHMMNI];	
struct	shminfo shminfo = {
	SHMMAX,
	SHMMIN,
	SHMMNI,
};


/*
 * NAME: ipcaccess
 *
 * FUNCTION:
 *	Check message, semaphore, or shared memory access permissions.
 *
 * EXECUTION ENVIRONMENT:
 *	Called by IPC system calls
 *
 * NOTES:
 *	This routine verifies the requested access permission for the current
 *	process.  Super-user is always given permission.  Otherwise, the
 *	appropriate bits are checked corresponding to owner, group, or
 *	everyone.  Zero is returned on success.  On failure, u.u_error is
 *	set to EACCES and one is returned.
 *	The arguments must be set up as follows:
 *		p - Pointer to permission structure to verify
 *		mode - Desired access permissions
 *
 * DATA STRUCTURES: set u.u_error
 *
 * RETURNS: 0 if successful	1 if not successful
 */

int
ipcaccess(p, mode)
register struct ipc_perm	*p;
register ushort			mode;
{
	uid_t 	current_uid;
	gid_t	current_gid;
	current_uid = getuidx(ID_EFFECTIVE);
	current_gid = getgidx(ID_EFFECTIVE);

	ASSERT( (mode == IPC_R) || (mode == IPC_W) );
	if ( mode == IPC_R)
	{
		if ( priv_req( BYPASS_DAC_READ ) )		
			return(0);
	}
	else
	{
		if ( priv_req( BYPASS_DAC_WRITE ) )
			return(0);
	}

	if(current_uid != p->uid && current_uid != p->cuid) {
		mode >>= 3;
		if (!groupmember(p->gid) && !groupmember(p->cgid))
			mode >>= 3;
	}
	if(mode & p->mode)
		return(0);
	u.u_error = EACCES;
	return(1);
}

/*
 * NAME: ipcget
 *
 * FUNCTION: Get message, semaphore, or shared memory structure.
 *
 * EXECUTION ENVIRONMENT:
 *	called ipc system calls
 *
 * NOTES:
 *	This routine searches for a matching key based on the given flags
 *	and returns a pointer to the appropriate entry.  A structure is
 *	allocated if the key doesn't exist and the flags call for it.
 *	The arguments must be set up as follows:
 *		key - Key to be used
 *		flag - Creation flags and access modes
 *		base - Base address of appropriate facility structure array
 *		cnt - # of entries in facility structure array
 *		size - sizeof(facility structure)
 *		status - Pointer to status word: set on successful completion
 *			only:	0 => existing entry found
 *				1 => new entry created
 *		mark - high water mark of descriptor allocation
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS: NULL with u.u_error set to an appropriate value if
 *	it fails, or a pointer to the initialized entry if it succeeds.
 *
 * MP MODIFICATION:
 *      The primitive compare_and_swap() is used to atomically compare the
 *         allocation status of each permissions structure in the array with
 *         the value 0 and, if true, store the value IPC_ALLOC.
 *      The primitive fetch_and_add() is used to atomically increment and
 *         decrement the high water mark for each IPC service.
 *      The key element in the permissions structure is no longer initialised
 *         in this routine. The calling routine is in charge of doing it as
 *         its last initialisation operation.
 *   Caution:
 *   --------
 *      Since many queues may be acquired at the same time, the value of 'mark'
 *      may be changed by any other thread in the system.
 *         We can afford 'mark' being increased even if it's our loops limit,
 *         but the relation between 'base' and 'mark' outside the for loops is
 *         always hazardous.
 *      For the same reason, you can never expect the records beyond 'mark' to
 *         be still free once you've exited a for loop.
 *      
 */

struct ipc_perm *
ipcget(key, flag, base, cnt, size, status, mark)
register struct ipc_perm	*base;
int				cnt,
				flag,
				size,
				*status;
key_t				key;
int				*mark;
{
	register struct ipc_perm	*a;	/* ptr to available entry */
	register int			i;	/* loop control */
#ifdef _POWER_MP
	mode_t			oldmode,	/* permission modes */
				newmode = IPC_ALLOC | (flag & 0777);
	int			inmark = *mark;	/* mark on procedure entry */
#endif

	if(key == IPC_PRIVATE) {
#ifndef _POWER_MP
		for(i = 0;i++ < *mark;
			base = (struct ipc_perm *)(((char *)base) + size)) {
			if(base->mode & IPC_ALLOC)
				continue;
			goto init;
		}
		if ( *mark < cnt ) {
			(*mark)++;
			goto init;
		}
#else
		for(i = 0;i++ < cnt;
			base = (struct ipc_perm *)(((char *)base) + size)) {
			if(base->mode & IPC_ALLOC)
				continue;
			oldmode = 0;
			if (compare_and_swap((atomic_p) &base->mode,
				&oldmode,newmode) == FALSE)
				continue;
			if (i >= inmark)
				fetch_and_add((atomic_p) mark,1);
			goto init;
		}
#endif
		u.u_error = ENOSPC;
		return(NULL);
	} else {
		for(i = 0, a = NULL;i++ < *mark;
			base = (struct ipc_perm *)(((char *)base) + size)) {
			if(base->mode & IPC_ALLOC) {
				if(base->key == key) {
					if((flag & (IPC_CREAT | IPC_EXCL)) ==
						(IPC_CREAT | IPC_EXCL)) {
						u.u_error = EEXIST;
						return(NULL);
					}
					if((flag & 0777) & ~base->mode) {
						u.u_error = EACCES;
						return(NULL);
					}
					*status = 0;
					return(base);
				}
				continue;
			}
			if(a == NULL)
				a = base;
		}
		if(!(flag & IPC_CREAT)) {
			u.u_error = ENOENT;
			return(NULL);
		}
#ifndef _POWER_MP
		if(a == NULL) {
			if( *mark < cnt ) {
				(*mark)++;
			} else {
				u.u_error = ENOSPC;
				return(NULL);
			}
		} else {
			base = a;
		}
#else
		if(a != NULL) {
			oldmode = 0;
			if (compare_and_swap((atomic_p) &a->mode,
				&oldmode,newmode) == TRUE) {
				base = a;
				goto init;
			}
		}
		for(;i++ < cnt;
			base = (struct ipc_perm *)(((char *)base) + size)) {
			if(base->mode & IPC_ALLOC)
				continue;
			oldmode = 0;
			if (compare_and_swap((atomic_p) &base->mode,
				&oldmode,newmode) == FALSE)
				continue;
			fetch_and_add((atomic_p) mark,1);
			goto init;
		}
		u.u_error = ENOSPC;
		return(NULL);
#endif
	}
init:
	*status = 1;
#ifndef _POWER_MP
	base->mode = IPC_ALLOC | (flag & 0777);
	base->key = key;
#endif
	base->cuid = base->uid = getuidx(ID_EFFECTIVE);
	base->cgid = base->gid = getgidx(ID_EFFECTIVE);
	return(base);
}

#ifdef _POWER_MP
/*
 * NAME: ipc_lock_init
 *
 * FUNCTION:
 *	This function calls shm_lock_init(),
 *	msg_lock_init() and sem_lock_init()
 *	to initialize the locks.
 *
 * EXECUTION ENVIRONMENT:
 *	Called by main.c during system initialization through init_tbl[]
 *	if _POWER_MP is defined.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None. 
 */

void
ipc_lock_init()
{
	/* initialize the shared memory lock */
	(void) shm_lock_init();

	/* initialize semaphore locks */
	(void) sem_lock_init();

	/* initialize tmessage queue locks */
	(void) msg_lock_init();
}
#endif /* _POWER_MP */
