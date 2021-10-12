static char sccsid[] = "@(#)18	1.23.1.10  src/bos/kernel/ipc/segoff.c, sysipc, bos411, 9428A410j 6/21/94 19:31:00";
/*
 * COMPONENT_NAME: (SYSIPC) IPC shared memeory services
 *
 * FUNCTIONS: mapfile mapfork rmseg rmmseg fdtoseg
 *            fmapfile fmapfork rmfmap
 *            mmapfork rmmapseg
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/user.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/vnode.h>
#include <sys/vattr.h>
#include <sys/fp_io.h>
#include <sys/stat.h>
#include <sys/syspest.h>
#include <sys/vmuser.h>

BUGXDEF(shmm_debug);

/*
 * macros used to store and retrieve information used to map
 * files in association with the fshmat fast-SVC.
 */
#define	FSHMAT_VAL(k,n,h)	( (k)<<31 | ((n)-1)<<28 | ((h) & 0xfffff) << 8 )
#define	FSHMAT_KEY(v)		( (v)>>31 )
#define	FSHMAT_SID(v)		( ((v)>>8) & 0xfffff )

void	rmmseg();
void	rmfmap();

/*
 * NAME: mapfile
 *
 * FUNCTION: maps a file give a file descriptor
 *
 * EXECUTION ENVIRONMENT:
 *	called by the shmat system call
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS:	0 if successful		-1 if not successful
 */

int
mapfile(fd, addr, flag)
int	fd;			/* file descriptor	*/
uint_t	*addr;			/* address to map at	*/
int	flag;			/* shat flags		*/
{
	register struct vnode	*vp;
	struct file		*fp;
	int			seg_num;
	vmhandle_t		handle;
	struct segstate		*seg_state;
	struct stat		statbuf;
	int			num_segs;
	ushort			seg_flags;
	int			i;
	int			rc;

	ASSERT( NULL != addr );

	/* 
	 * Get the file structure pointer for the file specified 
	 * by the file descriptor parameter. 
	 */
	if ((u.u_error = getf (fd, &fp)) != 0) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: getf failed"));
		return( -1 );
	}

	/*
	 * It is an error if the file was not opened for 
	 * write access and the the file is not being mapped 
	 * Read-Only. 
	 */
	if (((fp->f_flag & FWRITE) == 0 && !(flag & SHM_RDONLY))) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: bad flags"));
		u.u_error = EACCES;
		ufdrele(fd);
		return( -1 );
	}

	/*
	 * You can't map a readonly file for deferred update
	 */
	if ((flag & (SHM_RDONLY|SHM_COPY)) == (SHM_RDONLY|SHM_COPY)) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: bad flags"));
		u.u_error = EINVAL;
		ufdrele(fd);
		return( -1 );
	}

	/*
	 * It is an error if this process has already mapped a file 
	 * with the specified file descriptor. 
	 */
	if (fdtoseg (fd, SEG_MAPPED) != -1) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: segment mapped"));
		u.u_error = EEXIST;
		ufdrele(fd);
		return( -1 );
	}

	vp = fp->f_vnode;

	if (VTOGP(vp)->gn_type != VREG) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: not VREG\n"));
		u.u_error = EBADF;
		ufdrele(fd);
		return( -1 );
	}

	/*
	 * find the number of segments needed to map the file
	 */
	if (rc = fp_fstat(fp, &statbuf, STATSIZE, FP_SYS)) {
		BUGLPR(shmm_debug, BUGGID, ("mapfile: fp_fstat failed\n"));
		u.u_error = rc;
		ufdrele(fd);
		return( -1 );
	}

	if ((uint_t)statbuf.st_size > (uint_t)(SEGSIZE*8)) {
		BUGLPR(shmm_debug, 1, ("bad file size\n"));
		u.u_error = EFBIG;
		ufdrele(fd);
		return(-1);
	}

	if (statbuf.st_size == 0)
		num_segs = 1;
	else
		num_segs = ((uint_t)statbuf.st_size + (SEGSIZE-1)) / SEGSIZE;

	/*
	 * check in the user address space, to see if there is enough
	 * space to map the file
	 */
	if ( (seg_num = shm_findspace(addr, num_segs)) == -1 ) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: findspace failed\n"));
		ufdrele(fd);
		return( -1 );
	}

	if ( rc = fp_shmat(fp, *addr, flag, &handle) ) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: fp_shmat failed\n"));
		ufdrele(fd);
		u.u_error = rc;
		return( -1 );
	}

	shm_ldseg(*addr, handle, flag, num_segs);

	seg_flags = SEG_MAPPED;

	if ( !(flag & SHM_RDONLY) )
		seg_flags |= SEG_MRDWR;

	if ( flag & SHM_COPY )
		seg_flags |= SEG_DEFER;
	

	seg_state = &U.U_segst[ seg_num ];
	ASSERT( seg_state->segflag == SEG_AVAIL );
	seg_state->segfileno = fd;
	seg_state->num_segs = num_segs;
	seg_state->segflag = seg_flags;

	seg_state++;
	for (i = 1 ; i < num_segs ; i++ ) {
		ASSERT(seg_state->segflag == SEG_AVAIL);
		seg_state->segfileno = -1;
		seg_state->num_segs = 0;
		seg_state->segflag = seg_flags;
		seg_state++;
	}

	U.U_ufd[fd].flags |= UF_MAPPED;

	/* Decrement the file descriptor count */
	ufdrele(fd);
	return( 0 );
}

