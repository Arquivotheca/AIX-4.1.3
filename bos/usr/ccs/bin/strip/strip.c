static char sccsid[] = "@(#)70        1.36  src/bos/usr/ccs/bin/strip/strip.c, cmdaout, bos41B, 9505A 1/3/95 14:29:36";
#define RELEASE                       "strip: strip.c 1.36"

/*
 * COMPONENT_NAME: CMDAOUT (strip command)
 *
 * FUNCTIONS: already_stripped, copy, copy_scns, copy_symtab, done,
 *	error, get_head, isarchive, process_tabs, put_head, quit, read_tabs,
 *	reset_globals, stripar, stripfile, stripobj, copy_unstripped_file
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

/*
 *  strip
 *
 *  remove symbol table from object files
 *  remove local symbols from object modules in archive files
 *
 */

#include <stdio.h>
#include <stddef.h>
#include <ar.h>
#include <signal.h>
#include <xcoff.h>
#include <nl_types.h>
#include <locale.h>
#include <errno.h>
#include <stdarg.h>
#include "strip_msg.h"

nl_catd  catd;
#define MSGSTR(Num, Str) catgets(catd, MS_STRIP, Num, Str)

#define	CANT_OPEN "0654-400 Cannot open file."
#define	CANT_OPEN_TMP	"0654-401 Cannot open temporary file %s."
#if 0   /* 0654-402 is not used */
#define CANT_RECR	"0654-402 Cannot write to file, permissions \
are not correct."
#endif
#define READ_ERR	"0654-403 Cannot read from file."
#define WRITE_ERR	"0654-404 Cannot write to file."
#define NOT_XCOFF "0654-405 Specify an XCOFF object file or an archive of\n\
\tXCOFF object files."
#define READ_TAB_ERR	"0654-406 Internal error while reading file tables."
#define PROCESS_TAB_ERR	"0654-407 Internal error while processing file tables."
#define COPY_SCN_ERR	"0654-408 Internal error while copying sections."
#define COPY_SYM_ERR	"0654-409 Internal error while copying symbol table."
#define WRITE_TAB_ERR	"0654-410 Internal error while writing headers."
#define USAGE		"Usage: strip [-V] [-r[-l]|-x[-l]|-t|-H|-e|-E] File ..."
#define OPTMIX	"0654-411 The specified flags are mutually exclusive."
#define	TRUNCATE_ERR	"0654-412 The ftruncate system call failed."
#define SEEK_ERR         "0654-413 File seek error."
#define SEEKT_ERR        "0654-414 Temporary file %s: seek error."
#define ZERO_ERR         "0654-415 File size was zero."
#define ZEROT_ERR        "0654-416 Temporary file %s: size was zero."
#define CLOSE_ERR        "0654-417 File close error."
#define CLOSET_ERR       "0654-418 Temporary file %s: close error."
#define AR_STRIPPED	"0654-419 The specified archive file was already \
stripped."
#define ALREADY_STRIPPED "0654-420 The file was already stripped \
as specified."
#define WRITET_ERR	"0654-421 Cannot write to temporary file %s."
#define AR_LOADONLY_ERR	"0654-422 Cannot use -e or -E flags with an archive."


extern char	*mktemp(), *malloc(), *memcpy(), *strcpy(), *strcat(),
		*strncpy();
extern long	atol();
extern void	exit(), free();
static		stripobj(), stripar();
void		reset_globals();
void 		error(char *fmt, ...);
void 		quit(char *fmt, ...);

char		*Fname,		/* Name of file to be stripped */
		*Tfname;	/* Temp file name */
FILE		*Fptr,		/* Pointer to file to be stripped */
		*Tfptr;		/* Pointer to temp file */
FILHDR		Filhdr;		/* File header of file to be stripped */
AOUTHDR		*Aouthdr = NULL;/* Optional header of file to be stripped */
SCNHDR		*Scnhdr = NULL;	/* Section headers of file to be stripped */
char		*Symtab = NULL;	/* Symbol table of file to be stripped */
char		*Strtab = NULL;	/* String table of file to be stripped */
char		*Lnno = NULL;	/* Line number table of file to be stripped */
char		*Treloc = NULL,	/* Text reloc table of file to be stripped */
		*Dreloc = NULL;	/* Data reloc table of file to be stripped */
long		*Sym_xref = NULL,/* Symbol table cross reference array */
		*Str_xref = NULL,/* String table cross reference array */
		Num_of_syms = 0,/* Number of symbols in symbol table */
		Num_lnno = 0,	/* Number of line number entries in file */
		Txt_rels = 0,	/* Number of test relocation entries in file */
		Data_rels = 0,	/* Number of data relocation entries in file */
		Strsize = 0,	/* Original size of string table */
		Sym_pos = 0,	/* Position of symbol table in output file */
		Lnno_delta = 0; /* Delta value for lnno pointer fix up */
int		Hflag = 0,	/* Strip all headers flag */
		Tflag = 0,	/* Strip for traceback flag */
		Xflag = 0,	/* Strip all but statics and externals flag */
		Aflag = 0,	/* Strip all debug and rebind info (default) */
		Rflag = 0,	/* Strip for rebind flag */
		Lflag = 0,	/* Strip line number info flag */
		Vflag = 0,	/* Print the version number of strip command */
		eflag = 0,	/* Sets the F_LOADONLY flag */
		Eflag = 0,	/* Unsets the F_LOADONLY flag */
		Kflag = 0,	/* Special kernel strip flag */
		Exit_code = 0;	/* Value returned when process exits */
char		buf[BUFSIZ];	/* Temporary buffer */



main(argc, argv)
/*******************************************************************************
	main	entry point
*******************************************************************************/
int  argc;
char *argv[]; 
{       
	register int i;
	register int s;
	extern int optind;
	extern char *optarg;
	register long size;
	char *tmpdir;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_STRIP, NL_CAT_LOCALE);
	tmpdir = getenv("TMPDIR");

	if (argc == 1 )
	    {
		errno = 0;
		quit(MSGSTR(USAGE_MSG, USAGE));
	    }

	(void)signal(SIGHUP, SIG_IGN);
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);

	Tfname = tempnam(tmpdir,NULL);
	(void)close(creat(Tfname, 0600));

	while((i = getopt(argc, argv, "rltxHVeEK:")) != EOF)
	{
		switch (i)
		{
			case 'H':
				Hflag = 1;
				break;
			case 'l':
				Lflag = 1;
				break;
			case 'r':
				Rflag = 1;
				break;
		        case 'x':
				Xflag = 1;
				break;
		        case 't':
				Tflag = 1;
				break;
		        case 'V':
				Vflag = 1;
				break;
		        case 'e':
				eflag = 1;
				break;
		        case 'E':
				Eflag = 1;
				break;
			case 'K':
				if (!strcmp(optarg, "ernel"))
				{
					Kflag = 1;
					break;
				}
				/* fall thru if not '-Kernel' */
		        default:
				errno = 0;
				quit(MSGSTR(USAGE_MSG, USAGE));
		}
	}

	/* 
	 * The -K option is a special undocumented flag that is only
	 * used to strip a kernel executable.  This option will strip
	 * the relocation entries from the loaded section of the file.
	 * This is desirable to save space in the kernel(which will never
	 * be relocated), but it would cause serious for any other
	 * execuatble object file.  If this flag is specified,  there
	 * should be no other flags specified with it.
	 */
	if (Kflag && (Hflag|Lflag|Rflag|Xflag|Tflag|Vflag|eflag|Eflag))
	{
		errno = 0;
		quit(MSGSTR(USAGE_MSG, USAGE));
	}

	if(((Hflag + Xflag + Rflag + Tflag + Eflag + eflag) > 1) ||
	((Hflag + Lflag) > 1) ||
	((Tflag + Lflag) > 1) ||
	((Eflag + eflag + Lflag) > 1))
		{
			errno = 0;
			error(MSGSTR(OPTMIX_MSG, OPTMIX));
			quit(MSGSTR(USAGE_MSG, USAGE));
		}

	if((Hflag + Xflag + Rflag + Lflag + Tflag + eflag + Eflag) == 0)
		Aflag++;

	/* For each file to be stripped */
	for (; optind < argc; )
	{
		Fname = argv[optind];
	        if ((Tfptr=fopen(Tfname,"w+"))==NULL)
			quit(MSGSTR(CANT_OPEN_TMP_MSG, CANT_OPEN_TMP),Tfname);
	        if (stripfile(Fname))
	        {
			if ((Fptr = fopen(Fname, "w")) == NULL)
			{
			        error(MSGSTR(CANT_OPEN_MSG, CANT_OPEN));
			}
			else
			{
				if (fseek(Tfptr,0L,2) != 0 )
					quit(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),
						Tfname);
				if ((size=ftell(Tfptr)) == 0)
					quit(MSGSTR(ZEROT_ERR_MSG, ZEROT_ERR),
						Tfname);
				if (fseek(Tfptr, 0L, 0) != 0)
					quit(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),
						Tfname);
				/* copy tempfile back to Fname */
				while (size != 0) {
					s = BUFSIZ;
					if (size < BUFSIZ) s = (unsigned)size;
					if (fread((void *)buf, (size_t)1, 
						(size_t)s, Tfptr) != s) 
						quit(MSGSTR(READ_ERR_MSG,
							 READ_ERR));
					if (fwrite((void *)buf, (size_t)1, 
						(size_t)s, Fptr) != s)
						quit(MSGSTR(WRITE_ERR_MSG, 
							WRITE_ERR));
					size -= s;
				}

				if (fclose(Fptr) != 0)
					quit(MSGSTR(CLOSE_ERR_MSG, CLOSE_ERR));
			}
	    	}
		if (fclose(Tfptr) != 0)
			quit(MSGSTR(CLOSET_ERR_MSG, CLOSET_ERR),Tfname);

		if(++optind < argc)
			(void)reset_globals();
	}

	if (Vflag)
		(void)fprintf(stderr, "%s\n", RELEASE);

	done();
}

