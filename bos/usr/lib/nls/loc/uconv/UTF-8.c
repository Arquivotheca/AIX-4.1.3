static char sccsid[] = "@(#)71  1.3  src/bos/usr/lib/nls/loc/uconv/UTF-8.c, cmdiconv, bos411, 9428A410j 4/21/94 02:16:39";
/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:		_iconv_from_ucs
 *			_iconv_to_ucs
 *			_uconv_from_ucs
 *			_uconv_to_ucs
 *			_iconv_close
 *			_iconv_init
 *			instantiate
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

#include <stdlib.h>
#include <errno.h>

#include "iconvP.h"
#include "UCSTBL.h"
#include "uc_conv.h"
#include "get_modifier.h"

/*
 *   NAME:	_b_swap
 *
 *   FUNCTION:	Swap high byte and low byte of UCS characters.
 *
 *   RETURNS:	None
 */

static	void		_b_swap (
	uchar_t		*target,	/* Target buffer           */
	uchar_t		*source,	/* Source buffer           */
	size_t		len) {		/* Length of source string */

	uchar_t		tmp;
	size_t		i;

	len -= (len & 1);

	if (target != NULL) {
		for (i = 0; i < len; i += 2) {
			target[i] = source[i+1];
			target[i+1] = source[i];
		}
	}
	else {	for (i = 0; i < len; i += 2) {
			tmp = source[i];
			source[i] = source[i+1];
			source[i+1] = tmp;
		}
	}
}

/*
 *   NAME:	_utf_table
 *
 *   DESCRIPTION:	Table for checking UTF-8 string.
 *
 *	The UTF-8 definition:
 *
 *	The UCS transformation format (UTF-8) encodes UCS values in the range
 *	<0 - 0xffff> using multibyte characters of length 1, 2,  and 3 bytes.
 *	Single byte characters are reserved for ASCII characters in the range
 *	<0 - 0x7f> that have the high order bit set to 0.   For all encodings
 *	of more than one byte, the number of bytes is indicated by the number
 *	of high order bits set to 1. Every byte of multibyte character starts
 *	with 0x10vvvvvv.
 *
 *	Bytes  Bits    Min.    Max.  Byte sequence in binary
 *	-----  ----  ------  ------  --------------------------
 *	    1     7  0x0000  0x007f  0vvvvvvv
 *	    2    11  0x0080  0x07ff  110vvvvv 10vvvvvv
 *	    3    16  0x0800  0xffff  1110vvvv 10vvvvvv 10vvvvvv
 */

typedef	struct	{
	uchar_t		utf_nbyte_mask;
	uchar_t		utf_nbyte_value;
	int		shift;
	UniChar		ucs_range_mask;
	UniChar		ucs_range_min;
} _UTF_table;

static	_UTF_table	_utf_table[] = {
	{ 0x80, 0x00, 0*6, 0x007f, 0x0000 },	/* 1 byte sequence */
	{ 0xe0, 0xc0, 1*6, 0x07ff, 0x0080 },	/* 2 byte sequence */
	{ 0xf0, 0xe0, 2*6, 0xffff, 0x0800 },	/* 3 byte sequence */
	{ 0x00, 0x00,   0, 0x0000, 0x0000 }	/* End of table    */
};

/*
 *   NAME:	_from_ucs
 *
 *   FUNCTION:	Convert from UCS to UTF-8.
 *
 *   RETURNS:	0	- Successfully completed.
 *			  (Number of non-identical conversion)
 *		-1	- Error.
 */

