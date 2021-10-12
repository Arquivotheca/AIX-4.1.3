static char sccsid[] = "@(#)65	1.15.2.10  src/bos/usr/ccs/lib/libc/NLSsetup.c, libcloc, bos410 4/12/94 13:01:46";
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
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1994
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

/*
  Initialization methods
*/
void  __charmap_init(_LC_locale_objhdl_t);
void  __collate_init(_LC_locale_objhdl_t);
void  __ctype_init(_LC_locale_objhdl_t);
void  __monetary_init(_LC_locale_objhdl_t);
void  __numeric_init(_LC_locale_objhdl_t);
void  __resp_init(_LC_locale_objhdl_t);
void  __time_init(_LC_locale_objhdl_t);
void  __locale_init(_LC_locale_objhdl_t);

/* 
  Forward declaration
*/
_LC_locale_t __C_locale_object;

#if 0
/*

  This table was used to support 3.1 binary objects on 3.2.
  Since 3.1 binary support is no longer required on 4.x, this
  code is not needed.  In the future, I would recommend that 
  this code actually be removed from this file.
  
  This table supports only the 3.1 version of the MB_CUR_MAX macro.
*/

static wchar_t _C_xlate_31[] = {
0x0000, 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 
0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 
0x000f, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 
0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 
0x001f, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 
0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 
0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 
0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 
0x003f, 0x0040, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 
0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 
0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 
0x0077, 0x0078, 0x0079, 0x007a, 0x005b, 0x005c, 0x005d, 0x005e, 
0x005f, 0x0060, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 
0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 
0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 
0x0057, 0x0058, 0x0059, 0x005a, 0x007b, 0x007c, 0x007d, 0x007e, 
0x007f, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 
0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 
0x008f, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 
0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 
0x009f, 0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 
0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 
0x00af, 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 
0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 
0x00bf, 0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 
0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 
0x00cf, 0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 
0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 
0x00df, 0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 
0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 
0x00ef, 0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 
0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 
0x00ff,
};

static ctype_t _locp_chrtbl={
    0, 0, 0, 1, 1, 1, "C", 0, 0 /*was: _C_xlate_31*/, 0, NULL,
};

static loc_t _locp_data = {
    'Z','Z', 0, 0, 0,
    NULL,	       /* LC_COLLATE */
    &_locp_chrtbl,     /* LC_CTYPE */
    NULL,	       /* LC_MONETARY */
    NULL,	       /* LC_NUMERIC */
    NULL,	       /* LC_TIME */
    NULL,	       /* LC_MESSAGES */
    NULL,	       /* ... */
};

loc_t * _locp = &_locp_data;

#endif

static unsigned char cm_csmap[]={
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

_LC_charmap_t _C_charmap_object={
  /*
  ** Object header info
  */
  _LC_CHARMAP,               /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  0,			     /* size    */

  /* 
  ** charmap methods for C_locale
  */
  NL_CSINFO,
  
  MBTOWC_SB,
  MBSTOWCS_SB,
  WCTOMB_SB,
  WCSTOMBS_SB,

  MBLEN_SB,

  WCSWIDTH_LATIN,
  WCWIDTH_LATIN,

  __MBTOPC_SB,
  __MBSTOPCS_SB,
  __PCTOMB_SB,
  __PCSTOMBS_SB,

  CSID_STD,                  
  WCSID_STD,
  
  CHARMAP_INIT,		     /* init method */
  0,			     /* void * data */

  /*
  ** extension data
  */
  "ISO8859-1",		     /* codeset name*/
  1,			     /* cm_mb_cur_max */
  1,			     /* cm_mb_cur_min */
  1,			     /* max display width for this codeset */
  cm_csmap,		     /* cm_csmap */

  &__C_locale_object,	     /* loc_rec */
  0,			     /* __meth_ptr */
  0,                         /* __data_ptr */
};


#include <ctype.h>

_LC_classnm_t _C_classnms[]={
  "alnum", _ISALNUM,
  "alpha", _ISALPHA,
  "blank", _ISBLANK,
  "cntrl", _ISCNTRL,
  "digit", _ISDIGIT,
  "graph", _ISGRAPH,
  "lower", _ISLOWER,
  "print", _ISPRINT,
  "punct", _ISPUNCT,
  "space", _ISSPACE,
  "upper", _ISUPPER,
  "xdigit", _ISXDIGIT
};

wchar_t _C_upper[] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
	0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
	0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f, 
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 
	0x0060, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
	0x0058, 0x0059, 0x005a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 
	0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 
	0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f, 
	0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 
	0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f, 
	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 
	0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af, 
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 
	0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 
	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 
	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 
	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 
	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 
	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff};
	      	
wchar_t _C_lower[] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
	0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
	0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f, 
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
	0x0040, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
	0x0078, 0x0079, 0x007a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,  
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 
	0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 
	0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f, 
	0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 
	0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f, 
	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 
	0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af, 
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 
	0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 
	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 
	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 
	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 
	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 
	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff};

