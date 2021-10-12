static char sccsid[] = "@(#)42  1.37.1.3  src/bos/usr/bin/od/od.c, cmdscan, bos41J, 9507C 2/9/95 14:15:34";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 71
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */


/*
 * od -- octal, hex, decimal, character dump of data in a file.
 *
 * usage:  od [-abBcCdDefFhHiIlLoOpPsvxX] [-S [n]] [-w [n]] [file] [[+]offset[.|b|B]] [[+]label[.|b|B]]
 *
 * where the option flags have the following meaning:
 *   character	object	radix	signed?
 *	a	byte	(10)	(n.a.)	ASCII named byte stream
 *	b	byte	  8	 no	byte octal
 *	c	byte	 (8)	(no)	character with octal non-graphic bytes
 *	d	short	 10	 no
 *	D	long	 10	 no
 *	e,F	double	(10)		double precision floating pt.
 *	f	float	(10)		single precision floating pt.
 *	h,x	short	 16	 no
 *	H,X	long	 16	 no
 *	i	short	 10	yes
 *	I,l,L	long	 10	yes
 *	o,B	short	  8	 no	(default conversion)
 *	O	long	  8	 no
 *	s	short	 10	yes
 *	S[n]	string	 (8)		ASCII graphic strings
 *
 *	p				indicate EVEN parity on 'a' conversion
 *	P				indicate ODD parity on 'a' conversion
 *	v				show all data - don't skip like lines.
 *	w[n]				bytes per display line
 *
 * More than one format character may be given.
 * If {file} is not specified, standard input is read.
 * If {file} is not specified, then {offset} must start with '+'.
 * {Offset} may be HEX (0xnnn), OCTAL (0nn), or decimal (nnn.).
 * The default is octal. The same radix will be used to display the address.
 */

#define _ILS_MACROS

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <nl_types.h>
#include <ctype.h>
#include "od_msg.h"

static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_OD,Num,Str)

#define DBUF_SIZE	BUFSIZ
#define BIG_DBUF	32
#define NO		0
#define YES		1
#define EVEN	       -1
#define ODD		1
#define UNSIGNED	0
#define SIGNED		1
#define PADDR		1
#define MIN_SLEN	3
#define STDIN		0

static int	a_put();
static int	b_put();
static int	c_put();
static int	C_put();
static int	do_C_put();
static int	s_put();
static int	us_put();
static int	l_put();
static int	f_put();
static int	d_put();
static int	st_put();
static int	parity();
static int	canseek();
static int	put_sbuf();
static void	dumbseek();
static void	offset();
static void	pr_sbuf();
static void     put_addr(offset_t addrs, offset_t labl, char chr);
static void	line();
static int 	usage();

static struct dfmt {
	int	df_field;	/* external field required for object */
	int	df_size;	/* size (bytes) of object */
	int	df_radix;	/* conversion radix */
	int	df_signed;	/* signed? flag */
	int	df_paddr;	/* "put address on each line?" flag */
	int	(*df_put)();	/* function to output object */
	char	*df_fmt;	/* output string format */
} *conv_vec[128];		/* vector of conversions to be done */

static struct dfmt	ascii	= { 3, sizeof (char),   10,        0, PADDR,  a_put, 0};
static struct dfmt	byte	= { 3, sizeof (char),    8, UNSIGNED, PADDR,  b_put, 0};
static struct dfmt	cchar	= { 3, sizeof (char),    8, UNSIGNED, PADDR,  c_put, 0};
static struct dfmt	Cchar	= { 3, sizeof (char),    8, UNSIGNED, PADDR,  C_put, 0};
static struct dfmt	u_s_oct	= { 6, sizeof (short),   8, UNSIGNED, PADDR, us_put, 0};
static struct dfmt	u_c_dec	= { 3, sizeof (char),   10, UNSIGNED, PADDR,  b_put, 0};
static struct dfmt	u_s_dec	= { 5, sizeof (short),  10, UNSIGNED, PADDR, us_put, 0};
static struct dfmt	u_s_hex	= { 4, sizeof (short),  16, UNSIGNED, PADDR, us_put, 0};
static struct dfmt	u_c_hex	= { 3, sizeof (char),   16, UNSIGNED, PADDR,  b_put, 0};
static struct dfmt	u_l_oct	= {11, sizeof (long),    8, UNSIGNED, PADDR,  l_put, 0};
static struct dfmt	u_l_dec	= {10, sizeof (long),   10, UNSIGNED, PADDR,  l_put, 0};
static struct dfmt	u_l_hex	= { 8, sizeof (long),   16, UNSIGNED, PADDR,  l_put, 0};
static struct dfmt	s_c_dec	= { 3, sizeof (char),   10,   SIGNED, PADDR,  b_put, 0};
static struct dfmt	s_s_dec	= { 6, sizeof (short),  10,   SIGNED, PADDR,  s_put, 0};
static struct dfmt	s_l_dec	= {11, sizeof (long),   10,   SIGNED, PADDR,  l_put, 0};
static struct dfmt	flt	= {14, sizeof (float),  10,   SIGNED, PADDR,  f_put, 0};
static struct dfmt	dble	= {21, sizeof (double), 10,   SIGNED, PADDR,  d_put, 0};
static struct dfmt	string	= { 0,               0,  8,        0,    NO, st_put, 0};


