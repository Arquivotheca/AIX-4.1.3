static char sccsid[] = "@(#)95	1.1  src/bos/usr/bin/errlg/libras/odmperror.c, cmderrlg, bos411, 9428A410j 3/2/93 09:02:09";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: odm_perror
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:      odmperror
 * FUNCTION:  print out error codes from ODM in perror() style.
 * INPUTS:    NONE  (uses global variable 'odmerrno'
 * RETURNS:   NONE
 *
 * A routine like this will eventually be provided by the ODM in libodm.a
 * making this routine obsolete.
 */

#include <stdio.h>
#include <errno.h>
#include <odmi.h>
#include <libras.h>

extern odmerrno;

odm_perror(s,a,b,c,d)
char *s;
{
	char *odm_sb;

	if(s && s[0]) 
		Debug("No valid arguments to odmperror.\n");
	if(odmerrno == 0) {
		Debug("No error from odmerrno.\n");
		return;
	}
	if(0 == odm_err_msg(odmerrno, &odm_sb)) {
		Debug("%s\n",odm_sb);
		return;
	}
	Debug("Unknown odm error code: %d\n",odmerrno);
}

