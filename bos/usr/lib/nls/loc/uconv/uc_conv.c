static char sccsid[] = "@(#)12  1.7  src/bos/usr/lib/nls/loc/uconv/uc_conv.c, cmdiconv, bos41J, 9520A_all 5/9/95 12:47:31";
/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:		UCCINIT
 *			UCCTERM
 *			UCCM2U
 *			UCCU2M
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>

#include "iconvP.h"
#include "uc_conv.h"

/*
 *   NAME:	_U2M_sbcs
 *
 *   FUNCTION:	Conversion from UCS (SBCS type).
 *
 *   RETURN:	Error status code.
 */

static	int		_U2M_sbcs (
	_uc_ch_t	*ch,		/* Conversion handle                */
	UniChar		*in_buf,	/* Input buffer                     */
	size_t		*in_size,	/* #of UniChar of input / processed */
	uchar_t		*out_buf,	/* Output buffer                    */
	size_t		*out_size,	/* #of bytes of buffer / output     */
	size_t		*subs) {	/* #of non-identical conversions    */

	UniChar		*in_ptr,  *in_buf_end,  undef_uni;
	uchar_t		*out_ptr, *out_buf_end, undef_char, *p_char;
	_ucmap_sbcs_t	*ucmap;
	int		ret = UC_NO_ERRORS;


	in_buf_end  = (in_ptr  = in_buf ) + *in_size;
	out_buf_end = (out_ptr = out_buf) + *out_size;
	ucmap       = &(ch->uc_table->table->sbcs);
	undef_uni   = ucmap->undef_uni;
	undef_char  = ucmap->undef_char;
	*subs       = 0;

	while (in_ptr < in_buf_end) {

		p_char = (uchar_t*)ucmap
		       + (unsigned)(ucmap->U2Mof4set[(*in_ptr >> 8) & 0xff]) * 4
		       + (unsigned)(*in_ptr & 0xff);

		if ((*p_char == undef_char) &&
		    (*in_ptr != undef_uni)) {
			if ((ch->sub == NO_SUBSTITUTION) ||
			    (ch->sub == SUBSTITUTE_TO_UNICODE)) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			(*subs) ++;
			p_char = &(ch->subchar[0]);
		}
		if (out_ptr >= out_buf_end) {
			ret = UC_BUFFER_FULL; errno = E2BIG;
			break;
		}
		*out_ptr = *p_char;
		out_ptr ++;
		in_ptr  ++;
	}
	*in_size  = (size_t)(in_ptr  - in_buf);
	*out_size = (size_t)(out_ptr - out_buf);
	return ret;
}

/*
 *   NAME:	_U2M_dbcs
 *
 *   FUNCTION:	Conversion from UCS (DBCS type).
 *
 *   RETURNS:	Error status code.
 */

static	int		_U2M_dbcs (
	_uc_ch_t	*ch,		/* Conversion handle                */
	UniChar		*in_buf,	/* Input buffer                     */
	size_t		*in_size,	/* #of UniChar of input / processed */
	uchar_t		*out_buf,	/* Output buffer                    */
	size_t		*out_size,	/* #of bytes of buffer / output     */
	size_t		*subs) {	/* #of non-identical conversions    */

	UniChar		*in_ptr,  *in_buf_end;
	uchar_t		*out_ptr, *out_buf_end, *p_char;
	ushort_t	code;
	_ucmap_dbcs_t	*ucmap;
	int		ret = UC_NO_ERRORS;


	in_buf_end  = (in_ptr  = in_buf ) + *in_size;
	out_buf_end = (out_ptr = out_buf) + *out_size;
	ucmap = &(ch->uc_table->table->dbcs);
	*subs = 0;

	while (in_ptr < in_buf_end) {

		if ((ch->map == MAP_IRV) && (*in_ptr <= 0x7f)) {
			if (out_ptr >= out_buf_end) {
				ret = UC_BUFFER_FULL; errno = E2BIG;
				break;
			}
			*out_ptr = (uchar_t)(*in_ptr);
			out_ptr ++;
			in_ptr  ++;
			continue;
		}
		p_char = (uchar_t*)ucmap
		       + (unsigned)(ucmap->U2Mof4set[(*in_ptr >> 8) & 0xff]) * 4
		       + (unsigned)(*in_ptr & 0xff) * sizeof(ushort_t);

		code = (ushort_t)(p_char[0]<<8) + (ushort_t)p_char[1];

		if (code == 0xffff) {
			if ((ch->sub == NO_SUBSTITUTION) ||
			    (ch->sub == SUBSTITUTE_TO_UNICODE)) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			if (ch->subchar_len == 1)
				code = (ushort_t)(ch->subchar[0]);
			else	code = (ushort_t)(ch->subchar[0]<<8)
				     + (ushort_t)(ch->subchar[1]);
			p_char = (uchar_t*)&code;
			(*subs) ++;
		}
		if (code < 0x100) {

			/*
			 *	Single byte code.
			 */

			if (out_ptr >= out_buf_end) {
				ret = UC_BUFFER_FULL; errno = E2BIG;
				break;
			}
			out_ptr[0] = p_char[1];
			out_ptr ++;
		}
		else {	/*
			 *	Double byte code.
			 */

			if ((out_ptr + 2) > out_buf_end) {
				ret = UC_BUFFER_FULL; errno = E2BIG;
				break;
			}
			out_ptr[0] = p_char[0];
			out_ptr[1] = p_char[1];
			out_ptr += 2;
		}
		in_ptr ++;
	}
	*in_size  = (size_t)(in_ptr  - in_buf);
	*out_size = (size_t)(out_ptr - out_buf);
	return ret;
}

