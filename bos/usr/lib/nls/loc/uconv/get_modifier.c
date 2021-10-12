static char sccsid[] = "@(#)09  1.3  src/bos/usr/lib/nls/loc/uconv/get_modifier.c, cmdiconv, bos41J, 9509A_all 2/19/95 23:20:49";
/*
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		__get_modifier
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
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#include "get_modifier.h"

/*
 *   NAME:	__get_modifier
 *
 *   FUNCTION:	Parse modifier string and extract parameters.
 *
 *   RETURNS:	>=0	- Number of modifiers found in the string.
 *		-1	- Error.
 */
/*
 *   NOTE:	_m_entry_t (modifier table element)
 *
 *	Description:	The _m_entry_t is element of array of modifier name and
 *			pointer to its parser function.  Each conversion method
 *			has its own table to define modifiers, and passes it to
 *			__get_modifier() to validate specified modifier string.
 *
 *	Structure:
 *		uchar_t		*name;		* modifier name   *
 *		int		(*p_func)();	* parser function *
 *		void		*p_arg;		* modifier value  *
 *		uchar_t		*value;
 *		size_t		length;
 */

int	__get_modifier (
	const char	*m_string,
	_m_entry_t	*m_list) {

	uchar_t		*m_ptr, *eql_ptr, *v_ptr, *comma_ptr;
	int		m_len, v_len, m_count, i;


	if ( m_string == NULL) return -1;
	if (*m_string == '\0') return 0;

	/*
	 *	Clear 'value' and 'length' fields of given structure.
	 */

	for (i = 0; m_list[i].name != NULL; i ++) {
		m_list[i].value  = NULL;
		m_list[i].length = -1;
	}

	/*
	 *	Loop to process each modifier entry.
	 */

	if (*(m_ptr = (uchar_t*)m_string) == '@') m_ptr ++;

	m_count = 0;	/* Reset number of modifiers found */

	while (TRUE) {

		/*
		 *	Set work pointers as follows.
		 *
		 *	    m_ptr      eql_ptr  v_ptr  comma_ptr
		 *	    |          |        |      |
		 *	    |          |   +----+  +---+
		 *	    V	       V   V       V
		 *	+------------------------------+
		 *	| @ | modifier | = | value | , |
		 *	+------------------------------+
		 *	    <---------->   <------->
		 *	    m_len          v_len
		 */

		if ((comma_ptr = strchr (m_ptr, ',')) == NULL)
			comma_ptr = m_ptr + strlen (m_ptr);

		if (((eql_ptr = strchr (m_ptr, '=')) == NULL) ||
		    ((eql_ptr + 1) >= comma_ptr))
			return -1;

		v_ptr = eql_ptr + 1;
		m_len = eql_ptr - m_ptr;
		v_len = comma_ptr - v_ptr;

		/*
		 *	Check if a modifier matches given modifier patterns.
		 */

		for (i = 0; m_list[i].name != NULL; i ++) {

			if (memcmp (m_ptr, m_list[i].name, m_len) == 0) {

				if (m_list[i].value != NULL) return -1;
				if (m_len != strlen (m_list[i].name)) continue;

				/*
				 *	If specific syntax-checker exists,
				 *	invoke it with given p_arg.
				 */

				if (m_list[i].p_func != NULL) {

					uchar_t	tmp_value[PATH_MAX+1];

					strncpy (tmp_value, v_ptr, v_len);
					tmp_value[v_len] = '\0';
					if (((*(m_list[i].p_func))(
						tmp_value, m_list[i].p_arg)) < 0)
						return -1;
				}

				/*
				 *	Set modifier value to given structure
				 */

				m_list[i].value  = v_ptr;
				m_list[i].length = v_len;
				m_count ++;
				break;
			}
		}

		/*
		 *	If no match, say bye-bye
		 */

		if (m_list[i].name == NULL) return -1;
					
		/*
		 *	If it is the last modifier element in m_string, escape
		 *	from the loop, otherwise, prepare for the next element.
		 */

		if (*comma_ptr == '\0') break;
		m_ptr = comma_ptr + 1;
	}
	return m_count;
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

static uchar_t swap_endian( int endian ) {
	static uchar_t system[2] = { 0xff, 0xfe };
	int sys_endian;
	if ( endian == ENDIAN_SYSTEM ) return FALSE;
	if ( *((UniChar*)&system[0]) == 0xfffe )
		sys_endian = ENDIAN_BIG;
	else
		sys_endian = ENDIAN_LITTLE;

	if ( endian != sys_endian )
		return TRUE;
	else 
		return FALSE;
}

static	int		_chk_endian (
	uchar_t		*modifier,	/* Modifier string   */
	_m_endian_t	*endian) {	/* Returns parameter */

	int		n_names, source_len, i, j;
	uchar_t		*col_ptr, *target_ptr;


	n_names    = sizeof (endian_list) / sizeof (_m_value_t);
	col_ptr    = strchr (modifier, ':');
	if (col_ptr != NULL) {
		target_ptr = col_ptr + 1;
		source_len = col_ptr - modifier;
	} else	source_len = strlen (modifier);

	endian->source = endian->target = FALSE;  /* Assume ENDIAN SYSTEM */

	for (i = 0; i < n_names; i ++) {
		if (strncmp (endian_list[i].name, modifier, source_len) != 0)
			continue;

		if (col_ptr == NULL) {
			endian->source = 
			endian->target = swap_endian(endian_list[i].value);
			return 0;
		}
		for (j = 0; j < n_names; j ++) {
			if (strcmp (endian_list[j].name, target_ptr) != 0)
				continue;
			endian->source = swap_endian(endian_list[i].value);
			endian->target = swap_endian(endian_list[j].value);
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
	_m_subchar_t	*subchar) {	/* Returns parameter */


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
 *   NAME:	__iconv_parse_modifier
 *
 *   FUNCTION:	Parses toname and fromname of iconv_open modifiers.
 *
 *   RETURNS:	This function 
 *		a. converts any '@' in toname to \n
 *		b. Fills in iconvModRec
 */

        uchar_t* 		__iconv_parse_modifier (
	uchar_t*		t_name,
	uchar_t*		f_name,
	_m_core_t*		mod,
	uchar_t*		to_name) {

	uchar_t			*cs_name, *modifier;
	int			t_len, f_len, ret;
	_m_entry_t	_m_array[] = {
		{"map"		,_chk_map	,&mod->map	,NULL ,0},
		{"sub"		,_chk_sub	,&mod->sub	,NULL ,0},
		{"subchar"	,_chk_subchar	,&mod->subchar	,NULL ,0},
		{"endian"	,_chk_endian	,&mod->swap_endian,NULL ,0},
		{NULL		,NULL		,NULL	,NULL ,0}
	};

	/*
	 *	Supported modifier names & parser routine addresses table.
	 */

	mod->map	         = MAP_IRV;
	mod->sub                 = SUBSTITUTE_FROM_UNICODE;
	mod->subchar.val[0]      = '\0';
	mod->subchar.len         = 0;
	mod->subchar.ucs         = 0x001a;
	mod->swap_endian.source  = ENDIAN_SYSTEM;
	mod->swap_endian.target  = ENDIAN_SYSTEM;

	f_len = strlen (f_name);
        if ((modifier = (uchar_t*)strchr (t_name, '@')) == NULL) {
		modifier = "\0";
		t_len = strlen (t_name);
	} else	t_len = modifier - t_name;

	if ((f_len == 0) && (t_len == 0)) {
		errno = EINVAL;
		return (NULL);
	}
	memcpy (to_name, t_name, t_len);
	to_name[t_len] = '\0';
	cs_name = (f_len) ? f_name : to_name;

	/*
	 *	Get parameter from the modifier.
	 */

	if ((ret = __get_modifier (modifier, _m_array)) < 0) {
		errno = EINVAL;
		return (NULL);
	}

	return cs_name;
}
