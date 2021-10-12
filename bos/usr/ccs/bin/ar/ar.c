static char sccsid[] = "@(#)41  1.49  src/bos/usr/ccs/bin/ar/ar.c, cmdar, bos41J, 9514A_all 3/31/95 16:09:39";

/*
 * COMPONENT_NAME: CMDAR (ar command)
 *
 * FUNCTIONS: add_file_mem, add_free_mem, add_sym_mem, ar_error, ar_exit,
 *	ar_fclear, ar_fread, ar_fseek, ar_ftruncate, ar_fwrite,
 *	ar_malloc, ar_fprintf, ar_realloc, asc_bin_hdr, bin_asc_hdr,
 *	clear_free_list, copy_file, create_ar_file, create_mem_sym_tab,
 *	extract_file_mem, find_free_pos, find_mem_pos, get_ar_hdr,
 *	get_free_lst, get_mem_index, get_mem_sym_tab, list_file_mem,
 *	list_sym_tab, mov_file_mem, order_ar_file, process_mem_list,
 *	read_fixed_hdr, replace_file_mem, rmv_file_mem, rmv_sym_mem,
 *	safe_copy, set_mem_time, set_mod_pos, trim, unlink_file_mem,
 *	update_offset, write_file_mem, write_file_mode, write_fixed_hdr,
 *	write_mem_sym_error, write_mem_sym_tab, zero_fixed_hdr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <values.h>
#include <time.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/errno.h>
#ifdef _STD_ARG
#include <stdarg.h>
#else /* _STD_ARG */
#include <varargs.h>
#endif /* _STD_ARG */
#include <xcoff.h>
#include <ar.h>
#include <stdlib.h>

#include <nl_types.h>
#include "ar_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_AR, Num, Str)

#define ROUNDUP(x)	(((x) + 1) & ~1)
#define flg		(&FLG[-'a'])
#define MAX(x, y)	((x) < (y) ? (y) : (x))

#ifdef	LIBCIO_FIX
#define	ONE_K		1024
#endif	/* LIBCIO_FIX */

#define BUMP_VAL	256
#define COPY_IN		0x56
#define COPY_OUT	0x57
#define CREATE		0x33
#define HDR_RDSIZE	AR_HSZ + 256
#define MIN_RDSIZE	AR_HSZ + 14
#define NO_CREATE	0x34
#define P_ATEND		-1
#define	P_BEFORE	0
#define	P_AFTER		1
#define P_REPLACE	2

#define EXEC_MODE	'x'
#define NULL_MODE	'-'
#define READ_MODE	'r'
#define SGID_MODE	'g'
#define SUID_MODE	's'
#define SVTXT_MODE	't'
#define WRITE_MODE	'w'

#define	AR_CREATING	"ar: creating an archive file %s"
#define AR_NOEXIST	"ar: 0707-100 %s does not exist."
#define AR_USAGE	"Usage:\
\tar [-closvCT] {-h|p|t|x} [--] Archive [File ...]\
\n\tar [-closvCT] {-m|r[u]} [{-a|b|i} {PositionName}] [--] Archive File ...\
\n\tar [-closvCT] {-d|q} [--] Archive File ...\
\n\tar [-clvCT] {-o|s|w} [--] Archive"
#define BAD_OPTION	"ar: 0707-101 %c is not a valid flag."
#define BAD_HEADER	"ar: 0707-102 Internal error while reading \
member header."
#define CHAR_STRING	"%s"
#define COPY_ERR	"ar: 0707-103 Internal error while copying \
archive member."
#define	CORRUPT_OBJ	"ar: 0707-124 The object file %s is corrupted.\n\
\tSymbol table entries for %s can not be added \
to the archive file."
#define FCLEAR_ERR	"ar: 0707-104 The fclear system call failed."
#define FFLUSH_ERR	"ar: 0707-105 The fflush system call failed."
#define FIXHDR_ERR	"ar: 0707-106 Internal error while reading \
the fixed\n\theader of archive file %s."
#define FSLIMIT_ERR	"ar: 0707-107 The file size limit was reached when \
writing to the archive file.\n\
Use the ulimit command to increase the maximum user file size."
#define MAGIC_ERR	"ar: 0707-108 File %s is not an archive file."
#define MEM_NOTFOUND	"ar: 0707-109 Member name %s does not exist."
#define NO_MEM		"ar: 0707-110 There is not enough memory available \
now."
#define NO_MEM_TAB	"ar: 0707-123 The archive file %s is corrupted.\n\
\tThe member table is missing.\n\
\tThe command 'ar -o %s' may restore the member table"
#define NO_Q_POS	"ar: 0707-111 The -a, -b, or -i flag cannot be used \
with the -d or -q flag."
#define NO_SYM_TAB	"ar: 0707-112 There is no symbol table in %s.\n\
\tRecreate the symbol table by using the ar command with the -s flag."
#define OPEN_ERR	"ar: 0707-113 The fopen system call failed on file %s."
#define ORDERING_AR	"ar: sequentially ordering and compressing %s"
#define READ_ERR	"ar: 0707-114 The fread system call failed."
#define SAFE_ERR	"ar: 0707-115 Internal error while copying the /tmp \
file\n\tto archive file %1$s.\n\
\tA valid copy was saved in temporary file space %2$s.\n\
\tMake a copy of this file immediately."
#define SEEK_ERR	"ar: 0707-116 The fseek library call failed."
#define STAT_ERR	"ar: 0707-117 The fopen system call failed on file %s."
#define SYM_RDERROR	"ar: 0707-118 Internal error while reading the \
Global Symbol Table."
#define SYM_WRERROR	"ar: 0707-119 Internal error while writing \
to the Global Symbol Table."
#define TOO_MANY_CMDS	"ar: 0707-120 The specified flags are mutually \
exclusive.\n"
#define TRUNCATE_ERR	"ar: 0707-121 The ftruncate system call failed."
#define VERBOSE_CMD	"%c - %s"
#define WRITE_ERR	"ar: 0707-122 The fwrite system call failed."
#define EXTRACT_ERR	"ar: 0707-125 Cannot write over file %s when \
-C is used."

struct bin_hdr
{
	long	bh_size;
	long	bh_nxtmem;
	long	bh_prvmem;
	long	bh_date;
	long	bh_uid;
	long	bh_gid;
	unsigned long bh_mode;
	int	bh_namlen;
	char	*bh_name;
};

struct sym_item
{
	long	si_offset;
	union
	{
		char	*si_name;
		long	si_size;
	}	_si_data;
};

extern int		errno;

extern int		add_file_mem();

extern char		*ar_malloc(),			*ar_realloc(),
			*memcpy(), 			*trim(),
			*strcpy(),			 *strncpy();

extern long		atol(),
			find_free_pos(),		find_mem_pos(),
			sgetl(),			strtol(),
			fclear(),			get_mem_index();

extern off_t		lseek(),			ulimit();

extern void		create_mem_sym_tab(),
			mov_file_mem(),			rmv_file_mem(),
			add_free_mem(),			replace_file_mem(),
			unlink_file_mem(),		process_mem_list(),
			list_file_mem(),		extract_file_mem(),
			write_file_mem(),		list_sym_tab(),
			set_mem_time(),			set_mod_pos(),
			rmv_sym_mem(),			write_mem_sym_error(),
			get_mem_sym_tab(),		get_free_lst(),
			bin_asc_hdr(),			asc_bin_hdr(),
			get_ar_hdr(),			write_fixed_hdr(),
			write_mem_sym_tab(),		copy_file(),
			ar_fseek(),			ar_fread(),
			ar_fwrite(),			update_offset(),
			zero_fixed_hdr(),		ar_ftruncate(),
			ar_fclear(),			write_file_mode(),
			order_ar_file(),		clear_free_list(),
			add_sym_mem(),			exit();
void ar_exit(char *fmt, ...);
void ar_fprintf(FILE *s, char *fmt, ...);
void ar_error(char *fmt, ...);

			
extern FILE		*create_ar_file(),		*safe_copy();


/* The following global variables are used throughout ar */
char			FLG[26],	/* Command and option flags */
					/* for all lower-case flags */
			*c_zero = "\0",	/* Null character */
			*ar_fname,	/* File name of archive file */
			new_archive,	/* Flag to indicate a new archive */
			safe_err = 0,	/* Flag to indicate safe copy error */
			*orig_mem_tab,	/* Original archive member table */
			*orig_sym_tab,	/* Original archive symbol table */
			*tmp_ar_name,	/* Temp archive file name */
			*save_pos_m_name; /* Saved positioning member name */

long			num_of_mem,	/* Number of members in GST */
			mem_ary_size,	/* Size of in core member list */
			num_of_sym,	/* Number of symbols in GST */
			sym_ary_size,	/* Size of in core symbol list */
			num_of_free,	/* Number if items on free list */
			free_ary_size,	/* Size of in core free list */
			memtab_off,	/* Offset to Member Table */
			symtab_off,	/* Offset to Global Symbol Table */
			firstmem_off,	/* Offset to first member in archive */
			lastmem_off,	/* Offset to last member in archive */
			freelst_off,	/* Offset to member free list */
			prev_hdr_off,	/* Previous header pointer offset */
			next_hdr_off,	/* Next header offset */
			want_mem_index,	/* Next desired member index */
			new_free,	/* Num of items added to free list */
			fs_limit,	/* Maximum file size this process */
			cur_date;	/* Current date(time) */

int			nonfatal_errors = 0, /* nonfatal errors reported */
			Cflag = 0,	/* Indicate if C flag given. */
			save_pos_p_flag; /* Saved positioning flag */

struct sym_item		**mem_array,	/* Archive member list */
			*orig_mem_str,	/* Original member structures */
			**sym_array,	/* Archive symbol list */
			*orig_sym_str,	/* Original symbol structures */
			**free_array;	/* Free area list */


main(argc, argv)
/*******************************************************************************
	main - Entry point.
*******************************************************************************/
int argc;
char **argv;
{
	int		read_cmd = 0,
			write_cmd = 0,
			opt2;  /* for use with getopt */
	char 		*opt,
			*pos_name;
	FILE		*ar_fp;
	extern char	new_archive;
	extern char	*optarg;
	extern int	optind;
	short		posflags = 0;  /* count positioning flags used. */

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_AR, NL_CAT_LOCALE);
	/* Global variable Initialization */
	mem_array = sym_array = free_array = NULL;
	mem_ary_size = sym_ary_size = free_ary_size = 0;
	orig_sym_tab = orig_mem_tab = NULL;
	orig_mem_str = orig_sym_str = NULL;
	new_free = 0;
	tmp_ar_name = NULL;

	if(argc < 3)
	{
		ar_exit(MSGSTR(AR_USAGE_MSG, AR_USAGE));
	}

	/* Read in command line options */

	/* For some backward compatability, the "old" way of reading in
	 * arguments is being left in.  That way, a user who used to do
	 * "ar tv lib.a" can still do so.  If the user does "ar -tv lib.a"
	 * the new, "correct" method of getting arguments will be used. */
	opt = argv[1];
	if(*opt == '-')  /* started with a dash; do usual getopts method. */
		{
			while ((opt2=getopt(argc,argv,
				"a:b:ci:losuvzptwxdhmqrCT")) != EOF)
			{
				switch(opt2)
				{
				case 'a':
				case 'b':
				case 'i':
					++posflags;
					pos_name = trim(optarg);
				case 'c':
				case 'l':
				case 'o':
				case 's':
				case 'u':
				case 'v':
				case 'z':
					flg[opt2]++;
					break;
				case 'C':
					Cflag=1;
					break;
				case 'T':
					break;
				/* Commands */
				case 'p':
				case 't':
				case 'w':
				case 'x':
					read_cmd++;
					flg[opt2]++;
					break;
				case 'd':
				case 'h':
				case 'm':
				case 'q':
				case 'r':
					write_cmd++;
					flg[opt2]++;
					break;
				default:	ar_exit(MSGSTR(BAD_OPTION_MSG,
						BAD_OPTION), *opt);
				} /* switch */
			} /* while */

		} /* new improved getopts */
	else
	{
		for( ; *opt; opt++)	/* for each option */
		{
			switch(*opt)	/* switch on option */
			{	/* Options */
				case 'a':
				case 'b':
				case 'i':
					++posflags;
				case 'c':
				case 'l':
				case 'o':
				case 's':
				case 'u':
				case 'v':
				case 'z':
					flg[*opt]++;
					break;
				case 'C':
					Cflag=1;
					break;
				case 'T':
					break;
				/* Commands */
				case 'p':
				case 't':
				case 'w':
				case 'x':
					read_cmd++;
					flg[*opt]++;
					break;
				case 'd':
				case 'h':
				case 'm':
				case 'q':
				case 'r':
					write_cmd++;
					flg[*opt]++;
					break;
				default:	ar_exit(MSGSTR(BAD_OPTION_MSG,
						BAD_OPTION), *opt);
			} /* switch */
		} /* for *opt */

		/* Since the "old method" doesn't set optind automatically,
		 * set it here to point to the correct value. */
		optind = 2;

		/* Adjust parameters according to options */
		if(posflags)	/* -a, -b, or -i */
		{
			if(argc < 4)
			{
				ar_exit(MSGSTR(AR_USAGE_MSG, AR_USAGE));
			}
			/* Save positioning member */
			pos_name = trim(argv[optind]);
			++optind;
		}  /* if flg ... */

		/* Throw away --, so even the old way will work with this */
		if (!strcmp(argv[optind],"--"))
		{
			argc--;
			argv++;
		}

	}  /* old method of argument reading.... */

	/* Make sure no more than one positioning flag was given. */
	if (posflags > 1)
		ar_exit(MSGSTR(AR_USAGE_MSG, AR_USAGE));

	/* Positioning flags are only valid with -r and -m. */
	if ((posflags) && ((!flg['r']) && (!flg['m'])))
	{
		if (flg['q'] || flg['d'])
			ar_error(MSGSTR(NO_Q_POS_MSG, NO_Q_POS));
		ar_exit(MSGSTR(AR_USAGE_MSG, AR_USAGE));
	}

	/* Make sure only one "action" command is specified */
	if((read_cmd + write_cmd) > 1)
	{
		ar_error(MSGSTR(TOO_MANY_CMDS_MSG, TOO_MANY_CMDS));
		ar_exit(MSGSTR(AR_USAGE_MSG, AR_USAGE));
	}

	/* Make sure at least one non-neutral flag was given.
	  (that is, "-vlcCT" w/o any other flags is worthless.)   */
	if(!read_cmd && !write_cmd && !flg['s'] && !flg['o'])
		ar_exit(MSGSTR(AR_USAGE_MSG, AR_USAGE));


	/* Ignore appropiate signals if we are modifying the file */
	if(write_cmd || flg['s'] || flg['o'])
	{
		(void)signal(SIGHUP, SIG_IGN);
		(void)signal(SIGINT, SIG_IGN);
		(void)signal(SIGQUIT, SIG_IGN);
	}

	/* Open/create the archive file in the proper mode */
	if(write_cmd || flg['s'] || flg['o'])
	{
		if(flg['q'] || flg['r'])
			ar_fp = create_ar_file(argv[optind], "r+", CREATE);
		else
			ar_fp = create_ar_file(argv[optind], "r+", NO_CREATE);
	}
	else
	{
		ar_fp = create_ar_file(argv[optind], "r", NO_CREATE);
	}

	++optind;

	/* If safe copy is desired, make the copy now */
	if(write_cmd && flg['z'] && !flg['h'])
		ar_fp = safe_copy(ar_fp, COPY_IN);

	/* Get process file size limit */
	if((fs_limit = (long)ulimit((int)1, (off_t)0) * (long)512) < 0)
		fs_limit = MAXLONG;

	/* If an existing archive file read in useful information */
	if(!new_archive)
	{
		if(write_cmd ||  flg['w'] || (argc > optind) )
		{	/* get member and symbol tables */
			get_mem_sym_tab(ar_fp, memtab_off, symtab_off);
		}

		if(write_cmd && !flg['h'])	/* If modifying archive */
		{			
			zero_fixed_hdr(ar_fp);	/* zero fixed header */

			get_free_lst(ar_fp, freelst_off);  /* get free list */

			/* and position */
			if(flg['m'] || flg['r'] || flg['q'])
			{
				if(flg['a'])
				{	/* After named module */
					set_mod_pos(ar_fp, P_AFTER, pos_name);
				}
				else if(flg['b'] || flg['i'])
				{	/* Before named module */
					set_mod_pos(ar_fp, P_BEFORE, pos_name);
				}
				else
				{	/* At end of archive file */
					set_mod_pos(ar_fp, P_ATEND, pos_name);
				}
			}
		}
	}

	/* Now call the appropiate command */
	if(write_cmd)
	{	/* Write commands */
		register int arg_offset = argc - optind;

		if(flg['m'] && (arg_offset))
			mov_file_mem(ar_fp, &argv[optind], arg_offset);
		else if(flg['d'] && (arg_offset))
			rmv_file_mem(ar_fp, &argv[optind], arg_offset);
		else if(flg['h'])
		{
			cur_date = time((long *) 0);
			process_mem_list(ar_fp, &argv[optind], arg_offset,
				set_mem_time);
		}
		else if(flg['r'] && (arg_offset))
			replace_file_mem(ar_fp, &argv[optind], arg_offset);
		else if(flg['q'] && (arg_offset))
			(void)add_file_mem(ar_fp, &argv[optind], arg_offset);
	}
	else
	{
		/* Read commands */
		if(flg['p'])
			process_mem_list(ar_fp, &argv[optind], argc - optind,
				write_file_mem);
		else if(flg['t'])
			process_mem_list(ar_fp, &argv[optind], argc - optind,
				list_file_mem);
		else if(flg['w'])
			list_sym_tab();
		else if(flg['x'])
			process_mem_list(ar_fp, &argv[optind], argc - optind,
				extract_file_mem);
	}

	/* If desired recreate the symbol table if it needs update */
	if(flg['s'] && !write_cmd)
	{
		create_mem_sym_tab(ar_fp);
	}

	/* If items were added to the free list return space to file system */
	if(new_free > 0)
	{
		clear_free_list(ar_fp);
	}

	if((write_cmd && !flg['h']) || flg['s'])
	{	/* Update header and table */
		write_mem_sym_tab(ar_fp);
		write_fixed_hdr(ar_fp);
	}

	/* If safe copy is desired, copy back now */
	if(write_cmd && flg['z'] && !flg['h'])
		ar_fp = safe_copy(ar_fp, COPY_OUT);

	(void)fclose(ar_fp);

	/* If member ordering and compression is desired do it now */
	if(flg['o'])
	{
		order_ar_file(ar_fname);
	}

	exit(nonfatal_errors);
}


