static char sccsid[] = "@(#)96  1.2  src/bos/usr/ccs/lib/libiconv/uconv.c, libiconv, bos411, 9428A410j 2/28/94 21:06:16";
/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:		UniCreateUconvObject
 *			UniFreeUconvObject
 *			UniUconvToUcs
 *			UniUcnvFromUcs
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
#include <uconv.h>

/*
 *   NAME:	_UniChar_to_char
 *
 *   FUNCTION:	Convert UniChar string to char string.
 *
 *   RETURNS:	Pointer to the converted char string.
 */

static	uchar_t		_name_buf[PATH_MAX+1];

static	uchar_t		*_UniChar_to_char (
	UniChar		*uni_str) {

	size_t		uni_len, i;
	UniChar		*uni_p;


	if (uni_str == NULL) return (uchar_t*)NULL;

	for (uni_p = uni_str, uni_len = 0;
	    *uni_p != (UniChar)0;
	     uni_p ++, uni_len ++);

	if (uni_len == 0) return (uchar_t*)NULL;

	memset (_name_buf, (uchar_t)0, uni_len);
	for (i = 0; i < uni_len; i ++) _name_buf[i] = (uchar_t)uni_str[i];
	_name_buf[i] = '\0';
	return &_name_buf[0];
}

/*
 *   NAME:	_get_next_locpath
 *
 *   FUNCTION:	Get the next path element from the given $LOCPATH.
 *		The result followed by '/' is stored in the buffer,
 *		and the pointer to LOCPATH element is advanced.
 *
 *   RETURNS:	Length of the got path element.
 */

static	int		_get_next_locpath (
	uchar_t		**locpath,	/* Pointer to LOCPATH element */
	uchar_t		*name_buf) {	/* Returns path name          */

	uchar_t		*p;
	int		len;

	p = *locpath;
	while ((*p != '\0') && (*p != ':')) p ++;
	if ((len = p - *locpath) == 0) {
		name_buf[0] = '.';
		len = 1;
	}
	else	memcpy (name_buf, *locpath, len);

	if (*p != '\0') p ++;
	*locpath = p;
	return len;
}

/*
 *   NAME:	_instantiate
 *
 *   FUNCTION:	Called from the __lc_load() and invoke instantiation routine
 *		of loaded method.
 *
 *   RETURNS:	Core header of loaded object.
 */

static	_LC_object_t	*_instantiate (
	char		*path,
	_LC_object_t	*(*inst)()) {

	_LC_object_t	*header;

	if (inst == NULL) return NULL;

	header = (*inst)();

	if ((header->__magic   == _LC_MAGIC) &&
	    (header->__type_id == _LC_ICONV))
		return header;
	else	return NULL;
}

/*
 *   NAME:	UniCreateUconvObject
 *
 *   FUNCTION:	Open UCS converter.
 *
 *   RETURNS:	This function returns pointer to a conversion object to
 *		the variable pointed by 'object' argument.
 *		This function returns error status code as function value
 *		and set 'errno' to indicate the error condition.
 *
 *   ERROR CONDITIONS:
 *	0		- Successful completion.
 *	UCONV_ENOMEM	- Insufficient storage space
 *	UCONV_EINVAL	- Invalid input parameter
 */

int	UniCreateUconvObject (
	UniChar			*name,		/* MBCS codeset name     */
	UconvObject		*object) {	/* conversion descriptor */

	int			method_name_len, cs_name_len, len, def_path;
	uchar_t			*method_name, *cs_name, *modifier, name_buf[PATH_MAX+1];
	char			*locpath;
	_LC_core_iconv_t	*core_cd;
	UconvObject		cd = NULL;
	int			ret = 0;


	/*
	 *	Make converter method file path name.
	 *
	 *	CAUTION:	Only characters in portable character set
	 *			can be used in MBCS codeset name.
	 */

	cs_name = _UniChar_to_char (name);

	if ((modifier = (uchar_t*)strchr (cs_name, '@')) != NULL)
		cs_name_len = modifier - cs_name;
	else	cs_name_len = strlen (cs_name);

	len = strlen (UCONV_METHOD_PATH);
	method_name_len = len + cs_name_len;
	if ((method_name = malloc (method_name_len + 1)) == NULL) {
		ret = UCONV_ENOMEM; errno = ENOMEM;
		goto Return;
	}
	strcpy (method_name, UCONV_METHOD_PATH);
	strncpy(method_name + len, cs_name, cs_name_len);
	method_name[method_name_len] = '\0';

	/*
	 *	Check uid and gid, and set an appropreate search
	 *	path to find a file which will be loaded.
	 */

	if (!__issetuid ()) {
		locpath = getenv ("LOCPATH"); def_path = FALSE;
	} else	locpath = NULL;
	if ((locpath == NULL) || (*locpath == '\0')) {
		locpath = DEF_LOCPATH; def_path = TRUE;
	}
	else if (strcmp (locpath, DEF_LOCPATH) == 0) def_path = TRUE;

	while (TRUE) {

		/*
		 *	Get next element of $LOCPATH.
		 */

		if (*locpath == '\0') {
			if (!def_path) {
				locpath = DEF_LOCPATH; def_path = TRUE;
			}
			else	break;	/* End of loop */
		}
		len = _get_next_locpath (&locpath, &name_buf[0]);
		if ((len + method_name_len) > PATH_MAX) {
			ret = UCONV_EINVAL; errno = ENAMETOOLONG;
			goto Return;
		}

		/*
		 *	Try to load the converter.
		 */

		strcpy (name_buf + len, method_name);
		if ((core_cd = (_LC_core_iconv_t*)__lc_load (
			name_buf, _instantiate)) != NULL) {

			/*
			 *	Check if the method supports modifier.
			 */

			if ((modifier != NULL) &&
			   ((core_cd->hdr.__version < _LC_VERSION) ||
			   !(core_cd->hdr.__version & _LC_ICONV_MODIFIER))) {
				ret = UCONV_EINVAL; errno = EINVAL;
				goto Return;
			}

			/*
			 *	Invoke method initializer.
			 */

			cd = (UconvObject)((*(core_cd->init))(core_cd, cs_name, NULL));

			if (cd == -1) {
				cd = NULL;
				ret = UCONV_EINVAL; errno = EINVAL;
			} else	ret = 0;
			goto Return;
		}
	}

	/*
	 *	Not found anywhere
	 */

	if (ret == 0) {
		ret = UCONV_EINVAL; errno = EINVAL;
	}
Return:	if (method_name != NULL) free (method_name);

	*object = cd;
	return ret;
}

