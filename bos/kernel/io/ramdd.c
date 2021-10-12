static char sccsid[] = "@(#)98  1.16.1.9  src/bos/kernel/io/ramdd.c, sysio, bos412, 9446B 11/9/94 07:01:16";
/*
 * COMPONENT_NAME: SYSIO
 *
 * FUNCTIONS: Read and write blocks to compressed ram image
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * NOTE:
 *	The boot RAM disk is created by the Virtual
 *	Memory Manager at IPL time. All other RAM
 *	disks are created at the user's request.
 *
 *	Only four RAM disks are supported.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/vmker.h>
#include <sys/vmuser.h>
#include <sys/xmem.h>
#include <sys/adspace.h>
#include <sys/devinfo.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/syspest.h>
#include <sys/ramdd.h>
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <vmdefs.h>
#include <vmdisk.h>
#include <vmmacs.h>

#define NUMRAMDISKS 10
#define BOOTRAM	    0

BUGVDEF(ramdd_dbg, 0);

extern int init_obj_end, pg_obj_end, pg_com_end, endcomm;

/*
 *  Virtual memory handles for various ramdisks.
 */
uint	ramsrval[NUMRAMDISKS];
int	ramcompflag = 0;
char	*ram_buf = NULL;
int	ramopen = 0;
int	initpinned = 0;
int	ramsize;

lock_t ramddlk = LOCK_AVAIL;
static int ramd_uc_size = 0;	/* RAM disk size, when uncompressed */

/*
 * NAME: ram_disk_size
 *
 * FUNCTION: called by vmsi() to determine the number of bytes required
 *	for the ram disk
 *
 * NOTES:
 *	vmker.nrpages and vmker.badpages must have been initialized
 *	before calling this function
 *
 * EXECUTION ENVIRONMENT:
 *	Called durring vmsi() and ram_open()
 *
 * RETURNS:
 *	The number of bytes required for ram disk
 */
int
ram_disk_size(
	struct ramimage *rd)
{
	extern int ram_disk_end, ram_disk_start;

	/* Check for a compressed RAM disk image
	 */
	if (!strncmp(rd->r_magic, RAM_MAGIC, sizeof(rd->r_magic)))
	{
		if (ram_low_mem())
		{
			return((rd->r_blocks / 2) * PSIZE + DATA_START(rd));
		}
		else
		{
			return(rd->r_blocks * PSIZE + DATA_START(rd));
		}
	}
	else
	{
		/* This is not a compressed ram disk.  The initial size of
		 * disk image is contained in low memory variables. The
		 * current size of ram disk is kept in ramd_uc_size when ram
		 * disk is uncompressed. The value of ram_disk_end is not
		 * kept up to date because it resides in a read-only page.
		 */
		if (ramd_uc_size == 0)
			ramd_uc_size = ram_disk_end - ram_disk_start;
		return(ramd_uc_size);
	}
}

/*
 * NAME: ram_open
 *
 * FUNCTION:
 *	If the RAM disk does not already exist, create it.
 *
 * EXECUTION ENVIRONMENT:  
 *	Runs under a process. May page fault.
 *	
 * RETURN VALUE:
 * 	On error, returns appropriate error flag.
 *	Return 0 on successful completion.
 *
 * EXTERNAL PROCEDURES CALLED:  minor, vms_create, SRVAL
 */

