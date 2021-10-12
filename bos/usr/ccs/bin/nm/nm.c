static char sccsid[] = "@(#)59  1.16.1.17  src/bos/usr/ccs/bin/nm/nm.c, cmdaout, bos41B, 9505A 1/22/95 17:47:42";
/*
 * COMPONENT_NAME: CMDAOUT (nm command)
 *
 * FUNCTIONS: catchsig, cleanup, decotype, Demangle, error,
 *	get_scnhdrs, get_typelist, getauxname,
 *	getname, nm_arch, nm_fcn, nm_file, onintr, paren,
 *	Poutput, psyms, prepend, prosym, setflags, tagread
 *
 * ORIGINS: 27, 9, 3
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
 *
 */

/**************************************************************************
*                                                                         *
*	New nm.c constructed from SystemV.2 nm pieces, modified to	  *
*	handle XCOFF and use ARfuncs.					  *
*       --------------------------------				  *
*	Revised for V4 to comply with XPG4 standards, 1993/1994		  *
*									  *
**************************************************************************/

#include	<stdio.h>
#include	<signal.h>
#include	<strings.h>
#include	<stdlib.h>
#include	<xcoff.h>
#include	<dbxstclass.h>
#include	<IN/ARdefs.h>
#include	<errno.h>
#include	"nm_defs.h"
#include	<nl_types.h>
#include	"nm_msg.h"
#include	<locale.h>
#include	<demangle.h>

/* 
Maximum Symbol name length.  Added for the C++ compilier.  This
is an arbitrary limit.  In several places in the original code for
nm, there existed the number 256 as an array size.  That had to be
changed for the larger names that the C++ compilier might produce,
and at that time, all those explicit declarations were replaced 
with this define.  */   
/* 1993: Maximum Symbol length further increased from 1024 to 4096 */
#define	MAXSYMLEN 4096

#define MAX(x, y)       ((x) < (y) ? (y) : (x))

nl_catd		catd;
#define		MSGSTR(Num, Str) catgets(catd, MS_NM, Num, Str)

char	*getname(SYMENT *, int),	*getauxname(AUXENT *),
	*get_typelist(unsigned short);
void	prepend(char *, char *),	paren(char *),	error(char *),
	psyms(char *, long, int, int, int, int, AUXENT *, char *), 
	Poutput(char *, long, int, int, int, int, AUXENT *, char *);
static void nm_file(void);
int     onintr(void),	setflags(int, char **),	tagread(long, SYMENT *),
	get_scnhdrs(void);

static char *fmtsD[] = {
        FMTS_D_SDB_0,
        FMTS_D_SDB_1
};
static char *fmtsX[] = {
        FMTS_X_SDB_0,
        FMTS_X_SDB_1
};
static char *fmtsO[] = {
        FMTS_O_SDB_0,
        FMTS_O_SDB_1
};

static char *Title_buf;
static char **fmts=fmtsD;  /* number base formats (decimal default) */
static int Tflag=0;	/* 1 if names are to be truncated if needed */
static int vflag=0;	/* 1 if sort externals by value */
static int nflag=1;	/* 1 if sort externals by name (default) */
static int eflag=0;	/* 1 if only externals and statics wanted */
static int gflag=0;	/* 1 if only externals are wanted */
static int uflag=0;	/* 1 if undefined symbol output is wanted */
static int fflag=0;	/* 1 if full output is desired */
static int rflag=0;	/* 1 if sort order is to be reversed */
static int Aflag=0;	/* 1 if file name to be prepended to each line */
static int Bflag=0;	/* 1 if stdout is printed in old BSD format */
static int Cflag=0;	/* 1 if C++ names are not to be demangled */
static int Pflag=0;	/* 1 if portable format specified */
static int pflag=0;	/* 1 if no sorting wanted */

#ifdef HEADER_OPT
/* defining HEADER_OPT will cause nm to print a non-standard header
 * in front of each file (unless -P and/or -h are given).  
 * This could be used in the future if demand for an AIX-proprietary
 * nm option occurs.  However, it can't just be used as the default
 * output.  */
static int hflag=0;	/* 1 if no header is to be printed	  */
#else
			/* Header is always to be suppressed.     */
static int hflag=1;	/* 1 if no header is to be printed	  */
#endif

static int ret_code=0;  /* increment return code on error. */
static int show_fname=1; /* 1 if we need to show the name of file */
static int file_pad;    /* length of filename string, for padding */

static char *sortname;	/* name of tmp for sort	*/
static char sortcommand[MAXLEN];	/* command line to invoke sort	*/
static char *full_sortcmd;
static char outbuf[BUFSIZ];

static FILE *file, *file2, *sortout;
static long filepos;	/* position in file where object module starts */
static long Debug_offset; /* Offset to debug section for this file */
static struct filehdr hdr;
static SCNHDR *Scn_hdrs;
static char *filename, *membname;