/*
 * NAME: fmapfile
 *
 * FUNCTION: maps a file in preparation for use in fshmat fast-SVC
 *	     For this type of mapping, no segment registers are loaded now
 *	     but the information necessary for loading them is stored in
 *	     the ufd table to allow the fshmat fast-SVC to do so.
 *
 * EXECUTION ENVIRONMENT:
 *	called by the shmat system call
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS:	0 if successful		-1 if not successful
 */

int
fmapfile(fd, addr, flag)
int	fd;			/* file descriptor	*/
uint_t	*addr;			/* address to map at	*/
int	flag;			/* shat flags		*/
{
	register struct vnode	*vp;
	struct file		*fp;
	int			seg_num;
	vmhandle_t		handle;
	struct stat		statbuf;
	int			num_segs;
	int			i;
	int			rc;
	int			key;
	adspace_t		*adsp;
	int			sreg;

	ASSERT( NULL != addr );

	/* 
	 * Get the file structure pointer for the file specified 
	 * by the file descriptor parameter. 
	 */
	if ((u.u_error = getf (fd, &fp)) != 0) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: getf failed"));
		return( -1 );
	}

	/*
	 * It is an error if the file was not opened for 
	 * write access and the file is not being mapped 
	 * Read-Only. 
	 */
	if (((fp->f_flag & FWRITE) == 0 && !(flag & SHM_RDONLY))) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: bad flags"));
		u.u_error = EACCES;
		ufdrele(fd);
		return( -1 );
	}

	/*
	 * We don't support SHM_COPY for this type of mapping.
	 */
	if (flag & SHM_COPY) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: bad flags"));
		u.u_error = EINVAL;
		ufdrele(fd);
		return( -1 );
	}

	/*
	 * It is an error if this process has already mapped a file 
	 * with the specified file descriptor. 
	 */
	if (U.U_ufd[fd].flags & UF_MAPPED) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: segment mapped"));
		u.u_error = EEXIST;
		ufdrele(fd);
		return( -1 );
	}

	vp = fp->f_vnode;

	if (VTOGP(vp)->gn_type != VREG) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: not VREG\n"));
		u.u_error = EINVAL;
		ufdrele(fd);
		return( -1 );
	}

	/*
	 * find the number of segments needed to map the file
	 */
	if (rc = fp_fstat(fp, &statbuf, STATSIZE, FP_SYS)) {
		BUGLPR(shmm_debug, BUGGID, ("fmapfile: fp_fstat failed\n"));
		u.u_error = rc;
		ufdrele(fd);
		return( -1 );
	}

	if ((uint_t)statbuf.st_size > (uint_t)(SEGSIZE*8)) {
		BUGLPR(shmm_debug, 1, ("bad file size\n"));
		u.u_error = EFBIG;
		ufdrele(fd);
		return(-1);
	}

	if (statbuf.st_size == 0)
		num_segs = 1;
	else
		num_segs = ((uint_t)statbuf.st_size + (SEGSIZE-1)) / SEGSIZE;


	/*
	 * Indicate to the underlying fs that this file is
	 * being mapped.
	 */
	if ( rc = fp_shmat(fp, *addr, flag, &handle) ) {
		BUGLPR(shmm_debug, BUGGID, ("ERROR: fp_shmat failed\n"));
		ufdrele(fd);
		u.u_error = rc;
		return( -1 );
	}

	/*
	 * Save the key bit, number of segments, and segment id
	 * in the spare space in the ufd table where the fshmat
	 * fast-SVC can get to it.
	 */
	key = (flag & SHM_RDONLY) ? 1 : 0;
	
	U.U_ufd[fd].flags |= FSHMAT_VAL(key,num_segs,handle);

	/*
	 * Set flags indicating that this fd is mapped and that
	 * it is for fshmat.  This allows us to perform the
	 * correct cleanup when the file is closed via rmseg.
	 */
	U.U_ufd[fd].flags |= (UF_MAPPED | UF_FSHMAT);

	/*
	 * Mark as allocated all segment registers that could
	 * be used for mapping (3-12).  This prevents conflicts
	 * with any future segment register allocations.
	 */
	adsp = getadsp();
	for (sreg=SHMLOSEG; sreg<=SHMHISEG; sreg++)
		as_ralloc(adsp, sreg*SEGSIZE);

	/* Decrement the file descriptor count */
	ufdrele(fd);
	return( 0 );
}