static char	dbuf[DBUF_SIZE];		/* input buffer */
static char	mbuf[DBUF_SIZE];		/* buffer used if 2-byte crosses line*/
static char	lastdbuf[DBUF_SIZE];
static int	addr_base	= 8;		/* default address base is OCTAL */
static int	base		= -1;
static int nbase		= -1;			/* number base for -N */
static offset_t addr		= 0LL;		/* current file offset */
static offset_t label		= -1LL;		/* current label; -1 is "off" */
static int	dbuf_size	= 16;		/* file bytes / display line */
static int	_parity		= NO;		/* show parity on ascii bytes */
static char	fmt[]	= "            %s";	/* 12 blanks */

/* we must save global values and reset them for each call to double byte
 * routines.  For example if you call -CCCCCC, you want print the same line
 * six times.
 */
static int first_on_line = NO;			/*1st elmnt on line? replaces straggle*/
static int nls_skip_save = 0;			/* # of bytes to skip over	*/
static int already_read = 0;			/* next buffer already read in.	*/
static int nls_skip = 0;
static int changed = 0;
static int Aflag = 0;
static int Jflag = 0;
static int tdflag = 0;
static char	*icvt(long long value, int radix, int issigned, int ndigits);
static char	*scvt();
static char	*underline();		/* underline because of parity    */
static offset_t	get_addr();

static int status = 0;			/* do return status */
static int bytes;			/* number of bytes read on line */
static int save_bytes;			/* used for multi-line MB chars */
static int max_bytes = 0;		/* max readable bytes if nflag used */
static int nflag = 0;			/* -N flag from command line */

# define max(a,b)               (a<b ? b : a)
# define min(a,b)               (a>b ? b : a)

#define READABLE_BYTES	(nflag?min(dbuf_size,max_bytes):dbuf_size)