/******************************************************************************
*	main(argc, argv)
*
*	parses the command line (setflags())
*	prepares to field interrupts  (catchsig())
*	opens, processes and closes each command line object file argument 
*	cleans up after itself (cleanup())
*
*	prints:
*	- a usage message if there are no command line object file args
*	- an error message if it can't open a command line obj file arg
*	  or if the opened object file doesnt have the right magic number
*******************************************************************************/

main(int argc, char **argv)
{
	int argnum;
	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_NM, NL_CAT_LOCALE);

	if (argc==1)
	{
		(void)fprintf(stderr,MSGSTR(USAGE_MSG, USAGE));
		exit(1);
	}

	catchsig();

	/*  setflags parses options from command line with getopt() */
	argnum = setflags(argc, argv);

	if (argnum >= argc)
	{
		(void)fprintf(stderr,MSGSTR(USAGE_MSG, USAGE));
		exit(1);
	}

	if (argnum == argc - 1) /* if only one file; don't show filename */
		show_fname=0;

	for (  ; argnum < argc; ++argnum)
	{
		filename = argv[argnum];
		membname=NULL;
		if ((file=fopen(filename,"r"))==NULL)
		{
			error(MSGSTR(OPEN_ERR_MSG, OPEN_ERR));
			continue;
		}
		if ((file2=fopen(filename,"r"))==NULL)
		{
			error(MSGSTR(REOPEN_ERR_MSG, REOPEN_ERR));
			(void)fclose(file);
			continue;
		}

		if (ARisarchive(file))
			nm_arch();
		else
			nm_file();

		(void)fclose(file2);
		(void)fclose(file);
	}

	cleanup();
	exit(ret_code);
}

/*******************************************************************************
*	prints the given error message indicating the offending file
*******************************************************************************/
void error(char *message)
{
	int save_errno = errno; /* ensure errno doesn't get overridden
				   before we use perror() ! */

	if (sortout!=NULL)
		(void)fflush(sortout);
	if (membname==NULL)
		(void)fprintf(stderr, NM_ERR_MSG,filename,message);
	else
		(void)fprintf(stderr, NM_AR_ERR_MSG,filename,membname,
			message);
	if (save_errno != 0)
	{
		errno = save_errno;  /* put it back, in case it changed. */
		perror("nm");
		errno = 0;
	}

	++ret_code;
}

/*******************************************************************************
*	cleans up after an interrupt happens (or after a FATAL error)
*	ignores signals (interrupts) during its work
*******************************************************************************/
int onintr(void)
{
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGHUP, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGTERM, SIG_IGN);
	(void)fprintf(stderr, "\n");
	if (sortout!=NULL)
		(void)fclose(sortout);

	(void)unlink(sortname);
	exit(1);
}

/*******************************************************************************
*	prepares to field interrupts with the routine onintr()
*	namelist fields interrupts so that it can remove the temporary files
*	it has created if it should be interrupted
*******************************************************************************/
catchsig()
{
	if ((signal(SIGINT, SIG_IGN)) == SIG_DFL)
		(void)signal(SIGINT, (void (*)(int))onintr);
	if ((signal(SIGHUP, SIG_IGN)) == SIG_DFL)
		(void)signal(SIGHUP, (void (*)(int))onintr);
	if ((signal(SIGTERM, SIG_IGN)) == SIG_DFL)
		(void)signal(SIGTERM, (void (*)(int))onintr);
}

/*******************************************************************************
*	is called by main() when namelist terminates normally
*	cleanup simply unlinks temporary files
*******************************************************************************/
cleanup()
{
	(void)unlink(sortname);
	free(full_sortcmd);
}

/*******************************************************************************
*	nm_fcn
*******************************************************************************/
nm_fcn(ARparm *parm)
{
	if (parm->name[0]!=0)
	{
		membname=parm->name;
		nm_file();
	}
	return(0);
}

/*******************************************************************************
*	nm_arch
*******************************************************************************/
nm_arch()
{
	membname=NULL;    
	ARforeach(file,nm_fcn);
	membname=NULL;
}

/*******************************************************************************
*	decotype.h contains format strings used to print out the type field of
*	a symbol table entry
*
*	the minimum number of characters printed depends on the room 
*	left by the other fields in the output line (the type field may grow
*	beyond this minimum) and differs depending on the setting of -e and
*	the number base formats.
*******************************************************************************/
/*******************************************************************************
*	decotype(symbol, aux)
*
*	decodes the type and derived type field of the given symbol table entry
*	using information in the given auxiliary entry for array dimensions
*	or to find the tag name of a structure, union or enumeration;
*	writes the decoded information using format strings defined
*	in decotype.h
*
*	calls the namelist functions prepend() and paren() to build the
*	decoded type string
*
*	returns 1 or 0
*	decotype will fail only if it cannot read a structure, union or
*	enumeration tag entry that is supposed to be there
*******************************************************************************/
#define N_BTMASK    017
#define N_TMASK     060
#define N_TMASK1    0300
#define N_TMASK2    0360
#define N_BTSHFT    4
#define N_TSHIFT    2

