static char sccsid[] = "@(#)84	1.1  src/bldenv/meth/ko_KR/ko_KR_meth.c, cfgnls, bos412, GOLDA411a 3/11/94 12:06:17";
/*
 * COMPONENT_NAME: CFGNLS
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This file was generated by locdef.  It is the temporary method file that
 * locdef creates and loads to do codepoint conversion.  If the method (.m)
 * file changes, then this file must be recreated to reflect those changes.
 */
extern __mbstopcs_euckr(); 
extern __mbtopc_euckr(); 
extern __pcstombs_euckr(); 
extern __pctomb_euckr(); 
extern __csid_std(); 
extern __charmap_init(); 
extern __mblen_euckr(); 
extern __mbstowcs_euckr(); 
extern __mbtowc_euckr(); 
extern nl_csinfo(); 
extern __wcsid_std(); 
extern __wcstombs_euckr(); 
extern __wcswidth_euckr(); 
extern __wctomb_euckr(); 
extern __wcwidth_euckr(); 
extern __fnmatch_std(); 
extern __regcomp_std(); 
extern __regerror_std(); 
extern __regexec_std(); 
extern __regfree_std(); 
extern __strcoll_std(); 
extern __strxfrm_std(); 
extern __wcscoll_std(); 
extern __wcsxfrm_std(); 
extern __collate_init(); 
extern __get_wctype_std(); 
extern __ctype_init(); 
extern __is_wctype_std(); 
extern __towlower_std(); 
extern __towupper_std(); 
extern __locale_init(); 
extern __localeconv_std(); 
extern __nl_langinfo_std(); 
extern __monetary_init(); 
extern nl_moninfo(); 
extern __strfmon_std(); 
extern __numeric_init(); 
extern nl_numinfo(); 
extern __resp_init(); 
extern nl_respinfo(); 
extern __rpmatch_std(); 
extern __time_init(); 
extern nl_timinfo(); 
extern __strftime_std(); 
extern __strptime_std(); 
extern __wcsftime_std(); 

void * (*_tmp_method_tbl[])() = {
	__mbstopcs_euckr, 
	__mbtopc_euckr, 
	__pcstombs_euckr, 
	__pctomb_euckr, 
	__csid_std, 
	__charmap_init, 
	__mblen_euckr, 
	__mbstowcs_euckr, 
	__mbtowc_euckr, 
	nl_csinfo, 
	__wcsid_std, 
	__wcstombs_euckr, 
	__wcswidth_euckr, 
	__wctomb_euckr, 
	__wcwidth_euckr, 
	__fnmatch_std, 
	__regcomp_std, 
	__regerror_std, 
	__regexec_std, 
	__regfree_std, 
	__strcoll_std, 
	__strxfrm_std, 
	__wcscoll_std, 
	__wcsxfrm_std, 
	__collate_init, 
	__get_wctype_std, 
	__ctype_init, 
	__is_wctype_std, 
	__towlower_std, 
	__towupper_std, 
	__locale_init, 
	__localeconv_std, 
	__nl_langinfo_std, 
	__monetary_init, 
	nl_moninfo, 
	__strfmon_std, 
	0, 
	0, 
	0, 
	0, 
	0, 
	0, 
	0, 
	__numeric_init, 
	nl_numinfo, 
	__resp_init, 
	nl_respinfo, 
	__rpmatch_std, 
	__time_init, 
	nl_timinfo, 
	__strftime_std, 
	__strptime_std, 
	__wcsftime_std, 
};

typedef struct { 
		  char *method_name;
		  char *c_symbol[4];
		  char *pkg_name[4];
		  int method[4];
		 } std_method_t;