#define _C _ISCNTRL
#define _B (_ISBLANK|_ISPRINT|_ISSPACE)
#define _T (_ISBLANK|_ISSPACE)
#define _V _ISSPACE
#define _P (_ISPUNCT|_ISPRINT|_ISGRAPH)
#define _X _ISXDIGIT
#define _U (_ISUPPER|_ISALPHA|_ISALNUM|_ISPRINT|_ISGRAPH)
#define _L (_ISLOWER|_ISALPHA|_ISALNUM|_ISPRINT|_ISGRAPH)
#define _G (_ISGRAPH|_ISPRINT)
#define _N (_ISDIGIT|_ISALNUM|_ISPRINT|_ISGRAPH)
unsigned int _C_masks[]={

/*	 0	 1	 2	 3	 4	 5	 6	 7  */
/* 0*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 10*/	_C,	_T|_C,	_V|_C,	_V|_C,	_V|_C,	_V|_C,	_C,	_C,
/* 20*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 30*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 40*/	_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 50*/	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 60*/	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,
/* 70*/	_N|_X,	_N|_X,	_P,	_P,	_P,	_P,	_P,	_P,
/*100*/	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,
/*110*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*120*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*130*/	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P,
/*140*/	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,
/*150*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*160*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*170*/	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C,
/*200*/	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	  0,	 0,	 0,	 0,	 0,	 0,	 0,	 0
};

_LC_ctype_t _C_ctype_object={
  /*
  ** Object header info
  */
  _LC_CTYPE,                 /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  0,			     /* size    */

  /*
  ** Character Attributes Methods
  */
  TOWUPPER_STD,
  TOWLOWER_STD,

  GET_WCTYPE_STD,
  IS_WCTYPE_SB,

  CTYPE_INIT,		     /* init method */
  0,			     /* was: _C_xlate_31 (void * data) */

  /* 
  ** class extension data.
  */
  0,			    /* min process code */
  255,			    /* max process code */
  _C_upper,		    /* upper */
  _C_lower,		    /* lower */

  _C_masks,                 /* array of classification masks */
  0, 0,                     /* qidx, qmask = 0 */

  sizeof(_C_classnms) / sizeof(_LC_classnm_t),
  _C_classnms,

  &__C_locale_object,	     /* loc_rec */
  0,			     /* __meth_ptr */
  0,                         /* __data_ptr */
};