decotype(SYMENT *sym, AUXENT *aux)
{
	char typestr[TYPELEN],tmpstr[TMPLEN];
	unsigned short type,dertype,dtype,lastdtyp;
	SYMENT tag;
	int arydim, class;

       /* "typestr[ ]" is used to build up the decoded type and derived type
	* typestr will remain null ('\0') if the type/derived type field is
	* undefined (0) */

	typestr[0] = '\0';
	type = sym->n_type & N_BTMASK;
	class = sym->n_sclass;
	if (sym->n_type)
        (void)strcat(typestr,get_typelist(type));

	if ((type==T_STRUCT || type==T_UNION || type==T_ENUM) && 
	(((class==C_EXT || class==C_HIDEXT) && (int)(sym->n_numaux & 0xff)>1) ||
	((class!=C_EXT && class!=C_HIDEXT) && (int)(sym->n_numaux & 0xff)>0)))
	{
		if (aux->x_sym.x_tagndx != 0)
		{
			if (tagread(aux->x_sym.x_tagndx,&tag) != 1)
				return(0);
			(void)sprintf(tmpstr,TAGNDX_FMT,getname(&tag,class));
			(void)strcat(typestr,tmpstr);
		}
	}

       /*  the derived type indicators are decoded from right to left until
	*  a null field is found (DT_NON)
	*  derived type pointer (DT_PTR) is indicated by a prepended "*"
	*  derived type array (DT_ARY) is indicated by an appended "[ ]"
	*    (the array dimesion is filled in if it is present in an aux entry)
	*  derived type function (DT_FCN) is indicated by an appended "()"
	*  since derived types function and array associate more closely with
	*    a basic type than does derived type pointer, parenthesis are used
	*    to associate pointer more closely with the basic type if it 
	*    appears to the right of an array or function derived type
	*/

	dertype = sym->n_type;
	arydim = -1;
	lastdtyp = DT_NON;
	while ((dtype = (dertype & N_TMASK) >> N_BTSHFT) != DT_NON)
	{
		if (dtype==DT_PTR)
			prepend("*",typestr);
		else 
		{
			if (lastdtyp==DT_PTR)
				paren(typestr);

			if (dtype==DT_FCN)
				(void)strcat(typestr,"()");
			else if ((++arydim < 4) &&
			(((class==C_EXT || class==C_HIDEXT) &&
			(int)(sym->n_numaux & 0xff)>1) ||
			((class!=C_EXT && class!=C_HIDEXT) &&
			(int)(sym->n_numaux & 0xff)>0)))
			{
				(void)sprintf(tmpstr,"[%hd]",
				aux->x_sym.x_fcnary.x_ary.x_dimen[arydim]);
				(void)strcat(typestr,tmpstr);
			}
			else
				(void)strcat(typestr,"[ ]");
		}

		lastdtyp = dtype;
		dertype >>= N_TSHIFT;
	}

	return(1);
}

/*******************************************************************************
*	prepend(new,base)
*
*	prepends the string "new" to the string "base"
*	the result is left in "base"
*******************************************************************************/
void prepend(char *new, char *base)
{
	int i,j;

	for (i = 0; base[i] != '\0'; ++i);
	for (j = 0; new[j] != '\0'; ++j);
	for ( ; i >= 0; --i) base[i + j] = base[i];
	for (i = 0; j > 0; --j, ++i) base[i] = new[i];
	return;
}

/*******************************************************************************
puts parenthesis around the string "base"
*******************************************************************************/
void paren(char *base)
{
    register int i;

	for (i = 0; base[i] != '\0'; ++i);
	base[i] = ')';

	base[++i] = '\0';

	for ( ; i >= 0; --i) base[i + 1] = base[i];
	base[0] = '(';

	return;
}

