static char sccsid[] = "@(#)51	1.9  src/bos/usr/ccs/lib/libIN/FSdskname.c, libIN, bos411, 9428A410j 12/14/93 07:27:38";
/*
 * LIBIN: FSdskname
 *
 * ORIGIN: 9,10
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1993
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: 
 *      This routine uses the filesystem definition file to map the name
 *      of a mounted file system into the name of the device on which it
 *      is mounted.
 *
 * PARAMETERS:
 *      name of a file system
 *
 * RETURN VALUE DESCRIPTION: 
 *      name of device upon which it is normally mounted
 *      0       unable to locate
 */

#include <stdio.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <nl_types.h>
#include "libIN_msg.h"

char *FSdskname( fsname )
 register char *fsname;
{       register char *d = (char *) 0;
	register ATTR_t a;
	AFILE_t portf;
	static char dsknm[MAXREC];
	nl_catd catd;	/* message catalog file descriptor */

	if ((portf = AFopen(FSYSname,MAXREC,MAXATR)) == NULL)
	{
		catd = catopen(MF_LIBIN, NL_CAT_LOCALE);
		fprintf(stderr,catgets(catd, MS_LIBIN, M_CANTOPEN,
		    "Cannot open %s\n"), FSYSname);
		catclose(catd);
		return("unknown");
	}

	/*
	 * look for the named file system
	 */
	while ((a = AFnxtrec(portf)) != NULL)
	{       if (strcmp( fsname, a->AT_value ))
			continue;

		/*
		 * we found the stanza
		 *      get the other information and do the mount
		 */
		d = AFgetatr( a, "dev" );
		if (d == NULL)
		{
			catd = catopen(MF_LIBIN, NL_CAT_LOCALE);
			fprintf(stderr,catgets(catd, MS_LIBIN, M_FSDSK,
			    "No device specified for %s\n"), fsname);
			catclose(catd);
		}
		break;
	}

	strcpy(dsknm, d);
	AFclose(portf);
	return( dsknm );
}
