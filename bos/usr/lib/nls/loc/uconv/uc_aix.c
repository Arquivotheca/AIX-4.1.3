static char sccsid[] = "@(#)11  1.1  src/bos/usr/lib/nls/loc/uconv/uc_aix.c, cmdiconv, bos411, 9428A410j 11/2/93 10:33:59";
/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:		getTableName
 *			getUcTable
 *			freeUcTable
 *			resetState
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
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include "iconvP.h"
#include "uc_conv.h"

/*
 *	Anchor of conversion table control block.
 */

static	_uc_table_t	*_uc_table_anchor = NULL;

/*
 *   NAME:	getTableName
 *
 *   FUNCTION:	Check if specified conversion table is available. If it is
 *		available, returns its file/path name.
 *
 *   RETURNS:	This function searches $LOCPATH directories for specified UCS
 *		conversion table and returns its File/Path name to the buffer
 *		pointed by 'table_name' argument. If failed to find the table
 *		it returns error status code to indicate error.
 *
 *   ERROR STATUS CODE:
 *	UC_NO_ERRORS		- Successful completion.
 *	UC_NOT_ENOUGH_SPACE	- Not enough strage.
 *	UC_OTHER_ERRORS
 */

int	getTableName (
	uchar_t		*cs_name,	/* MBCS name                    */
	uchar_t		*table_name) {	/* Returns table File/Path name */

	int		fname_len, locpath_len, def_path, file_exist, err_flag;
	uchar_t		*fname, *locpath, name_buf[PATH_MAX+1], *p;
	struct stat	stat_buf;


	/*
	 *	Make UCS conversion table name.
	 */

	fname_len = strlen (cs_name) + strlen (UCONV_TABLE_PATH);
	if ((fname = malloc (fname_len + 1)) == NULL) {
		errno = ENOMEM; return UC_NOT_ENOUGH_SPACE;
	}
	strcpy (fname, UCONV_TABLE_PATH);
	strcat (fname, cs_name);

	/*
	 *	Get $LOCPATH value.
	 */

	if (!__issetuid ()) {
		locpath = getenv ("LOCPATH"); def_path = FALSE;
	} else  locpath = NULL;
	if ((locpath == NULL) || (*locpath == '\0')) {
		locpath = DEF_LOCPATH; def_path = TRUE;
	}
	else if (strcmp (locpath, DEF_LOCPATH) == 0) def_path = TRUE;

	file_exist = FALSE;
	err_flag   = UC_NO_ERRORS;
	while (TRUE) {

		/*
		 *	Get next element of $LOCPATH.
		 */

		if (*locpath == '\0') {
			if (!def_path) {
				locpath = DEF_LOCPATH; def_path = TRUE;
			}
			else	break;
		}
		p = locpath;
		while ((*p != '\0') && (*p != ':')) p ++;
		if ((locpath_len = p - locpath) == 0) {
			name_buf[0] = '.';
			locpath_len = 1;
		}
		else  memcpy (name_buf, locpath, locpath_len);

		if ((locpath_len + fname_len) > PATH_MAX) {
			errno = ENAMETOOLONG;
			err_flag = UC_OTHER_ERRORS; continue;
		}
		if (*p != '\0') p ++;
		locpath = p;

		/*
		 *      Check if the UCS conversion table exists.
		 */

		strcpy (name_buf + locpath_len, fname);
		if (stat (name_buf, &stat_buf) == 0) {
			file_exist = TRUE;
			break;
		}
        }
	free (fname);

	if (file_exist) {
		strcpy (table_name, name_buf);
		return UC_NO_ERRORS;
	}
	else if (err_flag == UC_NO_ERRORS) {
		errno = EINVAL;
		return UC_TABLE_NOT_AVAILABLE;
	}
	else	return err_flag;
}

/*
 *   NAME:	getUcTable
 *
 *   FUNCTION:	This function searches chain of the conversion table control
 *		block (_uc_table_t) for specified table name.   If it is not
 *		found, create a new unit and add it to the chain.
 *
 *   RETURNS:	This function returns pointer to found unit to the variable
 *		pointed by 'uc_table' argument.  If the function fails,  it
 *		returns error status code.
 *
 *   ERROR STATUS CODE:
 *	UC_NO_ERRORS		- Successful completion.
 *	UC_NOT_ENOUGH_SPACE	- Not enough strage.
 */

