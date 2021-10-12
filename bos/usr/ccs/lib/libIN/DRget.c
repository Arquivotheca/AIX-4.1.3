static char sccsid[] = "@(#)31	1.6  src/bos/usr/ccs/lib/libIN/DRget.c, libIN, bos411, 9428A410j 6/10/91 10:15:34";
/*
 * LIBIN: DRstart, DRget, DRend
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <IN/standard.h>
#include <IN/DRdefs.h>

/*
 * NAME: DRstart
 *
 * FUNCTION: Open a directory for reading by DRget.
 *
 * PARAMETERS: Directory name to open for reading.
 *	       Pointer to a struct dir_data DRget work area.
 *	       Flags to control directory reading.
 *
 * RETURN VALUE DESCRIPTION: Returns TRUE if the directory is open
 *	     for reading, otherwise returns FALSE.
 */
DRstart (dirname, d, flags)
char *dirname;
register struct dir_data *d;
{
	struct stat statb;
	
	if( !(flags&DIR_NOSTAT) )
	    if( stat(dirname, &statb) < 0 ||
		    (statb.st_mode&S_IFMT) != S_IFDIR )
		return FALSE;
	if( (d->dir_p = opendir(dirname)) == NULL )
            return FALSE;

	d->dir_flags = flags;
	d->dir_pos = 0;
	return TRUE;
}


/*
 * NAME: DRend
 *
 * FUNCTION: Close the directory.
 *
 * PARAMETERS: Pointer to a directory descriptor.
 *
 * RETURN VALUE DESCRIPTION: None.
 */
void
DRend (d)
register struct dir_data *d;
{
	if( d->dir_p != NULL ) closedir(d->dir_p);
}

/*
 * NAME: DRget
 *
 * FUNCTION: Get the next directory entry.
 *
 * PARAMETERS: Pointer to a struct dir_data DRget work area.
 *	       Directory name to open for reading.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the next DRget
 *	     struct dir_ent directory entry.  Returns NULL if an
 *	     error occurred, or on reaching the end of the directory.
 */
struct dir_ent *
DRget (d, dirname)
char *dirname;
register struct dir_data *d;
{
	static struct dir_ent dir_ent;
	register struct dirent *dp;
	extern char *CScpym();

	for( ;; )
	{   if( d->dir_p == NULL )
	    {   if( (d->dir_p = opendir(dirname)) == NULL )
		    return NULL;
		seekdir(d->dir_p, d->dir_pos);
	    }
	    if( (dp = readdir(d->dir_p)) == NULL )
	    {   closedir(d->dir_p);
		d->dir_p = NULL;
		return NULL;
	    }
	    if( d->dir_flags&DIR_CLOSE )
	    {   d->dir_pos = telldir(d->dir_p);
		closedir(d->dir_p);
		d->dir_p = NULL;
	    }

	    if( (*dp->d_name != '.' || d->dir_flags&DIR_LDOT) ) break;
	}
	CScpym(dir_ent.dir_name, dp->d_name, dp->d_namlen);

	return &dir_ent;
}
