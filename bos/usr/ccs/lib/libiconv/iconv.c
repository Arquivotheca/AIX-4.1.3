static char sccsid[] = "@(#)13  1.7.1.14 src/bos/usr/ccs/lib/libiconv/iconv.c, libiconv, bos411, 9438C411a 9/24/94 10:16:01";
/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:		instantiate
 *			iconv_open
 *			iconv_close
 *			iconv
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/lc_core.h>

#include <iconv.h>
#include <iconvTable.h>

#include "iconvP.h"

/*
 *   NAME:	_iconvTable_exec
 *
 *   FUNCTION:
 *      This function performs the one-to-one table conversion by a simple
 *	table lookup. This conversion is confined in a stateless character
 *	to character by indexing the array with the source codepoint.
 *
 *   RETURNS:
 *	This function updates the variables pointed by the arguments to reflect
 *	the extent of the conversion.  If the entire string in the input buffer
 *	is converted, the value pointed to by inbytesleft will be zero.  If the
 *	conversion is stopped due to condition described below,  the value will
 *	be non-zero and errno is set to indicate the condition.
 *
 *	This function returns 0 if successful completion,  or (size_t)-1 on an
 *	error condition.
 *
 *   ERROR CONDITION:
 *	E2BIG:
 *	If there is no room in the output buffer to place converted character,
 *	it is not put in the output buffer, and the inbuf points to the start
 *	of the character sequence that caused the output buffer overflow.
 */

static	size_t		_iconvTable_exec (iconvTable_t *cd,
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	register uchar_t	*in, *out, *table;
	register size_t		count;
	int			left, err_flag;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
	if (inbuf == NULL) return 0;

	if ((left = (int)(*outbytesleft - *inbytesleft)) >= 0) {
		count = *inbytesleft;
		*inbytesleft = 0;
		*outbytesleft = (size_t)left;
		err_flag = FALSE;
	}
	else {	count = *outbytesleft;
		*inbytesleft = (size_t)(- left);
		*outbytesleft = 0;
		err_flag = TRUE; errno = E2BIG;
	}
	table     = &(cd->table.data[0]);
	in        = *inbuf;
	out       = *outbuf;
	(*inbuf)  += count;		/* Update the buffer-pointers to    */
	(*outbuf) += count;		/* reflect extent of the conversion */

#ifdef _AIX
{	/*
	 *	Performance improvement by taking an advantage of
	 *	RISC/6000 processors and XLC compiler.
	 */

	register size_t		spin, index = 0;
	uchar_t			in_char0, out_char0,
				in_char1, out_char1;
	int			odd;

	odd   = count & 1;
	count = count >> 1;

	in_char0 = in[0];
	for (spin = 0; spin < count; spin ++) {
		in_char1     = in[index+1];
		out_char0    = table[in_char0];
		out_char1    = table[in_char1];
		in_char0     = in[index+2];
		out[index]   = out_char0;
		out[index+1] = out_char1;
		index += 2;
	}
	if (odd) out[index] = table[in_char0];
}
#else /*_AIX*/
{
	while ((count --) > 0) out[count] = table[in[count]];
}
#endif /*_AIX*/

	if (!err_flag)	return 0;
	else		return -1;
}

/*
 *   NAME:	_iconvTable_Lower_exec
 *
 *   FUNCTION:
 *      This function performs the one-to-one table conversion by a simple
 *	table lookup. This conversion is confined in a stateless character
 *	to character by indexing the array with the source codepoint.
 *
 *   RETURNS:
 *	This function updates the variables pointed by the arguments to reflect
 *	the extent of the conversion.  If the entire string in the input buffer
 *	is converted, the value pointed to by inbytesleft will be zero.  If the
 *	conversion is stopped due to condition described below,  the value will
 *	be non-zero and errno is set to indicate the condition.
 *
 *	This function returns the number of non-identical conversions performed
 *	or (size_t)-1 on any error condition.
 *
 *   ERROR CONDITIONS:
 *	EILSEQ:
 *      If an input character code does not belong to the source code set,
 *	no conversion is attempted,  and the inbuf points to the start of
 *	unindentified input character.
 *	E2BIG:
 *	If there is no room in the output buffer to place converted character,
 *	it is not put in the output buffer, and the inbuf points to the start
 *	of the character sequence that caused the output buffer overflow.
 */

