static char sccsid[] = "@(#)29	1.8  src/bos/usr/ccs/lib/libIN/CScurdir.c, libIN, bos411, 9428A410j 6/10/91 10:14:41";
/*
 * LIBIN: CScurdir
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
 * FUNCTION: Places the full pathname of the current directory
 *	in the caller's supplied string.
 *
 * RETURN VALUE DESCRIPTION: Returns 0 if successful, -1 on ERROR.
 */

CScurdir(str)
char *str;
{
	char	*getwd();

	return (getwd(str) == (char *) 0) ? -1: 0;
}