/*
 *   NAME:	UniFreeUconvObject
 *
 *   FUNCTION:	Free conversion object.
 *
 *   RETURNS:	This function returns error status code as function value
 *		and set 'errno' to indicate the condition.
 *
 *   ERROR CONDITIONS:
 *	0		- Successful completion.
 *	UCONV_EBADF	- Invalid conversion object.
 */

int	UniFreeUconvObject (
	UconvObject	object) {

	if (object == NULL) {
		errno = EBADF; return UCONV_EBADF;
	}
	if ((*(object->core.close))(object) == -1) {
		errno = EBADF; return UCONV_EBADF;
	}
	return 0;
}

/*
 *   NAME:	UniUconvToUcs
 *
 *   FUNCTION:	Conversion to UCS.
 *
 *   RETURNS:	This function updates the variables pointed by the arguments to
 *		reflect the extent of the conversion.   If the entire string in
 *		input buffer is converted, the value pointed by in_left will be
 *		zero. If input conversion is stopped due to condition described
 *		below, the value will be non-zero.
 *		This function returns error status code as function value,  and
 *		set errno to indicate the condition.
 *
 *   ERROR CONDITION:
 *	0		- Successful completion.
 *	UCONV_EBADF	- Invalid conversion object.
 *	UCONV_EILSEQ	- Invalid input character.
 *		If an input character does not belong to the "from" codeset, no
 *		conversion is attempted and the content of in_buf points to the
 *		start of the unindentified input character.
 *	UCONV_E2BIG	- Lack of space in output buffer.
 *		If there is no room in the output buffer to place the converted
 *		character, it is not placed and the content of in_buf points to
 *		the start of the character sequence that caused overflow.
 */

int	UniUconvToUcs (
	UconvObject	object,		/* Conversion object                */
	void		**in_buf,	/* Input buffer                     */
	size_t		*in_left,	/* #of bytes left in input buffer   */
	UniChar		**ucs_buf,	/* UCS output buffer                */
	size_t		*ucs_left, 	/* #of UniChars left in UCS buffer  */
	size_t		*non_identical){/* #of non-identical conversions    */


	if (object != NULL) {
		return ((*(object->uconv_to_ucs))(object,
			in_buf, in_left, ucs_buf, ucs_left, non_identical));
	}
	else {
		errno = EBADF; return UCONV_EBADF;
	}
}

/*
 *   NAME:	UniUconvFromUcs
 *
 *   FUNCTION:	Conversion from UCS.
 *
 *   RETURNS:	This function updates the variables pointed by the arguments to
 *		reflect the extent of the conversion.   If the entire string in
 *		input buffer is converted, the value pointed by in_left will be
 *		zero. If input conversion is stopped due to condition described
 *		below, the value will be non-zero.
 *		This function returns error status code as function value,  and
 *		set errno to indicate the condition.
 *
 *   ERROR CONDITION:
 *	0		- Successful completion.
 *	UCONV_EBADF	- Invalid conversion object.
 *	UCONV_EILSEQ	- Invalid input character.
 *		If an input character does not belong to the "from" codeset, no
 *		conversion is attempted and the content of in_buf points to the
 *		start of the unindentified input character.
 *	UCONV_E2BIG	- Lack of space in output buffer.
 *		If there is no room in the output buffer to place the converted
 *		character, it is not placed and the content of in_buf points to
 *		the start of the character sequence that caused overflow.
 */

int	UniUconvFromUcs (
	UconvObject	object,		/* Conversion object                */
	UniChar		**ucs_buf,	/* UCS string input buffer          */
	size_t		*ucs_left,	/* #of UniChars left in UCS buffer  */
	void		**out_buf,	/* Output buffer                    */
	size_t		*out_left,	/* #of bytes left in output buffer  */
	size_t		*non_identical){/* #of non-identical conversions    */


	if (object != NULL) {
		return ((*(object->uconv_from_ucs))(object,
			ucs_buf, ucs_left, out_buf, out_left, non_identical));
	}
	else {
		errno = EBADF; return UCONV_EBADF;
	}
}
