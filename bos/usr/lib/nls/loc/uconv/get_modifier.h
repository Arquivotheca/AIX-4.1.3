/* @(#)10       1.2  src/bos/usr/lib/nls/loc/uconv/get_modifier.h, cmdiconv, bos41J, 9509A_all 2/19/95 23:27:15
 *
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS: none
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

#ifndef _ICONV_GET_MODIFIER_H
#define _ICONV_GET_MODIFIER_H

#include <sys/types.h>
#include <uc_conv.h>

/*
 *	Modifier name & check function table unit.
 */

typedef struct {
	uchar_t		*name;
	int		(*p_func)();
	void		*p_arg;
	uchar_t		*value;	
	size_t		length;
} _m_entry_t;

/*
 *	Modifier name & value table unit.
 */

typedef	struct _mod_value_rec {
	uchar_t		*name;
	int		value;
} _m_value_t;


/*
 * 	Endian definition
 */
typedef struct {
        uchar_t         source;
        uchar_t         target;
} _m_endian_t;

#ifndef ENDIAN_SYSTEM
#define ENDIAN_SYSTEM   0
#define ENDIAN_BIG      0xfeff
#define ENDIAN_LITTLE   0xfffe
#endif /* ENDIAN_SYSTEM */

/*
 *	Subchar definition
 */

typedef struct {
        UniChar         ucs;   		        /* Subchar in UCS            */
	uchar_t		val[STEM_MAX+4];        /* Subchar in multibyte      */
	int		len;                    /* multibyte length          */
} _m_subchar_t;

/*
 *	Common Modifiers Supported by all UCS conversions
 */

typedef struct _mod_core_rec {
        uchar_t         map;                    /* Map option                */
        uchar_t         sub;                    /* Substitution option       */
        _m_endian_t     swap_endian;		/* UCS Endian modifier       */
	_m_subchar_t	subchar;                /* Substitution char         */
	_m_value_t	extension;		/* Others                    */
} _m_core_t;

/*
 *	Function prototype.
 */

extern	int	__get_modifier (const uchar_t*, _m_entry_t*);
extern  uchar_t* __iconv_parse_modifier( const uchar_t* to,
					const uchar_t* from,
					_m_core_t* mod,
					uchar_t* to_name_new);

#endif /*_ICONV_GET_MODIFIER_H*/