/*
 *   NAME:	_U2M_mbcs
 *
 *   FUNCTION:	Conversion from UCS (MBCS type).
 *
 *   RETURNS:	Error status code.
 *
 *   NOTE:	MBCS conversion table structure.
 *
 *	+--------------------------------+ -+
 *	| _ucmap_mbcs_t | _ucmap_com_t   |  |
 *	|               |----------------|  |
 *	|               | U2Mof4set[256] |  |
 *	|               |----------------|  | Header part
 *	|               | M2Uof4set[256] |  |
 *	|               |----------------|  |
 *	|               | code_len[256]  |  |
 *	|--------------------------------| -+
 *	| _uc_stem_t[g_nPlanes]          |    STEM info table
 *	|--------------------------------|
 *	| _uc_row_t * N                  |    Conversion ROWs
 *	|--------------------------------|
 *	| _uc_u2m_t * N                  |    Conversion table U2M
 *	+--------------------------------+
 */

static	int		_U2M_mbcs (
	_uc_ch_t	*ch,		/* Conversion handle                */
	UniChar		*in_buf,	/* Input buffer                     */
	size_t		*in_size,	/* #of UniChar of input / processed */
	uchar_t		*out_buf,	/* Output buffer                    */
	size_t		*out_size,	/* #of bytes of buffer / output     */
	size_t		*subs) {	/* #of non-identical conversions    */

	UniChar		*in_ptr,  *in_buf_end;
	uchar_t		*out_ptr, *out_buf_end, charbuf[STEM_MAX+3];
	ushort_t	stem_index, code;
	_ucmap_mbcs_t	*ucmap;
	_uc_stem_t	*stem;
	_uc_u2m_t	*u2m;
	int		ret = UC_NO_ERRORS, len;


	in_buf_end  = (in_ptr  = in_buf ) + *in_size;
	out_buf_end = (out_ptr = out_buf) + *out_size;
	ucmap = &(ch->uc_table->table->mbcs);
	stem  = (_uc_stem_t*)((char*)ucmap + sizeof (_ucmap_mbcs_t));
	*subs = 0;

	while (in_ptr < in_buf_end) {

		if ((ch->map == MAP_IRV) && (*in_ptr <= 0x7f)) {
			if (out_ptr >= out_buf_end) {
				ret = UC_BUFFER_FULL; errno = E2BIG;
				break;
			}
			*out_ptr = (uchar_t)(*in_ptr);
			out_ptr ++;
			in_ptr  ++;
			continue;
		}
		memset (charbuf, 0, sizeof (charbuf));

		u2m = (_uc_u2m_t*)((char*)ucmap
		    + (unsigned)(ucmap->U2Mof4set[(*in_ptr >> 8) & 0xff]) * 4
		    + (unsigned)(*in_ptr & 0xff) * sizeof (_uc_u2m_t));
		stem_index = u2m->stem_index;
		code       = u2m->code;

		if (code < 0x100) {
			charbuf[0] = (uchar_t)code;
			len = 1;
		}
		else if (stem_index == 0xffff) {
			if ((ch->sub == NO_SUBSTITUTION) ||
			    (ch->sub == SUBSTITUTE_TO_UNICODE)) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			len = ch->subchar_len;
			memcpy (charbuf, ch->subchar, len);
			(*subs) ++;
		}
		else if ((len = stem[stem_index].stem_len) == 0) {
			charbuf[0] = (uchar_t)((code >> 8) & 0xff);
			charbuf[1] = (uchar_t)(code & 0xff);
			len = 2;
		}
		else {	memcpy (charbuf, stem[stem_index].stem, len);
			charbuf[len  ] = (uchar_t)((code >> 8) & 0xff);
			charbuf[len+1] = (uchar_t)(code & 0xff);
			len += 2;
		}
		if ((out_ptr + len) > out_buf_end) {
			ret = UC_BUFFER_FULL; errno = E2BIG;
			break;
		}
		memcpy (out_ptr, charbuf, len);
		in_ptr ++;
		out_ptr += len;
	}
	*in_size  = (size_t)(in_ptr  - in_buf);
	*out_size = (size_t)(out_ptr - out_buf);
	return ret;
}