/*******************************************************************************
*	directs the namelisting of an individual object file
*	process verifies that there are in fact symbols in the named file
*	calls prosym to print the symbol table
*	based on -v or -n flag invokes system sort routine
*******************************************************************************/
static void nm_file(void)
{

	filepos=ftell(file);
	if (fread((void *)&hdr,(size_t) 1,(size_t) FILHSZ, file)< sizeof hdr)
	{
		errno = 0;  /* don't want perror message for this one. */
		error(MSGSTR(NOT_OBJECT_ERR_MSG, NOT_OBJECT_ERR));
		return;
	}

	if((hdr.f_magic != U802WRMAGIC) && (hdr.f_magic != U802ROMAGIC) &&
	(hdr.f_magic != U800WRMAGIC) && (hdr.f_magic != U800ROMAGIC) &&
	(hdr.f_magic != U802TOCMAGIC) && (hdr.f_magic != U800TOCMAGIC))
	{
		errno = 0;  /* don't want perror message for this one. */
		error(MSGSTR(NOT_XCOFF_ERR_MSG, NOT_XCOFF_ERR));
		return;
	}

	if (hdr.f_nsyms==0) 
	{	/* no symbols in file (not a true error). */
		if (sortout!=NULL)
		    (void)fflush(sortout);
		if (membname==NULL)
		    (void)fprintf(stdout, NM_ERR_MSG,filename,
			MSGSTR(NO_SYMBOLS_MSG, NO_SYMBOLS));
		else
		    (void)fprintf(stdout, NM_AR_ERR_MSG,filename,membname,
			MSGSTR(NO_SYMBOLS_MSG, NO_SYMBOLS));
		return;
	}

	if (!pflag)
		{
		if ((sortout=fopen(sortname,"w"))==NULL)
			{
			error(MSGSTR(TEMP_OPEN_ERR_MSG, TEMP_OPEN_ERR));
			onintr();
			} /* exits */
		}
	else    /* pflag ; set to stdout */
		sortout = stdout;
		
#ifndef HEADER_OPT
	/* Show name of file, if more than one file  */
	if (!Aflag && (membname || show_fname))
	{
		if (membname)
			(void)printf("%s[%s]:\n", filename, membname);
		else
			(void)printf("%s:\n", filename);
        }
#else
	if (!hflag)
	{
		if (uflag)
		{
			if (membname)
				(void)printf(MSGSTR(UNDEF_SYM_MEM_MSG,
					UNDEF_SYM_MEM), filename, membname);
			else
				(void)printf(MSGSTR(UNDEF_SYM_FIL_MSG,
					UNDEF_SYM_FIL), filename);
		}
		else
		{
			if (membname)
				(void)printf(MSGSTR(DEF_SYM_MEM_MSG,
					DEF_SYM_MEM), filename, membname);
			else
				(void)printf(MSGSTR(DEF_SYM_FIL_MSG,
					DEF_SYM_FIL), filename);
		}
	}
#endif

	if (prosym()==0)
	{
		(void)fflush(sortout);
		errno = 0; /* don't want perror message for this one. */
		error(MSGSTR(BAD_SYMTAB_ERR_MSG, BAD_SYMTAB_ERR));
		return;
	}
	(void)fflush(sortout);

	if (!pflag)  /* don't want to close output file if it's stdout! */
	{
		(void)fflush(stdout);
		(void)fclose(sortout);
		sortout=NULL;

		if (system(full_sortcmd) != 0)
		{
		errno = 0; /* don't want perror message for this one. */
		error(MSGSTR(SORT_ABEND_ERR_MSG, SORT_ABEND_ERR));
		onintr();
		}
	}
}