int add_file_mem(ar_fp, m_list, m_num)
/*******************************************************************************
	add_file_mem - Add members to the archive file

	This routine is responsible for adding new members to the archive
	file.  For each member in the list, the next free position in the
	archive will be determined, the member will be copied to this location,
	and the member will be added to the global symbol table.  All links
	and headers in the file will be updated.  Input Parameters:
		ar_fp	- File pointer for archive file.
		m_list	- List of members to be added.
		m_num	- Number of members in member list.
	Return Codes:
		0	- If all is well.
		-1	- In case of error.
*******************************************************************************/
FILE *ar_fp;
char *m_list[];
int m_num;
{
	struct bin_hdr	b_hdr;
	struct ar_hdr	*a_hdr;
	struct stat	statbuf;
	int		i,
			bh_boundary,       /* boundary for member alignment */
			err_code = 0;
	long		prev_off,
			f_offset;
	short		is_object;         /* set if member is an object */

	char		*m_name;
	FILE		*m_fp;
	extern long	prev_hdr_off,
			next_hdr_off,
			firstmem_off,
			lastmem_off;

	struct filehdr	f_hdr;
	AOUTHDR		opt_hdr;

	a_hdr = (struct ar_hdr *)ar_malloc(HDR_RDSIZE);
	prev_off = prev_hdr_off;

	/* For each new member */
	for(i = 0; i < m_num; i++)
	{
		/* Get stat and open */
		if(stat(m_list[i], (struct stat *)&statbuf)) 
		{
			ar_error(MSGSTR(STAT_ERR_MSG, STAT_ERR), m_list[i]);
			continue;
		}
		if((m_fp = fopen(m_list[i], "r")) == NULL)
		{
			ar_error(MSGSTR(OPEN_ERR_MSG, OPEN_ERR), m_list[i]);
			continue;
		}

		/* Trim all but leaf component from name */
		m_name = trim(m_list[i]);

		/* Initialize internal binary header */
		b_hdr.bh_size = statbuf.st_size;
		b_hdr.bh_prvmem = prev_off;
		b_hdr.bh_nxtmem = 0;
		b_hdr.bh_date = statbuf.st_mtime;
		b_hdr.bh_uid = statbuf.st_uid;
		b_hdr.bh_gid = statbuf.st_gid;
		b_hdr.bh_mode = statbuf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
		b_hdr.bh_namlen = strlen(m_name);
		b_hdr.bh_name = m_name;


		/* Check and see if member is an executable object */
		/* Open file, check magic, non-empty symbol tbl, executable */
		/* Note: U800's are for RT.  May no longer be necessary */
		if((fread((void *)&f_hdr, (size_t)1, (size_t)FILHSZ, m_fp) ==
		FILHSZ) && (f_hdr.f_magic == U802WRMAGIC ||
		f_hdr.f_magic == U802ROMAGIC || f_hdr.f_magic == U802TOCMAGIC ||
		f_hdr.f_magic == U800WRMAGIC || f_hdr.f_magic == U800ROMAGIC ||
		f_hdr.f_magic == U800TOCMAGIC) &&  (F_EXEC & f_hdr.f_flags) )
			is_object = 1;
		else
			is_object = 0;

		/* if an object, determine appropriate offsets */
		if ( (is_object)  &&  (f_hdr.f_opthdr)  &&
			!(fread((void *)&opt_hdr, (size_t)sizeof(AOUTHDR),
				(size_t)1, m_fp) == sizeof(AOUTHDR)) )
		{
			/* align at greater of algntext and algndata */
			bh_boundary = 2 << 
			    (MAX(opt_hdr.o_algntext,opt_hdr.o_algndata) - 1);

			/* don't align if request is bigger than page size */
			if ((bh_boundary > PAGE_SIZE) || (bh_boundary == 0))
				bh_boundary = 2;
		}
		else
			bh_boundary = 2;   /* any even-byte boundary */


		/* Find first free position in the archive file */
		f_offset = find_free_pos(ar_fp, (long)(ROUNDUP(b_hdr.bh_size) +
			ROUNDUP(b_hdr.bh_namlen)),b_hdr.bh_namlen,bh_boundary);

		/* Check to see if member will fit in file */
		if((f_offset + ROUNDUP(b_hdr.bh_size) +
		ROUNDUP(b_hdr.bh_namlen)) > fs_limit)
		{
			ar_error(MSGSTR(FSLIMIT_ERR_MSG, FSLIMIT_ERR));
			err_code = -1;
			break;
		}

		/* Link to previous member in list if present */
		if(prev_off == prev_hdr_off)
		{
			if(prev_hdr_off == 0)	/* First mem in archive */
				firstmem_off = f_offset;
			else
			{			/* Update previous member */
				ar_fseek(ar_fp, prev_off, 0);
				ar_fread((char *)a_hdr, 1, AR_HSZ, ar_fp);
				update_offset(a_hdr->ar_nxtmem, f_offset);
				ar_fseek(ar_fp, prev_off, 0);
				ar_fwrite((char *)a_hdr, 1, AR_HSZ, ar_fp);
			}
		}
		else
		{
			update_offset(a_hdr->ar_nxtmem, f_offset);

			ar_fseek(ar_fp, prev_off, 0);
			ar_fwrite((char *)a_hdr, 1, (int)(AR_HSZ +
				ROUNDUP(atol(a_hdr->ar_namlen))), ar_fp);
		}

		/* Copy new member(data only) to archive. */
		ar_fseek(ar_fp, (long)(f_offset + AR_HSZ +
		ROUNDUP(b_hdr.bh_namlen)), 0);
			
		if(b_hdr.bh_size > 0)
		{
			ar_fseek(m_fp, 0L, 0);
			copy_file(m_fp, ar_fp, b_hdr.bh_size);

			/* Pad with NULL byte if odd member length */
			if(b_hdr.bh_size & 0x00000001)
				ar_fwrite((char *)c_zero, 1, 1, ar_fp);
		}
		else
		{	/* Note:  When adding empty files we must write
			 *	  something, else find_free_pos may return
			 *	  offset of this header on next call.
			 */
			ar_fseek(ar_fp, -1L, 1);
			ar_fwrite((char *)c_zero, 1, 1, ar_fp);
		}
		(void)fclose(m_fp);

		/* Add new member to Global Symbol Table */
		add_sym_mem(ar_fp, m_name, f_offset, b_hdr.bh_size);
		want_mem_index++;	/* Add in order to mem_array */

		/* Convert header to ascii and save its position */
		bin_asc_hdr((struct bin_hdr *)&b_hdr, a_hdr);
		prev_off = f_offset;

		/* If verbose option write file name */
		if(flg['v'])
		{
			ar_fprintf(stdout, VERBOSE_CMD, 'q', m_list[i]);
		}
	}

	/* Now write the last header to complete member list */
	if(prev_off != prev_hdr_off)	/* Make sure we added something */
	{
		update_offset(a_hdr->ar_nxtmem, next_hdr_off);

		ar_fseek(ar_fp, prev_off, 0);
		ar_fwrite((char *)a_hdr, 1, (int)(AR_HSZ
			+ ROUNDUP(b_hdr.bh_namlen)), ar_fp);

		if(next_hdr_off == 0)	/* If last member in archive, mark it */
		{
			lastmem_off = prev_off;
		}
		else
		{	/* Update next member */
			ar_fseek(ar_fp, next_hdr_off, 0);
			ar_fread((char *)a_hdr, 1, AR_HSZ, ar_fp);
			update_offset(a_hdr->ar_prvmem, prev_off);
			ar_fseek(ar_fp, next_hdr_off, 0);
			ar_fwrite((char *)a_hdr, 1, AR_HSZ, ar_fp);
		}

		/* Update previous in case multiple calls are made by replace */
		prev_hdr_off = prev_off;
	}

	free((char *)a_hdr);

	if(err_code)
		mem_array = NULL;
	return(err_code);
}


void rmv_file_mem(ar_fp, m_list, m_num)
/*******************************************************************************
	rmv_file_mem - Remove a member from the archive file.

	This routine is designed to remove a number of members from the archive
	file.  It will unlink the member from the internal member list and
	add it to the free list.  At the same time all in memory tables will
	also be updated.  Input Parameters:
		ar_fp	- File pointer to archive file.
		m_list	- Member list containing names of members to be removed.
		m_num	- Number of members in the list to be removed.
*******************************************************************************/
FILE *ar_fp;
char *m_list[];
int m_num;
{
	int			i, j, tmp_cnt,
				not_deleted;
	char			**t_name,
				*m_del;
	long			tmp_len,
				save_num;
	struct ar_hdr		*tmp_hdr;
	extern long		num_of_mem,
				firstmem_off,
				lastmem_off;
	extern struct sym_item	**mem_array;

	/* Get memory for header */
	tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);

	/* Set up side arrays for processing */
	m_del = (char *)ar_malloc((unsigned)m_num);
	t_name = (char **)ar_malloc((unsigned)m_num * sizeof(char *));
	for(i = 0; i < m_num; i++)
	{
		m_del[i] = 0;
		t_name[i] = trim(m_list[i]);
	}

	/* For each member in the archive */
	save_num = num_of_mem;
	not_deleted = m_num;
	for(i = tmp_cnt = 0; (tmp_cnt < save_num) && not_deleted; i++)
	{
		if(mem_array[i] == NULL)
		{
			continue;
		}
		tmp_cnt++;

		/* For each member in the delete list */
		for(j = 0; j < m_num; j++)
		{
			if(m_del[j])
				continue;

			/* Check name for match */
			if(strcmp(mem_array[i]->_si_data.si_name, t_name[j]))
			{
				continue;
			}

			/* Found One */
			/* If verbose name deleted member */
			if(flg['v'])
			{
				ar_fprintf(stdout, VERBOSE_CMD, 'd', m_list[j]);
			}

			get_ar_hdr(ar_fp, mem_array[i]->si_offset, tmp_hdr);

			/* Unlink from member list */
			unlink_file_mem(ar_fp, tmp_hdr);

			/* Add member to the free list */
			tmp_len = ROUNDUP(atol(tmp_hdr->ar_size)) +
				ROUNDUP(atol(tmp_hdr->ar_namlen));
			add_free_mem(ar_fp, mem_array[i]->si_offset, tmp_len);

			/* Remove from in core symbol table */
			rmv_sym_mem(i);

			/* Mark named member found */
			m_del[j]++;
			not_deleted--;
			break;
		} /* for m_num */
	} /* for num_of_mem */
	free((char *)tmp_hdr);
	free((char *)t_name);


	/* Check for members not deleted */
	if(not_deleted)
	{
		for(i = 0; not_deleted; i++)
		{
			if(m_del[i])
				continue;

			ar_error(MSGSTR(MEM_NOTFOUND_MSG, MEM_NOTFOUND),
				m_list[i]);
			not_deleted--;
		}
	}

	free((char *)m_del);
	return;
}


