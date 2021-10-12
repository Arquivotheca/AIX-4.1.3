static char sccsid[] = "@(#)66	1.3  src/bos/usr/bin/dosdir/dlock.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:56:38";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dlock lcleanup lprint lprent 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "pcdos.h"
#include "doserrno.h"

int *llast;
struct lockent *lockarr;
extern int dos_pid;

static struct lockent emptylock = { 0 };

int
dlock(pid, file, off, len, lcode)
int pid;
FCB *file;
int off, len;
int lcode;
{
	struct lockent ent;
	int fh1, fh2;
	int ret;

static char *lcodestr[] = {"lock", "unlock", "testlock"};
TRACE(("dlock(pid = %d, file = %x, off = %d, len = %d, lcode = %s)\n",
		pid, file, off, len, lcodestr[lcode]));
	if(file->magic != FCBMAGIC){
		doserrno = DE_INVAL;
		return(-1);
	}
	/* len==0 is a noop */
	if(len == 0)
		return(0);
	if(off < 0 || len < 0){
		doserrno = DE_INVAL;
		return(-1);
	}

	ent.pid = pid;
	ent.fh1 = FHANDLE1(file);
	ent.fh2 = FHANDLE2(file);
	ent.offset = off;
	ent.length = len;

	flock_lock();
	switch(lcode){
	case L_LOCK:
		if((ret = lfind(&ent, ACT_MEMBER)) >= 0){
			doserrno = DE_DEADLK;
			ret = -1;
		} else
		ret = ladd(&ent);
		break;
	case L_UNLOCK:
		ret = ldel(&ent);
		break;
	case L_TEST:
		ret = ((lfind(&ent, ACT_MEMBER) >= 0)?-1:0);
		break;
	default:
		ret = -1;
		break;
	}
	flock_unlock();
	return(ret);
}

/*
 * ladd - add entry to end of lock array
 */
static
ladd(pent)
struct lockent *pent;
{
TRACE(("ladd(pent = %x)\n", pent));
if(dostrace)lprent(pent);
	if(*llast == DOS_NLOCKS){
		doserrno = DE_DEADLK;
		return(-1);
	}
	lockarr[(*llast)++] = *pent;
	return(0);
}

/*
 * ldel - lfind exact match in lock array
 *	copy *llast entry over found entry
 *	zero out old *llast entry (just to be neat)
 */
static
ldel(pent)
struct lockent *pent;
{
	int i;

TRACE(("ldel(pent = %x)\n", pent));
	if((i = lfind(pent, ACT_MATCH)) < 0)
		return(-1);
	lockarr[i] = lockarr[--(*llast)];
	lockarr[*llast] = emptylock;
	return(0);
}

/*
 * lfind - loop thru lock array, looking for an entry
 */
static
lfind(pent, action)
struct lockent *pent;
char action;
{
	int i;

TRACE(("lfind(pent = %x, action = %d)\n", pent, action));
	for(i=0; i<*llast; i++){
		if(lcmp(pent, &(lockarr[i]), action) == 0)
			return(i);	/* found a lock */
	}
	return(-1);			/* didn't find one */
}

/*
 * if action == ACT_MATCH, then we're trying to match a lock record
 * which we're trying to insert or delete.
 * if action == ACT_MEMBER, then we're trying to see if a region
 * lies within a locked region.
 * return code == 0 if match found, != 0 if not.
 */
static
lcmp(a, b, action)
struct lockent *a, *b;
char action;
{
TRACE(("lcmp(a = %x, b = %x, action = %d)\n", a, b, action));
if (dostrace){
	printf("a: ");
	lprent(a);
	printf("b: ");
	lprent(b);
}
	if(action == ACT_MATCH){
		if((a->pid != b->pid) ||
		   (a->fh1 != b->fh1) ||
		   (a->fh2 != b->fh2) ||
		   (a->offset != b->offset) ||
		   (a->length != b->length)) return(1);
	}
	else if(action == ACT_MEMBER){
		int a1, a2, b1, b2;

/* is it our lock? does the file not match? */
		if((a->pid == b->pid) ||
		   (a->fh1 != b->fh1) ||
		   (a->fh2 != b->fh2)) return(1);
/* possible lock, check the lock conditions */
/* is either region contained within the other? */
		a1 = a->offset;	a2 = a->offset + a->length;
		b1 = b->offset;	b2 = b->offset + b->length;
/* thanks to rob pike and karl heuer for this expr */
		if(!((a1 < b2) && (b1 < a2)))
			return(1); /* no lock */
	}
	return(0);	/* hit a lock */
}

/*
 * lcleanup - clean up this pid's locks, during crash and burn
 */
lcleanup(pid)
int pid;
{
	int i;

TRACE(("lcleanup(pid = %d)\n", pid));
	if((pid != dos_pid) && (geteuid() != 0)){
		doserrno = DE_PERM;
		return(-1);
	}
	flock_lock();
	for(i=0; i<*llast; i++){
		if(lockarr[i].pid == pid){
			lockarr[i] = lockarr[--(*llast)];
			lockarr[*llast] = emptylock;
			i--; /* we have replaced lockarr[*llast]! */
		}
	}
	flock_unlock();
	return(0);
}

lprint()
{
	int i;
	flock_lock();
	printf("%d lock%s:\n", *llast, *llast==1?"":"s");
	printf("pid	fh1		fh2	off	len\n");
	for(i=0; i<*llast; i++)
		lprent(&(lockarr[i]));
	flock_unlock();
}

lprent(pent)
struct lockent *pent;
{
		printf("%d	%x	%x	%d	%d\n",
			pent->pid,
			pent->fh1,
			pent->fh2,
			pent->offset,
			pent->length);
}