int
ram_open( dev_no, rwflag, ext )
dev_t	dev_no;				/* Device number		*/
int	rwflag;				/* Access requested		*/
int	ext;				/* Extended system call parm	*/
{
	int	rc;			/* Return code 			*/
	dev_t	minor_dev;		/* Minor device number		*/
	uint	sid;			/* Address id for create call	*/
	struct	ramimage *rd;		/* ram disk image		*/


	BUGLPR(ramdd_dbg, 10, ("ram_open: %x %x %x\n", dev_no, rwflag,
			ext));

	minor_dev=minor(dev_no);

	/*
	 *  Validate minor device number	
	 */
	if ( minor_dev >= NUMRAMDISKS )
	{
		BUGLPR(ramdd_dbg, 10, ("ram_open: bad minor number\n"));
		return( ENXIO );
	}

	/* Pin the initobj so it does not go away */
	if (initpinned == 0) 
	{
		pin (&pg_obj_end, (int)&init_obj_end - (int)&pg_obj_end-1);
		pin (&pg_com_end, (int)&endcomm - (int)&pg_com_end-1);
		initpinned = 1;
	}
	
	/*
	 *  If not the boot RAM disk.  the second condition tested is 
	 *  paranoia.  if ram is unmounted as root and re-mounted as
	 *  root the segment will have been deleted.
	 */
	if ( minor_dev != BOOTRAM || vmker.ramdsrval == 0)
	{
		BUGLPR(ramdd_dbg, 10, ("ram_open: creating new seg\n"));
		if (minor_dev == BOOTRAM)
			return(EINVAL);

		/*
		 *   See if RAM disk exists for this device.
		 */
		if (ramsrval[minor_dev] == 0)
		{
			/* 
			 *  If not, create the RAM disk.
			 */
			rc = vms_create(&sid,V_WORKING,NULL,SEGSIZE,SEGSIZE,0);
			if ( rc != 0 )
			   return ( ENOMEM );
			ramsrval[minor_dev] = SRVAL(sid,0,0);
		}
	}
	else {
		lockl(&ramddlk, LOCK_SHORT);
		ramsrval[minor_dev] = vmker.ramdsrval;
		rd = (struct ramimage *)vm_att(vmker.ramdsrval, 0);

		/* get a buffer to be used for read/write transfers
		 */
		if (ramopen == 0)
		{
			ram_buf = xmalloc(PSIZE, 0, pinned_heap);
			assert(ram_buf != NULL);
			ramopen = 1;
		}

		/*
		 * check magic number of ram image to see if it is a
		 * compressed ram image
		 */
		if (!strncmp(rd->r_magic, RAM_MAGIC, sizeof(rd->r_magic))) {
			BUGLPR(ramdd_dbg, 10, ("ram_open: packed image\n"));

			/*
			 * If the system is low on memory then reserve half
			 * the ram disk size and set flag to compress blocks
			 * on write.
			 */
			if (ram_low_mem()) {
				rd->r_flag = RAM_WPACK;
			} else {
				rd->r_flag = RAM_WCOPY;
			}

			ramsize = ram_disk_size(rd);

			/*
			 * serialize the initialization of compressed ram
			 * disk segment and data structures
			 */
			if (ramcompflag == 0) {
				rd->r_segsize = ramsize;
				rc = ram_comp_init();
				assert(rc == 0);
				ramcompflag = 1;
			}

		} else {
			BUGLPR(ramdd_dbg, 10,("ram_open: image not packed\n"));
			ramcompflag = 0;
		}
		vm_det(rd);
		unlockl(&ramddlk);
	}
	
	BUGLPR(ramdd_dbg, 10, ("ram_open: returning\n"));
	return( 0 );

}
/*
 * NAME: ram_close
 *
 * FUNCTION:
 *	Not much gets done here.
 *
 * EXECUTION ENVIRONMENT:  
 *	Runs under a process. May page fault.	
 *
 * RETURN VALUE:
 * 	On error, return error flag.
 *	Return 0 on successful completion.
 *
 * EXTERNAL PROCEDURES CALLED:  minor
 */

int
ram_close( dev_no, offchan, ext )
dev_t	dev_no;			/* Device number		*/
int	offchan;		/* Channel id			*/
int	ext;			/* Extended system call parm	*/
{
	dev_t	minor_dev;	/* Minor device number		*/
	struct ramimage *rd;	/* ram disk image */
	int rc;

	BUGLPR(ramdd_dbg, 10, ("ram_close: %x %x %x\n", dev_no, offchan,
			ext));
	minor_dev=minor(dev_no);

       /*
	*  Validate the minor number.
	*/
	if ( minor_dev >= NUMRAMDISKS )		
	{
		BUGLPR(ramdd_dbg, 10, ("ram_close: bad minor number\n"));
		return( ENXIO );
	}

	if (minor_dev == BOOTRAM)
	{
		lockl(&ramddlk, LOCK_SHORT);

		/* on final close of a compressed file system free compression
		 * data structures
		 */
		if (ramcompflag)
		{
			ramcompflag = 0;
			ram_comp_free();
		}

		/* on final close of boot file system free copy buffer
		 */
		if (ramopen)
		{
			ramopen = 0;
			xmfree(ram_buf, pinned_heap);
			ram_buf == NULL;

			if (initpinned != 0) 
			{
				unpin(&pg_obj_end,
				 (int)&init_obj_end-(int)&pg_obj_end-1);
				unpin(&pg_com_end,
				 (int)&endcomm - (int)&pg_com_end-1);
				initpinned = 0;
			}
		}
		unlockl(&ramddlk);
	}

	BUGLPR(ramdd_dbg, 10, ("ram_close: successful\n"));
	return(0);
}

