static char sccsid[] = "@(#)12	1.4  src/bos/usr/bin/dosdir/locate.c, cmdpcdos, bos411, 9428A410j 4/22/94 15:34:04";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: _DFlocate _findname 
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
 *  Locate, on a given device, the specified file name.  Start searching
 *      in current directory for device.  Leaves directory info for file
 *      in area pointed to by dir. Returns seek address else 0 for fail.
 */
_DFlocate(name,disk)
byte *name;
DCB *disk;
{
register byte  *q, *nm;
register int  d, seek, c, tnxtcl, count;
byte *_DFlocc(), *_DFskpc();
int read();

	dir = (pc_dirent *)&cluster[0];
	if ( (strcmp(name,"\\")==0)
	    || ((current.dir[disk->home].start == disk->root)
	    &&  (strcmp(name,".")==0))  )
		return(PC_ROOTDIR);

	if ((current.dir[disk->home].start == disk->root)
	    &&  (strcmp(name,"..")==0))
	{       doserrno = DE_NOTDIR;
		return(0);
	}

	nm = name;
	if ((nm[0] == '\\')                           /* start at the root? */
		|| (current.dir[disk->home].start == disk->root))
	{       if (nm[0] == '\\')                    /* absolute pathname? */
			nm++;                        /* step over ldg slash */
		seek = disk->root;
		count = (disk->data - disk->root)/32;
		tnxtcl = PC_EOF;
	}
	else
	{       seek = current.dir[disk->home].start;
		count = disk->clsize/32;
		tnxtcl = current.dir[disk->home].nxtcluster;
	}
	c = *(q = _DFlocc(nm,'\\'));
	*q = '\0';                    /* temp path end -- Nullify separator */

	_DFsetlock(disk);
	while (1)
	{       d = _findname(nm,disk,count,seek,tnxtcl);
		*q++ = c;                           /* remove temp path end */
		if (d)                             /* Found next component? */
		{       if (c == '\0')                  /* End of pathname? */
			{	_DFunlock(disk);
				return(d);
			}
			if (dir->df_attr & 0x10)      /* is it a directory? */
			{       tnxtcl = dir->df_lcl|(dir->df_hcl<<8);
				if (tnxtcl == 0)         /* is it the root? */
				{       seek = disk->root;
					count = (disk->data - disk->root)/32;
					tnxtcl = PC_EOF;
				}
				else
				{       seek = disk->data+disk->clsize*(tnxtcl-2);
					tnxtcl = getnextcluster(disk,tnxtcl);
					count = disk->clsize / 32;
				}
				nm = _DFskpc(q,'\\');
				c = *(q = _DFlocc(nm,'\\'));
				*q = '\0';
			}
			else
			{       doserrno = DE_NOTDIR;
				_DFunlock(disk);
				return(0);
			}
		}
		else
		{       if (c == '\0')                  /* End of pathname? */
				doserrno = DE_NOENT;
			else
				doserrno = DE_NOTDIR;
			_DFunlock(disk);
			return(0);
		}
	}
}
/*
 *  Search this level for file name - stop on success or first null directory.
 *	returns disk seek address for directory entry else 0 for failure.
 *	leaves directory entry in area pointed to by dir.
 */
_findname(name, disk, count, seek, tnxtcl)
register byte *name;
register int  count, seek, tnxtcl;
DCB *disk;
{
byte dname[12];
int read();
register byte *q, *p, *x;
register int n, c;
byte *_DFlocc(), *_DFcpym();


	for (n=0; n<11; n++)
		dname[n] = ' ';
	if (name[0] == '.')
	{       dname[0] = '.';
		if (name[1] == '.')
			dname[1] = '.';
		if (seek == disk->root)
		{	if (name[1] == '.')
			{       doserrno = DE_NOTDIR;
				return(0);
			}
			else
				return(PC_ROOTDIR);
		}
	}
	else
	{       c = *(q = _DFlocc(name,'.'));     /* Find start of extension */
		if (c)               /* terminate name if extension present */
			*q = '\0';
		p = _DFcpym(dname,name,8);
		*p = ' ';
		if (c)                      /* Set up extension, if present */
		{       p = _DFcpym(&dname[8],q+1,3);
			*p = ' ';
			*q = c;             /* restore . to name if present */
		}
		for (n=0;n<11;n++)
		    if (dname[n] >= 'a' && dname[n] <= 'z')
			dname[n] &= ~040;
	}
	dname[11] = '\0';
	while (tnxtcl)
	{       lseek(disk->fd,seek,0);
		dir = (pc_dirent *)&cluster[0];
		_devio(read,disk->fd,dir,count*32);
		for (n=0; n<count; n++)
		{       if ((dir->df_use) && (dir->df_use != DIR_MT)
			    && !(dir->df_attr & FIL_VOL))
			{       if (_DFcmpp(dname,dir) == 0)
					return(seek);
			}
			else
			{	if (dir->df_use == DIR_NIL)
				{       doserrno = DE_NOENT;
					return(0);
				}
			}
			dir++;
			seek += 32;
		}
		if (tnxtcl < PC_EOF)
		{	seek = disk->data + disk->clsize * (tnxtcl-2);
			count = disk->clsize/32;
			tnxtcl = getnextcluster(disk,tnxtcl);
		}
		else
			tnxtcl = 0;
	}
	doserrno = DE_NOENT;
	return(NULL);
}
