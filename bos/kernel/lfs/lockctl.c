static char sccsid[] = "@(#)34	1.8  src/bos/kernel/lfs/lockctl.c, syslfs, bos41J, 9507A 2/2/95 13:24:17";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: flckinit(), insflck(), delflck(), regflck(),
 *            flckadj(), blocked(),
 *            common_reclock(), convoff(), deadflck(),
 *            lock_wakeup (), flock_grow()
 *
 * ORIGINS: 3, 27
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

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*#ident	"@(#)kern-3b2:os/flock.c	1.6"*/
/* (IH)flock.c	1.1.1.4 */

/*static char sccsid[] = "@(#)aix_lckctl.c	1.4 88/09/27 09:26:10";*/

/*
 *	Notes :
 *
 *		This file contains all of the file/record locking
 *		specific routines.
 *
 *		All record lock lists (referenced by a pointer in
 *		the gnode) are ordered by starting position relative
 *		to the beginning of the file.
 *
 *		In this file the name "l_end" is a macro and is used
 *		in place of "l_len" because the end, not length, of
 *		the record lock is stored internally.
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sleep.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/vnode.h"
#include "sys/fs_locks.h"
#include "sys/lockname.h"
#include "sys/lock_alloc.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/flock.h"
#include "sys/var.h"
#include "sys/utsname.h"
#include "sys/syspest.h"
#include "sys/vmount.h"

/* region types */
#define	S_BEFORE	010
#define	S_START		020
#define	S_MIDDLE	030
#define	S_END		040
#define	S_AFTER		050
#define	E_BEFORE	001
#define	E_START		002
#define	E_MIDDLE	003
#define	E_END		004
#define	E_AFTER		005

struct	flckinfo flckinfo;		/* configuration and acct info	*/
struct	filock	*frlock;		/* pointer to record lock free list */
struct	filock	*sleeplcks;		/* head of chain of sleeping locks */
Simple_lock      filock_lock;		/* file lock table lock */

#define	LOCK_WAKEUP(ptr, retry_func)  	lock_wakeup(ptr, retry_func)

extern int nflox_min, nflox_max;

BUGVDEF(lckbug, 0x0)

/* build file lock free list
 */
flckinit()
{
	register i;

	v.v_lock = nflox_min;
	v.ve_lock = (caddr_t) &flox[nflox_min];

	/* Initialize the file lock table global lock */
	lock_alloc(&filock_lock,LOCK_ALLOC_PAGED,FILOCK_LOCK_CLASS,-1);
	simple_lock_init(&filock_lock);

	flckinfo.recs = v.v_lock;

	for (i=0; i<flckinfo.recs; i++) {
		if (frlock == NULL) {
			flox[i].next = flox[i].prev = NULL;
			frlock = &flox[i];
		} else {
			flox[i].next = frlock;
			flox[i].prev = NULL;
			frlock = (frlock->prev = &flox[i]);
		}
	}
}

/* Insert lock (lckdat) after given lock (fl); If fl is NULL place the
 * new lock at the beginning of the list and update the head ptr to
 * list which is stored at the address given by lck_list. 
 */
struct filock *
insflck(lck_list, lckdat, fl)
struct	filock	**lck_list;
struct	filock	*fl;
struct	eflock	*lckdat;
{
	register struct filock *new;
	struct filock *sf;


	BUGLPR(	lckbug, BUGACT,
		("insflck called:lck_list=%x, lckdat=%x, fl=%x\n",
		lck_list, lckdat, fl));

	/* 
	 * The NFS lock deamon may repeat a request for a lock already
	 * on the sleep locks queue.  In this case, return the current
	 * entry.
	 */
	if (lckdat->l_vfs == MNT_NFS && lck_list == &sleeplcks) {
		for(sf = sleeplcks; sf; sf = sf->next) {
			if (	sf->set.l_start	== lckdat->l_start
			    &&	sf->set.l_len	== lckdat->l_len
			    &&	sf->set.l_pid	== lckdat->l_pid
			    &&	sf->set.l_sysid	== lckdat->l_sysid
			    &&  sf->set.l_vfs 	== lckdat->l_vfs) {
				return(sf);
			}
		}
	}

	if( frlock == NULL )		/* any file locks available */
		flock_grow();		/* try to grow more */

