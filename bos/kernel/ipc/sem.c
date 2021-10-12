static char sccsid[] = "@(#)19	1.23.1.7  src/bos/kernel/ipc/sem.c, sysipc, bos411, 9431A411a 7/22/94 10:14:10";

/*
 * COMPONENT_NAME: (SYSIPC) IPC semaphore services
 *
 * FUNCTIONS:  semexit, semget, semop, atomic, order, semsleep, semundo
 *	semunrm sem_lock_init
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

#include <sys/user.h>
#include <sys/sem.h>
#ifndef _POWER_MP
#include <sys/lockl.h>
#endif
#include <sys/errno.h>
#include <sys/sysinfo.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/audit.h>
#include <sys/id.h>
#include "ipcspace.h"

#ifdef _POWER_MP
#include <sys/atomic_op.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif

extern struct semid_ds	sema[];		/* semaphore data structures 	*/
extern struct seminfo seminfo;		/* param information structure	*/
struct sem_undo	*semunp = NULL;		/* ptr to head of undo chain	*/
#ifndef _POWER_MP
lock_t sem_lock = LOCK_AVAIL;		/* semaphore services lock 	*/
#else
/*
 * The follwing value is a tunable value. The current value (16) might be small.
 */
#define	NB_SEM_LOCKS	16		/* Number of locks for SEMMNI sets   */

Simple_lock	lock_sem_creat;		/* Lock to create a non private set  */
Simple_lock	lock_sem_qid[NB_SEM_LOCKS];
					/* Locks to access semaphore sets    */
Simple_lock	lock_sem_undo;		/* Lock to access any undo structure */
					/*    or the chained list (semunp)   */
#endif

uint_t sem_mark = 0;			/* high water mark for descriptors */

extern time_t	time;			/* system idea of date		*/



struct semid_ds	*ipcget(),
		*semconv();
void		atomic(),
		order(),
		semundo(),
		semunrm(),
		semexit();

/*
 * NAME: sem_lock_init
 *
 * FUNCTION: This function is called in ipc_lock_init() in ipc.c.
 *           ipc_lock_init() is called during system initialization when
 *           _POWER_MP is defined.
 *
 * EXECUTION ENVIRONMENT: Process
 *
 * DATA STRUCTURES: lock_sem_creat, lock_sem_undo, lock_sem_qid[]
 *
 * RETURNS: None.
 */ 

#ifdef _POWER_MP
void
sem_lock_init()
{
	int i;

	/* initialize the semaphore locks */
	lock_alloc(&lock_sem_creat,LOCK_ALLOC_PAGED,
		SEM_LOCK_CLASS, 32767);
	simple_lock_init(&lock_sem_creat);

	lock_alloc(&lock_sem_undo,LOCK_ALLOC_PAGED,
		SEM_LOCK_CLASS, 32766);
	simple_lock_init(&lock_sem_undo);

	for (i=0;i < NB_SEM_LOCKS;i++) {
		lock_alloc(&lock_sem_qid[i],LOCK_ALLOC_PAGED,
		SEM_LOCK_CLASS, i);
		simple_lock_init(&lock_sem_qid[i]);
	}
}
#endif /* _POWER_MP */
/*
 * NAME: semaoe
 *
 * FUNCTION: Create or update adjust on exit entry.
 *
 * EXECUTION ENVIRONMENT:
 *	called by semaphore system calls
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS: 0 if successful	-1 if not successful
 *
 * MP MODIFICATION:
 *	Need the lock_sem_undo_lock to modify any undo structure or the
 *         chained list of undo structures. Means take the lock all the time.
 */

int
semaoe(val, id, num)
short	val,	/* operation value to be adjusted on exit */
	num;	/* semaphore # */
