static char sccsid[] = "@(#)54  1.20  src/bos/usr/ccs/bin/dump/dump.c, cmdaout, bos411, 9428A410j 3/10/94 10:11:59";

/*
 * COMPONENT_NAME: CMDAOUT (dump command)
 *
 * FUNCTIONS: dump_alloc, error, fread_OK, fseek_OK,
 *	hex_dump, output, pr_name, process_file, title, usage
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <a.out.h>
#include "dump_defs.h"
#include <locale.h>

nl_catd catd;

extern void dump_archive(), dump_xcoff();

char * dump_alloc(unsigned int);
int  fread_OK(void *, size_t, FILE *),	fseek_OK(FILE *, long);
void error(char *),	output(char *),	pr_name(void),	process_file(FILE *), 
     usage(void),	title(char *),
     hex_dump(char *, FILE *, long, long);

struct obj_format {
	char		*f_name;
	unsigned char	f_magic1;
	unsigned char	f_magic2;
	void		(*f_func)();
} formats[] = {
	{ "XCOFF",	0x01,	0xd8,	dump_xcoff	},
	{ "XCOFF",	0x01,	0xdd,	dump_xcoff	},
	{ "XCOFF",	0x01,	0xdf,	dump_xcoff	},
	{ "XCOFF",	0x01,	0x98,	dump_xcoff	},
	{ "XCOFF",	0x01,	0x9d,	dump_xcoff	},
	{ "XCOFF",	0x01,	0x9f,	dump_xcoff	},
	{ "ARCHIVE",	'>',	'a',	dump_archive	},
	{ "ARCHIVE",	'<',	'a',	dump_archive	},
	{ NULL,		'\0',	'\0',	(void (*)()) 0	}
};

/* flags that can be modified by command line arguments */
char	*Zname = NULL;
int	Ar_globals = 0,
	Ar_member = 0,
	Binder_sect = 0,
	Data_sect = 0,
	Ex_header = 0,
	Head_sect = 0,
	Loader_head = 0,
	Loader_reloc = 0,
	Loader_sect = 0,
	Loader_symt = 0,
	Line_nums = 0,
	Obj_sect = 0,
	Opt_header = 0,
	Print_header = 1,
	Reloc = 0,
	Str_table = 0,
	Sym_end = 0,
	Sym_start = 0,
	Sym_table = 0,
	Symbolic = 0,
	Underline = 0,
	Zend = 0,
	Zstart = 0;

char	*Current_file,		/* current object file being processed */
	*Member_name = NULL;	/* name of archive member being processed */
int	Obj_flag;
long	File_origin;
int	status = 0;		/* Overall return code for dump command */

main(int argc, char *argv[])
{
	register char	*p;
	register int	c;
	FILE		*filep;
	extern char	*optarg;
	extern int	optind;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_DUMP, NL_CAT_LOCALE);
	/* modify the command line arguments so getopt() can be used.
	 *	change all '+' flags to '-' and change letter to upper case
	 *	change -t to -+ if numeric option follows.
	 */
	for (c = 1; c < argc; ++c)
	{
		p = argv[c];

		if (p[0] == '-' && p[1] == 't')
		{
			if (isdigit((int)p[2]) ||
			(p[2] == '\0' && isdigit((int)*argv[c+1])))
				p[1] = '+';	/* change -t<num> to -+<num> */
		}
		else if (p[0] == '+' && p[1] == 't')
		{
			if (isdigit((int)p[2]) ||
			(p[2] == '\0' && isdigit((int)*argv[c+1])))
				p[0] = '-';	/* change +t<num> to -E<num> */
				p[1] = 'E';
		}
		else if (p[0] == '+' && p[1] == 'z') 
		{
			p[1] = 'Z';
			p[0] = '-';
		}
	}

	/* let getopt() parse the flags */
	while ((c = getopt(argc, argv, "acdghlnoprstHPRTuvz:E:Z:+:")) != EOF)
	{
		switch (c)
		{
		  case 'a':	/* dump archive header of each archive member */
			++Ar_member;
			break;

		  case 'c':	/* dump the string table */
			++Str_table;
			break;

		  case 'd':	/* dump the contents of the data section */
			++Data_sect;
			break;

		  case 'g':	/* dump global symbols in archive sym table */
			++Ar_globals;
			break;

		  case 'h':
			++Head_sect;
			break;

		  case 'l':	/* dump line number information */
			++Line_nums;
			break;

		  case 'n':	/* dump the loader section */
			++Loader_sect;
			break;

		  case 'o':	/* dump the main header + each opt header */
			++Opt_header;
			break;

		  case 'p':	/* do not print headers */
			Print_header = 0;
			break;

		  case 'r':	/* dump relocation information */
			++Reloc;
			break;

		  case 's':	/* dump object file section contents */
			++Obj_sect;
			break;

		  case 't':	/* dump symbol table entries */
			++Sym_table;
			break;

		  case 'u':	/* underline the name of the file */
			++Underline;
			break;

		  case 'v':	/* dump in symbolic form (not numeric) */
			++Symbolic;
			break;

		  case 'z':	/* dump line number entries for name */
			++Line_nums;
			Zname = optarg;
			if ((p = strchr(optarg, ',')) != NULL)
			{
				*(p++) = '\0';
				Zstart = atoi(p);
			}
			break;

		  case 'E':	/* dump only specified symbol table entry */
			Sym_end = atoi(optarg);
			Sym_table++;
			break;

		  case 'H':	/* dump the header of the loader sect */
			++Loader_head;
			break;

		  case 'R':	/* dump relocation info for loader sect) */
			++Loader_reloc;
			break;

		  case 'T':	/* dump sym table entries for loader */
			++Loader_symt;
			break;

		  case 'Z': 	/* last line number entry to dump */
			Zend = atoi(optarg);
			break;

		  case '+':	/* dump only specified symbol table entry */
			Sym_start = atoi(optarg);
			Sym_table++;
			break;

		  default:	/* modified flags will have erroneous message */
			usage();
			/*NOTREACHED*/
		}
	}

	Obj_flag = Str_table | Data_sect | Opt_header | Line_nums
			| Reloc | Obj_sect | Sym_table | Ex_header
			| Binder_sect | Loader_sect | Loader_head
			| Loader_reloc | Loader_symt | Head_sect;

	if (!Obj_flag && !Ar_globals && !Ar_member)
	{
		(void) fprintf(stderr, MSGSTR(DUMP_ONE_MSG, DUMP_ONE));
		usage();
	}

	if (optind >= argc)
		usage();

	for (; optind < argc; ++optind)
	{
		Current_file = argv[optind];

		if ((filep = fopen(Current_file, "r")) == NULL)
		{
			error(MSGSTR(CANT_OPEN_MSG, CANT_OPEN));
			continue;
		}

		process_file(filep);
		(void) fclose(filep);
	}

	exit(status);
	/*NOTREACHED*/
}