	if ((new = frlock) != NULL) {
		++flckinfo.reccnt;
		++flckinfo.rectot;
		frlock = new->next;
		if (frlock != NULL)
			frlock->prev = NULL;
		new->set = *lckdat;
		new->state = LCK_UNBLOCK;
		new->event = EVENT_NULL;
		if (fl == NULL) {
			new->next = *lck_list;
			if (new->next != NULL)
				new->next->prev = new;
			*lck_list = new;
		} else {
			new->next = fl->next;
			if (fl->next != NULL)
				fl->next->prev = new;
			fl->next = new;
		}
		new->prev = fl;
	} else {
		++flckinfo.recovf;
	}

	BUGLPR(lckbug, BUGACT, ("exiting insflock new=%x\n", new));

	return (new);
}

/* Delete lock (fl) from the record lock list. If fl is the first lock
 * in the list, remove it and update the head ptr to the list which is
 * stored at the address given by lck_list.
 */
delflck(lck_list, fl)
struct filock  **lck_list;
struct filock  *fl;
{


	BUGLPR(lckbug, BUGACT,
		("enter delflck: lck_list=%x, fl=%x\n", lck_list, fl));

	if (fl < &flox[0] || fl >= &flox[v.v_lock])
		panic("delflck:fl out of range");
	if (fl->prev != NULL)
		fl->prev->next = fl->next;
	else
		*lck_list = fl->next;
	if (fl->next != NULL)
		fl->next->prev = fl->prev;

	--flckinfo.reccnt;
	if (frlock == NULL) {
		fl->next = fl->prev = NULL;
		frlock = fl;
	} else {
		fl->next = frlock;
		fl->prev = NULL;
		frlock = (frlock->prev = fl);
	}
}

/* regflck sets the type of span of this (un)lock relative to the specified
 * already existing locked section.
 * There are five regions:
 *
 *  S_BEFORE        S_START         S_MIDDLE         S_END          S_AFTER
 *     010            020             030             040             050
 *  E_BEFORE        E_START         E_MIDDLE         E_END          E_AFTER
 *      01             02              03              04              05
 * 			|-------------------------------|
 *
 * relative to the already locked section.  The type is two octal digits,
 * the 8's digit is the start type and the 1's digit is the end type.
 */
int
regflck(ld, flp)
struct eflock *ld;
struct filock *flp;
{
	register int regntype;

	if (ld->l_start > flp->set.l_start) {
		if ((ld->l_start-1) == flp->set.l_end)
			return(S_END|E_AFTER);
		if (ld->l_start > flp->set.l_end)
			return(S_AFTER|E_AFTER);
		regntype = S_MIDDLE;
	} else if (ld->l_start == flp->set.l_start)
		regntype = S_START;
	else
		regntype = S_BEFORE;

	if (ld->l_end < flp->set.l_end) {
		if (ld->l_end == (flp->set.l_start-1))
			regntype |= E_START;
		else if (ld->l_end < flp->set.l_start)
			regntype |= E_BEFORE;
		else
			regntype |= E_MIDDLE;
	} else if (ld->l_end == flp->set.l_end)
		regntype |= E_END;
	else
		regntype |= E_AFTER;

	return (regntype);
}

/* Adjust file lock from region specified by 'ld', in the record
 * lock list indicated by the head ptr stored at the address given
 * by lck_list. Start updates at the lock given by 'insrtp'. It is 
 * assumed the list is ordered on starting position, relative to 
 * the beginning of the file, and no updating is required on any
 * locks in the list previous to the one pointed to by insrtp.
 * Insrtp is a result from the routine blocked().  Flckadj() scans
 * the list looking for locks owned by the process requesting the
 * new (un)lock :
 *
 * 	- If the new record (un)lock overlays an existing lock of
 * 	  a different type, the region overlaid is released.
 *
 * 	- If the new record (un)lock overlays or adjoins an exist-
 * 	  ing lock of the same type, the existing lock is deleted
 * 	  and its region is coalesced into the new (un)lock.
 *
 * When the list is sufficiently scanned and the new lock is not 
 * an unlock, the new lock is inserted into the appropriate
 * position in the list.
 */
