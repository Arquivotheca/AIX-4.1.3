static char sccsid[] = "@(#)24	1.31  src/bos/usr/bin/sed/sed0.c, cmdedit, bos41J, 9508A 2/2/95 15:09:22";
/*
 * COMPONENT_NAME: (CMDEDIT) sed0.c
 *
 * FUNCTIONS: main, fcomp, comploop, compsub, rline, address, cmp, 
 * text, search, dechain, ycomp, comple, getre and growspace.
 *
 * ORIGINS: 3, 10, 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include "sed.h"
#include <errno.h>

static FILE    *fin;
static FILE    *fcode[12];
static struct addr *lastre;
static wchar_t    sseof;
static char    *reend;

static char  *rhsend;
static char  *rhsp;

static union reptr     *ptrend;
static int     eflag = 0;
static int	compflag = 0;
static size_t  nbra;

/* pattern start, end and size */
/* Used to hold regular expression pattern string before call to regcomp */
static char    *pattern;
static char	*pend;
static int	psize;

static int     gflag = 0;
static int     nlno;
static char    fname[12][40];
static int     nfiles;
static union reptr *rep;
static char    *cp;
static char	*sp;
static struct label ltab[LABSIZE];
static struct label    *lab;
static struct label    *labend;
static int     depth;
static int 	opt;
static union reptr     **cmpend[DEPTH];
char    *badp;
static char    bad;

nl_catd catd;

#define CCEOF   22

static struct label    *labtab = ltab;

/*
 *	Define default messages.
 */
char    CGMES[]    = "sed: Function %s cannot be parsed.\n";
static char    LTL[]   = "sed: The label %s is greater than eight characters.\n";
static char    AD0MES[]   = "sed: Cannot specify an address with the %s function.\n";
static char    AD1MES[]   = "sed: Function %s allows only one address.\n";
char    MALLOCMES[]    = "sed: Memory allocation failed.\n";

/*
 *	Function prototypes.
 */
static void fcomp(void);
static void dechain(void);
static int rline(char *);
static struct addr *address(void);
static struct label *search(struct label *);
static char *text(char *, char *);
static struct addr *comple(wchar_t);
static int compsub(char *);
static int cmp(char *, char *);
static wchar_t	*ycomp(void);
static int getre(wchar_t);
static char *growspace(char *, char **);
static char *editscript;
int eargc;
int exit_status = 0;

main(int argc, char **argv)
{
	int	i;
	char	*filename;

	(void) setlocale(LC_ALL,"");	/* required by NLS environment tests */

	catd = catopen(MF_SED, NL_CAT_LOCALE);

	nlno = 0;
	badp = &bad;
	aptr = abuf;
	lab = labtab + 1;       /* 0 reserved for end-pointer */
	rep = ptrspace;
	/* Dynamic memory allocation for buffer storage */
	sp = growspace((char *)0, &reend);
	growbuff(&lsize, &linebuf, &lbend, (char **)0);
	growbuff(&hsize, &holdsp, &hend, (char **)0);
	growbuff(&gsize, &genbuf, &gend, (char **)0);
	growbuff(&psize, &pattern, &pend, (char **)0);

	ptrend = &ptrspace[PTRSIZE];
	labend = &labtab[LABSIZE];

	rhsp = growspace((char *)0, &rhsend);

	lastre = 0;
	lnum = 0;
	pending = 0;
	depth = 0;
	spend = linebuf;
	hspend = holdsp;
	fcode[0] = stdout;
	nfiles = 1;
	
	if (argc <= 1)
		USAGE_MSG(1);

	while ((opt = getopt(argc, argv, "nf:e:g")) != EOF) {
		switch (opt) {

		case 'n':
			nflag++;
			continue;

		case 'f':

			filename = optarg; 
				
			if((fin = fopen(filename, "r")) == NULL) {
				(void)fprintf(stderr, MSGSTR(PATTFIL, \
				      "sed: Cannot open pattern file %s.\n"), \
				      filename); 
				USAGE_MSG(2);
			}

			fcomp();
			compflag++;
			(void) fclose(fin);
			continue;

		case 'e':
			eflag++;
			compflag++;
			editscript = optarg;
			if (*editscript == '\0') {
				(void)fprintf(stderr, MSGSTR(NOSCRIPT, \
				      "sed: No editing script was provided.\n"));
				USAGE_MSG(2);
			}
			fcomp();
			eflag = 0;
			continue;

		case 'g':
			gflag++;
			continue;

		default:
			(void) fprintf(stderr, MSGSTR(UNKFLG, "sed: %c is not a valid flag.\n"), opt);  /* MSG */
			USAGE_MSG(1);
		} /* End Switch */
	} /* End While */
	
	eargc = argc - optind;
	argv = argv + optind;


	/* If you get to this section then sed assumes the form */
	/* sed [-n] script <file>.				*/
	if(!compflag && rep == ptrspace) {
		editscript = *argv;
		eflag++;
		fcomp();
		eflag = 0;
		eargc--;
		argv++;
	}

	if(depth) {
		(void) fprintf(stderr, MSGSTR(LEFTBRC, "sed: There are too many '{'.\n"));  /* MSG */
		exit(2);
	}

	labtab->address = rep;

	dechain();

	if(eargc <= 0)
		execute((char *)NULL);
	else while(--eargc >= 0) {
		execute(*argv++);
	}

	/* Defect 44584 */
	for (i=0; i<12; i++)
	{
		if ((fclose(fcode[i]) == EOF) && (errno != EBADF)) {
			perror("sed");
			exit(2);
		}
	}
	exit(exit_status);
}

