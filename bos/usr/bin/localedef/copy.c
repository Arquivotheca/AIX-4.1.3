static char sccsid[] = "@(#)09	1.2  src/bos/usr/bin/localedef/copy.c, cmdnls, bos411, 9428A410j 3/10/94 10:57:30";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 85
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 * 
 * RCSfile: copy.c,v $ $Revision: 1.1.2.2 $ (OSF) $Date: 1992/08/10 14:43:44 
 */

#include <locale.h>
#include <langinfo.h>
/*#include <sys/localedef.h>*/
#include "locdef.h"
#include "semstack.h"
#include "err.h"

int 	copying_ctype = 0;	/* are we copying a ctype table?     */
int 	copying_collate = 0;	/* are we copying a collation table? */
int	copying = 0;		/* general flag to indicate copying  */

extern _LC_collate_t	*collate_ptr;
extern _LC_ctype_t	*ctype_ptr;
extern _LC_monetary_t	*monetary_ptr;
extern _LC_numeric_t	*numeric_ptr;
extern _LC_time_t	*lc_time_ptr;
extern _LC_resp_t	*resp_ptr;
extern _LC_aix31_t	lc_aix31;

/*
 * Copy_locale  - routine to copy section of locale input files
 * 		  from an existing, installed, locale.
 *
 * 	  We reassign pointers so gen() will use the existing structures.
 */

void
copy_locale(int category)
{
	char *ret;
	item_t	*it;
	char *source;		/* user provided locale to copy from */
	char *orig_loc;		/* orginal locale */

	it = sem_pop();
	if (it->type != SK_STR)
		INTERNAL_ERROR;
	source = it->value.str;

	orig_loc = setlocale(category, NULL);
	if ((ret = setlocale(category, source)) == NULL) 
		error(CANT_LOAD_LOCALE, source);

	copying = 1;			/* make sure gen() puts out C code */
	switch(category) {

		case LC_COLLATE:
		collate_ptr = (_LC_collate_t *) __OBJ_DATA(__lc_collate);
		copying_collate = 1;
		break;

		case LC_CTYPE:
		ctype_ptr = (_LC_ctype_t *) __OBJ_DATA(__lc_ctype);
		copying_ctype = 1;		/* to use max_{upper,lower} */
		break;

		case LC_MONETARY:
		monetary_ptr = (_LC_monetary_t *) __OBJ_DATA(__lc_monetary);
		break;

		case LC_NUMERIC:
		numeric_ptr = (_LC_numeric_t *) __OBJ_DATA(__lc_numeric);
		break;

		case LC_TIME:
		lc_time_ptr = (_LC_time_t *) __OBJ_DATA(__lc_time);

#ifdef _AIX
		/* update AIX31 time values also */
		lc_aix31.nlldate  = __OBJ_DATA(__lc_locale)->nl_info[NLLDATE];
		lc_aix31.nltmisc  = __OBJ_DATA(__lc_locale)->nl_info[NLTMISC];
		lc_aix31.nltstr   = __OBJ_DATA(__lc_locale)->nl_info[NLTSTRS];
		lc_aix31.nltunits = __OBJ_DATA(__lc_locale)->nl_info[NLTUNITS];
		lc_aix31.nlyear   = __OBJ_DATA(__lc_locale)->nl_info[NLYEAR];
#endif
		break;

		case LC_MESSAGES:
		resp_ptr = (_LC_resp_t *) __OBJ_DATA(__lc_resp);
		break;
	} /* switch */

	ret = setlocale(category, orig_loc);
}
