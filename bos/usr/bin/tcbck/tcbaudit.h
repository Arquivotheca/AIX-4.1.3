/* @(#)68	1.1  src/bos/usr/bin/tcbck/tcbaudit.h, cmdsadm, bos411, 9428A410j 3/12/91 18:34:29 */
/*
 *
 * COMPONENT_NAME: (CMDSADM) sysck - system configuration checker
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 * This file contains the external declarations for all of the
 * auditing strings used by SYSCK.
 */

extern	char	*SYSCK_Check;
extern	char	*SYSCK_Update;
extern	char	*SYSCK_Install;
extern	char	*SYSCK_Cfg;
extern	char	*Event;

extern	char	*Fixed;
extern	char	*NotFixed;
extern	char	*CantFix;

/*
 * Declare the routines for creating audit records
 */

#ifdef	_NO_PROTO
extern	void	mk_audit ();
extern	void	mk_vaudit ();
extern	void	mk_audit_rec ();
#else
extern	void	mk_audit (char *, int, char *, char *, char *);
extern	void	mk_vaudit (char *, int, char *, char *, char *, ...);
extern	void	mk_audit_rec (char *, int, ...);
#endif
