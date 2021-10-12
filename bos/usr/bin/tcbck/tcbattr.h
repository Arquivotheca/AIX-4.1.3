/* @(#)64	1.1  src/bos/usr/bin/tcbck/tcbattr.h, cmdsadm, bos411, 9428A410j 3/12/91 18:31:34 */
/*
 *
 * COMPONENT_NAME: (CMDSADM) sysck - system configuration checker
 *
 * FUNCTIONS: NONE
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
 */

/*
 * Definitions to be included by programs that use "Attribute Files".
 * stdio.h must be included ahead of this include file.
 */

#define	MAXSTSIZ	4096
#define	MAXATSIZ	512
#define	MAXATTR		128

struct stanza {
	char	*at_name;
	char	*at_value;
};

struct	stanza	*read_stanza ();

#define	SYSCK_NAME	"/etc/security/sysck.cfg"
#define	OSYSCK_NAME	"/etc/security/osysck.cfg"