/*
 * NAME: ram_strategy
 *
 * FUNCTION:
 * 	Validates input:
 * 		b_dev field = bootdev
 * 		b_blkno field is on RAM disk
 * 	Returns the appropriate error, set in b_error field of
 * 	the "buf" struct.
 *
 * 	If invoked correctly, and if access is read, this routine
 *	returns the data requested or the portion thereof contained on
 *	the RAM disk by copying the data to the identified block in
 *	b_un field of the "buf" structure.
 *
 *	If access is write, this routine copies the data to
 *	the identified location on RAM disk.
 *
 * EXECUTION ENVIRONMENT:  
 *	Can be called by either a process or an interrupt handler.
 *	Does NOT page fault.
 * NOTE:
 *	This is why ramdd cannot be used in a paging	
 *	file system.
 *
 * EXTERNAL PROCEDURES CALLED:  minor, xmemin, xmemout, vm_att, vm_det
 *
 */

extern	void	bcopy();		/* See above description	*/

ram_strategy( bufp )

struct	buf	*bufp;			/* Pointer to block I/O "buf" struct */
{
	int	rc;			/* Return code from xmem routines */
	uint	offset;			/* Offset into RAM disk to b_bklno */
	uint	ramlength;		/* length of ram disk		*/
	caddr_t	start_addr;		/* Address on RAM disk equivalent */
					/* to the requested b_blkno	*/
	dev_t	minor_dev;		/* Minor device number derived	*/
				   	/* from bufp -> b_dev dev num	*/
	struct	buf	*next;		/* Pointer to next I/O "buf" struct */
	caddr_t	ram_data;
	int	oldpri;
	int	block;

	oldpri = i_disable(INTPAGER);
	BUGLPR(ramdd_dbg, 10, ("ram_strategy: %x\n", bufp));

	/* support multiple buf structs */

	for (; bufp; bufp = next) {

		/* iodone() smashes av_forw
		 */
		next = bufp->av_forw;

		/* Derive the minor device number.
		 */
		minor_dev = minor( bufp->b_dev );

		/*
		 *  Verify the device (b_dev) valid
		 *  i.e., that minor_dev < NUMRAMDISKS
		 */
		if ( minor_dev >= NUMRAMDISKS )
		{
		       /*
			*  Return invalid device error. 
			*/
			bufp->b_error = ENXIO;
			bufp->b_flags |= B_ERROR;
			iodone (bufp);
			continue;
		 }

		/*
		 *   See if RAM disk exists for this device.
		 */
		if (ramsrval[minor_dev] == 0)
		{
			/* 
			 *  If not, return invalid device error.
			 */
			bufp->b_error = ENXIO;
			bufp->b_flags |= B_ERROR;
			iodone (bufp);
			continue;
		 }

		/*
		 *  Compute ramlength and offset.
		 */
		ramlength = SEGSIZE;
		offset = (bufp->b_blkno) * UBSIZE;
		bufp->b_resid = bufp->b_bcount;

		/*
		 *  Verify requested start address is on device.
		 *  i.e., starts before end of RAM disk.
		 */
		if ( offset >= ramlength )
		{
			if(  offset > ramlength )
			{
				/*
				* blkno is beyond the range
				* of the device.
				*/
				bufp->b_flags |= B_ERROR;
				bufp->b_error = EIO;
			}
			iodone (bufp);
			continue;
		}

		/*
		 *  Verify request ends before the end of the RAM disk.
		 *  If not, adjust b_bcount (i.e. truncate the request).
		 */

		if ( offset + (bufp->b_bcount) > ramlength )
			bufp->b_bcount -= (ramlength - bufp->b_bcount);

		/*
		 *  Setup addressibility to the RAM disk.
		 *  Returns the start_addr of the requested block on the RAM disk.
		 */

		ram_data = vm_att(ramsrval[minor_dev], 0);

		/*
		 *  Copy to/from ram disk segment from/to buffer cache.
		 */
		if ( (bufp->b_flags) & B_READ ) {

			if (ramcompflag && minor_dev == BOOTRAM) {

				/*
				 * Only support page page aligned request in
				 * the stategy routine
				 */
				assert((bufp->b_blkno % (PSIZE/UBSIZE)) == 0);
				assert((bufp->b_bcount == PSIZE) ||
						 (bufp->b_bcount == PSIZE/2));
				block = bufp->b_blkno/(PSIZE/UBSIZE);
				start_addr = ram_buf;
				cram_read(ram_data, block, ram_buf);
			} else {
				start_addr = ram_data + offset;
			}
			/*
			 *  I/O request = READ.
			 *  Transfer data from RAM disk to identified block,
			 *  b_un.
			 */
			rc = xmemout(start_addr,bufp->b_un.b_addr,bufp->b_bcount,
				&bufp->b_xmemd);
		} else {

			if (ramcompflag && minor_dev == BOOTRAM) {
				assert(bufp->b_bcount == PSIZE);
				start_addr = ram_buf;
			} else {
				start_addr = ram_data + offset;
			}

			/*
			 *  I/O request = WRITE.
			 *  Transfer data from identified block, b_un, to
			 *  RAM disk.
			 */
			rc = xmemin(bufp->b_un.b_addr,start_addr,
					bufp->b_bcount, &bufp->b_xmemd);

			if (rc == XMEM_SUCC && ramcompflag &&
					 minor_dev == BOOTRAM) {

				/*
				 * only support page aligned request for
				 * compressed ram image
				 */
				block = bufp->b_blkno / (PSIZE/UBSIZE);
				assert((bufp->b_blkno % (PSIZE/UBSIZE)) == 0);
				cram_write(ram_data, block,
							 start_addr);
			}

		}

		if ( rc != XMEM_SUCC )
		{
			bufp->b_bcount = 0;
			bufp->b_error = EFAULT;
			bufp->b_flags |= B_ERROR;
		}

		/*
		 *  Detach from the RAM disk.
		 */
		vm_det(ram_data);

		/*
		 *  Indicate I/O completed
		 */
		bufp->b_resid -= bufp->b_bcount;
		iodone(bufp);
	}

