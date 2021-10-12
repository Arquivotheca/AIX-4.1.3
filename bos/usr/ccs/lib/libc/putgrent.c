static char sccsid[] = "@(#)99	1.6.1.2  src/bos/usr/ccs/lib/libc/putgrent.c, libcs, bos411, 9428A410j 4/30/93 12:52:51";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: putrent 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <grp.h>

/*                                                                    
 * FUNCTION: Update a group description in the files /etc/group
 *		and /etc/security/group.
 *
 * RETURN VALUE DESCRIPTIONS: Upon successful conpletion, PUTGRENT returns
 *				a value of 0. If PUTGRENT fails a nonzero
 *				value is returned.
 *
 */
/*
 * format a group file entry
 */

int
putgrent(g, f)
register struct group *g;
register FILE *f;
{
	register char	**q;
	char		buf[BUFSIZ+1];
	/*
	 * AIX Version 2.2.1 security enhancement 
	 */
	/* build one comma separated string from pointer array */
	if (g == NULL || f == NULL)
	{
#ifdef DEBUGX
		fprintf(stderr, "putgrent called with null group ptr\n");
#endif
		return (-1);
	}
	for (buf[0] = '\0', q = g->gr_mem; *q;)
	{
		strcat (buf, *q);
		if (*(++q) != NULL)
			strcat (buf, ",");
	}
	(void) fprintf(f, "%s:%s:%lu:%s",
			  g->gr_name, 	/* group name */
			  g->gr_passwd,	/* password default value */
			  g->gr_gid,  	/* group id */
			  buf);		/* list of comma separated grp names */

	(void) putc('\n', f);
	return(ferror(f));
	/* 
	 * TCSEC Division C Class C2 
	 */
}
