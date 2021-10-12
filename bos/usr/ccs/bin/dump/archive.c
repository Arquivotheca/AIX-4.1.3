static char sccsid[] = "@(#)53  1.17  src/bos/usr/ccs/bin/dump/archive.c, cmdaout, bos41B, 9504A 1/16/95 16:01:29";

/*
 * COMPONENT_NAME: CMDAOUT (dump command)
 *
 * FUNCTIONS: dump_ar_globals, dump_archive, dump_member
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <IN/ARdefs.h>
#include "dump_defs.h"
#include <time.h>

extern nl_catd catd;
extern void free();
extern long sgetl();
extern char *dump_alloc();

extern int Ar_globals, Ar_member, Print_header;
extern char *Current_file, *Member_name;
extern int Obj_flag;

int	Init_member;
int	At_sym_tab;
int	Ar_fmt;
long	Member_size;

/*
 * dump_ar_globals()	- dump global symbol table of an archive
 *
 *	the contents of the symbol table are:
 *		number of symbols (4 bytes)
 *		array of offsets into the archive (4 bytes * # of symbols)
 *		name string table (ar_size - (4 bytes * (# of symbols + 1)))
 *
 *	Each offset from the array associates with the corresponding name from
 *	the string table, in order.
 *
 *	This routine has to assume that the archive isn't corrupted.
 */
dump_ar_globals(filep, size)
	FILE *filep;
	long size;
{
	register char	*next_offset;
	register int	c;
	register long	num_syms,
			sym_off,
			save_pos,
			num_mems;
	char		buf[sizeof(long)];
	void		*offsets;

	(void) putchar('\n');
	output(Current_file);
	title(MSGSTR(AR_SYMTAB_MSG, AR_SYMTAB));
	(void) printf(MSGSTR(AR_SYM_TITLE_MSG, AR_SYM_TITLE));

	/* read number of symbols */
	switch(Ar_fmt)
	{
		case AIAF_AR_ID:
			if (!fread_OK(buf, sizeof(long), filep))
				return;
			break;
		default:
			break;
	}
	num_syms = sgetl(buf);

	/* Read in the array of offsets and position to symbol names */
	offsets = dump_alloc(sizeof(long) * (unsigned int) num_syms);
	switch(Ar_fmt)
	{
		case AIAF_AR_ID:
			if (fread(offsets, (size_t) sizeof(long), (size_t) num_syms, filep)
			!= num_syms)
				error(MSGSTR(READ_ERR_MSG, READ_ERR));
			size -= (sizeof(long) * (num_syms + 1));
			break;
		default:
			return;
	}
			

	for (next_offset = offsets; num_syms-- > 0; next_offset += sizeof(long))
	{
		(void) printf("%10ld\t", sgetl(next_offset));

		while (size-- > 0 && (c = getc(filep)) != '\0' && c != EOF)
			(void) putchar(c);

		(void) putchar('\n');

		if (c == EOF)
		{
			error(MSGSTR(READ_ERR_MSG, READ_ERR));
			break;
		}
	}

	(void) putchar('\n');
	free(offsets);
}

/*
 * dump_archive()	- dump archive files
 */
void
dump_archive(filep)
	FILE *filep;
{
	static int	in_archive = 0;
	extern int dump_member();

	/* do not dump an archive inside of an archive */
	if (in_archive)
		return;

	if (!(Ar_fmt = ARisarchive(filep)) || (Ar_fmt != AIAF_AR_ID))
	{
		error(MSGSTR(NOT_AN_OBJ_MSG, NOT_AN_OBJ));
		return;
	}

	in_archive = 1;
	Init_member = 1;
	At_sym_tab = 0;
	(void) ARforeach(filep, dump_member);
	Member_name = NULL;
	in_archive = 0;
}

/*
 * dump_member()	- process archive member
 */
dump_member(ar)
	ARparm *ar;
{
	struct	tm *nlsdate;
	char	nls_buf[NLTBMAX];

	Member_name = ar->name;
	Member_size = ar->size;

	if (*ar->name == '\0')
	{
		if ((Ar_fmt == AIAF_AR_ID) && !At_sym_tab)
		{
			At_sym_tab++;
			return(0);
		}
		else
		{
			if (Ar_globals)
				dump_ar_globals(ar->file, ar->size);

			/* return 1 if Ar_globals was only significant
			 * flag specified
			 */
			return(!Obj_flag && !Ar_member);
		}
	}

	if (Obj_flag)
		pr_name();

	if (Ar_member)	/* dump archive header of each member */
	{
		if (Print_header && (Obj_flag || Init_member))
		{
			title(MSGSTR(AR_HEADER_MSG, AR_HEADER));
			(void) printf(MSGSTR(AR_HD_TITLE_MSG, AR_HD_TITLE));
		}

		nlsdate = localtime(&ar->date);
		(void)strftime(nls_buf, (size_t)NLTBMAX, "%sD %X %Y", nlsdate);
		(void)printf("%-14.14s %20s %10ld %10ld   %#7.6lo  0x%.8lx\n",
			ar->name, nls_buf, ar->uid, ar->gid, ar->mode,
			ar->size);
	}

	if (Obj_flag)
		process_file(ar->file);

	Member_name = NULL;
	Init_member = 0;

	return(0);
}
