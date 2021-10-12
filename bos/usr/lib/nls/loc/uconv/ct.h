/* @(#)09       1.2  src/bos/usr/lib/nls/loc/uconv/ct.h, ils-zh_CN, bos41J, 9514A_all 3/28/95 15:12:38
 *
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		none
 *
 *   ORIGINS:		27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef ctUCS_H
#define ctUCS_H

#include "get_modifier.h"

#define MAX_N_CDS 45
#define DEF_UCS_NAME "UCS-2"
#define UCONV_ctUCS "iso2022.cfg"
#define UCONV_METHOD_PATH   "/uconv/"

typedef size_t  (*ctUCS_Func)();

typedef struct  _ctCharsetDescRec {
	struct  _ctCharsetDescRec* next;/* Chain of ct converters */ 
	iconv_t	conv;			/* iconv_t handle */
	ctUCS_Func func;		/* function instead of iconv */
	EscTbl	escape;			/* Escape Seq. Info */
	uchar_t gl;			/* True = GL, False = GR */
	uchar_t free;			/* True = static, False = malloc */
} ctCharsetDescRec, *ctCharsetDesc;

typedef struct		_LC_ucs_ct_iconv_rec {
	struct _UconvObject	object;
	_m_core_t	modifier;		/* Modifier info	     */
	ctCharsetDesc	top;			/* Charset DB                */
	ctCharsetDesc	current;		/* current Charset           */
	EscTblTbl*	ett;			/* Handle to Master Charsets */
						/*   ct, fold7 or fold8      */
	ctCharsetDesc	gl, gr;			/* GL/GR Charsets 	     */
	uchar_t*        cfg_table;		/* String DB - parsed...     */
	uchar_t* 	isctl;
	ctCharsetDescRec asciiCharsetRec;	/* ASCII Charset for top...  */
} _LC_ucs_ct_iconv_t;
						/* Note: top is treated as   */
						/*       default GR          */

#ifndef ENDIAN_SYSTEM
#define ENDIAN_SYSTEM   0
#define ENDIAN_BIG      0xfeff
#define ENDIAN_LITTLE   0xfffe
#endif /* ENDIAN_SYSTEM */


#define EscSeqLen(c) ((c)->escape.len + ((c)->escape.seg ? \
						(c)->escape.seglen + 2 : 0 ))


extern ctCharsetDescRec utfInvalidCharset;  /* dummy to identify */
extern ctCharsetDescRec utfCharset;

extern _LC_ucs_ct_iconv_t* _ctUCS_init( 
				_LC_core_iconv_t* core, 
				uchar_t* t_name, 
				uchar_t* f_name,
				EscTblTbl* ett);
extern int _ctUCS_close(
				_LC_ucs_ct_iconv_t* cd);
extern void _ctUCS_b_swap (
        			uchar_t*  target,
        			uchar_t*  source,
        			size_t    len);
extern int _ctUCS_reset_state (
				_LC_ucs_ct_iconv_t*,
				uchar_t** outbuf,
				size_t*   outbytesleft);
extern EscTbl* _iconv_FindWellKnownCharset(
                                _LC_ucs_ct_iconv_t* cd,
                                uchar_t* name,
                                size_t len);
extern EscTbl* _iconv_FindWellKnownEscape( 
				_LC_ucs_ct_iconv_t *cd,
				EscTblTbl** ett,
				const uchar_t* esc, size_t len);



#endif /*ctUCS_H*/

