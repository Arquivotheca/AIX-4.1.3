static char sccsid[] = "@(#)95	1.1  src/bos/usr/ccs/lib/libmi/tdb.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:39";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** tdb.c 2.2, last change 11/14/90
 **/


/* Text databases are flat file databases consisting of empty line seperated
** records.  The records consist of labelled fields, one to a line, e.g.:
**
**	name		wondermktg inc.
**	phone		1-800-555-HYPE
**	vp_of_sales	Jade Maxlie
**
**	phone		1-800-555-OHMM
**	comment		This little company is hopelessly out of touch.
**	name		One with the universe software
**	vp_of_rd	Nelson Naive
**
** As shown above, the creator of the database is free to add, omit, or
** reorder fields on a per record basis.
**
** All of the routines return a TDB pointer.  A nil return almost
** always means that a memory allocation failed somewhere.
** 
** These flat text files are 'compiled' into corresponding .tdb files
** by tdb_read() whenever their modification date is later than the
** .tdb file.
** The binary format looks like this:
**	TDB_MAGIC	(tdb0)
**	length of field record
**	lengths of fields
**	field names
**
**	length of value record
**	number of values
**	field value length array
**	field values
**	...
*/

#include <sys/stat.h>
#include <stdio.h>
#include <pse/common.h>
#include <sys/types.h>
#include <ctype.h>
#include <pse/q.h>	/**/
#include <pse/clib.h>
#include <pse/cpp.h>

/* Longest pathname possible */
#ifndef	MAX_PATH
#define	MAX_PATH	256
#endif

/* Largest buffer comfortably available as a stack array */
#ifndef	MAX_LBUF
#define	MAX_LBUF	1024
#endif

extern	char	* malloc(   unsigned size   );

#define	TDB_LAST	(-1)	/**/
#define	TDB_MAGIC	0x74626430		/* 't' 'd' 'b' '0' */

typedef struct tdb_s {	/**/
	Q	tdb_q;
	char	** tdb_field_names; /* shared between all records in the list*/
	char	** tdb_values;		/* private to this record */
} TDB;

#define	tdb_next	tdb_q.q_next	/**/
#define	tdb_prev	tdb_q.q_prev	/**/

/* # of bytes occupied in the file representation of canonical long */
#define	CANON_SIZE	4

TDB	* tdb_add_field();
TDB	* tdb_del_rec();
TDB	* tdb_read(), * tdb_read_rec();
TDB	* tdb_write(), * tdb_write_rec();

static char *
canon (ptr, val)
	char	* ptr;
	long	val;
{
reg	char	* cp1 = &ptr[CANON_SIZE];

	while (cp1 > ptr) {
		*--cp1 = (val & 0xFF);
		val >>= 8;
	}
	return ptr + CANON_SIZE;
}

static long
decanon (ptr)
	char	* ptr;
{
reg	long	val;

	val  = (long)(ptr[3] & 0xFF);
	if (ptr[2])
		val |= (long)(ptr[2] & 0xFF) << 8;
	if (ptr[1])
		val |= (long)(ptr[1] & 0xFF) << 16;
	if (ptr[0])
		val |= (long)(ptr[0] & 0xFF) << 24;
	return val;
}

static long
mod_date (file_name)
	char	* file_name;
{
	struct stat	stat1;

	if (stat(file_name, &stat1) < 0)
		return 0L;
	return stat1.st_mtime;
}

/* Add 'field' to all records in 'tdb' at 'position'.  If
** position == TDB_LAST field is inserted after all other fields.
** Nil is returned if memory is unavailable otherwise tdb is returned.
*/
TDB *
tdb_add_field (tdb, field, position)
reg	TDB	* tdb;
	char	* field;
	int	position;
{
	char	** new_names, ** old_names, ** values;
	TDB	* tdb_start;

	if (tdb_field_index(tdb, field) >= 0)
		return tdb;
	if (position == TDB_LAST) {
		position = cpp_len(tdb->tdb_field_names);
		if (position < 0)
			position = 0;
	}
	old_names = tdb->tdb_field_names;
	new_names = cpp_dup(old_names);
	if (!(new_names = cpp_ins(new_names, position, field)))
		return nilp(TDB);
	tdb_start = tdb;
	do {
		values = cpp_ins(tdb->tdb_values, position, "");
		if (!values) {
			while (tdb != tdb_start) {
				(void)cpp_del(tdb->tdb_values, position);
				tdb->tdb_field_names = old_names;
				tdb = Q_prv(TDB, tdb);
			}
			cpp_blast(new_names);
			return nilp(TDB);
		}
		tdb->tdb_field_names = new_names;
		tdb->tdb_values = values;
		tdb = Q_nxt(TDB, tdb);
	} while (tdb != tdb_start);
	cpp_blast(old_names);
	return tdb;
}