int	id;	/* semid */
{
	register struct undo		*uup,	/* ptr to entry to update */
					*uup2;	/* ptr to move entry */
	register struct sem_undo	*up,	/* ptr to process undo struct */
					*up2;	/* ptr to undo list */
	register int			i,	/* loop control */
					found;	/* matching entry found flag */

	if(val == 0)
		return(0);
	if(val > seminfo.semaem || val < -seminfo.semaem) {
		u.u_error = ERANGE;
		return(-1);
	}
#ifdef _POWER_MP
	simple_lock(&lock_sem_undo);
#endif
	if((up = (struct sem_undo *)U.U_semundo) == NULL) {
		if( (up = xmalloc( seminfo.semusz, 2, kernel_heap)) == NULL ){
#ifdef _POWER_MP
			simple_unlock(&lock_sem_undo);
#endif
			u.u_error = ENOMEM;
			return(-1);
		}

		up->un_np = NULL;
		up->un_cnt = 0;
		U.U_semundo = (void *)up;
	}
	for(uup = up->un_ent, found = i = 0;i < up->un_cnt;i++) {
		if(uup->un_id < id || (uup->un_id == id && uup->un_num < num)) {
			uup++;
			continue;
		}
		if(uup->un_id == id && uup->un_num == num)
			found = 1;
		break;
	}
	if(!found) {
		if(up->un_cnt >= seminfo.semume) {
#ifdef _POWER_MP
			simple_unlock(&lock_sem_undo);
#endif
			u.u_error = EINVAL;
			return(1);
		}
		if(up->un_cnt == 0) {
			up->un_np = semunp;
			semunp = up;
		}
		uup2 = &up->un_ent[up->un_cnt++];
		while(uup2-- > uup)
			*(uup2 + 1) = *uup2;
		uup->un_id = id;
		uup->un_num = num;
		uup->un_aoe = -val;
#ifdef _POWER_MP
		simple_unlock(&lock_sem_undo);
#endif
		return(0);
	}
	uup->un_aoe -= val;
	if(uup->un_aoe > seminfo.semaem || uup->un_aoe < -seminfo.semaem) {
		uup->un_aoe += val;
#ifdef _POWER_MP
		simple_unlock(&lock_sem_undo);
#endif
		u.u_error = ERANGE;
		return(1);
	}
	if(uup->un_aoe == 0) {
		uup2 = &up->un_ent[--(up->un_cnt)];
		while(uup++ < uup2)
			*(uup - 1) = *uup;
		if(up->un_cnt == 0) {

			/* Remove process from undo list. */
			if(semunp == up)
				semunp = up->un_np;
			else
				for(up2 = semunp;up2 != NULL;up2 = up2->un_np)
					if(up2->un_np == up) {
						up2->un_np = up->un_np;
						break;
					}
			up->un_np = NULL;
		}
	}
#ifdef _POWER_MP
	simple_unlock(&lock_sem_undo);
#endif
	return(0);
}

/*
 * NAME: semconv
 *
 * FUNCTION: Convert user supplied semid into a ptr to the associated
 *	semaphore header.
 *
 * EXECUTION ENVIRONMENT:
 *	Executed during system call
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS:
 *	pointer to semaphore header if successful
 *	NULL if not successful
 *
 * MP MODIFICATION:
 *	Upon success, hold the lock protecting the access to the semaphore set.
 */

struct semid_ds *
semconv(s)
uint_t	s;	/* semid */
{
	register struct semid_ds	*sp;	/* ptr to associated header */
	uint_t 				index;	/* descriptor index	*/
	uint_t				seq;	/* sequence number	*/

	seq = s / seminfo.semmni;
	index = s % seminfo.semmni;

	if ( index >= sem_mark ) {
		u.u_error = EINVAL;
		return( NULL );
	}

	sp = &sema[index];             	/* low 16 bits of semid=index*/

#ifdef _POWER_MP
        simple_lock(&lock_sem_qid[s % NB_SEM_LOCKS]);
#endif

	if ( (seq != sp->sem_perm.seq) ||
		!(sp->sem_perm.mode & IPC_ALLOC) ) {
#ifdef _POWER_MP
		simple_unlock(&lock_sem_qid[s % NB_SEM_LOCKS]);
#endif
		u.u_error = EINVAL;
		return(NULL);
	}
	return(sp);
}

/*
 * NAME: semctl
 *
 * FUNCTION: semctl system call
 *
 * EXECUTION ENVIRONMENT:
 *	System call
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS: 0 if successful	-1 if not successful
 *
 * MP MODIFICATION:
 *	Reset the key to IPC_PRIVATE while removing a semaphore set.
 */

