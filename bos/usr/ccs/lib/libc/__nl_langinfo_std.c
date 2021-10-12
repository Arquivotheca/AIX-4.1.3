static char sccsid[] = "@(#)74	1.5.1.3  src/bos/usr/ccs/lib/libc/__nl_langinfo_std.c, libcloc, bos411, 9428A410j 1/12/94 11:44:21";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __nl_langinfo_std
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <langinfo.h>
#include <sys/lc_sys.h>

/*
*  FUNCTION: __nl_langinfo_std
*
*  DESCRIPTION:
*  Returns the locale database string which corresponds to the specified
*  nl_item.
*/

char * __nl_langinfo_std(_LC_locale_objhdl_t hdl, nl_item item)
{
	char *ptr1, *ptr2;
	static char string[100];
	int  len;

    if (item >= _NL_NUM_ITEMS || item < 0) {
	string[0] = 0x00;
	return (string);
    }

    if ((item == YESSTR) || (item == NOSTR)){
	ptr1 = __OBJ_DATA(hdl)->nl_info[item];
	ptr2 = strchr(ptr1,':');      	     /* This may cause problems */
	if (ptr2 == NULL)                    /* because : is not a protected */
	    return ptr1;                     /* character in 932            */
	else {
	    if ((len = (ptr2 - ptr1)) > 100)  
	        ptr2 = (char *)malloc(len);
	    else
	        ptr2 = string;
	    strncpy(ptr2,ptr1,len);
	    ptr2[len] = '\0';
	    return ptr2;
        }
    }
    else if (item == CRNCYSTR) {
	ptr1 = __OBJ_DATA(hdl)->nl_info[item];
	if (ptr1[0] == '\0')
		return(ptr1);
	if ((len=strlen(ptr1)+2) > 100)
	    ptr2 = (char *)malloc(len);
	else
	    ptr2 = string;

	if (__OBJ_DATA(hdl)->nl_lconv->p_cs_precedes == 1)
	    ptr2[0] = '-';
	else if (__OBJ_DATA(hdl)->nl_lconv->p_cs_precedes == 0)
	    ptr2[0] = '+';
	else if (__OBJ_DATA(hdl)->nl_lconv->p_cs_precedes == 2)
	    ptr2[0] = '.';
	strncpy(&ptr2[1], ptr1, len);
	ptr2[len] = '\0';
	return ptr2;
    }
    else
        return __OBJ_DATA(hdl)->nl_info[item];
}

char * nl_numinfo() {}
char *nl_moninfo() {}
char *nl_respinfo() {}
char *nl_timinfo() {}
char *nl_csinfo() {}