/* Alters the value of 'field' to 'new_field_value'.  If 'field'
** does not exist it is created.  The current value of 'field' is
** free()ed.  Tdb is returned unless memory is unavailable in which
** case nil is returned.
*/
TDB *
tdb_chg_field (tdb, field_name, new_field_value)
	TDB	* tdb;
	char	* field_name;
	char	* new_field_value;
{
		char	* cp1;
		int	position;

	position = tdb_field_index(tdb, field_name);
	if (position < 0) {
		(void)err_get_str();
		if (!tdb_add_field(tdb, field_name, TDB_LAST))
			return nilp(TDB);
		position = tdb_field_index(tdb, field_name);
	}
	if (!(cp1 = estrdup(new_field_value)))
		return nilp(TDB);
	if (!tdb->tdb_values) {
		err_set_str("tdb_chgfield: tdb_values is nil");
		free(cp1);
		return nilp(TDB);
	}
	if (tdb->tdb_values[position])
		free(tdb->tdb_values[position]);
	tdb->tdb_values[position] = cp1;
	return tdb;
}

TDB *
tdb_chg_value (tdb, field_name, new_field_value)
	TDB	* tdb;
	char	* field_name;
	char	* new_field_value;
{
	char	* cp1;
	int	i1;

	i1 = tdb_field_index(tdb, field_name);
	if (i1 < 0)
		return nilp(TDB);
	if (!(cp1 = estrdup(new_field_value)))
		return nilp(TDB);
	if (tdb->tdb_values[i1])
		free(tdb->tdb_values[i1]);
	tdb->tdb_values[i1] = cp1;
	return nilp(TDB);
}

TDB *
tdb_del (tdb)
	TDB	* tdb;
{
	while (tdb = tdb_del_rec(tdb))
		noop;
	return nilp(TDB);
}

TDB *
tdb_del_field (tdb, field_name)
reg	TDB	* tdb;
	char	* field_name;
{
	int	i1;
reg	char	* cpp;
	TDB	* tdb_start;

	i1 = tdb_field_index(tdb, field_name);
	if (i1 < 0)
		return nilp(TDB);
	cpp_del(tdb->tdb_field_names, i1);
	tdb_start = tdb;
	do {
		cpp_del(tdb->tdb_values, i1);
		tdb = Q_nxt(TDB, tdb);
	} while (tdb != tdb_start);
	return tdb;
}

TDB *
tdb_del_rec (tdb)
reg	TDB	* tdb;
{
	if (tdb) {
		cpp_blast(tdb->tdb_values);
		if (Q_empty(&tdb->tdb_q)) {
			cpp_blast(tdb->tdb_field_names);
			free(tdb);
			return nilp(TDB);
		}
		tdb = Q_nxt(TDB, tdb);
		free(q_out(Q_prv(Q, tdb)));
	}
	return tdb;
}

/* Create a duplicate of 'tdb'. If 'tdb' is nil then create an
** empty tdb.  If 'tdb' is non-nil create new field values but use
** the same field_names array.
*/
TDB *
tdb_dup_rec (orig)
reg	TDB	* orig;
{
	reg	TDB	* tdb;
		char	* empty_arr[1];

	if (!(tdb = Q_new(TDB)))
		return nilp(TDB);
	if (orig) {
		if (tdb->tdb_values = cpp_dup(orig->tdb_field_names))
			tdb->tdb_field_names = orig->tdb_field_names;
	} else {
		empty_arr[0] = nilp(char);
		tdb->tdb_values = cpp_dup(empty_arr);
		tdb->tdb_field_names = cpp_dup(empty_arr);
	}
	if (tdb->tdb_field_names  &&  tdb->tdb_values)
		return tdb;
	return tdb_del(tdb);
}