main(argc, argv)
int	argc;
char	**argv;
{
	register char *p;
	register char *l;
	register same;
	struct dfmt	*d;
	struct dfmt	**cv = conv_vec;
	int	showall = NO;
	int	field, llen, nelm;
	int	max_llen = 0;
	char 	*ptr;
	char 	*addr_ptr;
	int	xbytes;
	int	just_started = 1;	/* have we just started processing? */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_OD,NL_CAT_LOCALE);
	errno = 0;

       /*  
	* Parse arguments and set up conversion vector
	*/

	argv++;
	argc--;

	if (argc > 0)
	{
		for (; **argv == '-'; argv++, argc--)
		{
			if ((argv[0][1]=='-') && (argv[0][2]==0)) {
                        	argc--, argv++; /* --, so do args */
				break;
				}

			p = *argv;
			while(*++p != '\0')
			{
				switch(*p)
				{
			       /*
				* Specify the input offset base 
				*/
				case 'A':
					Aflag = 1;
					if (*(++p) == '\0') {
						p = *(++argv);
						argc--;
					}
					switch(*p)
					{
					case 'd':
						addr_base = 10;
						break;
					case 'o':
						addr_base = 8;
						break;
				       /*
					* Do not display labels
					*/
					case 'n':
						Aflag = 2;
						break;
					case 'x':
						addr_base = 16;
						break;
					default:
						(void)fprintf(stderr, MSGSTR(AOPT, "-A option only accepts the following:  d, o, n, and x\n"));
						usage();
					}
					continue;
				case 'a':
					d = &ascii;
					break;
				case 'b':
					d = &byte;
					break;
				case 'C':
					d = &Cchar;
					break;
				case 'c':
					d = &cchar;
					break;
				case 'd':
					d = &u_s_dec;
					break;
				case 'D':
					d = &u_l_dec;
					break;
				case 'e':
				case 'F':
					d = &dble;
					break;
				case 'f':
					d = &flt;
					break;
				case 'h':
				case 'x':
					d = &u_s_hex;
					break;
				case 'H':
				case 'X':
					d = &u_l_hex;
					break;
				case 'i':
					d = &s_s_dec;
					break;
				case 'I':
				case 'l':
				case 'L':
					d = &s_l_dec;
					break;
			       /*
				* Specify number of bytes to interpret
				*/
				case 'N':
					nflag++;
					if (*(++p) == '\0') {
						p = *(++argv);
						argc--;
					}
					/* parse the string for type of number */
					if ((p[0] == '0') && ((p[1] == 'x') || (p[1] == 'X'))) {
						p += 2;
						nbase = 16;
					}
					else if ((p[0] == 'x') || (p[0] == 'X')) {
						p++;
						nbase = 16;
					} else if (p[0] == '0')
						nbase=8;
					else
						nbase=10;

					max_bytes = strtol(p, &ptr, nbase);
					if ((max_bytes <= 0) || errno || (p == ptr))   {
						(void)fprintf(stderr, MSGSTR(INVNUM, "Invalid number of bytes to interpret\n"));
						usage();
					}
					p = --ptr;
					continue;
				case 'o':
				case 'B':
					d = &u_s_oct;
					break;
				case 'O':
					d = &u_l_oct;
					break;
				case 'p':
					_parity = EVEN;
					continue;
				case 'P':
					_parity = ODD;
					continue;
			       /*
				* Specify number of bytes to skip
				*/
				case 'j':
					Jflag++;
					if (*(++p) == '\0') {
						p = *(++argv);
						argc--;
					}
					addr_ptr = p;
					p += strlen(p) - 1;
					continue;
				case 's':
					d = &s_s_dec;
					break;
				case 'S':
					d = &string;
					d->df_size = strtol(++p, &ptr, 10);
					if ((d->df_size <= 0) || errno || (p == ptr))  
						d->df_size = MIN_SLEN;
					p = --ptr;
					showall = YES;
					break;
			       /*
				* Parse type_string
				*/
				case 't':
					if (*++p == '\0')  {
						p = *(++argv);
						argc--;
					}
						
					for (; *p; p++) {
						switch(*p) {
							case 'a':
								d = &ascii;
								break;
							case 'c':
								d = &cchar;
								break;

						       /*
							*  Type specifier d may be followed
							*  by a letter or number indicating 
							*  the number
							*  of bytes to be transformed.  If
							*  invalid, use default.
							*/
							case 'd':
								tdflag = 1;
								d = &s_l_dec;
								if (isupper(*(p+1)) || isdigit(*(p+1))) {
									switch (*(++p)) {
									case '1':
									case 'C':
										d = &s_c_dec;
										break;
									case '2':
									case 'S':
										d = &s_s_dec;
										break;
									case '4':
									case 'I':
										break;
									case 'L':
										d = &s_l_dec;
										break;
									default:
										(void)fprintf(stderr, MSGSTR(DOPT, "d may only be followed with C, S, I, L, 1, 2, or 4\n"));
										status=1;
									}
								} 

								break;

						       /*
							*  Type specifier f may be followed
							*  by F, D, or L indicating that
							*  conversion should be applied to 
							*  an item of type float, double
							*  or long double.  OR a number
							*  indicating the number bytes.  If
							*  invalid, use default.
							*/
							case 'f':
								d = &dble;
								if (isupper(*(p+1)) || isdigit(*(p+1))) {
									switch (*(++p)) {
									case '4':
									case 'F':
										d = &flt;
										break;
									case '8':
									case 'D':
										break;
									case 'L':
										d = &dble;
										break;
									default:
										(void)fprintf(stderr, MSGSTR(FOPT, "f may only be followed with F, D, L, 4, or 8\n"));
										status=1;
										}
								} 

								break;

						       /*
							*  Type specifier o may be followed
							*  by a letter or number indicating the number
							*  of bytes to be transformed.  If
							*  invalid, use default.
							*/
							case 'o':
								d = &u_l_oct;
								if (isupper(*(p+1)) || isdigit(*(p+1))) {
									switch (*(++p)) {
									case '1':
									case 'C':
										d = &byte;
										break;
									case '2':
									case 'S':
										d = &u_s_oct;
										break;
									case '4':
									case 'I':
										break;
									case 'L':
										d = &u_l_oct;
										break;
									default:
										(void)fprintf(stderr, MSGSTR(OOPT, "o may only be followed with C, S, I, L, 1, 2, or 4\n"));
										status=1;
										}
								} 

								break;

						       /*
							*  Type specifier u may be followed
							*  by a letter or a number indicating the number
							*  of bytes to be transformed.  If
							*  invalid, use default.
							*/
							case 'u':
								d = &u_l_dec;
								if (isupper(*(p+1)) || isdigit(*(p+1))) {
									switch (*(++p)) {
									case '1':
									case 'C':
										d = &u_c_dec;
										break;
									case '2':
									case 'S':
										d = &u_s_dec;
										break;
									case '4':
									case 'I':
										break;
									case 'L':
										d = &u_l_dec;
										break;
									default:
										(void)fprintf(stderr, MSGSTR(UOPT, "u may only be followed with C, S, I, L, 1, 2, or 4\n"));
										status=1;
										}
								} 

								break;

						       /*
							*  Type specifier x may be followed
							*  by a letter or a number indicating the number
							*  of bytes to be transformed.  If
							*  invalid, use default.
							*/
							case 'x':
								d = &u_l_hex;
								if (isupper(*(p+1)) || isdigit(*(p+1))) {
									switch (*(++p)) {
									case '1':
									case 'C':
										d = &u_c_hex;
										break;
									case '2':
									case 'S':
										d = &u_s_hex;
										break;
									case '4':
									case 'I':
										break;
									case 'L':
										d = &u_l_hex;
										break;
									default:
										(void)fprintf(stderr, MSGSTR(XOPT, "x may only be followed with C, S, I, L, 1, 2, or 4\n"));
										status=1;
									}
								} 

								break;
							default:
								usage();
							}
							*(cv++) = d;
						}
						p--;
						continue;
				case 'w':
					if ((dbuf_size = strtol(++p, &ptr, 10)) > DBUF_SIZE )
						dbuf_size = DBUF_SIZE;
					if ((dbuf_size <= 0) || errno || (p == ptr))  
						dbuf_size = BIG_DBUF;
					p = --ptr;
					continue;
				case 'v':
					showall = YES;
					continue;
				case '?':
				default:
					usage();
				}
				*(cv++) = d;
			}
		}
	}
	

	/*
	 * if nothing spec'd, setup default conversion.
	 */
	if (cv == conv_vec)
		*(cv++) = &u_s_oct;

	*cv = (struct dfmt *)0;

	/*
	 * calculate display parameters
	 */
	for (cv = conv_vec; d = *cv; cv++)
	{
		nelm = (dbuf_size + d->df_size - 1) / d->df_size;
		llen = nelm * (d->df_field + 1);
		if (llen > max_llen)
			max_llen = llen;
	}

	/*
	 * setup df_fmt to point to uniform output fields.
	 */
	for (cv = conv_vec; d = *cv; cv++)
	{
		if (d->df_field)	/* only if external field is known */
		{
			nelm = (dbuf_size + d->df_size - 1) / d->df_size;
			field = max_llen / nelm;
			d->df_fmt = fmt + 12 - (field - d->df_field);
		}
	}

       /*
 	* Check whether input file specified.  If so, reopen 
 	* stdin as new file.  If Aflag or Jflag is specified,
	* a filename may start with at +.
 	*/
	if (argc > 0 && ((**argv != '+') || Aflag || Jflag))
	{
		while (!reopen(*argv, O_RDONLY, STDIN))
		{
			status = 1;
			if (argc == 1)
                        {
   				if (!just_started)	/* display remaining line, IF we */
					line(bytes);   	/* have processed something.     */
				perror(*argv);
				exit (1);
                        }
			perror(*argv);
			argv++;
			argc--;
			if ((**argv == '+') && !(Aflag || Jflag))
			{
				argv--;		/* backup */
				argc++;
				break;
			}
		}
		argv++;
		argc--;
	}

	just_started = 0;	/* be sure to output last line if subsequent file not found */

       /*
	*  Advance into the file the number of bytes
	*  specified via -S
	*/
	if (Jflag) {
		addr = get_addr(addr_ptr);
		offset(addr);
		addr = 0;
	}

       /*
	*  If 'Old Style' look for offset beginning with a 
	*  + or a digit
	*/
	else if ((**argv == '+' || isdigit(**argv)) && !(Aflag || Jflag))
	{
		addr = get_addr(*argv);
		offset(addr);
		argv++;
		argc--;

		if (**argv == '+' || isdigit(**argv))
		{
			label = get_addr(*argv);
			argv++;
			argc--;
		}
	}

       /* 
	*  Process either file or stdin.  Open new files and
	*  continue processing as necessary.
	*/
	while (argc >= 0)
	{
	       /*
	 	* main dump loop
	 	*/
		same = -1;
		while (already_read || (bytes = read(STDIN, (void *)dbuf, (size_t)READABLE_BYTES)) > 0)
		{
			if (already_read)
			{
				already_read = 0;
				(void)memcpy (dbuf,mbuf,dbuf_size);
			}

		       /*
			*  If more than one file is specified and
			*  the current file does not fill the buffer,
			*  open the next file and continue processing.
			*/
			/** if file is non-existant should output buffer...*/
			while ((bytes < dbuf_size) && (argc > 0)) 
			{
				while (!reopen(*argv, O_RDONLY, STDIN))
				{
					status  = 1;
					if (argc <= 1) 
					{
						line(bytes);
						perror(*argv);
						exit(1);
					}
					perror(*argv);
					argv++;
					argc--;
				}
				ptr = dbuf + bytes;
	        		if ((xbytes = read(STDIN, (void *)ptr, (size_t)max(READABLE_BYTES-bytes,0))) > 0) 
					bytes += xbytes;
				argv++;
				argc--;
			}

			if (same>=0 && bcmp(dbuf, lastdbuf, dbuf_size) == 0 && !showall)
			{
				if (same==0)
				{
					fputs("*\n",stdout);
					same = 1;
				}
			}
			else
			{
			       /*
				* If nflag is specified, only print
				* max_bytes bytes
				*/
				if (!nflag) 
					line(bytes);
	
				else {
					if (max_bytes > bytes) {
						line(bytes);
						max_bytes -= bytes;
					}
					else {
						int i;
						for (i=max_bytes;(i<DBUF_SIZE)&&(i<max_bytes+4);i++)
							dbuf[i]=0;
						line(max_bytes);
						addr += max_bytes;
						if (label >= 0)
							label += max_bytes;
						put_addr(addr, label, '\n');
						exit(status);
					}
				}
	
				same = 0;
				p = dbuf;
				l = lastdbuf;
				for (nelm = 0; nelm < dbuf_size; nelm++)
				{
					*l++ = *p;
					*p++ = '\0';
				}
			}
			addr += (already_read)? save_bytes : bytes;
			if (label >= 0)
				label += bytes;
	
		}

	       /*
		* If there are more files, open and
		* continue processing.
		*/
		if (argc == 0)
			break;
		else 
		{
			while (!reopen(*argv, O_RDONLY, STDIN))
			{
				perror(*argv);
				status = 1;
				argv++;
				argc--;
				if (argc == 0)
					exit(1);
			}
			argv++;
			argc--;
		}
	}

	/*
	 * Some conversions require "flushing".
	 */

	bytes = 0;
	for (cv = conv_vec; *cv; cv++)
	{
		if ((*cv)->df_paddr)
		{
			if (bytes++ == 0) {
			       /* 
				* If -An is specified, don't print labels
				*/
				if (Aflag == 2) 
					fputs("\n", stdout);
				else if (argc <= 0)
					put_addr(addr, label, '\n');
			}
		}
		else 
			(*((*cv)->df_put))(0, *cv);
	}

	return(status);
}