flckadj(lck_list, insrtp, ld, retry_id)
struct filock	**lck_list;
register struct filock *insrtp;
struct eflock	*ld;
ulong           *retry_id;
{
	register struct	filock	*flp, *nflp; 
	int regtyp;
	int ret_val = 0;
	int (* retry_func)() = NULL;

	nflp = (insrtp == NULL) ? *lck_list : insrtp;

	BUGLPR(lckbug, BUGACT,
		("called flckadj: lck_list=%x, insrtp=%x, ld=%x\n",
		lck_list, insrtp, ld));

	while (flp = nflp) {
		nflp = flp->next;
		if (	flp->set.l_pid	 == ld->l_pid
		    &&	flp->set.l_sysid == ld->l_sysid
		    &&	flp->set.l_vfs	 == ld->l_vfs)
		{

			/* release already locked region if necessary */

			switch (regtyp = regflck(ld, flp)) {
			case S_BEFORE|E_BEFORE:
				nflp = NULL;
				break;
			case S_BEFORE|E_START:
				if (ld->l_type == flp->set.l_type) {
					ld->l_end = flp->set.l_end;
					LOCK_WAKEUP(flp, &retry_func);
					delflck(lck_list, flp);
				}
				nflp = NULL;
				break;
			case S_START|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type) {
					return(0);
				}
			case S_START|E_AFTER:
				insrtp = flp->prev;
				LOCK_WAKEUP(flp, &retry_func);
				delflck(lck_list, flp);
				break;
			case S_BEFORE|E_END:
				if (ld->l_type == flp->set.l_type)
					nflp = NULL;
			case S_BEFORE|E_AFTER:
				LOCK_WAKEUP(flp, &retry_func);
				delflck(lck_list, flp);
				break;
			case S_START|E_MIDDLE:
				insrtp = flp->prev;
			case S_MIDDLE|E_MIDDLE:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type) {
					return(0);
				}
			case S_BEFORE|E_MIDDLE:
				if (ld->l_type == flp->set.l_type)
					ld->l_end = flp->set.l_end;
				else {
					/* setup piece after end of (un)lock */
					register struct	filock *tdi, *tdp;
					struct eflock td;

					td = flp->set;
					td.l_start = ld->l_end + 1;
					tdp = tdi = flp;
					do {
						if (tdp->set.l_start<td.l_start)
							tdi = tdp;
						else
							break;
					} while (tdp = tdp->next);
					if (insflck(lck_list,&td,tdi) == NULL)
						return(ENOLCK);
				}
				LOCK_WAKEUP(flp, &retry_func);
				if (regtyp == (S_MIDDLE|E_MIDDLE)) {
					/* setup piece before (un)lock */
					flp->set.l_end = ld->l_start - 1;
					insrtp = flp;
				} else
					delflck(lck_list, flp);
				nflp = NULL;
				break;
			case S_MIDDLE|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type) {
					return(0);
				}
				flp->set.l_end = ld->l_start - 1;
				LOCK_WAKEUP(flp, &retry_func);
				insrtp = flp;
				break;
			case S_MIDDLE|E_AFTER:
				LOCK_WAKEUP(flp, &retry_func);
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(lck_list, flp);
				} else {
					flp->set.l_end = ld->l_start - 1;
					insrtp = flp;
				}
				break;
			case S_END|E_AFTER:
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					LOCK_WAKEUP(flp, &retry_func);
					delflck(lck_list, flp);
				}
				break;
			case S_AFTER|E_AFTER:
				insrtp = flp;
				break;
			}
		}
	}

	if (ld->l_type != F_UNLCK) {
		if (flp = insrtp) {
			do {
				if (flp->set.l_start < ld->l_start)
					insrtp = flp;
				else
					break;
			} while (flp = flp->next);
		}
		if (insflck(lck_list, ld, insrtp) == NULL) {
			ret_val = ENOLCK;
		}
	}
	/* 
	 * if a retry funcion was returned from LOCK_WAKEUP, then
	 * an external locking agent exists (ie NFS) and its retry
	 * function should be called to iform it to do the equivelent
	 * of a wakeup for the remote waiting locks. The retry_id is
	 * returned back to the caller.  It is used to signal the
	 * caller of the value that the retry function returned.
	 * It is used to rpc.lockd currently, and is ignored for 
	 * local locking.
	 */
	if (retry_func) {
		ulong rc;
		rc = (* retry_func)(NULL, ld);
		if (retry_id && rc)
			*retry_id = rc;
	}

	BUGLPR(lckbug, BUGACT, ("flckadj returning %d\n", ret_val));
	return (ret_val);
}

/* blocked checks whether a new lock (lckdat) would be
 * blocked by a previously set lock owned by another process.
 * Insrt is set to point to the lock where lock list updating
 * should begin to place the new lock.
 */