/*
 * NAME: mapfork
 *
 * FUNCTION: This routine is called while processing a fork system call.
 *	A forked process inherits the mapped file segments from
 *	its parent, so the counters must be incremented.
 *
 * EXECUTION ENVIRONMENT:
 *	called from: shmfork
 *
 * RETURNS: NONE
 */

void
mapfork(seg)
uint_t seg;				/* segment number file is mapped to */
{
	struct segstate *segstate;	/* process segment state struct */
	struct file *fp;		/* file pointer */
	int rv;
	int flags;
	vmhandle_t handle;

	BUGLPR(shmm_debug, BUGNTA, ("mapfork seg=%X\n", seg));

	segstate = &U.U_segst[seg];

	flags = (segstate->segflag & SEG_MRDWR) ? 0 : SHM_RDONLY;

	if (segstate->segflag & SEG_DEFER)
		flags |= SHM_COPY;

	fp = U.U_ufd[segstate->segfileno].fp;
	rv = fp_shmat(fp, seg << SEGSHIFT, flags, &handle);
	ASSERT( rv == 0 );
	return;

}

/*
 * NAME: fmapfork
 *
 * FUNCTION: This routine is called while processing a fork system call.
 *	A forked process inherits the files mapped for use by fshmat by
 *	its parent, so the counters must be incremented.
 *
 * EXECUTION ENVIRONMENT:
 *	called from: fs_fork
 *
 * RETURNS: NONE
 */

void
fmapfork(fd)
int	fd;			/* file descriptor	*/
{
	struct file *fp;		/* file pointer */
	int rv;
	int flags;
	vmhandle_t handle;

	BUGLPR(shmm_debug, BUGNTA, ("fmapfork fd=%X\n", fd));

	flags = FSHMAT_KEY(U.U_ufd[fd].flags) ? SHM_RDONLY : 0;

	fp = U.U_ufd[fd].fp;
	rv = fp_shmat(fp, NULL, flags, &handle);
	ASSERT( rv == 0 );
	return;

}

