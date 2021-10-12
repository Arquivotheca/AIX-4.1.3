/* @(#)87	1.2  src/bos/usr/bin/newgrp/lsgratab.c, cmdsuser, bos41J, 9512A_all 3/14/95 15:55:16 */
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: sizeof
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tcbauth.h"

struct	lsgrattr	lsgratab [ ] =
{
	{ S_ID,			SEC_INT,	NULL},

	{ S_ADMIN,		SEC_BOOL,	chgbool},

	{ S_USERS,		SEC_LIST,	chglist},

	{ S_ADMS,		SEC_LIST,	chglist},

	{ S_GRPEXPORT,		SEC_BOOL,	chgbool},
};

int lsgratabsiz = sizeof(lsgratab)/sizeof(struct lsgrattr);