void mov_file_mem(ar_fp, m_list, m_num)
/*******************************************************************************
	mov_file_mem - Move members within the archive file

	This routine is responsible for moving archive members within the
	archive file.  It will actually change the links to the file members
	rather than actually move the members themselves.  Because of this
	it is only necessary to change the header associated with the members.
	Input Parameters:
		ar_fp	- File pointer to archive file.
		m_list	- List of archive members to be moved.
		m_num	- Number of members in the list.
*******************************************************************************/
FILE *ar_fp;
char *m_list[];
int m_num;
{
	int			i, j,
				tmp_cnt,
				not_moved;
	char			**t_name,
				*m_del;
	long			p_off,
				move_list,
				*move_indicies;
	struct ar_hdr		*prev_hdr,
				*this_hdr,
				*tmp_hdr;
	struct sym_item		*tmp_ptr;
	extern long		num_of_mem,
				prev_hdr_off,
				next_hdr_off,
				lastmem_off,
				firstmem_off,
				want_mem_index;
	extern struct sym_item	**mem_array;

	prev_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);
	this_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);
	move_list = p_off = 0;

	/* Set up side arrays for processing */
	m_del = (char *)ar_malloc((unsigned)m_num);
	t_name = (char **)ar_malloc((unsigned)m_num * sizeof(char *));
	move_indicies = (long *)ar_malloc((unsigned)m_num * sizeof(long));
	for(i = 0; i < m_num; i++)
	{
		m_del[i] = 0;
		t_name[i] = trim(m_list[i]);
		move_indicies[i] = -1;
	}

	/* For each member of the archive */
	not_moved = m_num;
	for(i = tmp_cnt = 0; (tmp_cnt < num_of_mem) && not_moved; i++)
	{
		if(mem_array[i] == NULL)
		{
			continue;	/* Empty item */
		}
		tmp_cnt++;

		for(j = 0; j < m_num; j++)
		{
			/* Check name for match */
			if(strcmp(mem_array[i]->_si_data.si_name, t_name[j]))
			{
				continue;	/* No match */
			}

			/* If verbose name moved member */
			if(flg['v'])
			{
				ar_fprintf(stdout, VERBOSE_CMD, 'm', m_list[j]);
			}

			/* Found One */
			get_ar_hdr(ar_fp, mem_array[i]->si_offset, this_hdr);

			/* Check for moving a member relative to itself */
			if(mem_array[i]->si_offset == prev_hdr_off)
			{
				prev_hdr_off = atol(this_hdr->ar_prvmem);		
			}
			if(mem_array[i]->si_offset == next_hdr_off)
			{
				next_hdr_off = atol(this_hdr->ar_nxtmem);
			}

			/* Unlink from current pos */
			unlink_file_mem(ar_fp, this_hdr);

			/* Link this member into the move list */
			if(move_list > 0)
			{
				update_offset(prev_hdr->ar_nxtmem,
					mem_array[i]->si_offset);
				ar_fseek(ar_fp, p_off, 0);
				ar_fwrite((char *)prev_hdr, 1, AR_HSZ, ar_fp);

				/* Update prvmem field of this header */
				update_offset(this_hdr->ar_prvmem, p_off);
			}
			else
			{
				move_list = mem_array[i]->si_offset;
			}


			/* Update loop counters */
			p_off = mem_array[i]->si_offset;
			tmp_hdr = this_hdr;
			this_hdr = prev_hdr;
			prev_hdr = tmp_hdr;

			/* Mark member found */
			move_indicies[m_num - not_moved] = i;
			m_del[j]++;
			not_moved--;
			break;
		} /* for m_num */
	} /* for num_of_mem */


	/* Now link the move list into the file member list */
	if(move_list > 0)	/* Make sure it exists */
	{
		/* Link first member of list */
		if(prev_hdr_off > 0)
		{
			get_ar_hdr(ar_fp, prev_hdr_off, this_hdr);
			update_offset(this_hdr->ar_nxtmem, move_list);
			ar_fseek(ar_fp, prev_hdr_off, 0);
			ar_fwrite((char *)this_hdr, 1, AR_HSZ, ar_fp);
		}
		else
		{
			firstmem_off = move_list;
		}
		if(p_off == move_list)	/* only 1 member */
		{
			update_offset(prev_hdr->ar_prvmem, prev_hdr_off);
		}
		else
		{
			get_ar_hdr(ar_fp, move_list, this_hdr);
			update_offset(this_hdr->ar_prvmem, prev_hdr_off);
			ar_fseek(ar_fp, move_list, 0);
			ar_fwrite((char *)this_hdr, 1, AR_HSZ, ar_fp);
		}

		/* Link last member of list */
		update_offset(prev_hdr->ar_nxtmem, next_hdr_off);
		ar_fseek(ar_fp, p_off, 0);
		ar_fwrite((char *)prev_hdr, 1, AR_HSZ, ar_fp);

		if(next_hdr_off == 0)
		{	/* This is new last member */
			lastmem_off = p_off;
		}
		else
		{	/* Make last member point to prev mem */
			get_ar_hdr(ar_fp, next_hdr_off, this_hdr);
			update_offset(this_hdr->ar_prvmem, p_off);
			ar_fseek(ar_fp, next_hdr_off, 0);
			ar_fwrite((char *)this_hdr, 1, AR_HSZ, ar_fp);
		}
	}

	/* Reorder elements of the member array to reflect the move */
	for(i = 0; i < (m_num - not_moved); i++)
	{
		tmp_ptr = mem_array[move_indicies[i]];
		mem_array[move_indicies[i]] = NULL;
		want_mem_index = get_mem_index();
		mem_array[want_mem_index] = tmp_ptr;
	}
		

	/* Check for members not found */
	if(not_moved)
	{
		for(i = 0; not_moved; i++)
		{
			if(m_del[i])
				continue;

			ar_error(MSGSTR(MEM_NOTFOUND_MSG, MEM_NOTFOUND),
				m_list[i]);
			not_moved--;
		}
	}

	free((char *)m_del);
	free((char *)t_name);
	free((char *)prev_hdr);
	free((char *)this_hdr);
	return;
}


void replace_file_mem(ar_fp, m_list, m_num)
/*******************************************************************************
	replace_file_mem - Replace archive file members

	This routine is designed to replacing members within the archive.
	Each member in the list will be replaced at the same location with
	a new copy.  If the member is not present it will simply be added
	at the position specified by the global positioning variables.
	Input Parameters:
		ar_fp	- File pointer for archive file.
		m_list	- List of members to be replaced.
		m_num	- Numbers of members in the member list.
*******************************************************************************/
FILE *ar_fp;
char *m_list[];
int m_num;
{
	char		v_save,
			*m_del,
			**t_name;
	int		i, j, tmp_cnt,
			not_replaced;
	struct ar_hdr	*tmp_hdr;
	struct stat	statbuf;


	/* Save default flag information */
	v_save = flg['v'];
	flg['v'] = 0;

	tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);

	/* Initialize side arrays for processing */
	m_del = (char *)ar_malloc((unsigned)m_num);
	t_name = (char **)ar_malloc((unsigned)m_num * sizeof(char *));
	for(i = 0; i < m_num; i++)
	{
		m_del[i] = 0;
		t_name[i] = trim(m_list[i]);
	}

	not_replaced = m_num;
	/* For each member of the archive */
	for(i = tmp_cnt = 0; (tmp_cnt < num_of_mem) && not_replaced; i++)
	{
		if(mem_array[i] == NULL)
			continue;
		tmp_cnt++;

		/* For each member in replace list */
		for(j = 0; j < m_num; j++)
		{
			if(m_del[j])
				continue;

			/* Check for name match */
			if(strcmp(t_name[j], mem_array[i]->_si_data.si_name))
				continue;

			/* We have found a match, make sure file exists */
			if(stat(m_list[j], &statbuf))
			{
				ar_error(MSGSTR(STAT_ERR_MSG, STAT_ERR),
					m_list[j]);
				m_del[j]++;
				not_replaced--;
				break;
			}
				
			/* Get header for this member */
			get_ar_hdr(ar_fp, mem_array[i]->si_offset, tmp_hdr);

			/* Process the 'u' option */
			if(flg['u'])
			{
				if(statbuf.st_mtime <= atol(tmp_hdr->ar_date))
				{
					m_del[j]++;
					not_replaced--;
					break;
				}
			}

			if(v_save)
				ar_fprintf(stdout, VERBOSE_CMD, 'r', m_list[j]);

			/* Save positioning pointers */
			prev_hdr_off = atol(tmp_hdr->ar_prvmem);
			if(mem_array[i]->si_offset == lastmem_off)
				next_hdr_off = 0L;
			else
				next_hdr_off = atol(tmp_hdr->ar_nxtmem);
			want_mem_index = i;

			/* Unlink member from archive file list, add to
			   free list and remove from symbol table.
			*/
			unlink_file_mem(ar_fp, tmp_hdr);
			add_free_mem(ar_fp, mem_array[i]->si_offset,
				ROUNDUP(atol(tmp_hdr->ar_size)) +
				ROUNDUP(atol(tmp_hdr->ar_namlen)));
			rmv_sym_mem(i);

			/* Add new(replaced) member at same position */
			if(add_file_mem(ar_fp, &m_list[j], 1))
			{
				not_replaced = 0;
				break;
			}

			/* Mark member replaced */
			m_del[j]++;
			not_replaced--;
			break;
		}
	}
	free((char *)tmp_hdr);
	free((char *)t_name);

	/* If named members were not replaced add them to archive */
	if(not_replaced)
	{
		/* Call set_mod_pos with Saved positioning parameters */
		set_mod_pos(ar_fp, save_pos_p_flag, save_pos_m_name);

		/* Now add each new member to the archive */
		for(i = tmp_cnt = 0; tmp_cnt < not_replaced; i++)
		{
			if(m_del[i])
				continue;

			if(v_save)
				ar_fprintf(stdout, VERBOSE_CMD, 'a', m_list[i]);

			if(add_file_mem(ar_fp, &m_list[i], 1))
			{
				tmp_cnt = not_replaced;
			}
			tmp_cnt++;
			/* Note that want_mem_index is incremented in */
			/* add_file_mem.			      */
		}
	}

	free((char *)m_del);
	flg['v'] = v_save;

	return;
}


void list_file_mem(ar_fp, mem_off, mem_hdr, mem_name)
/*******************************************************************************
	list_file_mem - List the member of a archive.

	This routine is designed to simply list a member of the archive.
	It will be passed a simgle archive member header containing info-
	rmation about the member to be listed.  This routine is designed
	to be called by 'process_mem_list'.
	Input Parameters:
		ar_fp		- File pointer to archive file.(unused)
		mem_off		- Archive file offset of member(unused)
		mem_hdr		- Archive member header.
		mem_name	- Archive member name.
*******************************************************************************/
FILE *ar_fp;
long mem_off;
struct ar_hdr *mem_hdr;
char *mem_name;
{
	struct tm	*nlsdate;
	char		nls_buf[NLTBMAX];
	struct bin_hdr	b_hdr;


	if(flg['v'])	/* verbose option */
	{
		asc_bin_hdr((struct bin_hdr *)&b_hdr, mem_hdr);
		write_file_mode(b_hdr.bh_mode);
		nlsdate = localtime(&b_hdr.bh_date);

		(void)strftime(nls_buf, (size_t)NLTBMAX, 
				"%b %e %H:%M %Y", nlsdate);
		ar_fprintf(stdout, " %5d/%-5d %6ld %s %s",
			b_hdr.bh_uid, b_hdr.bh_gid,
			b_hdr.bh_size, nls_buf, b_hdr.bh_name);
		free((char *)b_hdr.bh_name);
	}
	else
	{
		ar_fprintf(stdout, CHAR_STRING, trim(mem_name));
	}

	return;
}


void set_mem_time(ar_fp, mem_off, mem_hdr, mem_name)
/*******************************************************************************
	set_mem_time - Set modification time of the named member.

	This routine is designed to set the modification time of the named
	member of the archive.  The time set will be the current time which
	is kept in cur_date.  This routine will be called from 'process_mem_
	list' which will determine upon which members to operate.
	Input Parameters:
		ar_fp		- File pointer to archive file.
		mem_off		- Archive file offset of member(unused)
		mem_hdr		- Archive member header.
		mem_name	- Name of archive file member.
*******************************************************************************/
FILE *ar_fp;
long mem_off;
struct ar_hdr *mem_hdr;
char *mem_name;
{
	struct tm	*nlsdate;
	char		nls_buf[NLTBMAX];

	if(flg['v'])
	{
		nlsdate = localtime(&cur_date);
		(void)strftime(nls_buf, (size_t)NLTBMAX, "%b %e %H:%M %Y",
			nlsdate);
		ar_fprintf(stdout, "%s%s", nls_buf, mem_name);
	}

	update_offset(mem_hdr->ar_date, cur_date);
	ar_fseek(ar_fp, mem_off, 0);
	ar_fwrite((char *)mem_hdr, AR_HSZ, 1, ar_fp);

	return;
}


void extract_file_mem(ar_fp, mem_off, mem_hdr, mem_name)
/*******************************************************************************
	extract_file_mem - Extract archive file members.

	This routine is designed to extract members of an archive.  It 
	will create files of the same name in the current directory and
	copy the member contents to that file.
	Input Parameters:
		ar_fp		- File pointer to archive file.
		mem_off		- Archive file offset of member(unused)
		mem_hdr		- Archive member header.
		mem_name	- Name of archive file member
*******************************************************************************/
FILE *ar_fp;
long mem_off;
struct ar_hdr *mem_hdr;
char *mem_name;
{
	FILE *n_fp;
	struct stat statbuf;

	/* don't overwrite if filename exists in filesystem. */
	if(Cflag && !stat(mem_name, (struct stat *)&statbuf))
	{
		ar_error(MSGSTR(EXTRACT_ERR_MSG, EXTRACT_ERR), mem_name);
		return;
	}

	(void)close(creat(mem_name, 0666));

	if((n_fp = fopen(mem_name, "w")) == NULL)
	{
		ar_error(MSGSTR(OPEN_ERR_MSG, OPEN_ERR), mem_name);
	}
	else
	{
		if(flg['v'])
			ar_fprintf(stdout, VERBOSE_CMD, 'x', mem_name);

		ar_fseek(ar_fp, mem_off + (long)ROUNDUP(AR_HSZ +
			atol(mem_hdr->ar_namlen)), 0);
		copy_file(ar_fp, n_fp, atol(mem_hdr->ar_size));

		(void)fclose(n_fp);
		(void)chmod(mem_name, (int)(strtol((char *)mem_hdr->ar_mode,
			NULL, 8) & 0777));
	}

	return;
}


void write_file_mem(ar_fp, mem_off, mem_hdr, mem_name)
/*******************************************************************************
	write_file_mem - Write the named archive members to standard output

	This routine is designed to copy archive file members to standard
	output.  It will simply copy the raw data for each named member
	to standard output.  Input Parameters:
		ar_fp		- File pointer to archive file.
		mem_off		- Archive file offset of member(unused)
		mem_hdr		- Archive member header.
		mem_name	- Name of archive file member
*******************************************************************************/
FILE *ar_fp;
long mem_off;
struct ar_hdr *mem_hdr;
char *mem_name;
{

	if(flg['v'])
		ar_fprintf(stdout, "\n<%s>\n", mem_name);

	ar_fseek(ar_fp, mem_off + (long)ROUNDUP(AR_HSZ +
		atol(mem_hdr->ar_namlen)), 0);
	copy_file(ar_fp, stdout, atol(mem_hdr->ar_size));

	return;
}


void list_sym_tab()
/*******************************************************************************
	list_sym_tab - List the contents of the symbol table.

	This routine is responsible for listing the symbols contained
	in the global symbol table.  The in core copy of the symbol table
	will be used for this purpose.  Input Parameters:
		none
*******************************************************************************/
{
	int		i,j,
			tmp_cnt;
	char		*mem_name;
	long		tmp_offset = 0;

	/* Make sure there is something to list */
	if(num_of_sym == 0)
		ar_exit(MSGSTR(NO_SYM_TAB_MSG, NO_SYM_TAB), ar_fname);

	/* For each symbol in the table */
	for(i = tmp_cnt = 0; tmp_cnt < num_of_sym; i++)
	{
		if(sym_array[i] == NULL)
			continue;

		tmp_cnt++;
		/* See if object files have changed */
		if(sym_array[i]->si_offset != tmp_offset)
		{
			/* Search member array for this offset */
			for(j = 0; mem_array[j]->si_offset !=
			sym_array[i]->si_offset; j++);

			tmp_offset = mem_array[j]->si_offset;
			mem_name = mem_array[j]->_si_data.si_name;
		}

		/* Print symbol and member name */
		ar_fprintf(stdout, "%s\t%s",
			sym_array[i]->_si_data.si_name, mem_name);
	}

	return;
}


void create_mem_sym_tab(ar_fp)
/*******************************************************************************
	create_mem_sym_tab - Regenerate the member and symbol tables for file.

	This routine is designed to regenerate the archive file symbol table.
	It will first remove all current entries from the symbol table and
	then go through each file to recreate the table.
	Input Parameters:
		ar_fp	- File pointer to archive file.
*******************************************************************************/
FILE *ar_fp;
{
	long			tmp_off,
				n_len;
	struct ar_hdr		*tmp_hdr;
	extern long		lastmem_off,
				firstmem_off;

	/* First remove all current members from the table */
	if(orig_mem_str != NULL)
	{
		free((char *)orig_mem_str);
		orig_mem_str = NULL;
	}
	num_of_mem = 0;

	/* Now remove members from the current symbol table */
	if(orig_sym_str != NULL)
	{
		free((char *)orig_sym_str);
		orig_sym_str = NULL;
	}
	num_of_sym = 0;

	/* Free memory from original member and symbol tables */
	if(orig_mem_tab)
	{
		free((char *)orig_mem_tab);
		orig_mem_tab = NULL;
	}
	if(orig_sym_tab)
	{
		free((char *)orig_sym_tab);
		orig_sym_tab = NULL;
	}

	/* Now enter symbols for all members into the table */
	tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);

	tmp_off = firstmem_off;
	while((tmp_off != 0)  && (tmp_off != memtab_off))
	{
		get_ar_hdr(ar_fp, tmp_off, tmp_hdr);
		n_len = atol(tmp_hdr->ar_namlen);
		tmp_hdr->_ar_name.ar_name[n_len] = '\0';

		add_sym_mem(ar_fp, tmp_hdr->_ar_name.ar_name, tmp_off,
			atol(tmp_hdr->ar_size));

		tmp_off = atol(tmp_hdr->ar_nxtmem);
	}

	free((char *)tmp_hdr);

	return;
}


