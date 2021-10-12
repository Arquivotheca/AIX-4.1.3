static char sccsid[] = "@(#)85	1.5  src/bos/usr/ccs/lib/libqb/logaux.c, libqb, bos411, 9428A410j 5/24/91 16:09:30";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: get_align, get_was_idle, get_feed, get_from, get_header, get_qdate,
 *            get_title, get_to, get_trailer, get_mail_only, get_queue_name, 
 *            get_device_name, get_cmd_line
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
 * low-level routines available to queueing system backends
 * see "How to Write a Friendly Backend" for docs
*/

#include <IN/standard.h>
#include <IN/backend.h>
#include <IN/stfile.h>

extern struct stfile __stfile;

get_align(){ return (__stfile.s_align); }

get_was_idle(){ return (__stfile.s_was_idle); }

get_feed(){ return (__stfile.s_feed); }

char *
get_from(){ return (__stfile.s_from); }

get_header(){ return (__stfile.s_head); }

char *
get_qdate(){ return (__stfile.s_qdate); }

char *
get_title(){ return (__stfile.s_title); }

char *
get_to(){ return (__stfile.s_to); }

get_trailer(){ return (__stfile.s_trail); }

get_mail_only(){ return (__stfile.s_mailonly); }

char *
get_queue_name(){ return (__stfile.s_queue_name); }

char *
get_device_name(){ return (__stfile.s_device_name); }

char *
get_cmd_line(){ return (__stfile.s_cmdline); }
