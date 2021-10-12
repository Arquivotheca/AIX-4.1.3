/* @(#)01	1.2.1.4  src/bos/usr/bin/localedef/method.h, cmdnls, bos411, 9439A411b 9/25/94 22:18:16 */
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/lc_core.h>
#include <sys/lc_sys.h>

/*
** These defines are used by localedef.c and by sem_method.c
*/
#define CC_CMD_FMT 	"%s/bin/cc -c %s %s"
#define LDOPT_CMD_FMT "%s/bin/ld %s %s %s %s -o \"%s\"\
 -bM:SRO -H8 -bh:5 -elc_obj_hdl"
#define LD_CMD_FMT    "%s/bin/ld %s %s /usr/lib/libc.a -o \"%s\"\
 -bM:SRO -H8 -bh:5 -elc_obj_hdl"
#define LDOPT_CMD_FMT_METH "%s/bin/ld %s /usr/lib/libc.a %s %s -o \"%s\"\
 -H8 -bh:5 -estd_method_hdl"
#define LIBC "-lc"

/*
** These are the defines for the indexes for the global method table 
** 
** Changes here should be reflected in gen.c:  ie functions 
**	gen_charmap_tbl()
**	gen_collate_tbl()
**	gen_ctype_tbl()
**	gen_locale_tbl()
**	gen_monetary_tbl()
**	gen_resp_tbl()
**	gen_numeric_tbl()
**	gen_time_tbl()
*/


#define CHARMAP___MBSTOPCS      0x00
#define CHARMAP___MBTOPC        0x01
#define CHARMAP___PCSTOMBS      0x02
#define CHARMAP___PCTOMB        0x03
#define CHARMAP_CSID            0x04
#define CHARMAP_CHARMAP_INIT    0x05
#define CHARMAP_MBLEN           0x06
#define CHARMAP_MBSTOWCS        0x07
#define CHARMAP_MBTOWC          0x08
#define CHARMAP_NL_LANGINFO     0x09 
#define CHARMAP_WCSID           0x0a
#define CHARMAP_WCSTOMBS        0x0b
#define CHARMAP_WCSWIDTH        0x0c
#define CHARMAP_WCTOMB          0x0d
#define CHARMAP_WCWIDTH         0x0e

#define COLLATE_FNMATCH         0x0f
#define COLLATE_REGCOMP         0x10
#define COLLATE_REGERROR        0x11
#define COLLATE_REGEXEC         0x12
#define COLLATE_REGFREE         0x13
#define COLLATE_STRCOLL         0x14
#define COLLATE_STRXFRM         0x15
#define COLLATE_WCSCOLL         0x16
#define COLLATE_WCSXFRM         0x17
#define COLLATE_COLLATE_INIT    0x18

#define CTYPE_GET_WCTYPE        0x19
#define CTYPE_CTYPE_INIT        0x1a
#define CTYPE_IS_WCTYPE         0x1b
#define CTYPE_TOWLOWER          0x1c
#define CTYPE_TOWUPPER          0x1d

#define LOCALE_LOCALE_INIT      0x1e
#define LOCALE_LOCALECONV       0x1f
#define LOCALE_NL_LANGINFO      0x20

#define MONETARY_MONETARY_INIT  0x21
#define MONETARY_NL_LANGINFO    0x22
#define MONETARY_STRFMON        0x23

#define MSG_CATCLOSE            0x24
#define MSG_CATGETS             0x25
#define MSG_COMPRESS            0x26
#define MSG_DECOMPRESS          0x27
#define MSG_END_COMPRESS        0x28
#define MSG_MSG_INIT            0x29
#define MSG_START_COMPRESS      0x2a

#define NUMERIC_NUMERIC_INIT    0x2b
#define NUMERIC_NL_LANGINFO     0x2c

#define RESP_RESP_INIT          0x2d
#define RESP_NL_LANGINFO        0x2e
#define RESP_RPMATCH            0x2f

