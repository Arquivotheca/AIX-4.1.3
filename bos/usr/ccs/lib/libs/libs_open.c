static char sccsid[] = "@(#)18	1.4  src/bos/usr/ccs/lib/libs/libs_open.c, libs, bos411, 9428A410j 6/16/90 02:32:21";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: opst, clst, opgroup, clgroup, oppwd, clpwd, opbase, clbase
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

#include <stdio.h>
#include <usersec.h>
#include "libs.h"

char *
opst (name, af, mode)	/* open stanza file */
char	*name;
AFILE_t	af;
int	mode; 	/* notused */
{
	if (!(af))
		af = afopen (name);

	return ((char *) af);
}

int 
clst (af)	/* close stanza file */
AFILE_t	af;
{
	if (af)
		afclose (af);
}

char *
opgroup (name, af)	/* open base (colon) type file */
char	*name;		/* name not used */
char	*af;		/* attribute file pointer not used */
{
	setgrent();
	return ("");
}
int
clgroup (fp)	/* close base (colon) type file */
char	*fp;	/* not used */
{
	endgrent();
	return (0);
}
char *
oppwd (name, af)	/* open base (colon) type file */
char	*name;		/* not used */
char	*af;		/* not used */
{
	setpwent();
	return ("");
}

int
clpwd (fp)	/* close base (colon) type file */
char	*fp;	/* not used */
{
	endpwent();
	return (0);
}

/* file dependent routines */
char *
opbase (name, af)	/* open base (colon) type file */
char	*name;
char	*af;
{
	setpwent ();
	setgrent();
	return ("");
}

int
clbase (fp)	/* close base (colon) type file */
char	*fp;	/* not used */
{
	endpwent();
	endgrent();
	return (0);
}

