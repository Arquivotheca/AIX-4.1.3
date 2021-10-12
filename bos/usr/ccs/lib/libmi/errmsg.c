static char sccsid[] = "@(#)80	1.1  src/bos/usr/ccs/lib/libmi/errmsg.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:10";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** errmsg.c 2.1, last change 11/14/90
 **/


extern	int	sprintf(   char * buf, char * format, ...   );

extern	int	errno;
extern	int	sys_nerr;
extern	char	* sys_errlist[];

char *
errmsg (err)
	int	err;
{
	static	char	buf[40];

	if (err  ||  (err = errno)) {
		if (err > 0  &&  err < sys_nerr  &&  sys_errlist[err])
			return sys_errlist[err];
	}
	if (err)
		(void)sprintf(buf, "error number %d", err);
	else
		(void)sprintf(buf, "unspecified error");
	return buf;
}