_LC_coltbl_t _C_coltbl[]={
    0x101,0x101,0,0,0x101,0,  0x102,0x102,0,0,0x102,0,
    0x103,0x103,0,0,0x103,0,  0x104,0x104,0,0,0x104,0,
    0x105,0x105,0,0,0x105,0,  0x106,0x106,0,0,0x106,0,
    0x107,0x107,0,0,0x107,0,  0x108,0x108,0,0,0x108,0,
    0x109,0x109,0,0,0x109,0,  0x10A,0x10A,0,0,0x10A,0,
    0x10B,0x10B,0,0,0x10B,0,  0x10C,0x10C,0,0,0x10C,0,
    0x10D,0x10D,0,0,0x10D,0,  0x10E,0x10E,0,0,0x10E,0,
    0x10F,0x10F,0,0,0x10F,0,  0x110,0x110,0,0,0x110,0,
    0x111,0x111,0,0,0x111,0,  0x112,0x112,0,0,0x112,0,
    0x113,0x113,0,0,0x113,0,  0x114,0x114,0,0,0x114,0,
    0x115,0x115,0,0,0x115,0,  0x116,0x116,0,0,0x116,0,
    0x117,0x117,0,0,0x117,0,  0x118,0x118,0,0,0x118,0,
    0x119,0x119,0,0,0x119,0,  0x11A,0x11A,0,0,0x11A,0,
    0x11B,0x11B,0,0,0x11B,0,  0x11C,0x11C,0,0,0x11C,0,
    0x11D,0x11D,0,0,0x11D,0,  0x11E,0x11E,0,0,0x11E,0,
    0x11F,0x11F,0,0,0x11F,0,  0x120,0x120,0,0,0x120,0,
    0x121,0x121,0,0,0x121,0,  0x122,0x122,0,0,0x122,0,
    0x123,0x123,0,0,0x123,0,  0x124,0x124,0,0,0x124,0,
    0x125,0x125,0,0,0x125,0,  0x126,0x126,0,0,0x126,0,
    0x127,0x127,0,0,0x127,0,  0x128,0x128,0,0,0x128,0,
    0x129,0x129,0,0,0x129,0,  0x12A,0x12A,0,0,0x12A,0,
    0x12B,0x12B,0,0,0x12B,0,  0x12C,0x12C,0,0,0x12C,0,
    0x12D,0x12D,0,0,0x12D,0,  0x12E,0x12E,0,0,0x12E,0,
    0x12F,0x12F,0,0,0x12F,0,  0x130,0x130,0,0,0x130,0,
    0x131,0x131,0,0,0x131,0,  0x132,0x132,0,0,0x132,0,
    0x133,0x133,0,0,0x133,0,  0x134,0x134,0,0,0x134,0,
    0x135,0x135,0,0,0x135,0,  0x136,0x136,0,0,0x136,0,
    0x137,0x137,0,0,0x137,0,  0x138,0x138,0,0,0x138,0,
    0x139,0x139,0,0,0x139,0,  0x13A,0x13A,0,0,0x13A,0,
    0x13B,0x13B,0,0,0x13B,0,  0x13C,0x13C,0,0,0x13C,0,
    0x13D,0x13D,0,0,0x13D,0,  0x13E,0x13E,0,0,0x13E,0,
    0x13F,0x13F,0,0,0x13F,0,  0x140,0x140,0,0,0x140,0,
    0x141,0x141,0,0,0x141,0,  0x142,0x142,0,0,0x142,0,
    0x143,0x143,0,0,0x143,0,  0x144,0x144,0,0,0x144,0,
    0x145,0x145,0,0,0x145,0,  0x146,0x146,0,0,0x146,0,
    0x147,0x147,0,0,0x147,0,  0x148,0x148,0,0,0x148,0,
    0x149,0x149,0,0,0x149,0,  0x14A,0x14A,0,0,0x14A,0,
    0x14B,0x14B,0,0,0x14B,0,  0x14C,0x14C,0,0,0x14C,0,
    0x14D,0x14D,0,0,0x14D,0,  0x14E,0x14E,0,0,0x14E,0,
    0x14F,0x14F,0,0,0x14F,0,  0x150,0x150,0,0,0x150,0,
    0x151,0x151,0,0,0x151,0,  0x152,0x152,0,0,0x152,0,
    0x153,0x153,0,0,0x153,0,  0x154,0x154,0,0,0x154,0,
    0x155,0x155,0,0,0x155,0,  0x156,0x156,0,0,0x156,0,
    0x157,0x157,0,0,0x157,0,  0x158,0x158,0,0,0x158,0,
    0x159,0x159,0,0,0x159,0,  0x15A,0x15A,0,0,0x15A,0,
    0x15B,0x15B,0,0,0x15B,0,  0x15C,0x15C,0,0,0x15C,0,
    0x15D,0x15D,0,0,0x15D,0,  0x15E,0x15E,0,0,0x15E,0,
    0x15F,0x15F,0,0,0x15F,0,  0x160,0x160,0,0,0x160,0,
    0x161,0x161,0,0,0x161,0,  0x162,0x162,0,0,0x162,0,
    0x163,0x163,0,0,0x163,0,  0x164,0x164,0,0,0x164,0,
    0x165,0x165,0,0,0x165,0,  0x166,0x166,0,0,0x166,0,
    0x167,0x167,0,0,0x167,0,  0x168,0x168,0,0,0x168,0,
    0x169,0x169,0,0,0x169,0,  0x16A,0x16A,0,0,0x16A,0,
    0x16B,0x16B,0,0,0x16B,0,  0x16C,0x16C,0,0,0x16C,0,
    0x16D,0x16D,0,0,0x16D,0,  0x16E,0x16E,0,0,0x16E,0,
    0x16F,0x16F,0,0,0x16F,0,  0x170,0x170,0,0,0x170,0,
    0x171,0x171,0,0,0x171,0,  0x172,0x172,0,0,0x172,0,
    0x173,0x173,0,0,0x173,0,  0x174,0x174,0,0,0x174,0,
    0x175,0x175,0,0,0x175,0,  0x176,0x176,0,0,0x176,0,
    0x177,0x177,0,0,0x177,0,  0x178,0x178,0,0,0x178,0,
    0x179,0x179,0,0,0x179,0,  0x17A,0x17A,0,0,0x17A,0,
    0x17B,0x17B,0,0,0x17B,0,  0x17C,0x17C,0,0,0x17C,0,
    0x17D,0x17D,0,0,0x17D,0,  0x17E,0x17E,0,0,0x17E,0,
    0x17F,0x17F,0,0,0x17F,0,  0x180,0x180,0,0,0x180,0,
    0x181,0x181,0,0,0x181,0,  0x182,0x182,0,0,0x182,0,
    0x183,0x183,0,0,0x183,0,  0x184,0x184,0,0,0x184,0,
    0x185,0x185,0,0,0x185,0,  0x186,0x186,0,0,0x186,0,
    0x187,0x187,0,0,0x187,0,  0x188,0x188,0,0,0x188,0,
    0x189,0x189,0,0,0x189,0,  0x18A,0x18A,0,0,0x18A,0,
    0x18B,0x18B,0,0,0x18B,0,  0x18C,0x18C,0,0,0x18C,0,
    0x18D,0x18D,0,0,0x18D,0,  0x18E,0x18E,0,0,0x18E,0,
    0x18F,0x18F,0,0,0x18F,0,  0x190,0x190,0,0,0x190,0,
    0x191,0x191,0,0,0x191,0,  0x192,0x192,0,0,0x192,0,
    0x193,0x193,0,0,0x193,0,  0x194,0x194,0,0,0x194,0,
    0x195,0x195,0,0,0x195,0,  0x196,0x196,0,0,0x196,0,
    0x197,0x197,0,0,0x197,0,  0x198,0x198,0,0,0x198,0,
    0x199,0x199,0,0,0x199,0,  0x19A,0x19A,0,0,0x19A,0,
    0x19B,0x19B,0,0,0x19B,0,  0x19C,0x19C,0,0,0x19C,0,
    0x19D,0x19D,0,0,0x19D,0,  0x19E,0x19E,0,0,0x19E,0,
    0x19F,0x19F,0,0,0x19F,0,  0x1A0,0x1A0,0,0,0x1A0,0,
    0x1A1,0x1A1,0,0,0x1A1,0,  0x1A2,0x1A2,0,0,0x1A2,0,
    0x1A3,0x1A3,0,0,0x1A3,0,  0x1A4,0x1A4,0,0,0x1A4,0,
    0x1A5,0x1A5,0,0,0x1A5,0,  0x1A6,0x1A6,0,0,0x1A6,0,
    0x1A7,0x1A7,0,0,0x1A7,0,  0x1A8,0x1A8,0,0,0x1A8,0,
    0x1A9,0x1A9,0,0,0x1A9,0,  0x1AA,0x1AA,0,0,0x1AA,0,
    0x1AB,0x1AB,0,0,0x1AB,0,  0x1AC,0x1AC,0,0,0x1AC,0,
    0x1AD,0x1AD,0,0,0x1AD,0,  0x1AE,0x1AE,0,0,0x1AE,0,
    0x1AF,0x1AF,0,0,0x1AF,0,  0x1B0,0x1B0,0,0,0x1B0,0,
    0x1B1,0x1B1,0,0,0x1B1,0,  0x1B2,0x1B2,0,0,0x1B2,0,
    0x1B3,0x1B3,0,0,0x1B3,0,  0x1B4,0x1B4,0,0,0x1B4,0,
    0x1B5,0x1B5,0,0,0x1B5,0,  0x1B6,0x1B6,0,0,0x1B6,0,
    0x1B7,0x1B7,0,0,0x1B7,0,  0x1B8,0x1B8,0,0,0x1B8,0,
    0x1B9,0x1B9,0,0,0x1B9,0,  0x1BA,0x1BA,0,0,0x1BA,0,
    0x1BB,0x1BB,0,0,0x1BB,0,  0x1BC,0x1BC,0,0,0x1BC,0,
    0x1BD,0x1BD,0,0,0x1BD,0,  0x1BE,0x1BE,0,0,0x1BE,0,
    0x1BF,0x1BF,0,0,0x1BF,0,  0x1C0,0x1C0,0,0,0x1C0,0,
    0x1C1,0x1C1,0,0,0x1C1,0,  0x1C2,0x1C2,0,0,0x1C2,0,
    0x1C3,0x1C3,0,0,0x1C3,0,  0x1C4,0x1C4,0,0,0x1C4,0,
    0x1C5,0x1C5,0,0,0x1C5,0,  0x1C6,0x1C6,0,0,0x1C6,0,
    0x1C7,0x1C7,0,0,0x1C7,0,  0x1C8,0x1C8,0,0,0x1C8,0,
    0x1C9,0x1C9,0,0,0x1C9,0,  0x1CA,0x1CA,0,0,0x1CA,0,
    0x1CB,0x1CB,0,0,0x1CB,0,  0x1CC,0x1CC,0,0,0x1CC,0,
    0x1CD,0x1CD,0,0,0x1CD,0,  0x1CE,0x1CE,0,0,0x1CE,0,
    0x1CF,0x1CF,0,0,0x1CF,0,  0x1D0,0x1D0,0,0,0x1D0,0,
    0x1D1,0x1D1,0,0,0x1D1,0,  0x1D2,0x1D2,0,0,0x1D2,0,
    0x1D3,0x1D3,0,0,0x1D3,0,  0x1D4,0x1D4,0,0,0x1D4,0,
    0x1D5,0x1D5,0,0,0x1D5,0,  0x1D6,0x1D6,0,0,0x1D6,0,
    0x1D7,0x1D7,0,0,0x1D7,0,  0x1D8,0x1D8,0,0,0x1D8,0,
    0x1D9,0x1D9,0,0,0x1D9,0,  0x1DA,0x1DA,0,0,0x1DA,0,
    0x1DB,0x1DB,0,0,0x1DB,0,  0x1DC,0x1DC,0,0,0x1DC,0,
    0x1DD,0x1DD,0,0,0x1DD,0,  0x1DE,0x1DE,0,0,0x1DE,0,
    0x1DF,0x1DF,0,0,0x1DF,0,  0x1E0,0x1E0,0,0,0x1E0,0,
    0x1E1,0x1E1,0,0,0x1E1,0,  0x1E2,0x1E2,0,0,0x1E2,0,
    0x1E3,0x1E3,0,0,0x1E3,0,  0x1E4,0x1E4,0,0,0x1E4,0,
    0x1E5,0x1E5,0,0,0x1E5,0,  0x1E6,0x1E6,0,0,0x1E6,0,
    0x1E7,0x1E7,0,0,0x1E7,0,  0x1E8,0x1E8,0,0,0x1E8,0,
    0x1E9,0x1E9,0,0,0x1E9,0,  0x1EA,0x1EA,0,0,0x1EA,0,
    0x1EB,0x1EB,0,0,0x1EB,0,  0x1EC,0x1EC,0,0,0x1EC,0,
    0x1ED,0x1ED,0,0,0x1ED,0,  0x1EE,0x1EE,0,0,0x1EE,0,
    0x1EF,0x1EF,0,0,0x1EF,0,  0x1F0,0x1F0,0,0,0x1F0,0,
    0x1F1,0x1F1,0,0,0x1F1,0,  0x1F2,0x1F2,0,0,0x1F2,0,
    0x1F3,0x1F3,0,0,0x1F3,0,  0x1F4,0x1F4,0,0,0x1F4,0,
    0x1F5,0x1F5,0,0,0x1F5,0,  0x1F6,0x1F6,0,0,0x1F6,0,
    0x1F7,0x1F7,0,0,0x1F7,0,  0x1F8,0x1F8,0,0,0x1F8,0,
    0x1F9,0x1F9,0,0,0x1F9,0,  0x1FA,0x1FA,0,0,0x1FA,0,
    0x1FB,0x1FB,0,0,0x1FB,0,  0x1FC,0x1FC,0,0,0x1FC,0,
    0x1FD,0x1FD,0,0,0x1FD,0,  0x1FE,0x1FE,0,0,0x1FE,0,
    0x1FF,0x1FF,0,0,0x1FF,0,  0x200,0x200,0,0,0x200,0,
    0x201,0x201,0,0,0x201,0,  0x202,0x202,0,0,0x202,0,
};

