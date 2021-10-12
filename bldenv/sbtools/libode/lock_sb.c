/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: lock_sb
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: lock_sb.c,v $
 * Revision 1.1.8.1  1993/11/03  20:40:30  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:08  damon]
 *
 * Revision 1.1.6.5  1993/05/10  17:16:04  damon
 * 	CR 258. changed index to strchr
 * 	[1993/05/10  17:15:54  damon]
 * 
 * Revision 1.1.6.4  1993/04/28  20:07:28  damon
 * 	CR Pedantic changes
 * 	[1993/04/28  20:07:10  damon]
 * 
 * Revision 1.1.6.3  1993/04/12  17:24:14  damon
 * 	CR 446 Cleaned up include files
 * 	[1993/04/12  17:24:05  damon]
 * 
 * Revision 1.1.6.2  1993/04/08  19:28:52  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  19:28:40  damon]
 * 
 * Revision 1.1.2.7  1992/12/11  17:44:21  damon
 * 	CR 358. Added base_dir parameter to lock_sb call
 * 	[1992/12/11  16:47:43  damon]
 * 
 * Revision 1.1.2.6  1992/12/03  17:21:07  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:21  damon]
 * 
 * Revision 1.1.2.5  1992/11/13  15:20:27  root
 * 	Changed sys/time.h to time.h
 * 	[1992/11/13  15:04:37  root]
 * 
 * Revision 1.1.2.4  1992/11/12  18:27:58  damon
 * 	CR 329. Moved include of sys/errno.h to errno.h
 * 	[1992/11/12  18:12:02  damon]
 * 
 * Revision 1.1.2.3  1992/11/06  16:14:38  damon
 * 	CR 328. Removed extra debugging code
 * 	[1992/11/06  16:14:27  damon]
 * 
 * Revision 1.1.2.2  1992/10/29  15:30:48  damon
 * 	CR 321. Added sandbox locking
 * 	[1992/10/29  15:22:54  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)88  1.1  src/bldenv/sbtools/libode/lock_sb.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:17";
#endif /* not lint */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/sandboxes.h>
#include <ode/util.h>
#include <sys/param.h>
#include <sys/stat.h>

BOOLEAN
lock_sb ( const char * base_dir, const char * lock_file, const char * user )
{
    char buf[MAXPATHLEN];
    char buffer[MAXPATHLEN];
    char datebuf[MAXPATHLEN];
    char *ptr;
    struct stat statb;
    long now;
    struct tm *tm;
    int status;
    int fd;
    FILE *inf, *outf;

    for (;;) {
	if (access(lock_file, R_OK) == 0)
	    break;
	(void) concat(buf, sizeof(buf), lock_file, "_", user, NULL);
	now = time((long *)NULL);
	tm = localtime(&now);
	fdate(datebuf, "%03Wday %03Month %02day %02hour:%02min:%02sec", tm);
	if ((outf = fopen(buf, "w")) == NULL) {
	    ui_print ( VFATAL, "unable to open %s for write\n", buf);
            return ( FALSE );
        }
        (void) fprintf(outf, "Directory: %s\n", base_dir );
	(void) fprintf(outf, "Locked by: %s\n", user);
	(void) fprintf(outf, "Locked on: %s %04d\n\n",
		       datebuf, BASEYEAR + tm->tm_year);
	(void) fputs("Comments:  <<<replace this with anything others should know about this lock>>>\n", outf);
	if (ferror(outf) || fclose(outf) == EOF) {
	    (void) unlink(buf);
	    ui_print ( VFATAL, "error writing %s\n", buf);
            return ( FALSE );
	}
	status = link(buf, lock_file);
	(void) unlink(buf);
	if (status < 0) {
	    ui_print ( VFATAL, "[ Couldn't link lock file - Sorry ]\n");
	    if ((inf = fopen(lock_file, "r")) != NULL) {
		(void) ffilecopy(inf, stdout);
		(void) fclose(inf);
	    }
	    return ( FALSE );
	}
        editor ( lock_file, NULL );
/*
	(void) runp(EDIT_PROG, EDIT_PROG, lock_file, NULL);
*/
	if (stat(lock_file, &statb) < 0) {
	  ui_print ( VFATAL, "stat failed for %s\n", lock_file);
          return ( 0 );
	} else if (chmod(lock_file, (int)(statb.st_mode&NOWRITE)&MODEMASK) < 0) {
	  ui_print ( VFATAL, "chmod failed for %s\n", lock_file);
          return ( FALSE );
        }
    }
    if ((inf = fopen(lock_file, "r")) == NULL) {
      ui_print ( VFATAL, "[ Couldn't open lock file for reading - Sorry ]\n");
      return ( FALSE );
    }
    if (fgets(buffer, sizeof(buffer), inf) != NULL &&
	fgets(buffer, sizeof(buffer), inf) != NULL &&
	strncmp(buffer, "Locked by: ", 11) == 0) {
	if ((ptr = strchr(buffer, '\n')) != NULL)
	    *ptr = '\0';
	if (strcmp(buffer + 11, user) == 0) {
	    (void) fclose(inf);
	    inf = NULL;
	}
    }
    if (inf != NULL) {
	ui_print ( VALWAYS, "[ sandbox is locked by %s (not %s) - Sorry ]\n", buffer+11, user);
	(void) rewind(inf);
	(void) ffilecopy(inf, stdout);
	(void) fclose(inf);
	return ( FALSE );
    }
    if ((fd = open(lock_file, O_RDONLY)) < 0) {
	ui_print ( VFATAL, "[ Couldn't open lock file for reading - Sorry ]\n");
	return ( FALSE );
    }
#ifndef NO_FLOCK
    if (flock(fd, (LOCK_EX|LOCK_NB)) < 0) {
	if (errno != EWOULDBLOCK) {
	    ui_print ( VFATAL, "unable to lock %s\n", lock_file);
	    (void) close(fd);
	    return ( FALSE );
	}
	ui_print ( VALWAYS, "[ Waiting for sandbox lock ]\n");
#ifndef NO_AFS_BUG
	for (;;) {
	    errno = 0;
	    if (flock(fd, LOCK_EX) == 0)
		break;
	    if (errno != EWOULDBLOCK) {
		ui_print ( VFATAL, "unable to lock %s\n", lock_file);
		(void) close(fd);
		return ( FALSE );
	    }
	    (void) sleep(1);
	}
#else
	if (flock(fd, LOCK_EX) < 0) {
	    ui_print ( VFATAL, "unable to lock %s\n", lock_file);
	    (void) close(fd);
	    return ( FALSE );
	}
#endif
    }
#endif
  return ( TRUE );
}
