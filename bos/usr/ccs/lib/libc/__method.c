static char sccsid[] = "@(#)24	1.1.1.1  src/bos/usr/ccs/lib/libc/__method.c, libcloc, bos411, 9428A410j 5/25/92 13:53:08";
/*
 * COMPONENT_NAME: (LIBCLOC) Locale
 *
 * FUNCTIONS: NLSsetup.c
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/lc_core.h>
#include <sys/lc_sys.h>
#include <sys/localedef.h>
#include <langinfo.h>

extern __collate_init();
extern __ctype_init();
extern __monetary_init();
extern __charmap_init();
extern __resp_init();
extern __numeric_init();
extern __time_init();
extern __locale_init();

/* 
  charmap class methods 
*/
extern __mbtowc_sb();
extern __mbtowc_932();
extern __mbtowc_eucjp();
extern __mbstowcs_sb();
extern __mbstowcs_932();
extern __mbstowcs_eucjp();
extern __wctomb_sb();
extern __wctomb_932();
extern __wctomb_eucjp();
extern __wcstombs_sb();
extern __wcstombs_932();
extern __wcstombs_eucjp();
extern __mblen_sb();
extern __mblen_932();
extern __mblen_eucjp();
extern __wcswidth_latin();
extern __wcswidth_932();
extern __wcswidth_eucjp();
extern __wcwidth_latin();
extern __wcwidth_932();
extern __wcwidth_eucjp();
extern __csid_sb();
extern __wcsid_sb();
extern __mbtopc_sb();
extern __mbtopc_932();
extern __mbtopc_eucjp();
extern __mbstopcs_sb();
extern __mbstopcs_932();
extern __mbstopcs_eucjp();
extern __pctomb_sb();
extern __pctomb_932();
extern __pctomb_eucjp();
extern __pcstombs_sb();
extern __pcstombs_932();
extern __pcstombs_eucjp();
extern __csid_std();
extern __wcsid_std();
extern nl_csinfo();

/*
    ctype class methods
*/
extern __towupper_std();
extern __towlower_std();
extern __get_wctype_std();
extern __is_wctype_sb();
extern __is_wctype_std();

/*
    collation methods
*/
extern __fnmatch_std();
extern __fnmatch_C();
extern __regcomp_std();
extern __regerror_std();
extern __regexec_std();
extern __regfree_std();
extern __strcoll_C();
extern __strcoll_sb();
extern __strcoll_std();
extern __strxfrm_C();
extern __strxfrm_sb();
extern __strxfrm_std();
extern __wcscoll_C();
extern __wcscoll_sb();
extern __wcscoll_std();
extern __wcsxfrm_C();
extern __wcsxfrm_sb();
extern __wcsxfrm_std();

/*
    numeric methods
*/
extern nl_numinfo();

/*
    monetary methods
*/
extern __strfmon_std();
extern nl_moninfo();

/*
    time methods
*/
extern __strftime_std();
extern __strptime_std();
extern __wcsftime_std();
extern nl_timinfo();

/*
   locale methods
*/
extern __nl_langinfo_std();
extern __localeconv_std();

/*
    response methods
*/
extern nl_respinfo();
extern __rpmatch_C();
extern __rpmatch_std();
void   *(*__method_tbl[])()={
  __mbstopcs_sb,       /* __mbstopcs */
  __mbstopcs_932, 
  __mbstopcs_eucjp,
  __mbtopc_sb,	       /* __mbtopc */
  __mbtopc_932, 
  __mbtopc_eucjp,
  __pcstombs_sb,       /* __pcstombs */
  __pcstombs_932, 
  __pcstombs_eucjp,
  __pctomb_sb,	       /* __pctomb */
  __pctomb_932,   
  __pctomb_eucjp, 
  __csid_std,	       /* csid */
  __charmap_init,	       /* charmap_init */
  __mblen_sb,	       /* mblen */
  __mblen_932,   
  __mblen_eucjp,
  __mbstowcs_sb,	       /* mbstowcs */
  __mbstowcs_932, 
  __mbstowcs_eucjp, 
  __mbtowc_sb,	       /* mbtowc */
  __mbtowc_932,
  __mbtowc_eucjp, 
  nl_csinfo,	       /* nl_csinfo */
  __wcsid_std,	       /* wcsid */
  __wcstombs_sb,	       /* wcstombs */
  __wcstombs_932, 
  __wcstombs_eucjp, 
  __wcswidth_latin,      /* wcswidth */
  __wcswidth_932,   
  __wcswidth_eucjp,
  __wctomb_sb,	       /* wctomb */
  __wctomb_932, 
  __wctomb_eucjp, 
  __wcwidth_latin,       /* wcwidth */
  __wcwidth_932,
  __wcwidth_eucjp,
  __fnmatch_C,	       /* fnmatch */
  __fnmatch_std,
  (void *)0,		/* spare */
  __regcomp_std,	/* regcomp */
  __regerror_std,       /* regerror */
  (void *)0,		/* spare */
  __regexec_std,	/* regexec */
  __regfree_std,	/* regfree */
  __strcoll_C,	       /* strcoll */
  __strcoll_sb,	       
  __strcoll_std,
  __strxfrm_C,	       /* strxfrm */
  __strxfrm_sb,	       
  __strxfrm_std,
  __wcscoll_C,  	       /* wcscoll */
  __wcscoll_std,
  __wcsxfrm_C,	       /* wcsxfrm */
  __wcsxfrm_std,
  __collate_init,	       /* collate_init */
  __get_wctype_std,      /* get_wctype */
  __ctype_init,	       /* ctype_init */
  __is_wctype_sb,	       /* is_wctype */
  __is_wctype_std,
  __towlower_std,	       /* towlower */
  __towupper_std,	       /* towupper */
  __locale_init,	       /* locale_init */
  __localeconv_std,      /* localeconv */
  __nl_langinfo_std,     /* nl_langinfo */
  __monetary_init,       /* monetary_init */
  nl_moninfo,	       /* nl_moninfo */
  __strfmon_std,	       /* strfmon */
  __numeric_init,	       /* numeric_init */
  nl_numinfo,	       /* nl_numinfo */
  __resp_init,	       /* resp_init */
  nl_respinfo,	       /* nl_respinfo */
  __rpmatch_C,	       /* rpmatch */
  __rpmatch_std,
  __time_init,	       /* time_inif */
  nl_timinfo,	       /* nl_timinfo */
  __strftime_std,	       /* strftime */
  __strptime_std,	       /* strptime */
  __wcsftime_std,	       /* wcsftime */
};

void *    *(**__method_table)() = __method_tbl;
