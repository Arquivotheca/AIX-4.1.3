static char sccsid[] = "@(#)73	1.8  src/bos/usr/ccs/lib/libs/auditwrite.c, libs, bos411, 9428A410j 6/16/90 01:08:26";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: auditwrite
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	"stdio.h"
#include	"sys/errno.h"

int auditwritev (av)
char	**av;
{
	char	*event;
	int	result;
	char	*rec;		/* (allocated) buffer */
	char	*recp;		/* pointer to next space in buffer */
	int	reclen;		/* amount of buffer space left */
	char	buf[BUFSIZ];	/* initial buffer */
	int	r;

	event = *av++;
	result = (int) (*av++);

	recp = rec = buf;
	reclen = sizeof (buf);
	while (1)
	{
		char	*b;
		int	l;

		b = *av++;
		if (b == NULL)
			break;
		l = (int) (*av++);
		if (l <= 0)
			break;
		if (l > reclen)
		{
			char	*nbuf;
			int	nlen;
			int	olen;
			extern	char	*malloc ();

			olen = recp - rec;
			nlen = (((olen + l) + BUFSIZ) / BUFSIZ) * BUFSIZ;
			nbuf = malloc (nlen);
			if (nbuf == NULL)
			{
				extern	int	errno;

				if (rec != buf)
					free (rec);
				errno = ENOSPC;
				return (-1);
			};

			bcopy (rec, nbuf, olen);
			recp = &(nbuf[olen]);

			if (rec != buf)
				free (rec);

			rec = nbuf;
			reclen = nlen - olen;
		};
		bcopy (b, recp, l);
		recp += l;
		reclen -= l;
	};

	r = auditlog (event, result, rec, (recp - rec));

	if (rec != buf)
		free (rec);
	return (r);
}

int auditwrite (event, result)
char	*event;
int 	result;
{
	return (auditwritev (&event));
}
