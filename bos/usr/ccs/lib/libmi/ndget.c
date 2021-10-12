static char sccsid[] = "@(#)88	1.1  src/bos/usr/ccs/lib/libmi/ndget.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:26";
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
 ** ndget.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include <sys/stropts.h>
#include <pse/nd.h>
#include <errno.h>

extern	char	* malloc(   unsigned size   );

extern	int	errno;

char *
nd_get (fd, name, buf, buflen, errp)
	int	fd;
	char	* name;
	char	* buf;
	int	buflen;
	int	* errp;
{
	char	* cp;
	int	len;
	struct strioctl	stri;

	len = strlen(name) + 1;
	if (buf  &&  len <= buflen)
		cp = buf;
	else {
		if (!(cp = malloc(MAX(len, buflen)))) {
			*errp = ENOMEM;
			return nilp(char);
		}
		if (!buf) {
			buf = cp;
			buflen = MAX(buflen, len);
		}
	}
	stri.ic_cmd = ND_GET;
	stri.ic_timout = 0;
	stri.ic_dp = cp;
	stri.ic_len = MAX(len, buflen);
	strcpy(cp, name);
	if (ioctl(fd, I_STR, &stri) != -1) {
		if (stri.ic_len < buflen) {
			if (cp != buf) {
				strcpy(buf, cp);
				free(cp);
			}
			return buf;
		}
		*errp = EFBIG;
	} else
		*errp = errno;
	if (cp != buf)
		free(cp);
	return nilp(char);
}