/*******************************************************************************
*	prosym()
*	finds the symbol table
*	prints a heading
*	prints out each symbol table entry
*	returns 1 for success, 0 for a messed up symbol table
*******************************************************************************/
prosym() 
{
	SYMENT symbol;
	AUXENT aux,taux;
	register char *name;
	int name_err=0, i;
	register section,class,numaux,type;
	register long symindx,value;
	char *aux_fname = NULL;
	char name_buf[20], *name_ptr;
	char *full_name;

	if (get_scnhdrs())
		return(0);
	if (fseek(file,filepos+hdr.f_symptr,0)!=0)
		return(0);

	if (Aflag)
	{
		/* full_name - so we can have filename[membname] for archives */
		name_ptr = filename;
		if (membname == NULL)
		{
			full_name = (char *)malloc(strlen(name_ptr) + 2);
			sprintf(full_name, "%s:\0",name_ptr);
		}
		else
		{
			full_name = (char *)malloc(strlen(name_ptr) +
				strlen(membname) + 4);
			sprintf(full_name, "%s[%s]:\0",name_ptr,membname);
		}
		/* size of filename; make at least 9 (size of "filename ") */
		file_pad = MAX((int)strlen(full_name),9);

		if (Tflag)
		{
			strncpy(name_buf, name_ptr, 14);
			name_buf[14] = '\0';
			name_ptr = name_buf;
		}
	}

#ifdef HEADER_OPT
	if (!hflag && !Bflag)
	{
		if (Aflag)
			(void)printf("%1$-*2$s ", MSGSTR(FILE_NAME_MSG,
				FILE_NAME), file_pad);

		(void)printf(Title_buf);
	}
#endif

	for (symindx = 0L; symindx < hdr.f_nsyms; ++symindx)
	{
		if (fread((void *) &symbol,(size_t) 1,(size_t) SYMESZ,file)
		  !=SYMESZ)
			return(0);

		section = symbol.n_scnum;
		class = symbol.n_sclass;
		type = symbol.n_type;
		numaux = symbol.n_numaux;
		value = symbol.n_value;

		if (numaux > 0)
		{
			if (fread((void *) &aux,(size_t) AUXESZ,
			  (size_t) 1,file)!=1)
				return(0);

			for(i = 1; i < numaux; i++)
			{
				if (fread((void *) &taux,(size_t) AUXESZ,
				  (size_t) 1,file)!=1)
					return(0);
			}

			symindx += numaux;
		}

		name_err |= (name=getname(&symbol,class))==NULL;

		/* don't show .text, .data, .bss if fflag not given. */
		if (fflag == 0 && (strncmp(name, DOT_TEXT, 8) == 0 ||
			strncmp(name, DOT_DATA, 8) == 0 ||
			strncmp(name, DOT_BSS, 8) == 0 ||
			strcmp(name, NO_NAME_STR) == 0) )
				continue;

		if (uflag)
		{
			if (section==N_UNDEF)
				{
				if(Bflag)
					psyms(name,value,class,section,
						type,numaux,&aux,full_name);
				else
					Poutput(name,value,class,section,
						type,numaux,&aux,full_name);
				(void)fprintf(sortout,"\n");
				}
			continue;
        	}

		if (eflag && class!=C_EXT && (class!=C_HIDEXT && class!=C_STAT))
		{
			if (class == C_FILE)
				aux_fname = numaux > 0 ? getauxname(&aux): name;
			continue;
		}

		
		if (gflag && class!=C_EXT)	/* gflag for externals only. */
		{	
			if (class == C_FILE)
				aux_fname = numaux > 0 ? getauxname(&aux): name;
			continue;
		}

		if(section==N_DEBUG && Bflag)
		{
			if(Aflag)
			{
				psyms(name,value,class,section,
					type,numaux,&aux,full_name);
				(void)fprintf(sortout,"\n");
			}
			continue;
		}
		if(Bflag)
		{
			psyms(name,value,class,section,type,numaux,&aux,
				full_name);
		}
		else
		{
			Poutput(name,value,class,section,type,numaux,&aux,
				full_name);
		}

                if (decotype(&symbol,&aux)==0)
                        return(0);

		/* Print the size information for the symbol (if avail.)  */
		if (!Bflag)   /* BSD version never has included this.... */
               	    if (class == C_EXT || class == C_HIDEXT) {
			   register long symsize; /* size of csect */
			   register long fsize;   /* size of function length */
			   register stype;
			   if (numaux > 1) {
			      fsize = aux.x_sym.x_misc.x_fsize;
                              symsize = taux.x_csect.x_scnlen;
			      stype = taux.x_csect.x_smtyp & 0x07;
			   }
			   else {
                              symsize = aux.x_csect.x_scnlen;
			      stype = aux.x_csect.x_smtyp & 0x07;
			   }

                           /* print csect size info if XTY_SD or XTY_CM */
			   if (numaux > 0 && symsize != 0  &&
			      (stype == XTY_SD || stype == XTY_CM) )  {
				   if (Pflag)
				       (void)fprintf(sortout,fmts[Psize],
					(unsigned)symsize,"\n");
				   else
				       (void)fprintf(sortout,fmts[noPsize],
					(unsigned)symsize,"\n");
				   }
			   /* print function entry size if XTY_LD */
			   else if (numaux > 1 && fsize!=0 && stype==XTY_LD) {
				   if (Pflag)
                                	(void)fprintf(sortout,fmts[Psize],
						(unsigned)fsize,"\n");
				   else
                                	(void)fprintf(sortout,fmts[noPsize],
						(unsigned)fsize,"\n");
				   }
			}

	        /* print newline at end of symbol entry */
	        (void)fprintf(sortout,"\n");
	} /* For each symbol */

	free(full_name);
	return(!name_err);
}


/*******************************************************************************
*	psyms - print out symbol table entry in BSD format
*******************************************************************************/
void psyms(char *name, long value, int class, int section, int type, 
    int numaux, AUXENT *auxptr, char *nameptr)
{
	register int c;
	char *auxname;

	if (class == C_FILE)
	{
		c = 'f';
		auxname = numaux > 0 ? getauxname(auxptr): name;
		value = 0;
	}
	else if (section <= 0) switch (section)
	{
		case N_UNDEF:
			c = 'u';
			break;
		case N_ABS:
			c = 'a';
			break;
		case N_DEBUG:
			c = '-';
			break;
		default:
			c = '?';
			break;
	}
	else
	{
	    if((Scn_hdrs[section - 1].s_flags & 0x0000ffff) == STYP_TEXT)
		 c = 't';
	    else if((Scn_hdrs[section - 1].s_flags & 0x0000ffff) == STYP_DATA)
		 c = 'd';
	    else if((Scn_hdrs[section - 1].s_flags & 0x0000ffff) == STYP_BSS)
		 c = 'b';
	    else if((Scn_hdrs[section - 1].s_flags & 0x0000ffff) == STYP_INFO)
		 c = '-';
	    else c = '?';
	}

	if (Aflag) fprintf(sortout, "%1$-*2$s ", nameptr, file_pad);

	if (class==C_EXT)
		c = toupper(c);

	if (c == 'f' || ((c == 'u' || c == 'U') && value == 0))
		fprintf(sortout, "         -");
	else
		if (fmts==fmtsX)
			fprintf(sortout, "%#010lx", (unsigned)value);
		else if (fmts==fmtsD)
			fprintf(sortout, "%10u", (unsigned)value);
		else if (fmts==fmtsO)
			fprintf(sortout, "%#10lo", (unsigned)value);

	fprintf(sortout, " %c ", c);

	if (c == 'f')
		fprintf(sortout, "%s", auxname);
	else
		fprintf(sortout, "%s", name);
	return;
}

