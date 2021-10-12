static char sccsid[] = "@(#)78	1.1  src/bos/usr/ccs/lib/libs/pw_hist2.c, libs, bos411, 9428A410j 4/19/94 18:06:57";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: _delete_PWDHistory
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <string.h>
#include <fcntl.h>
#include <ndbm.h>
#include <sys/stat.h>
#include "libs.h"


/*
 * NAME: _delete_PWDHistory
 *                                                                    
 * FUNCTION:  If a history of passwords exist for the user, this routine will
 * 	      delete the history record.
 *
 * RETURNS:   
 *		-1 Internal Error
 *		 0 Success
 *		
 * Called by _PWDHistoryMaint - to remove the history list if history
 *                              checking has be disabled and the user has
 *                              old password history.
 *
 *           deletefile() from libs_write.c so that rmuser -p will
 *	                          delete the password history.
 *
 *           This subroutine is in a separate file from the other
 *           PWDHistory code to prevent the inclusion of the
 *           __crypt(), __setkey(), and __encrypt() libdes.a subroutines 
 *           into libc.a.  libc.a indirectly calls _delete_PWDHistory()
 *           and was requiring libdes.a to be linked into code that
 *           never needed it before the addition of the PWDHistory code.
 *
 */  
int
_delete_PWDHistory( char *uname )
{

    struct stat 	sbuf;
    datum 		key, data;
    DBM 		*db;
    int 		rc = 0;
	
	if (stat(HISTPGFILE, &sbuf) == 0 && 
	       ( (db = dbm_open(HISTFILE, O_RDWR, 0600)) != (DBM *)0))
		 
	{	
		key.dptr = uname;
		key.dsize = strlen(uname);

		data = dbm_fetch(db, key);

		if (data.dptr != (char *)NULL )
		{
		   	if (_writelock(dbm_dirfno(db)) == -1)    
		   	{
				dbm_close(db);
				return -1;
		   	} 

			rc = dbm_delete(db, key);
			_writeunlock(dbm_dirfno(db));
		}

		dbm_close(db);
	}

	return rc;
}
