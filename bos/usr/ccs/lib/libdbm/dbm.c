static char sccsid[] = "@(#)48	1.9  src/bos/usr/ccs/lib/libdbm/dbm.c, libdbm, bos411, 9428A410j 2/2/94 19:31:22";
/* 
 * COMPONENT_NAME: (LIBDBM) Data Base Management Library
 * 
 * FUNCTIONS: dbmclose, dbminit, delete, fetch, firstkey, forder, 
 *            nextkey, store 
 *
 * ORIGINS: 24 26
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* 
 * #ifndef lint
 * static char sccsid[] = "(#)dbm.c	5.3 (Berkeley) 85/08/15";
 * #endif 
 */

#include	"dbm.h"

#include	<nl_types.h>
#include	<limits.h>
#include 	"libdbm_msg.h"

#define	NODB	((DBM *)0)

static DBM *cur_db = NODB;

static char no_db[] = "dbm: no open database\n";

char *
getmsg(set_no, msg_no, def_msg)
long set_no;
long msg_no;
char *def_msg;

{
	nl_catd   catd;    	/* file descriptor for message catalog */
	char 	  *msg; 	/* message string returned from getmsg */
	static char buffer[NL_TEXTMAX];

	/* open the message catalog */
	catd = catopen(MF_LIBDBM, NL_CAT_LOCALE);
	/* get a the requested message */
	msg = catgets(catd, set_no, msg_no, def_msg);
	/* close the catalog */
	strcpy(buffer,msg);
	catclose(catd);
 	return(buffer);
}

dbminit(file)
	char *file;
{
	if (cur_db != NODB)
		dbm_close(cur_db);

	cur_db = dbm_open(file, 2, 0);
	if (cur_db == NODB) {
		cur_db = dbm_open(file, 0, 0);
		if (cur_db == NODB)
			return (-1);
	}
	return (0);
}

long
forder(key)
datum key;
{
	if (cur_db == NODB) {
		printf(getmsg(LIBDBM,DBM1,no_db));
		return (0L);
	}
	return (dbm_forder(cur_db, key));
}

datum
fetch(key)
datum key;
{
	datum item;

	if (cur_db == NODB) {
		printf(getmsg(LIBDBM,DBM1,no_db));
		item.dptr = 0;
		return (item);
	}
	return (dbm_fetch(cur_db, key));
}

delete(key)
datum key;
{
	if (cur_db == NODB) {
		printf(getmsg(LIBDBM,DBM1,no_db));
		return (-1);
	}
	if (dbm_rdonly(cur_db))
		return (-1);
	return (dbm_delete(cur_db, key));
}

store(key, dat)
datum key, dat;
{
	if (cur_db == NODB) {
		printf(getmsg(LIBDBM,DBM1,no_db));
		return (-1);
	}
	if (dbm_rdonly(cur_db))
		return (-1);

	return (dbm_store(cur_db, key, dat, DBM_REPLACE));
}

datum
firstkey()
{
	datum item;

	if (cur_db == NODB) {
		printf(getmsg(LIBDBM,DBM1,no_db));
		item.dptr = 0;
		return (item);
	}
	return (dbm_firstkey(cur_db));
}

datum
nextkey(key)
datum key;
{
	datum item;

	if (cur_db == NODB) {
		printf(getmsg(LIBDBM,DBM1,no_db));
		item.dptr = 0;
		return (item);
	}
	return (dbm_nextkey(cur_db));
}

#ifdef _AIX
/* 
 *  Added for portablility of applications from systems 
 *  which use dbmclose()
 */

dbmclose()
{
	return(0);
}

#endif /* _AIX  */