	BUGLPR(ramdd_dbg, 10, ("ram_stratgey: returning\n"));
	i_enable(oldpri);


} /* ... end ram_strategy */

/*
 * NAME: ram_ioctl
 *
 * FUNCTION:
 *	ioctl entry point
 *
 * EXECUTION ENVIRONMENT:  
 *	Runs under a process. May page fault.
 *	
 * RETURN VALUE:
 * 	On error, returns appropriate error flag.
 *	Return 0 on successful completion.
 *
 */

int ram_ioctl(devno, op, arg, devflag)
dev_t devno;
int op, arg;
ulong devflag;
{
	struct devinfo devinfo;
	struct ramimage *rd;
	dev_t	minor_dev;	
	unsigned int	last_page, pages, epages, ebytes;
	unsigned int    vmmsrsave,xptsrsave;
	extern int ram_disk_end, ram_disk_start;
	int rc;
	int p, plast, last, sid, sidx;
	int	oldpri;
	long	ngrpages;
	extern int update_rmap();

	rc = 0;
	rd = (struct ramimage *)vm_att(vmker.ramdsrval, 0);
	switch(op)
	{
/*
 *  IOCINFO - returns some information about the ram disk.  This   
 *      is a standard ioctl option that can be issued to find out 
 *      information about any device that uses ioctls.  A pointer to
 *      a structure of type devinfo should be passed in the 'arg'  
 *      parameter.  The information about the diskette will be loaded 
 *      into the devinfo structure. 
 */  
	case IOCINFO:
		devinfo.devtype = DD_PSEU;

		devinfo.flags = DF_RAND;

		if (!strncmp(rd->r_magic, RAM_MAGIC, sizeof(rd->r_magic)))
		   /* RAM disk is compressed */
		   devinfo.un.dk.numblks = rd->r_blocks * PSIZE/UBSIZE;
		else
		   devinfo.un.dk.numblks = ram_disk_size(rd)/UBSIZE; 
		/* copy the devinfo structure to the user or kernel */	
		if ( devflag & DKERNEL ) {
		    bcopy((char *)(&devinfo), (char *)(arg), sizeof(devinfo)); 
		} else {
		if (copyout((char *)(&devinfo), (char *)(arg), 
			sizeof(devinfo)) != 0)
			rc = EINVAL;
		}
		break;
/*
 *  IOCCONFIG - extends the RAM disk by the number of 512 byte blocks
 *              specified in arg.
 */
	case IOCCONFIG:
		minor_dev = minor(devno);
		if ( minor_dev != BOOTRAM ) {
			rc = EINVAL;
			break;
		}	
		lockl(&ramddlk, LOCK_SHORT); /* serialize IOCCONFIG */

		last_page = BTOPN(ram_disk_size(rd)); /* last page pinned */
		pages = (arg * UBSIZE + PSIZE - 1 )/PSIZE ; /* pages to extnd */
		ebytes = pages*PSIZE;   /* bytes to extend */

		/* determine effective pages to pin for compressed RAM */
		if (!strncmp(rd->r_magic, RAM_MAGIC, sizeof(rd->r_magic)) &&
			 ram_low_mem() ) {
			if ( ((pages>>1)<<1) != pages ) 
				pages++; 	 /* make it an even number */
			ebytes = pages/2*PSIZE + sizeof(struct ramblock)* pages;
		}

		/* pin (and allocate) new pages */
		if( (rc = ltpin( (caddr_t)((last_page+1)*PSIZE + (uint) rd),
				 ebytes )))	{
			unlockl(&ramddlk);
			break;	
		}

		/* update the ram disk size information */
		if (!strncmp(rd->r_magic, RAM_MAGIC, sizeof(rd->r_magic)))
			update_rmap( rd, pages);  /* compressed */
		else
			ramd_uc_size += (pages * PSIZE); /* uncompressed */

		unlockl(&ramddlk);
		break;
/*
 *  RIOCSYSMEM - returns the number of good real pages in the system
 */
	case RIOCSYSMEM:
		ngrpages = (long)(vmker.nrpages - vmker.badpages);
		
		/* copy the output to the user or kernel */	
		if ( devflag & DKERNEL ) {
		    bcopy((char *)(&ngrpages), (char *)(arg), sizeof(long)); 
		} else {
		if (copyout((char *)(&ngrpages), (char *)(arg), 
			sizeof(long)) != 0)
			rc = EINVAL;
		}
		break;
	default:
		rc = EINVAL;
		break;
	}
	vm_det(rd);
	return(rc);
}