#define TIME_TIME_INIT          0x30
#define TIME_NL_LANGINFO        0x31
#define TIME_STRFTIME           0x32
#define TIME_STRPTIME           0x33
#define TIME_WCSFTIME           0x34

#define LAST_METHOD             0x34

#define SB_CODESET	0
#define IBM_932		1
#define IBM_eucJP	2
#define USR_DEFINED	3

#define MX_METHOD_CLASS	4

typedef struct {
  char *method_name;	/* name of method (e.g. mbtowc, strcoll, ...) */

			/* symbol for function implementing method
			   (e.g, mbtowc_932, strcoll_std, ...	      */
  char *c_symbol[MX_METHOD_CLASS];

  char *pkg_name[MX_METHOD_CLASS];

#ifdef _PTR_METH	/* reference to instance of method  		*/
			/* this will be a pointer to the method if the  */
			/* library is using pointer methods or	        */

  void *(*method[MX_METHOD_CLASS])();

#else			/* an index into the method table if the libaray*/
			/* is using index methods.			*/

  int  method[MX_METHOD_CLASS];

#endif
} std_method_t;

extern int method_class;
extern std_method_t *std_methods;
extern int mb_cur_max;

typedef struct {
	std_method_t *std_tbl_ptr;
#ifndef _PTR_METH
	void * 	*(**meth_tbl_ptr)();
#endif
} std_meth_ptr_t;

#define METH_NAME(m)    (std_methods[m].c_symbol[method_class])

#ifdef _PTR_METH

#define METH_OFFS(m)    (m)

#else

#define METH_OFFS(m)    (std_methods[m].method[method_class])

#endif

#ifdef _PTR_METH

/* Used by localedef to invoke a method */
#define CALL_METH(n)    (*(std_methods[n].method[method_class]))

#else

/* Used by interface stubs */

/* Used by localedef to invoke a method */
#define CALL_METH(n)    (*__method_table[(n)])

#endif

/*
** This section is only used by localedef if there are method tables
** This defines the index number of the methods in the private method
** table 
*/

/*
** These are the methods found in the private __lc_locale method table
*/
#define PRVT_LOCALE_LOCALE_INIT		0x00
#define PRVT_LOCALE_NL_LANGINFO		0x01
#define PRVT_LOCALE_LOCALECONV		0x02
#define PRVT_LOCALE_END			0x02

/*
** These are the methods found in the private __lc_charmap method table
*/

#define PRVT_CHARMAP_CHARMAP_INIT	0x00
#define PRVT_CHARMAP_NL_LANGINFO	0x01
#define PRVT_CHARMAP___MBSTOPCS		0x02
#define PRVT_CHARMAP___MBTOPC		0x03
#define PRVT_CHARMAP___PCSTOMBS		0x04
#define	PRVT_CHARMAP___PCTOMB		0x05
#define PRVT_CHARMAP_CSID		0x06
#define PRVT_CHARMAP_MBLEN		0x07
#define	PRVT_CHARMAP_MBSTOWCS		0x08
#define	PRVT_CHARMAP_MBTOWC		0x09
#define PRVT_CHARMAP_WCSID		0x0a
#define PRVT_CHARMAP_WCSTOMBS		0x0b
#define PRVT_CHARMAP_WCSWIDTH		0x0c
#define PRVT_CHARMAP_WCTOMB		0x0d
#define PRVT_CHARMAP_WCWIDTH		0x0e
#define PRVT_CHARMAP_END		0x0e

/*
** These are the methods found in the private __lc_ctype method table
*/

#define PRVT_CTYPE_CTYPE_INIT		0x00
#define PRVT_CTYPE_GET_WCTYPE		0x01
#define PRVT_CTYPE_IS_WCTYPE		0x02
#define PRVT_CTYPE_TOWLOWER		0x03
#define PRVT_CTYPE_TOWUPPER		0x04
#define PRVT_CTYPE_END			0x04

