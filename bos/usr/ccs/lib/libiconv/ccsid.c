static char sccsid[] = "@(#)14  1.4  src/bos/usr/ccs/lib/libiconv/ccsid.c, libiconv, bos411, 9428A410j 12/7/93 14:12:02";
/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

#include "iconvP.h"
#include "ccsid.h"

static	ccsid_data_t	to_tbl = NULL, from_tbl = NULL;
static	ccsid_hdr_rec	ccsid_hdr = {0, "", 0, 0};


static
int	nextpath(char **locpath, char *buf)
{
	char	*locptr;
	int	len;

	locptr = *locpath;
	while (*locptr && *locptr != ':') locptr++;
	len = locptr - *locpath;
	if (len > _POSIX_PATH_MAX - 1) return -1;
	if (len == 0) {
		locptr++;
		buf[0] = '.';
		len = 1;
	} else {
		if (*locptr) ++locptr;
		(void)memcpy(buf, *locpath, len);
	}
	buf[len++] = '/';
	*locpath = locptr;
	return len;
}


static
int	is_header_valid(ccsid_hdr_t hdr)
{
	/*
	 *	Do you really want to check it ?
	 *	If yes, do it here.
	 */
	return TRUE;
}


void ccsid_close()
{
	if (to_tbl != NULL) {
		(void)free(to_tbl);
		to_tbl = NULL;
	}
	if (from_tbl != NULL) {
		(void)free(from_tbl);
		from_tbl = NULL;
	}
	ccsid_hdr.to_tbl_cnt = 0;
	ccsid_hdr.from_tbl_cnt = 0;
	return;
}


void	ccsid_open(uchar_t *tbl_path)
{
	int	fd = -1;
	int	i, pathlen, tbl_len, error = FALSE;
	uint_t	hdr_siz, to_siz, from_siz;
	uchar_t	*path, *search[3];
	uchar_t	namebuf[_POSIX_NAME_MAX * 2 + 2];
	uchar_t	tbl_name[_POSIX_PATH_MAX + 1];

	(void)strcpy(tbl_name, CCSID_PATH CCSID_TABLE);
	tbl_len = strlen(tbl_name);

	/*
	 *	Search data table in the following order
	 *		1. User-specified path.
	 *		2. LOCPATH
	 *		3. Fixed default path
	 */
	search[0] = tbl_path;
	search[1] = getenv("LOCPATH");
	search[2] = DEF_LOCPATH;

	for (i = 0; i < 3; i++) {
		if (search[i] == NULL) continue;
		path = search[i];

		/*
		 *	Search data table w/each element in path
		 */
		while (*path != '\0') {

			/*
			 *      Construct data file name
			 */
			pathlen = nextpath(&path, namebuf);
			if (pathlen < 0 ||
					_POSIX_PATH_MAX < pathlen + tbl_len) {
				errno = ENAMETOOLONG; return;
			}
			(void)strcpy(namebuf + pathlen, tbl_name);

			/*
			 *	Check to see if a valid data table exists
			 */
			hdr_siz = sizeof(ccsid_hdr);
			if ((fd = open(namebuf, O_RDONLY)) < 0) continue;
			if (read(fd, &ccsid_hdr, hdr_siz) != hdr_siz ||
				!is_header_valid(&ccsid_hdr)) {
				(void)close(fd); continue;
			}

			/*
			 *	Allocate memory and read data
			 */
			to_siz = ccsid_hdr.to_tbl_cnt * sizeof(ccsid_data_rec);
			from_siz = ccsid_hdr.from_tbl_cnt *
						sizeof(ccsid_data_rec);
			to_tbl = (ccsid_data_t)malloc(to_siz);
			from_tbl = (ccsid_data_t)malloc(from_siz);
			if ((to_tbl == NULL) || (from_tbl == NULL)) {
				error = TRUE; break;
			}
			if (read(fd, from_tbl, from_siz) != from_siz ||
					read(fd, to_tbl, to_siz) != to_siz) {
				error = TRUE; break;
			}
			/*
			 *	Happy End !
			 */
			(void)close(fd);
			return;
		}

		/*
		 *	If error occured...
		 */
		if (error) {
			if (fd >= 0) (void)close(fd);
			ccsid_close();
			return;
		}

		/*
		 *	Change path and search again
		 */
	}
	/*
	 *	NOT FOUND ANYWHERE
	 */
	return;
}


/*
 *	CCSID -> CS
 */
char *ccsidtocs(CCSID ccsid)
{
	int	high, low, i;

	if (from_tbl == NULL) {
		(void)ccsid_open(NULL);
		if (from_tbl == NULL) return NULL;
	}

	low = 0;
	high = ccsid_hdr.from_tbl_cnt - 1;
	while (low <= high) {
		i = (low + high) >> 1;
		if (ccsid < from_tbl[i].ccsid) high = i - 1;
		else if (ccsid > from_tbl[i].ccsid) low = i + 1;
		else {
			return from_tbl[i].cs;
		}
	}
	return NULL;
}


/*
 *	CS -> CCSID
 */
#ifdef CCSID_HASH
static
uint_t	find_idx(ccsid_data_t tbl, uint_t tbl_siz, char *str)
{
	uint_t	index, hash_val;

	index = hash_val = ccsid_hash(str, tbl_siz);
	for (; index < tbl_siz; index++)
		if (!strcmp(str, tbl[index].cs)) return index;
	for (index = 0; index < hash_val; index++)
		if (!strcmp(str, tbl[index].cs)) return index;
	return tbl_siz;
}

CCSID	cstoccsid(const char *codeset)
{
	uint_t	index;

	if (to_tbl == NULL) {
		(void)ccsid_open(NULL);
		if (to_tbl == NULL) return NULL;
	}

	index = find_idx(to_tbl, ccsid_hdr.to_tbl_cnt, codeset);
	return (index == ccsid_hdr.to_tbl_cnt)? 0: to_tbl[index].ccsid;
	
}
#else
CCSID	cstoccsid(const char *codeset)
{
	int	high, low, i, cmp_res;

	if (to_tbl == NULL) {
		(void)ccsid_open(NULL);
		if (to_tbl == NULL) return NULL;
	}

	low = 0;
	high = ccsid_hdr.to_tbl_cnt - 1;
	while (low <= high) {
		i = (low + high) >> 1;
		cmp_res = strcmp(codeset, to_tbl[i].cs);
		if (cmp_res < 0) high = i - 1;
		else if (cmp_res > 0) low = i + 1;
		else return to_tbl[i].ccsid;
	}
	return 0;
}
#endif
