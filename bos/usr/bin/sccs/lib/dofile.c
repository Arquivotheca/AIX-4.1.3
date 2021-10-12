static char sccsid[] = "@(#)82  1.14  src/bos/usr/bin/sccs/lib/dofile.c, cmdsccs, bos412, 9444A412a 10/25/94 16:03:52";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: do_file
 *
 * ORIGINS: 3, 10, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include       "defines.h"
#include	<sys/types.h>
#include 	<sys/param.h>
#include	<dirent.h>

extern int Fcnt;

char 	is_dir;           /* p46254 */
int	nfiles;
char	had_dir;
char	had_standinp;

char str[FILESIZE];
char ibuf[FILESIZE];

DIR *dirdat;
struct dirent *dirent;

do_dir(p, func)		/* p46253+ */ 
register char *p;
int (*func)();
{     
		is_dir = 1;	/* p46254 */
		had_dir = 1;
		Ffile = p;
		if ((dirdat = opendir(p)) == NULL)
			return;
		readdir(dirdat);			/* skip "." */
		readdir(dirdat);			/* skip ".." */
		do {
			dirent = readdir(dirdat) ;
			if (dirent != NULL) {
				sprintf(str,"%s/%s",p,dirent->d_name);
				if(sccsfile(str)) {
					Ffile = str;
					(*func)(str);
					nfiles++;
				}
			}
		} while (dirent != NULL);
		closedir(dirdat);
	is_dir = 0;             /* p46254 */
}				/* p46253. */

do_file(p,func)
register char *p;
int (*func)();
{
	if ((p[0] == '-') && ((isspace(p[1])) || ( p[1]==NULL))) {
		had_standinp = 1;
		while (gets(ibuf) != NULL) {
			if (sccsfile(ibuf)) {
				Ffile = ibuf;
				(*func)(ibuf);
				nfiles++;
			} else if (exists(ibuf) && 
				(Statbuf.st_mode & S_IFMT) == S_IFDIR) {
				do_dir(ibuf, func);
			} /* else it is non-existent or not SCCS */
			/*
			 * POSIX & X/Open say that non-SCCS files and 
			 * non-existent files are silently ignored
			 * when the - file operand is specified.
			 */
		}
	}
	else if (exists(p) && (Statbuf.st_mode & S_IFMT) == S_IFDIR) {
		do_dir(p, func);
	}
	else {
		Ffile = p;
		(*func)(p);
		nfiles++;
	}
}