static	size_t		_from_ucs (
	UniChar		**ucs_buf,	/* UCS input buffer               */
	size_t		*ucs_left,	/* #of UniChar left in UCS buffer */
	uchar_t		**utf_buf,	/* UTF output buffer              */
	size_t		*utf_left) {	/* #of bytes   left in UTF buffer */

	UniChar		*ucs_p, *ucs_buf_end, ucs_char;
	uchar_t		*utf_p, *utf_buf_end, *utf_p_save;
	int		err_flag = FALSE, n_bytes;


	ucs_buf_end = (ucs_p = *ucs_buf) + *ucs_left;
	utf_buf_end = (utf_p = *utf_buf) + *utf_left;

	while (ucs_p < ucs_buf_end) {
		if (utf_p >= utf_buf_end) {
			err_flag = TRUE; errno = E2BIG;
			break;
		}
		if (*ucs_p < 0x80) {

			/*
			 *	No conversion for ASCII character.
			 */

			*utf_p = (uchar_t)*ucs_p;
			ucs_p ++;
			utf_p ++;
			continue;
		}
		/*
		 *	Get number of bytes of multibyte encoding.
		 */

		ucs_char = *ucs_p;

		for (n_bytes = 1; _utf_table[n_bytes].utf_nbyte_mask != 0;
		     n_bytes ++) {
			if (ucs_char <= _utf_table[n_bytes].ucs_range_mask)
				break;
		}
		utf_p_save = utf_p;

		*utf_p = (uchar_t)((ucs_char >> _utf_table[n_bytes].shift) & 0xff);
		*utf_p |= _utf_table[n_bytes].utf_nbyte_value;
		utf_p ++;

		for (n_bytes --; n_bytes >= 0; n_bytes --) {
			if (utf_p >= utf_buf_end) {
				err_flag = TRUE; errno = E2BIG;
				utf_p = utf_p_save;
				break;
			}
			*utf_p = (uchar_t)((ucs_char >> _utf_table[n_bytes].shift) &0x3f);
			*utf_p |= 0x80;
			utf_p ++;
		}
		if (err_flag) break;
		ucs_p ++;
	}
	*ucs_left = ucs_buf_end - ucs_p;
	*utf_left = utf_buf_end - utf_p;
	*ucs_buf  = ucs_p;
	*utf_buf  = utf_p;

	if (!err_flag)	return (size_t)0;
	else		return (size_t)-1;
}

/*
 *   NAME:	_to_ucs
 *
 *   FUNCTION:	Convert from UTF-8 to UCS-2.
 *
 *   RETURNS:	0	- Successfully completed.
 *			  (Number of non-identical conversion)
 *		-1	- Error.
 */

static	size_t		_to_ucs (
	uchar_t		**utf_buf,
	size_t		*utf_left,
	UniChar		**ucs_buf,
	size_t		*ucs_left) {

	uchar_t		*utf_p, *utf_buf_end, utf_char, *utf_p_save;
	UniChar		*ucs_p, *ucs_buf_end, ucs_char;
	int		n_bytes, i, err_flag = FALSE;

	
	utf_buf_end = (utf_p = *utf_buf) + *utf_left;
	ucs_buf_end = (ucs_p = *ucs_buf) + *ucs_left;

	while (utf_p < utf_buf_end) {
		if (*utf_p < 0x80) {

			/*
			 *	No conversion for ASCII characters.
			 */

			if (ucs_p >= ucs_buf_end) {
				err_flag = TRUE; errno = E2BIG;
				break;
			}
			*ucs_p = (UniChar)*utf_p;
			utf_p ++;
			ucs_p ++;
			continue;
		}
		/*
		 *	Get number of bytes of multibyte encoding.
		 */

		for (n_bytes = 1; _utf_table[n_bytes].utf_nbyte_mask != 0;
		     n_bytes ++) {
			if ((_utf_table[n_bytes].utf_nbyte_mask & *utf_p)
			  == _utf_table[n_bytes].utf_nbyte_value)
				break;
		}
		if (_utf_table[n_bytes].utf_nbyte_mask == 0) {
			err_flag = TRUE; errno= EILSEQ;
			break;
		}
		ucs_char = (UniChar)*utf_p;
		utf_p_save = utf_p;
		utf_p ++;

		for (i = 0; i < n_bytes; i ++) {
			if (utf_p >= utf_buf_end) {

				/*
				 *	UTF-8 byte sequence has not been concluded.
				 *	It should be thought as truncated character.
				 */

				err_flag = TRUE; errno = EINVAL;
				utf_p = utf_p_save;
				break;
			}
			utf_char = *utf_p ^ 0x80;
			if (utf_char & 0xc0) {

				/*
				 *	Invalid UTF-8 byte found.
				 */

				err_flag = TRUE; errno = EILSEQ;
				utf_p = utf_p_save;
				break;
			}
			ucs_char <<= 6;
			ucs_char |= (UniChar)utf_char;
			utf_p ++;
		}
		if (err_flag) break;

		ucs_char &= _utf_table[n_bytes].ucs_range_mask;
		if (ucs_char < _utf_table[n_bytes].ucs_range_min) {
			err_flag = TRUE; errno = EILSEQ;
			utf_p = utf_p_save;
			break;
		}
		if (ucs_p >= ucs_buf_end) {
			err_flag = TRUE; errno = E2BIG;
			break;
		}
		*ucs_p = ucs_char;
		ucs_p ++;
	}
	*utf_left = utf_buf_end - utf_p;
	*ucs_left = ucs_buf_end - ucs_p;
	*utf_buf  = utf_p;
	*ucs_buf  = ucs_p;

	if (!err_flag)	return (size_t)0;
	else		return (size_t)-1;
}

