static char sccsid[] = "@(#)73	1.4  src/bos/usr/bin/dosdir/doschdir.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:57:07";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: doschdir _synch_unix 
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


#include "tables.h"
#include "doserrno.h"

#define NULLDIR "/usr/dos/nulldir"

int     doschdir(dir)
char   *dir;
{
	APATH   a;
	int i,chdir();
	LD ldev;
	PD pdev;
	char *d,*x,c,temp[128], *dospwd();

	TRACE(("doschdir: %s %d\n",dir,strlen(dir)));

	if (dir[1]==':')
	{       /* change disk but not directory                    */
		/* can't use analyze() here since this isn't a file */
		/* so this code is a specialized subset of it       */

		c = toupper((int)dir[0]);
		for (ldev=0; ldev<NUM_LOGICAL; ldev++) {
			if (ld(ldev).device_id[0]==c    &&
			     ld(ldev).device_id[1]==':')
				break;
		}
		if (ldev==NUM_LOGICAL) err_return( DE_NODEV );
		a.ldev = ldev;
		if (i=ld(ldev).assigned_ld)
			a.ldev = i;
		a.pdev = ld(a.ldev).pdevice;

		if (_synch_unix(a.pdev) < 0) return(-1);
		_e.current_disk = ldev;

		dir += 2;             /* rest of pathname */
		if (*dir == 0)
			return(0);
	}

	/* if it needed to be, current disk is now changed */

	if( _analyze( dir, &a ) < 0 ) err_return(DE_NOTDIR);

	switch( pd(a.pdev).contents )

	{       default:
		err_return( DE_NOTDIR );

		case is_dos:
		i = dchdir(pd(a.pdev).dcb, a.fpath);
		TRACE(("chdir: dos %s%s %d\n",ld(a.ldev).device_id,a.fpath,i));
		if (i)
			err_return( DE_NOTDIR );
		break;

		case is_unix:
		i = _devio(chdir,a.upath);
		TRACE(("chdir: unix %s rc=%d\n",a.upath,i));
		if (i)
			err_return( DE_NOTDIR );
		break;
	}
	if (strlen(a.fpath)==1)
		*ld(a.ldev).current_dir = 0;
	else
		strcpy(ld(a.ldev).current_dir, a.fpath);
	return(0);
}

_synch_unix(pdev)
PD pdev;
{
	/* Put unix directory in the right place.  If ldev is    */
	/* removable or non-dos, set unix dir to NULLDIR         */

	char d[128];
	register int i;
	register struct physical_device *p;
	int chdir();

	*d = '\0';
	p = &pd(pdev);
	if ( p->contents == is_unix && !p->removable )
	{       strcpy(d,str(p->mount_dir));
		if (*ld(p->ldevice).current_dir)
		{       strcat(d, ld(p->ldevice).current_dir);
			_flip(d,'\\','/');
		}
	}
	else
		strcpy(d, NULLDIR);
	TRACE(("synch_unix: chdir to %s\n",d));
	i = _devio(chdir,d);
	if (i<0) err_return( errno );
	return(0);
}
