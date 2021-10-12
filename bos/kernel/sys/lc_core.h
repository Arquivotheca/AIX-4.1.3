/* @(#)52	1.6.1.3  src/bos/kernel/sys/lc_core.h, libccnv, bos411, 9428A410j 11/3/93 13:11:49 */

/*
 *
 * COMPONENT_NAME: (LIBCLOC) Locale Related Data Structures and API
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __H_LC_CORE
#define __H_LC_CORE

#include <sys/types.h>
#ifdef _PTR_METH
#define __DECL_METH(__h,__n)  __h __n
#define __OBJ_DATA(__h)     (__h).obj
#else
#define __DECL_METH(__h,__n)  int __n
#define __OBJ_DATA(__h)	(__h).obj
#define __OBJ_METH(__h)	(__h).meth
#endif

typedef struct {

    unsigned short
	__type_id,
	__magic;
    unsigned long
	__version;
    
    size_t  __size;

} _LC_object_t;

/*
** Valid type ids for type_id above.
*/
#define _LC_CAR       1
#define _LC_LOCALE    2
#define _LC_CHARMAP   3
#define _LC_CTYPE     4
#define _LC_COLLATE   5
#define _LC_NUMERIC   6
#define _LC_MONETARY  7
#define _LC_TIME      8
#define _LC_RESP      9
#define _LC_OBJ_HDL   10

/*
** Object magic for V3.2/4.x
*/
#define _LC_MAGIC     0x4C43

/*
** Version AIX V4.1
*/
#define _LC_VERSION   0x40000000


#define _AIX320_LC_VERSION 0x33323030
#define _AIX323_LC_VERSION 0x33323130


/* Macros to extract major and minor version pieces from a version number */

#define _LC_MAJOR_PART(_lc_version) ((_lc_version >> 16) & 0x0000FFFF)
#define _LC_MINOR_PART(_lc_version) (_lc_version & 0x0000FFFF)

#define _LC_MAJOR (_LC_MAJOR_PART(_LC_VERSION))
#define _LC_MINOR (_LC_MINOR_PART(_LC_VERSION))


typedef struct {
  
    _LC_object_t  __hdr;
	
    /* Message Services API Methods */

    __DECL_METH(unsigned char *, __catgets);
    __DECL_METH(void,            __catclose);
    
    /* Message compression methods */
    __DECL_METH(char *, __compress);
    __DECL_METH(char *, __decompress);
    __DECL_METH(int,    __start_compress);
    __DECL_METH(int,    __end_compress);
    
    /* Initialization method */
    __DECL_METH(int,    __init);
    void    *__data;
} _LC_core_car_t;

typedef struct {

    _LC_object_t  __hdr;

    /* locale info method */
    __DECL_METH(char *,  __nl_langinfo);
    
    /* Process code conversion methods */
    __DECL_METH(size_t,  __mbtowc);
    __DECL_METH(size_t,  __mbstowcs);
    __DECL_METH(int,     __wctomb);
    __DECL_METH(size_t,  __wcstombs);
    
    /* Character encoding length method */
    __DECL_METH(int,     __mblen);
    
    /* Character display width methods */
    __DECL_METH(size_t,  __wcswidth);
    __DECL_METH(size_t,  __wcwidth);
    
    /* private PC/CP converters */
    __DECL_METH(int,     __mbtopc);
    __DECL_METH(int,     __mbstopcs);
    __DECL_METH(int,     __pctomb);
    __DECL_METH(int,     __pcstombs);
    
    /* character set id method */
    __DECL_METH(char *,  __csid);
    __DECL_METH(char *,  __wcsid);
    
    /* implementation initialization */
    __DECL_METH(int,     __init);
    void     *__data;
} _LC_core_charmap_t;


typedef struct {
 
    _LC_object_t  __hdr; 
    
    /* case convertersion methods */
    __DECL_METH(wchar_t,    __towupper);
    __DECL_METH(wchar_t,    __towlower);
    
    /* classification methods */
    __DECL_METH(unsigned int,  __get_wctype);
    __DECL_METH(int,           __is_wctype);
    
    /* implementation initialization */
    __DECL_METH(int,        __init);
    void     *__data;
} _LC_core_ctype_t;

typedef struct {

    _LC_object_t  __hdr;

    /* character collation methods */
    __DECL_METH(int,      __strcoll);
    __DECL_METH(size_t,   __strxfrm);
    
    /* process code collation methods */
    __DECL_METH(int,      __wcscoll);
    __DECL_METH(size_t,   __wcsxfrm);
    
    /* filename matching methods */
    __DECL_METH(int,      __fnmatch);
    
    /* regular expression methods */
    __DECL_METH(int,      __regcomp);
    __DECL_METH(size_t,   __regerror);
    __DECL_METH(int,      __regexec);
    __DECL_METH(void,     __regfree);
    
    /* implementation initialization */
    __DECL_METH(int,      __init);
    void     *__data;
} _LC_core_collate_t;


typedef struct {

    _LC_object_t  __hdr;
    
    /* time info method */
    __DECL_METH(char *,   __nl_langinfo);
    
    /* time character string formatting methods */
    __DECL_METH(size_t,   __strftime);
    __DECL_METH(char *,   __strptime);
    
    /* time process code string formatting methods */
    __DECL_METH(size_t,   __wcsftime);
    
    /* implementation initialization */
    __DECL_METH(int,      __init);
    void     *__data;
} _LC_core_time_t;


typedef struct {

    _LC_object_t  __hdr;

    /* monetary info method */
    __DECL_METH(char *,   __nl_langinfo);
    
    /* character string monetary formatting method */
    __DECL_METH(size_t,   __strfmon);
    
    /* implementation initialization */
    __DECL_METH(int,      __init);
    void     *__data;
} _LC_core_monetary_t;


typedef struct {

    _LC_object_t  __hdr;

    /* langinfo method */
    __DECL_METH(char *,   __nl_langinfo);
    
    /* implementation initialization */
    __DECL_METH(int,      __init);
    void     *__data;
} _LC_core_numeric_t;


typedef struct {

    _LC_object_t  __hdr;
    
    /* langinfo method */
    __DECL_METH(char *,  __nl_langinfo);
    
    /* response matching method */
    __DECL_METH(char *,  __rpmatch);
    
    /* implementation initialization */
    __DECL_METH(int,     __init);
    void        *__data;
} _LC_core_resp_t;

typedef struct {

    _LC_object_t __hdr;

    /* langinfo method */
    __DECL_METH(char *,         __nl_langinfo);
    __DECL_METH(struct lconv *, __localeconv);
    
    /* Initialization */
    __DECL_METH(int,   __init);
    void         *__data;
} _LC_core_locale_t;

#endif