/*
 *   NAME:	_iconv_from_ucs
 *
 *   FUNCTION:	Conversion from UCS (char based interface).
 *
 *   RETURNS:	Number of non-identical conversion performed.
 */

static	size_t		_iconv_from_ucs (
	_LC_ucs_iconv_t	*cd,
	uchar_t		**in_buf,	/* UCS input buffer             */
	size_t		*in_left,	/* #of bytes left in UCS buffer */
	uchar_t		**out_buf,	/* UTF output buffer            */
	size_t		*out_left) {	/* #of bytes left in UTF buffer */

	uchar_t		*tmp_buf, *tmp_ptr;
	size_t		ucs_left, subs;


	if ((cd == -1) || (cd == NULL)) {
		errno = EBADF; return (size_t)-1;
	}
	if (in_buf == NULL) return (size_t)0;

	if (cd->endian.source == ENDIAN_LITTLE) {
		if ((tmp_buf = malloc (*in_left)) == NULL) {
			errno = ENOMEM;
			return (size_t)-1;
		}
		_b_swap (tmp_buf, *in_buf, *in_left);
	}
	else	tmp_buf = *in_buf;

	tmp_ptr   = tmp_buf;
	ucs_left  = *in_left / sizeof (UniChar);

	subs = _from_ucs ((UniChar**)(&tmp_ptr), &ucs_left, out_buf, out_left);

	if (tmp_buf != *in_buf) free (tmp_buf);

	*in_buf  += tmp_ptr - tmp_buf;
	*in_left -= tmp_ptr - tmp_buf;

	if (subs == -1) return subs;
	if (*in_left != 0) {		/* Unconverted byte left in the input */
		errno = EINVAL;		/* buffer.  It should be thought as a */
		return (size_t)-1;	/* truncated UCS character.           */       
	}
	else	return subs;
}

/*
 *   NAME:	_iconv_to_ucs
 *
 *   FUNCTION:	Conversion to UCS (char based interface).
 *
 *   RETURNS:	Number of non-identical conversions performed.
 */

static	size_t		_iconv_to_ucs (
	_LC_ucs_iconv_t	*cd,
	uchar_t		**in_buf,	/* UTF input buffer             */
	size_t		*in_left,	/* #of bytes left in UTF buffer */
	uchar_t		**out_buf,	/* UCS output buffer            */
	size_t		*out_left) {	/* #of bytes left in UCS buffer */

	uchar_t		*out_ptr;
	size_t		ucs_left, subs;
	int		bytes_mod;


	if ((cd == -1) || (cd == NULL)) {
		errno = EBADF; return (size_t)-1;
	}
	if (in_buf == NULL) return (size_t)0;

	out_ptr   = *out_buf;
	ucs_left  = *out_left / sizeof (UniChar);
	bytes_mod = *out_left % sizeof (UniChar);

	subs = _to_ucs (in_buf, in_left, (UniChar**)(&out_ptr), &ucs_left);

	if (cd->endian.target == ENDIAN_LITTLE)
		_b_swap (NULL, *out_buf, out_ptr - *out_buf);

	*out_buf  = out_ptr;
	*out_left = ucs_left * sizeof (UniChar) + bytes_mod;

	return subs;
}

/*
 *   NAME:	_uconv_from_ucs
 *
 *   FUNCTION:	Convert from UCS (UniChar based).
 *
 *   RETURNS:	Error status code.
 */

