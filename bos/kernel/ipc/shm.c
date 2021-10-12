static char sccsid[] = "@(#)20	1.33.1.16  src/bos/kernel/ipc/shm.c, sysipc, bos411, 9439A411b 9/26/94 11:40:07";
/*
 * COMPONENT_NAME: (SYSIPC) IPC shared memory services
 *
 * FUNCTIONS: shmget, shmat, shmdt, shmctl, shmconv, ishmdt, shmex
 *	   shmfork, shmfree, shm_getseg, shmwait, psrmshm, shm_lock_init
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
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/user.h>
#include <sys/shm.h>
#include <sys/lockl.h>
#include <sys/errno.h>
#include <sys/vmuser.h>
#include <sys/adspace.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include <sys/id.h>
#ifdef _POWER_MP
#include <sys/atomic_op.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif

extern struct shmid_ds	shmem[];	/* shared memory headers */
extern struct shminfo   shminfo;        /* sys-wide shared memory info structure */

extern time_t		time;		/* system idea of date */

#ifdef _POWER_MP
Simple_lock lock_shm;
#define	LOCK_SHM   -1
#else
lock_t shm_lock = LOCK_AVAIL;		/* lock for shared memory services */
#endif

uint_t shm_mark = 0;			/* high water mark for descriptors */

BUGVDEF(shm_debug, 0);
BUGVDEF(shmm_debug, 0);

struct	shmid_ds	*ipcget(),
			*shmconv();

void			shmfree(),
			ishmdt(),
			rmshseg();

#ifdef _POWER_MP
	#define SHM_GET_LOCK(locked) { \
		if (!(locked)) { simple_lock(&lock_shm); (locked) = TRUE;}}
#else
	#define SHM_GET_LOCK(locked) { \
		if (!(locked)) {(locked) = lockl(&shm_lock,LOCK_SHORT);\
			ASSERT( (locked) == LOCK_SUCC ); (locked) = TRUE;}}
#endif

#ifdef _POWER_MP
	#define SHM_REL_LOCK(locked) {if ((locked)) simple_unlock(&lock_shm);}
#else
	#define SHM_REL_LOCK(locked) {if ((locked)) unlockl(&shm_lock);}
#endif

#ifdef _POWER_MP
/*
 * NAME: shm_lock_init
 *                                                                    
 * FUNCTION: This function is called in ipc_lock_init() in ipc.c.
 *	     ipc_lock_init() is called during system initialization when 
 *	     _POWER_MP is defined.
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * DATA STRUCTURES: lock_shm 
 *
 * RETURNS: None. 
 */  

void
shm_lock_init()
{
	/* initialize the shared memory lock */
	lock_alloc(&lock_shm,LOCK_ALLOC_PAGED,SHM_LOCK_CLASS,LOCK_SHM);
	simple_lock_init(&lock_shm);
}
#endif /* _POWER_MP */

/*
 * NAME: shmget
 *                                                                    
 * FUNCTION: Get an identifier for a shared memory segment associated
 *           with the key given, perhaps creating it along the way.
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	system call
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS: -1 if not successful    identifier if successful
 */  