/*
 *   NAME:	_U2M_ebcdic_stateful
 *
 *   FUNCTION:	From UCS conversion (EBCDIC stateful type).
 *
 *   RETURNS:	Error status code.
 */

static	int		_U2M_ebcdic_stateful (
	_uc_ch_t	*ch,		/* Conversion handle                */
	UniChar		*in_buf,	/* Input buffer                     */
	size_t		*in_size,	/* #of UniChar of input / processed */
	uchar_t		*out_buf,	/* Output buffer                    */
	size_t		*out_size,	/* #of bytes of buffer / output     */
	size_t		*subs) {	/* #of non-identical conversions    */

	UniChar				*in_ptr,  *in_buf_end;
	uchar_t				*out_ptr, *out_buf_end, *p_char;
	ushort_t			code;
	_ucmap_ebcdic_stateful_t	*ucmap;
	int				shift_state, ret = UC_NO_ERRORS;


	in_buf_end  = (in_ptr  = in_buf ) + *in_size;
	out_buf_end = (out_ptr = out_buf) + *out_size;
	shift_state = ch->state_flag;
	ucmap = &(ch->uc_table->table->ebcdic_stateful);
	*subs = 0;

	while (in_ptr < in_buf_end) {

		p_char = (uchar_t*)ucmap
		       + (unsigned)(ucmap->U2Mof4set[(*in_ptr >> 8) & 0xff]) * 4
		       + (unsigned)(*in_ptr & 0xff) * sizeof (ushort_t);

		code = (ushort_t)(p_char[0]<<8) + (ushort_t)p_char[1];

		if (code == 0xffff) {
			if ((ch->sub == NO_SUBSTITUTION) ||
			    (ch->sub == SUBSTITUTE_TO_UNICODE)) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			if (ch->subchar_len == 1)
				code = (ushort_t)(ch->subchar[0]);
			else	code = (ushort_t)(ch->subchar[0]<<8)
				     + (ushort_t)(ch->subchar[1]);
			p_char = (uchar_t*)&code;
			(*subs) ++;
		}
		if (shift_state == SHIFT_IN) {

			if (code < 0x100) {
				if (out_ptr == out_buf_end) {
					ret = UC_BUFFER_FULL; errno = E2BIG;
					break;
				}
				out_ptr[0] = p_char[1];
				out_ptr ++;
			}
			else {
				if ((out_ptr + 3) > out_buf_end) {
					ret = UC_BUFFER_FULL; errno = E2BIG;
					break;
				}
				shift_state = SHIFT_OUT;
				out_ptr[0] = SO;
				out_ptr[1] = p_char[0];
				out_ptr[2] = p_char[1];
				out_ptr += 3;
			}
		}
		else {
			if (code < 0x100) {

				if ((out_ptr + 2) > out_buf_end) {
					ret = UC_BUFFER_FULL; errno = E2BIG;
					break;
				}
				shift_state = SHIFT_IN;
				out_ptr[0] = SI;
				out_ptr[1] = p_char[1];
				out_ptr += 2;
			}
			else {
				if ((out_ptr + 2) > out_buf_end) {
					ret = UC_BUFFER_FULL; errno = E2BIG;
					break;
				}
				out_ptr[0] = p_char[0];
				out_ptr[1] = p_char[1];
				out_ptr += 2;
			}
		}
		in_ptr ++;
	}
	ch->state_flag = shift_state;		/* Save shift state */
	*in_size  = (size_t)(in_ptr  - in_buf);
	*out_size = (size_t)(out_ptr - out_buf);
	return ret;
}

/*
 *   NAME:	_M2U_sbcs
 *
 *   FUNCTION:	Conversion to UCS (SBCS type).
 *
 *   RETURNS:	Error status code.
 */

