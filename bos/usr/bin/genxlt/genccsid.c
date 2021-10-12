static char sccsid[] = "@(#)81  1.1  src/bos/usr/bin/genxlt/genccsid.c, cmdiconv, bos411, 9428A410j 12/7/93 13:43:49";
/*
 *   COMPONENT_NAME:    CMDICONV
 *
 *   FUNCTIONS:         main
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <locale.h>
#include <nl_types.h>

#include "ccsid.h"
#include "genxlt_msg.h"

/*
 *	IMPORTANT !
 *	This module is used as a sub-routine of the genxlt command
 *	to generate a ccsid<->cs name table for temporary solution
 *	of the libiconv ccsid function enhancement.  We should add
 *	a new command (e.g. genccsid)  to generate ccsid table and
 *	remove this function from the genxlt command in the future,
 *	because the ccsid table has completely defferent structure
 *	from the iconv conversion table.
 *
nl_catd         catd;
 */

#define MSGSTR(Num, Str) catgets(catd, MS_genxlt, Num, Str)
#define	ENTRY_UNIT	128

/*
 *   NAME:	_swap_rec
 *
 *   FUNCTION:
 *
 *   RETURNS:	None
 */

static	void		_swap_rec (
	ccsid_data_t	a,
	int		k,
	int		i) {

	ccsid_data_rec	tmp_rec;

	(void)memcpy (&tmp_rec, &a[k], sizeof (ccsid_data_rec));
	(void)memcpy (&a[k], &a[i],    sizeof (ccsid_data_rec));
	(void)memcpy (&a[i], &tmp_rec, sizeof (ccsid_data_rec));
	return;
}

/*
 *   NAME:      _sort_ccsid
 *
 *   FUNCTION:
 *
 *   RETURNS:   None
 */

static	void		_sort_ccsid (
	ccsid_data_t	tbl,
	int		lhs,
	int		rhs) {

	int		idx, tail;

	if (lhs >= rhs) return;
	(void)_swap_rec (tbl, lhs, (lhs + rhs) >> 1);
	tail = lhs;
	for (idx = tail + 1; idx <= rhs; idx++)
		if (tbl[idx].ccsid < tbl[lhs].ccsid)
			(void)_swap_rec(tbl, ++tail, idx);
	(void)_swap_rec (tbl, lhs, tail);
	(void)_sort_ccsid (tbl, lhs, tail - 1);
	(void)_sort_ccsid (tbl, tail + 1, rhs);
	return;
}

/*
 *   NAME:      _sort_sequencial
 *
 *   FUNCTION:
 *
 *   RETURNS:   None
 */

static	void		_sort_sequencial (
	ccsid_data_t	f_tbl,
	ccsid_data_t	tbl_buf,
	int		cnt) {

	(void)memcpy (f_tbl, tbl_buf, cnt * sizeof (ccsid_data_rec));
	(void)_sort_ccsid (f_tbl, 0, cnt - 1);
	return;
}


#ifdef CCSID_HASH
static	uint_t		_find_vacant_idx (
	ccsid_data_t	tbl,
	uint_t		tbl_siz,
	char		*str) {

	uint_t		index, hash_val;

	index = hash_val = ccsid_hash(str, tbl_siz);
	for (; index < tbl_siz; index++)
		if (tbl[index].cs[0] == '\0') return index;
	for (index = 0; index < hash_val; index++)
		if (tbl[index].cs[0] == '\0') return index;
	return tbl_siz;
}

static	void		_sort_hash (
	ccsid_data_t	t_tbl,
	ccsid_data_t	tbl_buf,
	int		entry_cnt,
	int		tbl_siz) {

	uint_t	idx;
	int	i, j;

	for (i = 0; i < entry_cnt; i++) {
		if (tbl_buf[i].ccsid == 0) continue;
		for (j = i + 1; j < entry_cnt; j++) {
			if (tbl_buf[j].ccsid == 0) continue;
			if (!strcmp(tbl_buf[i].cs, tbl_buf[j].cs))
				tbl_buf[j].ccsid = 0;
		}
		idx = _find_vacant_idx(t_tbl, tbl_siz, tbl_buf[i].cs);
		t_tbl[idx] = tbl_buf[i];
	}
	return;
}

static	int	_tsiz_tbl[] = {

	  3,   5,   7,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43, 
	 47,  53,  59,  61,  67,  71,  73,  79,  83,  89,  97, 101, 103, 
	107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 
	179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 
	251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 
	331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 
	409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 
	487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 
	577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 
	653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 
	743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 
	829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 
	929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997 
};
#else /*CCSID_HASH*/

/*
 *   NAME:      _sort_cs
 *
 *   FUNCTION:
 *
 *   RETURNS:   None
 */

static	void		_sort_cs (
	ccsid_data_t	tbl,
	int		lhs,
	int		rhs) {

	int		idx, tail;

	if (lhs >= rhs) return;
	(void)_swap_rec (tbl, lhs, (lhs + rhs) >> 1);
	tail = lhs;
	for (idx = tail + 1; idx <= rhs; idx++)
		if (strcmp (tbl[idx].cs, tbl[lhs].cs) < 0)
			(void)_swap_rec (tbl, ++tail, idx);
	(void)_swap_rec (tbl, lhs, tail);
	(void)_sort_cs (tbl, lhs, tail - 1);
	(void)_sort_cs (tbl, tail + 1, rhs);
	return;
}