void process_mem_list(ar_fp, m_list, m_num, ar_fcn)
/*******************************************************************************
	process_mem_list - Process the member list.

	This routine is used to determine which archive members will be
	acted upon.  It will be passed a list of desired members.  The
	function ar_fcn will be called for each member in the list.  If
	the list is empty,  the function will be called for each member in
	the list.  Input Parameters:
		ar_fp	- File pointer to the archive file.
		m_list	- List of member names to process.
		m_num	- Number of names in the member list.
		ar_fcn	_ Function to be called for members in list
*******************************************************************************/
FILE *ar_fp;
char *m_list[];
int m_num;
void (*ar_fcn)();
{
	int			i,
				tmp_len,
				n_len,
				num_found;
	char			*m_del,
				**t_name,
				*n_str;
	long			tmp_off;
	struct ar_hdr		*tmp_hdr;
	extern long		lastmem_off,
				firstmem_off;

	tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);
	n_str = ar_malloc((unsigned)HDR_RDSIZE);

	if(m_num == 0)	/* No file members specified so process all */
	{
		n_str = ar_malloc((unsigned)HDR_RDSIZE);
		tmp_off = firstmem_off;
		while((tmp_off != 0) && (tmp_off != memtab_off))
		{
			get_ar_hdr(ar_fp, tmp_off, tmp_hdr);

			n_len = atol(tmp_hdr->ar_namlen);
			(void)strncpy(n_str, tmp_hdr->_ar_name.ar_name, n_len);
			n_str[n_len] = '\0';

			ar_fcn(ar_fp, tmp_off, tmp_hdr, n_str);

			tmp_off = atol(tmp_hdr->ar_nxtmem);
		}
		free(n_str);

	}

	else if(num_of_mem > 0)		/* Member table in memory */
	{
		for(i = 0; i < m_num; i++)
		{
			if((tmp_off = find_mem_pos(ar_fp, trim(m_list[i]))) > 0)
			{
				get_ar_hdr(ar_fp, tmp_off, tmp_hdr);

				ar_fcn(ar_fp, tmp_off, tmp_hdr,
					m_list[i]);
			}
			else
			{
				ar_error(MSGSTR(MEM_NOTFOUND_MSG,
					MEM_NOTFOUND), m_list[i]);
			}
		}
	}

	else				/* Scan archive for members */
	{
		m_del = ar_malloc((unsigned)m_num);
		t_name = (char **)ar_malloc((unsigned)m_num * sizeof(char *));
		for(i = num_found = 0; i < m_num; i++)
		{
			m_del[i] = 0;
			t_name[i] = trim(m_list[i]);
		}

		tmp_off = firstmem_off;
		while((num_found < m_num) && tmp_off && (tmp_off != memtab_off))
		{
			get_ar_hdr(ar_fp, tmp_off, tmp_hdr);
			tmp_len = atol(tmp_hdr->ar_namlen);

			for(i = 0; i < m_num; i++)
			{
				if(m_del[i])
					continue;

				if(strncmp(t_name[i], tmp_hdr->_ar_name.ar_name,
				tmp_len))
					continue;

				ar_fcn(ar_fp, tmp_off, tmp_hdr,
					m_list[i]);
				m_del[i]++;
				num_found++;
			}

			tmp_off = atol(tmp_hdr->ar_nxtmem);
		}

		if(num_found < m_num)
		{
			for(i = 0; num_found < m_num; i++)
			{
				if(!m_del[i])
				{
					ar_error(MSGSTR(MEM_NOTFOUND_MSG,
						MEM_NOTFOUND), m_list[i]);
					num_found++;
				}
			}
		}

		free((char *)m_del);
		free((char *)t_name);
	}

	free((char *)tmp_hdr);

	return;
}


long find_free_pos(fp, size, namelen, bh_boundary)
/*******************************************************************************
	find_free_pos - Find the next free position in file.

	This routine will search the archive for an area of free space that
	will hold a member of size 'size'.  Note that 'size' should be the
	number of bytes contained in the member(rounded up to an even byte
	boundary) + the number of bytes in the member file name(rounded up to
	an even byte boundary).  If the area found is currently part of the
	free list,  it will be unlinked and all pointers adjusted.
	Input Parameters:
		fp	- File pointer to archive file.
		size	- Member size + name length
		namelen	- Member name length
		bh_boundary - alignment boundary for member
	Return Codes
		n	- Next offset at which member may be written.
*******************************************************************************/
FILE *fp;
long size;
int namelen;
int bh_boundary;
{
	int		save_index,
			f_count, i,
			header_index, /*division remainder of header address*/
			found = 0;
	struct ar_hdr	*tmp_ptr;
	long		tmp_off,
			prev_off,
			next_off,
			save_offset, /* offset to cleared free member */
			save_datsize; /* size of cleared member */

	if(free_array != NULL)	/* If free list exists check in core copy */
	{
		/* Search for smallest area in which size will fit */
		for(save_index = -1, i = f_count= 0; f_count < num_of_free; i++)
		{
			if(free_array[i] == NULL)
			{
				continue;
			}

			if(free_array[i]->_si_data.si_size >= size)
			{
				if((save_index < 0) ||
				(free_array[i]->_si_data.si_size < 
				free_array[save_index]->_si_data.si_size))
				{
				/* see if will fit evenly or (if neccessary) 
				with alignment space */
				if (((header_index = (free_array[i]->si_offset +
				   ROUNDUP(AR_HSZ+namelen)) % bh_boundary) == 0)
			        || (free_array[i]->_si_data.si_size >= 
				   (size + (bh_boundary - header_index)))  )
					{
					if (header_index == 0)
					    tmp_off = free_array[i]->si_offset;
					else
					    tmp_off = free_array[i]->si_offset
					       + (bh_boundary-header_index);
					save_index = i;
					}
				}
			}

			f_count++;
		}

		if(save_index >= 0)	/* If area found, adjust file links */
		{
			found++;
			tmp_ptr = (struct ar_hdr *)ar_malloc((unsigned)
				AR_HSZ);

			/* Read in the header */
			ar_fseek(fp, free_array[save_index]->si_offset, 0);
			ar_fread((char *)tmp_ptr, 1,
				AR_HSZ, fp);

			/* Save offsets so we can add them back if able. */
			save_offset = free_array[save_index]->si_offset;
			save_datsize = free_array[save_index]->_si_data.si_size;

			/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
			/* Remove space from free list */
			prev_off = atol(tmp_ptr->ar_prvmem);
			next_off = atol(tmp_ptr->ar_nxtmem);

			/* Update header that points to this one */
			if(prev_off)
			{
				ar_fseek(fp, prev_off, 0);
				ar_fread((char *)tmp_ptr, 1,
					AR_HSZ, fp);
				update_offset(tmp_ptr->ar_nxtmem,
					next_off);
				ar_fseek(fp, prev_off, 0);
				ar_fwrite((char *)tmp_ptr, 1,
					AR_HSZ, fp);
			}
			else
			{
				freelst_off = next_off;
			}

			/* Update header pointed to by this one */
			if(next_off)
			{
				ar_fseek(fp, next_off, 0);
				ar_fread((char *)tmp_ptr, 1,
					AR_HSZ, fp);
				update_offset(tmp_ptr->ar_prvmem,
					prev_off);
				ar_fseek(fp, next_off, 0);
				ar_fwrite((char *)tmp_ptr, 1,
					AR_HSZ, fp);
			}

			/* Adjust in core list */
			free((char *)free_array[save_index]);
			free_array[save_index] = NULL;
			num_of_free--;
			/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
					
			free((char *)tmp_ptr);
			/*##################################################*/
			/* If enough space in padding before new member,
			  add that space to the free list. */
			if ( (header_index != 0) && 
			   (bh_boundary - header_index) > (long)AR_HSZ )
			     /* add to free list */
			     add_free_mem(fp,save_offset,
					(bh_boundary - header_index - AR_HSZ));

			/* If enough space AFTER new member,
			  add that space to the free list as well. 
			  End of old freespace is si_offset + AR_HSZ + si_size;
			  End of new member is tmp_off + AR_HSZ + size */
			
			if ( ((save_offset + AR_HSZ + save_datsize) - 
			   (tmp_off + (long)AR_HSZ + size)) > (long)AR_HSZ )
			     /* add to free list */
			     add_free_mem(fp,tmp_off + AR_HSZ + size,
				(save_offset + AR_HSZ + save_datsize) -
				(tmp_off + AR_HSZ + size) - AR_HSZ );
			/*##################################################*/

		}
	}

	/* If nothing found on free list begin writing at present location
	   of member table.
	*/
	if(!found && (memtab_off > 0))
	{
		/* Start looking at the old member table location */
		if ((header_index = (memtab_off + ROUNDUP(AR_HSZ+namelen))
			% bh_boundary) == 0)
		{
			tmp_off = memtab_off;
			memtab_off += (AR_HSZ + size);
		}
		else
		{
			if ( (bh_boundary - header_index) > AR_HSZ )
			/* add to free list */
			add_free_mem(fp,memtab_off,
				(long)(bh_boundary - header_index - AR_HSZ));

			tmp_off = memtab_off + (bh_boundary - header_index);
			memtab_off = tmp_off + (AR_HSZ + size);
		}
		found++;
	}

	/* If no member table, simply start at end of file */
	if(!found)
	{
		ar_fseek(fp, 0L, 2);
		tmp_off = ROUNDUP(ftell(fp));
		if ((header_index = (tmp_off + ROUNDUP(AR_HSZ+namelen))
			% bh_boundary) != 0)
		{
			if ( (bh_boundary - header_index) > AR_HSZ )
			/* add to free list */
			add_free_mem(fp,tmp_off,
				(bh_boundary - header_index - AR_HSZ));
			tmp_off += (bh_boundary - header_index);
		}
	}

	return(tmp_off);
}


void add_free_mem(fp, offset, size)
/*******************************************************************************
	add_free_mem - Add a new member to the free list.

	This routine is designed to add a new member to the free list.  It
	will link this new member into the free list in the archive and also
	update the in core copy of the free list.  Input Parameters:
		fp	- File pointer to the archive file.
		offset	- File offset at which this member is located.
		size	- Size of this member(minus header but including name)
*******************************************************************************/
FILE *fp;
long offset, size;
{
	int			save_index, tmp_index, 
				i, j, match;
	long			prev_off, next_off,
				end_off;
	struct ar_hdr		*ar_ptr;
	extern struct sym_item	**free_array;
	extern long		num_of_free,
				free_ary_size,
				freelst_off;


	ar_ptr = (struct ar_hdr *)ar_malloc((unsigned)AR_HSZ);
	if(free_array == NULL)			/* Free list doesn't exist */
	{					/* Create one */
		free_array = (struct sym_item **)ar_malloc((unsigned)(BUMP_VAL *
			sizeof(struct sym_item *)));
		num_of_free = 0;
		free_ary_size = BUMP_VAL;
		save_index = num_of_free;
		prev_off = next_off = 0;
	}
	else if(num_of_free == free_ary_size)	/* Free list full */
	{					/* Increase size */
		free_array = (struct sym_item **)ar_realloc((char *)free_array,
			(unsigned)((free_ary_size + BUMP_VAL) *
			sizeof(struct sym_item *)));
		free_ary_size += BUMP_VAL;
		save_index = num_of_free;
	}
	else		/* Add to free list, check for adjacent free area */
	{
		end_off = offset + AR_HSZ + size;
		match = 1;
		save_index = num_of_free;

		for(j = 0; match && (j < 2); j++)
		{
			/* Run the free list and check for adjacent areas
			   that can be joined.  This will require at most
			   two passes through the list.
			*/
			for(i = tmp_index = 0; tmp_index <  num_of_free; i++)
			{
				if(free_array[i] == NULL)
				{
					save_index = i;
					continue;	/* Empty slot */
				}

				tmp_index++;	

				/* Check for adjacent area */
				if(((free_array[i]->si_offset + AR_HSZ +
				free_array[i]->_si_data.si_size) == offset)
				|| (end_off == free_array[i]->si_offset))
				{
					size += (free_array[i]->_si_data.si_size
						+ AR_HSZ);
					if(end_off != free_array[i]->si_offset)
						offset=free_array[i]->si_offset;

					/* Unlink the old member */
					ar_fseek(fp, free_array[i]->si_offset,
						0);
					ar_fread((char *)ar_ptr, 1,
						AR_HSZ, fp);
					prev_off = atol(ar_ptr->ar_prvmem);
					next_off = atol(ar_ptr->ar_nxtmem);

					/* Update headers if necessary */
					if(prev_off)
					{	/* Update previous header */
						ar_fseek(fp, prev_off, 0);
						ar_fread((char *)ar_ptr,
							1, AR_HSZ , fp);
						update_offset(ar_ptr->ar_nxtmem,
							next_off);
						ar_fseek(fp, prev_off, 0);
						ar_fwrite((char *)ar_ptr, 1,
							AR_HSZ, fp);
					}
					else
					{
						freelst_off = next_off;
					}
					if(next_off)
					{	/* Update next header */
						ar_fseek(fp, next_off, 0);
						ar_fread((char *)ar_ptr, 1,
							AR_HSZ, fp);
						update_offset(ar_ptr->ar_prvmem,
							prev_off);
						ar_fseek(fp, next_off, 0);
						ar_fwrite((char *)ar_ptr, 1,
							AR_HSZ, fp);
					}

					/* Remove from free list */
					free((char *)free_array[i]);
					free_array[i] = NULL;
					num_of_free--;
					save_index = i;

					match++;  /* Note match and quit */
					break;    /* looking on this pass */
				} /* if adjacent */
			} /* for i */

			if(match == 1)
				match = 0;  /* No adjacent areas */
			else
				match = 1;
		} /* for j */
	} /* else add to free list */


	/* Add to the in core free list */
	free_array[save_index] = (struct sym_item *)ar_malloc((unsigned)
		sizeof(struct sym_item));
	free_array[save_index]->si_offset = offset;
	free_array[save_index]->_si_data.si_size = size;
	num_of_free++;
	new_free++;

	/* Now link the member into the archive file free list */
	update_offset(ar_ptr->ar_size, size);
	update_offset(ar_ptr->ar_nxtmem, freelst_off);
	update_offset(ar_ptr->ar_prvmem, 0L);
	ar_fseek(fp, offset, 0);
	ar_fwrite((char *)ar_ptr, 1, AR_HSZ, fp);

	/* Update next header if it exists */
	if(freelst_off > 0)
	{
		ar_fseek(fp, freelst_off, 0);
		ar_fread((char *)ar_ptr, 1, AR_HSZ, fp);
		update_offset(ar_ptr->ar_prvmem, offset);
		ar_fseek(fp, freelst_off, 0);
		ar_fwrite((char *)ar_ptr, 1, AR_HSZ, fp);
	}
		
	freelst_off = offset;
	free((char *)ar_ptr);

	return;
}


void unlink_file_mem(ar_fp, hdr)
/*******************************************************************************
	unlink_file_mem - Unlink member from archive file.

	This routine is designed to unlink a file member from the member
	list within the archive file.  It will adjust the headers that point
	to the header passed to this routine.  Input Parameters:
		ar_fp	- File pointer to archive file.
		hdr	- Archive header of member to be unlinked.
*******************************************************************************/
FILE *ar_fp;
struct ar_hdr *hdr;
{
	struct ar_hdr	tmp_hdr;
	long		p_off,
			n_off;
	extern long	memtab_off;

	p_off = atol(hdr->ar_prvmem);
	n_off = atol(hdr->ar_nxtmem);

	if(p_off)	/* Unlink from previous header */
	{
		ar_fseek(ar_fp, p_off, 0);
		ar_fread((char *)&tmp_hdr, 1, AR_HSZ, ar_fp);

		update_offset(tmp_hdr.ar_nxtmem, n_off);

		ar_fseek(ar_fp, p_off, 0);
		ar_fwrite((char *)&tmp_hdr, 1, AR_HSZ, ar_fp);
	}
	else
	{
		if(n_off != memtab_off)
			firstmem_off = n_off;	/* This was the first member */
		else
			firstmem_off = 0;
	}

