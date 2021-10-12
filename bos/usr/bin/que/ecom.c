static char sccsid[] = "@(#)24  1.7  src/bos/usr/bin/que/ecom.c, cmdque, bos411, 9428A410j 4/17/91 10:12:49";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
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

/* Routines shared between enq and qstatus but not used by qdaemon.	*/
/* 	All for default queue stuff*/
 
#include <fcntl.h>
#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>
#include <IN/standard.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <IN/DRdefs.h>
#include <sys/stat.h>
#include "common.h"
#include "enq.h"

#include <IN/backend.h>
#include <sys/vmount.h>


/******************************************************
	SEARCH QUEUE LIST FOR A PARTICULAR NAME
******************************************************/
struct q *get_queue(gq_qname,gq_qlist)
char		*gq_qname;
struct q	*gq_qlist;
{
	register struct q *gq_thisq;

	for (gq_thisq = gq_qlist; gq_thisq; gq_thisq = gq_thisq->q_next) 
		if (!strncmp(gq_qname,gq_thisq->q_name,QNAME))
			break;
	return(gq_thisq);
}

/***********************************************
	SEARCH FOR DEFAULT PRINTER QUEUE
***********************************************/
struct q *default_queue(dq_qlist)
struct q	*dq_qlist;
{
	register char *dq_qname;

	if ((dq_qname = getenv("LPDEST")) || (dq_qname = getenv("PRINTER")))
		return(get_queue(dq_qname,dq_qlist));
	else
		return(dq_qlist);
} 