/*******************************************************************************
*	Poutput - print out symbol table entry in Portable (-P) format
*		similar to BSD output, but format order is different.
*******************************************************************************/
void Poutput(char *name, long value, int class, int section, int type, 
    int numaux, AUXENT *auxptr, char *nameptr)
{
	register int c;
	char *auxname;

	if (class == C_FILE)
	{
		c = 'f';
		auxname = numaux > 0 ? getauxname(auxptr): name;
		value = 0;
	}
	else if (section <= 0) switch (section)
	{
		case N_UNDEF:
			c = 'u';
			break;
		case N_ABS:
			c = 'a';
			break;
		case N_DEBUG:
			c = '-';
			break;
		default:
			c = '?';
			break;
	}
	else
	{
	    if((Scn_hdrs[section - 1].s_flags & 0x0000ffff) == STYP_TEXT)
		 c = 't';
	    else if((Scn_hdrs[section - 1].s_flags & 0x0000ffff) == STYP_DATA)
		 c = 'd';
	    else if((Scn_hdrs[section - 1].s_flags & 0x0000ffff) == STYP_BSS)
		 c = 'b';
	    else if((Scn_hdrs[section - 1].s_flags & 0x0000ffff) == STYP_INFO)
		 c = '-';
	}
	if (Aflag) fprintf(sortout, "%1$-*2$s ", nameptr, file_pad);

	if (class==C_EXT)
		c = toupper(c);

	if (c == 'f')
		fprintf(sortout, "%-20s", auxname);
	else
		fprintf(sortout, "%-20s", name);

	fprintf(sortout, " %c ", c);

	if (c == 'f' || ((c == 'u' || c == 'U') && value == 0))
		fprintf(sortout, "         -");
	else
		/* If Pflag is given, we won't print leading 0x or 0  */
		if (fmts==fmtsX) {
			if (Pflag)
			    fprintf(sortout, "%10lx", (unsigned)value);
			else
			    fprintf(sortout, "%#010lx", (unsigned)value);
			}
		else if (fmts==fmtsD)
			fprintf(sortout, "%10u", (unsigned)value);
		else if (fmts==fmtsO) {
			if (Pflag)
			    fprintf(sortout, "%10lo", (unsigned)value);
			else
			    fprintf(sortout, "%#10lo", (unsigned)value);
			}
	return;
}


/*******************************************************************************
*  Demangles a C++ name if the Cflag is not specified 
*******************************************************************************/
static void Demangle(char *name, int class)
{
	if (!Cflag && class != C_DECL)
	{
		char *rest ;
		struct Name *nm ;
		int containsDot = 0;	/* used for leading dot					*/
		char *textReturn;	/* holds return text from demangle */

		if (name[0]=='.')
		{
			/* 
			Copy everything after the dot and pass it to demangle.
			The dot will be in the first position....
			'demangle' doesn't handle the dot
			*/
			nm = demangle(name+1, &rest, RegularNames);
			containsDot = 1;	/* we contain a dot */
		}
		else
			nm = demangle(name,&rest,RegularNames) ;

		/*
		If demangle can't unmangle the name then return
		*/
		if (nm == NULL)
			return; 


		/*
		get the unmangled name
		*/
		textReturn = text(nm);
		if (textReturn != NULL)
		{
			if (containsDot == 1)
			{
				/*
				Put the dot back on front of the name if it had
				one to begin with
				*/
				name[0]= '.';
				(void)strncpy(name+1,textReturn,(MAXSYMLEN-1));
			}	
			else
				(void)strncpy(name,textReturn,MAXSYMLEN) ;

		/* 
		Null terminate name before leaving because the symbol name might
		have been longer than 1024 bytes.
		*/
		/* put a null terminator on end of array name 		*/	
		/* in case the symbols where larger than could fit	*/
		name[MAXSYMLEN+1]= '\0';
		}
		erase(nm);
	}
}