static	int		_M2U_sbcs (
	_uc_ch_t	*ch,		/* Conversion handle              */
	uchar_t		*in_buf,	/* Input buffer                   */
	size_t		*in_size,	/* #of bytes of input / processed */
	UniChar		*out_buf,	/* Output buffer                  */
	size_t		*out_size,	/* #of UniChar of buffer / output */
	size_t		*subs) {	/* #of non-identical conversions  */

	uchar_t		*in_ptr, *in_buf_end;
	UniChar		*out_ptr, unichar, *table;
	int		ret;


	in_ptr  = in_buf;
	out_ptr = out_buf;
	table   = ch->uc_table->table->sbcs.M2Utable;
	*subs   = 0;

	if (*in_size <= *out_size) {
		in_buf_end = in_buf + *in_size;
		ret = UC_NO_ERRORS;
	}
	else {	in_buf_end = in_buf + *out_size;

		if ((table[*in_buf_end] == 0xffff) &&
		    ((ch->sub == NO_SUBSTITUTION) ||
		     (ch->sub == SUBSTITUTE_FROM_UNICODE))) {
			ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
		}
		else {
			ret = UC_BUFFER_FULL; errno = E2BIG;
		}
	}
	while (in_ptr < in_buf_end) {
		if ((unichar = table[*in_ptr]) == 0xffff) {
			if ((ch->sub == NO_SUBSTITUTION) ||
			    (ch->sub == SUBSTITUTE_FROM_UNICODE)) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			(*subs) ++;
			*out_ptr = ch->sub_uni;
		} else	*out_ptr = unichar;
		in_ptr ++;
		out_ptr ++;
	}
	*in_size  = (size_t)(in_ptr  - in_buf);
	*out_size = (size_t)(out_ptr - out_buf);
	return ret;
}

/*
 *   NAME:	_M2U_dbcs
 *
 *   FUNCTION:	Conversion to UCS (DBCS type).
 *
 *   RETURNS:	Error status code.
 */

static	int		_M2U_dbcs (
	_uc_ch_t	*ch,		/* Conversion handle              */
	uchar_t		*in_buf,	/* Input buffer                   */
	size_t		*in_size,	/* #of bytes of input / processed */
	UniChar		*out_buf,	/* Output buffer                  */
	size_t		*out_size,	/* #of UniChar of buffer / output */
	size_t		*subs) {	/* #of non-identical conversions  */

	uchar_t		*in_ptr,  *in_buf_end;
	UniChar		*out_ptr, *out_buf_end, *p_ucs;
	_ucmap_dbcs_t	*ucmap;
	int		ret = UC_NO_ERRORS, code_len;


	in_buf_end  = (in_ptr  = in_buf ) + *in_size;
	out_buf_end = (out_ptr = out_buf) + *out_size;
	ucmap = &(ch->uc_table->table->dbcs);
	*subs = 0;

	while (in_ptr < in_buf_end) {

		if (ucmap->code_len[in_ptr[0]] == 1) {

			if ((ch->map == MAP_IRV) && (*in_ptr <= 0x7f)) {
				if (out_ptr >= out_buf_end) {
					ret = UC_BUFFER_FULL; errno = E2BIG;
					break;
				}
				*out_ptr = (UniChar)(*in_ptr);
				out_ptr ++;
				in_ptr  ++;
				continue;
			}
			p_ucs  = (UniChar*)((char*)ucmap
			       + (unsigned)(ucmap->M2Uof4set[0]) * 4
			       + (unsigned)in_ptr[0] * sizeof (UniChar));
			code_len = 1;
		}
		else {	/* Double byte code */

			if ((in_ptr + 1) == in_buf_end) {
			        if (*in_ptr < 0x20) {
				    ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				}
				else {
				    ret = UC_INPUT_CHAR_TRUNCATED; errno = EINVAL;
				}
				break;
			}
			p_ucs  = (UniChar*)((char*)ucmap
			       + (unsigned)(ucmap->M2Uof4set[in_ptr[0]]) * 4
			       + (unsigned)in_ptr[1] * sizeof (UniChar));
			code_len = 2;
		}
		if (*p_ucs == 0xffff) {	/* unconvertable */
			if ((ch->sub == NO_SUBSTITUTION) ||
			    (ch->sub == SUBSTITUTE_FROM_UNICODE)) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			(*subs) ++;
			p_ucs = &(ch->sub_uni);
		}
		if (out_ptr >= out_buf_end) {
			ret = UC_BUFFER_FULL; errno = E2BIG;
			break;
		}
		*out_ptr = *p_ucs;
		out_ptr ++;
		in_ptr += code_len;
	}
	*in_size  = (size_t)(in_ptr  - in_buf);
	*out_size = (size_t)(out_ptr - out_buf);
	return ret;
}