/*
 *   NAME:      _sort_by_cs
 *
 *   FUNCTION:
 *
 *   RETURNS:   None
 */

static	void		_sort_by_cs (
	ccsid_data_t	t_tbl,
	ccsid_data_t	tbl_buf,
	int		cnt,
	int		*t_cnt) {

	uint_t		idx = 0;
	int		i, j;

	for (i = 0; i < cnt; i++) {
		if (tbl_buf[i].ccsid == 0) continue;
		for (j = i + 1; j < cnt; j++) {
			if (tbl_buf[j].ccsid == 0) continue;
			if (!strcmp(tbl_buf[i].cs, tbl_buf[j].cs))
				tbl_buf[j].ccsid = 0;
		}
		t_tbl[idx++] = tbl_buf[i];
	}
	(void)_sort_cs (t_tbl, 0, idx - 1);
	*t_cnt = idx;
	return;
}
#endif

/*
 *	MAIN
 */

int	genccsid (nl_catd  catd) {

	char		lbuf[LINE_MAX];
	int		line, in_cnt, f_cnt, t_cnt, tsiz_cnt, alloc_cnt, i;
	ccsid_hdr_rec   hdr;
	ccsid_data_t    tbl_buf, f_tbl, t_tbl;

/*
	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_GENCCSID, NL_CAT_LOCALE);
*/

	/*
	 *	Read one line at a time and check if it's valid
	 */

	alloc_cnt = ENTRY_UNIT * 4;
	tbl_buf = (ccsid_data_t)malloc (alloc_cnt * sizeof (ccsid_data_rec));
	if (tbl_buf == NULL) {
		fprintf (stderr, MSGSTR (M_MSG_7, "Not enough memory.\n"));
		return 1;
	}
	for (line = in_cnt = 0; gets(lbuf) != NULL; line++) {
		if (in_cnt > alloc_cnt) {
			alloc_cnt += ENTRY_UNIT;
			tbl_buf = realloc(tbl_buf,
					alloc_cnt * sizeof (ccsid_data_rec));
			if (tbl_buf == NULL) {
				fprintf (stderr, MSGSTR (M_MSG_7, "Not enough memory\n"));
				return 1;
			}
		}
		if ((*lbuf == '#') || (*lbuf == '\0')) {
			continue;	/* Comment or blank line */
		}
		if (sscanf (lbuf, "%s %u", tbl_buf[in_cnt].cs,
					 &(tbl_buf[in_cnt].ccsid)) != 2) {
			fprintf (stderr, MSGSTR (M_MSG_5,
				"Invalid format at line %d\n"), line);
			return 1;
		}
		in_cnt ++;
	}

	/*
	 *	Create header information
	 */

	hdr.version = 0;
	for(i = 0; i < 20; i++) hdr.reserve[i] = '\0';
	hdr.from_tbl_cnt = f_cnt = in_cnt;
#ifdef CCSID_HASH
	for (t_cnt = 1; in_cnt >>= 1; t_cnt <<= 1);
	tsiz_cnt = sizeof(_tsiz_tbl)/sizeof(_tsiz_tbl[0]);
	for (; in_cnt < tsiz_cnt; in_cnt++)
		if (t_cnt < _tsiz_tbl[in_cnt]) {
			t_cnt = _tsiz_tbl[in_cnt];
			break;
		}
	hdr.to_tbl_cnt = t_cnt;
#else
	t_cnt = in_cnt;
#endif

	/*
	 *	Allocate memory for data section.
	 *	(use calloc to zero clear them)
	 */

	f_tbl = (ccsid_data_t)calloc (f_cnt, sizeof (ccsid_data_rec));
	t_tbl = (ccsid_data_t)calloc (t_cnt, sizeof (ccsid_data_rec));
	if (!(f_tbl && t_tbl)) {
		fprintf (stderr, MSGSTR (M_MSG_7, "Not enough memory\n"));
		return 1;
	}
	
	/*
	 *	Create data section.
	 */

	(void)_sort_sequencial (f_tbl, tbl_buf, f_cnt);
#ifdef CCSID_HASH
	(void)_sort_hash (t_tbl, tbl_buf, f_cnt, t_cnt);
#else
	(void)_sort_by_cs (t_tbl, tbl_buf, f_cnt, &t_cnt);
	hdr.to_tbl_cnt = t_cnt;
#endif

	/*
	 *	Output table.
	 */

	if ((fwrite(&hdr, sizeof(hdr), 1, stdout)) != 1 ||
	    (fwrite (f_tbl, sizeof(ccsid_data_rec), f_cnt, stdout)) != f_cnt ||
	    (fwrite (t_tbl, sizeof(ccsid_data_rec), t_cnt, stdout)) != t_cnt) {
		fprintf (stderr, MSGSTR (M_MSG_4,
			"Unable to write for target file.\n"));
		return 1;
	}
	return 0;
}