/*
 * NAME: mmapfork
 *
 * FUNCTION: This routine is called while processing a fork system call.
 *	For each mapping segment in the parent, the child needs its own
 *	mapping segment.
 *
 * EXECUTION ENVIRONMENT:
 *	called from shmfork while running under parent process
 *	which is why a pointer to the child's address space must be
 *	passed as a parameter
 *
 * RETURNS: NONE
 */

void
mmapfork(uchild, seg)
struct user *uchild;			/* pointer to child's user area */
uint_t seg;				/* segment number file is mapped to */
{
	int rc, msid;

	BUGLPR(shmm_debug, BUGNTA, ("mmapfork seg=%X\n", seg));

	/* create new mapping segment.
	 */
	if ((rc = vms_create(&msid,V_MAPPING,0,0,0,0)) != 0)
	{
		/* if segment creation fails, detach existing segment.
		 * child will get a signal when trying to reference
		 * mappings for this segment and won't cause aliasing
		 * problems by using parent's mapping sid
		 */
		as_det(&(uchild->U_adspace), seg << SEGSHIFT);
		return;
	}

	/* set mapping sid in child's address space for sreg.
	 * note that sreg is already marked as allocated in
	 * child's adspace since it was allocated in parent's
	 */
	as_seth(&(uchild->U_adspace), vm_setkey(msid,1), seg << SEGSHIFT);
	return;
}

/*
 * NAME: rmseg
 *
 * FUNCTION: This routine calls rmseg for all current process mapped
 *	segments to free the segment register, and decrement the appropriate
 *	mapped segment counter.
 *
 * EXECUTION ENVIRONMENT:
 *	Called by close
 *
 * RETURNS: NONE
 */

void
rmseg(fd)
register int fd;
{
	int sreg;

	BUGLPR(shmm_debug, BUGNTA, ("rmmseg fd= %d\n", fd));

	/*
	 * perform different clean-up depending on mapping type.
	 */
	if (U.U_ufd[fd].flags & UF_FSHMAT)
	{
		/*
		 * mapped for fshmat
		 */
		rmfmap(fd);
	}
	else
	{
		/*
		 * normal shmat
		 */
		sreg = fdtoseg(fd, SEG_MAPPED);
		ASSERT(sreg != -1 );
		rmmseg(sreg);
	}

}

/*
 * NAME: rmmseg
 *
 * FUNCTION: This routine frees the segment associated with the specified
 *	segment register, decrements the appropriate mapped segment
 *	counter.
 *
 * EXECUTION ENVIRONMENT:
 *	called from shmex ishmdt
 *
 * RETURNS: NONE
 */
  
void
rmmseg(sreg)
uint_t sreg;
{
	struct segstate		*segstate;
	struct file		*fp;
	int			flag;
	int			i;
	int			num_segs;

	ASSERT( sreg >= SHMLOSEG && sreg <= SHMHISEG );
	segstate = &U.U_segst[sreg];
	ASSERT( segstate->segflag & SEG_MAPPED );
	ASSERT( segstate->segfileno < OPEN_MAX );
	ASSERT( U.U_ufd[segstate->segfileno].flags & UF_MAPPED );
	ASSERT( segstate->num_segs >= 1 && segstate->num_segs <= 8 );

	flag = (segstate->segflag & SEG_MRDWR) ? 0 : SHM_RDONLY;
	if (segstate->segflag & SEG_DEFER)
		flag |= SHM_COPY;

	U.U_ufd[segstate->segfileno].flags &= ~UF_MAPPED;
	fp = U.U_ufd[ segstate->segfileno].fp;
	u.u_error = fp_shmdt(fp, flag);

	num_segs = segstate->num_segs;
	for (i = 0 ; i < num_segs ; i++) {
		segstate->segflag = SEG_AVAIL;
		segstate->segfileno = 0;
		segstate->num_segs = 0;
		segstate++;
	}

	shm_freespace(sreg << SEGSHIFT, num_segs);
}

/*
 * NAME: rmfmap
 *
 * FUNCTION: This routine clears the mapped file information from the given	
 *	ufd table entry and decrements the appropriate mapped segment
 *	counters.
 *
 * EXECUTION ENVIRONMENT:
 *	called from rmseg
 *
 * RETURNS: NONE
 */
  