/*
 * dump_alloc()	- allocate space while checking return value
 */
char * dump_alloc(unsigned int size)
{
	register char *ptr = malloc(size);

	if (ptr == NULL)
	{
		error(MSGSTR(NO_MEM_MSG, NO_MEM));
		exit(1);
	}

	return(ptr);
}

/*
 * error()	- print message with program name and current filename
 */
void error(char *message)
{
	extern int errno;

	if (Member_name && *Member_name)
		(void) fprintf(stderr, "dump: %s[%s]: ",
				Current_file, Member_name);
	else
		(void) fprintf(stderr, "dump: %s: ", Current_file);

	if (errno != 0)
	{
		errno = 0;
	}
	(void) fprintf(stderr, "%s", message);
	status = 2;
}

int fread_OK(void *ptr, size_t nitems, FILE *stream)
{
	if (fread(ptr, (size_t)1, nitems, stream) == nitems)
		return(1);

	error(MSGSTR(READ_ERR_MSG, READ_ERR));
	return(0);
}

int fseek_OK(FILE *stream, long offset)
{
	if (fseek(stream, offset, 0) == 0)
		return(1);

	error(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
	return(0);
}


void hex_dump(char *prompt, FILE *fp, long offset, long length)
{
	register int	c,
			i;
	register long	l,
			o;
	int		inc = 32,
			j;

	output(prompt);

	if (!fseek_OK(fp, File_origin + offset))
		return;

	for (o = 0, l = length; l > 0; l -= inc, o += inc)
	{
		(void) printf("%5lx: ", o);
		j = (l > inc) ? inc : l;

		for (i = 0; i < j; ++i)
		{
			if ((c = getc(fp)) == EOF)
			{
				error(MSGSTR(READ_ERR_MSG, READ_ERR));
				return;
			}

			(void) printf("%02X", c & 0xFF);

			if ((i % 4) == 3)
				(void) putchar(' ');
		}

		(void) putchar('\n');
	}
}

/*
 * output()	- output a string possibly underlining it
 */
void output(char *string)
{
	register char *p;

	if (Underline)
	{
		for (p = string; *p; ++p)
			(void) printf("_\b%c", *p);
	}
	else
		(void) printf(string);

	(void) printf(":\n");
}

/*
 * pr_name()	- print filename
 */
void pr_name(void)
{
	char	buf[256];

	(void) putchar('\n');
	if (Member_name)
		(void) sprintf(buf, "%s[%s]", Current_file, Member_name);
	else
		(void) sprintf(buf, "%s", Current_file);

	output(buf);
}

void process_file(FILE *fp)
{
	register struct obj_format *p;
	int	magic1,
		magic2;

	File_origin = ftell(fp);

	if ((magic1 = getc(fp)) == EOF || (magic2 = getc(fp)) == EOF)
	{
		error(MSGSTR(NOT_AN_OBJ_MSG, NOT_AN_OBJ));
		return;
	}

	(void) fseek(fp, (long)-2, (int)1);

	for (p = formats; p->f_name != NULL; ++p)
	{
		if (magic1 == p->f_magic1 && magic2 == p->f_magic2)
			break;
	}

	if (p->f_name == NULL)
	{
		error(MSGSTR(NOT_AN_OBJ_MSG, NOT_AN_OBJ));
		return;
	}

	(p->f_func)(fp);
}

void title(char *string)
{
	(void) printf("\n\t\t\t***%s***\n", string);
}

/*
 * usage()	- show how this program was intended to be used
 */
void usage(void)
{
	(void) fprintf(stderr, MSGSTR(USAGE_MSG, USAGE));
	exit(1);
}
