static char sccsid[] = "@(#)88	1.15.1.2  src/bos/usr/ccs/lib/libc/ttyname.c, libcio, bos411, 9428A410j 10/20/93 14:32:16";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: ttyname, ttyname_r 
 *
 * ORIGINS: 3,27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/errno.h>

#ifdef _THREAD_SAFE
/*
 * Since readdir_r() returns 0 on success, and an errno on failure, need to
 *  check both return code and value in entry since return code could be 0
 *  but entry could be NULL.
 */
#define READDIR(d) (!readdir_r(d, &dentry, &entry) && (entry!=NULL))
#define SUCCESS 0
#define FAILURE errno
#else
#define READDIR(d) ((entry = readdir(d)) != NULL)
#define SUCCESS rbuf
#define FAILURE NULL
static char rbuf[MAXNAMLEN];
#endif /*_THREAD_SAFE*/
/*                                                                    
 * FUNCTION: Return "/dev/ttyXX", the name of the tty belonging to file f.
 *
 * RETURN VALUE DESCRIPTION: NULL on error.
 */  
/*
 * ttyname(f): return "/dev/ttyXX", which is the name of the
 * tty belonging to file f.
 *
 * This program works in two passes: the first pass tries to
 * find the device by matching device and inode numbers; if
 * that doesn't work, it tries a second time, this time doing a
 * stat on every file in /dev and trying to match device numbers
 * only. If that fails too, NULL is returned.
 */

#ifdef _THREAD_SAFE
int
ttyname_r(int f, char *outbuf, size_t len)
#else 
char *
ttyname(int f)
#endif /*_THREAD_SAFE*/
{
#ifdef _THREAD_SAFE
	char rbuf[MAXNAMLEN];
	struct dirent dentry;
#endif /* _THREAD_SAFE */
	char *dev="/dev/";
	char newbuf[TTNAMEMAX];
	struct stat fsb, tsb;
	struct dirent *entry;
	register DIR *dirp;
	register pass1;

#ifdef _THREAD_SAFE
	/**********
	  If the buffer passed NULL, set errno and return
	**********/
	if((outbuf == NULL) || (len < 1)) {
		errno=EINVAL;
		return(FAILURE);
		}

#endif /*_THREAD_SAFE*/

	/* See if we can do it quickly first */
	if (ioctl(f, TXTTYNAME, newbuf) == 0) {
		strcpy(rbuf, dev);
		strcat(rbuf, newbuf);
#ifdef _THREAD_SAFE
		if (strlen(rbuf) >= len) {
			errno=ERANGE;
			return(FAILURE);
			}
		strcpy(outbuf, rbuf);
#endif /* _THREAD_SAFE */
		return(SUCCESS);
	}
	if(isatty(f) == 0) 
		return(FAILURE);
	if(fstat(f, &fsb) < 0)
		return(FAILURE);
	if((fsb.st_mode & S_IFMT) != S_IFCHR) {
		errno=ENOTTY;
		return(FAILURE);
		}
	if((dirp = opendir(dev)) == NULL)
		return(FAILURE);
	pass1 = 1;
	do {
		while(READDIR(dirp)) {
			if(pass1 && entry->d_ino != fsb.st_ino)
				continue;
			(void) strcpy(rbuf, dev);
			(void) strcat(rbuf, entry->d_name);
			if(stat(rbuf, &tsb) < 0)
				continue;
			if(tsb.st_rdev == fsb.st_rdev &&
				(tsb.st_mode & S_IFMT) == S_IFCHR &&
				(!pass1 || tsb.st_ino == fsb.st_ino)) {
				closedir(dirp);
#ifdef _THREAD_SAFE
				if (strlen(rbuf) >= len) {
					errno=ERANGE;
					return(FAILURE);
					}
				strcpy(outbuf, rbuf);
#endif /* _THREAD_SAFE */
				return(SUCCESS);
			}
		}
		rewinddir(dirp);
	} while(pass1--);
	closedir(dirp);
	errno=ENOTTY;
	return(FAILURE);
}
