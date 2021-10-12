static char sccsid[] = "@(#)06  1.4  UCSTBL.c, cmdiconv, bos41B 4/5/94 08:31:54";
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
#include <stdio.h>
#include <uconv.h>
#include <sys/errno.h>
#include <sys/types.h>

#include "iconvP.h"
#include "UCSTBL.h"
#include "get_modifier.h"
#include "uc_conv.h"

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

	len -= len & 1;

	if (target != NULL) {
		for (i = 0; i < len; i += 2) {
			target[i] = source[i+1];
			target[i+1] = source[i];
		}
	}
	else {
		for (i = 0; i < len; i += 2) {
			tmp = source[i];
			source[i] = source[i+1];
			source[i+1] = tmp;
		}
	}
}

/*
 *   NAME:	_iconv_from_ucs
 *
 *   FUNCTION:	Conversion from UCS.
 *
 *   RETURNS:	>= 0	- Number of non-identical conversion performed.
 *		-1	- Error.
 */

static	size_t		_iconv_from_ucs (
	_LC_ucs_iconv_t	*cd,		/* UCS conversion descriptor     */
	uchar_t		**in_buf,	/* UCS input buffer              */
	size_t		*in_left,	/* #of bytes left in UCS buffer  */
	uchar_t		**out_buf,	/* MBCS output buffer            */
	size_t		*out_left) {	/* #of bytes left in MBCS buffer */

	UniChar		*ucs_buf;
	size_t		ucs_size, out_size,
			bytes_mod, bytes_processed, subs;
	int		ret;

	if ((cd == -1) || (cd == NULL)) {
		errno = EBADF;
		return (size_t)-1;
	}

	/*
	 *	Reset state if 'in_buf' is set NULL.
	 */

	if (in_buf == NULL)
		return resetState (cd->object.ch, out_buf, out_left, TRUE);

	/*
	 *	Swap high & low bytes of UCS characters when the endian is
	 *	required LITTLE.
	 *
	 *	CAUSION: The following statements are hardware dependent.
	 */

	ucs_size  = *in_left / sizeof (UniChar);
	bytes_mod = *in_left % sizeof (UniChar);
	out_size  = *out_left;

	if (cd->endian.source == ENDIAN_LITTLE) {
		if ((ucs_buf = (UniChar*)malloc (*in_left)) == NULL) {
			errno = ENOMEM;
			return (size_t)-1;
		}
		_b_swap ((uchar_t*)ucs_buf, *in_buf, *in_left - bytes_mod);
	}
	else	ucs_buf = (UniChar*)*in_buf;

	ret = UCCU2M (
		cd->object.ch,		/* Conversion handle                */
		ucs_buf,		/* UCS input buffer                 */
		&ucs_size,		/* #of UniChars input / processed   */
		*out_buf,		/* MBCS output buffer               */
		&out_size,		/* #of bytes of buff size / output  */
		&subs);			/* #of non-identical conversions    */

	if (ucs_buf != (UniChar*)*in_buf) free (ucs_buf);

	bytes_processed = ucs_size * sizeof(UniChar);

	(*in_buf)   += bytes_processed;
	(*in_left)  -= bytes_processed;
	(*out_buf)  += out_size;
	(*out_left) -= out_size;

	if (ret != UC_NO_ERRORS) return (size_t)-1;

	if (bytes_mod) {
		errno = EINVAL;		/* This should be thought that      */
		return (size_t)-1;	/* truncated char is at the buff end*/
	}
	return subs;
}

/*
 *   NAME:	_iconv_to_ucs
 *
 *   FUNCTION:	Conversion to UCS.
 *
 *   RETURNS:	>= 0	- Number of non-identical conversion performed.
 *		-1	- Error.
 */