/*
 *   NAME:	_M2U_mbcs
 *
 *   FUNCTION:	Conversion to UCS (MBCS type).
 *
 *   RETURN:	Error status code.
 */

static	int		_M2U_mbcs (
	_uc_ch_t	*ch,		/* Conversion handle              */
	uchar_t		*in_buf,	/* Input buffer                   */
	size_t		*in_size,	/* #of bytes of input / processed */
	UniChar		*out_buf,	/* Output buffer                  */
	size_t		*out_size,	/* #of UniChar of buffer / output */
	size_t		*subs) {	/* #of non-identical conversions  */

	uchar_t		*in_ptr,  *in_buf_end, in_byte;
	UniChar		*out_ptr, *out_buf_end;
	ushort_t	of4set,	code_len, wk_len;
	_uc_row_t	*row;
	_ucmap_mbcs_t	*ucmap;
	int		ret = UC_NO_ERRORS, i;


	in_buf_end  = (in_ptr  = in_buf ) + *in_size;
	out_buf_end = (out_ptr = out_buf) + *out_size;
	ucmap = &(ch->uc_table->table->mbcs);
	*subs = 0;

	while (in_ptr < in_buf_end) {

		code_len = ucmap->code_len[in_ptr[0]];

		if (code_len == 0xffff) {
			ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
			break;
		}
		if ((in_buf_end - in_ptr) < code_len) {
			ret = UC_INPUT_CHAR_TRUNCATED; errno = EINVAL;
			break;
		}
		if (code_len == 1) {
			if ((ch->map == MAP_IRV) && (*in_ptr <= 0x7f)) {
				if (out_ptr >= out_buf_end) {
					ret = UC_BUFFER_FULL; errno = E2BIG;
					break;
				}
				*out_ptr = (UniChar)(*in_ptr);
				out_ptr ++;
				in_ptr  ++;
				continue;
			}
			else	of4set = ucmap->M2Uof4set[in_ptr[0]];
		}
		else {	of4set = ucmap->M2Uof4set[in_ptr[0]];

			for (i = 1; i < code_len; i ++) {
				row = (_uc_row_t*)((char*)ucmap + (unsigned)(of4set)*4);
				in_byte = in_ptr[i];
				if (in_byte < row->l_value) {
					of4set = 0xffff;
					break;
				}
				in_byte -= row->l_value;
				if (in_byte >= row->n_slots) {
					of4set = 0xffff;
					break;
				}
				of4set = row->nextOf4set[in_byte];
			}
		}
		if (of4set == 0xffff) {	/* unconvertable */
			if ((ch->sub == NO_SUBSTITUTION) ||
			    (ch->sub == SUBSTITUTE_FROM_UNICODE)) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			(*subs) ++;
			of4set = ch->sub_uni;
		}
		if (out_ptr >= out_buf_end) {
			ret = UC_BUFFER_FULL; errno = E2BIG;
			break;
		}
		*out_ptr = of4set;
		out_ptr ++;
		in_ptr += code_len;
	}
	*in_size  = (size_t)(in_ptr  - in_buf);
	*out_size = (size_t)(out_ptr - out_buf);
	return ret;
}

/*
 *   NAME:	_M2U_ebcdic_stateful
 *
 *   FUNCTION:	Conversion to UCS (EBCDIC stateful type).
 *
 *   RETURNS:	Error status code.
 */