_LC_collate_t _C_collate_object={
  /*
  ** Object header info
  */
  _LC_COLLATE,               /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  0,			     /* size    */

  /*
  ** Collation Methods
  */
  /* character collation methods */
  STRCOLL_C,
  STRXFRM_C,

  /* process code collation methods */
  WCSCOLL_C,
  WCSXFRM_C,

  /* filename matching methods */
  FNMATCH_C,

  /* regular expression methods */
  REGCOMP_STD,
  REGERROR_STD,
  REGEXEC_STD,
  REGFREE_STD,

  COLLATE_INIT,		     /* init method */
  0,			     /* void * data */

  /*
  ** Class Extension Data
  */
  
  0,			     /* co_nord   */
  { 0, 0, 0, 0, 0 },	     /* co_sort */

  0,                         /* co_wc_min */
  255,			     /* co_wc_max */
  
  257,			     /* co_col_min*/
  257+256,		     /* co_col_max*/

  _C_coltbl,		     /* co_coltbl */

  0,                         /* co_nsubs */
  0,                         /* co_subs  */

  0,                         /* co_special */

  &__C_locale_object,	     /* loc_rec */
  0,			     /* __meth_ptr */
  0,                         /* __data_ptr */
};

_LC_numeric_t _C_numeric_object={
    /* Object header info */
    _LC_NUMERIC,       /* type_id */
    _LC_MAGIC,	       /* magic   */
    _LC_VERSION,       /* version */
    0,		       /* size    */

    NL_NUMINFO,
  
    NUMERIC_INIT,      /* init method */
    0,		       /* void * data */

    /* Class Extension Data */
    ".",	       /* decimal_point */
    "",		       /* thousands_sep */
    0,		       /* grouping      */

    &__C_locale_object,/* loc_rec */
    0,		       /* __meth_ptr */
    0,                 /* __data_ptr */
};