/*
 * NAME: isarchive (From ARisarchive() in ARfuncs.o)
 *
 * FUNCTION: Given a FILE, determine an archive starts at the current
 *	     position in the file.  Leave the file positioned as it was
 *	     when this function was entered. 
 *
 * PARAMETERS: File descriptor of file to check.
 *
 * RETURN VALUE DESCRIPTION:
 *	      0 if not an archive or an error occured reading or seeking ,
 *	     47 (AIAF_AR_ID) for AIX Indexed archive file format
 *	      1 for other archive formats
 */
/* System III definitions */
#define	S3ARMAG	0177545
#define S3SARMAG (sizeof(int))

/* System V.0 definitions */
#define	S5ARMAG	"<ar>"
#define S5SARMAG 4

/* SysV.2 and Berkeley 4BSD definitions */
#define BKARMAG	"!<arch>\n"
#define BKSARMAG 8

/* AIX 3.1 definitions */
#define AIAFARMAG	"<aiaff>\n"
#define	AIAFSARMAG	8

/* Archive format identifying numbers */
#define NO_AR_ID	0x00
#define	OTHER_AR_ID	0x01
#define	AIAF_AR_ID	0x2f

int isarchive(FILE *file)
{       union
	{
		int magic3;
		char magic5[S5SARMAG];
		char magicB[BKSARMAG];
		char magicI[AIAFSARMAG];
	} magic;
	long curloc = ftell(file);

	if (fread(&magic, 1, sizeof(magic), file) < S3SARMAG)
		return 0;

	if (fseek(file, curloc, SEEK_SET) != 0) {
		error(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
		return 0;
	}

	if (magic.magic3 == S3ARMAG
	    || strncmp(magic.magic5, S5ARMAG, (size_t)S5SARMAG) == 0
	    || strncmp(magic.magicB, BKARMAG, (size_t)BKSARMAG) == 0)
		return OTHER_AR_ID;
	else if (strncmp(magic.magicI, AIAFARMAG, (size_t)AIAFSARMAG) == 0)
		return AIAF_AR_ID;
	else
		return NO_AR_ID;
}

stripfile(name)
/*******************************************************************************
	stripfile	Strip an individual file
		Determine if file is an archive. Call appropiate routine.
	Return Codes
		1	- If strip was performed, copy tmp to file
		0	- Archive strip or error, don't copy tmp to file
*******************************************************************************/
char *name;
{
        register int 	rc,
			f_type;

	if ((Fptr=fopen(name,"r"))==NULL)
	{
		error(MSGSTR(CANT_OPEN_MSG, CANT_OPEN));
		return(0);
	}
	f_type = isarchive(Fptr);
	if (f_type == NO_AR_ID)
		rc = stripobj();
	else if (Eflag || eflag)  /* cant set loadonly bit if its an archive */
		{
			errno = 0;
			error(MSGSTR(AR_LOADONLY_ERR_MSG, AR_LOADONLY_ERR));
		}
	else
		rc = stripar(f_type);
	if (fclose(Fptr) != 0)
		error(MSGSTR(CLOSE_ERR_MSG, CLOSE_ERR));
	return (rc);
}

static stripobj()
/*******************************************************************************
	stripobj	Strip a simple object file.
*******************************************************************************/
{
	if (get_head())
	{   
		errno = 0;
		error(MSGSTR(NOT_XCOFF_MSG, NOT_XCOFF));
		return(0);
	}

	if (!eflag && !Eflag)
	{
		if (already_stripped())
		{
			errno = 0;
			error(MSGSTR(ALREADY_STRIPPED_MSG, ALREADY_STRIPPED));
			return(0);
		}

		if (read_tabs())
		{
			error(MSGSTR(READ_TAB_ERR_MSG, READ_TAB_ERR));
			return(0);
		}

		if (process_tabs())
		{
			error(MSGSTR(PROCESS_TAB_ERR_MSG, PROCESS_TAB_ERR));
			return(0);
		}
	
		if (copy_scns())
		{
			error(MSGSTR(COPY_SCN_ERR_MSG, COPY_SCN_ERR));
			return(0);
		}

		if (copy_symtab())
		{
			error(MSGSTR(COPY_SYM_ERR_MSG, COPY_SYM_ERR));
			return(0);
		}
	}
	else
		copy_unstripped_file();

	if (put_head())
	{
		error(MSGSTR(WRITE_TAB_ERR_MSG, WRITE_TAB_ERR));
		return(0);
	}

	return(1);
}			


static stripar(ar_type)
/*******************************************************************************
	stripar		Strip an archive file
	Remove global symbol table from an archive file.
*******************************************************************************/
int ar_type;
{
	AR_HDR	arhdr;
	FL_HDR	flhdr;
	long	gst_off,
		mem_off;
	char	off_buf[16];
	FILE	*ar_fptr;

	if (ar_type == AIAF_AR_ID)
	{
		/* Use temporary file pointer to make updates now */
		if ((ar_fptr = fopen(Fname, "r+")) == NULL)
		{
		        error(MSGSTR(CANT_OPEN_MSG, CANT_OPEN));
			return(0);
		}

		/* Go to beginning of archive and read fixed header */
		if (fseek(ar_fptr, 0L, 0) != 0)
		{
			error(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
			(void)fclose(ar_fptr);
			return(0);
		}
		if (fread((void *)&flhdr, (size_t)FL_HSZ, (size_t)1, 
			ar_fptr) != 1)
		{
			error(MSGSTR(READ_ERR_MSG, READ_ERR));
			(void)fclose(ar_fptr);
			return(0);
		}

		/* Check for global symbol table */
		if ((gst_off = atol((char *)flhdr.fl_gstoff)) == 0)
		{
			errno = 0;
			error(MSGSTR(AR_STRIPPED_MSG, AR_STRIPPED));
			(void)fclose(ar_fptr);
			return(0);
		}

		/* Update global symbol table offset in fixed header */
		(void)sprintf(off_buf, "%-12ld", 0L);
		(void)strncpy(flhdr.fl_gstoff, off_buf, 12);

		/* Write modified fixed header */
		if (fseek(ar_fptr, 0L, 0) != 0)
		{
			error(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
			(void)fclose(ar_fptr);
			return(0);
		}
		if (fwrite((void *)&flhdr, (size_t)FL_HSZ, (size_t)1, ar_fptr) 
			!= 1)
		{
			error(MSGSTR(WRITE_ERR_MSG, WRITE_ERR));
			(void)fclose(ar_fptr);
			return(0);
		}

		/* Check for last mem */
		if ((mem_off = atol((char *)flhdr.fl_memoff)) == 0)
		{
			(void)fclose(ar_fptr);
			return(0);
		}

		/* Seek to last member and read ar header */
		if (fseek(ar_fptr, mem_off, 0) != 0)
		{
			error(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
			(void)fclose(ar_fptr);
			return(0);
		}
		if (fread((void *)&arhdr, (size_t)AR_HSZ, (size_t)1, ar_fptr) 
			!= 1)
		{
			error(MSGSTR(READ_ERR_MSG, READ_ERR));
			(void)fclose(ar_fptr);
			return(0);
		}

		/* Update next member offset in ar member header */
		(void)sprintf(off_buf, "%-12ld", 0L);
		(void)strncpy(arhdr.ar_nxtmem, off_buf, 12);

		/* Write modified ar header to file */
		if (fseek(ar_fptr, mem_off, 0) != 0)
		{
			error(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
			(void)fclose(ar_fptr);
			return(0);
		}
		if (fwrite((void *)&arhdr, (size_t)AR_HSZ, (size_t)1, ar_fptr)
			!= 1)
		{
			error(MSGSTR(WRITE_ERR_MSG, WRITE_ERR));
			(void)fclose(ar_fptr);
			return(0);
		}
	}

	else
	{
			errno = 0;
			error(MSGSTR(NOT_XCOFF_MSG, NOT_XCOFF));
			return(0);
	}
	

	/* Truncate file at position of GST */
	if (ftruncate(fileno(ar_fptr), gst_off))
	{
		error(MSGSTR(TRUNCATE_ERR_MSG, TRUNCATE_ERR));
		(void)fclose(ar_fptr);
		return(0);
	}

	if (fclose(ar_fptr) != 0)
		error(MSGSTR(CLOSE_ERR_MSG, CLOSE_ERR));

	return(0);	/* prevents copy of file */
}



read_tabs()
/*******************************************************************************
	read_tabs	Read tables needed to perform desired opertaions.
		Read tables needed by the following options:
	R, L, X, T	- Symbol and string tables.
	L && !X		- Relocation tables.
	!L		- Line number tables.
*******************************************************************************/
	
{
	int	i, j;

	if (Xflag || Rflag || Lflag || Tflag)  /* Read symbol & string table */
	{
		if (fseek (Fptr, Filhdr.f_symptr, 0) != 0)
			return(-1);
		if ((Symtab = (char *)malloc((unsigned)
		(Filhdr.f_nsyms * SYMESZ))) == NULL) 
			return(-1);
			

		if (fread((void *)Symtab, (size_t)SYMESZ, 
			(size_t)Filhdr.f_nsyms, Fptr)
			!= (int)Filhdr.f_nsyms)
			return(-1);

		if (fread((void *)&Strsize, (size_t)1, (size_t)sizeof(long), 
			Fptr) == sizeof(long))
		{
			if ((Strtab = malloc((unsigned)Strsize)) == NULL)
				return(-1);

			if (fseek (Fptr, (long)-(sizeof(long)), 1) != 0)
				return(-1);
			if (fread((void *)(Strtab), (size_t)1, (size_t)(Strsize)
			, Fptr) != (int)Strsize)
				return(-1);
		}
		else
		{
			if (!feof(Fptr))
				return(-1);
		}
	}

	if (Rflag || (Lflag && !Xflag))	/* Read the relocation entries */
	{
		for (i = 0; i < Filhdr.f_nscns; i++)
		{
			if (((Scnhdr[i].s_flags & 0x0000ffff) == STYP_TEXT) &&
			Scnhdr[i].s_nreloc)
			{
				/* Check for overflow */
				if ((Scnhdr[i].s_nreloc == 0xffff) &&
				(Scnhdr[i].s_nlnno == 0xffff))
				{
					for (j = 0; j < Filhdr.f_nscns; j++)
					{
						if (((Scnhdr[j].s_flags &
						0x0000ffff) == STYP_OVRFLO) &&
						(Scnhdr[j].s_nreloc == i+1))
						{
							Txt_rels =
							Scnhdr[j].s_paddr;
							break;
						}
					}
				}
				else
					Txt_rels = Scnhdr[i].s_nreloc;
						

				if (Txt_rels)
				{
					if ((Treloc = (char *)malloc((unsigned)
					(Txt_rels * RELSZ))) == NULL)
						return(-1);

					if (fseek(Fptr, Scnhdr[i].s_relptr,
						0) != 0)
						return(-1);

					if (fread((void *)Treloc, 
					    (size_t)RELSZ,
					    (size_t)Txt_rels, Fptr) 
					    != (int)Txt_rels)
						return(-1);
				}
			}

			if (((Scnhdr[i].s_flags & 0x0000ffff) == STYP_DATA) &&
			Scnhdr[i].s_nreloc)
			{
				/* Check for overflow */
				if ((Scnhdr[i].s_nreloc == 0xffff) &&
				(Scnhdr[i].s_nlnno == 0xffff))
				{
					for (j = 0; j < Filhdr.f_nscns; j++)
					{
						if (((Scnhdr[j].s_flags &
						0x0000ffff) == STYP_OVRFLO) &&
						(Scnhdr[j].s_nreloc == i+1))
						{
							Data_rels =
							Scnhdr[j].s_paddr;
							break;
						}
					}
				}
				else
					Data_rels = Scnhdr[i].s_nreloc;

				if (Data_rels)
				{
					if ((Dreloc = (char *)malloc((unsigned)
					(Data_rels * RELSZ))) == NULL)
						return(-1);

					if (fseek(Fptr, Scnhdr[i].s_relptr,
						0) != 0)
						return(-1);

					if (fread((void *)Dreloc, (size_t)RELSZ,
				 	    (size_t)Data_rels, Fptr) 
					    != (int)Data_rels)
						return(-1);
				}
			}
		}
	}

	if (!Lflag && !Hflag && !Aflag)	/* Need to read the lnno information */
	{
		for (i = 0; i < Filhdr.f_nscns; i++)
		{
			if (((Scnhdr[i].s_flags & 0x0000ffff) == STYP_TEXT) &&
			Scnhdr[i].s_nlnno)
			{
				/* Check for overflow */
				if ((Scnhdr[i].s_nreloc == 0xffff) &&
				(Scnhdr[i].s_nlnno == 0xffff))
				{
					for (j = 0; j < Filhdr.f_nscns; j++)
					{
						if (((Scnhdr[j].s_flags &
						0x0000ffff) == STYP_OVRFLO) &&
						(Scnhdr[j].s_nlnno == i+1))
						{
							Num_lnno =
							Scnhdr[j].s_vaddr;
							break;
						}
					}
				}
				else
					Num_lnno = Scnhdr[i].s_nlnno;

				if (Num_lnno != 0) 
				{ /* don't malloc if no lines */
					if ((Lnno = (char *)malloc((unsigned)
					(Num_lnno * LINESZ))) == NULL)
						return(-1);

					if (fseek(Fptr, Scnhdr[i].s_lnnoptr, 
						0) != 0)
						return(-1);

					if (fread((void *)Lnno, (size_t)LINESZ,
						(size_t)Num_lnno,
						Fptr) != (int)Num_lnno)
					return(-1);
				}
			}
		}
	}

	return(0);
}

process_tabs()
/*******************************************************************************
	process_tabs	Process tables according to strip options
		Perform all table processing on the in core copies according
	to the specified options.
	X	- Strip all but external and file symbols, adjust file
		  and relocation symbol indexes.
	T	- Strip all but function symbols, adjust line number
		  symbol indexes.
*******************************************************************************/
{
	int	i, j,
		last_file_pos = -1,
		last_file_value = -1;
	SYMENT	sym_str,
		*sym_ptr,
		file_str,
		*file_ptr;
	AUXENT	aux_str,
		*aux_ptr;
	RELOC	rel_str,
		*rel_ptr;
	LINENO	lnno_str,
		*lnno_ptr;

	if (Xflag || Rflag)	/* Strip all but external symbols */
	{

		/* Create cross reference table for all symbols snd strings */
		if (((Sym_xref = (long *)malloc((unsigned)(Filhdr.f_nsyms *
		sizeof(long)))) == NULL) ||
		((Str_xref = (long *)malloc((unsigned)(Filhdr.f_nsyms *
		sizeof(long)))) == NULL))
			return(-1);

		memset(Sym_xref, 0, (size_t)(Filhdr.f_nsyms * sizeof(long)));
		memset(Str_xref, 0, (size_t)(Filhdr.f_nsyms * sizeof(long)));

		/* Scan symbol table for all external and file symbols */
		for (i = 0; i < Filhdr.f_nsyms; i++)
		{
			if (i & 0x00000001)
			{
				(void)memcpy((char *)&sym_str, (char *)Symtab +
					(i * SYMESZ), SYMESZ);
				sym_ptr = &sym_str;
			}
			else
				sym_ptr = (SYMENT *)(Symtab + (i * SYMESZ));

			/* Check for external */
			if ((sym_ptr->n_sclass == C_EXT) ||
			(sym_ptr->n_sclass == C_HIDEXT))
			{	/* Check for file to update */
				if ((last_file_pos >= 0) &&
				(i >= last_file_value))
				{	/* Update previous to point here */
					file_ptr->n_value = Num_of_syms;
					if (last_file_pos & 0x00000001)
					{
						(void)memcpy((char *)(Symtab +
							(last_file_pos *
							SYMESZ)), (char *)
							file_ptr, SYMESZ);
					}
				/* Reset file values */
				last_file_pos = last_file_value = -1;
				}

				/* Check and note string table references */
				if ((sym_ptr->n_zeroes == 0L) &&
				sym_ptr->n_offset)
					Str_xref[i] = sym_ptr->n_offset;
				else
					Str_xref[i] = -1;

				/* Save this symbol for final table */
				Sym_xref[i] = Num_of_syms++;
				for (j = 0; j < sym_ptr->n_numaux; j++)
				{	/* Also save aux entries */
					Sym_xref[++i] = Num_of_syms++;
					Str_xref[i] = -1;
				}

				/* Update debug and typchk pointers */
				if (i & 0x00000001)
				{
					(void)memcpy((char *)&aux_str,
						(char *)Symtab + (i * SYMESZ),
						SYMESZ);
					aux_ptr = &aux_str;
				}
				else
					aux_ptr = (AUXENT *)(Symtab +
						(i * SYMESZ));

				/* Zero out references to these sections */
				aux_ptr->x_csect.x_parmhash = 0;
				aux_ptr->x_csect.x_snhash = 0;
				aux_ptr->x_csect.x_stab = 0;
				aux_ptr->x_csect.x_snstab = 0;

				/* If LD adjust SD pointer */
				if ((aux_ptr->x_csect.x_smtyp & 0x07) == XTY_LD)
				{
					if ((aux_ptr->x_csect.x_scnlen =
					Sym_xref[aux_ptr->x_csect.x_scnlen])
					== -1)
					{
						/* LD points to invalid SD */
						return(-1);
					}
				}

				/* Write back to table */
				if (i & 0x00000001)
				{
					(void)memcpy((char *)Symtab +
						(i * SYMESZ), (char *)aux_ptr,
						SYMESZ);
				}

			}
			/* Check for file */
			else if (sym_ptr->n_sclass == C_FILE)
			{	/* Check for update to previous file */
				if (last_file_pos >= 0)
				{	/* Update previous to point here */
					file_ptr->n_value = Num_of_syms;
					if (last_file_pos & 0x00000001)
					{
						(void)memcpy((char *)(Symtab +
						(last_file_pos *
							SYMESZ)), (char *)
							file_ptr, SYMESZ);
					}
				}

				/* Mark the position of file for later update */
				last_file_pos = i;
				last_file_value = sym_ptr->n_value;
				if (last_file_pos & 0x00000001)
				{
					(void)memcpy((char *)&file_str,
						(char *)sym_ptr,SYMESZ);
					file_ptr = &file_str;
				}
				else
					file_ptr = sym_ptr;

				/* Determine format of the file symbol */
				if (sym_ptr->n_numaux)
				{	/* Check/note string table ref in aux */
					if ((i + 1) & 0x00000001)
					{
						(void)memcpy((char *)&aux_str,
							(char *)(Symtab + (i +
							1) * SYMESZ), SYMESZ);
						aux_ptr = &aux_str;
					}
					else
					{
						aux_ptr = (AUXENT *)(Symtab +
							(i + 1) * SYMESZ);
					}
					if ((aux_ptr->x_file._x.x_zeroes == 0L)
					&& aux_ptr->x_file._x.x_offset)
						Str_xref[i+1] =
						aux_ptr->x_file._x.x_offset;
					else
						Str_xref[i+1] = -1;

					/* No string table entry in syment */
					Str_xref[i] = -1;
				}
				else
				{	/* Name in syment entry */
					/* Check/note string table references */
					if ((sym_ptr->n_zeroes == 0L) &&
					sym_ptr->n_offset)
						Str_xref[i] = sym_ptr->n_offset;
					else
						Str_xref[i] = -1;
				}

				/* Save this symbol for final table */
				Sym_xref[i] = Num_of_syms++;
				for (j = 0; j < sym_ptr->n_numaux; j++)
				{	/* Also save aux entries */
					Sym_xref[++i] = Num_of_syms++;
					/* Mark all but 1'st aux */
					if (j > 0)
						Str_xref[i] = -1;
				}
			}
			else
			{	/* Mark as deleted */
				Sym_xref[i] = -1;
				for (j = 0; j < sym_ptr->n_numaux; j++)
				{	/* Also mark aux entries */
					Sym_xref[++i] = -1;
					Str_xref[i] = -1;
				}
			}
		}
	} /* End of if Xflag or Rflag */


	if(Rflag)	/* Fix up relocation table symbol indexes */
	{
		for (i = 0; i < Txt_rels; i++)
		{
			if (i & 0x00000001)
			{
				(void)memcpy((char *)&rel_str, (char *)
					(Treloc + (i * RELSZ)), RELSZ);
				rel_ptr = &rel_str;
			}
			else
				rel_ptr = (RELOC *)(Treloc +
					(i * RELSZ));

			/* Adjust reloc symbol index */
			if (Sym_xref[rel_ptr->r_symndx] < 0)
			{	
				/* Reloc points to invalid sym entry */
				return(1);
			}
			rel_ptr->r_symndx = Sym_xref[rel_ptr->r_symndx];

			if (i & 0x00000001)
			{
				(void)memcpy((char *)(Treloc + (i * RELSZ)),
					(char *)&rel_str, RELSZ);
			}
		}

		/* Fix up the data relocation table symbol indexes */
		for (i = 0; i < Data_rels; i++)
		{
			if (i & 0x00000001)
			{
				(void)memcpy((char *)&rel_str, (char *)
					(Dreloc + (i * RELSZ)), RELSZ);
				rel_ptr = &rel_str;
			}
			else
				rel_ptr = (RELOC *)(Dreloc +
					(i * RELSZ));

			/* Adjust reloc symbol index */
			if (Sym_xref[rel_ptr->r_symndx] < 0)
			{	
				/* Reloc points to invalid sym entry */
				return(1);
			}
			rel_ptr->r_symndx = Sym_xref[rel_ptr->r_symndx];

			if (i & 0x00000001)
			{
				(void)memcpy((char *)(Dreloc + (i * RELSZ)),
					(char *)&rel_str, RELSZ);
			}
		}
	} /* End of if Rflag */



	if (Tflag)	/* Strip all but function and file symbols */
	{

		/* Create cross reference table for all symbols */
		if (((Sym_xref = (long *)malloc((unsigned)(Filhdr.f_nsyms *
		    sizeof(long)))) == NULL) ||
		    ((Str_xref = (long *)malloc((unsigned)(Filhdr.f_nsyms *
		    sizeof(long)))) == NULL))
			return(-1);

		memset(Sym_xref, 0, (size_t)(Filhdr.f_nsyms * sizeof(long)));
		memset(Str_xref, 0, (size_t)(Filhdr.f_nsyms * sizeof(long)));

		/* Scan symbol table for all function symbols */
		for (i = 0; i < Filhdr.f_nsyms; i++)
		{
			if (i & 0x00000001)
			{
				(void)memcpy((char *)&sym_str, (char *)(Symtab +
					(i * SYMESZ)), SYMESZ);
				sym_ptr = &sym_str;
			}
			else
				sym_ptr = (SYMENT *)(Symtab + (i * SYMESZ));

			/* Check for function */
			if (ISFCN(sym_ptr->n_type))
			{	/* Check and note string table references */
				if ((sym_ptr->n_zeroes == 0L) &&
				sym_ptr->n_offset)
					Str_xref[i] = sym_ptr->n_offset;
				else
					Str_xref[i] = -1;

				/* Save this symbol for final table */
				Sym_xref[i] = Num_of_syms++;
				for (j = 0; j < sym_ptr->n_numaux; j++)
				{	/* Also save aux entries */
					Sym_xref[++i] = Num_of_syms++;
					Str_xref[i] = -1;
				}

				/* Make sure this is external */
				if ((sym_ptr->n_sclass == C_EXT) ||
				(sym_ptr->n_sclass == C_HIDEXT))
				{
					/* Update debug and typchk pointers */
					if (i & 0x00000001)
					{
						(void)memcpy((char *)&aux_str,
							(char *)Symtab +
							(i * SYMESZ), SYMESZ);
						aux_ptr = &aux_str;
					}
					else
						aux_ptr = (AUXENT *)(Symtab +
							(i * SYMESZ));

					/* Zero references to these sections */
					aux_ptr->x_csect.x_parmhash = 0;
					aux_ptr->x_csect.x_snhash = 0;
					aux_ptr->x_csect.x_stab = 0;
					aux_ptr->x_csect.x_snstab = 0;

					/* Write back to table */
					if (i & 0x00000001)
					{
						(void)memcpy((char *)Symtab +
							(i * SYMESZ), (char *)
							aux_ptr, SYMESZ);
					}
				}
			}
			else if (sym_ptr->n_sclass == C_FILE)
			{	/* Check for update to previous file */
				if (last_file_pos >= 0)
				{	/* Update previous to point here */
					file_ptr->n_value = Num_of_syms;
					if (last_file_pos & 0x00000001)
					{
						(void)memcpy((char *)(Symtab +
							(last_file_pos *
							SYMESZ)), (char *)
							file_ptr, SYMESZ);
					}
				}

				/* Mark the position of file for later update */
				last_file_pos = i;
				last_file_value = sym_ptr->n_value;
				if (last_file_pos & 0x00000001)
				{
					(void)memcpy((char *)&file_str,
						(char *)sym_ptr,SYMESZ);
					file_ptr = &file_str;
				}
				else
					file_ptr = sym_ptr;

				/* Determine format of the file symbol */
				if (sym_ptr->n_numaux)
				{	/* Check/note string table ref in aux */
					if ((i + 1) & 0x00000001)
					{
						(void)memcpy((char *)&aux_str,
							(char *)(Symtab + (i +
							1) * SYMESZ), SYMESZ);
						aux_ptr = &aux_str;
					}
					else
					{
						aux_ptr = (AUXENT *)(Symtab +
							(i + 1) * SYMESZ);
					}
					if ((aux_ptr->x_file._x.x_zeroes == 0L)
					&& aux_ptr->x_file._x.x_offset)
						Str_xref[i+1] =
						aux_ptr->x_file._x.x_offset;
					else
						Str_xref[i+1] = -1;

					/* No string table entry in syment */
					Str_xref[i] = -1;
				}
				else
				{	/* Name in syment entry */
					/* Check/note string table references */
					if ((sym_ptr->n_zeroes == 0L) &&
					sym_ptr->n_offset)
						Str_xref[i] = sym_ptr->n_offset;
					else
						Str_xref[i] = -1;
				}

				/* Save this symbol for final table */
				Sym_xref[i] = Num_of_syms++;
				for (j = 0; j < sym_ptr->n_numaux; j++)
				{	/* Also save aux entries */
					Sym_xref[++i] = Num_of_syms++;
					/* Mark all but 1'st aux */
					if (j > 0)
						Str_xref[i] = -1;
				}
			}
			else
			{	/* Mark symbol to be deleted */
				Sym_xref[i] = -1;
				Str_xref[i] = -1;
				for (j = 0; j < sym_ptr->n_numaux; j++)
				{	/* Also mark aux entries */
					Sym_xref[++i] = -1;
					Str_xref[i] = -1;
				}
			}
		}

	} /* end of if Tflag */


	if (Lflag && !Hflag && !Tflag && !Xflag && !Aflag && !Rflag)
	/* Remove all line number references */
	{
		/* Scan symbol table for all function symbols */
		for (i = 0; i < Filhdr.f_nsyms; i++)
		{
			if (i & 0x00000001)
			{
				(void)memcpy((char *)&sym_str, (char *)(Symtab +
					(i * SYMESZ)), SYMESZ);
				sym_ptr = &sym_str;
			}
			else
				sym_ptr = (SYMENT *)(Symtab + (i * SYMESZ));

			/* Check for function */
			if (ISFCN(sym_ptr->n_type) && (sym_ptr->n_numaux >= 2)) 
			{
				/* Update lnnoptrs in aux entry */
				if ((i + 1) & 0x00000001)
				{
					(void)memcpy((char *)&aux_str,
						(char *)Symtab +
						((i + 1) * SYMESZ), SYMESZ);
					aux_ptr = &aux_str;
				}
				else
					aux_ptr = (AUXENT *)(Symtab +
							((i + 1) * SYMESZ));

				aux_ptr->x_sym.x_fcnary.x_fcn.x_lnnoptr = 0;
				aux_ptr->x_sym.x_fcnary.x_fcn.x_endndx = 0;

				/* Write back to table */
				if ((i + 1) & 0x00000001)
				{
					(void)memcpy((char *)Symtab +
						((i + 1) * SYMESZ), (char *)
						aux_ptr, SYMESZ);
				}
			}

			i += sym_ptr->n_numaux;
		}

		Num_of_syms = Filhdr.f_nsyms;

	} /* end of if Lflag only */



	if (!Lflag && !Aflag && !Hflag)	/* Update indexes in the lnno table */
	{
		for (i = 0; i < Num_lnno; i++)
		{
			if (i & 0x00000001)
			{
				(void)memcpy((char *)&lnno_str, (char *)(Lnno +
					(i * LINESZ)), LINESZ);
				lnno_ptr = &lnno_str;
			}
			else
				lnno_ptr = (LINENO *)(Lnno + (i * LINESZ));

			/* Adjust line number symbol index */
			if (!lnno_ptr->l_lnno)
			{
				if ((lnno_ptr->l_addr.l_symndx =
				Sym_xref[lnno_ptr->l_addr.l_symndx]) < 0)
				{	
					/* Points to invalid symbol entry */
					return(1);
				}
			}

			if (i & 0x00000001)
			{
				(void)memcpy((char *)(Lnno + (i * LINESZ)),
					(char *)&lnno_str, LINESZ);
			}
		}
	} /* If not Lflag */

	return(0);
}


copy_scns()
/*******************************************************************************
	copy_scns	Copy sections of the object file
		Copy all of the data associated with the file sections to the
	temp file.  This involves copying the raw data, relocation info and
	line number tables for each section.  Only copy data as directed by
	the specified options.
*******************************************************************************/
{
	int	i;
	long	in_pos, out_pos,
		copy_size, copy_pos;

	/* Set appropiate copy positions */
	in_pos = (long)FILHSZ + (long)Filhdr.f_opthdr + 
		(long)(Filhdr.f_nscns * SCNHSZ);
	if (Hflag)
		out_pos = 0L;
	else
		out_pos = in_pos;
	if (fseek(Fptr, in_pos, 0) != 0)
		quit(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
	if (fseek(Tfptr, out_pos, 0) != 0)
		quit(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);


	/* Copy according to options */
	if (Hflag)
	{
		if (fseek(Fptr, 0L, 2) != 0)
			quit(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
		copy_size = ftell(Fptr) - in_pos;
		if (fseek(Fptr, in_pos, 0) != 0)
			quit(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
		(void)copy(Fptr, Tfptr, copy_size);
		out_pos += copy_size;
	}
	else
	{
		/* Copy raw data for all the sections to the temp file.
		 * Update all section headers as we go.
		 */
		for (i = 0; i < Filhdr.f_nscns; i++)
		{
			/* First check for Overflow sections.  These need
			 * special processing because their raw data is
			 * really reloc and lnno entries.
			 */
			if ((Scnhdr[i].s_flags & 0x0000ffff) == STYP_OVRFLO)
			{
				/* Check for Text section relocation entries */
				if (((Scnhdr[Scnhdr[i].s_nreloc-1].s_flags
				& 0x0000ffff) == STYP_TEXT) &&
				(Rflag || (Lflag && !Xflag)))
				{
					if (fwrite((void *)Treloc,(size_t)RELSZ,
					    (size_t)Txt_rels, Tfptr) 
					    != (int)Txt_rels)
						quit(MSGSTR(WRITET_ERR_MSG,
							WRITET_ERR),Tfname);
					Scnhdr[i].s_relptr = out_pos;
					Scnhdr[Scnhdr[i].s_nreloc-1].s_relptr =
						out_pos;
					out_pos += Txt_rels * RELSZ;
				}
				/* Check for Data section relocation entries */
				else if (((Scnhdr[Scnhdr[i].s_nreloc-1].s_flags
				& 0x0000ffff) == STYP_DATA) &&
				(Rflag || (Lflag && !Xflag)))
				{
					if (fwrite((void *)Dreloc,(size_t)RELSZ,
					(size_t)Data_rels, Tfptr) !=
					(int)Data_rels)
						quit(MSGSTR(WRITET_ERR_MSG,
							WRITET_ERR),Tfname);
					Scnhdr[i].s_relptr = out_pos;
					Scnhdr[Scnhdr[i].s_nreloc-1].s_relptr =
						out_pos;
					out_pos += Data_rels * RELSZ;
				}
				else	/* Remove reloc Overflow count */
				{
					Scnhdr[i].s_paddr = 0;
					Scnhdr[i].s_relptr = 0;
					Scnhdr[Scnhdr[i].s_nreloc-1].s_relptr =
						0;
				}

				/* Check for Text section line number entries */
				if (((Scnhdr[Scnhdr[i].s_nlnno-1].s_flags
				& 0x0000ffff) == STYP_TEXT) && !Lflag && !Aflag)
				{
					if (fwrite((void *)Lnno,(size_t)LINESZ,
					    (size_t)Num_lnno, Tfptr) 
					    != (int)Num_lnno)
						quit(MSGSTR(WRITET_ERR_MSG,
							WRITET_ERR),Tfname);
					Lnno_delta = out_pos -
						Scnhdr[i].s_lnnoptr;
					Scnhdr[i].s_lnnoptr = out_pos;
					Scnhdr[Scnhdr[i].s_nlnno-1].s_lnnoptr =
						out_pos;
					out_pos += Num_lnno * LINESZ;
				}
				else	/* Remove lnno Overflow count */
				{
					Scnhdr[i].s_vaddr = 0;
					Scnhdr[i].s_lnnoptr = 0;
					Scnhdr[Scnhdr[i].s_nlnno-1].s_lnnoptr =
						0;
				}

				/* Check to see is anything left in Overflow */
				if ((Scnhdr[i].s_vaddr == 0) &&
				(Scnhdr[i].s_paddr == 0))
				{
					Scnhdr[Scnhdr[i].s_nreloc-1].s_nreloc =
						0;
					Scnhdr[Scnhdr[i].s_nlnno-1].s_nlnno = 0;
					Scnhdr[i].s_flags = -1;
					Scnhdr[i].s_size = 0;
					Scnhdr[i].s_scnptr = 0;
					Scnhdr[i].s_nreloc = 0;
					Scnhdr[i].s_nlnno = 0;
				}
			}

			else if ((Scnhdr[i].s_flags & 0x0000ffff) == STYP_DEBUG
			|| (Scnhdr[i].s_flags & 0x0000ffff) == STYP_TYPCHK
			|| (Scnhdr[i].s_flags & 0x0000ffff) == STYP_INFO
			|| (Scnhdr[i].s_flags & 0x0000ffff) == STYP_EXCEPT)
			{
				/* If only Lflag keep these sections */
				if (Lflag && !Hflag && !Tflag && !Xflag &&
				!Aflag && !Rflag)
				{
					if (fseek(Fptr,Scnhdr[i].s_scnptr,0)
						!=0) 
						quit(MSGSTR(SEEK_ERR_MSG, 
							SEEK_ERR));
					(void)copy(Fptr,Tfptr,Scnhdr[i].s_size);
					Scnhdr[i].s_scnptr = out_pos;
					out_pos += Scnhdr[i].s_size;
				}
				else
				/* else delete these sections */
				{
					Scnhdr[i].s_flags = -1;
					Scnhdr[i].s_size = 0;
					Scnhdr[i].s_scnptr = 0;
					Scnhdr[i].s_relptr = 0;
					Scnhdr[i].s_lnnoptr = 0;
					Scnhdr[i].s_nreloc = 0;
					Scnhdr[i].s_nlnno = 0;
				}
			}

			else if (((Scnhdr[i].s_flags&0x0000ffff) == STYP_LOADER)
			&& Kflag)
			{	/*
				 * strip the relocation entries from the
				 * loader section
				 */
				struct ldhdr ldhdr;
				unsigned long size_removed, remainder;

				if (fseek(Fptr, Scnhdr[i].s_scnptr, 0) != 0)
					quit(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
				if (fread((void *)&ldhdr, sizeof(struct ldhdr),
				    1, Fptr) != 1)
					quit(MSGSTR(READ_ERR_MSG, READ_ERR));

				/* update loader header */
				size_removed = ldhdr.l_nreloc *
					sizeof(struct ldrel);
				remainder = Scnhdr[i].s_size -
					(sizeof(struct ldhdr) +
					ldhdr.l_nsyms*sizeof(struct ldsym) +
					ldhdr.l_nreloc*sizeof(struct ldrel));
				ldhdr.l_nreloc = 0;
				ldhdr.l_impoff -= size_removed;
				ldhdr.l_stoff -= size_removed;

				/* write out modified header */
				copy_pos = (Scnhdr[i].s_scnptr);
				if (copy_pos != out_pos)
				{
					if (fseek(Tfptr, copy_pos, 0) != 0)
					    quit(MSGSTR(SEEKT_ERR_MSG, 
						SEEKT_ERR), Tfname);
					out_pos = copy_pos;
				}
				if (fwrite((void *)&ldhdr, sizeof(struct ldhdr),
				    1, Tfptr) != 1)
					quit(MSGSTR(WRITET_ERR_MSG,
					     WRITET_ERR), Tfname);

				/* copy symbol table */
				(void)copy(Fptr, Tfptr, ldhdr.l_nsyms *
					sizeof(struct ldsym));

				/* skip the relocation entries */
				if (fseek(Fptr, size_removed, 1) != 0)
					quit(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));

				/* copy the rest */
				(void)copy(Fptr, Tfptr, remainder);
				
				Scnhdr[i].s_scnptr = out_pos;
				Scnhdr[i].s_size -= size_removed;
				out_pos += Scnhdr[i].s_size;
			}

			else if (((Scnhdr[i].s_flags & 0x0000ffff) != STYP_BSS)
			&& Scnhdr[i].s_size && Scnhdr[i].s_scnptr)
			{
				if (fseek(Fptr, Scnhdr[i].s_scnptr, 0) != 0)
					quit(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));

				if (!Hflag)
				{
					copy_pos = (Scnhdr[i].s_scnptr);
					if (copy_pos != out_pos)
					{
						if (fseek(Tfptr, copy_pos,
							  0) != 0)
						    quit(MSGSTR(SEEKT_ERR_MSG, 
								SEEKT_ERR),
								Tfname);
						out_pos = copy_pos;
					}
				    }
				(void)copy(Fptr, Tfptr, Scnhdr[i].s_size);
				Scnhdr[i].s_scnptr = out_pos;
				out_pos += Scnhdr[i].s_size;
			}
		}

		/* Now copy all relocation information to the output file.
		 * If necessary use the updated in core copies for TEXT
		 * and DATA sections. Update all section headers as we go.
		 */
		for (i = 0; i < Filhdr.f_nscns; i++)
		{
			if (!Scnhdr[i].s_nreloc || Scnhdr[i].s_nreloc == 0xffff)
				continue;		

			if (Rflag || (Lflag && !Xflag))	/* Only if r or l */
			{

				if ((Scnhdr[i].s_flags & 0x0000ffff) == 
				STYP_TEXT)	/* Text */
				{	/* Write updated text reloc entries */
					if (fwrite((void *)Treloc,(size_t)RELSZ,
					    (size_t)Txt_rels, Tfptr) 
					    != (int)Txt_rels)
						quit(MSGSTR(WRITET_ERR_MSG,
							WRITET_ERR),Tfname);
					Scnhdr[i].s_relptr = out_pos;
					out_pos += Txt_rels * RELSZ;
				}
				else if ((Scnhdr[i].s_flags & 0x0000ffff) ==
				STYP_DATA)	/*Data*/
				{	/* Write updated data reloc entries */
					if (fwrite((void *)Dreloc,(size_t)RELSZ,
						(size_t)Data_rels, Tfptr) !=
						(int)Data_rels)
						quit(MSGSTR(WRITET_ERR_MSG,
							WRITET_ERR),Tfname);
					Scnhdr[i].s_relptr = out_pos;
					out_pos += Data_rels * RELSZ;
				}
				else	/* Any others ?  */
				{	/* Copy as is */
					if (fseek(Fptr, Scnhdr[i].s_relptr,
						0) !=0)
						quit(MSGSTR(SEEK_ERR_MSG, 
							SEEK_ERR));
					(void)copy(Fptr, Tfptr, (long)
						(Scnhdr[i].s_nreloc * RELSZ));
					Scnhdr[i].s_relptr = out_pos;
					out_pos += Scnhdr[i].s_nreloc * RELSZ;
				}
			}
			else	/* Delete the relocation information */
			{
				Scnhdr[i].s_relptr = 0L;
				Scnhdr[i].s_nreloc = 0L;
			}
		}

		/* Now copy all line number information to the output file.
		 * If necessary use the updated in core copies for TEXT
		 * section. Update all section headers as we go.
		 */
		for (i = 0; i < Filhdr.f_nscns; i++)
		{
			if (!Scnhdr[i].s_nlnno || Scnhdr[i].s_nlnno == 0xffff)
				continue;		

			if (!Lflag && !Aflag)	/* Only if specified */
			{
				if ((Scnhdr[i].s_flags & 0x0000ffff) ==
				STYP_TEXT)	/* Text */
				{	/* Write updated text lnno entries */
					if (fwrite((void *)Lnno,(size_t)LINESZ,
					    (size_t)Num_lnno, Tfptr) 
					    != (int)Num_lnno)
						quit(MSGSTR(WRITET_ERR_MSG,
							WRITET_ERR),Tfname);
					Lnno_delta = out_pos -
						Scnhdr[i].s_lnnoptr;
					Scnhdr[i].s_lnnoptr = out_pos;
					out_pos += Num_lnno * LINESZ;
				}
				else	/* Any others ?  */
				{	/* Copy as is */
					if (fseek(Fptr, Scnhdr[i].s_lnnoptr,
						0) != 0)
						quit(MSGSTR(SEEK_ERR_MSG, 
							SEEK_ERR));
					(void)copy(Fptr, Tfptr, (long)
						(Scnhdr[i].s_nlnno * LINESZ));
					Scnhdr[i].s_lnnoptr = out_pos;
					out_pos += Scnhdr[i].s_nlnno * LINESZ;
				}
			}
			else
			{
				Scnhdr[i].s_lnnoptr = 0;
				Scnhdr[i].s_nlnno = 0;
			}
		} /* for */
	} /* else */

	Sym_pos = out_pos;	/* Set offset to symbol table in output file */

	return(0);
}


copy_symtab()
/*******************************************************************************
	copy_symtab	Copy the symbol table to output
		Copy all symbols not marked deleted to the output symbol
	table.  Also write out the new string table as directed by the new
	symbol table.
*******************************************************************************/
{
	char	*tmp_del;
	int	i, j;
	long	str_size = sizeof(long),
		tmp_size;
	SYMENT	sym_str,
		*sym_ptr;
	AUXENT	aux_str,
		*aux_ptr;

	if (Aflag ||		/* We want to delete symbol table OR     */
	    Hflag)		/* we have already written symbol table  */
		return(0);	/* So simply return */

	/* In the case where only the Lflag(delete line numbers) was */
	/* specified,  we have already removed all lnno references   */
	/* in the symbol table.  Now we simply want to write out the */
	/* incore symbol and string table as they currently exist.   */
	if (Lflag && !Hflag && !Tflag && !Xflag && !Aflag && !Rflag)
	{
		if (fseek(Tfptr, Sym_pos, 0) != 0)
		{
			error(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);
			return(-1);
		}
		if (fwrite((void *)Symtab, (size_t)SYMESZ,
		(size_t)Filhdr.f_nsyms, Tfptr)
		!= (int)Filhdr.f_nsyms)
			return(-1);
	
		if (fwrite((void *)Strtab, (size_t)1, (size_t)Strsize, Tfptr)
		!= (int)Strsize)
			return(-1);
	
		return(0);
	}
	

	/* First write out the string table based on the new symbol table */
	/* Then update offsets into the new string table for new symbols */
	/* Use a second array to avoid dups in the string table */
	if((tmp_del = (char *)malloc((unsigned)Filhdr.f_nsyms)) == NULL)
		return(-1);
	for (i = 0; i < Filhdr.f_nsyms; i++)
		tmp_del[i] = 0x00;

	/* Write out the new string table based on the new symbol table */
	if (fseek(Tfptr, Sym_pos + (Num_of_syms * SYMESZ), 0) != 0)
		{
			error(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);
			return(-1);
		}
	for (i = 0; i < Filhdr.f_nsyms; i++)
	{
		if ((Str_xref[i] < 0) || /* Skip symbols with no strings OR */
		(tmp_del[i]))		 /* those already processed */
			continue;

		if (str_size == sizeof(long))	/* First write ? */
			if (fseek(Tfptr, (long)sizeof(long), 1) != 0)  
							/* Skip size */
			{
				error(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);
				return(-1);
			}

		tmp_size = strlen(Strtab + Str_xref[i]) + 1;
		if (fwrite((void *)(Strtab + Str_xref[i]), (size_t)1, 
			(size_t)tmp_size,Tfptr) != (int)tmp_size)
			quit(MSGSTR(WRITET_ERR_MSG, WRITET_ERR),Tfname);

		/* Update all other references to this string */
		for (j = i+1; j < Filhdr.f_nsyms; j++)
			if (!tmp_del[j] && (Str_xref[j] == Str_xref[i]))
			{
				Str_xref[j] = str_size;
				tmp_del[j] = 0x01;
			}

		/* Update this reference and the size */
		Str_xref[i] = str_size;
		tmp_del[i] = 0x01;
		str_size += tmp_size;
	}
	free ((char *)tmp_del);		/* Free temporary array */

	/* Now scan the list of symbols in the new symbol table.  Adjust */
	/* string table references, and exception. Write the entries out */
	if (fseek(Tfptr, Sym_pos, 0) != 0) /* Seek to symbol table position */
		{
			error(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);
			return(-1);
		}
	for (i = 0; i < Filhdr.f_nsyms; i++)
	{
		if (Sym_xref[i] < 0)	/* Skip deleted symbols */
			continue;

		if (i & 0x00000001)
		{
			(void)memcpy((char *)&sym_str, (char *)(Symtab +
				(i * SYMESZ)), SYMESZ);
			sym_ptr = &sym_str;
		}
		else
		{
			sym_ptr = (SYMENT *)(Symtab + (i * SYMESZ));
		}

		/* Check for long symbol names */
		if ((sym_ptr->n_zeroes == 0L) && sym_ptr->n_offset)
		{
			sym_ptr->n_offset = Str_xref[i]; /* Adjust offset */

			/* Update composite */
			if (i & 0x00000001)
			{
				(void)memcpy((char *)(Symtab + (i * SYMESZ)),
					(char *)sym_ptr, SYMESZ);
			}
		}
				 
		/* Check for file names in auxent entries */
		if ((sym_ptr->n_sclass == C_FILE) && sym_ptr->n_numaux)
		{
			if((i + 1) & 0x00000001)
			{
				(void)memcpy((char *)&aux_str, (char *)(Symtab +
					((i+1) * SYMESZ)), SYMESZ);
				aux_ptr = &aux_str;
			}
			else
			{
				aux_ptr = (AUXENT *)(Symtab + ((i + 1) *
					SYMESZ));
			}

			/* Check for long file names */
			if ((aux_ptr->x_file._x.x_zeroes == 0L) &&
			aux_ptr->x_file._x.x_offset)
			{
				/* Update string table reference */
				aux_ptr->x_file._x.x_offset = Str_xref[i+1];

				/* Update auxent entry for symbol */
				if ((i + 1) & 0x00000001)
				{
					(void)memcpy((char *)Symtab + ((i + 1) *
						SYMESZ), (char *)aux_ptr,
						SYMESZ);
				}
			}
		}

		/* Update lnnoptr fields in function aux entries */
		if ( (Num_lnno > 0) && (Lnno_delta != 0) &&
		ISFCN(sym_ptr->n_type) && ((int)sym_ptr->n_numaux > 1) )
		{
			if((i + 1) & 0x00000001)
			{
				(void)memcpy((char *)&aux_str, (char *)(Symtab +
					((i+1) * SYMESZ)), SYMESZ);
				aux_ptr = &aux_str;
			}
			else
			{
				aux_ptr = (AUXENT *)(Symtab + ((i + 1)
					* SYMESZ));
			}

			/* Update line number table pointer */
			aux_ptr->x_sym.x_fcnary.x_fcn.x_lnnoptr += Lnno_delta;

			/* Update auxent entry for symbol */
			if ((i + 1) & 0x00000001)
			{
				(void)memcpy((char *)Symtab + ((i + 1) *
					SYMESZ), (char *)aux_ptr, SYMESZ);
			}
		}

	
		/* Check for aux entries that contain an exception offset */
		if ( ((sym_ptr->n_sclass == C_EXT) ||
		(sym_ptr->n_sclass == C_HIDEXT)) &&
		((int)sym_ptr->n_numaux > 1) )
		{
			if((i + 1) & 0x00000001)
			{
				(void)memcpy((char *)&aux_str, (char *)(Symtab +
					((i+1) * SYMESZ)), SYMESZ);
				aux_ptr = &aux_str;
			}
			else
			{
				aux_ptr = (AUXENT *)(Symtab + ((i + 1)
					* SYMESZ));
			}

			/* Update exception table pointer */
			aux_ptr->x_sym.x_tagndx = 0;

			/* Update auxent entry for symbol */
			if ((i + 1) & 0x00000001)
			{
				(void)memcpy((char *)Symtab + ((i + 1) *
					SYMESZ), (char *)aux_ptr, SYMESZ);
			}
		}
			
		/* Write out symbol entry and all aux entries */
		if (fwrite((void *)(Symtab + i * SYMESZ), (size_t)1, 
			(size_t)SYMESZ, Tfptr) != SYMESZ)
			quit(MSGSTR(WRITET_ERR_MSG, WRITET_ERR),Tfname);

		/* Write all aux entries */
		for(j = 0; j < sym_ptr->n_numaux; j++)
		{
			if (fwrite((void *)(Symtab + (++i) * SYMESZ), (size_t)1,
			(size_t)SYMESZ, Tfptr) != SYMESZ)
				quit(MSGSTR(WRITET_ERR_MSG, WRITET_ERR),Tfname);
		}
	}

	/* Write out size of string table if it exists */
	if(str_size > sizeof(long))
	{
		if (fwrite((void *)&str_size, (size_t)1, (size_t)sizeof(long), 
			Tfptr) != sizeof(long))
			quit(MSGSTR(WRITET_ERR_MSG, WRITET_ERR),Tfname);
	}

	return(0);
}

copy_unstripped_file()
/*******************************************************************************
	copy_unstripped_file	For use with only the -e and -E flags (when
		file is not actually being stripped).
	Copies the whole original file to the temp file before put_head
	writes the header with the F_LOADONLY bit set (or unset).
*******************************************************************************/
{
	register long tmpsize;

	/* Files are already open, so we just need to copy. */
	if (fseek(Fptr,0L,2) != 0 )
		quit(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);
	tmpsize=ftell(Fptr);
	if (fseek(Tfptr, 0L, 0) != 0)
		quit(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);
	if (fseek(Fptr,0L,0) != 0 )
		quit(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);
	(void)copy(Fptr, Tfptr, tmpsize);
}

already_stripped()
/*******************************************************************************
	already_stripped	Check to see if file is already stripped
		Determine if file is already stripped according to options.
	Use the flags in the filhdr to determine what has already been
	striped from the file.
*******************************************************************************/
{
	if ((Filhdr.f_nsyms == 0) && (!Hflag))	/* Any symbols ? */
		return(1);

	if (Lflag && !Xflag && !Rflag && (Filhdr.f_flags & F_LNNO))
		return(1);

	if (Xflag && !Lflag && (Filhdr.f_flags & F_LSYMS) &&
	(Filhdr.f_flags & F_RELFLG))
		return(1);

	if (Rflag && !Lflag && (Filhdr.f_flags & F_LSYMS))
		return(-1);

	if (Xflag && Lflag && (Filhdr.f_flags & F_LSYMS) &&
	(Filhdr.f_flags & F_RELFLG) && (Filhdr.f_flags & F_LNNO))
		return(-1);

	if (Rflag && Lflag && (Filhdr.f_flags & F_LSYMS) &&
	(Filhdr.f_flags & F_LNNO))
		return(-1);

	/* Need test for -T option */
	return(0);
}

get_head()
/*******************************************************************************	get_head	Get the file headers
		Read in the filhdr, any optional header, and section headers
	from the object file.
*******************************************************************************/
{
	if (fseek(Fptr, 0L, 0) !=0)
	{
		error(MSGSTR(SEEK_ERR_MSG, SEEK_ERR));
		return(1);
	}
	if (fread((void *)&Filhdr, (size_t)FILHSZ, (size_t)1, Fptr) != 1)
		return(1);

	if((Filhdr.f_magic != U802WRMAGIC) && (Filhdr.f_magic != U802ROMAGIC) &&
	(Filhdr.f_magic != U800WRMAGIC) && (Filhdr.f_magic != U800ROMAGIC) &&
	(Filhdr.f_magic != U802TOCMAGIC) && (Filhdr.f_magic != U800TOCMAGIC))
	    	return(1);

	if(Filhdr.f_opthdr)
	{
		if((Aouthdr=(AOUTHDR*)malloc((unsigned)Filhdr.f_opthdr))==NULL)
			return(1);
 		if(fread((void *)Aouthdr, (size_t)1, (size_t)Filhdr.f_opthdr, 
			Fptr) != (int)Filhdr.f_opthdr)
			return(1);
	}

	if(Filhdr.f_nscns)
	{
		if((Scnhdr = (SCNHDR *)malloc((unsigned)(Filhdr.f_nscns *
		SCNHSZ))) == NULL)
			return(1);
 		if(fread((void *)Scnhdr, (size_t)SCNHSZ, 
			(size_t)Filhdr.f_nscns, Fptr) < (int)Filhdr.f_nscns)
			return(1);
	}
	return(0);
}

put_head()
/*******************************************************************************
	put head	Put the headers to the temp file
		Copy the file headers to the temporary file.  These include
	the file header, optional header,  and section headers.  It is also
	required to the Filhdr at this time.
*******************************************************************************/
{
	if (Hflag)		/* Hflag indicates headers should be deleted */
		return(0);	/* so just return */

	if (!eflag && !Eflag)
		/* Set number of symbols and symbol table position */
		if (Filhdr.f_nsyms = Num_of_syms)
			Filhdr.f_symptr = Sym_pos;
		else
			Filhdr.f_symptr = 0L;

	/* Set file header flags according to what was stripped */
	if (Aflag)
		Filhdr.f_flags |= (F_RELFLG + F_LNNO + F_LSYMS);
	else if (Xflag)
		Filhdr.f_flags |= (F_RELFLG + F_LSYMS);
	else if (Lflag)
		Filhdr.f_flags |= (F_LNNO);
	else if (Rflag)
		Filhdr.f_flags |= (F_LSYMS);
	else if(Tflag)
		Filhdr.f_flags |= (F_RELFLG + F_LSYMS);
	else if (eflag)
		Filhdr.f_flags |= (F_LOADONLY);
	else if (Eflag)
		Filhdr.f_flags &= ~(F_LOADONLY);

	/* Seek to beginning of temp output file and */
	if (fseek(Tfptr, 0L, 0) != 0)
		quit(MSGSTR(SEEKT_ERR_MSG, SEEKT_ERR),Tfname);

	/* Write file header */
	if (fwrite((void *)&Filhdr, (size_t)FILHSZ, (size_t)1, Tfptr) != 1)
		quit(MSGSTR(WRITET_ERR_MSG, WRITET_ERR),Tfname);
	
	/* Write optional header */
	if (Filhdr.f_opthdr)
	{
		if (fwrite((void *)Aouthdr, (size_t)1, (size_t)Filhdr.f_opthdr,
			Tfptr) != (int)Filhdr.f_opthdr)
			quit(MSGSTR(WRITET_ERR_MSG, WRITET_ERR),Tfname);
	}

	/* Write the section headers */
	if (Filhdr.f_nscns)
	{
		if (fwrite((void *)Scnhdr, (size_t)SCNHSZ, 
			(size_t)Filhdr.f_nscns, Tfptr) != (int)Filhdr.f_nscns)
			quit(MSGSTR(WRITET_ERR_MSG, WRITET_ERR),Tfname);
	}

	return(0);
}



copy(fr, to, size)
/*******************************************************************************
	copy_file	File copy for size bytes
*******************************************************************************/
register FILE *fr, *to;
register long size; 
{       register int s;

	while (size != 0) {
		s = BUFSIZ;
		if (size < BUFSIZ) s = (unsigned)size;
		if (fread((void *)buf, (size_t)1, (size_t)s, fr) != s) 
			quit(MSGSTR(READ_ERR_MSG, READ_ERR));
		if (fwrite((void *)buf, (size_t)1, (size_t)s, to) != s)
			quit(MSGSTR(WRITET_ERR_MSG, WRITET_ERR),Tfname);
		size -= s;
	}
	return(0);
}


void reset_globals()
/*******************************************************************************
	reset_globals		Reset global variables and free memory
*******************************************************************************/
{
	/* Reset global counters to zero */
	Sym_pos =
	Data_rels =
	Txt_rels =
	Num_lnno =
	Strsize =
	Lnno_delta =
	Num_of_syms = 0;

	/* Free any memory associated with global pointers */
	if(Symtab)	{free((char *)Symtab);		Symtab = NULL;}
	if(Strtab)	{free((char *)Strtab);		Strtab = NULL;}
	if(Treloc)	{free((char *)Treloc);		Treloc = NULL;}
	if(Dreloc)	{free((char *)Dreloc);		Dreloc = NULL;}
	if(Lnno)	{free((char *)Lnno);		Lnno = NULL;};
	if(Sym_xref)	{free((char *)Sym_xref);	Sym_xref = NULL;}
	if(Str_xref)	{free((char *)Str_xref);	Str_xref = NULL;}
	if(Aouthdr)	{free((char *)Aouthdr);		Aouthdr = NULL;}
	if(Scnhdr)	{free((char *)Scnhdr);		Scnhdr = NULL;}

	return;
}


done()
/*******************************************************************************
	done
*******************************************************************************/
{
	(void)unlink(Tfname); 
	(void)exit(Exit_code);
}

void quit(char *fmt, ...)
/*******************************************************************************
	quit
*******************************************************************************/
{
	va_list args;

	Exit_code = -1;

	va_start(args, fmt);
	(void)fprintf(stderr, "strip: %s -- ", Fname);
	(void)vfprintf(stderr, fmt, args);
	(void)putc('\n', stderr);
	va_end(args);

	if (errno != 0)
	{
		(void) putc('\t',stderr);
		perror("");
	}

	done();
}

void error(char *fmt, ...)
/*******************************************************************************
	error
*******************************************************************************/
{
	va_list args;

	Exit_code = -1;

	va_start(args, fmt);
	(void)fprintf(stderr, "strip: %s -- ", Fname);
	(void)vfprintf(stderr, fmt, args);
	(void)putc('\n', stderr);
	va_end(args);

	if (errno != 0)
	{
		(void) putc('\t',stderr);
		perror("");
		errno = 0;
	}

	return;
}