static	int		_uconv_from_ucs (
	_LC_ucs_iconv_t	*cd,
	UniChar		**ucs_buf,	/* UCS input buffer                */
	size_t		*ucs_left,	/* #of UniChars left in UCS buffer */
	void		**utf_buf,	/* UTF output buffer               */
	size_t		*utf_left,	/* #of bytes left in UTF buffer    */
	size_t		*subs) {	/* #of substitutions performed     */

	UniChar		*tmp_buf, *tmp_ptr;


	*subs = 0;

	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return UCONV_EBADF;
	}
	if (cd->endian.source == ENDIAN_LITTLE) {
		if ((tmp_buf = malloc (
			*ucs_left * sizeof (UniChar))) == NULL) {
			errno = ENOMEM;
			return (size_t)-1;
		}
		_b_swap ((uchar_t*)tmp_buf, (uchar_t*)*ucs_buf,
			*ucs_left * sizeof (UniChar));
	}
	else	tmp_buf = *ucs_buf;

	tmp_ptr = tmp_buf;

	*subs = _from_ucs (&tmp_ptr, ucs_left, utf_buf, utf_left);

	if (tmp_buf != *ucs_buf) free (tmp_buf);

	*ucs_buf += (tmp_ptr - tmp_buf) / sizeof (UniChar);

	if (*subs != (size_t)-1)
			return 0;
	else switch (errno) {
	case EBADF  :	return UCONV_EBADF;
	case ENOMEM :	return UCONV_ENOMEM;
	case E2BIG  :	return UCONV_E2BIG;
	case EILSEQ :	return UCONV_EILSEQ;
	case EINVAL :	return UCONV_EINVAL;
	default     :	return UCONV_EINVAL;
	}
}

/*
 *   NAME:	_uconv_to_uc
 *
 *   FUNCTION:	Conversion to UCS (UniChar based)
 *
 *   RETURNS:	Error status code.
 *
 *   ERROR CONDITIONS:
 *	E2BIG:		Output buffer overflow
 *	EINVAL:		Input UTF-8 sequence is truncated.
 *	EILSEQ:		Input UTF-8 byte is invalid.
 */

static	int		_uconv_to_ucs (
	_LC_ucs_iconv_t	*cd,
	void		**utf_buf,	/* UTF input buffer                */
	size_t		*utf_left,	/* #of bytes left in UTF buffer    */
	UniChar		**ucs_buf,	/* UCS output buffer               */
	size_t		*ucs_left,	/* #of UniChars left in UCS buffer */
	size_t		*subs) {	/* #of non-identical conversions   */

	uchar_t		*ucs_buf_top;


	*subs = 0;

	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
	ucs_buf_top = (uchar_t*)(*ucs_buf);

	*subs = _to_ucs (utf_buf, utf_left, ucs_buf, ucs_left);

	if (cd->endian.target == ENDIAN_LITTLE)
		_b_swap (NULL, ucs_buf_top, (uchar_t*)(*ucs_buf) - ucs_buf_top);

	if (*subs != (size_t)-1)
			return 0;
	else switch (errno) {
	case EBADF  : return UCONV_EBADF;
	case ENOMEM : return UCONV_ENOMEM;
	case E2BIG  : return UCONV_E2BIG;
	case EILSEQ : return UCONV_EILSEQ;
	case EINVAL : return UCONV_EINVAL;
	default     : return UCONV_EINVAL;
	}
}

/*
 *   NAME:	_iconv_close
 *
 *   FUNCTION:	Close UCS converter.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static	int		_iconv_close (
	_LC_ucs_iconv_t	*cd) {		/* UCS conversion descriptor */

	if ((cd != NULL) && (cd != -1)) {
		free (cd); return 0;
	}
	else {
		errno = EBADF; return -1;
	}
}

/*
 *   NAME:      _chk_endian
 *
 *   FUNCTION:  Check Syntax of 'endian' modifier, and extract parameters.
 *
 *   RETURNS:   0       - Successful completion.
 *              -1      - Error.
 */

static	_m_value_t	endian_list[] = {
	{ "big"		,ENDIAN_BIG    },
	{ "little"	,ENDIAN_LITTLE },
	{ "system"	,ENDIAN_SYSTEM },
};