static	size_t		_iconvTable_Lower_exec (iconvTable_t *cd,
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	register uchar_t	*in, *out, *table;
	register size_t		count, index;
	uchar_t			inval_handle, sub_handle,
				inval_mark, sub_mark, sub_char;
	size_t			subs;		/* Substitution counter */
	int			err_level;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return (size_t)-1;
	}
	if (inbuf == NULL) return (size_t)0;

	inval_handle =   cd->table.inval_handle;
	inval_mark   =   cd->table.inval_char;
	sub_handle   =   cd->table.sub_handle;
	sub_char     =   cd->table.sub_char;
	sub_mark     =   cd->table.sub_mark;
	table        = &(cd->table.data[0]);
	in           = *inbuf;
	out          = *outbuf;
	subs         = 0;
	index        = 0;

	if (*outbytesleft >= *inbytesleft) {
		count = *inbytesleft;
		err_level = 0;
	}
	else {	count = *outbytesleft;

		if (inval_handle &&
		   (table[in[count]] == inval_mark))
			errno = EILSEQ;
		else	errno = E2BIG;
		err_level = 1;
	}
#ifdef _AIX
{	/*
	 *	Performance improvement by taking an advantage of
	 *	RISC/6000 processors and XLC compiler.
	 */

	register size_t		spin;
	uchar_t			in_char0, out_char0,
				in_char1, out_char1;
	int			odd;

	odd   = count & 1;
	count = count >> 1;

	in_char0 = in[0];
	for (spin = 0; spin < count; spin ++) {
		in_char1  = in[index+1];
		out_char0 = table[in_char0];
		out_char1 = table[in_char1];
		in_char0  = in[index+2];
		if (inval_handle && (out_char0 == inval_mark)) {
			errno = EILSEQ; err_level = 2; break;
		}
		if (sub_handle && (out_char0 == sub_mark)) {
			out[index] = sub_char; subs ++;
		} else	out[index] = out_char0;
		index ++;

		if (inval_handle && (out_char1 == inval_mark)) {
			errno = EILSEQ; err_level = 2; break;
		}
		if (sub_handle && (out_char1 == sub_mark)) {
			out[index] = sub_char; subs ++;
		} else	out[index] = out_char1;
		index ++;
	}
	if (odd && (err_level < 2)) {
		out_char0 = table[in_char0];
		if (inval_handle && (out_char0 == inval_mark)) {
			errno = EILSEQ; err_level = 2;
		}
		else {	if (sub_handle && (out_char0 == sub_mark)) {
				out[index] = sub_char; subs ++;
			} else	out[index] = out_char0;
			index ++;
		}
	}
}
#else /*_AIX*/
{
	uchar_t		out_char;

	while ((count --) > 0) {
		out_char = table[in[index]];
		if (inval_handle && (out_char == inval_mark)) {
			errno = EILSEQ; err_level = 2; break;
		}
		if (sub_handle && (out_char == sub_mark)) {
			out[index] = sub_char; subs ++;
		} else	out[index] = out_char;
		index ++;
	}
}
#endif /*else _AIX*/

	(*inbytesleft)  -= index; (*inbuf)  += index;
	(*outbytesleft) -= index; (*outbuf) += index;
	if (err_level == 0)	return subs;
	else			return -1;
}

/*
 *   NAME:	_iconvTable_close
 *
 *   FUNCTION:	Close function for table converter.
 *
 *   RETURNS:	0 if successful completion, -1 if error.
 */