struct filock *
blocked(flp, lckdat, insrt)
struct filock *flp;
struct eflock  *lckdat;
struct filock **insrt;
{
	register struct filock *f;

	*insrt = NULL;
	for (f = flp; f != NULL; f = f->next) {
		if (f->set.l_start < lckdat->l_start)
			*insrt = f;
		else
			break;
		if (   (f->set.l_pid	== lckdat->l_pid)
		    && (f->set.l_sysid	== lckdat->l_sysid)
		    && (f->set.l_vfs	== lckdat->l_vfs))
		{
			if ((lckdat->l_start-1) <= f->set.l_end)
				break;
		} 
		else if ((lckdat->l_start <= f->set.l_end) && 
			(f->set.l_type == F_WRLCK || 
			(f->set.l_type == F_RDLCK && 
			lckdat->l_type == F_WRLCK)))
				return(f);
	}

	for (; f != NULL; f = f->next) {
		if (lckdat->l_end < f->set.l_start)
			break;
		if (lckdat->l_start <= f->set.l_end
		    && (   f->set.l_pid   != lckdat->l_pid
		        || f->set.l_sysid != lckdat->l_sysid
			|| f->set.l_vfs	  != lckdat->l_vfs)
		    && (f->set.l_type == F_WRLCK
		        || (f->set.l_type == F_RDLCK
		            && lckdat->l_type == F_WRLCK)))
			return(f);
	}

	return(NULL);
}

/*
 * NAME:	common_reclock(gp,
 *				size,
 *				offset,
 *				lckdat,
 *				cmd,
 *				retry_fcn,
 *				retry_id,
 *				lock_fcn,
 *				rele_fcn)
 *
 * FUNCTION:	The common_reclock routine gets and checks locks.
 *
 * PARMETERS:	gp is a struct gnode pointer for the object to be locked,
 *		size is the offset of the end of the object in bytes,
 *		offset is a ulong indicating where the lock should start,
 *		lckdat is a pointer to a flock structure with the locking
 *		details, cmd is an integer with the command for locking,
 *		retry_fcn, if non-NULL, is a pointer to a function that gets
 *		called for a sleep locks list entry when the corresponding
 *		blocking lock is released (for NFS and DS server use),
 *		retry_id is a pointer to a "ulong" used to return
 *		the identifier that will be passed as an arguement
 *		to the retry_fcn.  This return value can be used by the
 * 		caller to correlate this VNOP_LOCKCTL() call with
 *		a later (* retry_fcn)() call.  lock_fcn and rele_fcn are
 *		functions to lock and release the structure containing the
 *		gnode
 *		
 *
 * RETURN VALUE:	Zero is returned if fsync completes sucessfully.
 *			An error (errno) is returned if the routine failed.
 */