/* Create an entirely independent record for record copy of 'tdb'.  Alterations
** to the TDB returned will not alter the original tdb in any way.
*/
TDB *
tdb_dup (orig)
	TDB	* orig;
{
	reg	TDB	* runner;
		TDB	* tdb, * tdbp;

	if ((tdb = tdb_dup_rec(orig))  &&  orig) {
		if (!(tdb->tdb_field_names = cpp_dup(orig->tdb_field_names)))
			return tdb_del(tdb);
		for (runner = Q_nxt(TDB, orig); runner != orig; ) {
			if (!(tdbp = tdb_dup_rec(runner)))
				return tdb_del(tdb);
			tdbp->tdb_field_names = tdb->tdb_field_names;
			q_i_t((Q *)tdb, (Q *)tdbp);
			runner = Q_nxt(TDB, runner);
		}
	}
	return tdb;
}

int
tdb_field_index (tdb, field_name)
	TDB	* tdb;
	char	* field_name;
{
reg	char	* cp1, * cp2;
reg	char	** cpp;

	if (field_name  &&  (cpp = tdb->tdb_field_names)) {
		while (cp1 = *cpp) {
			cp2 = field_name;
			while (*cp1++ == *cp2++) {
				if (!cp1[-1])
					return cpp - tdb->tdb_field_names;
			}
			cpp++;
		}
	}
	return -1;
}

static char **
tdb_bcpp_read (fd)
	int	fd;
{
	char	* botptr;
	char	* buf;
	char	** cpp;
	char	* end_buf;
	int	entries;
	int	i1;
	long	l1, l2;
	int	len;
	char	lbuf[MAX_LBUF];
	char	* topptr;
	int	total_len;

	buf = &lbuf[0];
	if (read(fd, buf, (2 * CANON_SIZE)) != (2 * CANON_SIZE))
		return nilp(char *);
	total_len = l1 = decanon(&lbuf[0]);
	entries = l2 = decanon(&buf[CANON_SIZE]);
	if (total_len != l1
	||  total_len <= (2 * CANON_SIZE)
	||  entries != l2
	||  (entries + 1) <= 1)
		return nilp(char *);
	cpp = nilp(char *);
	total_len -= (2 * CANON_SIZE);
	if ((total_len > sizeof(lbuf)  &&  !(buf = newa(char, total_len)))
	||  read(fd, buf, total_len) != total_len
	|| !(cpp = newa(char *, entries + 1))) {
err:
		if (cpp) {
			for (i1 = 0; cpp[i1]; i1++)
				free(cpp[i1]);
			free(cpp);
		}
		if (buf != lbuf)
			free(buf);
		return nilp(char *);
	}
	topptr = buf;
	botptr = &topptr[entries * CANON_SIZE];
	end_buf = &buf[total_len];
	for (i1 = 0; i1 < entries; i1++) {
		len = l1 = decanon(topptr);
		if (len != l1)
			goto err;
		topptr += CANON_SIZE;
		if (len < 0  || (botptr + len) > end_buf)
			goto err;
		if (!(cpp[i1] = malloc(len)))
			goto err;
		memcpy(cpp[i1], botptr, len);
		botptr += len;
	}
	return cpp;
}