_LC_monetary_t _C_monetary_object={
  /*
  ** Object header info
  */
  _LC_MONETARY,              /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  0,			     /* size    */

  /* Methods for monetary class */
  NL_MONINFO,
  STRFMON_STD,
  
  MONETARY_INIT,	     /* init method */
  0,			     /* void * data */

  /* Class Extension Data */
  "",			     /* int_curr_symbol */
  "",			     /* currency_symbol */
  "",			     /* mon_decimal_point */
  "",			     /* mon_thousands_sep */
  "",			     /* mon_grouping    */
  "",			     /* positive_sign   */
  "",			     /* negative_sign   */
  -1,			     /* int_frac_digits */
  -1,			     /* frac_digits     */
  -1,			     /* p_cs_precedes   */
  -1,			     /* p_sep_by_space  */
  -1,			     /* n_cs_precedes   */
  -1,			     /* n_sep_by_space  */
  -1,			     /* p_sign_posn     */
  -1,			     /* n_sign_posn     */
  "",			     /* debit_sign 	*/
  "",			     /* credit_sign    	*/
  "",			     /* left_parenthesis */
  "",			     /* right_parenthesis */

  &__C_locale_object,	     /* loc_rec */
  0,			     /* __meth_ptr */
  0,                         /* __data_ptr */
};