static	int		_chk_endian (
	uchar_t		*modifier,      /* Modifier string   */
	endian_t	*endian) {      /* Returns parameter */

	int             n_names, source_len, i, j;
	uchar_t         *col_ptr, *target_ptr;


	n_names    = sizeof (endian_list) / sizeof (_m_value_t);
	col_ptr    = strchr (modifier, ':');
	source_len = (col_ptr == NULL) ? strlen (modifier) : col_ptr - modifier;
	target_ptr = col_ptr + 1;

	endian->source = endian->target = ENDIAN_SYSTEM;

	for (i = 0; i < n_names; i++) {
		if (strncmp (endian_list[i].name, modifier, source_len) != 0)
			continue;

		if (col_ptr == NULL) {
			endian->source = endian->target = endian_list[i].value;
			return 0;
		}
		for (j = 0; j < n_names; j ++) {
			if (strcmp (endian_list[j].name, target_ptr) != 0)
				continue;
			endian->source = endian_list[i].value;
			endian->target = endian_list[j].value;
			return 0;
		}
		break;
	}
	return -1;
}

/*
 *   NAME:	_iconv_init
 *
 *   FUNCTION:	Open UCS converter.
 *
 *   RETURNS:	This function returns pointer to a conversion descriptor to
 *		the variable pointed by 'cd' argument as described in XPG4.
 *		If failed, (_LC_ucs_iconv_t*)-1 is returned.
 *
 *   NOTE:	Codeset name of the 'unicode' or 'ISO10646' is not passed as
 *		a string, but is indicated by empty string.
 */

static	_LC_ucs_iconv_t		*_iconv_init (
	_LC_core_iconv_t	*core,
	uchar_t			*t_name,
	uchar_t			*f_name) {

	uchar_t			*modifier;
	int			t_len, f_len, ret;
	_LC_ucs_iconv_t		*cd;

	/*
	 *	Supported modifier names & parser routine addresses table.
	 */

	endian_t		endian = {ENDIAN_SYSTEM, ENDIAN_SYSTEM};
	_m_entry_t		m_array[] = {
		{"endian"	,_chk_endian	,&endian ,NULL ,0},
		{NULL		,NULL		,NULL    ,NULL ,0}
	};
	f_len = strlen (f_name);
	if ((modifier = (uchar_t*)strchr (t_name, '@')) == NULL) {
		modifier = "\0";
		t_len = strlen (t_name);
	} else	t_len = modifier - t_name;

	if ((f_len == 0) && (t_len == 0)) {
		errno = EINVAL;
		return (_LC_ucs_iconv_t*)-1;
	}

	/*
	 *	Get parameter from the modifier.
	 */

	if ((ret = __get_modifier (modifier, m_array)) < 0) {
		errno = EINVAL;
		return (_LC_ucs_iconv_t*)-1;
	}

	/*
	 *	Allocate conversion descriptor.
	 */

	if ((cd = malloc (sizeof (_LC_ucs_iconv_t))) == NULL) {
		errno = ENOMEM;
		return (_LC_ucs_iconv_t*)-1;
	}
	cd->object.core              = *core;
	cd->object.uconv_from_ucs    = (int(*)())_uconv_from_ucs;
	cd->object.uconv_to_ucs      = (int(*)())_uconv_to_ucs;
	if (f_len == 0)
		cd->object.core.exec = (size_t(*)())_iconv_from_ucs;
	else	cd->object.core.exec = (size_t(*)())_iconv_to_ucs;
	cd->endian                   = endian;

	return cd;
}

/*
 *   NAME:	instantiate
 *
 *   FUNCTION:	Instantiation method of this converter.
 *
 *   RETURNS:	Pointer to the descriptor.
 */

_LC_core_iconv_t	*instantiate (void) {

	static	_LC_core_iconv_t	core;

	core.hdr.__magic   = _LC_MAGIC;
	core.hdr.__version = _LC_VERSION | _LC_ICONV_MODIFIER;
	core.hdr.__type_id = _LC_ICONV;
	core.hdr.__size    = sizeof (_LC_core_iconv_t);
	core.init          = (_LC_core_iconv_t*(*)())_iconv_init;
	core.close         = (int(*)())_iconv_close;
	core.exec          = (size_t(*)())NULL;

	return &core;
}