static	size_t		_iconv_to_ucs (
	_LC_ucs_iconv_t	*cd,		/* UCS conversion descriptor     */
	uchar_t		**in_buf,	/* MBCS input buffer             */
	size_t		*in_left,	/* #of bytes left in MBCS buffer */
	uchar_t		**out_buf,	/* UCS output buffer             */
	size_t		*out_left) {	/* #of bytes left in UCS buffer  */

	size_t		in_size, ucs_size, bytes_output, subs;
	int		ret;

	if ((cd == -1) || (cd == NULL)) {
		errno = EBADF;
		return -1;
	}

	/*
	 *	Reset state if 'in_buf' argument is set NULL.
	 */

	if (in_buf == NULL)
		return resetState (cd->object.ch, out_buf, out_left, FALSE);

	in_size   = *in_left;
	ucs_size  = *out_left / sizeof (UniChar);

	ret = UCCM2U (
		cd->object.ch,		/* Conversion handle               */
		*in_buf,		/* MBCS input buffer               */
		&in_size,		/* #of bytes of input / processed  */
		(UniChar*)(*out_buf),	/* UCS output buffer               */
		&ucs_size,		/* #of UniChars of buffer / output */
		&subs);			/* #of non-identical conversions   */

	/*
	 *	Swap high & low bytes of UCS characters when the endian is
	 *	required LITTLE.
	 *
	 *	CAUSION: The following statements are hardware dependent.
	 */

	bytes_output = ucs_size * sizeof (UniChar);

	if (cd->endian.target == ENDIAN_LITTLE)
		_b_swap (NULL, *out_buf, bytes_output);

	(*in_buf)   += in_size;
	(*in_left)  -= in_size;
	(*out_buf)  += bytes_output;
	(*out_left) -= bytes_output;

	if (ret == UC_NO_ERRORS)
		return subs;
	else	return (size_t)-1;
}

/*
 *   NAME:	_uconv_from_ucs
 *
 *   FUNCTION:	Conversion from UCS (for UniChar based API).
 *
 *   RETURNS:	Error status code.
 */

static	int		_uconv_from_ucs (
	_LC_ucs_iconv_t	*cd,		/* UCS conversion descriptor       */
	UniChar		**ucs_buf,	/* UCS string input buffer         */
	size_t		*ucs_left,	/* #of UniChars left in UCS buffer */
	void		**out_buf,	/* MBCS string output buffer       */
	size_t		*out_left,	/* #of bytes left in MBCS buffer   */
	size_t		*subs) {	/* #of non-identical conversions   */

	UniChar		*ucs_swap_buf;
	size_t		ucs_size, out_size;
	uchar_t		*out_ptr;
	int		ret;

	if ((cd == -1) || (cd == NULL)) {
		errno = EBADF;
		return UCONV_EBADF;
	}

	/*
	 *	Swap high & low bytes of UCS characters when the endian is
	 *	required LITTLE.
	 *
	 *	CAUSION: The following statements are hardware dependent.
	 */

	if (cd->endian.source == ENDIAN_LITTLE) {
		if ((ucs_swap_buf = (UniChar*)malloc (
			(*ucs_left) * sizeof (UniChar))) == NULL) {
			errno = ENOMEM;
			return UCONV_ENOMEM;
		}
		_b_swap ((uchar_t*)ucs_swap_buf, (uchar_t*)*ucs_buf,
			 (*ucs_left) * sizeof (UniChar));
	}
	else	ucs_swap_buf = *ucs_buf;

	ucs_size = *ucs_left;
	out_size = *out_left;
	out_ptr  = (uchar_t*)(*out_buf);

	ret = UCCU2M (
		cd->object.ch,		/* Conversion handle              */
		ucs_swap_buf,		/* UCS input buffer               */
		&ucs_size,		/* #of UniChars input / processed */
		out_ptr,		/* MBCS output buffer             */
		&out_size,		/* #of bytes of buffer / output   */
		subs);			/* #of non-identical conversions  */

	if (ucs_swap_buf != (UniChar*)*ucs_buf) free (ucs_swap_buf);

	(*ucs_buf ) += ucs_size;
	(*ucs_left) -= ucs_size;
	( out_ptr ) += out_size;
	(*out_left) -= out_size;
	(*out_buf )  = (void*)out_ptr;

	switch (ret) {
	case UC_NO_ERRORS            : return 0;
	case UC_INVALID_HANDLE       : return UCONV_EBADF;
	case UC_NOT_ENOUGH_SPACE     : return UCONV_ENOMEM;
	case UC_BUFFER_FULL          : return UCONV_E2BIG;
	case UC_INVALID_CHAR_FOUND   : return UCONV_EILSEQ;
	case UC_INPUT_CHAR_TRUNCATED : return UCONV_EINVAL;
	case UC_OTHER_ERRORS         :
	default			     : return UCONV_EINVAL;
	}
}

/*
 *   NAME:	_uconv_to_ucs
 *
 *   FUNCTION:	Conversion to UCS (for UniChar based API).
 *
 *   RETURNS:	Error status code.
 */

