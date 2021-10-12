static char sccsid[] = "@(#)40	1.9  src/bos/usr/ccs/lib/libIN/CSdevname.c, libIN, bos411, 9428A410j 6/10/91 10:14:44";
/*
 * LIBIN: CSdevname
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
 *
 * FUNCTION: Return full pathname of device open on given file descriptor.
 *      NULL is returned if a device is not opened on fd.
 *
 * RETURN VALUE DESCRIPTION: 
 */

#include <sys/limits.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <IN/standard.h>

char *
CSdevname(fd)
{
	struct stat sb;
	struct dirent *db;
	static char dbuf[MAXNAMLEN+6] = "/dev/";
	register ino_t ino;
	register dev_t dev;
	DIR *dfd;

	if( fstat(fd, &sb) < 0 ||
		(sb.st_mode&S_IFMT) != S_IFCHR &&
		(sb.st_mode&S_IFMT) != S_IFBLK )
	    return NULL;
	dev = sb.st_dev;
	ino = sb.st_ino;
	dfd = opendir("/dev");
	stat("/dev", &sb);
	if (dev != sb.st_dev) return NULL;
	db = readdir(dfd);      /* read "." */
	db = readdir(dfd);      /* read ".." */
	while ((db = readdir(dfd)) != NULL)
	    if (db->d_ino == ino)
	    {   CScpym(dbuf + 5, db->d_name, MAXNAMLEN);
		closedir(dfd);
		return dbuf;
	    }
	closedir(dfd);
	return NULL;
}
