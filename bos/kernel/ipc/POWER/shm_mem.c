static char sccsid[] = "@(#)05	1.28  src/bos/kernel/ipc/POWER/shm_mem.c, sysipc, bos411, 9428A410j 6/10/94 17:45:05";

/*
 * COMPONENT_NAME: (SYSIPC) IPC shared memeory services
 *
 * FUNCTIONS: shm_ldseg shm_findspace shm_getseg shm_dseg
 *	      vm_map_ldseg, vm_map_findspace, disclaim,      
 *            disclaimx
 *
 * ORIGINS: 27 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/adspace.h>
#include <sys/user.h>
#include <sys/seg.h>
#include <sys/syspest.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include <sys/pseg.h>
#include <sys/vmuser.h>
#include <sys/lock.h>
#include <vmm/vmdefs.h>

BUGXDEF(shm_debug);
BUGXDEF(shmm_debug);
BUGVDEF(disclaimdbg, 0);

/*
 * NAME: shm_ldseg
 *
 * FUNCTION: load a segment in users address space
 *
 * EXECUTION ENVIRONMENT:
 *	Must be a process
 *
 * RETURNS: NONE
 */

void
shm_ldseg(addr, handle, flags, num_segs )
uint addr;			/* address to attach to		*/
vmhandle_t handle;
int flags;			/* segment protection flag	*/
int num_segs;			/* number of segments to load	*/
{
	adspace_t *adsp;
	uint_t sid;
	int key;
	int i;
	int rc;

	BUGLPR(shm_debug,BUGNTA,("shm_ldseg: addr=%X handle=%X flags=%X \n", \
		addr, handle, flags));
	ASSERT(num_segs == 1 || flags & SHM_MAP);

	adsp = getadsp();
	/*
	 * if this is a read only segment set the protection key
	 */
	key = (flags & SHM_RDONLY) ? 1 : 0;

	if (flags & SHM_MAP) {
		sid = SRTOSID(handle);
		for ( i = 0 ; i < num_segs ; i++) {
			rc = as_ralloc(adsp, addr);
			ASSERT( rc == 0 );
			as_seth(adsp, SRVAL(sid, key, 0), addr);
			sid += NEXTSID;
			addr += SEGSIZE;
		}
	} else {
		handle = vm_setkey(handle, key);
		rc = as_ralloc(adsp, addr);
		ASSERT( rc == 0 );
		as_seth(adsp, handle, addr);
	}
}

/*
 * NAME: shm_findspace
 *
 * FUNCTION: checks users address space to see if a segment
 *	register can be allocated.
 *
 * EXECUTION ENVIRONMENT:
 *	called from shmat, mapfile
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS: 0 if successful	-1 if not successful
 */
int
shm_findspace(addr , num_segs)
caddr_t *addr;			/* shmat address		*/
int num_segs;			/* number of consecutive segs	*/
{
	adspace_t *asp;
	ulong_t alloc;
	ulong_t alloc_mask;
	uint_t seg;

	BUGLPR(shm_debug, BUGNTA, ("shm_findspace: addr = %X\n", *addr));
	ASSERT( NULL != addr );
	ASSERT( num_segs > 0 && num_segs <= 8);

	asp = getadsp();
	alloc = asp->alloc;
	alloc_mask = (ulong_t)(((long)UZBIT) >> (num_segs - 1));

	if (*addr == 0) {
		alloc_mask >>= SHMLOSEG;
		for (seg = SHMLOSEG ; seg <= SHMHISEG ; seg++) {
			if ((alloc_mask & alloc) == 0)
				break;
			alloc_mask >>= 1;
		}
		if ((seg + num_segs - 1) > SHMHISEG) {
			BUGLPR(shm_debug, BUGGID, \
				("shm_findspace: no segregs\n"));
			u.u_error = EMFILE;
			return(-1);
		}
		*addr = (caddr_t)(seg << SEGSHIFT);
	} else {
		seg = (uint_t)*addr >> SEGSHIFT;
		alloc_mask >>= seg;
		if ( (uint_t)(*addr) & (SHMLBA-1)	||
				(seg < SHMLOSEG)	||
				(seg + num_segs - 1 > SHMHISEG) ) {
			BUGLPR(shm_debug, BUGGID, \
				("shm_findspace: invalid address = %X\n", \
				*addr) );
			u.u_error = EINVAL;
			return( -1 );
		}

		if ((alloc_mask & alloc) != 0) {
			u.u_error = EINVAL;
			return( -1 );
		}

	}

	ASSERT((seg >= SHMLOSEG) && (seg + num_segs - 1 <= SHMHISEG));
	return( seg );

}