static	int		_uconv_to_ucs (
	_LC_ucs_iconv_t	*cd,		/* UCS conversion descriptor       */
	void		**in_buf,	/* MBCS string input buffer        */
	size_t		*in_left,	/* #of bytes left in MBCS buffer   */
	UniChar		**ucs_buf,	/* UCS string output buffer        */
	size_t		*ucs_left,	/* #of UniChars left in UCS buffer */
	size_t		*subs) {	/* #of non-identical conversions   */

	int		in_size, ucs_size;
	uchar_t		*in_ptr;
	int		ret;

	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF;
		return UCONV_EBADF;
	}
	in_ptr   = (uchar_t*)*in_buf;
	in_size  = *in_left;
	ucs_size = *ucs_left;

	ret = UCCM2U (
		cd->object.ch,		/* Conversion handle               */
		in_ptr,			/* input buffer                    */
		&in_size,		/* #of bytes of input / processed  */
		*ucs_buf,		/* output buffer                   */
		&ucs_size,		/* #of UniChars of buffer / output */
		subs);			/* #of non-identical conversions   */

	/*
	 *	Swap high & low bytes of UCS characters when the endian is
	 *	required LITTLE.
	 *
	 *	CAUSION: The following statements are hardware dependent.
	 */

	if (cd->endian.target == ENDIAN_LITTLE)
		_b_swap (NULL, (uchar_t*)*ucs_buf, ucs_size * sizeof (UniChar));

	( in_ptr  ) += in_size;
	(*in_left ) -= in_size;
	(*ucs_buf ) += ucs_size;
	(*ucs_left) -= ucs_size;
	(*in_buf  )  = (void*)in_ptr;

	switch (ret) {
	case UC_NO_ERRORS            : return 0;
	case UC_INVALID_HANDLE       : return UCONV_EBADF;
	case UC_NOT_ENOUGH_SPACE     : return UCONV_ENOMEM;
	case UC_BUFFER_FULL          : return UCONV_E2BIG;
	case UC_INVALID_CHAR_FOUND   : return UCONV_EILSEQ;
	case UC_INPUT_CHAR_TRUNCATED : return UCONV_EINVAL;
	case UC_OTHER_ERRORS         :
	default			     : return UCONV_EINVAL;
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
	_LC_ucs_iconv_t	*cd) {		/* Conversion descriptor */

	int		ret;

	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF;
		return -1;
	}
	ret = UCCTERM (cd->object.ch);
	free (cd);
	if (ret == UC_NO_ERRORS)
		return 0;
	else	return -1;
}