	if(n_off && (n_off != memtab_off))	/* Unlink from next header */
	{
		ar_fseek(ar_fp, n_off, 0);
		ar_fread((char *)&tmp_hdr, 1, AR_HSZ, ar_fp);

		update_offset(tmp_hdr.ar_prvmem, p_off);

		ar_fseek(ar_fp, n_off, 0);
		ar_fwrite((char *)&tmp_hdr, 1, AR_HSZ, ar_fp);
	}
	else
	{
		lastmem_off = p_off;	/* This was the last member */
	}
}


void set_mod_pos(fp, p_flag, m_name)
/*******************************************************************************
	set_mod_pos - Set global position variables for archive modifications

	This routine is designed to be called before any operations that
	require member positioning are performed.  After calling this 
	routine two global variables 'prev_hdr_off' and 'next_hdr_off'
	will be set.  These two variables are set with respect to the target
	position of the desired operation.
		fp	- File pointer to archive file.
		p_flag	- Positioning flag P_BEFORE for before named member,
			  P_AFTER for after named member,  P_ATEND for at end
			  of archive file.
		m_name	- Member name used for positioning.
*******************************************************************************/
FILE *fp;
int p_flag;
char *m_name;
{
	long		tmp_off;
	struct ar_hdr	*tmp_hdr;
	extern long	lastmem_off;
	extern long	prev_hdr_off, next_hdr_off;
	extern long	want_mem_index;
	extern struct sym_item
			**mem_array;


	/* Save the positioning parameters.  Replace may need them. */
	save_pos_p_flag = p_flag;
	save_pos_m_name = m_name;

	/* If adding to end of file simply set pointers */
	if(p_flag == P_ATEND)
	{
		prev_hdr_off = lastmem_off;
		next_hdr_off = 0;
		want_mem_index = num_of_mem;
	}
	else	/* Search for named member */
	{
		if(tmp_off = find_mem_pos(fp, m_name))
		{
			tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned) AR_HSZ);
			ar_fseek(fp, tmp_off, 0);
			ar_fread((char *)tmp_hdr, 1, AR_HSZ, fp);

			/* Search mem_array for this offset */
			for(want_mem_index = 0;
				mem_array[want_mem_index]->si_offset != tmp_off;
				want_mem_index++);

			if(p_flag == P_BEFORE)		/* P_BEFORE */
			{
				prev_hdr_off = atol(tmp_hdr->ar_prvmem);
				next_hdr_off = tmp_off;
			}
			else if(p_flag == P_AFTER)	/* P_AFTER */
			{
				want_mem_index++;
				prev_hdr_off = tmp_off;
				if(tmp_off == lastmem_off)
					next_hdr_off = 0;
				else
					next_hdr_off = atol(tmp_hdr->ar_nxtmem);
			}
			else if(p_flag == P_REPLACE)	/* P_REPLACE */
			{
				prev_hdr_off = atol(tmp_hdr->ar_prvmem);
				if(tmp_off == lastmem_off)
					next_hdr_off = 0;
				else
					next_hdr_off = atol(tmp_hdr->ar_nxtmem);
			}
		}
		else	/* member name not found */
		{	/* set to end of file */
			want_mem_index = num_of_mem;
			prev_hdr_off = lastmem_off;
			next_hdr_off = 0;
		}
	}

	return;
}


long find_mem_pos(fp, m_name)
/*******************************************************************************
	find_mem_pos - Find member position(offset).

	This routine is designed to return the file offset of the named
	member.  If the in core symbol table exists it will be used to
	find the offset.  If not the archive file must be searched.
	Input Parameters:
		fp	- File pointer to archive file.
		m_name	- Archive member name to be located.
	Return Codes
		n	- File offset to named member.
		0	- If member not found.
*******************************************************************************/
FILE *fp;
char *m_name;
{
	int			tmp_cnt, i;
	long			tmp_off;
	struct ar_hdr		*tmp_hdr;
	extern long		num_of_mem,
				firstmem_off,
				memtab_off;
	extern struct sym_item	**mem_array;

	if(num_of_mem > 0)	/* If present look in in core table */
	{
		for(i = tmp_cnt = 0; tmp_cnt < num_of_mem; i++)
		{
			if(mem_array[i] == NULL)
				continue;
			tmp_cnt++;
			
			if(strcmp(m_name, mem_array[i]->_si_data.si_name))
			{
				continue;	/* No match */
			}
			return(mem_array[i]->si_offset);
		}
		return(0);
	}

	else		/* Scan the archive file */
	{
		tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);
		tmp_off = firstmem_off;

		while((tmp_off > 0) && (tmp_off != memtab_off))
		{
			/* Read the header */
			get_ar_hdr(fp, tmp_off, tmp_hdr);

			/* Check for match in names */
			if(strncmp(m_name, tmp_hdr->_ar_name.ar_name,
			(int)atol(tmp_hdr->ar_namlen)))
			{
				tmp_off = atol(tmp_hdr->ar_nxtmem);
				continue;
			}

			free((char *)tmp_hdr);
			return(tmp_off);
		}
		free((char *)tmp_hdr);
		return(0);
	}
}


void order_ar_file(ar_name)
/*******************************************************************************
	order_ar_file - Order the Archive file.

	This routine is designed to order and compress an archive file.  It
	should only be called after all operations on the archive file have
	completed and all existing data has been written to the archive.
	This routine will create a new archive file with the members being
	put sequentially in order in the file.  All members in the free list
	will also be removed, unless alignment requirements leaves a large
	enough gap.  Input Parameters:
		ar_name	- Name of archive file.
*******************************************************************************/
char *ar_name;
{
	int	i, tmp_cnt,
		bh_boundary;
	char	*m_name,
		*tmp_dir = NULL;
	long	m_namlen,
		m_size,
		prev_off,
		this_off,
		tmp_off,
		firstmem_save,
		memtab_save;
	extern long
		lastmem_off,
		firstmem_off,
		memtab_off;
	FILE	*new_fp,
		*old_fp;
	struct ar_hdr	*tmp_hdr, *prev_hdr;
	struct filehdr	f_hdr;
	short	is_object;
	AOUTHDR	opt_hdr;

	/* Remove members from the member table */
	want_mem_index = num_of_mem = 0;
	if(orig_mem_str != NULL)
	{
		free((char *)orig_mem_str);
		orig_mem_str = NULL;
	}
	if(orig_mem_tab != NULL)
	{
		free((char *)orig_mem_tab);
		orig_mem_tab = NULL;
	}
	if(mem_array != NULL)
	{
		free((char *)mem_array);
		mem_array = NULL;
		mem_ary_size = 0;
	}

	/* Remove symbols from the member and symbol table */
	num_of_sym = 0;
	if(orig_sym_str != NULL)
	{
		free((char *)orig_sym_str);
		orig_sym_str = NULL;
	}
	if(orig_sym_tab != NULL)
	{
		free((char *)orig_sym_tab);
		orig_sym_tab = NULL;
	}
	if(sym_array != NULL)
	{
		free((char *)sym_array);
		sym_array = NULL;
		sym_ary_size = 0;
	}

	/* Remove free members from the free table */
	for(i = tmp_cnt = 0; tmp_cnt < num_of_free; i ++)
	{
		if(free_array[i] == NULL)
			continue;

		tmp_cnt++;
		free((char *)free_array[i]);
		free_array[i] = NULL;
	}
	num_of_free = free_ary_size = 0;
	if(free_array != NULL)
	{
		free((char *)free_array);
		free_array = NULL;
	}

	/* Open existing archive file and save important member offsets */
	flg['c']++;		/* Suppress messages */
	old_fp = create_ar_file(ar_name, "r", NO_CREATE);
	firstmem_save = firstmem_off;
	memtab_save = memtab_off;

	/* Create a new archive file,  overwrites existing offsets */
	if(flg['l'])
		tmp_dir = ".";
	tmp_ar_name = tempnam(tmp_dir, "ar");
	new_fp = create_ar_file(tmp_ar_name, "w+", CREATE);

	tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);
	prev_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);
	m_name = ar_malloc((unsigned)HDR_RDSIZE);
	prev_off = firstmem_off;
	ar_fseek(new_fp, 0L, 2);
	this_off = ROUNDUP(ftell(new_fp));
	tmp_off = firstmem_save;

	/* Now scan the existing archive(in order) and copy members
	   to new archive
	*/
	if(flg['v'])
		ar_fprintf(stderr, MSGSTR(ORDERING_AR_MSG, ORDERING_AR),
			ar_name);
	while((tmp_off > 0) && (tmp_off != memtab_save))
	{
		/* Get old header, seek to data */
		get_ar_hdr(old_fp, tmp_off, tmp_hdr);
		m_namlen = atol(tmp_hdr->ar_namlen);
		m_size = atol(tmp_hdr->ar_size);
		ar_fseek(old_fp, (long)(tmp_off + AR_HSZ +
			ROUNDUP(m_namlen)), 0);

		/* Update member links in new header */
		update_offset(tmp_hdr->ar_prvmem, prev_off);

		/* Check and see if member is an executable object */
		/* Open file, check magic, non-empty symbol tbl, executable */
		/* Note: U800's are for RT.  May no longer be necessary */
		if((fread((void *)&f_hdr, (size_t)1, (size_t)FILHSZ, old_fp) ==
                	FILHSZ) && (f_hdr.f_magic == U802WRMAGIC ||
		f_hdr.f_magic == U802ROMAGIC || f_hdr.f_magic == U802TOCMAGIC ||
		f_hdr.f_magic == U800WRMAGIC || f_hdr.f_magic == U800ROMAGIC ||
		f_hdr.f_magic == U800TOCMAGIC) &&  (F_EXEC & f_hdr.f_flags) )
			is_object = 1;
		else
			is_object = 0;

		/* if an object, determine appropriate offsets */
		if ( (is_object)  &&  (f_hdr.f_opthdr)  &&
			!(fread((void *)&opt_hdr, (size_t)sizeof(AOUTHDR),
				(size_t)1, old_fp) == sizeof(AOUTHDR)) )
		{
			/* align at greater of algntext and algndata */
			bh_boundary = 2 << 
			    (MAX(opt_hdr.o_algntext,opt_hdr.o_algndata) - 1);

			/* don't align if request is bigger than page size */
			if ((bh_boundary > PAGE_SIZE) || (bh_boundary == 0))
				bh_boundary = 2;
		}
		else
			bh_boundary = 2;   /* any even-byte boundary */

		/* Find first free position in the archive file */
		this_off = find_free_pos(new_fp, (long)(ROUNDUP(m_size)+
			ROUNDUP(m_namlen)),m_namlen,bh_boundary);

		/* Write new member header */
		ar_fseek(new_fp, this_off, 0);
		ar_fwrite(tmp_hdr, 1, (int)(AR_HSZ + ROUNDUP(m_namlen)),
			new_fp);

		/* Copy member data to the new file */
		ar_fseek(old_fp, (long)(tmp_off + AR_HSZ +
			ROUNDUP(m_namlen)), 0);
		copy_file(old_fp, new_fp, ROUNDUP(m_size));

		/* Pad with NULL byte if odd member length */
		if(m_size & 0x00000001)
			ar_fwrite((char *)c_zero, 1, 1, new_fp);

		if (prev_off != 0 )
		{			/* Update previous member */
			ar_fseek(new_fp, prev_off, 0);
			ar_fread((char *)prev_hdr, 1, AR_HSZ, new_fp);
			update_offset(prev_hdr->ar_nxtmem, this_off);
			ar_fseek(new_fp, prev_off, 0);
			ar_fwrite((char *)prev_hdr, 1, AR_HSZ, new_fp);
		}
		else   /* first member */
			firstmem_off = this_off;

		/* Add new member to Global Symbol Table */
                /* Create symbol table entries for this member */
                (void)strncpy(m_name,tmp_hdr->_ar_name.ar_name, (int)m_namlen);
                m_name[m_namlen] = '\0';
		add_sym_mem(new_fp, m_name, this_off, m_size);
		want_mem_index++;	/* Add in order to mem_array */

		if(flg['v'])
			ar_fprintf(stdout, VERBOSE_CMD, 'o', m_name);

		/* Update loop offset counters */
		tmp_off = atol(tmp_hdr->ar_nxtmem);
		prev_off = this_off;
		this_off += (AR_HSZ + ROUNDUP(m_namlen) +
			ROUNDUP(m_size));
	}

	lastmem_off = prev_off;
	ar_fseek(new_fp, 0L, 2);
	if (prev_off != 0 )
	{	/* Update previous member; in this case, last member. */
		memtab_off = ftell(new_fp);
		ar_fseek(new_fp, prev_off, 0);
		ar_fread((char *)prev_hdr, 1, AR_HSZ, new_fp);
		update_offset(prev_hdr->ar_nxtmem, memtab_off);
		ar_fseek(new_fp, prev_off, 0);
		ar_fwrite((char *)prev_hdr, 1, AR_HSZ, new_fp);
	}
	else
		memtab_off = 0;		/* No members */

	/* Free memory associated with temporary structures */
	free((char *)tmp_hdr);
	free((char *)prev_hdr);
	free((char *)m_name);

	/* Now write out the symbol table and fixed header for this new file */
	write_mem_sym_tab(new_fp);
	write_fixed_hdr(new_fp);

	/* Copy the new archive file to the old */
	ar_fseek(new_fp, 0L, 2);
	m_size = ftell(new_fp);
	ar_fseek(new_fp, 0L, 0);

	(void)fclose(old_fp);
	old_fp = fopen(ar_name, "w+");

	copy_file(new_fp, old_fp, m_size);

	(void)fclose(new_fp);
	(void)fclose(old_fp);
	(void)unlink(tmp_ar_name);

	return;
}


