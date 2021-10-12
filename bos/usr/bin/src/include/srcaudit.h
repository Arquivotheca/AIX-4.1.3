/* @(#)97	1.1  src/bos/usr/bin/src/include/srcaudit.h, cmdsrc, bos411, 9428A410j 11/10/89 15:35:41 */
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


#ifndef _H_SRCAUDIT
#define _H_SRCAUDIT
	
/* auditlog events */
#define AUDIT_SRC_START		"SRC_Start"
#define AUDIT_SRC_STOP		"SRC_Stop"
#define AUDIT_SRC_ADDSSYS	"SRC_Addssys"
#define AUDIT_SRC_CHSSYS	"SRC_Chssys"
#define AUDIT_SRC_DELSSYS	"SRC_Delssys"
#define AUDIT_SRC_ADDSERVER	"SRC_Addserver"
#define AUDIT_SRC_CHSERVER	"SRC_Chserver"
#define AUDIT_SRC_DELSERVER	"SRC_Delserver"

#endif
