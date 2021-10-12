static char sccsid[] = "@(#)71	1.4  src/bos/usr/bin/dosdir/dopen.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:56:59";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dopen incr_accesscnt 
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

/*
 *      DOPEN returns pointer to dir entry in specified dirpath
 */

FCB *dopen(disk, pathname, oflags, mode)
DCB *disk;
byte *pathname;
long oflags;
int  mode;
{
register int d, i;
register byte *p;
int startcluster;
FCB *fcb,*dcreate();

	if (disk->magic != DCBMAGIC)
	{       doserrno = DE_INVAL;
		return(NULL);
	}
	_DFsetlock(disk);
	if ((d = _DFlocate(pathname,disk)) == 0)        /* can't find name! */
	{       if (oflags & DO_CREAT)
		{	if ((fcb = dcreate(disk,pathname,mode))  !=  NULL)
				incr_accesscnt(fcb);
			_DFunlock(disk);
			return(fcb);
		}
		doserrno = DE_NOENT;
		_DFunlock(disk);
		return(NULL);
	}
	else if ((oflags & DO_CREAT) && (oflags & DO_EXCL))
	{       doserrno = DE_EXIST;
		_DFunlock(disk);
		return(NULL);
	}
	if (d == PC_ROOTDIR)                     /* can't open root as file */
	{       doserrno = DE_ISDIR;
		_DFunlock(disk);
		return(NULL);
	}
	if (dir->df_attr & FIL_VOL)             /* can't open label as file */
	{       doserrno = DE_NOENT;
		_DFunlock(disk);
		return(NULL);
	}
	if( (oflags & (DO_WRONLY|DO_RDWR)) && (dir->df_attr & FIL_RO) )
	{       doserrno = DE_ACCES;
		_DFunlock(disk);
		return(NULL);
	}
	for (i=0; i<MAXFILES; i++)
		if (files.fcb[i].magic != FCBMAGIC)
			break;
	if (i == MAXFILES)
	{       doserrno = DE_MFILE;
		_DFunlock(disk);
		return(NULL);
	}
	fcb = &files.fcb[i];
	fcb->clustsize    = disk->bpb.pb_secsiz * disk->bpb.pb_csize;
	startcluster = dir->df_lcl | (dir->df_hcl << 8);

	if ( oflags & DO_TRUNC )
	{	if (_DFreclaimspace(disk, startcluster)  !=  0)
		{	_DFunlock(disk);
			return(NULL);
		}
		_DFputfat(disk);
		fcb->changed      = 1;
		fcb->timestamp    = time((time_t *)0);
		fcb->size         = 0;
		fcb->startcluster = 0;
		fcb->seek         = 0;
	}
	else
	{
		fcb->changed      = 0;
		fcb->size         = dir->df_siz0 | (dir->df_siz1<<8)
				    | (dir->df_siz2<<16) | (dir->df_siz3<<24);
		fcb->startcluster = startcluster;
		fcb->seek         = disk->data +
				    fcb->clustsize * (fcb->startcluster-2);
	}
	fcb->d_seek       = d;
	fcb->disk         = disk;
	fcb->nowcluster   = fcb->startcluster;
	fcb->offset       = 0;                   /* point file at beginning */
	fcb->clseek       = 0;                /* at start of current sector */
	fcb->oflag        = oflags;
	fcb->magic        = FCBMAGIC;

	/* Increment file access count.
	 */
	incr_accesscnt(fcb);

	if (oflags & DO_APPEND)
		dlseek(fcb,0,2);
	_DFunlock(disk);
	return(fcb);
}

/*   Mark file as currently in use so that nobody
 * will delete it out from under us.
 *   Ideally we just keep a "use count" in the file's
 * directory entry, but this isn't possible in the current
 * implementation because directories are not kept in-core
 * and we aren't allowed to diddle with the on-disk directory
 * structure.
 *   So for the time being, keep a "use count" in the in-core
 * FAT entry for the file's first cluster.  Unfortunately, this
 * means that *ALL* files must have at least one cluster, even
 * zero-length files.  Oh, well.
 */
incr_accesscnt(file)
FCB *file;
{
	/* Every file must contain at least one cluster.
	 *  **** Should delete file if '_DFgetnewcluster' fails.
	 */
	if (file->startcluster  ==  0)
		if (_DFgetnewcluster(file, file->disk)  <=  0)
			return;

	use_cluster(file->disk, file->startcluster);
}
