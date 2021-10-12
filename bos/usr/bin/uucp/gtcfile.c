static char sccsid[] = "@(#)01	1.5  src/bos/usr/bin/uucp/gtcfile.c, cmduucp, bos411, 9428A410j 6/17/93 14:06:20";
/* 
 * COMPONENT_NAME: CMDUUCP gtcfile.c
 * 
 * FUNCTIONS: commitall, gtcfile, svcfile, wfabort, wfcommit 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.gtcfile.c
	gtcfile.c	1.1	7/29/85 16:33:07
*/
#include "uucp.h"
/* VERSION( gtcfile.c	5.2 -  -  ); */

extern nl_catd catd;
#define NCSAVE	30	/* no more than 30 saved C files, please */
static int ncsave;
static struct {
	char	file[NAMESIZE];
	char	sys[NAMESIZE];
} csave[NCSAVE];

/*
 *	svcfile  - save the name of a C. file for system sys for re-using
 *	returns
 *		none
 */

svcfile(file, sys)
char	*file, *sys;
{
	ASSERT(ncsave < NCSAVE, MSGSTR(MSG_GTC1, "TOO MANY SAVED C FILES"),
			 "", ncsave);
	(void) strcpy(csave[ncsave].file, BASENAME(file, '/'));
	(void) strcpy(csave[ncsave].sys, sys);
	ncsave++;
	return;
}

/*
 *	gtcfile - copy into file the name of the saved C file for system sys
 *
 *	returns
 *		SUCCESS	-> found one
 *		FAIL	-> none saved
 *		
 */

gtcfile(file, sys)
char	*file, *sys;
{
	register int	i;

	for (i = 0; i < ncsave; i++)
		if (strncmp(sys, csave[i].sys, SYSNSIZE) == SAME) {
			(void) strcpy(file, csave[i].file);
			return(SUCCESS);
		}
	
	return(FAIL);
}

/*	commitall()
 *
 *	commit any and all saved C files
 *
 *	returns
 *		nothing
 */

commitall()
{
	/* not an infinite loop; wfcommit() decrements ncsave */
	while (ncsave > 0)
		wfcommit(csave[0].file, csave[0].file, csave[0].sys);
}

/*
 *	wfcommit - move wfile1 in current directory to SPOOL/sys/wfile2
 *	return
 *		none
 */

wfcommit(wfile1, wfile2, sys)
char	*wfile1, *wfile2, *sys;
{
	int	i;
	char	cmitfile[MAXFULLNAME];
	char	*file1Base, *file2Base;

	DEBUG(6, "commit %s ", wfile2);
	mkremdir(sys);		/* sets RemSpool */
	file1Base = BASENAME(wfile1, '/');
	file2Base = BASENAME(wfile2, '/');
	sprintf(cmitfile, "%s/%s", RemSpool, file2Base);
	DEBUG(6, "to %s\n", cmitfile);
	
	ASSERT(access(cmitfile, 0) != 0,
		 MSGSTR(MSG_UDEFS_17, "FILE EXISTS"), cmitfile, 0);
	ASSERT(xmv(wfile1, cmitfile) == 0, 
		MSGSTR(MSG_UDEFS_10, "CANNOT LINK"), cmitfile, errno);

	/* if wfile1 is a saved C. file, purge it from the saved list */
	for (i = 0; i < ncsave; i++) {
		if (EQUALS(file1Base, csave[i].file)) {
			--ncsave;
			(void) strcpy(csave[i].file, csave[ncsave].file);
			(void) strcpy(csave[i].sys, csave[ncsave].sys);
			break;
		}
	}
	return;
}

/*	wfabort - unlink any and all saved C files
 *	return
 *		none
 */

wfabort()
{
	register int	i;

	for (i = 0; i < ncsave; i++)
		(void) unlink(csave[i].file);
	ncsave = 0;
	return;
}
