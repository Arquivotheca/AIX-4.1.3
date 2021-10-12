static char sccsid[] = "@(#)32        1.11.1.2  src/bos/kernel/s/aud/globals.c, syssaud, bos411, 9428A410j 2/24/94 17:06:09";

/*
 * COMPONENT_NAME: (SYSSAUD) Auditing Management
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include        <sys/types.h>
#include        <sys/user.h>
#include        <sys/errno.h>
#include        <sys/file.h>
#include        <sys/systm.h>
#include        <sys/priv.h>
#include        <sys/auditk.h>
#include        <sys/audit.h>
#include        <sys/lockl.h>

Simple_lock	audit_obj_lock;
Simple_lock     audit_lock;

int	audit_flag = AUDIT_OFF;	
int	audit_panic;	

struct	file	*audit_bin = NULL;
struct	file	*audit_next = NULL;
struct  audit_anchor_t 	audit_anchor = {EVENT_NULL, 0};
long	audit_size = 0;
long	audit_threshold;

struct	base_events	*be_symtab;	
struct	base_events	*hashtab[HASHLEN];
int	auditevent_block = 0;
int	audobj_block = 0; 
int	oevent_total = 0;	
int	object_total = 0;	
int	cevent; 	
int	nevents; 	
char	class_names[MAX_ANAMES][16];
int	be_total_len = 0;