semctl( int semid, int semnum, int cmd, int arg) 
{ 
	register struct	semid_ds	*sp;	/* ptr to semaphore header */
	register struct sem		*p;	/* ptr to semaphore */
	register int			i;	/* loop control */
	register int			rval;	/* return value */
	struct semid_ds			semds;	/* copy of header */
	short				*array;	/* copy of array parameter */
	short				*tp;
	int				bytes;
	int				rv;
	static int svcnumD = 0;
	static int svcnumO = 0;
	static int svcnumM = 0;
	uid_t				current_uid;
	gid_t				current_gid;

	current_uid = getuidx(ID_EFFECTIVE);
	current_gid = getgidx(ID_EFFECTIVE);
	TRCHKGT_SYSC(SEMCTL, semid, semnum, cmd, arg, NULL);
#ifndef _POWER_MP
	sysinfo.sema++;		/* update sysinfo semaphore counter */
	cpuinfo[CPUID].sema++;
#else
	fetch_and_add((atomic_p) &sysinfo.sema,1);
	fetch_and_add((atomic_p) &cpuinfo[CPUID].sema,1);
	
#endif

#ifndef _POWER_MP
	rv = lockl(&sem_lock,LOCK_SHORT);
	ASSERT( LOCK_SUCC == rv );
#endif

	/*
	 * MP mode : On successful completion,
	 *           semconv() holds the lock protecting the semaphore set.
	 */
	if((sp = semconv(semid)) == NULL)
		goto outxnr;
	rval = 0;
	switch(cmd) {

	/* Remove semaphore set. */
	case IPC_RMID:
		if(audit_flag && audit_svcstart("SEM_Delete", &svcnumD, 1, semid)){
			audit_svcfinis();
		}
		if(current_uid != sp->sem_perm.uid && current_uid != sp->sem_perm.cuid
			&& !priv_req(BYPASS_DAC_WRITE)) {
			u.u_error = EPERM;
			break;
		}
		semunrm(semid, 0, sp->sem_nsems);
		for(i = sp->sem_nsems, p = sp->sem_base;i--;p++) {
			p->semval = p->sempid = 0;
			if(p->semncnt) {
				e_wakeup(&p->semnwait);
				p->semncnt = 0;
			}
			if(p->semzcnt) {
				e_wakeup(&p->semzwait);
				p->semzcnt = 0;
			}
 		}
		
		rv = xmfree( sp->sem_base , kernel_heap );		
		ASSERT( rv == 0 );
		sp->sem_perm.seq++;
#ifdef _POWER_MP
		/*
		 * MP mode : Avoid a concurrent semget() finding a released set.
		 */
		sp->sem_perm.key = IPC_PRIVATE;
#endif
		sp->sem_perm.mode = 0;
		break;

	/* Set ownership and permissions. */
	case IPC_SET:
		if(current_uid != sp->sem_perm.uid && current_uid != sp->sem_perm.cuid
			 && !priv_req(SET_OBJ_DAC)) {
			u.u_error = EPERM;
			break;
		}
		if(copyin(arg, &semds, sizeof(semds))) {
			u.u_error = EFAULT;
			break;
		}
		if(audit_flag && audit_svcstart("SEM_Owner", &svcnumO, 3, 
		semid, semds.sem_perm.uid, semds.sem_perm.gid)){
			audit_svcfinis();
		}
		sp->sem_perm.uid = semds.sem_perm.uid;
		sp->sem_perm.gid = semds.sem_perm.gid;
		sp->sem_perm.mode = (semds.sem_perm.mode & 0777) | IPC_ALLOC;
		sp->sem_ctime = time;
		break;

	/* Get semaphore data structure. */
	case IPC_STAT:
		if(ipcaccess(&sp->sem_perm, SEM_R))
			break;
		if(audit_flag && audit_svcstart("SEM_Mode", &svcnumM, 2, 
		semid, semds.sem_perm.mode)){
			audit_svcfinis();
		}
		if(copyout(sp, arg, sizeof(*sp))) {
			u.u_error = EFAULT;
			break;
		}
		break;

	/* Get # of processes sleeping for greater semval. */
	case GETNCNT:
		if(ipcaccess(&sp->sem_perm, SEM_R))
			break;
		if(semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			break;
		}
		rval = (sp->sem_base + semnum)->semncnt;
		break;

	/* Get pid of last process to operate on semaphore. */
	case GETPID:
		if(ipcaccess(&sp->sem_perm, SEM_R))
			break;
		if(semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			break;
		}
		rval = (sp->sem_base + semnum)->sempid;
		break;

	/* Get semval of one semaphore. */
	case GETVAL:
		if(ipcaccess(&sp->sem_perm, SEM_R))
			break;
		if(semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			break;
		}
		rval = (sp->sem_base + semnum)->semval;
		break;

	/* Get all semvals in set. */
	case GETALL:
		if(ipcaccess(&sp->sem_perm, SEM_R))
			break;
		bytes = sp->sem_nsems * sizeof(short);
		if( (array = xmalloc(bytes, 2, kernel_heap)) == NULL ) {
			u.u_error = ENOMEM;
			break;
		}
		for( i = sp->sem_nsems, tp = array, p = sp->sem_base;
				 i ; p++,tp++,i-- )
			*tp = p->semval;

		if(  copyout(array, arg, bytes)  )
			u.u_error = EFAULT;
		rv = xmfree(array, kernel_heap);
		ASSERT( rv == 0 );
		break;
	/* Get # of processes sleeping for semval to become zero. */
	case GETZCNT:
		if(ipcaccess(&sp->sem_perm, SEM_R))
			break;
		if(semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			break;
		}
		rval = (sp->sem_base + semnum)->semzcnt;
		break;

	/* Set semval of one semaphore. */
	case SETVAL:
		if(ipcaccess(&sp->sem_perm, SEM_A))
			break;
		if(semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			break;
		}
		if((unsigned)arg > (unsigned)seminfo.semvmx) {
			u.u_error = ERANGE;
			break;
		}
		if((p = sp->sem_base + semnum)->semval = arg) {
			if(p->semncnt) {
				p->semncnt = 0;
				e_wakeupx(&p->semnwait, E_WKX_NO_PREEMPT);
			}
		} else
			if(p->semzcnt) {
				p->semzcnt = 0;
				e_wakeupx(&p->semzwait, E_WKX_NO_PREEMPT);
			}
		p->sempid = U.U_procp->p_pid;
		semunrm(semid, semnum, semnum);
		break;

	/* Set semvals of all semaphores in set. */
	case SETALL:
		if(ipcaccess(&sp->sem_perm, SEM_A))
			break;
		bytes = sizeof(short) * sp->sem_nsems;
		if( (array = xmalloc(bytes, 2, kernel_heap)) == NULL ){
			BUGPR(("xmalloc failed\n"));
			u.u_error = ENOMEM;
			break;
		}
		if( copyin(arg , array, bytes) ) {
			xmfree(array, kernel_heap);
			u.u_error = EFAULT;
			break;
		}
		for(i = 0;i < sp->sem_nsems;)
			if((unsigned)array[i++] > (unsigned)seminfo.semvmx) {
				xmfree(array, kernel_heap);
				u.u_error = ERANGE;
				goto outxnr_0;
			}
		semunrm(semid, 0, sp->sem_nsems);
		for(i = 0, p = sp->sem_base;i < sp->sem_nsems;
			(p++)->sempid = U.U_procp->p_pid) {
			if(p->semval = array[i++]) {
				if(p->semncnt) {
					p->semncnt = 0;
					e_wakeupx(&p->semnwait,
							E_WKX_NO_PREEMPT);
				}
			} else
				if(p->semzcnt) {
					p->semzcnt = 0;
					e_wakeupx(&p->semzwait,
							E_WKX_NO_PREEMPT);
				}
		}

		xmfree(array, kernel_heap);
		break;
	default:
		u.u_error = EINVAL;
		break;
	}

outxnr_0:

#ifdef _POWER_MP
	simple_unlock(&lock_sem_qid[semid % NB_SEM_LOCKS]);
#endif

outxnr:

#ifndef _POWER_MP
	unlockl(&sem_lock);
#endif

	return u.u_error ? -1 : rval;
}

