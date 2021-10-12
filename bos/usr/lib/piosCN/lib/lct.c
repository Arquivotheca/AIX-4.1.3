static char sccsid[] = "@(#)89	1.2  src/bos/usr/lib/piosCN/lib/lct.c, ils-zh_CN, bos41J, 9516A_all 4/17/95 15:24:15";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include	"lc.h"

FONTinforec FI[] = {
/*0*/	{ NO_QUERY, 0, "iso8859",	"1" },
	{ NO_QUERY, 0, "iso8859",	"7" },
	{ NO_QUERY, 0, "iso8859",	"9" },
	{ NO_QUERY, 0, "jisx0201.1976",	"0" },
	{ NO_QUERY, 0, "jisx0201.1976",	"1" },
/*5*/	{ NO_QUERY, 0, "jisx0208.1983",	"0" },
	{ NO_QUERY, 0, "jisx0212.1990",	"0" },
	{ NO_QUERY, 0, "ksc5601.1987",	"0" },
	{ NO_QUERY, 0, "cns11643.1986",	"1" },
	{ NO_QUERY, 0, "cns11643.1986",	"2" },
/*10*/	{ NO_QUERY, 0, "ibm",		"850" },
	{ NO_QUERY, 0, "ibm",		"pc850" },
	{ NO_QUERY, 0, "ibm",		"udcjp" },
	{ NO_QUERY, 0, "ibm",		"ibmcs01007ext" },
	{ NO_QUERY, 0, "ibm",		"udctw" },
/*15*/	{ NO_QUERY, 0, "ibm",		"sbdtw" },
	{ NO_QUERY, 0, "cns11643.1986", "3" },
	{ NO_QUERY, 0, "cns11643.1986", "e" },
	{ NO_QUERY, 0, "cns11643.1986", "f" },
	{ NO_QUERY, 0, "ibm",		"sbstw" },
/*20*/	{ NO_QUERY, 0, "gb2312.1980", "0" },
	{ NO_QUERY, 0, "ibm",		"sbdcn" },
	{ NO_QUERY, 0, "ibm",		"udccn" },
	{ NO_QUERY, 0, "ucs2.gb2312.1980", "0" },
	{ NO_QUERY, 0, "ucs2.cns11643.1986",	"1" },
/*25*/	{ NO_QUERY, 0, "ucs2.cns11643.1986",	"2" },
	{ NO_QUERY, 0, "ucs2.cjk",	"rest" },
	{ NO_QUERY, 0, 0,		0 },
};

FONTtblrec FT[] = {
/*IBM850*/
/*0*/	{ 0, FI + 10 },
	{ 0, FI },
	{ 0, FI + 11 },
/*3*/	{ 0, FI + 10 },
	{ 0, FI },
	{ 0, FI + 11 },
/*6*/	{ 0, FI + 10 },
	{ 0, FI + 11 },

/*ISO8859-1*/
/*8*/	{ 0, FI },
	{ 0, FI + 10 },
	{ 0, FI + 11 },
/*11*/	{ 0, FI },
	{ 0, FI + 10 },
	{ 0, FI + 11 },

/*ISO8859-7*/
/*14*/	{ 0, FI + 1 },
	{ 0, FI },
/*16*/	{ 0, FI + 1 },

/*ISO8859-9*/
/*17*/	{ 0, FI + 2 },
	{ 0, FI },
/*19*/	{ 0, FI + 2 },

/*932,eucJP*/
/*20*/	{ 0, FI + 3 },
	{ 0, FI + 10 },
	{ 0, FI },
	{ 0, FI + 11 },
/*24*/	{ 0, FI + 5 },
/*25*/	{ 0, FI + 3 },
/*26*/	{ 0, FI + 12 }, /* for JP ibm selected */
	{ 0, FI + 13 },
/*28*/	{ 0, FI + 12 },	/* for JP user defined */
	{ 0, FI + 13 },

/*KR*/
/*30*/	{ 0, FI },
/*31*/	{ 0, FI + 7 },

/*TW*/
/*32*/	{ 0, FI },
/*33*/	{ 0, FI + 8 },
/*34*/	{ 0, FI + 9 },
/*35*/	{ 0, FI + 14 },
/*36*/	{ 0, FI + 15 },
/*37*/	{ 0, FI + 19 },
/*38*/	{ 0, FI + 16 },
/*39*/	{ 0, FI + 17 },
/*40*/	{ 0, FI + 18 },

/*CN*/
/*41*/	{ 0, FI },
/*42*/	{ 0, FI + 20 },
/*43*/	{ 0, FI + 23 },
/*44*/	{ 0, FI + 21 },
/*45*/	{ 0, FI + 22 },

/*ZH*/
/*46*/	{ 0, FI },
/*47*/	{ 0, FI + 20 },
/*48*/	{ 0, FI + 23 },
/*49*/	{ 0, FI + 8 },
/*50*/	{ 0, FI + 24 },
/*51*/	{ 0, FI + 9 },
/*52*/	{ 0, FI + 25 },
/*53*/	{ 0, FI + 26 },
/*54*/	{ 0, FI + 21 },
/*55*/	{ 0, FI + 22 },
/*56*/	{ 0, FI + 14 },
/*57*/	{ 0, FI + 15 },
	{ 0, 0 }
};