static	int	_iconvTable_close (iconvTable_t *cd) {

	if ((cd != NULL) && (cd != -1)) {
		free (cd);
		return 0;
	}
	else {	errno = EBADF;
		return -1;
	}
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
 *   NAME:	_is_ucsName
 *
 *   FUNCTION:	Check if input codeset name is UCS or not.
 *
 *   RETURNS:	TRUE	- UCS name
 *		FALSE	- non-UCS name
 */

static	uchar_t		*_ucsAlias_buf = NULL;
static	int		_ucsAlias_buf_size = 0;
static	int		_search_ucsAlias ();
static	int		_read_ucsAlias ();

static	int		_is_ucsName (uchar_t *cs_name) {

	if (_search_ucsAlias (cs_name))	return TRUE;
	else if (!_read_ucsAlias ())	return FALSE;
	else return _search_ucsAlias (cs_name);
}

/*
 *   NAME:	_search_ucsAlias
 *
 *   FUNCTION:	Search UCS alias name buffer.
 *
 *   RETURNS:	TRUE if found, FALSE if not found.
 */

static	int		_search_ucsAlias (uchar_t *cs_name) {

	uchar_t		*alias_name, *p;


	if (strcmp (cs_name, DEF_UCS_NAME) == 0) return TRUE;

	if (_ucsAlias_buf == NULL) return FALSE;

	p = _ucsAlias_buf;
	while (*p != '\0') {
		for (; isspace (*p); p ++);
		if (*p == '#') {		/* Comment line */
			for (p ++; (*p != '\n') && (*p != '\0'); p ++);
			continue;
		}
		if (*p == '\0') break;
		alias_name = p;
		for (; isgraph (*p); p ++);
		if (strncmp (cs_name, alias_name, p - alias_name) == 0)
			return TRUE;
	}
	return FALSE;
}

/*
 *   NAME:	_read_ucsAlias
 *
 *   FUNCTION:	Read UCS alias file into UCS name buffer.
 *
 *   RETURNS:	TRUE if successful completion, FALSE if error.
 *
 *   ERROR STATUS CODE: (errno)
 *	ENOMEM		- Not enough storage
 *	ENAMETOOLONG	- Too long file name
 *	EINVAL		- Alias name file not exist
 */

static	int		_read_ucsAlias () {

	struct stat	stat_buf;
	uchar_t		*fname, *locpath, name_buf[PATH_MAX+1];
	int		fname_len, locpath_len, def_path, file_exist, fd;


	fname_len = strlen (UCONV_ALIAS_FILE) + strlen (UCONV_TABLE_PATH);
	if ((fname = malloc (fname_len + 1)) == NULL) {
		errno = ENOMEM;
		return FALSE;
	}
	strcpy (fname, UCONV_TABLE_PATH);
	strcat (fname, UCONV_ALIAS_FILE);

	if (!__issetuid ()) {
		locpath = getenv ("LOCPATH"); def_path = FALSE;
	} else  locpath = NULL;
	if ((locpath == NULL) || (*locpath == '\0')) {
		locpath = DEF_LOCPATH; def_path = TRUE;
	}
	else if (strcmp (locpath, DEF_LOCPATH) == 0) def_path = TRUE;

	file_exist = FALSE;
	while (TRUE) {

		if (*locpath == '\0') {
			if (!def_path) {
				locpath = DEF_LOCPATH; def_path = TRUE;
			}
			else {	errno = EINVAL;
				break;
			}
		}
		locpath_len = _get_next_locpath (&locpath, &name_buf[0]);
		if ((locpath_len + fname_len) > PATH_MAX) {
                        errno = ENAMETOOLONG;
			break;
		}
		strcpy (&name_buf[locpath_len], fname);
		if (stat (name_buf, &stat_buf) == 0) {
			file_exist = TRUE;
			break;
		}
	}
	if (!file_exist) {
		free (fname);
		return FALSE;
	}
	if (_ucsAlias_buf_size < stat_buf.st_size) {
		_ucsAlias_buf_size = stat_buf.st_size + 1;
		if  (_ucsAlias_buf != NULL) free (_ucsAlias_buf);
		if ((_ucsAlias_buf = malloc (_ucsAlias_buf_size)) == NULL) {
			errno = ENOMEM;
			free (fname);
			return FALSE;
		}
	}
	if ((fd = open (name_buf, O_RDONLY)) < 0) {
		free (fname);
		return FALSE;
	}
	if (read (fd, _ucsAlias_buf, stat_buf.st_size) != stat_buf.st_size) {
		close (fd); free (fname);
		return FALSE;
	}
	_ucsAlias_buf[stat_buf.st_size] = '\0';

	close (fd); free (fname);
	return TRUE;
}

/*
 *   NAME:	instantiate
 *
 *   FUNCTION:	Instantiation method called by __lc_load().
 *
 *	iconv_open()
 *	|    calls the __lc_load() to load specified converter.
 *	|
 *	+-- __lc_load()
 *	|   |	loads the converter with the load() that returns pointer to
 *	|   |	a routine to instantiate the the loaded converter.
 *	|   |
 *	|   +-- instantiate()
 *	|	|    is passed the pointer to the instantiater, and calls it.
 *	|	|
 *	|	+-- Instantiater of the loaded converter
 *	|   		allocates cd (conversion descriptor) and makes header
 *	|   		portion (_LC_object_t),   and sets entry addresses of
 *	|   		init(), close() and exec() functions of the converter.
 *	|
 *	iconv_open() gets (_LC_core_iconv_t*)cd that has _LC_object_t portion
 *	as a header and the pointers to the functions.
 *
 *   RETURNS:	Pointer to a conversion descriptor if successfully completed,
 *		NULL if any error.
 */

static	_LC_object_t	*instantiate (
	char		*path,
	_LC_object_t	*(*inst)()) {	/* Method instantiater */

	_LC_object_t	*header;

	if (inst == NULL) return NULL;

	header = (*inst)();

	if ((header->__magic   == _LC_MAGIC) &&
	    (header->__type_id == _LC_ICONV))
		return header;
	else	return NULL;
}

/*
 *   NAME:	iconv_open
 *
 *   FUNCTION:	Load specified converter and initialize it.
 *
 *   RETURNS:	Pointer to the conversion descriptor if successful
 *		completion, (iconv_t)-1 if error.
 */

iconv_t	iconv_open (
	const uchar_t		*t_name,
	const uchar_t		*f_name) {

	int			method_name_len, table_name_len, name_len, *fd,
				t_len, t_is_ucs_name, f_is_ucs_name, def_path,
				err_save = errno,
				err_flag = FALSE;
	uchar_t			name_buf[PATH_MAX+1], *modifier, *locpath,
				*method_name = NULL,
				*table_name  = NULL;
	_LC_core_iconv_t	*core_cd;
	iconvTable_t		*tbl_cd;
	iconv_t			cd = -1;


	/*
	 *	Make converter method/table file path name, concatenating
	 *	"from" and "to" code set name. If modifier is attached to
	 *	the "to" name, cut it out.
	 *
	 *	CAUTION:	Only characters in portable character set
	 *			can be used in both "to" and "from" name.
	 */

	if (_is_ucsName (f_name)) {
		name_len = strlen (DEF_UCS_NAME);
		memcpy (&name_buf[0], DEF_UCS_NAME, name_len);
		f_is_ucs_name = TRUE;
	}
	else {	name_len = strlen (f_name);
		memcpy (&name_buf[0], f_name, name_len);
		f_is_ucs_name = FALSE;
	}
	name_buf[name_len ++] = '_';
	if ((modifier = (uchar_t*)strchr (t_name, '@')) == NULL) {
		modifier = "\0";
		t_len = strlen (t_name);
	} else	t_len = modifier - t_name;
	memcpy (&name_buf[name_len], t_name, t_len);
	name_buf[name_len + t_len] = '\0';
	if (_is_ucsName (&name_buf[name_len])) {
		t_len = strlen (DEF_UCS_NAME);
		memcpy (&name_buf[name_len], DEF_UCS_NAME, t_len);
		name_buf[name_len + t_len] = '\0';
		t_is_ucs_name = TRUE;
	} else	t_is_ucs_name = FALSE;
	name_len += t_len;
	method_name_len = name_len + strlen (ICONV_METHOD_PATH);
	table_name_len  = name_len + strlen (ICONV_TABLE_PATH);
	if (((method_name = malloc (method_name_len + 1)) == NULL) ||
	    ((table_name  = malloc (table_name_len  + 1)) == NULL)) {
		errno = ENOMEM; goto Return;
	}
	strcpy (method_name, ICONV_METHOD_PATH);
	strcat (method_name, name_buf);
	strcpy (table_name , ICONV_TABLE_PATH);
	strcat (table_name , name_buf);

	/*
	 *	Check uid and gid, and set an appropreate search
	 *	path to find a file which will be loaded.
	 */

	if (!__issetuid ()) {
		locpath = getenv ("LOCPATH"); def_path = FALSE;
	} else  locpath = NULL;
	if ((locpath == NULL) || (*locpath == '\0')) {
		locpath = DEF_LOCPATH; def_path = TRUE;
	}
	else if (strcmp (locpath, DEF_LOCPATH) == 0) def_path = TRUE;

	while (TRUE) {

		if (*locpath == '\0') {
			if (!def_path) {
				locpath = DEF_LOCPATH; def_path = TRUE;
			}
			else {	errno = EINVAL;
				break;
			}
		}
		name_len = _get_next_locpath (&locpath, &name_buf[0]);
		if (((name_len + method_name_len) > PATH_MAX) ||
		    ((name_len +  table_name_len) > PATH_MAX)) {
			errno = ENAMETOOLONG;
			break;
		}

		/*
		 *	Check if method converter is available.
		 */

		strcpy (&name_buf[name_len], method_name);
		if ((core_cd = (_LC_core_iconv_t*)__lc_load (
			name_buf, instantiate)) != NULL) {

			/*
			 *	Check if the method supports modifier.
			 */

			if ((*modifier != '\0') &&
			    ((core_cd->hdr.__version < _LC_VERSION) ||
			    !(core_cd->hdr.__version & _LC_ICONV_MODIFIER))) {
				errno = EINVAL;
				break;
			}

			/*
			 *	Invoke method initializer.
			 */

			if (t_is_ucs_name) t_name = modifier;
			if (f_is_ucs_name) f_name = "\0";

			cd = (iconv_t)((*(core_cd->init))(core_cd, t_name, f_name));

			if ((cd != -1) && (cd != NULL)) {
				errno = err_save;
				break;		/* Happy end */
			}
			cd = -1;
                }

		/*
		 *	Check if table converter is available.
		 */

		strcpy (&name_buf[name_len], table_name);
		if ((fd = open (name_buf, O_RDONLY)) > 0) {

			if ((tbl_cd = malloc (sizeof (iconvTable_t))) == NULL) {
				close (fd);
				errno = ENOMEM;
				break;
			}
			if (read (fd, &tbl_cd->table, sizeof (IconvTable)) !=
				sizeof (IconvTable)) {
				close (fd); free (tbl_cd);
				continue;
			}
			close (fd);
			if ((tbl_cd->table.magic != ICONV_REL1_MAGIC) &&
			    (tbl_cd->table.magic != ICONV_REL2_MAGIC)) {
				free (tbl_cd);
				continue;
			}

			/*
			 *	Table converter supports no modifier.
			 */

			if (*modifier != '\0') {
				free (tbl_cd);
				errno = EINVAL;
				break;
			}

			/*
			 *	Initialize the table converter.
			 */

			tbl_cd->core.hdr.__magic   = _LC_MAGIC;
			tbl_cd->core.hdr.__version = _LC_VERSION;
			tbl_cd->core.hdr.__type_id = _LC_ICONV;
			tbl_cd->core.hdr.__size    = sizeof(_LC_core_iconv_t);
			if ((tbl_cd->table.magic == ICONV_REL2_MAGIC) && 
			    (tbl_cd->table.inval_handle || tbl_cd->table.sub_handle))
			     tbl_cd->core.exec = (size_t(*)())_iconvTable_Lower_exec;
			else tbl_cd->core.exec = (size_t(*)())_iconvTable_exec;
			tbl_cd->core.init  = (_LC_core_iconv_t*(*)())NULL;
			tbl_cd->core.close = (int(*)())_iconvTable_close;
	
			cd = (iconv_t)tbl_cd;
			errno = err_save;
			break;		/* Happy end */
		}
	}
Return:	if (method_name != NULL) free (method_name);
	if (table_name  != NULL) free (table_name);
	return cd;
}

/*
 *   NAME:	iconv_close
 *
 *   FUNCTION:	Pass control to each converter's close routine.
 *
 *   RETURNS:	Whatever the close function of each converter returns.
 */

int	iconv_close (iconv_t cd) {

	int	ret, err_save = errno;


	if ((cd != NULL) && (cd != -1)) {
		ret = (*(cd->close))(cd);
		if (ret != -1) errno = err_save;
		return ret;
	}
	else {	errno = EBADF;
		return -1;
	}
}

/*
 *   NAME:	iconv
 *
 *   FUNCTION:	Pass control to each converter's exec routine.
 *
 *   RETURNS:	Whatever the exec function of each converter returns.
 */

size_t	iconv (iconv_t cd,
	const uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	int		err_save = errno;
	size_t		ret;


	if ((cd != NULL) && (cd != -1)) {
		ret = ((*(cd->exec))(cd, inbuf, inbytesleft, outbuf, outbytesleft));
		if (ret != -1) errno = err_save;
		return ret;
	}
	else {	errno = EBADF;
		return (size_t)-1;
	}
}