/*
 * NAME: semexit
 *
 * FUNCTION: Cleans up semaphore resources on exit
 *
 * EXECUTION ENVIRONMENT:
 *	Called by exit to clean up on process exit.
 *
 * RETURNS: NONE
 *
 * MP MODIFICATION:
 *      There's only one thread remaining in the process. So we don't need to
 *         protect concurrent accesses to the proc's undo structure.
 *	We remove the undo structure from the chained list ASAP. Then we can
 *         work on our own without the lock_sem_undo lock.
 * 	We just need the lock_sem_undo lock to protect the removal of the undo
 *         structure from the chained list.
*/

void
semexit()
{
	register struct sem_undo	*up,	/* process undo struct ptr */
					*p;	/* undo struct ptr */
	register struct semid_ds	*sp;	/* semid being undone ptr */
	register int			i;	/* loop control */
	register long			v;	/* adjusted value */
	register struct sem		*semp;	/* semaphore ptr */

	if((up = (struct sem_undo *)U.U_semundo) == NULL)
		return;
#ifndef _POWER_MP
	i = lockl( &sem_lock, LOCK_SHORT );
	ASSERT( i == LOCK_SUCC );
#endif
	
	if(up->un_cnt == 0)
		goto cleanup;

#ifdef _POWER_MP
        simple_lock(&lock_sem_undo);
#endif
	if(semunp == up)
		semunp = up->un_np;
	else
		for(p = semunp;p != NULL;p = p->un_np)
			if(p->un_np == up) {
				p->un_np = up->un_np;
				break;
			}
#ifdef _POWER_MP
        simple_unlock(&lock_sem_undo);
#endif
	for(i = up->un_cnt;i--;) {
		/*
		 * MP mode : On successful completion,
		 *           semconv() holds the lock protecting the semaphore set.
		 */
		if((sp = semconv(up->un_ent[i].un_id)) == NULL)
			continue;
		v = (long)(semp = sp->sem_base + up->un_ent[i].un_num)->semval +
			up->un_ent[i].un_aoe;
		if(v < 0 || v > seminfo.semvmx) {
#ifdef _POWER_MP
			simple_unlock(&lock_sem_qid
				[up->un_ent[i].un_id % NB_SEM_LOCKS]);
#endif
			continue;
			}
		semp->semval = v;
		if(v == 0 && semp->semzcnt) {
			semp->semzcnt = 0;
			e_wakeupx(&semp->semzwait, E_WKX_NO_PREEMPT);
		}
		if(up->un_ent[i].un_aoe > 0 && semp->semncnt) {
			semp->semncnt = 0;
			e_wakeupx(&semp->semnwait, E_WKX_NO_PREEMPT);
		}
#ifdef _POWER_MP
		simple_unlock(&lock_sem_qid
			[up->un_ent[i].un_id % NB_SEM_LOCKS]);
#endif
	}
	up->un_cnt = 0;
cleanup:
	i = xmfree( up, kernel_heap );
	ASSERT( i == 0 );
	U.U_semundo = NULL;
#ifndef _POWER_MP
	unlockl( &sem_lock );
#endif
}

