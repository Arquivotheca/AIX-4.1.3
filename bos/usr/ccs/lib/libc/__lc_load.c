static char sccsid[] = "@(#)53	1.3.1.5  src/bos/usr/ccs/lib/libc/__lc_load.c, libcloc, bos411, 9428A410j 1/12/94 11:32:49";
/*
 * COMPONENT_NAME: (LIBCLOC) LIBC Locale functions
 *
 * FUNCTIONS: __lc_load, __issetuid
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/id.h>

/*
*  FUNCTION: __issetuid
*
*  DESCIPTION:
*  Determines if the current program is running in a secure context outside
*  that of the user executing the program.
*/
int __issetuid(void)
{
    uid_t suid, euid, ruid;
    gid_t sgid, egid, rgid;
    
    suid = getuidx(ID_SAVED); 
    ruid = getuidx(ID_REAL);
    euid = getuidx(ID_EFFECTIVE);
 
    sgid = getgidx(ID_SAVED); 
    rgid = getgidx(ID_REAL);
    egid = getgidx(ID_EFFECTIVE);

    /* check if privilege acquired somehow */
    if (euid != ruid || egid != rgid)
	return TRUE;
    /* check that they can't get back from whence they came. */
    else if (suid != ruid || sgid != rgid)
	return TRUE;
    else
	return FALSE;
}

	
/*
*  FUNCTION: __lc_load
*
*  DESCIPTION:
*  The function loads an object file from 'path'.  The user supplied
*  instantiate function is always executed if it and path are specified.
*/
void * __lc_load(const char *path, void *(*instantiate)())
{

    void *p;

    if (path == NULL)
	return NULL;

    /* load specified object */
    p = (void *)load(path, 0, NULL);
    

    /* Invoke the instantiate method if it is not null.                      */
    /* The instantiate method is responsible for verifying the validity      */
    /* of the object.  (ie checking _LC_MAGIC, major/minor version #'s, etc) */
    /* Instantiate may be passed a NULL value if the specified object does   */
    /* not exist.                                                            */

    if (instantiate != NULL) {
	
	/* invoke the caller supplied instantiate method */
	p = (void *)(*instantiate)(path,p);
    }    

    return p;
}
