static char sccsid[] = "@(#)56	1.2  src/bos/usr/lpp/Unicode/methods/__wcwidth_unistd.c, cfgnls, bos411, 9428A410j 4/20/94 10:41:59";
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: __wcwidth_unistd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <ctype.h>
#include <sys/lc_core.h>
#include <sys/lc_sys.h>

/* NOTE: This will have to be fixed whenever charmap is updated. */
int csid2width[] = {
    1, /* CS0 */
    1, /* CS1 */
    1, /* CS2 */
    1, /* CS3 */
    1, /* CS4 */
    1, /* CS5 */
    1, /* CS6 */
    1, /* CS7 */
    1, /* CS8 */
    1, /* CS9 */
    2, /* CS10 */
    1, /* CS11 */
    2, /* CS12 */
    2, /* CS13 */
    2, /* CS14 */
    2, /* CS15 */
    2, /* CS16 */
    2, /* CS17 */
    1, /* CS18 */
};

int __wcwidth_unistd(_LC_charmap_objhdl_t hdl, const wchar_t wc)
{
    	int	len = 0;
    
    	/* If wc is NULL, return 0. */
    	if (wc == (wchar_t )NULL) {
		return(0);
	}

	/* Check if character is printable. */
	/* NOTE: Uses global locale. */
	if (!iswprint(wc)) {
	    return (-1);
	}
 
	/* Use localedef csid table. */
#ifdef GLOBAL
	return (csid2width[wcsid(wc)]);
#else
	if (__OBJ_METH(hdl)) {
	    /* Non-global. */
	    return (csid2width[(int)(_CALLMETH(hdl,__wcsid)(hdl, wc))]);
	}
	else { /* A null handle was passed (probably by localedef) */
            return (2); /* This is used to compute max_disp_width. */
	}
#endif
}
