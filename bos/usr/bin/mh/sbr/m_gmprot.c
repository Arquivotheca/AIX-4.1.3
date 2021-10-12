static char sccsid[] = "@(#)73	1.3  src/bos/usr/bin/mh/sbr/m_gmprot.c, cmdmh, bos411, 9428A410j 6/15/90 22:13:19";
/* 
 * COMPONENT_NAME: CMDMH m_gmprot.c
 * 
 * FUNCTIONS: m_gmprot 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* m_gmprot.c - return the msg-protect value */

#include "mh.h"
#include <stdio.h>


m_gmprot () {
    register char  *cp;

    return atooi ((cp = m_find ("msg-protect")) && *cp ? cp : msgprot);
}
