static char sccsid[] = "@(#)89	1.1  src/bos/usr/ccs/lib/libmi/ndset.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:28";
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
 ** ndset.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include <sys/stropts.h>
#include <pse/nd.h>

extern	char	* malloc(   unsigned size   );

int
nd_set (fd, name, buf)
	int	fd;
	char	* name;
	char	* buf;
{
	char	* cp;
	int	i1;
	int	len;
	struct strioctl	stri;

	len = 2 + strlen(name) + strlen(buf);
	i1 = -1;
	if (cp = malloc(len)) {
		/* kernel expects:	name\0value\0	 */
		strcpy(cp, name);
		strcpy(cp + (strlen(name) + 1), buf);
		stri.ic_cmd = ND_SET;
		stri.ic_timout = 0;
		stri.ic_len = len;
		stri.ic_dp = cp;
		i1 = ioctl(fd, I_STR, &stri);
		free(cp);
	}
	return i1;
}