/*
 * NAME: vm_map_ldseg
 *
 * FUNCTION: load segment(s) in users address space for use by mmap()
 *
 * EXECUTION ENVIRONMENT:
 *	must be a process
 *	called from vm_map_enter
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: NONE
 */
void
vm_map_ldseg(start, end)
caddr_t	start;		/* start address of range */
caddr_t	end;		/* end address of range */
{
	adspace_t *asp;
	struct segstate	*segstate;
	uint_t	seg;
	int	sid;
	caddr_t	sstart;
	int 	rc;

	asp = getadsp();
	seg = (uint_t)start >> SEGSHIFT;
	ASSERT((seg >= SHMLOSEG) && (seg <= SHMHISEG));
	segstate = &U.U_segst[seg];
	sstart = (caddr_t)((uint_t)start & ~SOFFSET);

	/* ensure that segments are allocated for mmap() for
	 * the range [start, end).
	 */
	for ( ; sstart < end; sstart += SEGSIZE) {

		/* if segment is not already set up for mmap()
		 */
		if (segstate->segflag != SEG_MMAP)
                   {   
		    ASSERT(segstate->segflag == SEG_AVAIL);

		    /* allocate mapping segment
		     * it is unlikely that this will fail but if it does
		     * just give up -- references to the mappings will
		     * cause the process to be signalled
		     */
		    if ((rc = vms_create(&sid,V_MAPPING,0,0,0,0)) != 0)
			    return;

		    /* allocate sreg in adspace
		     */
		    rc = as_ralloc(asp, sstart);
		    ASSERT(rc == 0);
		    as_seth(asp, SRVAL(sid, 1, 0), sstart);

		    /* initialize segstate structure to indicate
		     * the segment is allocated for mmap().
		     */
		    segstate->segflag = SEG_MMAP;
		    segstate->num_segs = 1;
		   }
		segstate++;
	}

	return;
}

/*
 * NAME: vm_map_findspace
 *
 * FUNCTION: checks users address space to see if a segment
 *	register can be allocated for mmap()
 *
 * EXECUTION ENVIRONMENT:
 *	called from vm_map_enter
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 if successful
 *	   -1 if not successful and sets start to suggested new search address
 */
int
vm_map_findspace(start, end)
caddr_t	*start;
caddr_t end;
{
	adspace_t *asp;
	ulong_t	alloc;
	ulong_t	alloc_mask;
	struct	segstate *segstate;
	uint_t	seg;
	caddr_t	sstart;

	/* get current segment register allocation status
	 * and create mask with bit for starting sreg.
	 */
	asp = getadsp();
	alloc = asp->alloc;
	seg = (uint_t)*start >> SEGSHIFT;
	ASSERT((seg >= SHMLOSEG) && (seg <= SHMHISEG));
	alloc_mask = (ulong_t) (UZBIT >> seg);
	sstart = (caddr_t)((uint_t)*start & ~SOFFSET);
	segstate = &U.U_segst[seg];

	/* for each segment covered by range, make sure it is
	 * available for allocation or is already allocated
	 * for use by mmap().
	 */
	for ( ; sstart < end; sstart += SEGSIZE) {
		if ((alloc_mask & alloc) != 0 &&
		    (segstate->segflag != SEG_MMAP))
			break;
		alloc_mask >>= 1;
		segstate++;
	}

	/* if range not available for mmap() indicate
	 * failure and suggest new search start address
	 */
	if (sstart < end) {
		BUGLPR(shm_debug, BUGGID, \
			("vm_map_findspace: segreg not available\n"));
		*start = sstart + SEGSIZE;
		return (-1);
	}

	return (0);
}

/*
 * NAME: shm_freespace
 *
 * FUNCTION: free consecutive segment register(s)
 *
 * EXECUTIN ENVIRNOMENT:
 *	called by rmmseg
 *
 * RETURNS: NONE
 */
void
shm_freespace(addr, num_segs)
uint_t addr;
int num_segs;
{
	int i;
	adspace_t *asp;

	ASSERT((addr & (SEGSIZE-1)) == 0);
	ASSERT(num_segs > 0 && num_segs <= 8);

	asp = getadsp();
	for (i = 0 ; i < num_segs ; i++ ) {
		as_det(asp, addr);
		addr += SEGSIZE;
	}

	return;
}

