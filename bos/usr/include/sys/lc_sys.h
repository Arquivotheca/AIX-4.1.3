/* @(#)53	1.2  src/bos/usr/include/sys/lc_sys.h, libcloc, bos411, 9428A410j 1/12/93 18:15:23 */

/*
 * COMPONENT_NAME: LIBCLOC
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_LC_SYS
#define _H_LC_SYS
#include <sys/lc_core.h>

#define MB_MAX_LEN	4

#ifdef _PTR_METH

/* Used by interface stubs */
#define _CALLMETH(__h,__n)   (*(__OBJ_DATA(__h)->core.__n))


#else

extern void* *(**__method_table)();
extern void  *(*__method_tbl[])();

/* Used by interface stubs */
#define _CALLMETH(__h,__n)   (*(__h.meth[__OBJ_DATA(__h)->core.__n]))

#endif

/*
** What follows are the locations of each of the macros in the global
** method table.  
** 
** #ifdef _PTR_METH is true, these values MUST match the order
** of the methods in __method_table in com/lib/c/loc/NLSsetup.c 
**
** These values are used only by localedef to initialize the standard
** method table.  
**
** These values can be used by a locale method file to specify the system
** defined method to be used. They can be used as a c_funct_name.
**
*/
#ifdef _PTR_METH
#define __MBSTOPCS_SB     	__mbstopcs_sb     
#define __MBSTOPCS_932    	__mbstopcs_932    
#define __MBSTOPCS_EUCJP  	__mbstopcs_eucjp  
#define __MBTOPC_SB       	__mbtopc_sb       
#define __MBTOPC_932      	__mbtopc_932      
#define __MBTOPC_EUCJP    	__mbtopc_eucjp    
#define __PCSTOMBS_SB     	__pcstombs_sb     
#define __PCSTOMBS_932    	__pcstombs_932    
#define __PCSTOMBS_EUCJP  	__pcstombs_eucjp  
#define __PCTOMB_SB       	__pctomb_sb       
#define __PCTOMB_932      	__pctomb_932      
#define __PCTOMB_EUCJP    	__pctomb_eucjp    
#define CSID_STD          	__csid_std          
#define CHARMAP_INIT      	__charmap_init      
#define MBLEN_SB          	__mblen_sb          
#define MBLEN_932         	__mblen_932         
#define MBLEN_EUCJP       	__mblen_eucjp       
#define MBSTOWCS_SB       	__mbstowcs_sb       
#define MBSTOWCS_932      	__mbstowcs_932      
#define MBSTOWCS_EUCJP    	__mbstowcs_eucjp    
#define MBTOWC_SB         	__mbtowc_sb         
#define MBTOWC_932        	__mbtowc_932        
#define MBTOWC_EUCJP      	__mbtowc_eucjp      
#define NL_CSINFO         	nl_csinfo         
#define WCSID_STD         	__wcsid_std         
#define WCSTOMBS_SB       	__wcstombs_sb       
#define WCSTOMBS_932      	__wcstombs_932      
#define WCSTOMBS_EUCJP    	__wcstombs_eucjp    
#define WCSWIDTH_LATIN    	__wcswidth_latin    
#define WCSWIDTH_932      	__wcswidth_932      
#define WCSWIDTH_EUCJP    	__wcswidth_eucjp    
#define WCTOMB_SB         	__wctomb_sb         
#define WCTOMB_932        	__wctomb_932        
#define WCTOMB_EUCJP      	__wctomb_eucjp      
#define WCWIDTH_LATIN     	__wcwidth_latin     
#define WCWIDTH_932       	__wcwidth_932       
#define WCWIDTH_EUCJP     	__wcwidth_eucjp     
#define FNMATCH_C         	__fnmatch_C         
#define FNMATCH_STD       	__fnmatch_std       
/* #define	xxxxxxxxx         	/*	xxxxxxxxx */        
#define REGCOMP_STD       	__regcomp_std       
#define REGERROR_STD      	__regerror_std      
/* #define 	xxxxxxxxx        	/*	xxxxxxxxx   */      
#define REGEXEC_STD       	__regexec_std       
#define REGFREE_STD       	__regfree_std       
#define STRCOLL_C         	__strcoll_C         
#define STRCOLL_SB        	__strcoll_sb        
#define STRCOLL_STD       	__strcoll_std       
#define STRXFRM_C         	__strxfrm_C         
#define STRXFRM_SB        	__strxfrm_sb        
#define STRXFRM_STD       	__strxfrm_std       
#define WCSCOLL_C         	__wcscoll_C         
#define WCSCOLL_STD       	__wcscoll_std       
#define WCSXFRM_C         	__wcsxfrm_C         
#define WCSXFRM_STD       	__wcsxfrm_std       
#define COLLATE_INIT      	__collate_init      
#define GET_WCTYPE_STD    	__get_wctype_std    
#define CTYPE_INIT        	__ctype_init        
#define IS_WCTYPE_SB      	__is_wctype_sb      
#define IS_WCTYPE_STD     	__is_wctype_std     
#define TOWLOWER_STD      	__towlower_std      
#define TOWUPPER_STD      	__towupper_std      
#define LOCALE_INIT       	__locale_init       
#define LOCALECONV_STD    	__localeconv_std    
#define NL_LANGINFO_STD   	__nl_langinfo_std   
#define MONETARY_INIT     	__monetary_init     
#define NL_MONINFO        	nl_moninfo        
#define STRFMON_STD       	__strfmon_std       
#define NUMERIC_INIT      	__numeric_init      
#define NL_NUMINFO        	nl_numinfo        
#define RESP_INIT         	__resp_init         
#define NL_RESPINFO       	nl_respinfo       
#define RPMATCH_C         	__rpmatch_C         
#define RPMATCH_STD       	__rpmatch_std       
#define TIME_INIT         	__time_init         
#define NL_TIMINFO        	nl_timinfo        
#define STRFTIME_STD      	__strftime_std      
#define STRPTIME_STD      	__strptime_std      
#define WCSFTIME_STD      	__wcsftime_std      