ram_read (dev, uiop, chan, ext)
dev_t	dev;
struct	uio	*uiop;
chan_t	chan;
int	ext;
{
	return ram_rdwr (dev, uiop, UIO_READ);
}	

ram_write (dev, uiop, chan, ext)
dev_t	dev;
struct	uio	*uiop;
chan_t	chan;
int	ext;
{
	return ram_rdwr (dev, uiop, UIO_WRITE);
}	

ram_rdwr (dev, uiop, rw)
dev_t	dev;
struct	uio	*uiop;
int	rw;
{
	int	rc = 0;			/* Return code from xmem routines */
	off_t	off;			/* Offset into RAM disk */
	uint	ramlength;		/* length of ram disk		*/
	caddr_t	s;			/* Address on RAM disk */
	dev_t	minor_dev;		/* Minor device number derived	*/

	BUGLPR(ramdd_dbg, 10, ("ram_rdwr: %x %x %x\n", dev, uiop, rw));
	minor_dev = minor (dev);

	/* Verify the device i.e., that minor_dev < NUMRAMDISKS
	 * and if RAM disk exists for this device.
	 */
	if (minor_dev >= NUMRAMDISKS || ramsrval[minor_dev] == 0) {
		BUGLPR(ramdd_dbg, 10, ("ram_rdwr: error\n"));
		return(ENXIO);
	}

	ramlength = SEGSIZE;
	off = uiop->uio_offset;

	/* Verify requested start address is on device.
	 *  i.e., starts before end of RAM disk.
	 */
	if (off >= ramlength)
	{	if (off > ramlength)
			rc = EIO;
		BUGLPR(ramdd_dbg, 10, ("ram_rdwr: error\n"));
		return(rc);
	}

	/* Verify request ends before the end of the RAM disk.
	 *  If not, adjust uio_resid (i.e. truncate the request).
	 */

	if ((off + uiop->uio_resid) > ramlength )
		uiop->uio_resid = ramlength - off;

	/* This opperation will not be supported for compressed ram
	 * disk
	 */
	if (ramcompflag && minor_dev == BOOTRAM) {
		BUGLPR(ramdd_dbg, 10,
			 ("ram_rdwr: called for packed image\n"));
		return(EIO);
	}

	/* Setup addressibility to the RAM disk.
	 * Returns the start of the requested block on the RAM disk.
	 */
	s = vm_att (ramsrval[minor_dev], off);


	/* Copy to/from ram disk segment from/to buffer cache.
	 */
	rc = uiomove (s+off, uiop->uio_resid, 
				(rw == UIO_READ)? UIO_READ: UIO_WRITE, uiop);
	vm_det (s);

	BUGLPR(ramdd_dbg, 10, ("ram_rdwr: rc=%x\n", rc));

	return rc;
}


/*
 * NAME: ram_low_mem
 *
 * FUNCTION: checks if system is booting with a small memory
 *	configuration (less than 12 Megs)
 *
 * RETURNS:
 *	0 if there is a large amount of real memory
 *	1 if there is a small amount of real memory
 */
int
ram_low_mem()
{

	if ((vmker.nrpages - vmker.badpages) > 0xc00) {
		BUGLPR(ramdd_dbg, 10, ("ram_low_mem: return 0\n"));
		return(0);
	} else {
		BUGLPR(ramdd_dbg, 10, ("ram_low_mem: return 1\n"));
		return(1);
	}
}