/*
 * NAME: shm_getseg
 *
 * FUNCTION: create a shared memory segment
 *
 * EXECUTION ENVIRONMENT:
 *	called by shmget
 *
 * RETURNS: 0 if successful	-1 if not successful
 */

int
shm_getseg( sp )
register struct shmid_ds *sp;
{
	int sid;		/* segment id returned by vmscreate */
	int rv;
	int vtype;

	ASSERT( NULL != sp );
	BUGLPR(shm_debug, BUGNTA, ("shm_getseg: sp=%X\n", sp));

        /* If this process requires early paging space allocation,
        * mark the segment.
        */
        vtype = V_WORKING | V_UREAD;
        if (curproc->p_flag & SPSEARLYALLOC)
                vtype |= V_PSEARLYALLOC;

	/*
	 * shared segments are read only to user with key 1
	 */
	if( rv = vms_create(&sid, vtype, 0, sp->shm_segsz,
				 sp->shm_segsz, 0) )
	{
		BUGPR(("shm: vms_create failed, rv = %d\n", rv));
		return( -1 );
	}

	sp->shm_handle = SRVAL(sid,0,0);
	return( 0 );
}

/*
 * NAME: shm_dseg
 *
 * FUNCTION: delete a shared memory segment
 *
 * EXECUTION ENVIRONMENT:
 *	called by shmctl
 *
 * RETURNS: 0 if successful		-1 if not successful
 */

int
shm_dseg(sp)
struct shmid_ds *sp;
{
	register int rv;
	int segid;

	/*
	 * call vmm to delete segment
	 */
	segid = SRTOSID(sp->shm_handle);
	if ((rv = vms_delete(segid)) != 0) {
		BUGPR((("segoff.c: vms_delete(%d) returned %d\n", segid, rv)))
		return( -1 );
	}

	return( 0 );
}

/*
 * NAME: shm_growseg
 *
 * FUNCTION: change size of shared memory segment
 *
 * EXECUTION ENVIRONMENT:
 *	called by shmctl
 *
 * RETURNS: 0 if successful		-1 if not successful
 */

int
shm_growseg(sp, seg_size)
struct shmid_ds *sp;
uint_t seg_size;
{
	int	rv;
	uint_t	segid;

	/*
	 * call vmm to grow segment
	 */
	segid = SRTOSID(sp->shm_handle);
	if ((rv = vms_limits(segid, seg_size, 0)) != 0 ){
		BUGPR(("vms_limits failed rv=%d\n", rv));
		return( -1 );
	}

	return( 0 );

}
/* 
 * NAME: disclaim
 *
 * FUNCTION: disclaim contents of memory address range
 *
 * NOTES:
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: sets u.u_error
 *
 * RETURNS: 0 if successful, -1 if not successful
 */ 

int disclaim(char *addr,unsigned len,unsigned flag)
{
	uint_t chunk;  

	if (flag != ZERO_MEM) {
		BUGLPR(disclaimdbg, 1 , ("disclaim: bad flags\n"));
		u.u_error = EINVAL;
		return( -1 );
	}

	/* disclaim segment by segment */
	do {
		/* get the chunk that is in a whole segment */
		chunk = MIN(len , SEGSIZE -((uint_t)addr & SOFFSET));

		u.u_error = disclaimx(addr,chunk); 
		len -= chunk;
		addr += chunk;
	} while (len !=0 && u.u_error == 0);

	return (u.u_error ? -1 :0);
}

/* 
 * NAME: disclaimx
 *
 * FUNCTION: disclaims the contents of a memory range
 *      that does not straddle a segment
 *      Makes sure that the segment
 *	is not a mapped file.  It checks for write access permission
 *	to the address range.  If O.K., then vm_release is called to discard
 *	the pages.  Failure should not cause a panic, instead the
 *	call should fail with EFAULT.  If the call is successful, 
 *	any bytes of the indicated range forming a partial page
 *	are then cleared
 *
 *
 * NOTES:
 * RETURNS: 0 if successful	errno	if not successful
 */ 

