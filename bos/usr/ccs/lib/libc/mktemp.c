static char sccsid[] = "@(#)64	1.13  src/bos/usr/ccs/lib/libc/mktemp.c, libcenv, bos412, 9448A 12/1/94 16:11:28";
/*
 *   COMPONENT_NAME: libcenv
 *
 *   FUNCTIONS: mktemp
 *
 *   ORIGINS: 3,27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
$RCSfile: mktemp.c,v $ $Revision: 2.14.2.5 $ (OSF) $Date: 1992/02/27 21:40:29 $";
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/times.h>
#include <ts_supp.h>

#ifdef _THREAD_SAFE
#include <lib_lock.h>
#include <rec_mutex.h>
extern struct rec_mutex		_mktemp_rmutex;
extern lib_lock_functions_t	_libc_lock_funcs;
#endif /* _THREAD_SAFE */

/* '.' is used, but care is taken to make sure that it is never the first
   character of a filename, i.e. if only "XXXXXX" is passed in the resulting
   name would not begin with '.'
*/
#define	LETTERS	\
	"abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ_."
/*---------------------------------------------------------------------------*
 * Synopsis	: generate a unique filename
 * Parameters	:
 *		inout	template for filename
 * Return	: template if name cannot be generated
 * Fatal Errors	: none
 * Description	:
 *		Replace the trailing 'X's in the template string with
 *		a unique filename using a letter and an id.
 *		If all permutations are used return template and set the first
 *		element of the template to NULL.
 * Notes	:
 *---------------------------------------------------------------------------*/
char *
mktemp(char *template)
{
	/*
	 * This static is locked by the _mktemp_rmutex
	 */
	static unsigned short selector=0;
	struct  tms tms;
	unsigned long random;
	char	*last = template;
	char	*first;
	char	*s0;
	long long int	id;
	int	save_errno;
	unsigned short int xsubi[3];

	/*
	 * check for NULL
	 */
	if (template == NULL || template[0] == '\0')
		return (template);

	/*
	 * Save off errno in case access() changes it
	 */
	save_errno = errno;

	/*
	 * check for trailing 'X'
	 */
	if (*(last = template + strlen(template) - 1) != 'X') {
		if (access(template, F_OK) == 0)
			*template = '\0';
		errno = save_errno;
		return (template);
	}

	TS_LOCK(&_mktemp_rmutex);
	id = (long long)times(&tms) << 24  | (long)getpid() << 8 | selector++;
	TS_UNLOCK(&_mktemp_rmutex);

	/*
	 * only change upto 6 X's
	 */
	for (s0 = last; s0 >= template && *s0 == 'X' && s0>last-6; s0--) {
		first = s0;
		*s0 = LETTERS[id & 0x3f];
		id >>= 6;
	}
	/*
	 * make sure we don't put a '.' as the 
	 * first character in the file name
	 */
	if (first == template && *first == '.')
		*first = '_';

	if (access(template, F_OK) == -1) {
		errno = save_errno;
		return (template);
	}

	/*
	 * loop trying names with different letters
	 */
	for (s0 = LETTERS; *first = *s0; s0++) {
		if (access(template, F_OK) == -1) {
			/*
			 * Make sure that the file name does not start
			 * with '.', if so, do not use it
			 */
			if (*first == '.' && first == template)
				continue;
			errno = save_errno;
			return (template);
		}
	}

	/*
	 * all names used
	 */
	errno = save_errno;
	*template = '\0';
	return (template);
}