#else

#define __MBSTOPCS_SB     0
#define __MBSTOPCS_932    1
#define __MBSTOPCS_EUCJP  2
#define __MBTOPC_SB       3
#define __MBTOPC_932      4
#define __MBTOPC_EUCJP    5
#define __PCSTOMBS_SB     6
#define __PCSTOMBS_932    7
#define __PCSTOMBS_EUCJP  8
#define __PCTOMB_SB       9
#define __PCTOMB_932      10
#define __PCTOMB_EUCJP    11
#define CSID_STD          12
#define CHARMAP_INIT      13
#define MBLEN_SB          14
#define MBLEN_932         15
#define MBLEN_EUCJP       16
#define MBSTOWCS_SB       17
#define MBSTOWCS_932      18
#define MBSTOWCS_EUCJP    19
#define MBTOWC_SB         20 
#define MBTOWC_932        21
#define MBTOWC_EUCJP      22
#define NL_CSINFO         23
#define WCSID_STD         24
#define WCSTOMBS_SB       25
#define WCSTOMBS_932      26
#define WCSTOMBS_EUCJP    27
#define WCSWIDTH_LATIN    28
#define WCSWIDTH_932      29
#define WCSWIDTH_EUCJP    30
#define WCTOMB_SB         31
#define WCTOMB_932        32
#define WCTOMB_EUCJP      33
#define WCWIDTH_LATIN     34
#define WCWIDTH_932       35
#define WCWIDTH_EUCJP     36
#define FNMATCH_C         37
#define FNMATCH_STD       38
/*	#define xxxxxxxxx         39   ** spare ** */
#define REGCOMP_STD       40
#define REGERROR_STD      41
/*	#define xxxxxxxxx         42   ** spare ** */
#define REGEXEC_STD       43
#define REGFREE_STD       44
#define STRCOLL_C         45
#define STRCOLL_SB        46
#define STRCOLL_STD       47
#define STRXFRM_C         48
#define STRXFRM_SB        49
#define STRXFRM_STD       50
#define WCSCOLL_C         51
#define WCSCOLL_STD       52
#define WCSXFRM_C         53
#define WCSXFRM_STD       54
#define COLLATE_INIT      55
#define GET_WCTYPE_STD    56
#define CTYPE_INIT        57
#define IS_WCTYPE_SB      58
#define IS_WCTYPE_STD     59
#define TOWLOWER_STD      60
#define TOWUPPER_STD      61
#define LOCALE_INIT       62
#define LOCALECONV_STD    63
#define NL_LANGINFO_STD   64
#define MONETARY_INIT     65
#define NL_MONINFO        66
#define STRFMON_STD       67 
#define NUMERIC_INIT      68
#define NL_NUMINFO        69
#define RESP_INIT         70
#define NL_RESPINFO       71
#define RPMATCH_C         72
#define RPMATCH_STD       73
#define TIME_INIT         74
#define NL_TIMINFO        75
#define STRFTIME_STD      76
#define STRPTIME_STD      77
#define WCSFTIME_STD      78


#endif /* _PTR_METH */
#endif

