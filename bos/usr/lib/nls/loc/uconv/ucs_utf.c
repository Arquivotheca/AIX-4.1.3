/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:		__iconv_ucs2utf
 *			__iconv_utf2ucs
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
 *   NAME:	__iconv_ucs2utf
 *
 *   FUNCTION:	Convert from UCS to UTF-8.
 *
 *   RETURNS:	0	- Successfully completed.
 *			  (Number of non-identical conversion)
 *		-1	- Error.
 */

	size_t		__iconv_ucs2utf(
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
 *   NAME:	__iconv_utf2ucs
 *
 *   FUNCTION:	Convert from UTF-8 to UCS-2.
 *
 *   RETURNS:	0	- Successfully completed.
 *			  (Number of non-identical conversion)
 *		-1	- Error.
 */

	size_t		__iconv_utf2ucs (
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

