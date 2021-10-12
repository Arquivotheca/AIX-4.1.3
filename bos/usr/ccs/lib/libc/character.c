static char sccsid[] = "@(#)21	1.1.1.1  src/bos/usr/ccs/lib/libc/character.c, libcstr, bos411, 9428A410j 5/25/92 14:08:01";
/*
 *   COMPONENT_NAME: LIBCSTR
 *
 *   FUNCTIONS: char_collate_std,char_collate_sb
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
int char_collate_std(_LC_collate_objhdl_t hdl, char *str1, char *str2, 
		     int order)
{
    return(strcmp(str1, str2));
}

int char_collate_sb(_LC_collate_objhdl_t hdl, char *str1, char *str2, 
		    int order)
{
    return(strcmp(str1, str2));
}