static boolean
tdb_bcpp_write (fd, cpp)
	int	fd;
	char	** cpp;
{
	char	* buf;
reg	char	* botptr;
reg	char	* cp1;
reg	char	** cpp1;
	long	fields;
	char	lbuf[MAX_LBUF];
	long	len;
	boolean	ret;
	char	* topptr;
	long	total_len;

	if (!cpp)
		return false;
	fields = 0L;
	total_len = (long)(CANON_SIZE + CANON_SIZE);
	for (cpp1 = cpp; cp1 = *cpp1++; ) {
		fields++;
		total_len += CANON_SIZE + strlen(cp1) + 1;
	}
	buf = &lbuf[0];
	if (total_len > sizeof(lbuf)) {
		if ((int)total_len != total_len
		|| !(buf = newa(char, (int)total_len)))
			return false;
	}
	topptr = buf;
	topptr = canon(topptr, total_len);
	topptr = canon(topptr, fields);
	botptr = &topptr[fields * CANON_SIZE];
	for (cpp1 = cpp; cp1 = *cpp1; cpp1++) {
		while (*botptr++ = *cp1++)
			noop;
		len = cp1 - *cpp1;
		topptr = canon(topptr, len);
	}
	cp1 = buf;
	while (total_len > 0) {
		if (total_len > (16 * 1024))
			len = write(fd, cp1, (16 * 1024));
		else
			len = write(fd, cp1, (int)total_len);
		if (len <= 0)
			break;
		cp1 += len;
		total_len -= len;
	}
	if (buf != lbuf)
		free(buf);
	return total_len == 0;
}

static TDB *
tdb_bread (file_name)
	char	* file_name;
{
	char	canon_buf[CANON_SIZE];
	int	fd;
	char	** field_names;
	char	lbuf[MAX_LBUF];
	TDB	* tdb, * tdb1;

	fd = open(file_name, 0);
	if (fd < 0)
		return nilp(TDB);
	tdb = nilp(TDB);
	if (read(fd, canon_buf, CANON_SIZE) == CANON_SIZE
	&&  decanon(canon_buf) == TDB_MAGIC
	&& (field_names = tdb_bcpp_read(fd))) {
		while (tdb1 = Q_new(TDB)) {
			if (!(tdb1->tdb_values = tdb_bcpp_read(fd))) {
				free(tdb1);
				break;
			}
			tdb1->tdb_field_names = field_names;
			if (tdb)
				q_i_t(tdb, tdb1);
			else
				tdb = tdb1;
		}
		if (!tdb)
			cpp_blast(field_names);
	}
	close(fd);
	return tdb;
}

static TDB *
tdb_bwrite (tdb, file_name)
reg	TDB	* tdb;
	char	* file_name;
{
	char	canon_buf[CANON_SIZE];
	int	fd;
	TDB	* tdb_start = tdb;

	if (!tdb  ||  !file_name) {
		err_set_str("tdb_write: nil pointer passed in");
		return nilp(TDB);
	}
	unlink(file_name);
#ifdef	VMS
	fd = creat(file_name, 0666, "alq=100", "fop=ctg,tef");
#else
	fd = creat(file_name, 0666);
#endif
	if (fd < 0) {
		err_set_str("tdb_bwrite: couldn't create binary file");
		return nilp(TDB);
	}
	canon(canon_buf, TDB_MAGIC);
	if (write(fd, canon_buf, CANON_SIZE) != CANON_SIZE) {
err:		err_set_str("tdb_bwrite: write failed");
		close(fd);
		unlink(file_name);
		return nilp(TDB);
	}
	if (!tdb_bcpp_write(fd, tdb->tdb_field_names))
		goto err;
	do {
		if (!tdb_bcpp_write(fd, tdb->tdb_values))
			goto err;
		tdb = Q_nxt(TDB, tdb);
	} while (tdb != tdb_start);
	close(fd);
	return tdb;
}

TDB *
tdb_read (file_name)
	char	* file_name;
{
	FILE	* fp;
	TDB	* tdb;
	boolean	is_err = false;

/* #ifdef OUTDEF */
	char	lbuf[MAX_PATH + 5];
	
	strcpy(lbuf, file_name);
	strcat(lbuf, ".tdb");
	if ((unsigned long)mod_date(file_name) <= (unsigned long)mod_date(lbuf)) {
		if (tdb = tdb_bread(lbuf))
			return tdb;
	}
/* #endif */
	if (!(fp = fopen(file_name, "r"))) {
		err_set_str("tdb_read: couldn't open '%s'", file_name);
		return nilp(TDB);
	}
	if (tdb = tdb_read_rec(nilp(TDB), fp, &is_err)) {
		while (tdb_read_rec(tdb, fp, &is_err)) {
			if (is_err) {
				err_set_str("tdb_read: out of memory");
				break;
			}
		}
	}
	fclose(fp);
	if (is_err)
		return tdb_del(tdb);
/* #ifdef OUTDEF */
	tdb_bwrite(tdb, lbuf);
/* #endif */
	return tdb;
}