int
shmget( key_t key, int size, int shmflg)
{
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	int				s;	/* ipcget status */
	uint_t				rv;	/* returned shm identifier */
	int svcrc = 0;
	static int svcnum = 0;

	if (audit_flag)
		svcrc = audit_svcstart("SHM_Create",&svcnum,0);

	TRCHKGT_SYSC(SHMGET, key, size, shmflg, NULL, NULL);
	BUGLPR(shm_debug, BUGNTA, ("shmget: key=%d size=%d shmflag=%d\n",
		key, size, shmflg) );

#ifdef _POWER_MP
	simple_lock(&lock_shm);
#else
	rv = lockl(&shm_lock,LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
#endif

	if((sp = ipcget(key, shmflg, shmem, shminfo.shmmni,
			sizeof(*sp), &s, &shm_mark)) == NULL){
		BUGLPR(shm_debug, BUGGID, ("shmget: ipcget failed\n"));
		goto outx;
	}

	if(s)
	{
		/* This is a new shared memory segment.  Allocate memory and
			finish initialization. */
		if(size < shminfo.shmmin || size > shminfo.shmmax) {
			u.u_error = EINVAL;
			sp->shm_perm.mode = 0;
			BUGLPR(shm_debug, BUGGID, ("shmget: bad seg size\n"));
			goto outx;
		}
		sp->shm_segsz = size;
		if ( shm_getseg(sp) ) {
			u.u_error = ENOMEM;
			sp->shm_perm.mode = 0;
			BUGLPR(shm_debug, BUGGID, \
				("shmget: seg create failed\n"));
			goto outx;
		}

		sp->shm_nattch = sp->shm_cnattch = 0;
		sp->shm_atime = sp->shm_dtime = 0;
		sp->shm_ctime = time;
		sp->shm_lpid = 0;
		sp->shm_cpid = U.U_procp->p_pid;
#ifdef _POWER_MP
		sp->shm_perm.key = key;
#endif
	} else  /** it is not a new segment **/
		if(size && sp->shm_segsz < size) {
			u.u_error = EINVAL;
			BUGLPR(shm_debug, BUGGID, \
				 ("shmget: seg size error\n"));
			goto outx;
		}
	rv = (sp->shm_perm.seq * shminfo.shmmni) + (sp - shmem);

outx:

	if(svcrc){

		if(u.u_error){

			int bval = -1;

			audit_svcbcopy(&bval, sizeof(int));
			audit_svcfinis();

		}
		else {

			audit_svcbcopy(&rv, sizeof(uint_t));
			audit_svcfinis();

		}

	}

#ifdef _POWER_MP
	simple_unlock(&lock_shm);
#else
	unlockl(&shm_lock);
#endif
	BUGLPR(shm_debug, BUGNTF, ("shmget: rv = %d  errno = %d\n", \
		u.u_error ? -1:rv, u.u_error) );
	return u.u_error ? -1 : rv;
}

/*
 * NAME: shmat
 *
 * FUNCTION: shmat system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: Changes u.u_error
 *
 * RETURNS:
 *	start address of shared memory if successful
 *	-1 if not successful
 *
 */

void * shmat(int shmid, const void *iaddr, int flag)
{
	struct shmid_ds	*sp;		/* shared memory header ptr	*/
	struct segstate *segstate;	/* pointer users segment state	*/
	int rv;
	int seg_num;			/* user segment being shmated	*/
	static int svcnum = 0;
        char * addr = iaddr;
	int mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	if(audit_flag && audit_svcstart("SHM_Open", &svcnum, 1, shmid)){
		audit_svcfinis();
	}

	TRCHKGT_SYSC(SHMAT, shmid, iaddr, flag, NULL, NULL);
	BUGLPR(shm_debug, BUGNTF, ("shmat shmid= %X addr= %X flag= %X\n", \
		shmid, addr, flag ) );

#ifdef _POWER_MP
	simple_lock(&lock_shm);
#else
	rv = lockl(&shm_lock,LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
#endif

	if(flag & SHM_RND)
		addr = (char *)((uint)addr & ~(SHMLBA - 1));

	if (flag & SHM_MAP) {
		(void)mapfile(shmid, &addr, flag);
		goto outx;
	}

	if (flag & SHM_FMAP) {
		(void)fmapfile(shmid, &addr, flag);
		goto outx;
	}

	if((sp = shmconv(shmid, SHM_DEST)) == NULL)
		goto outx;
	if(ipcaccess(&sp->shm_perm, SHM_R)){
		BUGLPR(shm_debug, BUGGID, ("shmat: ipcaccess failed\n"));
		goto outx;
	}

	if((flag & SHM_RDONLY) == 0)
		if(ipcaccess(&sp->shm_perm, SHM_W)) {
			BUGLPR(shm_debug, BUGGID, \
				 ("shmat: ipcaccess failed\n"));
			goto outx;
		}

	if( (seg_num = shm_findspace(&addr, 1)) == -1 ) {
		BUGLPR(shm_debug, BUGGID, ("shmat: shm_findspace failed\n"));
		goto outx;
	}

	/*
	 * Give process addressability to the section of memory.
	 * Then set segment state in u block
	 */
	shm_ldseg(addr, sp->shm_handle, flag, 1);

	segstate = &U.U_segst[ seg_num ];
	ASSERT(segstate->segflag == SEG_AVAIL);
	segstate->segflag = SEG_SHARED;
	segstate->num_segs = 1;
	segstate->shm_ptr = sp;

	/*
	 * Don't need to clear segment on first attach; it is created as 0's
	 */
	sp->shm_nattch++;
	sp->shm_cnattch++;
	sp->shm_atime = time;
	sp->shm_lpid = U.U_procp->p_pid;
outx:

#ifdef _POWER_MP
	simple_unlock(&lock_shm);
#else
	unlockl(&shm_lock);
#endif

	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	BUGLPR(shm_debug, BUGNTF, ("shmat: rv=%X errno=%d\n", \
		u.u_error ? -1:(int)addr, u.u_error));
	return u.u_error ? (void *)-1 : (void *)addr;
}

/*
 * NAME:shmdt
 *
 * FUNCTION: shmdt system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: Sets u.u_error
 *
 * RETURNS: 0 if successful	-1 if not successful
 */

int
shmdt(const void  *addr)
{
	int rv;
	int mthread, waslocked;

	/*
	 * Acquire address space lock if multi-threaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	TRCHKLT_SYSC(SHMDT, addr);
	BUGLPR(shm_debug, BUGNTF, ("shmdt addr= 0x%X\n", addr));

#ifdef _POWER_MP
        simple_lock(&lock_shm);
#else
        rv = lockl(&shm_lock,LOCK_SHORT);
        ASSERT( rv == LOCK_SUCC );
#endif
	
	ishmdt(addr);

#ifdef _POWER_MP
	simple_unlock(&lock_shm);
#else
	unlockl(&shm_lock);
#endif

	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);

	BUGLPR(shm_debug, BUGNTF, ("shmdt rv = %d errno = %d\n", \
		u.u_error ? -1 : 0, u.u_error) );
	return u.u_error ? -1 : 0;
}

/*
 * NAME: shmctl
 *
 * FUNCTION: shmctl system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: changes u.u_error
 *
 * RETURNS: 0 if successful	-1 if not successful
 */
int
shmctl(int shmid, int cmd, struct shmid_ds *arg)
/* int			shmid;	*/
/* int			cmd;	*/
/* struct shmid_ds *	arg;	*/
{
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	struct shmid_ds			ds;	/* hold area for IPC_SET */
/*	int i, segno;   */
	int rv;
	static int svcnumC = 0;
	static int svcnumO = 0;
	static int svcnumM = 0;
	uid_t	current_uid;

	TRCHKGT_SYSC(SHMCTL, shmid, cmd, arg, NULL, NULL);
	BUGLPR(shm_debug, BUGNTF, ("shmctl shmid=%X cmd=%X arg=%X\n" , \
			shmid, cmd, arg) );

#ifdef _POWER_MP
        simple_lock(&lock_shm);
#else
        rv = lockl(&shm_lock,LOCK_SHORT);
        ASSERT( rv == LOCK_SUCC );
#endif


	if((sp = shmconv(shmid, (cmd == IPC_STAT) ? 0 : SHM_DEST))
	    == NULL)
		goto outx;
	current_uid = getuidx(ID_EFFECTIVE);

	switch(cmd) {

	/* Remove shared memory identifier. */
	case IPC_RMID:
		if(current_uid != sp->shm_perm.uid && current_uid != sp->shm_perm.cuid
			&& !priv_req(BYPASS_DAC_WRITE)) {
			u.u_error = EPERM;
			goto outx;
		}
		sp->shm_ctime = time;
		sp->shm_perm.mode |= SHM_DEST;

		if(audit_flag && audit_svcstart("SHM_Close", &svcnumC, 1, shmid)){
			audit_svcfinis();
		}
		/* Change key to private so old key can be reused without
			waiting for last detach.  Only allowed accesses to
			this segment now are shmdt() and shmctl(IPC_STAT).
			All others will give bad shmid. */
		sp->shm_perm.key = IPC_PRIVATE;

		/* Adjust counts to counter shmfree decrements. */
		sp->shm_nattch++;
		sp->shm_cnattch++;
		shmfree(sp);
		goto outx;

	/* Set ownership and permissions. */
	case IPC_SET:
		if(current_uid != sp->shm_perm.uid && current_uid != sp->shm_perm.cuid
			&& !priv_req(SET_OBJ_DAC)) {
			u.u_error = EPERM;
			goto outx;
		}
		if(copyin(arg, &ds, SHMID_SIZE)) {
			u.u_error = EFAULT;
			goto outx;
		}
		if(audit_flag && audit_svcstart("SHM_Owner", &svcnumO, 3, 
		shmid, ds.shm_perm.uid, ds.shm_perm.gid)){
			audit_svcfinis();
		}
		sp->shm_perm.uid = ds.shm_perm.uid;
		sp->shm_perm.gid = ds.shm_perm.gid;
		sp->shm_perm.mode = (ds.shm_perm.mode & 0777) |
			(sp->shm_perm.mode & ~0777);
		sp->shm_ctime = time;
		goto outx;

	/* Get shared memory data structure. */
	case IPC_STAT:
		if(ipcaccess(&sp->shm_perm, SHM_R))
			goto outx;
		if(audit_flag && audit_svcstart("SHM_Mode", &svcnumM, 2, 
		shmid, ds.shm_perm.mode)){
			audit_svcfinis();
		}
		if(copyout(sp, arg, SHMID_SIZE))
			u.u_error = EFAULT;
		goto outx;

	case SHM_SIZE:
		if(current_uid != sp->shm_perm.uid && current_uid != sp->shm_perm.cuid
			&& !priv_req(SET_OBJ_STAT)) {
			u.u_error = EPERM;
			goto outx;
		}
		if(copyin(arg, &ds, SHMID_SIZE)) {
			u.u_error = EFAULT;
			goto outx;
		}
		if(ds.shm_segsz > shminfo.shmmax) {
			u.u_error = EINVAL;
			goto outx;
		}
		if(shm_growseg(sp, ds.shm_segsz)) {
			u.u_error = ENOMEM;
			goto outx;
		}

		sp->shm_ctime = time;
  		sp->shm_segsz =  ds.shm_segsz;
		break;		/* goto outx; */

	default:
		u.u_error = EINVAL;
		goto outx;
	}
outx:

#ifdef _POWER_MP
	simple_unlock(&lock_shm);
#else
	unlockl(&shm_lock);
#endif
	BUGLPR(shm_debug, BUGNTF, ("shmctl: rv = %d errno = %d\n" , \
		u.u_error ? -1 : 0, u.u_error) );
	return u.u_error ? -1 : 0;
}

/*
 * NAME ishmdt
 *
 * FUNCTION: internal shared memory detach routine
 *
 * EXECUTION ENVIRONMENT:
 *	called by shmdt system call
 *
 * DATA STRUCTURES: changes u.u_error
 *
 * RETURNS: NONE
 */

void
ishmdt(addr)
register char *addr;
{
	register struct shmid_ds        *sp;
	register int i;
	register struct segstate *segstate;

	BUGLPR(shm_debug, BUGNTA, ("ishmdt addr=%X\n", addr));
	/*
	 * Check for segment alignment and segment being shared
	 */
	i = (unsigned int)addr >> SEGSHIFT;
	if ( (((int)addr & (SHMLBA-1)) == 0) &&
		(i >= SHMLOSEG) && (i <= SHMHISEG) ){

		segstate = &U.U_segst[i];
		if (segstate->segflag & SEG_MAPPED) {
			rmmseg(i);
			return;
		}
		if (segstate->segflag & SEG_SHARED) {
			rmshseg(segstate, i);
			return;
		}
	}

	BUGLPR(shm_debug, BUGGID, ("bad address =%X\n" , addr));
	u.u_error = EINVAL;
	return;
}

/*
 * NAME: shmconv
 *
 * FUNCTION: Convert user supplied shmid into a ptr to the associated
 *	shared memory header.
 *
 * EXECUTION ENVIRONMENT:
 *	called by shared memory system calls
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS:
 *	pointer to shared memory descriptor if successful
 *	NULL if not successful
 */

struct shmid_ds *
shmconv(s, flg)
uint_t	s;		/* shmid */
int	flg;		/* error if matching bits are set in mode */
{
	register struct shmid_ds	*sp;	/* ptr to associated header */
	uint_t index;				/* descriptor index	*/
	uint_t seq;				/* sequence number	*/

	seq = s / shminfo.shmmni;	/* get sequence number and index */
	index = s % shminfo.shmmni;

	if ( index >= shm_mark ) {
		u.u_error = EINVAL;
		return( NULL );
	}

	sp = &shmem[index];		/* low 16 bits of shmid=index */
	if ((seq != sp->shm_perm.seq)	/* high 16 bits = usage cnt */
	   || (sp->shm_perm.mode & flg)
	   || !(sp->shm_perm.mode & IPC_ALLOC) ) {
		u.u_error = EINVAL;
		BUGLPR(shm_debug, BUGGID, ("shmconv: bad index=%X\n" , s));
		return(NULL);
	}

	return(sp);
}


/*
 * NAME:shmex
 *
 * FUNCTION: Called by exit and exec to handle shared memory processing.
 *
 * EXECUTION ENVIRONMENT:
 *	caller must be a process
 *
 * RETURNS: NONE
 *
 * NOTES:
 *      This function was previously called shmexec. shmexit used to
 *      call shmexec.
 *
 *      Same processing for exec and exit. At exit, no thread locking 
 *      is required because the process is single threaded.
 */
shmex()
{
	struct shmid_ds *sp;
	struct segstate *seg_state;
	int seg;
	int locked = 0;
	int num_segs;
	int avail;

	BUGLPR(shm_debug, BUGNTF, ("shmex\n"));

	/* Detach any attached segments. */
	seg = SHMLOSEG;
	do{
		seg_state = &U.U_segst[seg];
		num_segs = seg_state->num_segs;
		avail = seg_state->segflag == SEG_AVAIL;
		if ( seg_state->segflag & SEG_MAPPED) {
			SHM_GET_LOCK(locked);
			rmmseg(seg);
			}
		
		else if (seg_state->segflag & SEG_MMAP){
			SHM_GET_LOCK(locked);
			rmmapseg(seg);
			}
		else if (seg_state->segflag & SEG_SHARED){
			SHM_GET_LOCK(locked);
			rmshseg(seg_state, seg);}
		seg += (avail) ? 1 : num_segs;
	}while(seg <= SHMHISEG);

	ASSERT(seg == SHMHISEG + 1);

	SHM_REL_LOCK(locked);
}

/*
 * NAME: shmfork
 *
 * FUNCTION: handles shared memory fork processing
 *
 * EXECUTION ENVIRONMENT:
 *	Called by kfork while running under parent process
 *
 * RETURNS: NONE
 *
 * NOTE:
 *      Caller must be holding the process user address space lock.
 */

void
shmfork(uchild)
struct user *uchild;		/* pointer to child's user area */
{
	struct segstate *ssptr;
	struct shmid_ds *shmptr;
	int seg;
	int locked=0;
	int mthread;

	BUGLPR(shm_debug, 1, ("shmfork\n"));

	/*
	 * If this process is multithreaded, copy the
	 * adspace allocation under the adspace lock.
	 * XXX - Note that all the allocation bits are
	 * copied here.  This assumes that the same
	 * segment registers are always allocated in
	 * the parent and the child.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
		uchild->U_adspace.alloc = U.U_adspace.alloc;

	/*
	 * update counts on any attached segments.
	 */
	seg = SHMLOSEG;
	do {
		ssptr = &U.U_segst[seg];

		/*
		 * If this process is multithreaded, copy the
		 * adspace and segstate structure here under the
		 * adspace lock.  Big data segments have already
		 * been handled.
		 */
		if (mthread && !(ssptr->segflag & SEG_WORKING))
		{
			uchild->U_segst[seg] = *ssptr;
			uchild->U_adspace.srval[seg] =
				U.U_adspace.srval[seg];
		}

		if (ssptr->segflag & SEG_SHARED){
			SHM_GET_LOCK(locked);
			shmptr = ssptr->shm_ptr;
			shmptr->shm_nattch++;
			shmptr->shm_cnattch++;
	    	} else if (ssptr->segflag & SEG_MAPPED) {
			SHM_GET_LOCK(locked);
			mapfork( seg );}
	    	else if (ssptr->segflag & SEG_MMAP){
			SHM_GET_LOCK(locked);
			mmapfork(uchild, seg);}

		seg += (ssptr->segflag == SEG_AVAIL) ? 1 : ssptr->num_segs;
	} while(seg <= SHMHISEG);

	ASSERT(seg == SHMHISEG + 1);

	SHM_REL_LOCK(locked);
}

/*
 * NAME:shmfree
 *
 * FUNCTION: Decrement counts.  Free segment if indicated.
 *
 * EXECUTION ENVIRONMENT:
 *	called by shmctl
 *
 * RETURNS: NONE
 */

void
shmfree(sp)
register struct shmid_ds *sp;	 /* shared memory header ptr */
{
	ASSERT( sp != NULL );

	sp->shm_nattch--;
	if (--(sp->shm_cnattch) == 0 && sp->shm_perm.mode & SHM_DEST) {
		sp->shm_perm.mode = 0;
		sp->shm_perm.seq++;
		shm_dseg(sp);
	}
}

/*
 * NAME: rmshseg
 *
 * FUNCTION: remove a memory segment
 *
 * EXECUTION ENVIRONMENT:
 *	called by shm_exit and ishmdt
 *
 * RETURNS: NONE
 */
void
rmshseg(segst,addr)
struct segstate *segst;
uint_t addr;
{
	struct shmid_ds *sp;

	sp = segst->shm_ptr;
	shm_freespace(addr << SEGSHIFT, 1);
	segst->shm_ptr = (struct shmid_ds *)0;
	segst->segflag = SEG_AVAIL;
	segst->num_segs = 0;

	/* segment cannot be freed until SID shootdown has occurred */
	shmfree(sp);
	sp->shm_dtime = time;
	sp->shm_lpid = U.U_procp->p_pid;

	return;
}

/*
 * NAME: psrmshm
 *
 * FUNCTION: remove all the shared memory segments attached to a process
 *
 * EXECUTION ENVIRONMENT:
 *	process
 *
 * NOTE:
 *	this is only used by processes killed by the paging space monitor
 *
 * RETURNS: NONE
 */
void
psrmshm(p)
struct proc	*p;
{
    struct shmid_ds        *sp;    /* shared memory header ptr */
    struct segstate	*segst;
    struct segstate *endsegst;
    int		shmid;
    
    segst = U.U_segst;
    endsegst = U.U_segst + NSEGS;
    while (segst < endsegst) {
	if ((segst->segflag & (SEG_AVAIL | SEG_SHARED)) == SEG_SHARED) {
	    
	    shmid = (segst->shm_ptr - shmem) +
		segst->shm_ptr->shm_perm.seq * shminfo.shmmni;
	    
	    
	    /* if there is only one process (this one) attached to
	       this segment, remove the id so the segment will
	       go away when this process exits */
	    if (((sp = shmconv(shmid, SHM_DEST)) != NULL) && 
                 (sp->shm_cnattch == 1))
		 shmctl (shmid, IPC_RMID, NULL);
	}
	++segst;
    }
    return;

}