/*
 * NAME: put_addr
 *                                                                    
 * FUNCTION:  Print out the current file address.
 *                                                                    
 */  
static void
put_addr(offset_t	addrs,
	 offset_t	labl,
	 char 		chr)
{
	(void)fputs(icvt((long long)addrs, addr_base, UNSIGNED, 7), stdout);
	if (labl >= 0)
		(void)printf(" (%s)", icvt((long long)labl, addr_base, UNSIGNED, 7));
	putchar(chr);
	
}

/*
 * NAME: line
 *                                                                    
 * FUNCTION:  When line is called we have determined the line is different
 *		from the previous line.  We then print it out in all the
 *		formats called for in the command line.
 *                                                                    
 */  
static void
line(n)
int	n;
{
	register i,first;
	register struct dfmt *c;
	register struct dfmt **cv = conv_vec;
	int	newnls_skip = 0,

	nls_skip_save = nls_skip;
	first = YES;
	while (c = *cv++)
	{
		nls_skip = nls_skip_save;
		if (c->df_paddr)
		{
			if (first)
			{
			       /* 
				* If -An is specified, don't print labels
				*/
				if (Aflag == 2)
					(void)fputs("\t", stdout);
				else
					put_addr(addr, label, ' ');
				first = NO;
			}
			else
			{
				putchar('\t');
				if (label >= 0)
					fputs("\t  ", stdout);
			}
		}
		i = 0;
		changed = 0;
		first_on_line = YES;
		while (i < n) {
			i += (*(c->df_put))(dbuf+i, c);
			first_on_line = NO;
			}

		if (changed) {
			newnls_skip |= nls_skip;
		}

		if (c->df_paddr)
			putchar('\n');
	}
	nls_skip = newnls_skip;
}