/*******************************************************************************
*	getname - get the name of a symbol in a permanent fashion
*******************************************************************************/
char *getname(SYMENT *sym, int class)
{
	static char name[MAXSYMLEN+1];
	register int count=MAXSYMLEN;
	register char *np=name;
	register int ch;
	if (sym->n_zeroes!=0)
	{
		(void)strncpy(name,sym->n_name,SYMNMLEN);
		name[SYMNMLEN] = '\0';
		Demangle(name,class) ;
		return(name);
	}

	if(sym->n_offset == 0)
	{
		return MSGSTR(NO_NAME_STR_MSG, NO_NAME_STR);
	}

	if(sym->n_sclass & DBXMASK)
	{
		if(Debug_offset > 0)
			(void)fseek(file2, Debug_offset + sym->n_offset, 0);
		else
			return MSGSTR(BAD_NAME_STR_MSG, BAD_NAME_STR);
	}
	else
		(void)fseek(file2, filepos + hdr.f_symptr + hdr.f_nsyms*SYMESZ
			+ sym->n_offset, 0);

	while ((ch=getc(file2))!=0 && ch!=EOF && --count>=0)
		*np++=ch;

	*np=0;

	if (ch==EOF)
		return MSGSTR(BAD_NAME_STR_MSG, BAD_NAME_STR);
	Demangle(name,class) ;
	if (strlen(name)>20 && Tflag)
	{
		name[19]='*';
		name[20]=0;
	}
	return(name);
}


/*******************************************************************************
*	getauxname - get the file name in an auxilary entry 
*******************************************************************************/
char *getauxname(AUXENT *aux)
{
	static char name[MAXSYMLEN+1];
	register int count=MAXSYMLEN;
	register char *np=name;
	register int ch;
	if (aux->x_file._x.x_zeroes != 0)
	{
		(void)strncpy(name,aux->x_file.x_fname,FILNMLEN);
		name[FILNMLEN] = '\0';
		return(name);
	}

	if(aux->x_file._x.x_offset == 0)
	{
		return MSGSTR(NO_NAME_STR_MSG, NO_NAME_STR);
	}

	(void)fseek(file2, filepos + hdr.f_symptr + hdr.f_nsyms*SYMESZ +
		aux->x_file._x.x_offset, 0);

	while ((ch=getc(file2))!=0 && ch!=EOF && --count>=0)
		*np++=ch;

	*np=0;

	if (ch==EOF)
		return MSGSTR(BAD_NAME_STR_MSG, BAD_NAME_STR);

	if (strlen(name)>20 && Tflag)
	{
		name[19]='*';
		name[20]=0;
	}
	return(name);
}