void add_sym_mem(fp, o_name, o_offset, o_size)
/*******************************************************************************
	add_sym_mem - Add a member to the global symbol table

	This routine is designed to add a new member to the global symbol
	table.  An entry will be placed in the mem_array for the member name.
	Entries will also be added to the sym_array for external symbols
	referenced by this archive member.  Input Parameters:
		fp	- File pointer to the archive file.
		o_name	- Name of the member file.
		o_offset- Offset of member header in archive file.
		o_size	- Size of this member.
*******************************************************************************/
FILE *fp;
char *o_name;
long o_offset;
long o_size;
{
	int		i, /* tmp_cnt, */
			mem_index;
	long		tmp_size,
			obj_start,
			obj_extsyms;
	char 		*obj_syms,
			*obj_strs = NULL,
			*name_ptr;
	SYMENT		sym_str,
			*sym_ptr;
	struct sym_item	**tmp_array;
	struct filehdr	f_hdr;
	extern struct sym_item **mem_array, **sym_array;
	extern long num_of_mem, num_of_sym;

	/* Allocate memory for new entry in the member array */
	mem_index = get_mem_index();

	mem_array[mem_index] = (struct sym_item *)ar_malloc((unsigned)
		sizeof(struct sym_item));
	mem_array[mem_index]->_si_data.si_name = (char *)ar_malloc((unsigned)
		strlen(o_name) + 1);

	(void)strcpy(mem_array[mem_index]->_si_data.si_name, o_name);
	mem_array[mem_index]->si_offset = o_offset;
	num_of_mem++;

	/* Check and see if member is an object file */
	obj_start = o_offset + ROUNDUP(AR_HSZ + strlen(o_name));
	ar_fseek(fp, obj_start, 0);
	if((fread((void *)&f_hdr, (size_t)1, (size_t)FILHSZ, fp) ==
	FILHSZ) && (f_hdr.f_magic == U802WRMAGIC ||
	f_hdr.f_magic == U802ROMAGIC || f_hdr.f_magic == U802TOCMAGIC ||
	f_hdr.f_magic == U800WRMAGIC || f_hdr.f_magic == U800ROMAGIC ||
	f_hdr.f_magic == U800TOCMAGIC) && (f_hdr.f_nsyms != 0)
	&& !(F_LOADONLY & f_hdr.f_flags) )
	{
		/* Member is an object file,  we must read the symbol
		   table in this file and add external entries to the
		   global symbol table.
		*/
		tmp_array = (struct sym_item **)ar_malloc((unsigned)
			(f_hdr.f_nsyms * sizeof(struct sym_item **)));
		obj_extsyms = 0;

		obj_syms = (char *)ar_malloc((unsigned)(f_hdr.f_nsyms *
			SYMESZ));
		
		if((fseek(fp, obj_start + f_hdr.f_symptr, 0) != 0) ||
		(fread((void *)obj_syms, (size_t)1,
			(size_t)(f_hdr.f_nsyms * SYMESZ), fp) !=
			(int)(f_hdr.f_nsyms * SYMESZ)))
		{
			free((char *)tmp_array);
			free((char *)obj_syms);
			ar_error(MSGSTR(CORRUPT_OBJ_MSG, CORRUPT_OBJ), o_name,
				o_name);
			return;
		}

		/* Read entire symbol table */
		if(((ftell(fp) - obj_start + 4) < o_size) &&
		(fread((void *)&tmp_size, (size_t)1, (size_t)sizeof(long), fp)
			== sizeof(long)) && (tmp_size != 0))
		{
			if(((obj_strs = (char *)malloc((unsigned)tmp_size))
			== NULL) || (fread((void *)(obj_strs + sizeof(long)), 
			(size_t)1,
			(size_t)(tmp_size - sizeof(long)), fp) !=
			(size_t)(tmp_size - sizeof(long))))
			{
				free((char *)tmp_array);
				free((char *)obj_syms);
				if(obj_strs != NULL)
					free((char *)obj_strs);
				ar_error(MSGSTR(CORRUPT_OBJ_MSG, CORRUPT_OBJ),
					o_name, o_name);
				return;
			}
		}

		/* Process individual syment entries */
		for(i = 0; i < f_hdr.f_nsyms; i++)
		{
			if(i & 1)	/* Odd symbols aligned by compiler */
			{
				(void)memcpy((char *)&sym_str, (char *)
					obj_syms + (i * SYMESZ), SYMESZ);
				sym_ptr = (SYMENT *)&sym_str;
			}
			else
			{
				sym_ptr = (SYMENT *)(obj_syms +
					(i * SYMESZ));
			}

			/* Check for global defined symbols */
			if(sym_ptr->n_scnum > N_UNDEF &&
			sym_ptr->n_sclass == C_EXT)
			{
				/* If global and defined add the symbol to
				   a temporary array for this object
				*/
				tmp_array[obj_extsyms] = (struct sym_item *)
					ar_malloc((unsigned)
					sizeof(struct sym_item));

				if(sym_ptr->n_zeroes == 0)
					name_ptr = obj_strs + sym_ptr->n_offset;
				else
				{
					name_ptr = sym_ptr->n_name;
					sym_ptr->n_name[8] = '\0';
				}
				tmp_array[obj_extsyms]->_si_data.si_name =
					(char *)ar_malloc((unsigned)
					strlen(name_ptr) + 1);
				(void)strcpy(tmp_array[obj_extsyms]->
					_si_data.si_name, name_ptr);
				tmp_array[obj_extsyms++]->si_offset = o_offset;
			}

			i += sym_ptr->n_numaux;
		}

		if(obj_extsyms > 0)
		{
			/* If global defined symbols found, add to the
			   global symbol table of the archive.
			*/
			if(sym_array == NULL)
			{
				sym_array = (struct sym_item **)ar_malloc(
					(unsigned)((sym_ary_size + obj_extsyms)
					* sizeof(struct sym_item **)));
			}
			else
			{
				sym_array = (struct sym_item **)ar_realloc(
					(char *)sym_array, (unsigned)
					(sym_ary_size + obj_extsyms) *
					sizeof(struct sym_item **));
			}
			num_of_sym += obj_extsyms;

			for(i = 0; i < obj_extsyms; i++)
			{
				sym_array[sym_ary_size++] = tmp_array[i];
			}
		}

		free((char *)tmp_array);
		free((char *)obj_syms);
		if(obj_strs != NULL)
			free((char *)obj_strs);
	}
	return;
}


void rmv_sym_mem(mem_index)
/*******************************************************************************
	rmv_sym_mem - Remove a member from the global symbol table

	This routine is designed to remove references to an archive member
	from the global symbol table.  These references include those in the
	member and symbol arrays.  Input parameters:
		mem_index	- Index to member in member array.
*******************************************************************************/
int mem_index;
{
	int i, tmp_cnt, num_rmvd;
	long tmp_offset = 0;

	/* First remove member from the member array */
	tmp_offset = mem_array[mem_index]->si_offset;
	mem_array[mem_index] = NULL;
	num_of_mem--;

	/* Now search the symbol array for entries that contain
	   the same offset.
	*/
	num_rmvd = 0;
	for(i = tmp_cnt = 0; tmp_cnt < num_of_sym; i++)
	{
		if(sym_array[i] == NULL)
		{
			continue;
		}

		tmp_cnt++;
		if(sym_array[i]->si_offset == tmp_offset)
		{
			/* If offset matches, remove them */
			while(i < sym_ary_size)
			{
				if(sym_array[i]->si_offset == tmp_offset)
				{
					sym_array[i++] = NULL;
					num_rmvd++;
				}
				else
					break;
			}
			break;
		}
	}
	num_of_sym -= num_rmvd;

	return;
}


void get_mem_sym_tab(fp, mem_off, sym_off)
/*******************************************************************************
	get_mem_sym_tab - Read in the global member and symbol tables

	This routine is responsible for reading the global member and
	symbol table members loacted at 'mem_off' and 'sym_off'.  After
	reading the tables this routine will initialize the global arrays
	'mem_array' and 'sym_array' with the information contained in
	the tables.  Input parameters:
		fp	- File pointer to archive.
		mem_off	- File offset where the member table is located.
		sym_off	- File offset where the member table is located.
*******************************************************************************/
FILE *fp;
long mem_off, sym_off;
{
	struct ar_hdr *tmp_hdr;
	char *mem_n_ptr, *sym_n_ptr;
	char tmp_num_mem[12];
	long tmp_num_sym;
	char *mem_i_ptr;
	long *sym_i_ptr;
	long tmp_size, tmp_number;
	extern struct sym_item **mem_array, **sym_array;

	/* See if member table exists */
	if(mem_off == 0)
		return;

	/* Read in member header for member table */
	tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);
	ar_fseek(fp, mem_off, 0);
	ar_fread((char *)tmp_hdr, 1, AR_HSZ, fp);

	/* Read in number of members */
	ar_fread((char *)tmp_num_mem, 1, (int)12, fp);

	/* Convert number and size to a machine readable longs */
	tmp_number = atol((char *)tmp_num_mem);
	tmp_size = atol((char *)tmp_hdr->ar_size);

	(void)free((char *)tmp_hdr);

	/* Read in entire member table */
	orig_mem_tab = (char *)ar_malloc((unsigned)(tmp_size - 12));
	ar_fread((char *)orig_mem_tab, 1, (int)(tmp_size -12), fp);


	/* Get memory for global arrays */
	mem_array = (struct sym_item **)ar_malloc((unsigned)tmp_number *
		sizeof(struct sym_item *));
	orig_mem_str = (struct sym_item *)ar_malloc((unsigned)tmp_number *
		sizeof(struct sym_item));
	mem_ary_size = tmp_number;

	/* Initialize loop control variables */
	mem_i_ptr = (char *)(orig_mem_tab);
	mem_n_ptr = orig_mem_tab + ((long)12 * mem_ary_size);
	num_of_mem = 0;

	while(tmp_number--)	/* Set up the member indexes */
	{
		mem_array[num_of_mem] = orig_mem_str++;
		mem_array[num_of_mem]->si_offset = atol((char *)mem_i_ptr);
		mem_i_ptr += 12;
		mem_array[num_of_mem]->_si_data.si_name = mem_n_ptr;
		mem_n_ptr += (strlen(mem_n_ptr) + 1);
		num_of_mem++;
	}


	/* Now process the symbol table in a similar fashion */
	/* See if symbol table exists */
	if(sym_off == 0)
		return;

	/* Read in member header for member table */
	tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)HDR_RDSIZE);
	ar_fseek(fp, sym_off, 0);
	ar_fread((char *)tmp_hdr, 1, AR_HSZ, fp);

	/* Read in number of members */
	ar_fread((char *)&tmp_num_sym, 1, sizeof(long), fp);

	/* Convert number and size to machine readable longs */
	tmp_number = sgetl((char *)&tmp_num_sym);
	tmp_size = atol((char *)tmp_hdr->ar_size);

	(void)free((char *)tmp_hdr);

	/* Read in entire member table */
	orig_sym_tab = (char *)ar_malloc((unsigned)(tmp_size - sizeof(long)));
	ar_fread((char *)orig_sym_tab, 1, (int)(tmp_size - sizeof(long)), fp);

	/* Get memory for global arrays */
	sym_array = (struct sym_item **)ar_malloc((unsigned)tmp_number *
		sizeof(struct sym_item *));
	orig_sym_str = (struct sym_item *)ar_malloc((unsigned)tmp_number *
		sizeof(struct sym_item));
	sym_ary_size = tmp_number;

	/* Initialize loop control variables */
	sym_i_ptr = (long *)(orig_sym_tab);
	sym_n_ptr = orig_sym_tab + (sizeof(long) * sym_ary_size);
	num_of_sym = 0;

	while(tmp_number--)	/* Set up the member indexes */
	{
		sym_array[num_of_sym] = orig_sym_str++;
		sym_array[num_of_sym]->si_offset = sgetl((char *)sym_i_ptr++);
		sym_array[num_of_sym]->_si_data.si_name = sym_n_ptr;
		sym_n_ptr += (strlen(sym_n_ptr) + 1);
		num_of_sym++;
	}
	return;
}


void get_free_lst(fp, offset)
/*******************************************************************************
	get_free_lst - Read in the free list from archive file

	This routine is responsible for reading in the free list from the
	archive file.  It will build an in memory index to the free list
	which can be used to locate members of a particular size.
	Input Parameters:
		fp	- File pointer to archive file.
		offset	- Offset to first member on the free list.
*******************************************************************************/
FILE *fp;
long offset;
{
	extern struct sym_item **free_array;
	extern long	num_of_free,
			free_ary_size;
	int		i;
	struct ar_hdr 	*tmp_hdr;


	if(free_array)
		free((char *)free_array);
	num_of_free = 0;

	free_array = (struct sym_item **)ar_malloc((unsigned)
		sizeof(struct sym_item) * BUMP_VAL);
	free_ary_size = BUMP_VAL;

	tmp_hdr = (struct ar_hdr *)ar_malloc((unsigned)AR_HSZ);

	while(offset > 0)	/* Read until end of list */
	{
		ar_fseek(fp, offset, 0);	/* Read header */
		if(fread((void *)tmp_hdr, (size_t)1, (size_t)AR_HSZ, fp) !=
		AR_HSZ)
		{
			ar_exit(MSGSTR(READ_ERR_MSG, READ_ERR));
		}

		/* Add area to the free list */
		free_array[num_of_free] = (struct sym_item *)
			ar_malloc((unsigned)sizeof(struct sym_item));
		free_array[num_of_free]->si_offset = offset;
		free_array[num_of_free]->_si_data.si_size =
			atol(tmp_hdr->ar_size);
		num_of_free++;

		/* Bump area if necessary */
		if(num_of_free == free_ary_size)
		{
			free_array = (struct sym_item **)ar_realloc((char *)
				free_array, (unsigned)(free_ary_size *
				sizeof(struct sym_item *) + BUMP_VAL *
				sizeof(struct sym_item *)));
			free_ary_size += BUMP_VAL;
		}

		offset = atol(tmp_hdr->ar_nxtmem);
	}

	free((char *)tmp_hdr);

	/* Null fill extra pointers in free array */
	for(i = num_of_free; i < free_ary_size; i++)
	{
		free_array[i] = NULL;
	}

	return;
}


void asc_bin_hdr(b_hdr, a_hdr)
/*******************************************************************************
	asc_bin_hdr - convert header from ascii to binary format

	This routine is designed to convert an archive header from its
	original ascii format to an internal binary format.
	Input parameters:
		b_hdr	- Pointer to binary header structure.
		a_hdr	- Pointer to ascii header structure.
*******************************************************************************/
struct bin_hdr *b_hdr;
struct ar_hdr *a_hdr;
{
	b_hdr->bh_size = atol((char *)a_hdr->ar_size);
	b_hdr->bh_nxtmem = atol((char *)a_hdr->ar_nxtmem);
	b_hdr->bh_nxtmem = atol((char *)a_hdr->ar_prvmem);
	b_hdr->bh_date = atol((char *)a_hdr->ar_date);
	b_hdr->bh_uid = atol((char *)a_hdr->ar_uid);
	b_hdr->bh_gid = atol((char *)a_hdr->ar_gid);
	b_hdr->bh_mode = strtol((char *)a_hdr->ar_mode, NULL, 8);
	b_hdr->bh_namlen = atol((char *)a_hdr->ar_namlen);
	b_hdr->bh_name = (char *)ar_malloc((unsigned)b_hdr->bh_namlen + 1);
	(void)strncpy(b_hdr->bh_name, a_hdr->_ar_name.ar_name,
		b_hdr->bh_namlen);
	*(b_hdr->bh_name + b_hdr->bh_namlen) = '\0';
	
	return;
}


void bin_asc_hdr(b_hdr, a_hdr)
/*******************************************************************************
	bin_asc_hdr - convert header from binary to ascii format

	This routine is designed to convert an archive header in internal
	binary format to the standard ascii format.  Input parameters:
		b_hdr	- Pointer to binary header structure.
		a_hdr	- Pointer to ascii header structure.
*******************************************************************************/
struct bin_hdr *b_hdr;
struct ar_hdr *a_hdr;
{
	(void)sprintf((char *)a_hdr,
		"%-*ld%-*ld%-*ld%-*ld%-*ld%-*ld%-*o%-*d%s",
		sizeof(a_hdr->ar_size), b_hdr->bh_size,
		sizeof(a_hdr->ar_nxtmem), b_hdr->bh_nxtmem, 
		sizeof(a_hdr->ar_prvmem), b_hdr->bh_prvmem,
		sizeof(a_hdr->ar_date), b_hdr->bh_date,
		sizeof(a_hdr->ar_uid), b_hdr->bh_uid,
		sizeof(a_hdr->ar_gid), b_hdr->bh_gid, 
		sizeof(a_hdr->ar_mode), b_hdr->bh_mode,
		sizeof(a_hdr->ar_namlen), b_hdr->bh_namlen,
		b_hdr->bh_name);
	(void)strcpy((char *)(a_hdr->_ar_name.ar_name +
		ROUNDUP(strlen(b_hdr->bh_name))), AIAFMAG);

	return;
}


void get_ar_hdr(fp, offset, hdr_ptr)
/*******************************************************************************
	get_ar_hdr - get archive header

	This routine is designed to retrieve the archive header from a
	file.  The routine will be passed two parameters:
		fp 	- Open file pointer for the file to be read.
		offset	- File offset where the archive header begins.
		hdr_ptr	- Area used to store archive header(Note: this must
			  be large enough for the member name).
*******************************************************************************/
FILE *fp;
long offset;
struct ar_hdr *hdr_ptr;
{
	int n_read;
	long n_length;
	char *read_ptr = (char *)hdr_ptr;
	static int read_size = MIN_RDSIZE;

	ar_fseek(fp, offset, 0);
	n_read = fread((void *)read_ptr, (size_t)1, (size_t)read_size, fp);

	if(n_read < AR_HSZ)
	{
		ar_exit(MSGSTR(BAD_HEADER_MSG, BAD_HEADER));
	}