/*
 * NAME: s_put
 *                                                                    
 * FUNCTION: Print out a signed short.
 *                                                                    
 * RETURN VALUE DESCRIPTION:  size of a short.
 *			    
 */  

static
s_put(n, d)
short	*n;
struct dfmt	*d;
{
	(void)printf(d->df_fmt, icvt((long long)*n, d->df_radix, d->df_signed, d->df_field));
	return(d->df_size);
}

/*
 * NAME: us_put
 *                                                                    
 * FUNCTION: Print out an unsigned short in the "d" base.  hex, oct, dec.
 *                                                                    
 * RETURN VALUE DESCRIPTION: size of a short
 *			    
 */  
static
us_put(n, d)
unsigned short	*n;
struct dfmt	*d;
{
	(void)printf(d->df_fmt, icvt((long long)*n, d->df_radix, d->df_signed, d->df_field));
	return(d->df_size);
}

/*
 * NAME: l_put
 *                                                                    
 * FUNCTION:  print out an long.  -D -H -X -O options
 *                                                                    
 * RETURN VALUE DESCRIPTION:  size of a long.
 *			    
 */  
static
l_put(n, d)
long	*n;
struct dfmt	*d;
{
	if (tdflag)
		(void)printf(d->df_fmt, icvt((long long)*n, d->df_radix, d->df_signed, d->df_field));
	else
		(void)printf(d->df_fmt, icvt((long long)(unsigned long)*n, d->df_radix, d->df_signed, d->df_field));
	return(d->df_size);
}

/*
 * NAME: d_put
 *                                                                    
 * FUNCTION: print out a double  -E -F option.
 *                                                                    
 * RETURN VALUE DESCRIPTION: size of a double
 *			    
 */  
static
d_put(f, d)
double	*f;
struct dfmt *d;
{
	char fbuf[24];
	struct l { long n[2]; };

#if	vax
	if ((((struct l *)f)->n[0] & 0xff00) == 0x8000)	/* Vax illegal f.p. */
		(void)sprintf(fbuf, "    %08x %08x",
			((struct l *)f)->n[0], ((struct l *)f)->n[1]);
	else
#endif

		(void)sprintf(fbuf, "%21.14e", *f);
	(void)printf(d->df_fmt, fbuf);
	return(d->df_size);
}

/*
 * NAME: f_put
 *                                                                    
 * FUNCTION: print out a floating point. in D format.
 *                                                                    
 * RETURN VALUE DESCRIPTION: size of a float.
 *			    
 */  
static
f_put(f, d)
float	*f;
struct dfmt *d;
{
	char fbuf[16];

#if	vax
	if ((*(long *)f & 0xff00) == 0x8000)	/* Vax illegal f.p. form */
		(void)sprintf(fbuf, "      %08x", *(long *)f);
	else
#endif
		(void)sprintf(fbuf, "%14.7e", *f);
	(void)printf(d->df_fmt, fbuf);
	return(d->df_size);
}


static char	asc_name[34][4] = {
/* 000 */	"nul",	"soh",	"stx",	"etx",	"eot",	"enq",	"ack",	"bel",
/* 010 */	" bs",	" ht",	" lf",	" vt",	" ff",	" cr",	" so",	" si",
/* 020 */	"dle",	"dc1",	"dc2",	"dc3",	"dc4",	"nak",	"syn",	"etb",
/* 030 */	"can",	" em",	"sub",	"esc",	" fs",	" gs",	" rs",	" us",
/* 040 */	" sp",	"del"
};

/*
 * NAME: a_put
 *                                                                    
 * FUNCTION:  print out value in ascii, using known values for unprintables.
 *                                                                    
 * RETURN VALUE:  1
 *			    
 */  
static
a_put(cc, d)
char	*cc;
struct dfmt *d;
{
	int c = *cc;
	register char s[4] = {' ', ' ', ' ', '\0'};
	register pbit = parity((int)c & 0377);

	c &= 0177;
	if (isgraph(c))
	{
		s[2] = c;
		if (pbit == _parity)
			(void)printf(d->df_fmt, underline(s));
		else
			(void)printf(d->df_fmt, s);
	}
	else
	{
		if (c == 0177)
			c = ' ' + 1;
		if (pbit == _parity)
			(void)printf(d->df_fmt, underline(asc_name[c]));
		else
			(void)printf(d->df_fmt, asc_name[c]);
	}
	return(1);
}

/*
 * NAME: parity
 *                                                                    
 * FUNCTION: return the parity of a given word.
 *                                                                    
 * RETURN VALUE:  	ODD - Odd parity
 *			EVEN - Even parity
 */  
static
parity(word)
int	word;
{
	register int p = 0;
	register int w = word;
	register int hw;

	if (w)
		do
		{
			p ^= 1;
			hw = (~(-w));	/* lint complains about undefined   */
		} while(w &= hw);	/* evaluation order for w &=(~(-w)) */
	return (p? ODD:EVEN);
}

