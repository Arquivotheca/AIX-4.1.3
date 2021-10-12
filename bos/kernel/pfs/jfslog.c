static char sccsid[] = "@(#)28	1.2  src/bos/kernel/pfs/jfslog.c, syspfs, bos411, 9428A410j 12/6/93 16:55:08";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfslogadd, jfslogdel
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "sys/types.h"
#include "sys/user.h"
#include "sys/malloc.h"
#include "sys/lockl.h"
#include "sys/errno.h"

#include "jfs/log.h"

/*
 * NAME:  jfslogadd(logdata, len)
 *
 * FUNCTION: 	setup jfs log information for the current user.  
 *
 *		setup consists allocating memory for a loginfo
 *		structure and log data buffer, copying the specified
 *		log data to the buffer, and updating the current
 *		user's loginfo pointer (located in the ublock) to
 *		reflect the existence of loginfo.
 *
 *		jfs commit processing (finicom()) will add the specified
 *		log data to any COMMIT records which are written to the
 *		log due to file system activity performed under the user.
 *
 * PARAMETERS:
 *		logdata	- pointer to data to be added to the log
 *		len	- len of logdata
 *
 * RETURN VALUES:
 *		0	- successful
 *		EINVAL	- invalid length
 *		EEXIST	- loginfo already exists
 *
 */

int
jfslogadd(logdata, len)
char *logdata;
int len;
{
	char *ptr;
	struct loginfo *lp;
	void **ulpp;

	/* check if the len of the logdata
	 * is valid.
	 */
	if (len <= 0 || len > MAXLINFOLEN)
		return(EINVAL);

	/* check if loginfo already exists.
	 */
	ulpp = &u.u_loginfo;

	if (*ulpp != NULL)
		return(EEXIST);

	/* get memory for the loginfo structure and 
	 * data buffer.
	 */
	if ((ptr = (char *) malloc(len + sizeof(struct loginfo))) == NULL)
		return(ENOMEM);

	/* setup loginfo structure and copy logdata to
	 * the buffer.
	 */
	lp = (struct loginfo *) ptr;
	lp->li_len = len;
	lp->li_buf = ptr + sizeof(struct loginfo);
	bcopy(logdata,lp->li_buf,lp->li_len);

	/* update ublock loginfo pointer.
	 */
	*ulpp = (void *)lp;
	
	return(0);

}

/*
 * NAME:  jfslogdel()
 *
 * FUNCTION:	delete jfs log information for the current user.  free
 *		memory allocated for loginfo struct and log data buffer.
 *		update the current user's loginfo pointer (ublock) to
 *		reflect the deletion of loginfo. 
 *
 * PARAMETERS:
 *		NONE
 *
 * RETURN VALUES:
 *		0	- successful
 *		EINVAL	- loginfo does not exist
 *
 */

int
jfslogdel()
{
	void **ulpp;

	/* check if loginfo exists.
	 */
	ulpp = &u.u_loginfo;

	if (*ulpp == NULL)
		return(EINVAL);
	
	/* free loginfo memory and NULL the
	 * loginfo pointer in the ublock.
	 */
	free(*ulpp);
	*ulpp = NULL;

	return(0);
}
