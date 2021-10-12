static char sccsid[] = "@(#)84	1.4  src/bos/usr/ccs/lib/libqb/log.c, libqb, bos411, 9428A410j 10/16/90 15:22:40";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: log_init, log_status, log_progress, log_percent, log_pages,
 *            log_change, get_job_number, get_backend, get_copies
 *
 * ORIGINS: 9, 27
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

/*
 * top-level routines for queueing system backends, except
 *      routines to do burst pages.
 * see "How to Write a Friendly Backend" for docs.
 */

#include <varargs.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <IN/stfile.h>

struct stfile __stfile;
static char __sfname[150];

#define STFD    3       /* file descriptor for status file */

extern long lseek();

log_init(name)
	char *name; {
	return (sfget());
}


log_status(stat) {
	if ((stat > 0) && (stat <= STATMAX))
	{
		__stfile.s_status = stat;
		return (sfput());
	}
	return (0);
}

log_progress(pages, percent) {
	log_pages(pages);
	log_percent(percent);
	return (sfput());
}

log_percent(percent ) {
	__stfile.s_percent = percent;
	return (sfput());
}

log_pages(pages ) {
	__stfile.s_pages = pages;
	return (sfput());
}


log_charge(charge)
	long charge; {
	__stfile.s_charge = charge;
	return (sfput());
}



get_job_number(){
	if (!__stfile.s_jobnum) return (-1);       /* safety first */
		return (__stfile.s_jobnum);
}

get_backend(){
	if (getenv("QUEUE_BACKEND"))
		return (TRUE);
	else
		return (FALSE);
	
}

get_copies(){
	if (!__stfile.s_copies) return (1);       /* safety first */
	return (__stfile.s_copies);
}

static sfget() {
	register int fd;

	if (lseek(STFD, 0L, 0) < 0)
		goto error;
	if (read(STFD, &__stfile, sizeof(__stfile)) != sizeof(__stfile))
		goto error;
	return (0);
error:
	return (-1);
}

static sfput() {
	int fd;

	if (lseek(STFD, 0L, 0) < 0)
		goto error;
	if (write(STFD, &__stfile, sizeof(__stfile)) != sizeof(__stfile))
		goto error;
	return (0);
error:
	return (-1);
}