/*
 * NAME: underline
 *                                                                    
 * FUNCTION: underline a given string.
 *                                                                    
 * RETURN VALUE:  return a pointer to the underlined string.
 *			    
 */  
static char *
underline(s)
char	*s;
{
	static char ulbuf[16];
	register char *u = ulbuf;

	while (*s)
	{
		if (*s != ' ')
		{
			*u++ = '_';
			*u++ = '\b';
		}
		*u++ = *s++;
	}
	*u = '\0';
	return(ulbuf);
}

/*
 * NAME: b_put
 *                                                                    
 * FUNCTION:  print out a byte in octal
 *                                                                    
 * RETURN VALUE:  1
 *			    
 */  
static int
b_put(b, d)
char	*b;
struct dfmt *d;
{
	(void)printf(d->df_fmt, icvt((long long)*b & 0377, d->df_radix, d->df_signed, d->df_field));
	return(1);
}

/*
 * NAME: C_put
 *                                                                    
 * FUNCTION: print out a NLS character in ascii escape sequences.
 *                                                                    
 * RETURN VALUE:  1
 *			    
 */  

static int 
C_put(cc, d)
char	*cc;
struct dfmt *d;
{
	return (do_C_put (cc,d,1));
}

/*
 * NAME: do_C_put
 *                                                                    
 * FUNCTION: print out a value in ascii or ascii escape sequences.
 *                                                                    
 * RETURN VALUE:  1
 *			    
 */  
static int 
do_C_put(cc, d,doesq)
unsigned char	*cc;
struct dfmt *d;
int doesq;
{
	register char	*s;
	register int	n;
	register int	c = *cc;
	register int	disp_len;	/* for defect #44443 */
	char buffer[8];
	char buffer2[32];
	int mbcnt;
	int mbufcnt = 0;
	wchar_t wc;
	int i;

	changed++;
	mbcnt = mblen(cc, MB_CUR_MAX);
	if ( (!nls_skip) && (mbcnt != 1 && mbcnt != 0) ) /* start of valid MB char */
	{
		char mbchar[5];
		int mbl = 1;

		nls_skip += (mbcnt > 0)? (mbcnt - 1) : 0;
		mbchar[0] = c;

loop:
		if (cc[mbl] == '\0')
		{
			if (!already_read)
			{
				save_bytes = bytes;
				if ((bytes = read(STDIN, (void *)mbuf, (size_t)READABLE_BYTES)) <= 0)
				{
				/* Only part of a double byte character found. */
					already_read = 0;
					mbchar[mbl] = 0;
					mbcnt = mbl;
				}
				else
				{
					already_read++;
					mbuf[bytes] = '\0';
					mbchar[mbl++] = mbuf[mbufcnt++];
				}
			}
			else {
				mbchar[mbl++] = mbuf[mbufcnt++];
				if (mbl > bytes) {
					mbchar[mbl] = '\0';
					mbcnt = nls_skip = mbl;
					goto next;
				}
			}
		}
		else
			mbchar[mbl] = cc[mbl++];

		mbchar[mbl] = NULL;
		mbcnt = mblen(mbchar, MB_CUR_MAX); 
		if(!nls_skip)
			nls_skip += (mbcnt > 0)? (mbcnt - 1) : 0;
		/*
		 *  we must handle the case if the
		 *  multi byte char is split on a line
		 *  boundry, mblen will return -1, but
		 *  we have to check next char to see if
		 *  it's part of multi byte char therefore
		 *  we go to loop.
		 */
		if (mbcnt == -1) {
			if (mbl < MB_CUR_MAX)
				goto loop;
			else {
			/* od should NEVER output an invalid mbchar,     *
			 * display it in /<num> format, see defect 45743 */
			/**** actually it should be printed in 3-byte ****/
			/**** octal,  so do the single byte thing     ****/
				mbcnt = 1;
				mbchar[mbcnt] = NULL; 
				nls_skip = 0;
				s = scvt(c, d);
				goto skip;
				}
			}
next:
		if (doesq)
		{
			bzero(buffer, sizeof(buffer));
			bzero(buffer2, sizeof(buffer2));
			strcpy(buffer2,"\\");
			strcat(buffer2,"<");
			for (i=0; i<mbcnt; i++) {
				sprintf(buffer, "%2.2x", mbchar[i]); 
				strcat(buffer2, buffer);
				}
			strcat(buffer2, ">");
			s = buffer2;
		}
		else
			s = mbchar;

	}
	else if (nls_skip) /* continuing MB char */
		{ nls_skip = (nls_skip > 0)? nls_skip - 1: 0;
		if (!doesq) 
			(void)fputs ("  **",stdout);
		else	
			if (doesq && (nls_skip>0) && !first_on_line)
				(void)fputs ("  ",stdout);
			else
				if (doesq && (nls_skip==0) && first_on_line)
					(void)fputs ("    ",stdout);
				else
					if (doesq && (nls_skip==1) && first_on_line)
						(void)fputs ("        ",stdout);
					else
						if (doesq && (nls_skip==2) && first_on_line)
							(void)fputs ("          ",stdout);

		return (1);
	}
	else /* single byte char */
	{
		s = scvt(c, d);
	}
skip:
	/* defect 44443: first check to see if mbswidth() fails		*/
	/* in some cases d->df_field is 3, and s is a 4-byte char	*/
	/* when this happens we need to re-run mbswidth() with the	*/
	/* actual length of this character				*/
	if ((disp_len = mbswidth(s, d->df_field)) < 0 )
		disp_len = mbswidth(s,mblen(s,MB_CUR_MAX));
	if (disp_len == -1)
		disp_len = 1;
	for (n = d->df_field - disp_len; n > 0; n--)
		putchar(' ');
	(void)printf(d->df_fmt, s);
	return(1);
}
/*
 * NAME: c_put
 *                                                                    
 * FUNCTION: display value as character.
 *                                                                    
 * RETURN VALUE: 1
 *			    
 */  
