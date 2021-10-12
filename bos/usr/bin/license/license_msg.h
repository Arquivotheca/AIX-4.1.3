/* @(#)68       1.3.1.3  src/bos/usr/bin/license/license_msg.h, cmdlicense, bos411, 9430C411a 7/21/94 17:00:54 */
/*
 *   COMPONENT_NAME: CMDLICENSE
 *
 *   FUNCTIONS: catgets
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <limits.h>
#include <nl_types.h>
#include <locale.h>
#define CATFILE "license.cat"

#define CHANGE_SET      1
#define CHANGE_USAGE 1
#define CHANGE_NUM   2
#define CHANGE_FLOAT 3
#define CHANGE_REBOOT 4

#define LIST_SET        2
#define LIST_USAGE   1

#define LICENSE_SET     3
#define LICENSE_OPEN  1
#define LICENSE_CREATE 2
#define LICENSE_RW     3
#define LICENSE_STAT   4
#define LICENSE_MALLOC 5
#define LICENSE_READ   6
#define LICENSE_EQ     7
#define LICENSE_SEEK   8
#define LICENSE_WRITE  9
#define LICENSE_MAXL   10
#define LICENSE_FNUM   11
#define LICENSE_BNUM   12
#define LICENSE_INVAL	13
#define LICENSE_INITTAB 14
#define LICENSE_ENABLE 15
#define LICENSE_DISABLE 16
#define LICENSE_MODE_ON  17
#define LICENSE_MODE_OFF 18
#define LICENSE_TITLE 19

#ifdef R5A
#define catgets(a,b,c,d) d
#endif