_LC_time_t _C_time_object={
  /*
  ** Object header info
  */
  _LC_TIME,                  /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  0,			     /* size    */
 
  NL_TIMINFO,
  
  STRFTIME_STD, 
  STRPTIME_STD,
  WCSFTIME_STD, 

  TIME_INIT,		     /* init method */
  0,			     /* void * data */

  /* Class Extension Data */
  "%m/%d/%y",		     /* d_fmt */
  "%H:%M:%S",		     /* t_fmt */
  "%a %b %e %H:%M:%S %Y",    /* d_t_fmt */
  "%I:%M:%S %p",             /* t_fmt_ampm */
  { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
  { "Sunday", "Monday", "Tuesday", "Wednesday", 
    "Thursday", "Friday", "Saturday" },
  { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
    "Oct", "Nov", "Dec" },
  { "January", "February", "March", "April", "May", "June", "July", 
    "August", "September", "October", "November", "December" },
  { "AM", "PM" },
  "",                        /* era */
  "",                        /* era_year */
  "",                        /* era_d_fmt */
  "",                        /* alt_digits */
  "",                        /* era_d_t_fmt */
  "",                        /* era_t_fmt */

  &__C_locale_object,	     /* loc_rec */
  0,			     /* __meth_ptr */
  0,                         /* __data_ptr */
};

_LC_resp_t _C_resp_object={
  /*
  ** Object header info
  */
  _LC_RESP,                  /* type_id */
  _LC_MAGIC,		     /* magic   */
  _LC_VERSION,		     /* version */
  0,			     /* size    */

  NL_RESPINFO,
  RPMATCH_C,
  
  RESP_INIT,		     /* init method */
  0,			     /* void * data */

  /*
  ** Class Extension Data 
  */
  "^[yY].*",		     /* yesexpr */
  "^[nN].*",		     /* noexpr */
  "yes:y:Y",		     /* yesstr */
  "no:n:N",		     /* nostr */

  &__C_locale_object,	     /* loc_rec */
  0,			     /* __meth_ptr */
  0,                         /* __data_ptr */
};

struct lconv __C_lconv={
  ".",				/* decimal_point     */
  "",				/* thousands_sep     */
   0,				/* grouping          */
  "",				/* int_curr_symbol   */
  "",				/* currency_symbol   */
  "",				/* mon_decimal_point */
  "",				/* mon_thousands_sep */
  "",				/* mon_grouping      */
  "",				/* positive_sign     */
  "", 				/* negative_sign     */
  -1,				/* int_frac_digits   */
  -1,				/* frac_digits       */
  -1,				/* p_cs_precedes     */
  -1,				/* p_sep_by_space    */
  -1,				/* n_cs_precedes     */
  -1,				/* n_sep_by_space    */
  -1,				/* p_sign_posn       */
  -1,				/* n_sign_posn       */
  "",				/* right parenthesis */
  ""				/* left parenthesis  */
};

static _LC_locale_objhdl_t * init(_LC_locale_objhdl_t);

_LC_locale_t __C_locale_object={

  _LC_LOCALE, _LC_MAGIC, _LC_VERSION, 0,

  NL_LANGINFO_STD,
  LOCALECONV_STD,
  
  LOCALE_INIT,			/* init method */
  0,				/* data pointer */

  /* info strings */
  "",			                           /* NOT USED */
  "%a %b %e %H:%M:%S %Y",			   /* d_t_fmt */
  "%m/%d/%y",					   /* d_fmt */
  "%H:%M:%S",					   /* t_fmt */
  "AM",						   /* AM_STR  */
  "PM",						   /* PM_STR  */
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", /* ABDAY_x */
  "Sunday", "Monday", "Tuesday", "Wednesday",     
  "Thursday", "Friday", "Saturday",		   /* DAY_x   */
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
  "Aug", "Sep", "Oct", "Nov", "Dec",               /* ABMON_x */
  "January", "February", "March", "April", "May",
  "June", "July", "August", "September", "October",
  "November", "December",			   /* MON_x   */
  ".",						   /* RADIXCHAR */
  "",						   /* THOUSEP */
  "yes:y:Y",					   /* YESSTR  */
  "no:n:N",					   /* NOSTR   */
  "",						   /* CRNCYSTR*/
  "ISO8859-1", 	                                   /* CODESET */
  "%b %d %Y",					   /* NLLDATE */
  "at:each:every:on:through:am:pm:zulu",	   /* NLTMISC */
						   /* NLTSTR */
  "now:yesterday:tomorrow:noon:midnight:next:weekdays:weekend:today",
  "minute:minutes:hour:hours:day:days:week:weeks:month:months:year:years:min:mins",
						   /* NLTUNITS */
  "19890108,Heisei:19261225,Showa:",               /* NLYEAR */
  "",						   /* T_FMT_AMPM */
  "",						   /* ERA */
  "",						   /* ERA_D_FMT */
  "",						   /* ERA_D_T_FMT */
  "",						   /* ERA_T_FMT */
  "",						   /* ALT_DIGITS */
  "^[yY]",					   /* YESEXPR */
  "^[nN]",					   /* NOEXPR */
  &__C_lconv,
  
  { &_C_charmap_object, 
    &__method_tbl,
    0,
  },
  { &_C_collate_object, 
    &__method_tbl,
    0,
  },
  { &_C_ctype_object, 
    &__method_tbl,
    0,
  },
  { &_C_monetary_object, 
    &__method_tbl,
    0,
  },
  { &_C_numeric_object, 
    &__method_tbl,
    0,
  },
  { &_C_resp_object, 
    &__method_tbl,
    0,
  },
  { &_C_time_object, 
    &__method_tbl,
    0,
  },

  &__C_locale_object,	     /* loc_rec */
  0,			     /* __meth_ptr */
  0,                         /* __data_ptr */
}; 

_LC_locale_objhdl_t    	_C_locale     = { 
						&__C_locale_object,
				     		&__method_tbl,
						0,
					};

_LC_charmap_objhdl_t   	__lc_charmap  = { 
						&_C_charmap_object,
					   	&__method_tbl,
						0,
					};

_LC_ctype_objhdl_t   	__lc_ctype    = {    	
						&_C_ctype_object,
					   	&__method_tbl,
						0,
					};

_LC_collate_objhdl_t   	__lc_collate  = {    	
						&_C_collate_object,
					   	&__method_tbl,
						0,
					};

_LC_numeric_objhdl_t   	__lc_numeric  = {    	
						&_C_numeric_object,
					   	&__method_tbl,
						0,
					};

_LC_monetary_objhdl_t   __lc_monetary = {    	
						&_C_monetary_object,
					   	&__method_tbl,
						0,
					};

_LC_time_objhdl_t   	__lc_time     = {    	
						&_C_time_object,
					   	&__method_tbl,
						0,
					};

_LC_resp_objhdl_t   	__lc_resp     = {    	
						&_C_resp_object,
					   	&__method_tbl,
						0,
					};

_LC_locale_objhdl_t   	__lc_locale   = {    	
						&__C_locale_object,
					   	&__method_tbl,
						0,
					};


/*
*  FUNCTION: collate_init
*
*  DESCRIPTION:
*  Initialization method for collate locale object.  
*/
void  __collate_init(_LC_locale_objhdl_t lp)
{

    _LC_charmap_t *cmap = __OBJ_DATA(lp)->lc_charmap.obj;

#if 0
    /* 
      reset 3.1 version of mb_cur_xxx to support 3.1 MB_CUR_xxx macros 
    */

    _locp->lc_chrtbl->mb_cur_max  = cmap->cm_mb_cur_max;
    _locp->lc_chrtbl->mb_cur_min  = cmap->cm_mb_cur_min;

#endif

}


/*
*  FUNCTION: ctype_init
*
*  DESCRIPTION:
*  Initialization method for ctype locale object. 
*/
void  __ctype_init(_LC_locale_objhdl_t lp)
{

    _LC_charmap_t *cmap = __OBJ_DATA(lp)->lc_charmap.obj;

#if 0
    /* 
      reset 3.1 version of mb_cur_max to support 3.1 MB_CUR_xxx macros 
    */
    _locp->lc_chrtbl->lc_caseconv = cmap->core.__data;
    _locp->lc_chrtbl->mb_cur_max  = cmap->cm_mb_cur_max;
    _locp->lc_chrtbl->mb_cur_min  = cmap->cm_mb_cur_min;

#endif
}


/*
*  FUNCTION: __monetary_init
*
*  DESCRIPTION:
*  Initialization method for monetary locale object.
*/
void  __monetary_init(_LC_locale_objhdl_t lp)
{

    _LC_monetary_t *mon;
    char           **nl_info;
    struct lconv   *nl_lconv;

    nl_info  = __OBJ_DATA(lp)->nl_info;
    nl_lconv = __OBJ_DATA(lp)->nl_lconv;
    mon      = __OBJ_DATA(lp)->lc_monetary.obj;

    /* set nl_langinfo() information */
   nl_info[CRNCYSTR] = mon->currency_symbol;

    /* setup localeconv() structure */
   nl_lconv->int_curr_symbol    = mon->int_curr_symbol;
   nl_lconv->currency_symbol    = mon->currency_symbol;
   nl_lconv->mon_decimal_point  = mon->mon_decimal_point;
   nl_lconv->mon_thousands_sep  = mon->mon_thousands_sep;
   nl_lconv->mon_grouping       = mon->mon_grouping;
   nl_lconv->positive_sign      = mon->positive_sign;
   nl_lconv->negative_sign      = mon->negative_sign;
   nl_lconv->int_frac_digits    = mon->int_frac_digits;
   nl_lconv->frac_digits        = mon->frac_digits;
   nl_lconv->p_cs_precedes      = mon->p_cs_precedes;
   nl_lconv->p_sep_by_space     = mon->p_sep_by_space;
   nl_lconv->n_cs_precedes      = mon->n_cs_precedes;
   nl_lconv->n_sep_by_space     = mon->n_sep_by_space;
   nl_lconv->p_sign_posn        = mon->p_sign_posn;
   nl_lconv->n_sign_posn        = mon->n_sign_posn;
   nl_lconv->left_parenthesis   = mon->left_parenthesis;
   nl_lconv->right_parenthesis  = mon->right_parenthesis;
}

/*
*  FUNCTION: __charmap_init
*
*  DESCRIPTION:
*  Locale charmap class object initialization method.
*/
void  __charmap_init(_LC_locale_objhdl_t lp)
{

    /* set nl_langinfo() information */
    __OBJ_DATA(lp)->nl_info[CODESET] = __OBJ_DATA(lp)->lc_charmap.obj->cm_csname;
}


/*
*  FUNCTION: __resp_init
*
*  DESCRIPTION:
*  Initialization method for the response class.
*/
void  __resp_init(_LC_locale_objhdl_t lp)
{

    _LC_resp_t *resp = __OBJ_DATA(lp)->lc_resp.obj;

    __OBJ_DATA(lp)->nl_info[YESEXPR] = resp->yesexpr;
    __OBJ_DATA(lp)->nl_info[NOEXPR] = resp->noexpr;
    __OBJ_DATA(lp)->nl_info[YESSTR] = resp->yesstr;
    __OBJ_DATA(lp)->nl_info[NOSTR]  = resp->nostr;
}


/*
*  FUNCTION: __numeric_init
*
*  DESCRIPTION:
*  Numeric category object initialization method.
*/
void __numeric_init(_LC_locale_objhdl_t lp)
{

    _LC_numeric_t *num;
    char          **nl_info;
    struct lconv  *nl_lconv;

    nl_info  = __OBJ_DATA(lp)->nl_info;
    nl_lconv = __OBJ_DATA(lp)->nl_lconv;
    num      = __OBJ_DATA(lp)->lc_numeric.obj;

    /* set nl_langinfo() information */
    nl_info[RADIXCHAR] = num->decimal_point;
    nl_info[THOUSEP]   = num->thousands_sep;

    /* setup localeconv() lconv structure */
    nl_lconv->decimal_point = num->decimal_point;
    nl_lconv->thousands_sep = num->thousands_sep;
    nl_lconv->grouping      = num->grouping;
}


/*
*  FUNCTION: __time_init
*
*  DESCRIPTION:
*  This is the initialization method for the time category
*/
void __time_init(_LC_locale_objhdl_t lp)
{

    _LC_time_t  *time;
    char        **nl_info;

    nl_info = __OBJ_DATA(lp)->nl_info;
    time    = __OBJ_DATA(lp)->lc_time.obj;

    /* set nl_langinfo() information */
    nl_info[D_FMT]    = time->d_fmt;
    nl_info[T_FMT]    = time->t_fmt;
    nl_info[D_T_FMT]  = time->d_t_fmt;
    nl_info[AM_STR]   = time->am_pm[0];
    nl_info[PM_STR]   = time->am_pm[1];

    nl_info[ERA]      = time->era;
    nl_info[T_FMT_AMPM]   = time->t_fmt_ampm;
    nl_info[ERA_D_FMT]    = time->era_d_fmt;
    nl_info[ERA_T_FMT]    = time->era_t_fmt;
    nl_info[ERA_D_T_FMT]  = time->era_d_t_fmt;
    nl_info[ALT_DIGITS]   = time->alt_digits;

    /* copy abbreviate day name pointers ABDAY_x */
    memcpy(&(nl_info[ABDAY_1]), &(time->abday[0]), 7 * sizeof(char *));

    /* copy day name pointers DAY_x */
    memcpy(&(nl_info[DAY_1]), &(time->day[0]), 7 * sizeof(char *));

    /* copy abbreviated month name pointers ABMON_x */
    memcpy(&(nl_info[ABMON_1]), &(time->abmon[0]), 12 * sizeof(char *));

    /* copy month name pointers MON_x */
    memcpy(&(nl_info[MON_1]), &(time->mon[0]), 12 * sizeof(char *));

}

/*
*  FUNCTION: __locale_init
*
*  DESCRIPTION:
*  Initialization method for the locale handle.
*/
void __locale_init(_LC_locale_objhdl_t lp)
{
  _CALLMETH(__OBJ_DATA(lp)->lc_charmap, __init)(lp);
  _CALLMETH(__OBJ_DATA(lp)->lc_collate, __init)(lp);
  _CALLMETH(__OBJ_DATA(lp)->lc_ctype, __init)(lp);
  _CALLMETH(__OBJ_DATA(lp)->lc_monetary, __init)(lp);
  _CALLMETH(__OBJ_DATA(lp)->lc_numeric, __init)(lp);
  _CALLMETH(__OBJ_DATA(lp)->lc_resp, __init)(lp);
  _CALLMETH(__OBJ_DATA(lp)->lc_time, __init)(lp);

}