/******************************************************************************
*	scans the command line for flag arguments, setting appropriate flags
*	eliminates flag arguments from the command line
*	forms temporary file names and command strings for -n or -v flags
*	- sortcommand=sort sortfield sortname  if -v or -n flag on
*	  sortfield is set to name field (+0 -1) if -n flag is on
*	  sortfield is set to value field (+2 -3) if -v flag is on
*	  sortfield is set to both (concatenated) if both flags are on
*
*	returns filec, the number of non flag arguments in the command line
*	(each is assumed to be an object file or archive file name)
*******************************************************************************/
int setflags(int flagc, char **flagv)
{
	static char sortfield[8]="";  /* holds the options to pass to sort */
	int filec=0;
	short errflg=0, setfmt=0;
	char **filev;
	char *tmpdir;		      /* TMPDIR env var; "/tmp" default. */
	extern int optind;
	extern char *optarg;

	while ((filec = getopt(flagc, flagv, "ABCfhPprTvgeudoxnOt:")) != EOF)
	{
		switch (filec) {
			case 'A': Aflag=1;
				break;
			case 'd': fmts=fmtsD;
				setfmt = 1;
				break;
			case 'o': fmts=fmtsO;
				setfmt = 1;
				break;
			case 'x': fmts=fmtsX;
				setfmt = 1;
				break;
			case 'f': fflag=1;
				break;
			case 'P':
				if (Bflag)
					errflg++;
				else
					Pflag=1;
				hflag=1;
				break;
			case 'p': pflag=1;
				break;
			case 'T': Tflag=1;
				break;
			case 'v': vflag=1;
				nflag=0;
				break;
			case 'e':
				if (gflag || uflag)
					errflg++;
				else
					eflag=1;
				break;
			case 'g':
				if (eflag || uflag)
					errflg++;
				else
					gflag=1;
				break;
			case 'u':
				if (eflag || gflag)
					errflg++;
				else
					uflag=1;
				break;
			case 'h': hflag=1; 
				break;
			case 'r': rflag=1;
				break;
			case 'C': Cflag=1;
				break;
			case 't':
				if (*optarg == 'x')
					fmts=fmtsX;
				else if (*optarg == 'd')
					fmts=fmtsD;
				else if (*optarg == 'o')
					fmts=fmtsO;
				else errflg++;
				setfmt = 1;
				break;
			case 'O': Aflag=1; /* no typo; for bkwd cmpat */
				break;
			case 'n': nflag=1;
				vflag=0;
				break;
			case 'B':
				if (Pflag)
					errflg++;
				else
					Bflag=1;
				break;
			default:
				errflg++;
				break;
                		} /* switch */
	} /* while */

	if (errflg)
	{
		(void)fprintf(stderr,MSGSTR(USAGE_MSG, USAGE));
		exit(1);
	}

	/* Decimal is default, but hex is if -P flag given. */
	if (Pflag && !setfmt)
		fmts = fmtsX;

	/* FMTS_D_0 is the header message.  Octal and Hex have same header. */
	/*  (Name should be FMTS_HEADER or something, but is left as is
	    because it has been translated w/ that name in nm.msg.)  */
	if((Title_buf = malloc(strlen(MSGSTR(FMTS_D_0_MSG,
	    FMTS_D_0)) + 2)) == NULL)
	{
		error(MSGSTR(NO_MEM_MSG, NO_MEM));
		exit(-1);
	}
	(void)strcpy(Title_buf, MSGSTR(FMTS_D_0_MSG, FMTS_D_0));
					
	setbuf(stdout,outbuf);

	/* get a unique filename in temporary directory (/tmp default). */
        tmpdir = getenv("TMPDIR");
        sortname = tempnam(tmpdir,NULL);

        /* set sort flags */
	if ((nflag && Bflag) || (vflag && !Bflag))
	{
		if(Aflag)
			(void)strcat(sortfield, "+3 -4 ");
		else
			(void)strcat(sortfield, "+2 -3 ");
	}
	else 
	{
               	if(Aflag)
			(void)strcat(sortfield, "+1 -2 ");
		else
			(void)strcat(sortfield, "+0 -1 ");
	}

        if (rflag)
                (void)sprintf(sortcommand,"sort -r %s",
                        sortfield);
        else
                (void)sprintf(sortcommand,"sort %s",
                        sortfield);

	if((full_sortcmd =
	  (char *)malloc((strlen(sortname)+MAXLEN)*sizeof(char))) == NULL)
	{
		error(MSGSTR(NO_MEM_MSG, NO_MEM));
		exit(-1);
	}
	full_sortcmd = strcat(sortcommand,sortname);

	return(optind);
}

/******************************************************************************
* tagread
******************************************************************************/
int tagread(long symnum, SYMENT *sym)
{
	return(0<=symnum && symnum < hdr.f_nsyms &&
		!fseek(file2,filepos+hdr.f_symptr+symnum*SYMESZ,0) &&
		fread((void *) sym,(size_t) SYMESZ,(size_t) 1,file2)==1);
}

/******************************************************************************
* get_scnhdrs
******************************************************************************/
int get_scnhdrs(void)
{
	register i;

	if(Scn_hdrs != NULL)
		free((char *)Scn_hdrs);

	if((Scn_hdrs = (SCNHDR *)malloc(SCNHSZ * hdr.f_nscns)) == NULL)
		return(-1);

	if(fseek(file, (long)(filepos + FILHSZ + hdr.f_opthdr), 0) ||
	(fread((void *) Scn_hdrs,(size_t) SCNHSZ,(size_t)hdr.f_nscns, file) 
	  != hdr.f_nscns))
		return(-1);

	Debug_offset = -1;
	for(i=0; i < hdr.f_nscns; i++)
	{
		if((Scn_hdrs[i].s_flags & STYP_DEBUG) == STYP_DEBUG)
		{
			Debug_offset = Scn_hdrs[i].s_scnptr;
			break;
		}
	}

	return(0);
}

/******************************************************************************
* get_typelist
******************************************************************************/
char *get_typelist(unsigned short type)
/*
 * Get the character string representation for the symbol type.
 */
{
	switch(type)
	{
	case T_NULL:	return(VOID_TYPE);
	case T_ARG:	return(ARG_TYPE);
	case T_CHAR:	return(CHAR_TYPE);
	case T_SHORT:	return(SHORT_TYPE);
	case T_INT:	return(INT_TYPE);
	case T_LONG:	return(LONG_TYPE);
	case T_FLOAT:	return(FLOAT_TYPE);
	case T_DOUBLE:	return(DOUBLE_TYPE);
	case T_STRUCT:	return(STRUCT_TYPE);
	case T_UNION:	return(UNION_TYPE);
	case T_ENUM:	return(ENUM_TYPE);
	case T_MOE:	return(ENMEM_TYPE);
	case T_UCHAR:	return(UCHAR_TYPE);
	case T_USHORT:	return(USHORT_TYPE);
	case T_UINT:	return(UINT_TYPE);
	case T_ULONG:	return(ULONG_TYPE);
	default:	return("???");
	}
}