CSinforec	CSI[] = {
/*IBM-850*/
/*0*/	{ FONT_P, 1, 0, "IBM-850", 3, FT },
	{ FONT_X, 1, 0, "IBM-850", 3, FT + 3 },
	{ FONT_X, 1, 0, "IBM-850", 2, FT + 6 },
/*ISO8859-1*/
/*3*/	{ FONT_P, 1, 0, "ISO8859-1", 3, FT + 8 },
	{ FONT_X, 1, 0, "ISO8859-1", 3, FT + 11 },
/*ISO8859-7*/
/*5*/	{ FONT_P, 1, 0, "ISO8859-7", 2, FT + 14 },
	{ FONT_X, 1, 0, "ISO8859-7", 1, FT + 16 },
/*ISO8859-9*/
/*7*/	{ FONT_P, 1, 0, "ISO8859-9", 2, FT + 17 },
	{ FONT_X, 1, 0, "ISO8859-9", 1, FT + 19 },
/*932,eucJP*/
/*9*/	{ FONT_P, 1, 0, "JISX0201.1976-0", 4, FT + 20 },
	{ FONT_X, 2, 0, "JISX0208.1983-0", 1, FT + 24 },
	{ FONT_X, 1, 0, "JISX0201.1976-0", 1, FT + 25 },
	{ FONT_X, 2, 0, "IBM-udcJP", 2, FT + 26 },	/* JP ibm selected */
	{ FONT_X, 2, 0, "IBM-udcJP", 2, FT + 28 },	/* JP user defined */
/*KR*/
/*14*/	{ FONT_P, 1, 0, "ISO8859-1", 1, FT + 30 },
	{ FONT_X, 2, 0, "KSC5601.1987-0", 1, FT + 31 },
/*TW*/
/*16*/	{ FONT_P, 1, 0, "ISO8859-1", 1, FT + 32 },
	{ FONT_X, 2, 0, "CNS11643.1986-1", 1, FT + 33 },
	{ FONT_X, 2, 0, "CNS11643.1986-2", 1, FT + 34 },
	{ FONT_X, 2, 0, "IBM-udcTW", 1, FT + 35 },
	{ FONT_X, 2, 0, "IBM-sbdTW", 1, FT + 36 },
	{ FONT_X, 2, 0, "CNS11643.1986-3", 1, FT + 37 },
	{ FONT_X, 2, 0, "CNS11643.1986-E", 1, FT + 38 },
	{ FONT_X, 2, 0, "CNS11643.1986-F", 1, FT + 39 },
	{ FONT_X, 1, 0, "IBM-sbsTW", 1, FT + 40 },
/*CN*/
/*25*/	{ FONT_X, 1, 0, "ISO8859-1", 1, FT + 41},
	{ FONT_X, 2, 0, "GB2312.1980-0", 2, FT + 42},
	{ FONT_X, 2, 0, "IBM-sbdCN", 1, FT + 44},
	{ FONT_X, 2, 0, "IBM-udcCN", 1, FT + 45},
/*ZH*/
/*29*/	{ FONT_X, 1, 0, "ISO8859-1", 1, FT + 46},
	{ FONT_X, 2, 0, "GB2312.1980-0", 2, FT + 47},
	{ FONT_X, 2, 0, "CNS11643.1986-1", 2, FT + 49 },
	{ FONT_X, 2, 0, "CNS11643.1986-2", 2, FT + 51 },
	{ FONT_X, 2, 0, "UCS2.CJK-REST", 1, FT + 53 },
	{ FONT_X, 2, 0, "IBM-sbdCN", 1, FT + 54 },
	{ FONT_X, 2, 0, "IBM-udcCN", 1, FT + 55 },
	{ FONT_X, 2, 0, "IBM-udcTW", 1, FT + 56 },
	{ FONT_X, 2, 0, "IBM-sbdTW", 1, FT + 57 },
/*38*/	{ NO_FONT, 0, 0, 0, 0, 0 }
};


