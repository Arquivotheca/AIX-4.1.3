/* @(#)22       1.9.1.6  src/bos/kernel/sys/cred.h, syssauth, bos411, 9428A410j 3/28/94 03:02:52 */
/*
 *   COMPONENT_NAME: SYSSAUTH
 *
 *   FUNCTIONS: crhold
 *
 *
 *   ORIGINS: 3,9,24,26,27,83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) 1988 by Sun Microsystems, Inc.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef _H_CRED
#define _H_CRED


#include <sys/limits.h>
#include <sys/types.h>
#include <sys/priv.h>
#include <sys/lockl.h>

#define	SHORT_NGROUPS	NGROUPS_MAX

struct ucred {
	int	cr_ref;			/* reference count */
	/* user ID's - managed by {gs}etuidx() */
	uid_t   cr_ruid;		/* real user id */
	uid_t   cr_uid;			/* effective user id */
	uid_t   cr_suid;		/* saved user id */
	uid_t   cr_luid;		/* login user id */
	uid_t   cr_acctid;		/* accounting id */
	/* group ID's - managed by {gs}etgidx() */
	gid_t   cr_gid;			/* effective group id */
	gid_t   cr_rgid;		/* real group id */
	gid_t   cr_sgid;		/* saved group id */
	/* concurrent group set - managed by {gs}etgroups() */
	short	cr_ngrps;		/* how many groups in list */
	gid_t   cr_groups[NGROUPS_MAX];	/* short list of groups */

	/*
	 * max privileges
	 * at exec(), this is: old effective + old bequeathed
	 */
	priv_t	cr_mpriv;

	/*
	 * inherited privileges
	 * at exec(), this is: old bequeathed
	 */
	priv_t	cr_ipriv;

	/*
	 * current privileges
	 * at exec(), this is: old bequeathed + program's
	 */
	priv_t	cr_epriv;

	/*
	 * bequeathed privileges
	 * at exec(), this is: old bequeathed + program's
	 */
	priv_t	cr_bpriv;

	long    cr_pag;                 /* AFS process authentication group */
};


void crlock(void);
void crunlock(void);
void credlock(void);
void credunlock(void);
void crfree(struct ucred *);
struct ucred *crref(void);
struct ucred *crget(void);
struct ucred *crcopy(struct ucred *);
struct ucred *crdup(struct ucred *);
void crset(struct ucred *);
void crhold(struct ucred *);

#endif /* _H_CRED */
 