common_reclock(
struct gnode *	gp,
offset_t	size,
offset_t	offset,
struct eflock *	lckdat,
int		cmd,
int (*		retry_fcn)(),
ulong *		retry_id,
int (*		lock_fcn)(),
int (*		rele_fcn)())
{
	register struct filock  **lock_list, *sf;
	struct	filock *found, *insrt = NULL;
	int retval = 0;
	int contflg = 0;
	short whence;
	int klock;	                /* save kernel_lock state       */

	BUGLPR(lckbug, BUGACT,
		("comm_reclock called: gp=0x%x, offset=%d, lckdat=0x%x, cmd=0x%x, retry_fcn=0x%x",
		gp, offset, lckdat, cmd, retry_fcn));

	/* Convert start to be relative to beginning of file */
	whence = lckdat->l_whence;
	if (retval = convoff(size, offset, lckdat, SEEK_SET))
		return (retval);

	/* Convert l_len to be the end of the rec lock l_end */
	if (lckdat->l_len < 0)
		return (EINVAL);

	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += (lckdat->l_start - 1);

	/* check for arithmetic overflow */
	if (lckdat->l_start > lckdat->l_end)
		return (EINVAL);

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	FILOCK_LOCK();
	GN_RECLK_LOCK(gp);

	lock_list = & gp->gn_filocks;

	do {
		contflg = 0;
		switch (lckdat->l_type) {
		case F_RDLCK:
		case F_WRLCK:
			if ((found=blocked(*lock_list, lckdat, &insrt))==NULL) {
				/* not blocked */
				if (cmd & SETFLCK) {
					retval=flckadj(
					  lock_list, insrt, lckdat, retry_id);
				} else
					lckdat->l_type = F_UNLCK;
			} 
			else if (retry_fcn) {
				/*
				 * blocked but caller cannot sleep,
				 * so check for deadlock and enter
				 * entry with retry_fcn on sleep locks
				 * list
				 */
				if (deadflck(found, lckdat)) {
					retval = EDEADLK;
				} else if ((sf = insflck(&sleeplcks,lckdat,
						NULL)) == NULL) {
					retval = ENOLCK;
				} else {
					found->state |= LCK_BLOCKER;
					sf->state    |= LCK_BLOCKED;
					sf->pid   = found->set.l_pid;
					sf->sysid = found->set.l_sysid;
					sf->vfs = found->set.l_vfs;
					sf->filockp = (struct filock *)found;
					sf->retry_fcn = (int (*) ())retry_fcn;
					*retry_id = (ulong)found;
					retval = EAGAIN;
				}
			}
			else if (cmd & SLPFLCK) {
				/* blocked */
				/* do deadlock detection here */
				if (deadflck (found, lckdat)) {
					retval = EDEADLK;
				} 
				else if((sf=insflck(&sleeplcks,lckdat,
						NULL)) == NULL) {
					retval = ENOLCK;
				} 
				else {
					found->state |= LCK_BLOCKER;
					sf->state    |= LCK_BLOCKED;
					sf->pid   = found->set.l_pid;
					sf->sysid = found->set.l_sysid;
					sf->vfs = found->set.l_vfs;
					sf->filockp = found;
					sf->retry_fcn = NULL;

					if (cmd & INOFLCK) {
						ASSERT(rele_fcn != NULL);
						(*rele_fcn)(gp->gn_data);
					}

					e_assert_wait(&found->event, 1);

					GN_RECLK_UNLOCK(gp);
					FILOCK_UNLOCK();

					if (e_block_thread()
							== THREAD_INTERRUPTED)
						retval = EINTR;
					else
						contflg = 1;

					if (cmd & INOFLCK) {
						ASSERT(lock_fcn != NULL);
						(*lock_fcn)(gp->gn_data);
					}
					FILOCK_LOCK();
					GN_RECLK_LOCK(gp);

					sf->pid   = 0;
					sf->sysid = 0;
					sf->vfs = 0;
					sf->filockp = 0;
					delflck (&sleeplcks, sf);
				}
			} else if (cmd & SETFLCK) {
				retval = EAGAIN;
			} else {
				*lckdat = found->set;
			}
			break;
		case F_UNLCK:
			/* removing a file record lock */
			if (cmd & SETFLCK) {
				/*
				 * NFS may remove entries it placed on the
				 * sleep locks list. If sysid is -1 then
				 * remove all NFS entries on the sleep queue.
				 */
				if (lckdat->l_vfs == MNT_NFS && *retry_id) {
					struct filock  *next;
					sf = sleeplcks;
					while(sf) { 
					   next = sf->next;
					   if(sf->set.l_vfs == MNT_NFS) {
					      if (lckdat->l_sysid == -1) {
							delflck(&sleeplcks,sf);
					      } else if (sf->set.l_sysid ==
					      lckdat->l_sysid &&
					      sf->set.l_pid == 
					      lckdat->l_pid &&
					      sf->set.l_len == 
					      lckdat->l_len &&
					      sf->set.l_start == 
					      lckdat->l_start) {
							delflck(&sleeplcks,sf);
							break;
					      }
					   }
					   sf = next;
					}
				} else
					retval = flckadj(lock_list, *lock_list,
						lckdat, retry_id);
			}
			break;
		default:
			/* invalid lock type */
			retval = EINVAL;
			break;
		}
	} while (contflg);
	GN_RECLK_UNLOCK(gp);
	FILOCK_UNLOCK();

endloop:
	/* Restore l_len */
	if (lckdat->l_end == MAXEND)
		lckdat->l_len = 0;
	else
		lckdat->l_len -= (lckdat->l_start-1);
	if (! (cmd & SETFLCK))
		whence = 0;
	convoff(size, offset, lckdat, whence);
BUGLPR(lckbug, BUGACT, ("comm_reclock returning = %d\n", retval));

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return(retval);
}

/* convoff - converts the given data (start, whence) to the
 * given whence.
 */