static
c_put(cc, d)
char	*cc;
struct dfmt *d;
{
	return (do_C_put(cc,d,0));
}

/*
 * NAME: scvt
 *                                                                    
 * FUNCTION:  convert the character to a representable string.
 *                                                                    
 * RETURN VALUE:  A pointer to the string.
 *			    
 */  
static char *scvt(c, d)
int	c;
struct dfmt	*d;
{
	static char s[2];

	switch(c)
	{
		case '\0':
			return("\\0");

		case '\a':
			return("\\a");

		case '\b':
			return("\\b");

		case '\f':
			return("\\f");

		case '\n':
			return("\\n");

		case '\r':
			return("\\r");

		case '\t':
			return("\\t");

		case '\v':
			return("\\v");

		default:
			if (isprint(c))
			{
				s[0] = c; /* Depends on "s" being STATIC initialized to zero */
				return(s);
			}
			return(icvt((long long)c, d->df_radix, d->df_signed, d->df_field));
	}
}

/*
 * Look for ASCII strings.
 * A string contains bytes > 037 && < 177 (or >007 && <<016 excluding 013),
 * and ends with a null.
 * The minimum length is given in the dfmt structure.
 */

#define CNULL		'\0'
#define S_EMPTY	0
#define S_FILL	1
#define	S_CONT	2
#define SBUFSIZE	1024

#undef	isstring
#define	isstring(c)	((((c) > 037) && ((c) < 0177)) || \
			(((c) != 013) && ((c) > 007) && ((c) <016)))


static char	str_buf[SBUFSIZE];
static int	str_mode = S_EMPTY;
static char	*str_ptr;
static long	str_addr;
static long	str_label;

/*
 * NAME: st_put
 *                                                                    
 * FUNCTION:  print a string if it is at least d->df_size bytes long.
 *                                                                    
 * RETURN VALUE:  1
 *			    
 */  
static 
st_put(cc, d)
char	*cc;
struct dfmt	*d;
{
	register int	c;

	if (cc == 0)
	{
		pr_sbuf(d, YES);
		return(1);
	}

	c = *(unsigned char *)cc;

	if (str_mode & S_FILL)
	{
		if (isstring(c))
			put_sbuf(c, d);
		else
		{
			*str_ptr = CNULL;
			if ( !c )
				pr_sbuf(d, YES);
			str_mode = S_EMPTY;
		}
	}
	else if (isstring(c))
	{
		str_mode = S_FILL;
		str_addr = addr + (cc - dbuf);	  /* ugly */
		if ((str_label = label) >= 0)
			str_label += (cc - dbuf); /*  ''  */
		str_ptr = str_buf;
		put_sbuf(c, d);
	}

	return(1);
}

static
put_sbuf(c, d)
int	c;
struct dfmt	*d;
{
	*str_ptr++ = c;
	if (str_ptr >= (str_buf + SBUFSIZE))
	{
		pr_sbuf(d, NO);
		str_ptr = str_buf;
		str_mode |= S_CONT;
	}
}

/*
 * NAME: pr_sbuf
 *                                                                    
 * FUNCTION: print out the string buffer.
 */  

static void
pr_sbuf(d, end)
struct dfmt	*d;
int	end;
{
	register char	*p = str_buf;

	if (str_mode == S_EMPTY
	    || (!(str_mode & S_CONT) && (str_ptr - str_buf) < d->df_size))
		return;

	if (!(str_mode & S_CONT))
	       /* 
		* If -An is specified, don't print labels
		*/
		if (Aflag == 2) 
			(void)fputs("\t", stdout);
		else
			put_addr(str_addr, str_label, ' ');

	while (p < str_ptr)
		(void)fputs(scvt(*p++, d), stdout);

	if (end)
		putchar('\n');
}

/*
 * integer to ascii conversion
 *
 * This code has been rearranged to produce optimized runtime code.
 */

#define MAXINTLENGTH	64
static char		_digit[] = "0123456789abcdef";
static char		_icv_buf[MAXINTLENGTH+1];
static long long	_mask = 0x7fffffffffffffffLL;

/*
 * NAME: icvt
 *                                                                    
 * FUNCTION: return from a given stream the first value.
 *                                                                    
 * RETURN VALUE:  the value is an ascii stream printed in the RADIX given.
 */  