static	int		_M2U_ebcdic_stateful (
	_uc_ch_t	*ch,		/* Conversion handle              */
	uchar_t		*in_buf,	/* Input buffer                   */
	size_t		*in_size,	/* #of bytes of input / processed */
	UniChar		*out_buf,	/* Output buffer                  */
	size_t		*out_size,	/* #of UniChar of buffer / output */
	size_t		*subs) {	/* #of non-identical conversions  */

	uchar_t				*in_ptr,  *in_buf_end;
	UniChar				*out_ptr, *out_buf_end, *p_ucs;
	_ucmap_ebcdic_stateful_t	*ucmap;
	int				ret = UC_NO_ERRORS, code_len,
					shift_state;


	in_buf_end  = (in_ptr  = in_buf ) + *in_size;
	out_buf_end = (out_ptr = out_buf) + *out_size;
	shift_state =  ch->state_flag;
	ucmap = &(ch->uc_table->table->ebcdic_stateful);
	*subs = 0;

	while (in_ptr < in_buf_end) {
		if (shift_state == SHIFT_IN) {
			if (in_ptr[0] == SI) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			if (in_ptr[0] == SO) {
				shift_state = SHIFT_OUT;
				++in_ptr;
				continue;
			}
			p_ucs  = (UniChar*)((char*)ucmap
			       + (unsigned)(ucmap->M2Uof4set[0]) * 4
			       + (unsigned)in_ptr[0] * sizeof (UniChar));
			code_len = 1;
		}
		else if (in_ptr[0] == SO) {
			ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
			break;
		}
		else if (in_ptr[0] == SI) {
			shift_state = SHIFT_IN;
			++in_ptr;
			continue;
		}
		else if ((in_ptr + 1) >= in_buf_end) {
			ret = UC_INPUT_CHAR_TRUNCATED; errno = EINVAL;
			break;
		}
		else {	p_ucs  = (UniChar*)((char*)ucmap
			       + (unsigned)(ucmap->M2Uof4set[in_ptr[0]]) * 4
			       + (unsigned)in_ptr[1] * sizeof (UniChar));
			code_len = 2;
		}
		if (*p_ucs == 0xffff) {
			if ((ch->sub == NO_SUBSTITUTION) ||
			    (ch->sub == SUBSTITUTE_FROM_UNICODE)) {
				ret = UC_INVALID_CHAR_FOUND; errno = EILSEQ;
				break;
			}
			(*subs) ++;
			p_ucs = &(ch->sub_uni);
		}
		if (out_ptr >= out_buf_end) {
			ret = UC_BUFFER_FULL; errno = E2BIG;
			break;
		}
		*out_ptr = *p_ucs;
		out_ptr ++;
		in_ptr += code_len;
	}
	ch->state_flag = shift_state;
	*in_size  = (size_t)(in_ptr  - in_buf);
	*out_size = (size_t)(out_ptr - out_buf);
	return ret;
}

/*
 *   NAME:	UCCINIT
 *
 *   FUNCTION:	Initialize UCS conversion.
 *
 *   RETURNS:	This function returns pointer to the conversion handle
 *		to the variable pointed by 'conv_handle' argument.  If
 *		any error is occurred, it returns error status code.
 *
 *   ERROR STATUS CODE:
 *	UC_INVALID_SUBCHAR	- Invalid subchar specified.
 *	UC_INVALID_OPTION	- Invalid option specified.
 *	UC_INVALID_TABLE	- Conversion table has invalid format.
 *	UC_TABLE_NOT_AVAILABLE	- Conversion table does not exist.
 *	UC_NOT_ENOUGH_SPACE	- Shortage of storage.
 */