/*
** These are the methods found in the private __lc_collate method table
*/
#define PRVT_COLLATE_COLLATE_INIT	0x00
#define PRVT_COLLATE_FNMATCH		0x01
#define PRVT_COLLATE_REGCOMP		0x02
#define PRVT_COLLATE_REGERROR		0x03
#define PRVT_COLLATE_REGEXEC		0x04
#define PRVT_COLLATE_REGFREE		0x05
#define PRVT_COLLATE_STRCOLL		0x06
#define PRVT_COLLATE_STRXFRM		0x07
#define PRVT_COLLATE_WCSCOLL		0x08
#define PRVT_COLLATE_WCSXFRM		0x09
#define PRVT_COLLATE_END		0x09

/*
** These are the methods found in the private __lc_monetary method table
*/
#define PRVT_MONETARY_MONETARY_INIT	0x00
#define PRVT_MONETARY_NL_LANGINFO	0x01
#define PRVT_MONETARY_STRFMON		0x02
#define PRVT_MONETARY_END		0x02

/*
** These are the methods found in the private __lc_resp method table
*/
#define PRVT_RESP_RESP_INIT		0x00
#define PRVT_RESP_NL_LANGINFO		0x01
#define PRVT_RESP_RPMATCH		0x02
#define PRVT_RESP_END			0x02
#define PRVT_MSG_CATCLOSE		0x03
#define PRVT_MSG_CATGETS		0x04
#define PRVT_MSG_COMPRESS		0x05
#define PRVT_MSG_DECOMPRESS		0x06
#define PRVT_MSG_END_COMPRESS		0x07
#define PRVT_MSG_MSG_INIT		0x08

/* 
** These are the methods found in the private __lc_numeric method table
*/
#define PRVT_NUMERIC_NUMERIC_INIT	0x00
#define PRVT_NUMERIC_NL_LANGINFO	0x01
#define PRVT_NUMERIC_END		0x01

/*
** These are the methods found in the private __lc_time method table
*/
#define PRVT_TIME_TIME_INIT		0x00
#define PRVT_TIME_NL_LANGINFO		0x01
#define PRVT_TIME_STRFTIME		0x02
#define PRVT_TIME_STRPTIME		0x03
#define PRVT_TIME_WCSFTIME		0x04
#define PRVT_TIME_END			0x04

/*
** This is the max length of the XXX_NAME made large enough for
** future global names of methods to be of a reasonable length.
** This is used by gram.y.
*/

#define MAX_METHOD_NAME		50

