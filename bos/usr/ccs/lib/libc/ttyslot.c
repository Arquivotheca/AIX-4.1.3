static char sccsid[] = "@(#)99	1.7  src/bos/usr/ccs/lib/libc/ttyslot.c, libcio, bos411, 9428A410j 3/4/94 10:39:19";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: ttyslot 
 *
 * ORIGINS: 3,27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/
/*
 *
 * Return the number of the slot in the utmp file
 * corresponding to the current user: try for file 0, 1, 2.
 * Returns -1 if slot not found.
 */
#include <sys/types.h>
#include <sys/dir.h>
#include "utmp.h"
#define	NULL	0

#ifdef _THREAD_SAFE
extern int ttyname_r();
#else /* _THREAD_SAFE */
extern char *ttyname();
#endif /* _THREAD_SAFE */
extern char *strrchr();
extern int strncmp(), open(), read(), close();

int
ttyslot(void)
{
	struct utmp ubuf;
#ifdef _THREAD_SAFE
	char tp_buf[MAXNAMLEN+1];
	char *tp=tp_buf;
#else /* _THREAD_SAFE */
	register char *tp;
#endif /* _THREAD_SAFE */
	register char *p;
	register int s, fd;

	
#ifdef _THREAD_SAFE
	if((ttyname_r(0, tp_buf, MAXNAMLEN) != 0) && 
	   (ttyname_r(1, tp_buf, MAXNAMLEN) != 0) &&
	   (ttyname_r(2, tp_buf, MAXNAMLEN) != 0))
#else 
	if((tp=ttyname(0)) == NULL && 
	    (tp=ttyname(1)) == NULL &&
	    (tp=ttyname(2)) == NULL)
#endif /* _THREAD_SAFE */
		return(-1);

	if((p=strrchr(tp, '/')) == NULL)
		p = tp;
	/* HFT hack:  If last link in device name is all numeric, take it
	 * as a channel number, and back up tail pointer to include previous
	 * link, as well.
	 */
	else if (!p[strspn(p, "/0123456789")])
		while (p != tp && *(p - 1) != '/')
			--p;
	else
		p++;

	if((fd=open(UTMP_FILE, 0)) < 0)
		return(-1);
	s = 0;
	while(read(fd, (char*)&ubuf, sizeof(ubuf)) == sizeof(ubuf)) {
		if(    (ubuf.ut_type == INIT_PROCESS ||
			ubuf.ut_type == LOGIN_PROCESS ||
			ubuf.ut_type == USER_PROCESS ||
			ubuf.ut_type == DEAD_PROCESS ) &&
			strncmp(p, ubuf.ut_line, sizeof(ubuf.ut_line)) == 0){

			(void) close(fd);
			return(s);
		}
		s++;
	}
	(void) close(fd);
	return(-1);
}