/*
 *   NAME:	_chk_endian
 *
 *   FUNCTION:	Check Syntax of 'endian' modifier, and extract parameters.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static	_m_value_t	endian_list[] = {
	{ "big"		,ENDIAN_BIG    },
	{ "little"	,ENDIAN_LITTLE },
	{ "system"	,ENDIAN_SYSTEM }
};

static	int		_chk_endian (
	uchar_t		*modifier,	/* Modifier string   */
	endian_t	*endian) {	/* Returns parameter */

	int		n_names, source_len, i, j;
	uchar_t		*col_ptr, *target_ptr;


	n_names    = sizeof (endian_list) / sizeof (_m_value_t);
	col_ptr    = strchr (modifier, ':');
	if (col_ptr != NULL) {
		target_ptr = col_ptr + 1;
		source_len = col_ptr - modifier;
	} else	source_len = strlen (modifier);

	endian->source = endian->target = ENDIAN_SYSTEM;

	for (i = 0; i < n_names; i ++) {
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
 *   NAME:	_chk_map
 *
 *   FUNCTION:	Check syntax of "map" modifier, and extract parameter.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static	int	_chk_map (
	uchar_t		*modifier,	/* Modifier string   */
	int		*map) {		/* Returns parameter */

#define	M_CMP(val) (modifier[0] == val[0] && strcmp (modifier, val) == 0)

	if	(M_CMP ("irv"))		*map = MAP_IRV;
	else if (M_CMP ("none"))	*map = MAP_NONE;
	else	 			return -1;
	return 0;

#undef	M_CMP
}

/*
 *   NAME:	_chk_sub
 *
 *   FUNCTION:	Check syntax of 'sub' modifier, and extract parameter.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static	int		_chk_sub (
	uchar_t		*modifier,	/* Modifier string   */
	int		*sub) {		/* Returns parameter */

#define	M_CMP(val)	(modifier[0] == val[0] && strcmp (modifier, val) == 0)

	if	(M_CMP ("from-ucs"))	*sub = SUBSTITUTE_FROM_UNICODE;
	else if (M_CMP ("to-ucs"))	*sub = SUBSTITUTE_TO_UNICODE;
	else if (M_CMP ("yes"))		*sub = SUBSTITUTE_BOTH_WAYS;
	else if (M_CMP ("no"))		*sub = NO_SUBSTITUTION;
	else	 			return -1;
	return 0;

#undef	M_CMP
}

/*
 *   NAME:	_chk_subchar
 *
 *   FUNCTION:	Check Syntax of 'subchar' modifier, and extract parameters.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static	int		_chk_subchar (
	uchar_t		*modifier,	/* Modifier string   */
	subchar_t	*subchar) {	/* Returns parameter */


	if (modifier == NULL || *modifier == '\0') return -1;

	if (*modifier != '\\') {
		strcpy (subchar->val, modifier);
		subchar->len = strlen (modifier);
	}
	else {	/*
		 *	Numerial character expression.
		 *	ptr                        tail_ptr
		 *	|                          |
		 *	V                          V
		 *	+----------------------------+
		 *	|\|x|XX|\|x|XX| ::: |\|x|XX|0|
		 *	+----------------------------+
		 */

		uchar_t		*ptr, *tail_ptr, *next_ptr;
		ulong_t		ulong_buf;
		int		base;

		tail_ptr = (ptr = modifier) + strlen (modifier);
		subchar->len = 0;
		while (ptr < tail_ptr) {

			if (((tail_ptr - ptr) < 3) || (*ptr != '\\'))
				return -1;

			if      (*(ptr + 1) == 'x') base = 16;
			else if (*(ptr + 1) == 'd') base = 10;
			else return -1;

			ulong_buf = strtoul (ptr + 2, &next_ptr, base);
			if ((ptr + 2 == next_ptr) || (ulong_buf > 0xff))
				return -1;

			ptr = next_ptr;
			subchar->val[subchar->len++] = (uchar_t)ulong_buf;

			if (subchar->len > (STEM_MAX+2)) return -1;
		}
		subchar->val[subchar->len] = '\0';
	}
	return 0;
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

	uchar_t			to_name[PATH_MAX+1], *cs_name, *modifier;
	int			t_len, f_len, ret;
	_LC_ucs_iconv_t		*cd     = NULL;

	/*
	 *	Supported modifier names & parser routine addresses table.
	 */

	int			map	= MAP_IRV;
	int			sub     = SUBSTITUTE_FROM_UNICODE;
	subchar_t		subchar = {{'\0'}, 0};
	endian_t		endian  = {ENDIAN_SYSTEM, ENDIAN_SYSTEM};
	_m_entry_t		_m_array[] = {
		{"map"		,_chk_map	,&map		,NULL ,0},
		{"sub"		,_chk_sub	,&sub		,NULL ,0},
		{"subchar"	,_chk_subchar	,&subchar	,NULL ,0},
		{"endian"	,_chk_endian	,&endian	,NULL ,0},
		{NULL		,NULL		,NULL		,NULL ,0}
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
	memcpy (to_name, t_name, t_len);
	to_name[t_len] = '\0';
	cs_name = (f_len) ? f_name : to_name;

	/*
	 *	Get parameter from the modifier.
	 */

	if ((ret = __get_modifier (modifier, _m_array)) < 0) {
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

	/*
	 *	Open UCS converter.
	 */

	ret = UCCINIT (
		cs_name,		/* MBCS name                 */
		map,			/* Map option                */
		sub,			/* Conversion option         */
		(UniChar)0x001a,	/* Substitution char in UCS  */
		subchar.val,		/* Substitution char in MBCS */
		&(cd->object.ch));	/* Returns conversion handle */

	if (ret == UC_NO_ERRORS) {

		/*
		 *	Make conversion descriptor.
		 */

		cd->object.core              = *core;
		cd->object.uconv_from_ucs    = (int(*)())_uconv_from_ucs;
		cd->object.uconv_to_ucs      = (int(*)())_uconv_to_ucs;
		if (f_len == 0)
			cd->object.core.exec = (size_t(*)())_iconv_from_ucs;
		else	cd->object.core.exec = (size_t(*)())_iconv_to_ucs;
		cd->endian                   = endian;
		return (_LC_ucs_iconv_t*)cd;
	}
	else {
		free (cd);
		return (_LC_ucs_iconv_t*)-1;
	}
}

/*
 *   NAME:      instantiate
 *
 *   FUNCTION:  Instantiation method of this converter.
 *
 *   RETURNS:   Pointer to the descriptor.
 */

_LC_core_iconv_t	*instantiate(void) {

        static _LC_core_iconv_t core;

        core.hdr.__magic   = _LC_MAGIC;
        core.hdr.__version = _LC_VERSION | _LC_ICONV_MODIFIER;
        core.hdr.__type_id = _LC_ICONV;
        core.hdr.__size    = sizeof (_LC_core_iconv_t);
        core.init          = (_LC_core_iconv_t*(*)())_iconv_init;
        core.close         = (int(*)())_iconv_close;
        core.exec          = (size_t(*)())NULL;

        return &core;
}