#define __MBSTOPCS_SB_NAME	"__mbstopcs_sb"
#define __MBSTOPCS_932_NAME	"__mbstopcs_932"
#define __MBSTOPCS_EUCJP_NAME	"__mbstopcs_eucjp"
#define __MBTOPC_SB_NAME	"__mbtopc_sb"
#define __MBTOPC_932_NAME	"__mbtopc_932"
#define __MBTOPC_EUCJP_NAME	"__mbtopc_eucjp"
#define __PCSTOMBS_SB_NAME	"__pcstombs_sb"
#define __PCSTOMBS_932_NAME	"__pcstombs_932"
#define __PCSTOMBS_EUCJP_NAME	"__pcstombs_eucjp"
#define __PCTOMB_SB_NAME	"__pctomb_sb"
#define __PCTOMB_932_NAME	"__pctomb_932"
#define __PCTOMB_EUCJP_NAME	"__pctomb_eucjp"
#define CSID_STD_NAME		"__csid_std"
#define CHARMAP_INIT_NAME	"__charmap_init"
#define MBLEN_SB_NAME		"__mblen_sb"
#define MBLEN_932_NAME		"__mblen_932"
#define MBLEN_EUCJP_NAME	"__mblen_eucjp"
#define MBSTOWCS_SB_NAME	"__mbstowcs_sb"
#define MBSTOWCS_932_NAME	"__mbstowcs_932"
#define MBSTOWCS_EUCJP_NAME	"__mbstowcs_eucjp"
#define MBTOWC_SB_NAME		"__mbtowc_sb"
#define MBTOWC_932_NAME		"__mbtowc_932"
#define MBTOWC_EUCJP_NAME	"__mbtowc_eucjp"
#define NL_CSINFO_NAME		"nl_csinfo"
#define WCSID_STD_NAME		"__wcsid_std"
#define WCSTOMBS_SB_NAME	"__wcstombs_sb"
#define WCSTOMBS_932_NAME	"__wcstombs_932"
#define WCSTOMBS_EUCJP_NAME	"__wcstombs_eucjp"
#define WCSWIDTH_LATIN_NAME	"__wcswidth_latin"
#define WCSWIDTH_932_NAME	"__wcswidth_932"
#define WCSWIDTH_EUCJP_NAME	"__wcswidth_eucjp"
#define WCTOMB_SB_NAME		"__wctomb_sb"
#define WCTOMB_932_NAME		"__wctomb_932"
#define WCTOMB_EUCJP_NAME	"__wctomb_eucjp"
#define WCWIDTH_LATIN_NAME	"__wcwidth_latin"
#define WCWIDTH_932_NAME	"__wcwidth_932"
#define WCWIDTH_EUCJP_NAME	"__wcwidth_eucjp"
#define FNMATCH_C_NAME		"__fnmatch_C"
#define FNMATCH_STD_NAME	"__fnmatch_std"
#define REGCOMP_STD_NAME	"__regcomp_std"
#define REGERROR_STD_NAME	"__regerror_std"
#define REGEXEC_STD_NAME	"__regexec_std"
#define REGFREE_STD_NAME	"__regfree_std"
#define STRCOLL_C_NAME		"__strcoll_C"
#define STRCOLL_SB_NAME		"__strcoll_sb"
#define STRCOLL_STD_NAME	"__strcoll_std"
#define STRXFRM_C_NAME		"__strxfrm_C"
#define STRXFRM_SB_NAME		"__strxfrm_sb"
#define STRXFRM_STD_NAME	"__strxfrm_std"
#define WCSCOLL_C_NAME		"__wcscoll_C"
#define WCSCOLL_STD_NAME	"__wcscoll_std"
#define WCSXFRM_C_NAME		"__wcsxfrm_C"
#define WCSXFRM_STD_NAME	"__wcsxfrm_std"
#define COLLATE_INIT_NAME	"__collate_init"
#define GET_WCTYPE_STD_NAME	"__get_wctype_std"
#define CTYPE_INIT_NAME		"__ctype_init"
#define IS_WCTYPE_SB_NAME	"__is_wctype_sb"
#define IS_WCTYPE_STD_NAME	"__is_wctype_std"
#define TOWLOWER_STD_NAME	"__towlower_std"
#define TOWUPPER_STD_NAME	"__towupper_std"
#define LOCALE_INIT_NAME	"__locale_init"
#define LOCALECONV_STD_NAME	"__localeconv_std"
#define NL_LANGINFO_STD_NAME	"__nl_langinfo_std"
#define MONETARY_INIT_NAME	"__monetary_init"
#define NL_MONINFO_NAME		"nl_moninfo"
#define STRFMON_STD_NAME	"__strfmon_std"
#define NUMERIC_INIT_NAME	"__numeric_init"
#define NL_NUMINFO_NAME		"nl_numinfo"
#define RESP_INIT_NAME		"__resp_init"
#define NL_RESPINFO_NAME	"nl_respinfo"
#define RPMATCH_C_NAME		"__rpmatch_C"
#define RPMATCH_STD_NAME	"__rpmatch_std"
#define TIME_INIT_NAME		"__time_init"
#define NL_TIMINFO_NAME		"nl_timinfo"
#define STRFTIME_STD_NAME	"__strftime_std"
#define STRPTIME_STD_NAME	"__strptime_std"
#define WCSFTIME_STD_NAME	"__wcsftime_std"
