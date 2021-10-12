static char sccsid[] = "@(#)62	1.5  src/bos/usr/bin/dosdir/dcreate.c, cmdpcdos, bos411, 9428A410j 8/2/91 13:57:50";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dcreate _findslot 
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
 *      DCREATE returns pointer to fcb  else NULL for error
 */

FCB *dcreate(disk, pathname, mode)
DCB *disk;
byte *pathname;
int mode;
{
byte *_DFlocc(), *_DFcpym();
register byte *q, *p, xc, *name;
register int d, n, c, i, size, absolute;
time_t time(time_t *timer);
int read();
FCB *fcb;

	if (disk->magic != DCBMAGIC)
	{       doserrno = DE_INVAL;
		return(NULL);
	}
	for (i=0; i<MAXFILES; i++)              /* any space in file table? */
		if (files.fcb[i].magic != FCBMAGIC)
			break;
	if (i==MAXFILES)
	{       doserrno = DE_MFILE;
		return(NULL);
	}
	_DFsetlock(disk);
	if (dremove(disk,pathname) < 0)          /* end only if badpathname */
	{       if (doserrno != DE_NOENT)     /* continue if just not found */
		{       _DFunlock(disk);
			return(NULL);
		}
	}
	fcb = &files.fcb[i];
	p = pathname;
	if ((*p == '\\') || ( current.dir[disk->home].pathname == PC_ROOTDIR))
		absolute = 1;
	else
		absolute = 0;
	while (*p)
		p++;                          /* search for end of pathname */
	while (( *p != '\\') && (p >= pathname))
		p--;                          /* look for preceding "slash" */
	name = &p[1];                         /* point at lowest level name */

	TRACE(("DCREATE: name= %s root=%d data=%d\n",
		name, disk->root,disk->data));

#define DOSCHARS  ".ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\'`$#&@!%()-_^~"
	for( q=name; *q; q++ )     /* check for validity of name */
		if (strchr(DOSCHARS,*q)==0) {
		      doserrno = DE_FNAME;
		      _DFunlock(disk);
		      return(NULL);
		}

	if (p < pathname)
		p++;                       /* point at slash or at 1st byte */
	xc = *p;                                          /* save this byte */
	*p = '\0';                                /* temp end pathname here */
	TRACE(("DCREATE: pathname= %s\n",pathname));

	if (pathname[0] != '\0')
	{       if ((d = _DFlocate(pathname,disk)) == 0)  /* cant find dir  */
		{       _DFunlock(disk);
			return(NULL);
		}
		if (d == PC_ROOTDIR)
		{
			if ((d = _findslot(disk,NULL)) == 0)
			{       _DFunlock(disk);
				return(NULL);
			}
		TRACE(("DCREATE: non-null pathname, == ROOT d= %d\n",d));
		}
		else
		{
			if ((d = _findslot(disk,dir)) == 0)
			{       _DFunlock(disk);
				return(NULL);
			}
		TRACE(("DCREATE: non-null pathname, != ROOT d= %d\n",d));
		}
	}
	else
	{       if (absolute)
		{
			if ((d = _findslot(disk,NULL)) == 0)
			{       _DFunlock(disk);
				return(NULL);
			}
		TRACE(("DCREATE: null pathname, == ABSOLUTE d= %d\n",d));
		}

		else                                  /* RELATIVE */
		{
			lseek(disk->fd,current.dir[disk->home].pathname,0);
			_devio(read,disk->fd,dir,32);
			if ((d = _findslot(disk,dir)) == 0)
			{       _DFunlock(disk);
				return(NULL);
			}
		TRACE(("DCREATE: null pathname, != ABSOLUTE d= %d\n",d));
		}
	}
	*p = xc;                         /* restore first character to name */
	p = (byte *)dir;
	for (i=0;i<11;i++)
		*p++ = ' ';
	for (i=0; i<21; i++)
		*p++ = '\0';
	c = *(q = _DFlocc(name,'.'));             /* Find start of extension */
	if (c)                              /* Set up extension, if present */
		*q = '\0';
	p = _DFcpym(dir,name,8);
	*p = ' ';
	if (c)
	{       p = _DFcpym(dir->df_ext,q+1,3);
		*p = ' ';
		*q = c;                     /* restore . to name if present */
	}
	p = (byte *)dir;
	for (n=0;n<11;n++)
		if (p[n] >= 'a' && p[n] <= 'z')
			p[n] &= ~040;
	dir->df_attr = mode & 0xff;                   /* set attribute byte */
	_DFmakedate(dir,time((time_t *)0));  /* update time and date fields */
	if (lseek(disk->fd,d,0) < 0)  /* seek to dir disk locn */
	{       doserrno = errno;
		_DFunlock(disk);
		return(NULL);
	}
	if (_devio1(disk, d, disk->protect,disk->fd,dir,32) < 0)        /* rewrite directory */
	{       doserrno = errno;
		_DFunlock(disk);
		return(NULL);
	}
	_DFunlock(disk);
	fcb->d_seek = d;
	fcb->disk = disk;
	fcb->size = 0;
	fcb->startcluster = 0;
	fcb->nowcluster = 0;
	fcb->clustsize = disk->bpb.pb_secsiz * disk->bpb.pb_csize;
	fcb->seek = 0;
	fcb->offset = 0;                         /* point file at beginning */
	fcb->clseek = 0;                      /* at start of current sector */
	fcb->changed = 0;                                /* flag as unchanged */
	fcb->oflag = DO_RDWR;
	fcb->magic = FCBMAGIC;
	return(fcb);
}
/*
 *
 */
_findslot(disk,ndir)
DCB *disk;
pc_dirent *ndir;
{
int seek, count, tnxtcl, tcurrcl;
int read();
register int n;
FCB file[1];            /* fake entry   */
pc_dirent direc;
char *p, *calloc();
	if (ndir == NULL)
	{       seek = disk->root;
		count = (disk->data - disk->root) / 32;
		tnxtcl = PC_EOF;
	}
	else
	{       tcurrcl = ndir->df_lcl | (ndir->df_hcl<<8);
		seek = disk->data + disk->clsize * (tcurrcl-2);
		count = disk->clsize / 32;
		tnxtcl = getnextcluster(disk,tcurrcl);
	}
	while (tnxtcl)
	{       for (n=0; n<count; n++, seek+=32)
		{       lseek(disk->fd,seek,0);
			_devio(read,disk->fd,&direc,32);
			if ((direc.df_use == DIR_NIL)
			     || (direc.df_use == DIR_MT))
				return(seek);
		}
		if (tnxtcl < PC_EOF)
		{	seek = disk->data + disk->clsize * (tnxtcl-2);
			count = disk->clsize/32;
			tcurrcl = tnxtcl;
			tnxtcl = getnextcluster(disk,tcurrcl);
		}
		else
		{       if (ndir)
			{       file->nowcluster = tcurrcl;
				if ((n = _DFgetnewcluster(file,disk)) == 0)
					return(0);
				seek = disk->data+disk->clsize*(n-2);
				p = calloc(disk->clsize,1);
				if (p==0)
				{       doserrno = errno;
					return(0);
				}
				lseek(disk->fd,seek,0);  /* clear directory */
				if (_devio1(disk, seek, disk->protect,disk->fd,
				   p,disk->clsize) < 0)  /* rewrite cluster */
				{       doserrno = errno;
					free(p);
					return(0);
				}
				free(p);
				_DFputfat(disk);     /* keep FAT up-to-date */
			}
			else
				tnxtcl = 0;
		}
	}
	doserrno = DE_RFULL;
	return(0);
}