int	UCCINIT (
	uchar_t		*cs_name,		/* MBCS name                 */
	int		map,			/* Map option                */
	int		sub,			/* Substitution option       */
	UniChar		sub_uni,		/* Substitution char in UCS  */
	uchar_t		subchar[STEM_MAX+3],	/* Substitution char in MBCS */
	_uc_ch_t	**conv_handle) {	/* Returns conversion handle */

	uchar_t		table_name[PATH_MAX+1], charbuf[STEM_MAX+3];
	UniChar		unichar;
	size_t		in_size, out_size, subs;
	_uc_table_t	*uc_table;		/* Conversion table C/B      */
	_uc_ch_t	*ch;			/* Conversion handle         */
	int		subchar_len,
			ret = UC_NO_ERRORS;


	*conv_handle = NULL;

	/*
	 *	Get the conversion table file/path name.
	 */

	if (cs_name == NULL) {
		errno = EINVAL; return UC_OTHER_ERRORS;
	}
	if ((ret = getTableName (cs_name, table_name)) != UC_NO_ERRORS)
		return ret;

	/*
	 *	Get the conversion table.
	 */

	if ((ret = getUcTable (table_name, &uc_table)) != UC_NO_ERRORS)
		return ret;

	/*
	 *	Make a conversion handle.
	 */

	if ((ch = (_uc_ch_t*)malloc (sizeof (_uc_ch_t))) == NULL) {
		freeUcTable (uc_table);
		errno = ENOMEM; return UC_NOT_ENOUGH_SPACE;
	}
	ch->uc_table    = uc_table;		/* Conversion table */
	ch->state_flag  = SHIFT_IN;		/* Reset state flag */
	ch->uconv_class = uc_table->table->com.uconv_class;

	/*
	 *	Check specified MBCS substitution character.
	 */

	if ((subchar_len = strlen (subchar)) > 0) {

		switch (uc_table->table->com.uconv_class) {
		case UC_CLASS_SBCS:
			if (subchar_len > 1) ret = UC_INVALID_SUBCHAR;
			break;
		case UC_CLASS_EBCDIC_STATEFUL:
		case UC_CLASS_DBCS:
			if (subchar_len > 2) ret = UC_INVALID_SUBCHAR;
			break;
		case UC_CLASS_MBCS:
			if (subchar_len > (STEM_MAX+2)) ret = UC_INVALID_SUBCHAR;
			break;
		default:ret = UC_INVALID_TABLE;
		}
	}
	else if (uc_table->table->com.sub_uni == (UniChar) 0xffff ) {
		/*
		 * If uni_sub in table == 0xffff, then default is no
		 * substitute.  Code point 0xffff is an invalid UCS
		 * coded character.
		 */
		subchar_len = 0;
		subchar = uc_table->table->com.subchar;
		sub = NO_SUBSTITUTION;
	}
	else if ((subchar_len = strlen (uc_table->table->com.subchar)) > 0) {

		/*
		 *	Use substitution character defined in the table.
		 */

		if (subchar_len > (STEM_MAX + 2))
			ret = UC_INVALID_TABLE;
		else	subchar = uc_table->table->com.subchar;
	}
	else {	/*
		 *	Use default substitution character. (Equivalent
		 *	character of UCS default substitution character)
		 */

		ch->sub  = NO_SUBSTITUTION;
		in_size  = 1;
		out_size = sizeof (charbuf);
		unichar  = UC_DEF_SUB;
		memset (charbuf, 0, sizeof (charbuf));
		ret = UCCU2M (ch, &unichar, &in_size, charbuf, &out_size, &subs);
		if (ret == UC_NO_ERRORS) {
			subchar = charbuf;
			subchar_len = out_size;
		}
		else	ret = UC_OTHER_ERRORS;
	}
	if (ret == UC_NO_ERRORS) {
		memcpy (ch->subchar, subchar, subchar_len);
		ch->subchar_len = subchar_len;
	}
	else if ((sub == SUBSTITUTE_FROM_UNICODE) ||
		 (sub == SUBSTITUTE_BOTH_WAYS)) {

		freeUcTable (uc_table);
		free (ch);
		errno = EINVAL;
		return ret;
	}
	else {
		ch->subchar[0] = '\0';
		ch->subchar_len = 0;
	}
	if ( map == MAP_IRV ) {
		switch (uc_table->table->com.uconv_class) {
		case UC_CLASS_DBCS: {
		    _ucmap_dbcs_t	*ucmap = &(uc_table->table->dbcs);
		    int i ;
		    for ( i = 0 ; i < 256 && (ucmap->code_len[i] != 1) ; ++i );
		    if ( i == 256 ) map = MAP_NONE;
		    break;
		}
		case UC_CLASS_MBCS: {
		    _ucmap_mbcs_t	*ucmap = &(uc_table->table->mbcs);
		    int i;
		    for ( i = 0 ; i < 256 && (ucmap->code_len[i] != 1) ; ++i );
		    if ( i == 256 ) map = MAP_NONE;
		    break;
		}
		}
	    }
	ch->map      = map;
	ch->sub      = sub;
	ch->sub_uni  = sub_uni;
	*conv_handle = ch;
	return UC_NO_ERRORS;
}

/*
 *   NAME:	UCCTERM
 *
 *   FUNCTION:	Terminate UCS conversion.
 *
 *   RETURNS:	Error status code.
 *
 *   ERROR STATUS CODE:
 *	UC_INVALID_HANDLE	- Invalid conversion handle.
 */

int	UCCTERM (
	_uc_ch_t	*ch) {

	int		ret;

	if ((ch == NULL) ||
	    (ch->uc_table == NULL) ||
	    (ch->uc_table->table == NULL) ||
	    (ch->uc_table->used_count < 1)) {
		errno = EBADF; return UC_INVALID_HANDLE;
	}
	ret = freeUcTable (ch->uc_table);
	free (ch);
	return ret;
}

