static char sccsid[] = "@(#)00	1.3.1.1  src/bos/usr/bin/localedef/method.c, cmdnls, bos411, 9428A410j 5/25/92 11:34:24";
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/limits.h>
#include <sys/lc_sys.h>
#include "method.h"
#ifdef _PTR_METH
extern __MBSTOPCS_SB();  
extern __MBSTOPCS_932(); 
extern __MBSTOPCS_EUCJP(); 
extern __MBTOPC_SB();      
extern __MBTOPC_932();   
extern __MBTOPC_EUCJP(); 
extern __PCSTOMBS_SB();  
extern __PCSTOMBS_932(); 
extern __PCSTOMBS_EUCJP();
extern __PCTOMB_SB();     
extern __PCTOMB_932();   
extern __PCTOMB_EUCJP(); 
extern CSID_STD();      
extern CHARMAP_INIT(); 
extern MBLEN_SB();    
extern MBLEN_932();  
extern MBLEN_EUCJP();      
extern MBSTOWCS_SB();     
extern MBSTOWCS_932();   
extern MBSTOWCS_EUCJP();
extern MBTOWC_SB();    
extern MBTOWC_932();  
extern MBTOWC_EUCJP();     
extern NL_CSINFO();       
extern WCSID_STD();      
extern WCSTOMBS_SB();   
extern WCSTOMBS_932(); 
extern WCSTOMBS_EUCJP();    
extern WCSWIDTH_LATIN();   
extern WCSWIDTH_932();    
extern WCSWIDTH_EUCJP(); 
extern WCTOMB_SB();     
extern WCTOMB_932();   
extern WCTOMB_EUCJP();
extern WCWIDTH_LATIN();     
extern WCWIDTH_932();      
extern WCWIDTH_EUCJP();   
extern FNMATCH_C();      
extern FNMATCH_STD();   
/* extern	xxxxxxxxx         		xxxxxxxxx */        
extern REGCOMP_STD();  
extern REGERROR_STD();
/* extern 	xxxxxxxxx        		xxxxxxxxx   */      
extern REGEXEC_STD();       	
extern REGFREE_STD();       
extern STRCOLL_C();        
extern STRCOLL_SB();      
extern STRCOLL_STD();    
extern STRXFRM_C();     
extern STRXFRM_SB();   
extern STRXFRM_STD(); 
extern WCSCOLL_C();  
extern WCSCOLL_STD();      
extern WCSXFRM_C();       
extern WCSXFRM_STD();    
extern COLLATE_INIT();  
extern GET_WCTYPE_STD();    
extern CTYPE_INIT();       
extern IS_WCTYPE_SB();    
extern IS_WCTYPE_STD();  
extern TOWLOWER_STD();      	
extern TOWUPPER_STD();      
extern LOCALE_INIT();      
extern LOCALECONV_STD();  
extern NL_LANGINFO_STD();
extern MONETARY_INIT();     	
extern NL_MONINFO();        
extern STRFMON_STD();      
extern NUMERIC_INIT();    
extern NL_NUMINFO();     
extern RESP_INIT();     
extern NL_RESPINFO();  
extern RPMATCH_C();   
extern RPMATCH_STD();
extern TIME_INIT();        
extern NL_TIMINFO();      
extern STRFTIME_STD();   
extern STRPTIME_STD();  
extern WCSFTIME_STD(); 
#endif
std_method_t std_methods_tbl[]={
/*
std_method_t std_methods[]={
*/
{ "charmap.__mbstopcs", 
"__mbstopcs_sb",  "__mbstopcs_932", "__mbstopcs_eucjp",  0, 
LIBC, LIBC, LIBC, 0,
__MBSTOPCS_SB,  __MBSTOPCS_932, __MBSTOPCS_EUCJP, -1,
},

{ "charmap.__mbtopc", 
"__mbtopc_sb", "__mbtopc_932", "__mbtopc_eucjp", 0,
LIBC, LIBC, LIBC, 0,
__MBTOPC_SB, __MBTOPC_932, __MBTOPC_EUCJP,  -1, 
},

{ "charmap.__pcstombs",
"__pcstombs_sb",  "__pcstombs_932", "__pcstombs_eucjp",  0,
LIBC, LIBC, LIBC, 0,
__PCSTOMBS_SB,  __PCSTOMBS_932, __PCSTOMBS_EUCJP, -1,
},

{ "charmap.__pctomb", 
"__pctomb_sb",    "__pctomb_932",   "__pctomb_eucjp", 0,
LIBC, LIBC, LIBC,  0,
__PCTOMB_SB,    __PCTOMB_932,   __PCTOMB_EUCJP,  -1,
},

{ "charmap.csid", 
"__csid_std", "__csid_std", "__csid_std", "__csid_std",
LIBC, LIBC, LIBC, LIBC,
CSID_STD, CSID_STD, CSID_STD, CSID_STD,
},

{ "charmap.init",  
"__charmap_init", "__charmap_init", "__charmap_init", "__charmap_init",
LIBC, LIBC, LIBC, LIBC,
CHARMAP_INIT, CHARMAP_INIT, CHARMAP_INIT, CHARMAP_INIT,
},

{ "charmap.mblen",
"__mblen_sb", "__mblen_932", "__mblen_eucjp", 0,
LIBC, LIBC, LIBC, 0,
MBLEN_SB,   MBLEN_932,   MBLEN_EUCJP,   -1,
},

{ "charmap.mbstowcs", 
"__mbstowcs_sb", "__mbstowcs_932", "__mbstowcs_eucjp", 0,
LIBC, LIBC, LIBC, 0,
MBSTOWCS_SB, MBSTOWCS_932, MBSTOWCS_EUCJP, -1,
},

{ "charmap.mbtowc", 
"__mbtowc_sb",    "__mbtowc_932",   "__mbtowc_eucjp",   0,
LIBC, LIBC, LIBC, 0,
MBTOWC_SB,      MBTOWC_932,     MBTOWC_EUCJP, -1,
},

{ "charmap.nl_langinfo",
"nl_csinfo", "nl_csinfo", "nl_csinfo", "nl_csinfo",
LIBC, LIBC, LIBC, LIBC,
NL_CSINFO, NL_CSINFO, NL_CSINFO, NL_CSINFO,
},

{ "charmap.wcsid", 
"__wcsid_std", "__wcsid_std", "__wcsid_std", "__wcsid_std",
LIBC, LIBC, LIBC, LIBC,
WCSID_STD, WCSID_STD, WCSID_STD, WCSID_STD,
},

{ "charmap.wcstombs", 
"__wcstombs_sb", "__wcstombs_932", "__wcstombs_eucjp", 0,
LIBC, LIBC, LIBC, 0,
WCSTOMBS_SB, WCSTOMBS_932, WCSTOMBS_EUCJP, -1,
},

{ "charmap.wcswidth", 
"__wcswidth_latin", "__wcswidth_932", "__wcswidth_eucjp", 0,
LIBC, LIBC, LIBC, 0,
WCSWIDTH_LATIN,   WCSWIDTH_932,   WCSWIDTH_EUCJP, -1,
},

{ "charmap.wctomb", 
"__wctomb_sb", "__wctomb_932", "__wctomb_eucjp", 0,
LIBC, LIBC, LIBC,  0,
WCTOMB_SB, WCTOMB_932, WCTOMB_EUCJP,  -1,
},

{ "charmap.wcwidth", 
"__wcwidth_latin",   "__wcwidth_932",  "__wcwidth_eucjp", 0,
LIBC, LIBC, LIBC, 0,
WCWIDTH_LATIN,     WCWIDTH_932,    WCWIDTH_EUCJP, -1,
},

{ "collate.fnmatch",
"__fnmatch_std", "__fnmatch_std", "__fnmatch_std", "__fnmatch_std",
LIBC, LIBC, LIBC, LIBC,
FNMATCH_STD, FNMATCH_STD, FNMATCH_STD, FNMATCH_STD,
},

{ "collate.regcomp",
"__regcomp_std", "__regcomp_std", "__regcomp_std", "__regcomp_std",
LIBC, LIBC, LIBC, LIBC,
REGCOMP_STD, REGCOMP_STD, REGCOMP_STD, REGCOMP_STD,
},

{ "collate.regerror",
"__regerror_std", "__regerror_std", "__regerror_std", "__regcomp_std",
LIBC, LIBC, LIBC, LIBC,
REGERROR_STD, REGERROR_STD, REGERROR_STD, REGERROR_STD,
},

{ "collate.regexec",
"__regexec_std", "__regexec_std", "__regexec_std", "__regexec_std",
LIBC, LIBC, LIBC, LIBC,
REGEXEC_STD, REGEXEC_STD, REGEXEC_STD, REGEXEC_STD,
},

{ "collate.regfree",
"__regfree_std", "__regfree_std", "__regfree_std", "__regfree_std",
LIBC, LIBC, LIBC, LIBC,
REGFREE_STD, REGFREE_STD, REGFREE_STD, REGFREE_STD,
},

{ "collate.strcoll",
"__strcoll_sb", "__strcoll_std", "__strcoll_std", "__strcoll_std",
LIBC, LIBC, LIBC, LIBC,
STRCOLL_SB, STRCOLL_STD, STRCOLL_STD, STRCOLL_STD,
},

{ "collate.strxfrm",
"__strxfrm_sb", "__strxfrm_std", "__strxfrm_std", "__strxfrm_std",
LIBC, LIBC, LIBC, LIBC,
STRXFRM_SB, STRXFRM_STD, STRXFRM_STD, STRXFRM_STD,
},

{ "collate.wcscoll",
"__wcscoll_std", "__wcscoll_std", "__wcscoll_std", "__wcscoll_std",
LIBC, LIBC, LIBC, LIBC,
WCSCOLL_STD, WCSCOLL_STD, WCSCOLL_STD, WCSCOLL_STD,
},

{ "collate.wcsxfrm",
"__wcsxfrm_std", "__wcsxfrm_std", "__wcsxfrm_std", "__wcsxfrm_std",
LIBC, LIBC, LIBC, LIBC,
WCSXFRM_STD, WCSXFRM_STD, WCSXFRM_STD, WCSXFRM_STD,
},

{ "collate.init", 
"__collate_init", "__collate_init", "__collate_init", "__collate_init",
LIBC, LIBC, LIBC, LIBC,
COLLATE_INIT, COLLATE_INIT, COLLATE_INIT, COLLATE_INIT,
},

{ "ctype.get_wctype",    
"__get_wctype_std",  "__get_wctype_std", "__get_wctype_std",  "__get_wctype_std", 
LIBC, LIBC, LIBC, LIBC,
GET_WCTYPE_STD,  GET_WCTYPE_STD, GET_WCTYPE_STD,  GET_WCTYPE_STD,
},

{ "ctype.init", 
"__ctype_init", "__ctype_init", "__ctype_init", "__ctype_init",
LIBC, LIBC, LIBC, LIBC,
CTYPE_INIT, CTYPE_INIT, CTYPE_INIT, CTYPE_INIT, 
},

{ "ctype.is_wctype",     
"__is_wctype_std", "__is_wctype_std", "__is_wctype_std", "__is_wctype_std", 
LIBC, LIBC, LIBC, LIBC,
IS_WCTYPE_STD, IS_WCTYPE_STD, IS_WCTYPE_STD, IS_WCTYPE_STD,
},

{ "ctype.towlower", 
"__towlower_std", "__towlower_std", "__towlower_std", "__towlower_std", 
LIBC, LIBC, LIBC, LIBC,
TOWLOWER_STD, TOWLOWER_STD, TOWLOWER_STD, TOWLOWER_STD,
},

{ "ctype.towupper", 
"__towupper_std", "__towupper_std", "__towupper_std", "__towupper_std", 
LIBC, LIBC, LIBC, LIBC,
TOWUPPER_STD, TOWUPPER_STD, TOWUPPER_STD, TOWUPPER_STD, 
},

{ "locale.init", 
"__locale_init", "__locale_init", "__locale_init", "__locale_init" ,
LIBC, LIBC, LIBC, LIBC,
LOCALE_INIT, LOCALE_INIT, LOCALE_INIT, LOCALE_INIT,
},

{ "locale.localeconv", 
"__localeconv_std", "__localeconv_std", "__localeconv_std", "__localeconv_std", 
LIBC, LIBC, LIBC, LIBC,
LOCALECONV_STD, LOCALECONV_STD, LOCALECONV_STD, LOCALECONV_STD,
},

{ "locale.nl_langinfo",
"__nl_langinfo_std", "__nl_langinfo_std", "__nl_langinfo_std", "__nl_langinfo_std", 
LIBC, LIBC, LIBC, LIBC,
NL_LANGINFO_STD, NL_LANGINFO_STD, NL_LANGINFO_STD, NL_LANGINFO_STD,
},

{ "monetary.init", 
"__monetary_init", "__monetary_init", "__monetary_init", "__monetary_init",
LIBC, LIBC, LIBC, LIBC,
MONETARY_INIT, MONETARY_INIT, MONETARY_INIT, MONETARY_INIT,
},

{ "monetary.nl_langinfo",
"nl_moninfo", "nl_moninfo", "nl_moninfo", "nl_moninfo", 
LIBC, LIBC, LIBC, LIBC,
NL_MONINFO, NL_MONINFO, NL_MONINFO, NL_MONINFO,
},

{ "monetary.strfmon",
"__strfmon_std", "__strfmon_std", "__strfmon_std", "__strfmon_std", 
LIBC, LIBC, LIBC, LIBC,
STRFMON_STD, STRFMON_STD, STRFMON_STD, STRFMON_STD,
},

{ "msg.catclose", 0,0,0,0, 0,0,0,0,  -1,-1,-1,-1, },

{ "msg.catgets",  0,0,0,0,  0,0,0,0, -1,-1,-1,-1, },

{ "msg.compress",0,0,0,0,   0,0,0,0,  -1,-1,-1,-1,},
{ "msg.decompress",0,0,0,0,  0,0,0,0,  -1,-1,-1,-1, },
{ "msg.end_compress",0,0,0,0,   0,0,0,0, -1,-1,-1,-1, },
{ "msg.init", 0,0,0,0,  0,0,0,0,  -1,-1,-1,-1, },
{ "msg.start_compress",0,0,0,0,  0,0,0,0,  -1,-1,-1,-1, },

{ "numeric.init", 
"__numeric_init", "__numeric_init", "__numeric_init", "__numeric_init",
LIBC, LIBC, LIBC, LIBC,
NUMERIC_INIT, NUMERIC_INIT, NUMERIC_INIT, NUMERIC_INIT,
},

{ "numeric.nl_langinfo", 
"nl_numinfo", "nl_numinfo", "nl_numinfo", "nl_numinfo",
LIBC, LIBC, LIBC, LIBC,
NL_NUMINFO, NL_NUMINFO, NL_NUMINFO, NL_NUMINFO,
},

{ "resp.init", 
"__resp_init", "__resp_init", "__resp_init", "__resp_init",
LIBC, LIBC, LIBC, LIBC,
RESP_INIT, RESP_INIT, RESP_INIT, RESP_INIT,
},

{ "resp.nl_langinfo",
"nl_respinfo", "nl_respinfo", "nl_respinfo", "nl_respinfo", 
LIBC, LIBC, LIBC, LIBC,
NL_RESPINFO, NL_RESPINFO, NL_RESPINFO, NL_RESPINFO,
},

{ "resp.rpmatch", 
"__rpmatch_std", "__rpmatch_std", "__rpmatch_std", "__rpmatch_std", 
LIBC, LIBC, LIBC, LIBC,
RPMATCH_STD, RPMATCH_STD, RPMATCH_STD, RPMATCH_STD,
},

{ "time.init", 
"__time_init", "__time_init", "__time_init", "__time_init",
LIBC, LIBC, LIBC, LIBC,
TIME_INIT, TIME_INIT, TIME_INIT, TIME_INIT,
},

{ "time.nl_langinfo",    
"nl_timinfo", "nl_timinfo", "nl_timinfo", "nl_timinfo", 
LIBC, LIBC, LIBC, LIBC,
NL_TIMINFO, NL_TIMINFO, NL_TIMINFO, NL_TIMINFO,
},

{ "time.strftime",
"__strftime_std", "__strftime_std", "__strftime_std", "__strftime_std", 
LIBC, LIBC, LIBC, LIBC,
STRFTIME_STD, STRFTIME_STD, STRFTIME_STD, STRFTIME_STD, 
},

{ "time.strptime", 
"__strptime_std", "__strptime_std", "__strptime_std", "__strptime_std" , 
LIBC, LIBC, LIBC, LIBC,
STRPTIME_STD, STRPTIME_STD, STRPTIME_STD, STRPTIME_STD,
},

{ "time.wcsftime", 
"__wcsftime_std", "__wcsftime_std", "__wcsftime_std", "__wcsftime_std", 
LIBC, LIBC, LIBC, LIBC,
WCSFTIME_STD, WCSFTIME_STD, WCSFTIME_STD, WCSFTIME_STD,
},
};

std_method_t *std_methods=std_methods_tbl;