	if((n_length = atol((char *)hdr_ptr->ar_namlen)) >
	(read_size - AR_HSZ))
	{
		n_read += fread((void *)(read_ptr + read_size), (size_t)1, 
		    (size_t)ROUNDUP(n_length - (read_size - AR_HSZ)), fp);
		read_size = ROUNDUP(n_length + AR_HSZ);
	}
		
	if(n_read < (AR_HSZ + ROUNDUP(n_length)))
	{
		ar_exit(MSGSTR(BAD_HEADER_MSG, BAD_HEADER));
	}

	else
		return;
}


void read_fixed_hdr(fp)
/*******************************************************************************
	read_fixed_hdr - Read the fixed header of the archive file.

	This routine is designed to simply read the fixed length header
	at the beginning of the archive file.  It will store the values
	read in global variables.  Input Paramaters:
		fp -	File pointer to the archive file.
*******************************************************************************/
FILE *fp;
{
	struct fl_hdr	fl_hdr;
	extern long	memtab_off,
			symtab_off,
			firstmem_off,
			lastmem_off,
			freelst_off;

	ar_fseek(fp, 0L, 0);
	if(fread((void *)&fl_hdr, (size_t)1, (size_t)FL_HSZ, fp) != FL_HSZ)
		ar_exit(MSGSTR(FIXHDR_ERR_MSG, FIXHDR_ERR), ar_fname);

	/* check magic */
	if(strncmp(fl_hdr.fl_magic, AIAMAG, SAIAMAG))
	{
		ar_exit(MSGSTR(MAGIC_ERR_MSG, MAGIC_ERR), ar_fname);
	}

	memtab_off = atol(fl_hdr.fl_memoff);
	symtab_off = atol(fl_hdr.fl_gstoff);
	firstmem_off = atol(fl_hdr.fl_fstmoff);
	lastmem_off = atol(fl_hdr.fl_lstmoff);
	freelst_off = atol(fl_hdr.fl_freeoff);

	if((firstmem_off != 0)  && (memtab_off == 0))
	{
		ar_exit(MSGSTR(NO_MEM_TAB_MSG, NO_MEM_TAB), ar_fname, ar_fname);
	}

	return;
}


FILE *create_ar_file(name, o_flag, c_flag)
/*******************************************************************************
	create_ar_file - Create archive file.

	This routine is designed to simply create an archive file.  It will
	first test the named file.  If it is an archive file it will immediatly
	return.  If it exists and is not an archive file an error message
	will be written and the program will terminate.  If the file does
	not exist it is created and the archive magic string is written to it.
	Input Parameters:
		name	- Name of archive file.
		o_flag	- Flag to indicate how file should be opened.
	Return Codes:
		fp	- File pointer to open archive file.
*******************************************************************************/
char	*name,
	*o_flag,
	c_flag;
{
	FILE		*fp;
	struct stat	statbuf;

	/* check file existence */
	ar_fname = name;
	if(stat(name, (struct stat *)&statbuf))
	{
		/* Test create flag */
		if(c_flag == NO_CREATE)
		{
			ar_exit(MSGSTR(AR_NOEXIST_MSG, AR_NOEXIST), name);
		}

		/* File doesn't exist, create and open it */
		(void)close(creat(name, 0666));
		if((fp = fopen(name, o_flag)) == NULL)
		{
			ar_exit(MSGSTR(OPEN_ERR_MSG, OPEN_ERR), name);
		}
		new_archive++;

		/* Initialize fixed header global values, and write */
		firstmem_off = lastmem_off = memtab_off =
		symtab_off = freelst_off = 0;
		write_fixed_hdr(fp);

		if(!flg['c'])
		{
			ar_fprintf(stderr, MSGSTR(AR_CREATING_MSG,
				AR_CREATING), name);
		}
	}

	else	/* File already exists */
	{
		if((fp = fopen(name, o_flag)) == NULL)
		{
			ar_exit(MSGSTR(OPEN_ERR_MSG, OPEN_ERR), name);
		}

		/* read in the fixed header */
		read_fixed_hdr(fp);
	}

	return(fp);
}


void write_file_mode(mode)
/*******************************************************************************
	write_file_mode - Write mode of file in long form.

	This routine is designed to write the file mode in the long
	expanded format(like ls -l).  It will simply test bits of the
	mode to determine what characters to write.  Input Parameters:
		mode	- Numeric file mode.
*******************************************************************************/
unsigned mode;
{
	/* Owner permissions */
	if(mode & S_IRUSR) (void)putchar(READ_MODE);
	else (void)putchar(NULL_MODE);
	if(mode & S_IWUSR) (void)putchar(WRITE_MODE);
	else (void)putchar(NULL_MODE);
	if(mode & S_ISUID) (void)putchar(SUID_MODE);
	else if(mode & S_IXUSR) (void)putchar(EXEC_MODE);
	else (void)putchar(NULL_MODE);

	/* Group permissions */
	if(mode & S_IRGRP) (void)putchar(READ_MODE);
	else (void)putchar(NULL_MODE);
	if(mode & S_IWGRP) (void)putchar(WRITE_MODE);
	else (void)putchar(NULL_MODE);
	if(mode & S_ISGID) (void)putchar(SGID_MODE);
	else if(mode & S_IXGRP) (void)putchar(EXEC_MODE);
	else (void)putchar(NULL_MODE);

	/* Other permissions */
	if(mode & S_IROTH) (void)putchar(READ_MODE);
	else (void)putchar(NULL_MODE);
	if(mode & S_IWOTH) (void)putchar(WRITE_MODE);
	else (void)putchar(NULL_MODE);
	if(mode & S_ISVTX) (void)putchar(SVTXT_MODE);
	else if(mode & S_IXOTH) (void)putchar(EXEC_MODE);
	else (void)putchar(NULL_MODE);

	return;
}


void write_fixed_hdr(fp)
/*******************************************************************************
	write_fixed_hdr - Write the fixed header to the archive file

	This routine is designed to simply write the fixed length header
	to the archive.  The values used for this header will come from
	the global variables.  Input Parameters:
		fp	- FIle pointer to the archive file.
*******************************************************************************/
FILE *fp;
{
	struct fl_hdr	*fl_ptr;
	extern long	memtab_off,
			symtab_off,
			firstmem_off,
			lastmem_off,
			freelst_off;

	fl_ptr = (struct fl_hdr *)ar_malloc((unsigned)
		FL_HSZ + 1);
	(void)sprintf((char *)fl_ptr, "%s%-*ld%-*ld%-*ld%-*ld%-*ld", 
		AIAMAG, sizeof(fl_ptr->fl_memoff), memtab_off,
		sizeof(fl_ptr->fl_gstoff), symtab_off,
		sizeof(fl_ptr->fl_fstmoff), firstmem_off,
		sizeof(fl_ptr->fl_lstmoff), lastmem_off,
		sizeof(fl_ptr->fl_freeoff), freelst_off);

	ar_fseek(fp, 0L, 0);
	ar_fwrite((char *)fl_ptr, 1, FL_HSZ, fp);

	free((char *)fl_ptr);

	return;
}


void zero_fixed_hdr(fp)
/*******************************************************************************
	zero_fixed_hdr - zero fill the fixed header

	This routine is simply designed to zero out the fixed header
	before any modifications are made to the archive file.  This
	will prevent the archive from becoming corrupted if the ar
	process is interrupted.  This routine should only be called
	if modifications are being made to internal archive file pointers.
	The routine will also zero the ar_nxtmem field of the last member
	of the archive.  In effect the link from the last member to the
	member table will be severed.  If the link is not removed some
	routines may create circular or NULL links.

	Input Parameters:
		fp	- Archive file file pointer.
*******************************************************************************/
FILE *fp;
{
	extern long	memtab_off,
			symtab_off,
			firstmem_off,
			lastmem_off,
			freelst_off;
	long		tmp_memtab_off,
			tmp_symtab_off,
			tmp_firstmem_off,
			tmp_lastmem_off,
			tmp_freelst_off;
	struct ar_hdr	tmp_hdr;

	/* Save offsets */
	tmp_memtab_off = memtab_off;
	tmp_symtab_off = symtab_off;
	tmp_firstmem_off = firstmem_off;
	tmp_lastmem_off = lastmem_off;
	tmp_freelst_off = freelst_off;

	/* Zero offsets and rewrite header */
	memtab_off = symtab_off = firstmem_off = lastmem_off =freelst_off = 0;
	write_fixed_hdr(fp);
	if(fflush(fp) == EOF)
		ar_exit(MSGSTR(FFLUSH_ERR_MSG, FFLUSH_ERR));

	/* Restore offsets */
	memtab_off = tmp_memtab_off;
	symtab_off = tmp_symtab_off;
	firstmem_off = tmp_firstmem_off;
	lastmem_off = tmp_lastmem_off;
	freelst_off = tmp_freelst_off;

	/* Remove link from last member to member table */
	if (lastmem_off != 0)
	{
		ar_fseek(fp, lastmem_off, 0);
		ar_fread((char *)&tmp_hdr, 1, AR_HSZ, fp);
		update_offset(tmp_hdr.ar_nxtmem, 0L);
		ar_fseek(fp, lastmem_off, 0);
		ar_fwrite((char *)&tmp_hdr, 1, AR_HSZ, fp);
	}

	return;
}


void clear_free_list(fp)
/*******************************************************************************
	clear_free_list - Return free list space to the filesystem.

	This routine is designed to return the space occupied by the
	free list to the filesystem.  run the free list snf call fclear
	for the new members added to the list.  Input Parameters:
		fp	- File pointer to archive file.
*******************************************************************************/
FILE *fp;
{
	struct ar_hdr	*tmp_hdr;
	long		tmp_off;

	tmp_hdr = (struct ar_hdr *)ar_malloc(AR_HSZ);

	tmp_off = freelst_off;
	while(new_free && (tmp_off > 0))	/* Scan free list */
	{
		ar_fseek(fp, tmp_off, 0);
		ar_fread((char *)tmp_hdr, 1, AR_HSZ, fp);

		ar_fclear(fp, (long)(tmp_off + AR_HSZ),
			atol(tmp_hdr->ar_size));

		tmp_off = atol(tmp_hdr->ar_nxtmem);
	}

	free((char *)tmp_hdr);

	return;
}


long get_mem_index()
/******************************************************************************
	get_mem_index - get index for next member in table

	This routine is designed to return the desired index in the member
	table.  It will increase the size of the member array if necessary.
	The global variable want_mem_index will contain the index that is
	desired.  If necessary other member indexes will be shifted to
	free up this slot.  Input Parameters
		None
	Return Codes:
		Returnes the desired member index
*******************************************************************************/
{
	long		save,
			i;
	struct sym_item *tmp_ptr;

	extern long	want_mem_index,
			mem_ary_size,
			num_of_mem;
	extern struct sym_item
			**mem_array;
	
	/* Allocate memory for new entry in the member array */
	if(mem_array == NULL)	/* If empty allocate area */
	{
		mem_array = (struct sym_item **)ar_malloc((unsigned)(BUMP_VAL *
			sizeof(struct sym_item *)));
		mem_ary_size += BUMP_VAL;
		for(i = num_of_mem; i < mem_ary_size; i++)
			mem_array[i] = NULL;
	}
	else if((num_of_mem == mem_ary_size) ||		/* If full bump area */
		(want_mem_index >= mem_ary_size))
	{
		mem_array = (struct sym_item **)ar_realloc((char *)mem_array,
			(unsigned)((mem_ary_size + BUMP_VAL) *
			sizeof(struct sym_item *)));
		mem_ary_size += BUMP_VAL;
		for(i = num_of_mem; i < mem_ary_size; i++)
			mem_array[i] = NULL;
	}

	save = i = want_mem_index;
	while(mem_array[save] != NULL)
	{
		tmp_ptr = mem_array[i + 1];
		mem_array[i + 1] = mem_array[save];
		mem_array[save] = tmp_ptr;
		i++;
	}

	return(want_mem_index);
}