int	getUcTable (
	uchar_t		*table_name,	/* Conversion table File/Path name */
	_uc_table_t	**uc_table) {	/* Returns pointer to found unit   */

	FILE		*fh     = NULL;	/* File handle of conversion table */
	_uc_table_t	*uc_tbl = NULL;	/* Conversion table control block  */
	_ucmap_com_t	ucmap_com;	/* Conversion table header         */
	int		ret;


	/*
	 *	Search for the conversion control block unit.
	 */

	for (uc_tbl  = _uc_table_anchor;
	     uc_tbl != NULL;
	     uc_tbl  = uc_tbl->next) {
		if ((uc_tbl->name != NULL)  &&
		    (strcmp (uc_tbl->name, table_name) == 0)) break;
	}
	if (uc_tbl == NULL) {

		/*
		 *	Allocate new unit.
		 */

		if ((uc_tbl = malloc (sizeof (_uc_table_t))) == NULL) {
			ret = UC_NOT_ENOUGH_SPACE; errno = ENOMEM;
			goto Bail;
		}
		if ((uc_tbl->name = malloc (strlen (table_name) + 1)) == NULL) {
			ret = UC_NOT_ENOUGH_SPACE; errno = ENOMEM;
			goto Bail;
		}
		strcpy (uc_tbl->name, table_name);

		/*
		 *	Read the conversion table.
		 */

		if (((fh = fopen (table_name, "rb")) == NULL) ||
		     (fread (&ucmap_com, sizeof (_ucmap_com_t), 1, fh) != 1)) {
			ret = UC_TABLE_NOT_AVAILABLE;
			goto Bail;
		}
		if ((uc_tbl->table = malloc (ucmap_com.size)) == NULL) {
			ret = UC_NOT_ENOUGH_SPACE; errno = ENOMEM;
			goto Bail;
		}
		if ((fseek (fh, 0L, SEEK_SET) != 0) ||
		    (fread (uc_tbl->table, ucmap_com.size, 1, fh) != 1)) {
			ret = UC_TABLE_NOT_AVAILABLE;
			goto Bail;
		}
		if ((uc_tbl->table->com.bom     != 0xfeff)  ||
		    (uc_tbl->table->com.version != UC_VERSION) ||
		    (uc_tbl->table->com.release != UC_RELEASE)) {
			ret = UC_INVALID_TABLE; errno = EINVAL;
			goto Bail;
		}

		/*
		 *	Link the new unit to the list.
		 */

		if (_uc_table_anchor == NULL) {
			_uc_table_anchor = uc_tbl;
			_uc_table_anchor->next = NULL;
		}
		else {
			uc_tbl->next = _uc_table_anchor->next;
			_uc_table_anchor->next = uc_tbl;
		}
		uc_tbl->anchor = _uc_table_anchor;
		uc_tbl->used_count = 0;
	}
	uc_tbl->used_count ++;

	*uc_table = uc_tbl;
	return UC_NO_ERRORS;

Bail:	if (uc_tbl != NULL) {
		if (uc_tbl->name  != NULL) free (uc_tbl->name);
		if (uc_tbl->table != NULL) free (uc_tbl->table);
		free (uc_tbl);
	}
	if (fh != NULL) fclose (fh);
	*uc_table = NULL;
	return ret;
}

/*
 *   NAME:	freeUcTable
 *
 *   FUNCTION:	Free conversion table control block.
 *
 *   RETURNS:	Error status code
 *
 *   ERROR STATUS CODE:
 *	UC_NO_ERRORS		- Successful completion.
 *	UC_INVALID_HANDLE	- Invalid conversion handle.
 */

int	freeUcTable (
	_uc_table_t	*uc_table) {	/* Pointer to the freed DATA_INFO */

	_uc_table_t	*uc_tbl, *uc_prev;


	/*
	 *	Search the chain for specified '_uc_table' unit.
	 */

	for (uc_tbl  = _uc_table_anchor;
	     uc_tbl != NULL;
	     uc_tbl  = uc_tbl->next) {
		if (uc_tbl == uc_table) break;
		uc_prev = uc_tbl;
	}
	if (uc_tbl == NULL) {
		errno = EBADF; return UC_INVALID_HANDLE;
	}

	/*
	 *	'used_count' indicates the number of users of the conversion
	 *	table.    If any users remain after this termination, do not
	 *	free uc_table and keep it in the chain.
	 */

	if ((uc_tbl->used_count --) > 0) return UC_NO_ERRORS;

	/*
	 *	Cut the '_uc_table_t' unit out from the chain.
	 */

	uc_prev->next = uc_tbl->next;

	/*
	 *	Free UCS conversion table.
	 */

	if (uc_tbl->name  != NULL) free (uc_tbl->name);
	if (uc_tbl->table != NULL) free (uc_tbl->table);
	free (uc_tbl);

	return UC_NO_ERRORS;
}

/*
 *   NAME:	resetState
 *
 *   FUNCTION:	Reset shift state saved in the UCS converter.
 *
 *   NOTE:	This function is based on the Iconv interface (XPG4).
 *
 *   RETURNS:	0	- Successfully completed.
 *		-1	- Error.
 */

size_t	resetState (
	_uc_ch_t	*ch,		/* Conversion handle           */
	uchar_t		**out_buf,	/* Output buffer               */
	size_t		*out_left,	/* Bytes left in output buffer */
	int		to_ebc) {	/* Conversion to EBCDIC        */

	size_t		ret;


	if (ch == NULL) {
		errno = EBADF; return (size_t)-1;
	}
	if (ch->uconv_class != UC_CLASS_EBCDIC_STATEFUL) return 0;

	if (ch->state_flag == SHIFT_IN) return 0;

	if (to_ebc) {
		if (out_buf != NULL) {
			if (*out_left >= 1) {
				**out_buf = (uchar_t)SI;
				*out_buf ++;
				*out_left --;
				ret = 0;
			}
			else {
				errno = E2BIG;
				ret = -1;
			}
		}
	}
	ch->state_flag = SHIFT_IN;
	return ret;
}