/*
 *	Read sed commands into reptr structure ptrspace.
 *	Ptrspace stores address data, type of command,
 *	regular expressions and any new/inserted text, as well
 *	as any flags that are necessary for the processing
 *	of these commands.
 */
static void fcomp(void)
{
	char   *tp;
	struct addr 	*op;

	union reptr     *pt, *pt1;
	int     i;
	struct label    *lpt;

	op = lastre;

	if (rline(linebuf) < 0)  
		return;
	if (*linebuf == '#') {
			/* if "#n" on first line, same effect as 
			   using -n flag from command line */
		if(linebuf[1] == 'n')
			nflag = 1;
	} else {
		cp = linebuf;
		goto comploop;
	}

	for(;;) {
		if(rline(linebuf) < 0)  
			break;
		if (*linebuf == '#')	/* skip comments anywhere! */
			continue;

		cp = linebuf;
comploop:
		while(*cp == ' ' || *cp == '\t')	/* skip white space */
			cp++;
		if(*cp == '\0')
			continue;
		if(*cp == ';') {
			cp++;
			goto comploop;
		}

		for (;;) {
			rep->r1.ad1 = address();
			if (errno != MORESPACE)
				break;
			sp = growspace(sp, &reend);
		}
		if(errno == BADCMD) {
			(void) fprintf(stderr, MSGSTR(CGMSG, CGMES), linebuf); /*MSG */
			exit(2);
		}

		if(errno == REEMPTY) {
			if(op) 
				rep->r1.ad1 = op;
			else {
				(void) fprintf(stderr, MSGSTR(FRSTRE, "sed: The first regular expression cannot be null.\n"));  /* MSG */
				exit(2);
			}
		} else if(errno == NOADDR) {
			rep->r1.ad1 = 0;
		} else {
			op = rep->r1.ad1;
			if(*cp == ',' || *cp == ';') {
				cp++;
				for (;;) {
					rep->r1.ad2 = address();
					if (errno != MORESPACE)
						break;
					sp = growspace(sp, &reend);
				}
				if(errno == BADCMD || errno == NOADDR) {
					(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exit(2);
				}
				if(errno == REEMPTY) 
					rep->r1.ad2 = op;
				else
					op = rep->r1.ad2;

			} else
				rep->r1.ad2 = 0;
		}

		while (sp >= reend) 
			sp = growspace(sp, &reend);

		while(*cp == ' ' || *cp == '\t')	/* skip wkite space */
			cp++;
swit:
		switch(*cp++) {

			default:
				(void) fprintf(stderr, MSGSTR( BADCMND, "sed: %s is not a recognized function.\n"), linebuf);  /* MSG */
				exit(2);

			case '!':
				rep->r1.negfl = 1;
				goto swit;

			case '{':
				rep->r1.command = BCOM;
				rep->r1.negfl = !(rep->r1.negfl);
				cmpend[depth++] = &rep->r2.lb1;
				if(++rep >= ptrend) {
					(void) fprintf(stderr, MSGSTR( TOOMANYCMDNS, "sed: There are more than 1000 commands in pattern file.\n"), linebuf);  /* MSG */
					exit(2);
				}
				if(*cp == '\0') continue;

				goto comploop;

			case '}':
				if(rep->r1.ad1) {
					(void) fprintf(stderr, MSGSTR(AD0MSG, AD0MES), linebuf);  /* MSG */
					exit(2);
				}

				if(--depth < 0) {
					(void) fprintf(stderr, MSGSTR( RGHTBRC, "sed: There are too many '}'.\n"));  /* MSG */
					exit(2);
				}
				*cmpend[depth] = rep;

				continue;

			case '=':
				rep->r1.command = EQCOM;
				if(rep->r1.ad2) {
					(void) fprintf(stderr, MSGSTR(AD1MSG, AD1MES), linebuf);  /* MSG */
					exit(2);
				}
				break;

			case ':':
				if(rep->r1.ad1) {
					(void) fprintf(stderr, MSGSTR( AD0MSG, AD0MES), linebuf);  /* MSG */
					exit(2);
				}

				while(*cp++ == ' ');
				cp--;


				tp = lab->asc;
				while((*tp++ = *cp++))
					if(tp > &(lab->asc[8])) {
						(void) fprintf(stderr, MSGSTR(LTLMSG, LTL), linebuf);  /* MSG */
						exit(2);
					}

				if(lpt = search(lab)) {
					if(lpt->address) {
						(void) fprintf(stderr, MSGSTR(DUPLBL, "sed: There are more than one %s labels.\n"), linebuf);  /* MSG */
						exit(2);
					}
				} else {
					lab->chain = 0;
					lpt = lab;
					if(++lab >= labend) {
						(void) fprintf(stderr, MSGSTR( LABELCNT, "sed: There are too many labels in file %s.\n"), linebuf);  /* MSG */
						exit(2);
					}
				}
				lpt->address = rep;

				continue;

			case 'a':
				rep->r1.command = ACOM;
				if(rep->r1.ad2) {
					(void) fprintf(stderr, MSGSTR( AD1MSG, AD1MES), linebuf);  /* MSG */
					exit(2);
				}
				if(*cp == '\\') cp++;
				if(*cp++ != '\n') {
					(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exit(2);
				}
				rep->r1.rhs = sp;
				while ((sp = text(rep->r1.rhs, reend)) == 0) {
					sp = growspace(rep->r1.rhs, &reend);
					rep->r1.rhs = sp;
				}
				break;
			case 'c':
				rep->r1.command = CCOM;
				if(*cp == '\\') cp++;
				if(*cp++ != ('\n')) {
					(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exit(2);
				}
				rep->r1.rhs = sp;
				while ((sp = text(rep->r1.rhs, reend)) == 0) {
					sp = growspace(rep->r1.rhs, &reend);
					rep->r1.rhs = sp;
				}
				break;
			case 'i':
				rep->r1.command = ICOM;
				if(rep->r1.ad2) {
					(void) fprintf(stderr, MSGSTR( AD1MSG, AD1MES), linebuf);  /* MSG */
					exit(2);
				}
				if(*cp == '\\') cp++;
				if(*cp++ != ('\n')) {
					(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exit(2);
				}
				rep->r1.rhs = sp;
				while ((sp = text(rep->r1.rhs, reend)) == 0) {
					sp = growspace(rep->r1.rhs, &reend);
					rep->r1.rhs = sp;
				}
				break;

			case 'g':
				rep->r1.command = GCOM;
				break;

			case 'G':
				rep->r1.command = CGCOM;
				break;

			case 'h':
				rep->r1.command = HCOM;
				break;

			case 'H':
				rep->r1.command = CHCOM;
				break;

			case 't':
				rep->r1.command = TCOM;
				goto jtcommon;

			case 'b':
				rep->r1.command = BCOM;
jtcommon:
				while(*cp == ' ')
					cp++;

				if(*cp == '\0') {
					if(pt = labtab->chain) {
						while(pt1 = pt->r2.lb1)
							pt = pt1;
						pt->r2.lb1 = rep;
					} else
						labtab->chain = rep;
					break;
				}
				tp = lab->asc;
				while((*tp++ = *cp++))
					if(tp > &(lab->asc[8])) {
						(void) fprintf(stderr, MSGSTR( LTLMSG, LTL), linebuf);  /* MSG */
						exit(2);
					}
				cp--;

				if(lpt = search(lab)) {
					if(lpt->address) {
						rep->r2.lb1 = lpt->address;
					} else {
						pt = lpt->chain;
						while(pt1 = pt->r2.lb1)
							pt = pt1;
						pt->r2.lb1 = rep;
					}
				} else {
					lab->chain = rep;
					lab->address = 0;
					if(++lab >= labend) {
						(void) fprintf(stderr, MSGSTR( LABELCNT, "sed: There are too many labels in file %s.\n"), linebuf);  /* MSG */
						exit(2);
					}
				}
				break;

			case 'n':
				rep->r1.command = NCOM;
				break;

			case 'N':
				rep->r1.command = CNCOM;
				break;

			case 'p':
				rep->r1.command = PCOM;
				break;

			case 'P':
				rep->r1.command = CPCOM;
				break;

			case 'r':
				rep->r1.command = RCOM;
				if(rep->r1.ad2) {
					(void) fprintf(stderr, MSGSTR( AD1MSG, AD1MES), linebuf);  /* MSG */
					exit(2);
				}
				if(*cp++ != ' ') {
					(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exit(2);
				}
				rep->r1.rhs = sp;
				while ((sp = text(rep->r1.rhs, reend)) == 0) {
					sp = growspace(rep->r1.rhs, &reend);
					rep->r1.rhs = sp;
				}
				break;

			case 'd':
				rep->r1.command = DCOM;
				break;

			case 'D':
				rep->r1.command = CDCOM;
				rep->r2.lb1 = ptrspace;
				break;

			case 'q':
				rep->r1.command = QCOM;
				if(rep->r1.ad2) {
					(void) fprintf(stderr, MSGSTR( AD1MSG, AD1MES), linebuf);  /* MSG */
					exit(2);
				}
				break;

			case 'l':
				rep->r1.command = LCOM;
				break;

			case 's':
				rep->r1.command = SCOM;
				cp += mbtowc(&sseof, cp, MB_CUR_MAX); 
				rep->r1.re1 = comple(sseof);
				if(errno == BADCMD) {
					(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exit(2);
				}
				if(errno == REEMPTY) 
					rep->r1.re1 = op;
				else 
					op = rep->r1.re1;

				rep->r1.rhs = rhsp;
				while (errno = compsub(rep->r1.rhs)) {
					if (errno == MORESPACE) {
						rhsp = growspace(rhsp, &rhsend);
						rep->r1.rhs = rhsp;
						continue;
					}
					(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exit(2);
				}
				if(gflag)
					rep->r1.gfl = GLOBAL_SUB;
				else
					rep->r1.gfl = 1;
				while (strspn(cp, "gpPw0123456789")) {
					if(*cp == 'g')
						rep->r1.gfl = GLOBAL_SUB;
					else if(*cp == 'p')
						rep->r1.pfl = 1;
					else if(*cp == 'P')
						rep->r1.pfl = 2;
					else if(*cp == 'w') {
						cp++;
						if(*cp++ !=  ' ') {
							(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
							exit(2);
						}
						if(nfiles > MAXWFILES) {
							(void) fprintf(stderr, MSGSTR( FILECNT, "sed: The w function allows a maximum of ten files.\n"));  /* MSG */
							exit(2);
						}

						(void)text(fname[nfiles], (char *)0);
						for(i = nfiles - 1; i >= 0; i--)
							if(cmp(fname[nfiles],fname[i]) == 0) {
								rep->r1.fcode = fcode[i];
								goto done;
							}
						if((rep->r1.fcode = fopen(fname[nfiles], "w")) == NULL) {
							(void) fprintf(stderr, MSGSTR( FILEOPEN, "cannot open %s\n"), fname[nfiles]);  /* MSG */
							exit(2);
						}
						fcode[nfiles++] = rep->r1.fcode;
						break;
					} else {
						rep->r1.gfl = (short) strtol(cp,&tp,10);
						if (rep->r1.gfl == 0) {
							(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
							exit(2);
						}
						cp = --tp;
					}
					cp++;
				}
				break;

			case 'w':
				rep->r1.command = WCOM;
				if(*cp++ != ' ') {
					(void) fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exit(2);
				}
				if(nfiles > MAXWFILES){
					(void) fprintf(stderr, MSGSTR( FILECNT, "sed: The w function allows a maximum of ten files.\n"));  /* MSG */
					exit(2);
				}

				(void) text(fname[nfiles], (char *)0);
				for(i = nfiles - 1; i >= 0; i--)
					if(cmp(fname[nfiles], fname[i]) == 0) {
						rep->r1.fcode = fcode[i];
						goto done;
					}

				if((rep->r1.fcode = fopen(fname[nfiles], "w")) == NULL) {
					(void) fprintf(stderr, MSGSTR(CREATERR, "sed: Cannot create file %s.\n"), fname[nfiles]);  /* MSG */
					exit(2);
				}
				fcode[nfiles++] = rep->r1.fcode;
				break;

			case 'x':
				rep->r1.command = XCOM;
				break;

			case 'y':
				rep->r1.command = YCOM;
				cp += mbtowc(&sseof, cp, MB_CUR_MAX);

				rep->r1.ytxt = ycomp();
				if(rep->r1.ytxt == 0) {
					(void) fprintf(stderr, 
						MSGSTR(CGMSG, CGMES), linebuf);
					exit(2);
				}
				break;

		}
done:
		if (rep->r1.command == SCOM && !rep->r1.re1)
			if (lastre)
				rep->r1.re1 = lastre;
			else {
				(void) fprintf(stderr, MSGSTR(FRSTRE, "sed: The first regular expression cannot be null.\n"));  /* MSG */
				exit(2);
			}
		if(++rep >= ptrend) {
			(void) fprintf(stderr, MSGSTR(CMNDCNT, "sed: There are too many commands for the %s function.\n"), linebuf);  /* MSG */
			exit(2);
		}

		if(*cp++ != '\0') {
			if(cp[-1] == ';')
				goto comploop;
			(void) fprintf(stderr, MSGSTR(CGMSG, CGMES), linebuf);  /* MSG */
			exit(2);
		}

	}
	rep->r1.command = 0;
	lastre = op;
}

/*
 *	Write replacement string for substitution command
 *	into rhsbufparm. Any '\\' characters are left in the
 *	string and parsed for in the replacement phase.
 */
static int    compsub(char *rhsbufparm)
{
	register char   *q;
	register char   *p;
	wchar_t	wc;
	int	len;

	p = rhsbufparm;
	q = cp;
	while (*q) {
		if (p > rhsend)
			return(MORESPACE);
		if(*q == '\\') {
			*p++ = *q++;
				/* check for illegal subexpression */
			if (*q > nbra + '0' && *q <= '9')
				return(0);
			if ((len = mblen(q, MB_CUR_MAX)) < 1)
				break;
		} else {
			if ((len = mbtowc(&wc, q, MB_CUR_MAX)) < 1)
				break;
			if(wc == sseof) {
				*p++ = '\0';
				if (p > rhsend)
					return(MORESPACE);
				cp = q + len;
				rhsp = p;	/* update the rhsbuf pointer */
				return(0);
			}
		}
		while (len--)
			*p++ = *q++;
	}
	return(BADCMD);
}

/*
 *	Read in a single command line.
 */
static int	rline(char *lbuf)
{
	register char   *p, *q;
	char		*p1;
	register        t;
	static char     *saveq;
	int	i, len;
	char 	str[MB_LEN_MAX];

	p = lbuf - 1;

	/* This will be true when called from the main loop after  */
	/* encountering the -e option and eflag is set to 1 and    */
	/* when called from fcomp() when eflag will be set to -1.  */
	if(eflag) {
		/* If eflag is > 0, then this is the first call    */
		/* from fcomp() from main() and q must be set.     */
		if(eflag > 0) {
			eflag = -1;
			q = editscript;
		/* If eflag = -1, then q is already set and we are */
		/* just getting the rest if there is any left.     */
		} else {
			if((q = saveq) == 0)
				return(-1);
		}

		saveq = 0;
		while (*q) {
			/* Don't test for '\n' after '\\' */
			if(*q == '\\') {
				*++p = *q++; 	/* Copy the '\\' */
				if(*q == '\0') 
					return(-1);
			} else if (*q == '\n') {
				*++p = '\0';
				saveq = ++q;
				return(1);
			}
			if ((len = mblen(q, MB_CUR_MAX)) < 1)
				return(-1);
			while(p+len >= lbend) {
				p1 = p;
				growbuff(&lsize, &linebuf, &lbend, &p1);
				p = p1;
			}
			while (len--)
				*++p = *q++;
		}
		*++p = '\0';
		return(1);
	}

	while((t = getc(fin)) != EOF) {
		/* Check that the command line is not exceeding the allocated */
		/* linebuff buffer. Posix 4.55.4 and 2.2.2.151 limits the     */
		/* length of the line to POSIX2_LINE_MAX. For functionality   */
		/* reason, we are allowing up to the allocate buffer of LBSIZE*/
		/* growbuff() can be used to dynamically increase the buffer  */
		if ((p+1)>=lbend) {
			fprintf(stderr, MSGSTR(CMDSLINEMAX,
			    "sed: Command line is too long.\n"));
			exit(2);
		}
		if (t == '\\') {
			*++p = t;
			t = getc(fin);
		} else if (t == '\n') {
			*++p = '\0';
			return(1);
		}
		len = 1;
		str[0] = t;
		while (t != EOF && mblen(str, MB_CUR_MAX) != len) {
			if ((p+len)>=lbend) {
				fprintf(stderr, MSGSTR(CMDSLINEMAX,
				    "sed: Command line is too long.\n"));
				exit(2);
			}
			if (++len > MB_CUR_MAX)
				return(-1);
			str[len-1] = t = getc(fin);
		}
		while(p+len >= lbend) {
			p1 = p;
			growbuff(&lsize, &linebuf, &lbend, &p1);
			p = p1;
		}
		for (i=0; t != EOF && i<len; i++)
			*++p = str[i];
	}
	return(-1);
}

/*
 *	Store an address into addr structure if one is present.
 *	If not, set errno flag :
 *		- BADCMD, error in command line.
 *		- REEMPTY, a regular expr. indentified but empty.
 *			i.e. substitute previous RE.
 *		- NOADDR, no address given.
 *		- MORESPACE, need a bigger buffer
 */
static struct addr *address(void)
{
	struct addr	*addrbuf;
	char   *rcp, *rsp, *saveCp = cp ;
	long    lno;

	errno = 0;
	rsp = sp;
	
	if(*cp == '$') {
		if ((addrbuf = (struct addr *)malloc(sizeof(struct addr))) == 0)
		{
			(void) fprintf(stderr, MSGSTR(MALLOCMSG, MALLOCMES));
			exit(2);
		}
		addrbuf->afl = STRA;
		addrbuf->ad.str = rsp;
		*rsp++ = CEND;
		*rsp++ = CCEOF;
		if (rsp >= reend) {
			errno = MORESPACE;
			free((void *)addrbuf);
			return(0);
		}
		cp++;
		sp = rsp;
		return(addrbuf);
	}

	if (*cp == '/' || *cp == '\\' ) {	/* address is RE */
		struct addr *retval;
		int mbtowc_ret;
		if ( *cp == '\\' )
			cp++;
		mbtowc_ret = mbtowc(&sseof, cp, MB_CUR_MAX); 
		cp += mbtowc_ret;
		if ( (retval = comple(sseof)) == (struct addr *)MORESPACE )
		{
			cp = saveCp;
			errno = MORESPACE;
			return(0);
		}
		else
			return( retval );
	}

	rcp = cp;
	lno = 0;

	while(*rcp >= '0' && *rcp <= '9')	/* address is line number */
		lno = lno*10 + *rcp++ - '0';

	if(rcp > cp) {
		if ((addrbuf = (struct addr *)malloc(sizeof(struct addr))) == 0) {
			(void) fprintf(stderr, MSGSTR(MALLOCMSG, MALLOCMES));
			exit(2);
		}
		addrbuf->afl = STRA;
		addrbuf->ad.str = rsp;
		*rsp++ = CLNUM;
		*rsp++ = nlno;
		tlno[nlno++] = lno;
		if(nlno >= NLINES) {
			(void) fprintf(stderr, MSGSTR( LINECNT, "sed: There are too many line numbers specified.\n"));  /* MSG */
			exit(2);
		}
		*rsp++ = CCEOF;
		if (rsp >= reend) {
			errno = MORESPACE;
			free((void *)addrbuf);
			return (0);
		}
		cp = rcp;
		sp = rsp;
		return(addrbuf);
	}
	errno = NOADDR;
	return(0);
}

static int cmp(char *a, char *b)
{
	register char   *ra, *rb;

	ra = a - 1;
	rb = b - 1;

	while(*++ra == *++rb)
		if(*ra == '\0') return(0);
	return(1);
}

/*
 *	Read text from linebuf(cp) into textbuf.
 *	Return null if textbuf exceeds endbuf.
 */
static char    *text(char *textbuf, char *endbuf)
{
	register char   *p, *q;
	int	len;

	p = textbuf;
	q = cp;
	for(;;) {
		if (endbuf && (p >= endbuf))
			return(0);
		if(*q == '\\') 
			q++;	/* Discard '\\' and read next character */
		if(*q == '\0') {
			*p = *q;
			cp = q;
			return(++p);
		}
		/* Copy multi-byte character to p */
		if ((len = mblen(q, MB_CUR_MAX)) < 1)
			(void) fprintf(stderr, MSGSTR(CGMSG, CGMES), linebuf);
		while (len--)
			*p++ = *q++;
	}
}

static struct label    *search(struct label *ptr)
{
	struct label    *rp;

	rp = labtab;
	while(rp < ptr) {
		if(cmp(rp->asc, ptr->asc) == 0)
			return(rp);
		rp++;
	}

	return(0);
}

static void dechain(void)
{
	struct label    *lptr;
	union reptr     *rptr, *trptr;

	for(lptr = labtab; lptr < lab; lptr++) {

		if(lptr->address == 0) {
			(void) fprintf(stderr, MSGSTR( UNDFNLBL, "sed: %s is not a defined label.\n"), lptr->asc);  /* MSG */
			exit(2);
		}

		if(lptr->chain) {
			rptr = lptr->chain;
			while(trptr = rptr->r2.lb1) {
				rptr->r2.lb1 = lptr->address;
				rptr = trptr;
			}
			rptr->r2.lb1 = lptr->address;
		}
	}
}

/*
 *	Parse a 'y' command i.e y/xyz/abc/
 *	where xyz are the characters to be matched and
 *	abc are their replacement characters. 
 *	N.B. these characters can be multi-byte or the string "\\n"
 *	Return a pointer to the buffer storing these characters.
 */
static wchar_t	*ycomp(void)
{
	wchar_t c1, c2;
	wchar_t *ybuf, *yp;
	char	*tsp, *ssp;
	int	len1, len2;
	size_t  count = 0;

	ssp = cp;
	tsp = cp;
	/* Determine no of characters to be matched */
	/* and then allocate space for their storage */
	/* Set tsp to point to the replacement characters */
	for (;;) {
		if (*tsp == '\0') 
			return (0);
		if ((len1 = mbtowc(&c1, tsp, MB_CUR_MAX)) > 0) {
			tsp += len1;
			if (c1 == sseof)
				break;
			count++;
		} else
			return (0);
	}

	/* Allocate space for the characters to be replaced and */
	/* their replacements. The buffer will be built up by storing */
	/* the characters to be replaced and their replacement one after */
	/* the other i.e. in pairs in the buffer. For the search and replace */
	/* stage every second char will be tested and when a match is found */
	/* it will be replaced by the next character in the search buffer */

	ybuf = (wchar_t *)malloc((count * 2 + 1)*sizeof(wchar_t));
	if (!ybuf) {
		(void) fprintf(stderr, MSGSTR(MALLOCMSG, MALLOCMES));/* MSG */
		exit(2);
	}

	yp = ybuf;
	while ((len1 = mbtowc(&c1,ssp,MB_CUR_MAX)) > 0) {
		len2 = mbtowc(&c2, tsp, MB_CUR_MAX);
		if (c1 == sseof)
			break;
		if (len2 < 1 || *tsp == '\0' || c2 == sseof)
			return (0);
		if (len1 == 1 && *ssp == '\\' && ssp[1] == 'n') {
			ssp++;
			(void) mbtowc(&c1,"\n", MB_CUR_MAX);
		}
		ssp += len1;
		*yp++ = c1;

		if(len2 == 1 && *tsp == '\\' && tsp[1] == 'n') {
			tsp++;
			(void) mbtowc(&c2,"\n", MB_CUR_MAX);
		}
		tsp += len2;
		*yp++ = c2;
	}
	if(c2 != sseof)
		return (0);
	cp = tsp + len2;
	yp = '\0';

	return (ybuf);
}

/*
 *	Compile the regular expression, returning the results
 *	in a struct addr structure and setting the appropriate
 *	error number if required.
 */
static struct addr	*comple(wchar_t reeof)
{
	struct addr	*res;
	regex_t	*reg;
	int	cflag;

	errno = 0;
	/* Read reg. expr. string into pattern */
	cflag = getre(reeof);
	if (!cflag) {
		if ((reg = (regex_t *)malloc(sizeof(regex_t))) == 0) {
			(void) fprintf(stderr, MSGSTR(MALLOCMSG, MALLOCMES));
			exit(2);
		}
		if (regcomp(reg, pattern, 0) == 0) {
			if ((res=(struct addr *)malloc(sizeof(struct addr)))==0)
			{
			    (void)fprintf(stderr, MSGSTR(MALLOCMSG, MALLOCMES));
			    exit(2);
			}
			res->afl = REGA;
			res->ad.re = reg;
			nbra = reg->re_nsub;
			return(res);
		} else {
			free((void *)reg);
			errno = BADCMD;
		}
	} else
		errno = cflag;
	return(0);
}

/*
 *	Read regular expression into pattern, replacing reeof
 *	with a null terminator. Maintains cp, the pointer
 *	into the linebuf string. 
 */
static int	getre(wchar_t reeof)
{
	register char	*p1;
	char *p2;
	wchar_t	wc;
	int	empty = 1, len;
	int	brackcnt = 0;

	p1 = pattern;
	for (;;) {
		if (*cp == '\0' || *cp == '\n')
			break;
		if ((len = mbtowc(&wc, cp, MB_CUR_MAX)) < 1)
			break;
		if (!brackcnt && wc == reeof) {
			cp += len;
			*p1 = '\0';
			return (empty ? REEMPTY : 0);
		}
		empty = 0;
		if (*cp == '\\' && !brackcnt) {
			*p1 = *cp++;
			if (*cp == 'n') {
				while (p1 >= pend) {
					p2 = p1;
					growbuff(&psize, &pattern, &pend, &p2);
					p1 = p2;
				}
				*p1++ = '\n';
				cp++;
				continue;
			}
			while (p1 >= pend) {
				p2 = p1;    /* need p2 because p1 is register */
				growbuff(&psize, &pattern, &pend, &p2);
				p1 = p2;
			}
			p1++;
			if ((len = mblen(cp, MB_CUR_MAX)) < 1)
				break;
		/* Special code to allow delimiter to occur within bracket
		   expression without a preceding backslash */
		} else if (*cp == '[') {
			if (!brackcnt) {
				if (((*(cp+1) == '^') && (*(cp+2) == ']')) || (*(cp+1) == ']'))
					brackcnt++;
				brackcnt++;
			} else {
				if ((*(cp+1) == '.') || (*(cp+1) == ':') || (*(cp+1) == '='))
					brackcnt++;
			}
		} else if (*cp == ']' && brackcnt)
			brackcnt--;
		while ((p1 + len) >= pend) {
			p2 = p1;	/* need p2 because p1 is register */
			growbuff(&psize, &pattern, &pend, &p2);
			p1 = p2;
		}
		while (len--)
			*p1++ = *cp++;
	}
	return (BADCMD);
}

static char *last = 0;

/*
 *	Dynamic memory allocation for buffer storage.
 */
static char *growspace(char *buf, char **endp)
{
	int amount;

	if (last && buf == last) { /* can do realloc */
		amount = (*endp - last) << 1;
		last = realloc(last, amount);
	} else {
		if (!buf || (amount = *endp - buf) < LBSIZE)
			amount = LBSIZE;
		last = malloc(amount);
	}
	if (!last) {
		(void) fprintf(stderr, MSGSTR(MALLOCMSG, MALLOCMES));
		exit(2);
	}
	*endp = last + amount;
	return last;
}