CODEinforec CDI[] = {
/*0*/	{ "IBM-850",	"En_US", 3, CSI },
	{ "ISO8859-1",	"en_US", 2, CSI + 3 },
	{ "ISO8859-7",	"el_GR", 2, CSI + 5 },
	{ "ISO8859-9",	"tr_TR", 2, CSI + 7 },
	{ "IBM-932",	"Ja_JP", 5, CSI + 9  },
/*5*/	{ "IBM-eucJP",	"ja_JP", 5, CSI + 9 },
	{ "IBM-eucKR",	"ko_KR", 2, CSI + 14},
	{ "IBM-eucTW",	"zh_TW", 9, CSI + 16 },
	{ "IBM-eucCN",	"zh_CN", 4, CSI + 25 },
	{ "UTF-8",	"ZH_CN", 9, CSI + 29 },
/*10*/	{ 0, 0, 0, 0 }
};

LOCinforec LCI[] = {
/*0*/	{ "C", CDI },
	{ "da_DK", CDI + 1 },
	{ "de_CH", CDI + 1 },
	{ "de_DE", CDI + 1 },
	{ "en_GB", CDI + 1 },
/*5*/	{ "en_US", CDI + 1 },
	{ "es_ES", CDI + 1 },
	{ "fi_FI", CDI + 1 },
	{ "fr_BE", CDI + 1 },
	{ "fr_CA", CDI + 1 },
/*10*/	{ "fr_FR", CDI + 1 },
	{ "fr_CH", CDI + 1 },
	{ "is_IS", CDI + 1 },
	{ "it_IT", CDI + 1 },
	{ "nl_BE", CDI + 1 },
/*15*/	{ "nl_NL", CDI + 1 },
	{ "no_NO", CDI + 1 },
	{ "pt_PT", CDI + 1 },
	{ "sv_SE", CDI + 1 },
	{ "en_JP", CDI + 5 },
/*20*/	{ "ja_JP", CDI + 5 },
	{ "el_GR", CDI + 2 },
	{ "tr_TR", CDI + 3 },
	{ "en_KR", CDI + 6 },
	{ "ko_KR", CDI + 6 },
/*25*/	{ "en_TW", CDI + 7 },
	{ "zh_TW", CDI + 7 },
	{ "Da_DK", CDI },
	{ "De_CH", CDI },
	{ "De_DE", CDI },
/*30*/	{ "En_GB", CDI },
	{ "En_US", CDI },
	{ "Es_ES", CDI },
	{ "Fi_FI", CDI },
	{ "Fr_BE", CDI },
/*35*/	{ "Fr_CA", CDI },
	{ "Fr_FR", CDI },
	{ "Fr_CH", CDI },
	{ "Is_IS", CDI },
	{ "It_IT", CDI },
/*40*/	{ "Nl_BE", CDI },
	{ "Nl_NL", CDI },
	{ "No_NO", CDI },
	{ "Pt_PT", CDI },
	{ "Sv_SE", CDI },
/*45*/	{ "En_JP", CDI + 4 },
	{ "Ja_JP", CDI + 4 },
	{ "Jp_JP", CDI + 4 },
	{ "en_CN", CDI + 8 },
	{ "zh_CN", CDI + 8 },
/*50*/	{ "en_ZH", CDI + 9 },
	{ "ZH_CN", CDI + 9 },
	{ 0, 0 }
};