std_method_t _tmp_std_methods[]={
{"charmap.__mbstopcs",
  {0, 0, 0, "__mbstopcs_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 0}},
{"charmap.__mbtopc",
  {0, 0, 0, "__mbtopc_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 1}},
{"charmap.__pcstombs",
  {0, 0, 0, "__pcstombs_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 2}},
{"charmap.__pctomb",
  {0, 0, 0, "__pctomb_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 3}},
{"charmap.csid",
  {0, 0, 0, "__csid_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 4}},
{"charmap.init",
  {0, 0, 0, "__charmap_init"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 5}},
{"charmap.mblen",
  {0, 0, 0, "__mblen_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 6}},
{"charmap.mbstowcs",
  {0, 0, 0, "__mbstowcs_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 7}},
{"charmap.mbtowc",
  {0, 0, 0, "__mbtowc_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 8}},
{"charmap.nl_langinfo",
  {0, 0, 0, "nl_csinfo"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 9}},
{"charmap.wcsid",
  {0, 0, 0, "__wcsid_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 10}},
{"charmap.wcstombs",
  {0, 0, 0, "__wcstombs_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 11}},
{"charmap.wcswidth",
  {0, 0, 0, "__wcswidth_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 12}},
{"charmap.wctomb",
  {0, 0, 0, "__wctomb_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 13}},
{"charmap.wcwidth",
  {0, 0, 0, "__wcwidth_euckr"},
  {0, 0, 0, "ko_KR.o"},
  {-1, -1, -1, 14}},
{"collate.fnmatch",
  {0, 0, 0, "__fnmatch_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 15}},
{"collate.regcomp",
  {0, 0, 0, "__regcomp_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 16}},
{"collate.regerror",
  {0, 0, 0, "__regerror_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 17}},
{"collate.regexec",
  {0, 0, 0, "__regexec_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 18}},
{"collate.regfree",
  {0, 0, 0, "__regfree_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 19}},
{"collate.strcoll",
  {0, 0, 0, "__strcoll_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 20}},
{"collate.strxfrm",
  {0, 0, 0, "__strxfrm_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 21}},
{"collate.wcscoll",
  {0, 0, 0, "__wcscoll_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 22}},
{"collate.wcsxfrm",
  {0, 0, 0, "__wcsxfrm_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 23}},
{"collate.init",
  {0, 0, 0, "__collate_init"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 24}},
{"ctype.get_wctype",
  {0, 0, 0, "__get_wctype_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 25}},
{"ctype.init",
  {0, 0, 0, "__ctype_init"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 26}},
{"ctype.is_wctype",
  {0, 0, 0, "__is_wctype_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 27}},
{"ctype.towlower",
  {0, 0, 0, "__towlower_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 28}},
{"ctype.towupper",
  {0, 0, 0, "__towupper_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 29}},
{"locale.init",
  {0, 0, 0, "__locale_init"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 30}},
{"locale.localeconv",
  {0, 0, 0, "__localeconv_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 31}},
{"locale.nl_langinfo",
  {0, 0, 0, "__nl_langinfo_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 32}},
{"monetary.init",
  {0, 0, 0, "__monetary_init"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 33}},
{"monetary.nl_langinfo",
  {0, 0, 0, "nl_moninfo"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 34}},
{"monetary.strfmon",
  {0, 0, 0, "__strfmon_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 35}},
{"msg.catclose",
  {0, 0, 0, "0"},
  {0, 0, 0, ""},
  {-1, -1, -1, 36}},
{"msg.catgets",
  {0, 0, 0, "0"},
  {0, 0, 0, ""},
  {-1, -1, -1, 37}},
{"msg.compress",
  {0, 0, 0, "0"},
  {0, 0, 0, ""},
  {-1, -1, -1, 38}},
{"msg.decompress",
  {0, 0, 0, "0"},
  {0, 0, 0, ""},
  {-1, -1, -1, 39}},
{"msg.end_compress",
  {0, 0, 0, "0"},
  {0, 0, 0, ""},
  {-1, -1, -1, 40}},
{"msg.init",
  {0, 0, 0, "0"},
  {0, 0, 0, ""},
  {-1, -1, -1, 41}},
{"msg.start_compress",
  {0, 0, 0, "0"},
  {0, 0, 0, ""},
  {-1, -1, -1, 42}},
{"numeric.init",
  {0, 0, 0, "__numeric_init"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 43}},
{"numeric.nl_langinfo",
  {0, 0, 0, "nl_numinfo"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 44}},
{"resp.init",
  {0, 0, 0, "__resp_init"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 45}},
{"resp.nl_langinfo",
  {0, 0, 0, "nl_respinfo"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 46}},
{"resp.rpmatch",
  {0, 0, 0, "__rpmatch_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 47}},
{"time.init",
  {0, 0, 0, "__time_init"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 48}},
{"time.nl_langinfo",
  {0, 0, 0, "nl_timinfo"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 49}},
{"time.strftime",
  {0, 0, 0, "__strftime_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 50}},
{"time.strptime",
  {0, 0, 0, "__strptime_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 51}},
{"time.wcsftime",
  {0, 0, 0, "__wcsftime_std"},
  {0, 0, 0, "-lc"},
  {-1, -1, -1, 52}},
};

	 typedef struct {
		 std_method_t * std_meth_ptr;
		 void * (**method_tbl_ptr)();
} std_meth_ptr_t;
	std_meth_ptr_t std_method_hdl = {
	 &_tmp_std_methods,
	 &_tmp_method_tbl,
};