static char *
icvt(long long	value,
     int	radix,
     int	issigned,
     int	ndigits)
{
	register long long 	val = value;
	register long 		rad = radix;
	register char		*b = &_icv_buf[MAXINTLENGTH];
	register char		*d = _digit;
	register long long	tmp1;
	register long long 	tmp2;
	long long		rem;
	long long		kludge;
	int			sign;

	if (val == 0)
	{
		*--b = '0';
		sign = 0;
		goto done; /*return(b);*/
	}

	if (issigned && (sign = (val < 0)))	/* signed conversion */
	{
		/*
		 * It is necessary to do the first divide
		 * before the absolute value, for the case -2^31
		 *
		 * This is actually what is being done...
		 * tmp1 = (int)(val % rad);
		 * val /= rad;
		 * val = -val
		 * *--b = d[-tmp1];
		 */
		tmp1 = val / rad;
		*--b = d[(tmp1 * rad) - val];
		val = -tmp1;
	}
	else				/* unsigned conversion */
	{
		sign = 0;
		if (val < 0)
		{	/* ALL THIS IS TO SIMULATE UNSIGNED LONG MOD & DIV */
			kludge = _mask - (rad - 1);
			val &= _mask;
			/*
			 * This is really what's being done...
			 * rem = (kludge % rad) + (val % rad);
			 * val = (kludge / rad) + (val / rad) + (rem / rad) + 1;
			 * *--b = d[rem % rad];
			 */
			tmp1 = kludge / rad;
			tmp2 = val / rad;
			rem = (kludge - (tmp1 * rad)) + (val - (tmp2 * rad));
			val = ++tmp1 + tmp2;
			tmp1 = rem / rad;
			val += tmp1;
			*--b = d[rem - (tmp1 * rad)];
		}
	}

	while (val)
	{
		/*
		 * This is really what's being done ...
		 * *--b = d[val % rad];
		 * val /= rad;
		 */
		tmp1 = val / rad;
		*--b = d[val - (tmp1 * rad)];
		val = tmp1;
	}

done:
	if (sign)
		*--b = '-';

	tmp1 = ndigits - (&_icv_buf[MAXINTLENGTH] - b);
	tmp2 = issigned? ' ':'0';
	while (tmp1 > 0)
	{
		*--b = tmp2;
		tmp1--;
	}

	return(b);
}

/*
 * NAME: get_addr
 *                                                                    
 * FUNCTION: return the address of the given string in the file.
 */  
static offset_t
get_addr(s)
register char *s;
{
	register char *p;
	register long long a;
	register int d;
	char *ptr;

	if (*s=='+')
		s++;

       /*
	*  Parse type_string to determine input base
	*/
	if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) {
		s += 2;
		base = 16;
	}
	else if ((s[0] == 'x') || (s[0] == 'X')) {
		s++;
		base = 16;
	} else if ((s[0] == '0') && (s[1] != '\0'))
		base = 8;
	else if (Jflag || strchr(s, '.')) 
		base = 10;
	else if (base < 0)
		base=8;

       /*
	*  Parse input base
	*/
	errno = 0;
	a = strtoll(s, &ptr, base);
	
	if ((a < 0) || errno || (s == ptr)) {
		(void)fprintf(stderr, MSGSTR(INVOFF, "Invalid offset\n"));
		usage();
	}

       /*
	*  If A flag is not specified, use input base parsed above as
	*  base for labels.  Otherwise use base specified via -A option.
	*/
	if (!Aflag)
		addr_base = base;

	s = ptr;

       /*
	*  Offset may be followed by a multiplier
	*/
	switch (*s) {
	case '.':
		s++;
		break;
	case 'b':
		a *= 512;
		break;
	case 'k':
	case 'B':
		a *= 1024;
		break;
	case 'm':
		a *= 1048576;
		break;
	}

	return(a);
}


/*
 * NAME: offset
 *                                                                    
 * FUNCTION:  seek to the appropriate starting place.
 *                                                                    
 */  

static void
offset(offset_t a)
{
	if (canseek(STDIN))
	{
		/*
		 * in case we're accessing a raw disk,
		 * we have to seek in multiples of a physical block.
		 */
		(void)llseek(STDIN, a & 0xfffffffffffffe00LL, SEEK_SET);
		a &= 0x1ffLL;
	}
	dumbseek(STDIN, a);
}

/*
 * NAME: dumbseek
 *                                                                    
 * FUNCTION:  just read from stdin until the appropriate spot is reached.
 */  

static void
dumbseek(int		fd,
	 offset_t	s_offset)
{
	char	buf[BUFSIZ];
	int	n;
	size_t	nr;

	while (s_offset > 0)
	{
		nr = s_offset > BUFSIZ ? BUFSIZ : s_offset;
		if ((n = read(fd, (void *)buf, (size_t)nr)) != nr)
		{
			fputs("EOF\n",stderr);
			exit(1);
		}
		s_offset -= n;
	}
}


/*
 * NAME: canseek
 *                                                                    
 * FUNCTION: check to see if we are reading from a pipe, file, or stdin.
 *                                                                    
 * RETURN VALUE:	1 if file 
 *			0 if pipe or stdin			    
 */  
static
canseek(int fd)
{
	struct stat statb;

	return( (fstat(fd, &statb)==0) &&
		(statb.st_nlink > 0) &&		/*!pipe*/
		(!isatty(fd)) );
}

static
usage()
{
	(void)fprintf(stderr, MSGSTR(USAGE, "usage: od [-abBcCdDefFhHiIlLoOpPsvxX] [-S [n]] [-w [n]] [file] [[+]offset[.|b|B]] [[+]label[.|b|B]]\n"));
	(void)fprintf(stderr, MSGSTR(USAGE1,"       od [-v] [-A address_base] [-N count] [-j skip] [-t type_string] [file...]\n"));
	exit(1);
}

/*
 *  NAME:
 *	reopen
 *
 *  FUNCTION:
 *	duplicate functionality of freopen() using file descriptors
 *
 *  RETURN:
 *	return 0 on error, 1 on success
 */
static int
reopen(char	*fname,
       mode_t	mode,
       int	target)
{
	int fd;

	errno = 0;
	if ((fd = open(fname, mode)) >= 0 &&
	    dup2(fd, STDIN) == STDIN && !close(fd))
		return 1;
	if (fd >= 0)
		close(fd);
	return 0;
}
