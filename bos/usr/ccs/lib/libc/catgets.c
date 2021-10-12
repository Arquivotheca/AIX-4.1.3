static char sccsid[] = "@(#)09	1.24  src/bos/usr/ccs/lib/libc/catgets.c, libcmsg, bos41J 3/21/95 10:44:26";
/*
 * COMPONENT_NAME: (LIBCMSG) LIBC Message Catalog Functions
 *
 * FUNCTIONS: catgets, read_msg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <nl_types.h>
#include <errno.h>
#include "catio.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _catalog_rmutex;
#endif /* _THREAD_SAFE */


static	char *read_msg(nl_catd, int, int, char *);

/*
 * NAME: catgets
 *                                                                    
 * FUNCTION: Get a pointer to a message from a message catalogue.
 *
 * ARGUMENTS:
 *	catd		- catalog descripter obtained from catopen()
 *	setno		- message catalogue set number
 *	msgno		- message numner within setno
 *	def		- default message text
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	catgets executes under a process.
 *
 * NOTE: Shared message file support could be enabled at some later time.
 *
 * RETURNS: Returns a pointer to the message on success.
 *	On any error, the default string is returned.
 *
 */  

char *catgets(nl_catd catd, int setno, int msgno, const char *def)
{
	int	err;	/* preserve errno for caller */
	int	pid;	/* pid of current process */
	char	*mp;	/* return message text ptr */

	/*
	 * return default text if catd is invalid or no message catalogue
	 */
	if (catd == CATD_ERR || catd == NULL || catd->_fd == FILE_UNUSED) 
		return(def);

	TS_LOCK(&_catalog_rmutex);

	TS_PUSH_CLNUP(&_catalog_rmutex);
	/*
	 * perform deferred open now
	 * return default text if no message catalogue file
	 * if catd is from parent, redefine catd for the child
	 */
	err = errno;
	if (catd->_fd == NULL) {
		if ( _cat_do_open(catd) == 0) {
			catd->_fd = FILE_UNUSED;
			errno = err;
			TS_POP_CLNUP(0);
			TS_UNLOCK(&_catalog_rmutex);
			return (def);
		}
		catd->_pid = getpid();
	}
	else if ((pid = getpid()) != catd->_pid) {
		catd->_pid = pid;
		fclose(catd->_fd);
		catd->_fd = _cat_openfile(catd->_name);
		if (catd->_fd == FILE_UNUSED) {
			catd->_fd = FILE_DUMMY;
			_cat_hard_close(catd);
			errno = err;
			TS_POP_CLNUP(0);
			TS_UNLOCK(&_catalog_rmutex);
			return (def);
		}
	}
#ifdef _FUTURE_MAPPED
	if (catd->_mem) {	/*----  for mapped files ----*/
		if (setno <= catd->_hd->_setmax) {
			if (msgno <= catd->_set[setno]._n_msgs) {
				if (catd->_set[setno]._mp[msgno]._offset) {
					RETURN(catd->_mem + 
                                        catd->_set[setno]._mp[msgno]._offset);
				}
			}
		}
		RETURN(def);
	}
	else {	/*---- for unmapped files ----*/
#endif


		mp = read_msg(catd, setno, msgno, def);
		TS_POP_CLNUP(0);
		errno = err;
		TS_UNLOCK(&_catalog_rmutex);
		return (mp);
#ifdef _FUTURE_MAPPED
	}
#endif
}

/*
 * NAME: read_msg
 *
 * FUNCTION: Reads a message from an unmapped file and places the text in a
 *      static buffer.
 *
 * ARGUMENTS:
 *      catd            - catalogue descripter returned from catopen()
 *      setno           - message set number
 *      msgno           - message number within set
 *	def		- default message text
 *
 * EXECUTION ENVIRONMENT:
 *      Executes under a process.
 *
 * RETURNS: default message text pointer on any error.
 *      Pointer to catalogue message text if success.
 *
 */

static char *read_msg(nl_catd catd, int setno, int msgno, char *def)
{ 
        struct _msgptr *mpt;	/* pointer to message offset */
        char	**mt;		/* pointer to message text pointer */
     
	if (setno > 0 && setno <= catd->_hd->_setmax) {
		if (msgno > 0 && msgno <= catd->_set[setno]._n_msgs) {
			mpt = &catd->_set[setno]._mp[msgno];
			if (mpt->_offset != 0) {
				mt = &catd->_set[setno]._msgtxt[msgno];
				if (*mt == NULL) {
					if ((*mt = (char *)malloc(mpt->_msglen + 1)) == NULL)
				 	     	return(def);
					fseek(catd->_fd, (long)mpt->_offset, 0); 
					if (fread((void *)*mt, (size_t)(mpt->_msglen+1), (size_t)1, catd->_fd) != 1) {
						free(*mt);
						*mt = NULL;
						return(def);
					}
				}
				return(*mt);
			}
		}
	}
	return(def);
}