void write_mem_sym_tab(fp)
/*******************************************************************************
	write_mem_sym_tab - Write the member and symbol tables to file.

	This routine is designed to write the member and symbol tables to
	the archive file.  The information for these tables is kept in core
	during file processing.  This information will be converted to a
	machine independent format and then written to the archive.  Input
	Parameters:
		fp	- File pointer to the archive file.
*******************************************************************************/
FILE *fp;
{
	int			i;
	long			start_of_tab,
				mem_str_size,
				sym_str_size,
				tmp_size,
				tmp_cnt,
				num_sym_buf;
	char			num_mem_buf[16];
	struct ar_hdr		*a_ptr;
	extern long		num_of_mem,
				num_of_sym,
				lastmem_off,
				memtab_off,
				symtab_off,
				cur_date;
	extern struct sym_item	**mem_array,
				**sym_array;


	/* First write out the member table.  This table will always
	 * be present if the archive file in not empty.
	 */
	/* Check for in core table,  If not present nothing to do */
	if(mem_array == NULL) 
		return;

	/* If no members in archive file remove member table */
	if(num_of_mem == 0)
	{
		if(memtab_off != 0)	/* Truncate file */
		{
			ar_ftruncate(fp, memtab_off);
			memtab_off = 0;
		}
		return;
	}

	/* Find location at which to start writing the member table */
	if(memtab_off == 0)
	{
		ar_fseek(fp, 0L, 2);
		memtab_off = ROUNDUP(ftell(fp));
	}
	else
	{
		memtab_off = ROUNDUP(memtab_off);
	}
	start_of_tab = memtab_off + AR_HSZ;
	if(fseek(fp, (long)start_of_tab, 0))
	{
		ar_ftruncate(fp, memtab_off);
		memtab_off = 0;
		ar_error(MSGSTR(SYM_WRERROR_MSG, SYM_WRERROR));
		return;
	}


	/* Write the number of members to the table */
	(void)sprintf((char *)num_mem_buf, "%-12ld", num_of_mem);
	if (fwrite((void *)num_mem_buf, (size_t)1, (size_t)12, fp) != (int)12)
	{
		write_mem_sym_error(fp, &memtab_off, &lastmem_off);
		return;
	}


	/* Convert member offsets to ascii and write them out */
	for(i = tmp_cnt = 0; tmp_cnt < num_of_mem; i++)
	{
		if(mem_array[i] == NULL)
		{
			continue;	/* No data at this entry */
		}

		(void)sprintf((char *)num_mem_buf, "%-12ld",
			mem_array[i]->si_offset);

		if (fwrite((void *)num_mem_buf, (size_t)1, (size_t)12, fp)
			!= (int)12)
		{
			write_mem_sym_error(fp, &memtab_off, &lastmem_off);
			return;
		}

		tmp_cnt++;
	}

	/* Next the member name strings will be written out */
	for(i = tmp_cnt = mem_str_size = 0; tmp_cnt < num_of_mem; i++)
	{
		if(mem_array[i] == NULL)
		{
			continue;
		}

		tmp_size = strlen(mem_array[i]->_si_data.si_name) + 1;

		if(fwrite((void *)mem_array[i]->_si_data.si_name, (size_t)1,
		(size_t)tmp_size, fp) != (size_t)tmp_size)
		{
			write_mem_sym_error(fp, &memtab_off, &lastmem_off);
			return;
		}

		mem_str_size += tmp_size;
		tmp_cnt++;
	}

	/* Create and write out the member header for the table */
	a_ptr = (struct ar_hdr *)ar_malloc((unsigned)AR_HSZ + 1);
	(void)sprintf((char *)a_ptr,
		"%-*ld%-*ld%-*ld%-*ld%-*ld%-*ld%-*o%-*d%-*s",
		sizeof(a_ptr->ar_size), (int)12 + (num_of_mem * (int)12) +
		mem_str_size,
		sizeof(a_ptr->ar_nxtmem), 0L, 
		sizeof(a_ptr->ar_prvmem), lastmem_off,
		sizeof(a_ptr->ar_date), cur_date,
		sizeof(a_ptr->ar_uid), 0,
		sizeof(a_ptr->ar_gid), 0,
		sizeof(a_ptr->ar_mode), 0,
		sizeof(a_ptr->ar_namlen), 0,
		sizeof(a_ptr->_ar_name), AIAFMAG);

	ar_fseek(fp, memtab_off, 0);
	if(fwrite((void *)a_ptr, (size_t)1, (size_t)AR_HSZ, fp) != AR_HSZ)
		write_mem_sym_error(fp, &memtab_off, &lastmem_off);

	/* Keep track of where the symbol may go */
	symtab_off = memtab_off + AR_HSZ + ROUNDUP((int)12 +
		(num_of_mem * (int)12) + mem_str_size);


	/* Update the last member to point to this table */
	if(lastmem_off > 0)
	{
		ar_fseek(fp, lastmem_off, 0);
		ar_fread((char *)a_ptr, 1, AR_HSZ, fp);
		update_offset(a_ptr->ar_nxtmem, memtab_off);
		ar_fseek(fp, lastmem_off, 0);
		ar_fwrite((char *)a_ptr, 1, AR_HSZ, fp);
	}
	free((char *)a_ptr);


	/* Now check to see if a symbol table is present,  if so 
	 * write it out in the same manner as the member table.
	 */
	/* If no symbols in archive file remove symbol table */
	if(num_of_sym == 0)
	{
		/* Truncate file */
		ar_ftruncate(fp, symtab_off);
		symtab_off = 0;
		return;
	}

	/* Find location at which to start writing the symbol table */
	start_of_tab = symtab_off + AR_HSZ;
	if(fseek(fp, (long)start_of_tab, 0))
	{
		ar_ftruncate(fp, symtab_off);
		symtab_off = 0;
		ar_error(MSGSTR(SYM_WRERROR_MSG, SYM_WRERROR));
		return;
	}


	/* Write the number of symbols to the table */
	(void)sputl(num_of_sym, (char *)&num_sym_buf);
	if (fwrite((void *)&num_sym_buf, (size_t)1, (size_t)sizeof(long), fp)
		!= sizeof(long))
	{
		write_mem_sym_error(fp, &symtab_off, &memtab_off);
		return;
	}

	/* Convert symbol offsets to machine independent longs and write out */
	for(i = tmp_cnt = 0; tmp_cnt < num_of_sym; i++)
	{
		if(sym_array[i] == NULL)
		{
			continue;	/* No data at this entry */
		}

		(void)sputl(sym_array[i]->si_offset, (char *)
			&(sym_array[i]->si_offset));

		if(fwrite((void *)&(sym_array[i]->si_offset), (size_t)1,
		(size_t)sizeof(long), fp) != (size_t)sizeof(long))
		{
			write_mem_sym_error(fp, &symtab_off, &memtab_off);
			return;
		}

		tmp_cnt++;
	}

	/* Next write out the strings associated with the symbol table */
	for(i = tmp_cnt = sym_str_size = 0; tmp_cnt < num_of_sym; i++)
	{
		if(sym_array[i] == NULL)
		{
			continue;
		}

		tmp_size = strlen(sym_array[i]->_si_data.si_name) + 1;

		if(fwrite((void *)sym_array[i]->_si_data.si_name, (size_t)1,
		(size_t)tmp_size, fp) != (size_t)tmp_size)
		{
			write_mem_sym_error(fp, &symtab_off, &memtab_off);
			return;
		}

		sym_str_size += tmp_size;
		tmp_cnt++;
	}


	/* Create and write out the member header for the table */
	a_ptr = (struct ar_hdr *)ar_malloc((unsigned)AR_HSZ + 1);
	(void)sprintf((char *)a_ptr,
		"%-*ld%-*ld%-*ld%-*ld%-*ld%-*ld%-*o%-*d%-*s",
		sizeof(a_ptr->ar_size), sizeof(long) +
		(num_of_sym * sizeof(long)) + sym_str_size,
		sizeof(a_ptr->ar_nxtmem), 0L,
		sizeof(a_ptr->ar_prvmem), memtab_off,
		sizeof(a_ptr->ar_date), cur_date,
		sizeof(a_ptr->ar_uid), 0,
		sizeof(a_ptr->ar_gid), 0,
		sizeof(a_ptr->ar_mode), 0,
		sizeof(a_ptr->ar_namlen), 0,
		sizeof(a_ptr->_ar_name), AIAFMAG);

	ar_fseek(fp, symtab_off, 0);
	ar_fwrite((char *)a_ptr, 1, AR_HSZ, fp);

	/* Update the member header of the member table to point here */
	ar_fseek(fp, memtab_off, 0);
	ar_fread((char *)a_ptr, 1, AR_HSZ, fp);
	update_offset(a_ptr->ar_nxtmem, symtab_off);
	ar_fseek(fp, memtab_off, 0);
	ar_fwrite((char *)a_ptr, 1, AR_HSZ, fp);

	free((char *)a_ptr);

	return;
}
void write_mem_sym_error(fp, table_off, previous_off)
/*
	Error handling routine for write_mem_sym_tab. 
*/
FILE *fp;
long *table_off, *previous_off;
{
	struct ar_hdr		*a_ptr;

	a_ptr = (struct ar_hdr *)ar_malloc((unsigned)AR_HSZ + 1);
	(void)ftruncate(fileno(fp), *table_off);
	*table_off = 0;
	if(*previous_off > 0)
	{
		ar_fseek(fp, previous_off, 0);
		ar_fread((char *)a_ptr, 1, AR_HSZ, fp);
		update_offset(a_ptr->ar_nxtmem, *table_off);
		ar_fseek(fp, *previous_off, 0);
		ar_fwrite((char *)a_ptr, 1, AR_HSZ, fp);
	}
	ar_error(MSGSTR(SYM_WRERROR_MSG, SYM_WRERROR));
	free((char *)a_ptr);
	return;
}


void copy_file(in_fp, out_fp, size)
/*******************************************************************************
	copy_file - Copy input file to output file.

	This routine is designed to copy the input file to the output file.
	Each file pointer must be positioned to the appropiate position in
	the file before calling this routine.  Input Parameters:
		in_fp	- Input file pointer.
		out_fp	- Output file pointer.
		size	- Amount of data to copy.
*******************************************************************************/
FILE *in_fp, *out_fp;
long size;
{
	static char	*buf = NULL;
	register long	count;

	if(buf == NULL)
	{
		buf = ar_malloc((unsigned)BUFSIZ);
	}

	while(size)
	{
#ifdef	LIBCIO_FIX
		size -= (count = size < ONE_K ? size : ONE_K);
#else	/* LIBCIO_FIX */
		size -= (count = size < BUFSIZ ? size : BUFSIZ);
#endif	/* LIBCIO_FIX */
		ar_fread(buf, 1, (int)count, in_fp);
		ar_fwrite((char *)buf, 1, (int)count, out_fp);
	}

	return;
}


char *trim(path)
/*******************************************************************************
	trim - Trim pathname to leaf node.

	This routine is designed to reduce a file pathname to it's last
	leaf node.  This is the name stored in the archive.
	Input Parameters:
		path	- File Pathname
	Return Codes:
		path	- Trimed to leaf node
*******************************************************************************/
char *path;
{
	register char *p1, *p2;

	for(p1 = path; *p1; p1++)
		;

	while (p1 > path)
	{
		if (*--p1 != '/')
			break;
		*p1 = 0;
	}

	p2 = path;
	for (p1 = path; *p1; p1++)
	{
		if (*p1 == '/')
			p2 = p1 + 1;
	}
	return (p2);
}


void update_offset(string, value)
/*******************************************************************************
	update_offset - Update offset value in archive header.

	This routine is responsible for updating the offset fields of an
	archive member header.  It serves as a front end for the sprintf
	command which may overwrite data with the trailing '\0' character.
	Input Parameters:
		string	- Pointer to character string to contain offset.
		value	- Value to store at offset.
*******************************************************************************/
char *string;
long value;
{
	char buf[16];

	(void)sprintf(buf, "%-12ld", value);
	(void)strncpy(string, buf, 12);

	return;
}


FILE *safe_copy(f_fp, in_or_out)
/*******************************************************************************
	safe_copy - Create a safe working copy.

	This routine will simply create a safe temoporary copy of the archive.
	Input parameters are the following.
		f_fp	-	File pointer to file to copy in or out.
		in_or_out - 	Flag to indicate copy in or out of temp file.
	Return Codes:
		fp	-	File pointer to working copy of archive.
		NULL	_	In case of error.
*******************************************************************************/
FILE *f_fp;
int in_or_out;
{
	static FILE	*save_fp = NULL;
	FILE		*tmp_fp;
	long		tmp_size;
	int		new_save;
	char		*tmp_dir = NULL,
			flg_save;

	if(in_or_out == COPY_IN)	/* Copy in requires file creation */
	{
		/* If we just created new archive don't copy just return */
		if(new_archive)
			return(f_fp);

		/* Save pointers and flags */
		save_fp = f_fp;
		flg_save = flg['c'];
		flg['c'] = 1;

		/* Create and open the temporary safe copy file */
		new_save = new_archive;
		if(flg['l'])
			tmp_dir = ".";
		else
			tmp_dir = getenv("TMPDIR");
		tmp_ar_name = tempnam(tmp_dir, "ar");
		if((tmp_fp = fopen(tmp_ar_name, "w+")) == NULL)
		{
			ar_exit(MSGSTR(OPEN_ERR_MSG, OPEN_ERR), tmp_ar_name);
		}

		/* Restore flags */
		new_archive = new_save;
		flg['c'] = flg_save;

		/* Now copy the original archive file to temp file */
		ar_fseek(save_fp, 0L, 2);
		tmp_size = ftell(save_fp);
		ar_fseek(save_fp, 0L, 0);
		copy_file(save_fp, tmp_fp, tmp_size);
		ar_fseek(save_fp, 0L, 0);
		ar_fseek(tmp_fp, 0L, 0);

		/* Return pointer to the temporary file */
		return(tmp_fp);
	}

	else	/* Copy temporary file to original saved archive */
	{
		/* Make sure copy was made */
		if(save_fp == NULL)
			return(f_fp);

		/* Set flag to indicate any error this file copy */
		safe_err++;

		/* Copy temporary to original */
		ar_fseek(f_fp, 0L, 2);
		tmp_size = ftell(f_fp);
		ar_fseek(f_fp, 0L, 0);
		copy_file(f_fp, save_fp, tmp_size);

		/* Unlink the temporary file */
		(void)fclose(f_fp);
		(void)unlink(tmp_ar_name);

		/* Copy completed ok */
		safe_err = 0;

		return(save_fp);
	}
}


/*******************************************************************************
	ar_* 	- Error checking routines for ar.

	The following routines are used to localize error checking from
	system and library calls. In the case of unrecoverable errors an
	exit routine will be called to terminate the program.
*******************************************************************************/

char *ar_malloc(size)
/*******************************************************************************
	ar_malloc
*******************************************************************************/
unsigned size;
{
	char *mem_ptr;

	if((mem_ptr = (char *)malloc(size)) == NULL)
	{
		ar_exit(MSGSTR(NO_MEM_MSG, NO_MEM));
	}

	return(mem_ptr);
}


char *ar_realloc(ptr, size)
/*******************************************************************************
	ar_realloc
*******************************************************************************/
char *ptr;
unsigned size;
{
	char *mem_ptr;

	if((mem_ptr = (char *)realloc(ptr, size)) == NULL)
	{
		ar_exit(MSGSTR(NO_MEM_MSG, NO_MEM));
	}

	return(mem_ptr);
}


void ar_fseek(stream, offset, whence)
/*******************************************************************************
	ar_fseek
*******************************************************************************/
FILE *stream;
long offset;
int whence;
{
	if(fseek(stream, offset, whence) != 0)
		ar_exit(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));

	return;
}


void ar_fread(ptr, size, nitems, stream)
/*******************************************************************************
	ar_fread
*******************************************************************************/
char *ptr;
int size, nitems;
FILE *stream;
{
	if(fread((void *)ptr, (size_t)size, (size_t)nitems, stream) != nitems)
		ar_exit(MSGSTR(READ_ERR_MSG, READ_ERR));

	return;
}


void ar_fwrite(ptr, size, nitems, stream)
/*******************************************************************************
	ar_fwrite
*******************************************************************************/
char *ptr;
int size, nitems;
FILE *stream;
{
	if(fwrite((void *)ptr, (size_t)size, (size_t)nitems, stream) != nitems)
		ar_exit(MSGSTR(WRITE_ERR_MSG, WRITE_ERR));

	return;
}


void ar_ftruncate(fp, length)
/*******************************************************************************
	ar_ftruncate
*******************************************************************************/
FILE *fp;
long length;
{
	if(fflush(fp) == EOF)
		ar_exit(MSGSTR(FFLUSH_ERR_MSG, FFLUSH_ERR));

	if(ftruncate(fileno(fp), length))
		ar_exit(MSGSTR(TRUNCATE_ERR_MSG, TRUNCATE_ERR));

	return;
}


void ar_fclear(fp, position, nbytes)
/*******************************************************************************
	ar_fclear
*******************************************************************************/
FILE *fp;
long position, nbytes;
{
	if(fflush(fp) == EOF)
		ar_exit(MSGSTR(FFLUSH_ERR_MSG, FFLUSH_ERR));

	if(lseek(fileno(fp), position, 0) < 0L)
		ar_exit(MSGSTR(FCLEAR_ERR_MSG, FCLEAR_ERR));

	if(fclear(fileno(fp), nbytes) != nbytes)
		ar_exit(MSGSTR(FCLEAR_ERR_MSG, FCLEAR_ERR));

	return;
}


#ifdef _STD_ARG
void ar_exit(char *fmt, ...)
#else /* _STD_ARG */
void ar_exit(va_alist)
#endif /* _STD_ARG */
/*******************************************************************************
	ar_exit
*******************************************************************************/
#ifndef _STD_ARG
va_dcl
#endif /* _STD_ARG */
{
	va_list args;

        if (errno != 0)
                perror("ar");

#ifdef _STD_ARG

	va_start(args, fmt);
#else /* _STD_ARG */
	char *fmt;

	va_start(args);
	fmt = va_arg(args, char *);
#endif /* _STD_ARG */
	(void)vfprintf(stderr, fmt, args);
	(void)putc('\n', stderr);
	va_end(args);
	if(flg['z'] && safe_err)
	{
		ar_error(MSGSTR(SAFE_ERR_MSG, SAFE_ERR), ar_fname,
			tmp_ar_name);
	}
	else if((flg['o'] || flg['z']) && (tmp_ar_name != NULL))
	{
		(void)unlink(tmp_ar_name);
	}
	(void)exit((int)1);
}


#ifdef _STD_ARG
void ar_fprintf(FILE *stream, char *fmt, ...)
#else /* _STD_ARG */
void ar_fprintf(stream, va_alist)
#endif /* _STD_ARG */
/*******************************************************************************
	ar_fprintf
*******************************************************************************/
#ifndef _STD_ARG
FILE *stream;
va_dcl
#endif /* _STD_ARG */
{
	va_list args;
#ifdef _STD_ARG

	va_start(args, fmt);
#else /* _STD_ARG */
	char *fmt;

	va_start(args);
	fmt = va_arg(args, char *);
#endif /* _STD_ARG */
	(void)vfprintf(stream, fmt, args);
	(void)putc('\n', stream);
	va_end(args);
	return;
}


#ifdef _STD_ARG
void ar_error(char *fmt, ...)
#else /* _STD_ARG */
void ar_error(va_alist)
#endif /* _STD_ARG */
/*******************************************************************************
	ar_error
*******************************************************************************/
#ifndef _STD_ARG
va_dcl
#endif /* _STD_ARG */
{
	va_list args;

        if (errno != 0)
        {
                perror("ar");
                errno = 0;
        }

#ifdef _STD_ARG
	va_start(args, fmt);
#else /* _STD_ARG */
	char *fmt;

	va_start(args);
	fmt = va_arg(args, char *);
#endif /* _STD_ARG */
	(void)vfprintf(stderr, fmt, args);
	(void)putc('\n', stderr);
	va_end(args);
	nonfatal_errors++;

	return;
}