void
rmfmap(fd)
int	fd;			/* file descriptor	*/
{
	struct file		*fp;
	int			flag;
	int			i;
	int			num_segs;
	int			sid;

	/*
	 * remove fmap data from file descriptor entry 
	 * and decrement mapped segment counters.
	 */
	flag = FSHMAT_KEY(U.U_ufd[fd].flags) ? SHM_RDONLY : 0;
	sid = FSHMAT_SID(U.U_ufd[fd].flags);

	U.U_ufd[fd].flags &= 0x000000ff;
	U.U_ufd[fd].flags &= ~(UF_MAPPED | UF_FSHMAT);
	fp = U.U_ufd[fd].fp;
	u.u_error = fp_shmdt(fp, flag);

	/*
	 * remove any current mapping of this file from
	 * the process address space (invalidate any sreg
	 * referring to this file if the file is not mapped
	 * using some other fd).
	 */
	for (i=0; i<U.U_maxofile; i++)
		if ((U.U_ufd[i].fp != NULL) &&
		    (FSHMAT_SID(U.U_ufd[i].flags) == sid))
			break;
	if (i == U.U_maxofile)
	{
                adspace_t *adsp;
                adsp = &(U.U_adspace);
                for (i=SHMLOSEG; i<=SHMHISEG; i++)
                        if ((adsp->srval[i] & 0xfffff) == sid)
                        {
                                adsp->srval[i] = NULLSEGVAL;
                        }
	}

	/* SID shootdown must happen before we return.
	 */
}

/*
 * NAME: rmmapseg
 *
 * FUNCTION: This routine frees the segment associated with the specified
 *	segment register: frees corresponding mapping segment, segstate
 *	entry, and adspace entry.
 *
 * EXECUTION ENVIRONMENT:
 *	called from shmex
 *
 * RETURNS: NONE
 */
  
void
rmmapseg(sreg)
uint_t sreg;
{
	struct segstate		*segstate;
	adspace_t		*adsp;
	vmhandle_t		handle;
	int			rc;

	ASSERT( sreg >= SHMLOSEG && sreg <= SHMHISEG );
	segstate = &U.U_segst[sreg];
	ASSERT( segstate->segflag & SEG_MMAP );
	ASSERT( segstate->num_segs == 1 );

	/* free the segstate entry
	 */
	segstate->segflag = SEG_AVAIL;
	segstate->segfileno = 0;
	segstate->num_segs = 0;

	/* Get sreg value before freeing it.
	 * The segment register's value is protected by the adspace lock.
	 */
	adsp = getadsp();
	handle = as_getsrval(adsp, sreg << SEGSHIFT);

	/* free the sreg in the address space
	 */
	shm_freespace(sreg << SEGSHIFT, 1);

	/* delete the mapping segment after addressability has been removed
	 */
	rc = vms_delete(SRTOSID(handle));

	return;
}

/*
 * NAME: fdtoseg
 *
 * FUNCTION: This routine searches the the users segments' segment state
 * 	structure for a segment which satisfies the specified file descriptor
 * 	and flag (&) parameters, and returns the segment register number
 * 	if it is found.
 *
 * EXECUTION ENVIRONMENT:
 *	Caller must be a process
 *	Called from mapfile
 *
 * RETURNS:
 *	segment register number if found
 *	-1 if not found
 */

int
fdtoseg(fd, flag)
int fd, flag;
{
	struct segstate *segstate;
	uint_t seg;

	BUGLPR(shmm_debug, BUGNTA, ("fdtoseg fd=%d flag=%x\n", fd, flag));

	seg = SHMLOSEG;
	do {
		segstate = &U.U_segst[seg];	
		if ((segstate->segflag & flag) &&
				 (segstate->segfileno == fd))
			return(seg);
		seg += (segstate->segflag == SEG_AVAIL)?1:segstate->num_segs;
	} while (seg <= SHMHISEG);

	ASSERT(seg == SHMHISEG + 1);
	return(-1);

}