/*
 * NAME: semget
 *
 * FUNCTION: semget system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: set u.u_error
 *
 * RETURNS: 0 if successful	-1 if not successful
 *
 * MP MODIFICATION:
 *	Take the lock_sem_creat lock to create a non private semaphore set.
 *	Set the value of the key as the last initialisation operation.
*/

int
semget( key_t key, int nsems, int semflg) 
{
	register struct semid_ds *      sp;     /* semaphore header ptr */
	register int			i;	/* temp */
	int				s;	/* ipcget status return */
	struct ipc_id *			ip;
	uint_t				rval;
	int				rv;
	struct sem			*semp;	/* allocated semaphore */
	int svcrc = 0;
	static int svcnum = 0;

	if (audit_flag)
		svcrc = audit_svcstart("SEM_Create",&svcnum,0);

	TRCHKGT_SYSC(SEMGET, key, nsems, semflg, NULL, NULL);
#ifndef _POWER_MP
	sysinfo.sema++;		/* update sysinfo semaphore counter */
	cpuinfo[CPUID].sema++;
#else
	fetch_and_add((atomic_p) &sysinfo.sema,1);
	fetch_and_add((atomic_p) &cpuinfo[CPUID].sema,1);
#endif


#ifndef _POWER_MP
	rv = lockl(&sem_lock,LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
#else
	if ((key != IPC_PRIVATE) && (semflg & IPC_CREAT))
		simple_lock(&lock_sem_creat);
#endif

	if((sp = ipcget(key, semflg, sema, seminfo.semmni, sizeof(*sp), &s,
			&sem_mark)) == NULL)
		goto outx;
	if(s) {

		/* This is a new semaphore set.  Finish initialization. */
		if(nsems <= 0 || nsems > seminfo.semmsl) {
			u.u_error = EINVAL;
			sp->sem_perm.mode = 0;
			goto outx;
		}
		if( (sp->sem_base = xmalloc(sizeof(struct sem)*nsems, 2,
					 kernel_heap)) == NULL ) {
			BUGPR(("xmalloc failed\n" ));
			u.u_error = ENOMEM;
			sp->sem_perm.mode = 0;
			goto outx;
		}

		/*
		 * initialize semaphores
		 */
		bzero( sp->sem_base, sizeof(struct sem) * nsems);
		semp = sp->sem_base;
		for( i = 0 ; i < nsems ; i++ ) { 
			semp->semnwait = EVENT_NULL;
			semp->semzwait = EVENT_NULL;
			semp++;
		}

		sp->sem_nsems = nsems;
		sp->sem_ctime = time;
		sp->sem_otime = 0;
#ifdef _POWER_MP
		sp->sem_perm.key = key;
#endif

	} else
		if(nsems && sp->sem_nsems < nsems) {
			u.u_error = EINVAL;
			goto outx;
		}


					/* set semid value to sem_perm.seq */
					/* || index into sema array */ 
	rval = (sp->sem_perm.seq * seminfo.semmni) + (sp - sema);

outx:

	if(svcrc){

		if(u.u_error){

			int bval = -1;

			audit_svcbcopy(&bval, sizeof(int));
			audit_svcfinis();

		}
		else {

			audit_svcbcopy(&rval, sizeof(uint_t));
			audit_svcfinis();

		}

	}

#ifndef _POWER_MP
	unlockl(&sem_lock);
#else
	if ((key != IPC_PRIVATE) && (semflg & IPC_CREAT))
		simple_unlock(&lock_sem_creat);
#endif
	return u.u_error ? -1 : rval;
}


/*
 * NAME: semop
 *
 * FUNCTION: semop system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES:
 *	changes u.u_error
 *
 * RETURNS: 0 if successful	-1 if not successful
 */

int
semop(int semid, struct sembuf *sops, size_t nsops)
{
	register struct semid_ds	*sp;	/* ptr to associated header */
	register struct sembuf		*op;
	ushort				perm_req;
	int				i;
	int				rv;
	static int svcnum = 0;
	struct sembuf			*ksops;
	struct sembuf			sopbuf[32];
	int free, lock;

	if(audit_flag && audit_svcstart("SEM_Op", &svcnum, 1, semid)){
		audit_svcfinis();
	}

	TRCHKGT_SYSC(SEMOP, semid, sops, nsops, NULL, NULL);
#ifndef _POWER_MP
	sysinfo.sema++;		/* update sysinfo semaphore counter */
	cpuinfo[CPUID].sema++;
#else
	fetch_and_add((atomic_p) &sysinfo.sema,1);
	fetch_and_add((atomic_p) &cpuinfo[CPUID].sema,1);
#endif

	/* free must be initialized early, because of jumps to out_op
	 */
	free = 0;
	lock = 0;

	if (nsops == 0)  
 		goto out_op;
	if(nsops > seminfo.semopm) 
	{
		u.u_error = E2BIG;
		goto out_op;
	}

	/* xmallocs are expensive, so it the number of semops is small copy
	 * them onto stack.  This covers the majority of semop calls.  Use
	 * xmalloc on large values
	 */

	if (nsops <= (sizeof(sopbuf)/sizeof(sopbuf[0])))
	{
		ksops = sopbuf;
	}
	else
	{
		ksops = (struct sembuf *)xmalloc(nsops*sizeof(struct sembuf),
							 2, kernel_heap);
		if (ksops == NULL)
		{
			u.u_error = ENOMEM;
			goto out_op;
		}
		free = 1;
	}

	/* do copy in before getting the lock, to avoid page fault while
	 * holding the semaphore lock
	 */
	if ( copyin((caddr_t)sops, (caddr_t)ksops,(int)(nsops*sizeof(*sops))))
	{
		u.u_error = EFAULT;
		goto out_op;
	}

#ifndef _POWER_MP
	rv = lockl(&sem_lock,LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
	lock = 1;
#endif

	/*
	 * Check for valid semid, number of operations within limit, 
	 * valid addresses, and semnums within range of semaphore 
	 * array -- includes copy from user space to kernel space.
	 */

	/*
	 * MP mode : On successful completion,
	 *           semconv() holds the lock protecting the semaphore set.
	 */
	if((sp = semconv(semid)) == NULL)
	 	goto out_op;
#ifdef _POWER_MP
	lock = 1;
#endif

				/* # operations = 0; don't update sem_otime */
				/*    or sem_pid */ 
		
				/* Verify that sem #s are in range and   */
	                        /* permissions are granted. */
	perm_req = 0;
	for(i = 0, op = (ksops);i++ < nsops;op++) 
	{
		perm_req = perm_req | op->sem_op;
		if(op->sem_num >= sp->sem_nsems) 
		{
			u.u_error = EFBIG;
			goto out_op;
		}
	}
	if (ipcaccess(&sp->sem_perm, perm_req ? SEM_A : SEM_R))
		goto out_op;
                                /* process semaphore operations in array */ 
	if (ksops[0].sem_flg & SEM_ORDER)
		order (sp,semid,ksops,sops,nsops);   
	else
		atomic(sp,semid,ksops,nsops);   

	/* set time in semaphore if all ops o.k. */
        if (u.u_error == 0)
		sp->sem_otime = time;	
out_op:

	if (lock)
#ifndef _POWER_MP
		unlockl(&sem_lock);
#else
		simple_unlock(&lock_sem_qid[semid % NB_SEM_LOCKS]);
#endif
	if (free)
		xmfree(ksops, kernel_heap);
	return u.u_error ? -1 : 0;

}

/*
 * NAME: atomic
 *
 * FUNCTION: atomic semaphore operations
 *
 * EXECUTION ENVIRONMENT:
 *	Called once by the semop sytem call
 *
 * DATA STRUCTURES: sets u.u_error 
 *
 * RETURNS: NONE
 */
 
void
atomic(sp,semid,sops,nsops)
register struct  semid_ds *	sp;	/* ptr to semaphore data structure */
int				semid;
struct sembuf *			sops;
size_t				nsops;
{
	register struct sem	*semp;	/* ptr to semaphore */
	register struct sembuf	*op;	/* ptr to semaphore operation */
	register	int	i;

	/*
	 * Loop waiting for the operations to be satisfied atomically.
	 * Actually, do the operations and undo them if a wait is needed
	 * or an error is detected.
	 */
loopagain:
	for(i = 0, op = (sops);i < nsops;i++, op++)
	{
		semp = sp->sem_base + op->sem_num;
		if(op->sem_op > 0)
		{
			if(op->sem_op + (long)semp->semval > seminfo.semvmx ||
				(op->sem_flg & SEM_UNDO &&
				semaoe(op->sem_op, semid, op->sem_num)))
			{
				if(u.u_error == 0)
					u.u_error = ERANGE;
				if(i)
					semundo((sops), i, semid, sp);
				return ;
			}
			semp->semval += op->sem_op;
			if(semp->semncnt) {
				semp->semncnt = 0;
				e_wakeupx(&semp->semnwait, E_WKX_NO_PREEMPT);
			}
			continue;
		}
		if(op->sem_op < 0)
		{	
			if(semp->semval >= -op->sem_op) 
			{
				if(-op->sem_op > seminfo.semvmx ||
					(op->sem_flg & SEM_UNDO &&
				    	semaoe(op->sem_op, semid, op->sem_num)))
				{
					if (u.u_error == 0)
 						u.u_error = ERANGE;
					if(i)
						semundo((sops), i, semid, sp);
					return ;
				}
				semp->semval += op->sem_op;
				if(semp->semval == 0 && semp->semzcnt)
 				{
					semp->semzcnt = 0;
					e_wakeupx(&semp->semzwait,
							E_WKX_NO_PREEMPT);
				}
				continue;
			}
			if(i)
				semundo((sops), i, semid, sp);
			if(op->sem_flg & IPC_NOWAIT)
		 	{
				u.u_error = EAGAIN;
				return ;
			}
			if( u.u_error = semsleep(&semp->semncnt,
					 &semp->semnwait, sp) )
				return;
			else goto loopagain;
		}
		if(semp->semval) 
		{	if(i)
				semundo((sops), i, semid, sp);
			if(op->sem_flg & IPC_NOWAIT) 
			{	u.u_error = EAGAIN;
				return ;
			}
			if( u.u_error = semsleep(&semp->semzcnt,
					 &semp->semzwait, sp) )
				return;
			else goto loopagain;
		}
	}

	/*
	 * All operations succeeded.  Update sempid for accessed semaphores.
	 */
	for(i = 0, op = (sops);i++ < nsops;
		(sp->sem_base + (op++)->sem_num)->sempid = U.U_procp->p_pid);

}

/*
 * NAME: order
 *
 * FUNCTION: does semaphore operations in order
 *
 * EXECUTION ENVIRONMENT:
 *	Called by semops system call
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS: NONE
 */

void
order(sp,semid,sops,usops,nsops)
register struct semid_ds *	sp;	/* ptr to semaphore header	*/
int				semid;
struct sembuf *			sops;
struct sembuf *			usops;	/* user copy of semops		*/
size_t				nsops;
{
	register struct sem	*semp;		/* ptr to semaphore */
	register struct sembuf	*op;		/* ptr to semaphore operation*/
	register    	int	i;
			/* LOOP THROUGH SEMAPHORE OPERATIONS DOING THEM IN */
			/* ORDER UNTIL DONE OR AN ERROR CAUSES RETURN */
	for(i=0, op = (sops); i < nsops; i++, op++)
	{	semp = sp->sem_base + op->sem_num;
		op->sem_flg &= ~SEM_ERR;	/* reset SEM_ERR bit */
		if (op->sem_op == 0)	/* BEGIN ZERO SEM_OP */
		{	if (semp->semval != 0)
				if (op->sem_flg & IPC_NOWAIT)
				{	u.u_error = EAGAIN;
					goto out_order;
				}
				else do   
				 	if (u.u_error = semsleep(
						&semp->semzcnt,
						&semp->semzwait, sp))
						goto out_order;
				while (semp->semval != 0);
			semp->sempid = U.U_procp->p_pid;
		}  				/* END OF ZERO SEM_OP */	
		else if (op->sem_op > 0)	/* BEGIN POS SEM_OP */
		{	if (op->sem_op + (long)semp->semval > seminfo.semvmx)
			{	u.u_error = ERANGE;
				goto out_order;
			}
			if (op->sem_flg & SEM_UNDO)
				if(semaoe(op->sem_op, semid, op->sem_num))
					goto out_order;
			semp->semval += op->sem_op;
			if(semp->semncnt)
			{	semp->semncnt = 0;
				e_wakeupx(&semp->semnwait, E_WKX_NO_PREEMPT);
			}
			semp->sempid = U.U_procp->p_pid;
		}				/* END OF POS SEM_OP */
		else				/* BEGIN NEG SEM_OP */
		{	if (-op->sem_op > seminfo.semvmx)
			{	u.u_error = ERANGE;
				goto out_order;
			}
			if (semp->semval < -op->sem_op)
			{	if (op->sem_flg & IPC_NOWAIT)
				{	u.u_error = EAGAIN;
					goto out_order;
				}
				else do
					if(u.u_error = semsleep(
						&semp->semncnt, 
						&semp->semnwait,sp))
						goto out_order;
				while (semp->semval < -op->sem_op);
			}
			if (op->sem_flg & SEM_UNDO)
				if(semaoe(op->sem_op, semid, op->sem_num))
					goto out_order;
			semp->semval += op->sem_op;
			if (semp->semval == 0 && semp->semzcnt)
			{	semp->semzcnt = 0;
				e_wakeupx(&semp->semzwait, E_WKX_NO_PREEMPT);
			}
			semp->sempid = U.U_procp->p_pid;
		}				/* END OF NEG SEM_OP */
	}
	return;

out_order:
	(sops+i)->sem_flg |= SEM_ERR;
	if ( copyout(&( (sops+i)->sem_flg ), &( (usops+i)->sem_flg),
			 sizeof(usops->sem_flg)) )
		u.u_error = EFAULT;

}

/*
 * NAME: semsleep
 *
 * FUNCTION: sleep on semaphore event
 *
 * RETURNS: 
 *	0 	-	if successful
 *	EINTR	-	if signal received
 *	EIDRM	-	if sem id was removed while sleeping 
 *
 * MP MODIFICATION:
 *	Replace the call to e_sleepl by a call to e_sleep_thread, with the
 *         appropriate lock. Modify the test of the return code, too.
 */

int
semsleep(count, wait_list, sp)
ushort			*count;		/* number of procs on list	*/
int			*wait_list;	/* wait list to be added to 	*/
struct semid_ds 	*sp;		/* pointer to semaphore header	*/
{
	int	sleeprtn;		/* return value from e_sleep	*/
	int	seq;			/* sequence number when released */
#ifdef _POWER_MP
	int	semid;
#endif

	(*count)++;

	seq = sp->sem_perm.seq;
#ifndef _POWER_MP
	sleeprtn = e_sleepl(&sem_lock, wait_list, EVENT_SIGRET);
	if (sleeprtn == EVENT_SIG) {
#else
	semid = (seq * seminfo.semmni) + (sp - sema);
	sleeprtn = e_sleep_thread(wait_list,
			&lock_sem_qid[semid % NB_SEM_LOCKS],
			LOCK_SIMPLE|INTERRUPTIBLE);
	if (sleeprtn == THREAD_INTERRUPTED) {
#endif

		if ( (sp->sem_perm.mode & IPC_ALLOC) &&
					(sp->sem_perm.seq == seq) ) {
			if ( *count <= 1 ) {
				*count = 0;
				e_wakeupx( wait_list, E_WKX_NO_PREEMPT );
			} else {
				(*count)--;
			}
		}
		return(EINTR);

	} else {

		if (  ((sp->sem_perm.mode & IPC_ALLOC) == 0) ||
				(sp->sem_perm.seq != seq)  )
			return (EIDRM);
		return (0);

	}
}

/*
 * NAME: semundo
 *
 * FUNCTION: Undo work done up to finding an operation that can't be done.
 *
 * EXECUTION ENVIRONMENT:
 *	Called during semops system call
 *
 * RETURNS: NONE
*/

void
semundo(op, n, id, sp)
register struct sembuf		*op;	/* first operation that was done ptr */
register int			n,	/* # of operations that were done */
				id;	/* semaphore id */
register struct semid_ds	*sp;	/* semaphore data structure ptr */
{
	register struct sem	*semp;	/* semaphore ptr */

	for(op += n - 1;n--;op--) {
		if(op->sem_op == 0)
			continue;
		semp = sp->sem_base + op->sem_num;
		semp->semval -= op->sem_op;
		if(op->sem_flg & SEM_UNDO)
			semaoe(-op->sem_op, id, op->sem_num);
	}
}

/*
 * NAME: semunrm
 *
 * FUNCTION: Undo entry remover.
 *
 * EXECUTION ENVIRONMENT:
 *	Called by semctl system call
 *
 * NOTES:
 *	This routine is called to clear all undo entries for a set of
 *	 semaphores that are being removed from the system or are being
 *	reset by SETVAL or SETVALS commands to semctl.
 *
 * RETURNS: NONE
 *
 * MP MODIFICATION:
 *	Need the lock_sem_undo_lock to modify any undo structure or the
 *         chained list of undo structures. Means take the lock all the time.
 */

void
semunrm(id, low, high)
int	id;	/* semid */
ushort	low,	/* lowest semaphore being changed */
	high;	/* highest semaphore being changed */
{
	register struct sem_undo	*pp,	/* ptr to predecessor to p */
					*p;	/* ptr to current entry */
	register struct undo		*up;	/* ptr to undo entry */
	register int			i,	/* loop control */
					j;	/* loop control */

	pp = NULL;
#ifdef _POWER_MP
	simple_lock(&lock_sem_undo);
#endif
	p = semunp;
	while(p != NULL) {

		/* Search through current structure for matching entries. */
		for(up = p->un_ent, i = 0;i < p->un_cnt;) {
			if(id < up->un_id)
				break;
			if(id > up->un_id || low > up->un_num) {
				up++;
				i++;
				continue;
			}
			if(high < up->un_num)
				break;
			for(j = i;++j < p->un_cnt;
				p->un_ent[j - 1] = p->un_ent[j]);
			p->un_cnt--;
		}

		/* Reset pointers for next round. */
		if(p->un_cnt == 0)

			/* Remove from linked list. */
			if(pp == NULL) {
				semunp = p->un_np;
				p->un_np = NULL;
				p = semunp;
			} else {
				pp->un_np = p->un_np;
				p->un_np = NULL;
				p = pp->un_np;
			}
		else {
			pp = p;
			p = p->un_np;
		}
	}
#ifdef _POWER_MP
	simple_unlock(&lock_sem_undo);
#endif
}