int
disclaimx(addr,len)
char 		*addr;
unsigned int	len;
{
	uint_t segno;		/* 4 high order bits of addr		*/
	vmhandle_t srval;	/* segment reg value			*/
	uint_t limit;		/* maximum address for segment		*/
	uint_t saddr;		/* start address for vm_release		*/
	uint_t slen;		/* length for vm_release		*/
	struct segstate *sp;	/* pointer to segment state for addr	*/
	struct adspace_t *asp;	/* pointer to users address space	*/
	label_t jmp_buf;	/* exception buffer			*/
	int isawsseg = 0;	/* working segment flag			*/
	int rc =0;

	BUGLPR(disclaimdbg, 1 , ("disclaim: addr=%X len=%X\n",	\
		addr, len));

		
	asp = getadsp();

	segno = (uint_t)addr >> SEGSHIFT;

	srval = as_geth(asp, addr);

	if ( segno == PRIVSEG ) {
		limit = SEGOFFSET(USTACK_TOP);
		srval = vm_setkey(srval, 0);
		isawsseg = 1;
	} else if ((sp = &U.U_segst[segno])->segflag & SEG_WORKING) {
		limit = U.U_dsize - SEGSIZE;
		srval = vm_setkey(srval, 0);
		isawsseg = 1;
	} else {
		/*
	 	 * must be a shared memory segment
	 	 */
		if (!(sp->segflag & SEG_SHARED)) {
			rc = ((sp->segflag & SEG_MAPPED) ||
				     (sp->segflag & SEG_MMAP)) ? 
							EINVAL : EFAULT;
			BUGLPR(disclaimdbg, 1, ("disclaim: bad seg type\n"));
			as_puth(asp,srval);
			return( rc);
		}

		/*
		 * check that the user has write access
		 */

		if ( vm_getkey(srval) ) {
			BUGLPR(disclaimdbg, 1, ("disclaim: seg read only\n"));
			as_puth(asp,srval);
			return(EFAULT );
		}

		/*
		 * get the size limit for this segment
		 */
		limit = sp->shm_ptr->shm_segsz;

	}

	/*
	 * check that the address range is addressable by the user
	 */
	if ( limit < (SEGOFFSET(addr) + len) ) { 
		BUGLPR(disclaimdbg, 1, ("disclaim: seg limit exceded\n"));
		as_puth(asp,srval);
		return(EFAULT);
	}


	if (setjmpx(&jmp_buf)) {
		BUGLPR(disclaimdbg, 1, ("disclaim: exception received\n"));
		as_puth(asp,srval);
		return(EFAULT);
	}

	/*
	 * attach to the shared memory segment
	 */
	addr = vm_att(srval, addr);

	/*
	 * if there are not full pages to clear;
	 * or the disclaim is in the process private
	 * segment and it is plockd then zero the
	 * range and return
	 */
	if (len < PAGESIZE ||
	    isawsseg && (U.U_lock & (PROCLOCK | DATLOCK))) {
		bzero(addr, len);
		goto exit;
	}

	/*
	 * check if the address range's start contains a partial page
	 * if it does adjust the start address and length for vm_release
	 * to begin at page boundary
	 */
	if ( (uint_t)addr & (PAGESIZE-1) ) {
		saddr = ((uint_t)addr & ~(PAGESIZE-1)) + PAGESIZE;
		slen = len - (saddr - (uint_t)addr);
	} else {
		saddr = (uint_t)addr;
		slen = len;
	}

	/*
	 * if the address range ends with a partial page page then
	 * reduce the slen to not include the partial page
	 */
	if ( ((uint_t)addr + len) & (PAGESIZE-1) )
		slen = slen - ( ((uint_t)addr + len) & (PAGESIZE-1) );

	/*
	 * call vm_release to release page frames.  This also sets the
	 * pages to logically zero
	 */

	BUGLPR(disclaimdbg, 1, ("disclaim: calling vm_release: %X %X\n", \
		saddr, slen));
	if (rc = vm_release(saddr, slen)) {
		BUGLPR(disclaimdbg, 1, ("disclaim: vm_release rc =%d\n", \
			rc));
		rc = EFAULT;
		goto exit;
	}

	/*
	 * clear partial page at end of address range
	 */
	bzero( ((uint_t)addr + len) & ~(PAGESIZE-1),
				 ((uint_t)addr+len) & (PAGESIZE-1) );

	/*
	 * clear partial page at start of address range if it exists
	 */
	if ( (uint_t)addr != saddr )
		bzero( addr, PAGESIZE - ((uint_t)addr & (PAGESIZE-1)) );

exit:
	vm_det(addr);
	as_puth(asp,srval);
	clrjmpx(&jmp_buf);
	return rc;
}
