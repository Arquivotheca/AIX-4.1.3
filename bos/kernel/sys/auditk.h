/* @(#)69     1.22  src/bos/kernel/sys/auditk.h, syssaud, bos411, 9428A410j 2/24/94 16:55:11 */
/*
 * COMPONENT_NAME: (SYSSAUD) Auditing Management
 *
 * FUNCTIONS: auditk.h for audit kernel definitions
 *
 * ORIGINS: 27 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef _H_AUDITK
#define _H_AUDITK

#include	"audit.h"
#include	<sys/sleep.h>
#include	<sys/types.h>
#include	<sys/lockl.h>

/*
 * this file describes auditing structures private to the kernel
 */

#define	MAX_ANAMES	32
#define	ALL_CLASS	31
#define	ALL_SIZE	sizeof (struct audit_class) + 5
#define MAX_EVNTSIZ	16
#define MAX_EVNTNUM	16
#define MAX_PATHSIZ	256
#define HASHLEN 	256
#define MAX_TABSIZ 	48*1024

/* this is the format of base event entries in the kernel symbol table */
struct	base_events{
	char	be_name[16];
	ulong_t	be_bitmap;
	struct	base_events	*be_next;
	struct	base_events	*be_prev;
};

struct audit_anchor_t {
	int	lock;
	int 	error;
};

extern  Simple_lock	audit_lock;
extern  Simple_lock	audit_obj_lock;
extern	struct	base_events	*be_symtab;
extern	struct	file	*audit_bin;
extern	struct	file	*audit_next;
extern	struct  base_events *hashtab[HASHLEN];
extern  struct	audit_anchor_t audit_anchor;
extern	long	audit_threshold;
extern	long	audit_size;
extern	int	audit_flag;
extern	int	be_total_len;
extern	int	audit_panic;
extern  int     audobj_block;
extern  int     auditevent_block;
extern 	int     oevent_total;
extern 	int     object_total;
extern	int	nevents;
extern	int   	cevent;
extern	int	(*auditdev)(int, char *, int);
extern 	char	*audit_getname();
extern	char	class_names[MAX_ANAMES][16];
extern	void	prochadd();
extern 	void	prochdel();

#define	audit_is_on(id) \
	(be_symtab[(id)].be_bitmap & U.U_procp->p_auditmask)

#define audit_svc(){\
struct uthread *ut = curthread->t_uthreadp;\
if (ut->ut_audsvc) \
        if((ut->ut_audsvc)->svcnum){\
		simple_lock(&audit_lock);\
                if(U.U_procp->p_auditmask & be_symtab[(ut->ut_audsvc)->svcnum].be_bitmap) {\
			simple_unlock(&audit_lock);\
                        audit_svcfinis(); \
		} else\
			simple_unlock(&audit_lock);\
                ut->ut_audsvc->svcnum = 0; \
        } \
}

/* generate an "arbitrary" audit record from within the kernel */
#define	_auditlog(event,result,buf,len) {\
		if(audit_flag & AUDIT_ON) {\
			static int	_id; \
			if(_id == 0)_id = audit_klookup(event); \
			if((_id > 0) && audit_is_on(_id)) \
				audit_write(_id, result, buf, len); \
		} \
	}

#define	TCBMOD	 1
#define	TCBLEAK  2
#define	PRIVFAIL 3
#define	PRIVUSE  4

#endif /* _H_AUDITK */