TDB *
tdb_read_rec (tdb, fp, errp)
	TDB	* tdb;
	FILE	* fp;
	boolean	* errp;
{
		char	* buf;
	reg	char	* cp1;
		char	* cp2;
		int	field_cnt;
	reg	TDB	* tdbp;

	*errp = false;
	if (buf = newa(char, 4096)) {
		if (tdbp = tdb_dup_rec(tdb)) {
			char	** cpp;

			for (cpp = tdbp->tdb_values; *cpp; cpp++) {
				if (cp1 = strdup("")) {
					free(*cpp);
					*cpp = cp1;
				} else
					cpp[0][0] = '\0';
			}
			if (tdb)
				q_i_t(tdb, tdbp);
			field_cnt = 0;
			while (fgets(buf, 4096, fp)) {
				for (cp1 = buf; *cp1; cp1++) {
					if (*cp1 == ' '  ||  *cp1 == '\t') {
						do {
							*cp1++ = '\0';
						}while(*cp1==' ' ||*cp1=='\t');
						break;
					}
				}
				if (*cp1 == '\0') {	/* End of Record */
					if (field_cnt > 0)
						return tdb ? tdb : tdbp;
					continue;
				}
				if ((cp2 = strrchr(cp1, '\n'))  &&  !cp2[1])
					*cp2 = '\0';
				if (!tdb_chg_field(tdbp, buf, cp1))
					break;
				field_cnt++;
				if (field_cnt < 0) {	/* Overflow */
					*errp = true;
					break;
				}
			}
			free(buf);
			if (field_cnt > 0)
				return tdb ? tdb : tdbp;
			return tdb_del(q_out(tdbp));
		}
		free(buf);
	}
	*errp = true;
	return tdb;
}

TDB *
tdb_search (tdb, field_name, value_to_match)
	TDB	* tdb;
	char	* field_name;
	char	* value_to_match;
{
		char	* cp1, * cp2;
		boolean	matched;
		int	position;
		TDB	* tdb_start;
	
	position = tdb_field_index(tdb, field_name);
	if (position < 0)
		return nilp(TDB);
	tdb_start = tdb;
	do {
		if (cp1 = tdb->tdb_values[position]) {
			do {
				if (cp2 = strchr(cp1, '|'))
					*cp2 = '\0';
				matched = !strcmp(cp1, value_to_match);
				if (cp2)
					*cp2++ = '|';
				if (matched)
					return tdb;
			} while (cp1 = cp2);
		}
		tdb = (TDB *)tdb->tdb_next;
	} while (tdb != tdb_start);
	return nilp(TDB);
}

TDB *
tdb_write_rec (tdb, fp)
	TDB	* tdb;
	FILE	* fp;
{
	reg	char	** values;
	reg	char	** names;
		boolean	some_fields_output = false;

	if ((values = tdb->tdb_values)  &&  (names = tdb->tdb_field_names)) {
		for ( ; *names; names++) {
			if (values[0]  &&  values[0][0]) {
				if (fprintf(fp, "%s\t%s\n", *names, *values)<0)
					return nilp(TDB);
				some_fields_output = true;
			}
			values++;
		}
	}
	if (some_fields_output)		/* seperate us from the next record */
		fprintf(fp, "\n");
	return tdb;
}

#ifdef DEBUG
tdb_printf (tdb)
	TDB	* tdb;
{
	TDB	* tdb1;

	cpp_printf("field_names", tdb->tdb_field_names);
	tdb1 = tdb;
	do {
		printf("values[%d]:\n", tdb1-tdb);
		cpp_printf("\t", tdb1->tdb_values);
		tdb1 = tdb1->tdb_next;
	} while (tdb1 != tdb);
}

cpp_printf (prefix, cpp)
	char	* prefix;
	char	** cpp;
{
	char	** cpp1;

	if (!cpp) {
		printf("%s empty\n", prefix);
		return;
	}
	for (cpp1 = cpp; *cpp1; cpp1++)
		printf("%s[%d] == '%s'\n", prefix, cpp1-cpp, *cpp1);
}
#endif
