static char sccsid[] = "@(#)90  1.6  src/bos/usr/ccs/lib/libodm/odmpath.c, libodm, bos411, 9428A410j 1/14/93 17:35:57";

/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS: legal_size
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef R5A
#include <unistd.h>
#else
#define MAXNAMLEN 255
#endif
#include <sys/dir.h>

#include <odmi.h>


/*
 * NAME:  legal_size
 *
 * FUNCTION:
 *
 *   Takes a path and determines the maximum file name size for that filesystem.
 *
 * RETURNS:
 *   The maximum file name size for the filesystem if successful, -1 otherwise.
 */
int legal_size (classname)
char *classname;   /* Object class name */

{
        int returncode;
        char *local, *odm_set_path ();
	char *p;
        char location [MAX_ODM_PATH + MAX_ODMI_NAME + 3];

        /* find the default directory for the object class */

        local = odm_set_path ((char *) NULL);
        (void) strcpy (location, local);

        /* Make it current directory of the default path, if no default */
        /* path exists - then current directory                         */
        (void) strcat (location, "/.");
#ifndef R5A
        returncode = pathconf (location, _PC_NAME_MAX);
#else
        returncode = MAXNAMLEN;
#endif
        p = odm_set_path (local);
	if (p > (char *) NULL)
		free(p);
	if (local > (char *) NULL)
		free(local); /* SMU Defect 51665 */

        return (returncode);
}