int
convoff(size, offset, lckdat, whence)
offset_t size;
offset_t offset;
struct eflock *	lckdat;
int whence;
{
	if (lckdat->l_whence == SEEK_CUR)
		lckdat->l_start += offset;
	else if (lckdat->l_whence == SEEK_END)
		lckdat->l_start += size;
	else if (lckdat->l_whence != SEEK_SET)
		return (EINVAL);
	if (lckdat->l_start < 0)
		return (EINVAL);
	if (whence == SEEK_CUR)
		lckdat->l_start -= offset;
	else if (whence == SEEK_END)
		lckdat->l_start -= size;
	else if (whence != SEEK_SET)
		return (EINVAL);
	lckdat->l_whence = whence;
	return (0);
}

/* deadflck does the deadlock detection for the given record */
int
deadflck(flp, lockdat)
struct filock	*flp;
struct eflock	*lockdat;
{
	register struct filock *blck, *sf;
	pid_t blckpid;
	ulong blcksysid;
	ushort blckvfs;

	blck = flp;	/* current blocking lock pointer */
	blckpid = blck->set.l_pid;
	blcksysid = blck->set.l_sysid;
	blckvfs = blck->set.l_vfs;
	do {
		if (	(blckpid   == lockdat->l_pid)
		    &&	(blcksysid == lockdat->l_sysid)
		    &&	(blckvfs   == lockdat->l_vfs))
			return(1);
		/* if the blocking process is sleeping on a locked region,
		 * change the blocked lock to this one.
		 */
		for (sf = sleeplcks; sf != NULL; sf = sf->next) {
			if (	(blckpid   == sf->set.l_pid)
			     && (blcksysid == sf->set.l_sysid)
			     && (blckvfs   == sf->set.l_vfs)
			     && (sf->state & LCK_BLOCKED)) {
				blckpid   = sf->pid;
				blcksysid = sf->sysid;
				blckvfs   = sf->vfs;
				break;
			}
		}
		blck = sf;
	} while (blck != NULL);
	return(0);
}

/*
** lock wakeup
**
** 1. marks lock as zombie on sleep and current lock lists
** 2. iff necessary, wakes up anyone sleeping on this lock
*/

lock_wakeup (lp, retry_func)
struct filock	*lp;
int (**retry_func)();
{
	struct filock *slp, *next;
	
	/*
	 * never will succeed on this test if we're deleting
	 * the lock from the sleep lock list since
	 * it will be LCK_BLOCKED, not LCK_BLOCKER
	 */
	if (lp->state & LCK_BLOCKER)
	{
		next = sleeplcks;
		while (slp = next) {
			next = slp->next;
			/*
			 *  Find any sleep locks blocking on this lock and
			 * mark them as not LCK_BLOCKED so deadflck will ignore
			 * them.  
			 */
			if (lp == slp->filockp) {
				slp->state &= ~LCK_BLOCKED;
				lp->state  &= ~LCK_BLOCKER;
				/*
				** never test for WASBLOCK
				** but useful when debugging
				*/
				lp->state |= LCK_WASBLOCK;

				/* 
				 * If there is a retry funcion assoc.
				 * with this sleeper, then callit and
				 * return the address of the retry funcion
				 * so that flckadj() can call the retry
				 * when it has done all of its lock_wakeups
				 */
				if (slp->retry_fcn) {
					(*slp->retry_fcn)(lp, NULL);
					*retry_func = slp->retry_fcn;
					slp->pid = 0;
					slp->sysid = 0;
					slp->vfs = 0;
					slp->retry_fcn = NULL;
					delflck(&sleeplcks, slp);
				}
			}
		}
		e_wakeupx(&lp->event, E_WKX_NO_PREEMPT);
	}
	return;
}

/*
* flock_grow - Attempt to grow the file lock list.
*/
static
flock_grow()
{
	register struct filock *fip;
	register struct filock *fend;
	register int i;

	fip = (struct filock *) v.ve_lock;
	i = MIN(nflox_max, v.v_lock + PAGESIZE/sizeof(struct filock));
	fend = flox + i;

	/* initialize another page of file locks */
	for( ; fip < fend; fip++ )
	{
		if (frlock == NULL)
		{
			fip->next = fip->prev = NULL;
			frlock = fip;
		}
		else
		{
			fip->next = frlock;
			fip->prev = NULL;
			frlock = (frlock->prev = fip);
		}
	}

	v.v_lock = i;
	v.ve_lock = (caddr_t) fip;
}