/*
 *   NAME:	UCCU2M
 *
 *   FUNCTION:	Conversion from UCS to MBCS.
 *
 *   RETURNS:	Error status code.
 *
 *   ERROR STATUS CODE:
 *	UC_INVALID_HANDLE
 *		Invalid conversion handle is specified.
 *	UC_BUFFER_FULL
 *		No room in the output buffer to place the converted data.
 *	UC_INVALID_CHAR_FOUND
 *		Input stream has a character that can not be converted.  If the
 *		conversion handle specifies option of either NO_SUBSTITUTION or
 *		SUBSTITUTE_FROM_UNICODE,  the conversion will stop at the first
 *		byte of the invalid character.   If the conversion is performed
 *		with either SUBSTITUTE_BOTH_WAYS or SUBSTITUTE_TO_UNICODE,  all
 *		invalid characters are substituted.
 *	UC_INPUT_CHAR_TRUNCATED:
 *		The 'in_size' has been exhausted in the middle of conversion.
 *		The value returned in 'in_size' indicates the number of bytes
 *		missing to complete current character conversion.
 *	UC_NOT_ENOUGH_SPACE:
 *		No memory to allocate temporary buffer.
 */

int	UCCU2M (
	_uc_ch_t	*ch,
	UniChar		*in_buf,
	size_t		*in_size,
	uchar_t		*out_buf,
	size_t		*out_size,
	size_t		*subs) {

	if ((ch == NULL) || (in_buf == NULL) || (out_buf == NULL)) {
		errno = EBADF;
		return UC_INVALID_HANDLE;
	}
	switch (ch->uconv_class) {
	case UC_CLASS_SBCS:
		return _U2M_sbcs (
			ch, in_buf, in_size, out_buf, out_size, subs);
	case UC_CLASS_DBCS:
		return _U2M_dbcs (
			ch, in_buf, in_size, out_buf, out_size, subs);
	case UC_CLASS_MBCS:
		return _U2M_mbcs (
			ch, in_buf, in_size, out_buf, out_size, subs);
	case UC_CLASS_EBCDIC_STATEFUL:
		return _U2M_ebcdic_stateful (
			ch, in_buf, in_size, out_buf, out_size, subs);
	default:return UC_INVALID_HANDLE;
	}
}

/*
 *   NAME:	UCCM2U
 *
 *   FUNCTION:	Conversion from MBCS to UCS.
 *
 *   RETURNS:	This function updates the variables pointed by the arguments
 *		to reflect the result of the conversion. The value pointed to
 *		by 'in_size' is set number of bytes of processed input string.
 *		The value pointed to by 'out_size' is set number of UniChars
 *		of ouput characters.
 *		If any error is occurred, it returns error status code.
 *
 *   ERROR STATUS CODE:
 *	UC_INVALID_HANDLE
 *		Invalid conversion handle is specified.
 *	UC_BUFFER_FULL
 *		No room in the output buffer to place converted data.
 *	UC_INVALID_CHAR_FOUND:
 *		Input stream has a character that can not be converted.  If the
 *		conversion handle specifies option of either NO_SUBSTITUTION or
 *		SUBSTITUTE_FROM_UNICODE,  the conversion will stop at the first
 *		byte of the invalid character.   If the conversion is performed
 *		with either SUBSTITUTE_BOTH_WAYS or SUBSTITUTE_TO_UNICODE,  all
 *		invalid characters are substituted.
 *	UC_INPUT_CHAR_TRUNCATED:
 *		The 'in_size' has been exhausted in the middle of conversion.
 *		The value returned in 'in_size' indicates the number of bytes
 *		missing to complete current character conversion.
 *	UC_NOT_ENOUGH_SPACE:
 *		No memory to allocate temporary buffer.
 */

int	UCCM2U (
	_uc_ch_t*	ch,
	uchar_t		*in_buf,
	size_t		*in_size,
	UniChar		*out_buf,
	size_t		*out_size,
	size_t		*subs) {

	if ((ch == NULL) || (in_buf == NULL) || (out_buf == NULL)) {
		errno = EBADF;
		return UC_INVALID_HANDLE;
	}
	switch (ch->uconv_class) {
	case UC_CLASS_SBCS:
		return _M2U_sbcs (
			ch, in_buf, in_size, out_buf, out_size, subs);
	case UC_CLASS_DBCS:
		return _M2U_dbcs (
			ch, in_buf, in_size, out_buf, out_size, subs);
	case UC_CLASS_MBCS:
		return _M2U_mbcs (
			ch, in_buf, in_size, out_buf, out_size, subs);
	case UC_CLASS_EBCDIC_STATEFUL:
		return _M2U_ebcdic_stateful (
			ch, in_buf, in_size, out_buf, out_size, subs);
	default:return UC_INVALID_HANDLE;
	}
}
