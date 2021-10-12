/* @(#)02       1.3  src/bos/usr/include/uc_convP.h, libiconv, bos411, 9428A410j 4/5/94 08:35:16
 *
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		Definitions for ULS method converters
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __UC_CONVP
#define __UC_CONVP

#include <sys/types.h>
#include <sys/limits.h>

/* UCS conversion table version & release -----------------------------------*/

#define	UC_VERSION			0x01
#define	UC_RELEASE			0x00

/* UCS conversion table copyright statement ---------------------------------*/

#define	COPYRIGHT_STATEMENT		\
"(C) COPYRIGHT International Business Machines Corp. 1993"

/* UCS conversion table types 'uconv_class' ---------------------------------*/

#define UC_CLASS_SBCS			1	/* UCS <-> SBCS              */
#define UC_CLASS_DBCS			2	/* UCS <-> DBCS  (stateless) */
#define UC_CLASS_MBCS			3	/* UCS <-> MBCS  (stateless) */
#define UC_CLASS_EBCDIC_STATEFUL	4	/* UCS <-> EBCDIC (stateful) */
#define UC_CLASS_INVAL			0	/* Invalid type              */

/* UCS conversion table default substitution character ----------------------*/

#define	UC_DEF_SUB			0x001a
#define	STEM_MAX			29

/* UCS conversion table header structures -----------------------------------*/

typedef struct {
	ulong_t		size;			/* = size of table           */
	ushort_t	bom;			/* = 0xfeff (big endian)     */
	uchar_t		version;		/* = UC_VERSION              */
	uchar_t		release;		/* = UC_RELEASE              */
	uchar_t		copyright[70];		/* Copyright statement       */
	ushort_t	uconv_class;		/* UCONV CLASS               */
	UniChar		sub_uni;		/* Substitution char in UCS  */
	uchar_t		subchar[STEM_MAX+3];	/* Substitution char in MBCS */
	uchar_t		cs_name[PATH_MAX+1];	/* Code set name             */
	uchar_t		locale [PATH_MAX+1];	/* Locale name               */
} _ucmap_com_t;

typedef struct {			/* Header for SBCS type              */
	_ucmap_com_t	com;		/* Common part                       */
	ushort_t	U2Mof4set[256];	/* Offsets to conversion rows        */
	UniChar		M2Utable[256];	/* UCS conversion table              */
	UniChar		undef_uni;	/* Equivalent char of undef_char     */
	uchar_t		undef_char;	/* Marker for invalid char in MBCS   */
	uchar_t		filler;		/*                                   */
} _ucmap_sbcs_t;

typedef struct {			/* Header for DBCS type              */
	_ucmap_com_t	com;		/* Common part                       */
	ushort_t	U2Mof4set[256];	/* Offsets to conversion rows        */
	ushort_t	M2Uof4set[256];	/* Offsets to conversion rows        */
	ushort_t	code_len[256];	/* Code length array                 */
} _ucmap_dbcs_t;

typedef struct {			/* Header for MBCS type              */
	_ucmap_com_t	com;		/* Common part                       */
	ushort_t	U2Mof4set[256];	/* Offsets to conversion rows        */
	ushort_t	M2Uof4set[256];	/* Offsets to EUC rows               */
	ushort_t	code_len[256];	/* Code length array                 */
} _ucmap_mbcs_t;

typedef struct {			/* Header for stateful Ebcdic        */
	_ucmap_com_t	com;		/* Common part                       */
	ushort_t	U2Mof4set[256];	/* Offsets to conversion rows        */
	ushort_t	M2Uof4set[256];	/* Offsets to conversion rows        */
} _ucmap_ebcdic_stateful_t;

typedef union {
	_ucmap_com_t			com;
	_ucmap_sbcs_t			sbcs;
	_ucmap_dbcs_t			dbcs;
	_ucmap_mbcs_t			mbcs;
	_ucmap_ebcdic_stateful_t	ebcdic_stateful;
} _ucmap_hdr_t;

/* MBCS conversion table ----------------------------------------------------*/

typedef struct {			/* MBCS conversion row table unit    */
	ushort_t	n_slots;	/* Number of slots of this table     */
	ushort_t	l_value;	/* Lowest byte value in row          */
	ushort_t	nextOf4set[256];/* Offsets to next rows              */
} _uc_row_t;

typedef struct {			/* MBCS STEM information             */
	uchar_t		stem[STEM_MAX+1];/*Stem string                       */
	ushort_t	stem_len;	/* Stem length                       */
} _uc_stem_t;

typedef	struct {			/* UCS_MBCS conversion table unit    */
	ushort_t	stem_index;	/* Index for stem table              */
	ushort_t	code;		/* The last 2 byte of codepoint      */
} _uc_u2m_t;

/* Conversion table control block -------------------------------------------*/

typedef	struct _uc_table_rec	_uc_table_t;
struct	_uc_table_rec {
	_uc_table_t	*next;		/* Next table                        */
	_uc_table_t	*anchor;	/* Anchor of the tables              */
	uchar_t		*name;		/* File/Path name of the table       */
	_ucmap_hdr_t	*table;		/* Conversion table                  */
	int		used_count;	/* Used counter                      */
};

/* Conversion handle --------------------------------------------------------*/

#define	SHIFT_IN	0
#define SHIFT_OUT	1

typedef struct _uc_ch_rec {
	_uc_table_t	*uc_table;		/* Conversion table C/B      */
	ushort_t	uconv_class;		/* Type of conversion        */
	int		state_flag;		/* State flag                */
	int		map;			/* Map option                */
	int		sub;			/* Substitution option       */
	UniChar		sub_uni;		/* Substitution char in UCS  */
	uchar_t		subchar[STEM_MAX+1];	/* Substitution char in MBCS */
	int		subchar_len;		/* Substitution char length  */
} _uc_ch_t;

/* Internal use function prototypes -----------------------------------------*/

extern	int		getTableName (	/* Get table File/Path name          */
	uchar_t		*cs_name,	/* MBCS name                         */
	uchar_t		*table_name);	/* Returns table File/Path name      */

extern	int		getUcTable (	/* Get conversion table              */
	uchar_t		*table_name,	/* Table File/Path name              */
	_uc_table_t	**uc_table);	/* Returns conversion table C/B      */

extern	int		freeUcTable (	/* Free conversion table             */
	_uc_table_t	*uc_table);	/* Conversion table C/B              */

extern	size_t		resetState (	/* Reset state of converter          */
	_uc_ch_t	*ch,		/* Conversion handle                 */
	uchar_t		**out_buf,	/* Output buffer                     */
	size_t		*out_left,	/* Bytes left in output buffer       */
	int		to_ebc);	/* Conversion to EBCDIC              */

#endif /*!__UC_CONVP*/
