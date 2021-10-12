static char sccsid[] = "@(#)48	1.3  src/bos/usr/bin/mh/sbr/directory.c, cmdmh, bos411, 9428A410j 6/15/90 22:11:46";
/* 
 * COMPONENT_NAME: CMDMH directory.c
 * 
 * FUNCTIONS: closedir, mkdir, opendir, readdir, rename 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/dir.h>

/*
 * open a directory.
 */
#ifdef _NO_PROTO
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;

	if ((fd = open(name, 0)) == -1)
		return NULL;
	if ((dirp = (DIR *)malloc(sizeof(DIR))) == NULL) {
		close (fd);
		return NULL;
	}
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return dirp;
}





/*
 * close a directory.
 */
void
closedir(dirp)
	register DIR *dirp;
{
	close(dirp->dd_fd);
	dirp->dd_fd = -1;
	dirp->dd_loc = 0;
	free(dirp);
}




/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct direct *dp;

	for (;;) {
		if (dirp->dd_loc == 0) {
			dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, 
			    DIRBLKSIZ);
			if (dirp->dd_size <= 0)
				return NULL;
		}
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct direct *)(dirp->dd_buf + dirp->dd_loc);
		if (sizeof(struct direct) > DIRBLKSIZ + 1 - dirp->dd_loc)
			return NULL;
		dirp->dd_loc += sizeof(struct direct);
		if (dp->d_ino == 0)
			continue;
		return (dp);
	}
}







mkdir(dirname)
register char	*dirname;
{
	static char	buf[BUFSIZ];
	sprintf(buf, "mkdir %s", dirname);
	system(buf);
}

rename(oldname, newname)
register char	*oldname, *newname;
{
	register int	rc;

	if ((rc = link(oldname, newname)) != 0)
		return(rc);
	return(unlink(oldname));
}
#endif _NO_PROTO
