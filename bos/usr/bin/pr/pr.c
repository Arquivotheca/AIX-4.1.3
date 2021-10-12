static char sccsid[] = "@(#)16  1.33  src/bos/usr/bin/pr/pr.c, cmdfiles, bos41J, 9523A_all 5/31/95 22:15:07";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: pr
 *
 * ORIGINS: 3, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <sys/types.h>
#include <errno.h>
#include <nl_types.h>
#include <langinfo.h>
#include "pr_msg.h"

static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_PR,Num,Str)

#define ESC     '\033'
#define LENGTH  66
#define LINEW   72
#define NUMW    5
#define MARGIN  10
#define DEFTAB  8

#define wcNEWLINE       ('\n')
#define wcTAB           ('\t')
#define wcSPACE         (' ')
#define wcFF            ('\f')

#define _WEOF  ((wchar_t) WEOF)
#define FOREVER 1
#define NO_ACTION

/*
 *      PR command (print files in pages and columns, with headings)
 *      2+head+2+page[56]+5
 */

static FILE  *mustopen();
static char nulls[] = "";
typedef struct { FILE *f_f; char *f_name; wchar_t f_nextc; } FILS;
static wchar_t Etabc, Itabc, Nsepc;
static FILS *Files;
static int Multi = 0, Nfiles = 0, Error = 0, onintr(void);
static int mbcm; /* MB_CUR_MAX */

#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
typedef int ANY;
typedef unsigned UNS;

#define NFILES  10
static char obuf[BUFSIZ];
static int Ttyout;
#define istty(F)        isatty((int)fileno(F))
#define done()          /* no cleanup */
#define INTREXIT        _exit
#define HEAD    "%s %s %s %d\n\n\n", date, head, MSGSTR(PAGE, "Page"), Page
#define cerror(S)       fprintf(stderr, "pr: %s", S)
#define STDINNAME()     nulls
#define TTY     "/dev/tty", "r"
#define PROMPT()        putc('\7', stderr) /* BEL */
#define NOSFILE nulls
#define TABS(N,C)       if ((N = intopt(&arg[1], &C)) <= 0) N = DEFTAB
#define ETABS   (Inpos % Etabn)
#define NSEPC   wcTAB

static ANY 	*getspace();
static wchar_t get();

static long Lnumb = 0;
static FILE *Ttyin = stdin;
static int Dblspace = 1, Fpage = 1, Formfeed = 0,
        Length = LENGTH, Linew = 0, Offset = 0, Ncols = 1, Pause = 0, 
        Colw, Plength, Margin = MARGIN, Numw, Report = 1,
        Etabn = 0, Itabn = 0;
static wchar_t Sepc = 0;
static char *Head = NULL;
static wchar_t *Buffer = NULL, *Bufend;
typedef struct { wchar_t *c_ptr, *c_ptr0; long c_lno; } *COLP;
static COLP Colpts;

static int Page, Nspace, Inpos;
static wchar_t  __WC = '\0';

static int Outpos, Lcolpos, Pcolpos, Line, eiflag = 0;

#define EMPTY   14      /* length of " -- empty file" */
typedef struct err { struct err *e_nextp; char *e_mess; } ERR;
static ERR *Err = NULL, *Lasterr = (ERR *)&Err;

/*
 * NAME: fixtty
 *                                                                    
 * FUNCTION: set up a buffer for the tty
 */  

/* ARGSUSED */
static fixtty()
{
        setbuf(stdout, obuf);
        if (signal(SIGINT, SIG_IGN) != SIG_IGN) signal(SIGINT, (void (*)(int))onintr);
        Ttyout= istty(stdout);
        return;
}

/*
 * NAME: GETDATE
 *                                                                    
 * FUNCTION:  return date file was last modified
 */  
static char *
GETDATE(void)
{
        static char     *lc_time;
        static char     now[64] = "";
        static struct stat sbuf, nbuf;

		lc_time=setlocale(LC_TIME, NULL);
        if (Nfiles > 1 || Files->f_name == nulls) {
                if (now[0] == '\0') {
                        time(&nbuf.st_mtime);
			if (!strcmp(lc_time, "C") || !strcmp(lc_time, "POSIX"))
                        	strftime(now, 63, "%b %e %H:%M %Y", localtime(&nbuf.st_mtime));
			else
				strftime(now, 63, "%c", localtime(&nbuf.st_mtime));
                }
        } else {
                stat(Files->f_name, &sbuf);
		if (!strcmp(lc_time, "C") || !strcmp(lc_time, "POSIX"))
                	strftime(now, 63, "%b %e %H:%M %Y", localtime(&sbuf.st_mtime));
		else
			strftime(now, 63, "%c", localtime(&sbuf.st_mtime));
        }
        return (now);
}

/*
 * NAME: pr [options] [files]
 *                                                                    
 * FUNCTION: Writes a file to standard output.  If a file is not specified
 *      or a '-' is for the file name then standard input is read.
 *      OPTIONS:
 *      -a        Displays multi-column output
 *      -d        Double-spaces the output
 *      -e[c][n]  Expands tabs to n+1, 2*n+1, 3*n+1 etc. Default is 8
 *                If a character c is specified then that character becomes
 *                the input tab character.
 *      -f        Uses a form-feed character to advance to a new page.
 *                (Historical; see -F below.)
 *      -F        Uses a form-feed character to advance to a new page.
 *                (POSIX)
 *      -h header  Displays string as the page header instead of the file
 *      -i[c][n]  replaces white spaces with tabs at n+1, 2*n+1, 3*n+1 etc. 
 *                Default is 8.  If a character c is specified then that 
 *                character becomes the output tab  character.
 *      -l num    Sets the length of a page to num
 *      -m        combines and writes all files at the same time, with each
 *                in a separate column (overrides -num and -a flags).
 *      -n[c][n]  number of digits used for numbering lines c is added to
 *                the line.
 *      -o num    Indents each line by num spaces.
 *      -p        pauses before beginning each page.
 *                (Historical; not POSIX.)
 *      -r        suppresses diagnostic messages if the system can't open files
 *      -s[char]  separates columns by the single character char instead 
 *                of spaces
 *      -t        does not display header and footer
 *      -w num    sets the width of a line
 *      -num      produce num-column  output
 *      +num      begin the display with page num (default is 1)
 */  
main(argc, argv)
int argc;
char *argv[];
{
        FILS fstr[NFILES];
        int c, nb, mcolflag = 0, nfdone = 0;
	char *arg, *cp;

        (void) setlocale(LC_ALL,"");
        catd = catopen(MF_PR, NL_CAT_LOCALE);

        mbcm = MB_CUR_MAX;

        Nsepc = Itabc = Etabc = wcTAB;
        Files = fstr;
        fixtty();
        do {
                if (optind >= argc) break;
                arg = argv[optind];
                c = arg[0];
                if (c == '+') {
                        Fpage = atoix((char)c,&arg[1]);
                        optind++;
                } else if (c == '-') {
			if (isdigit(arg[1])) {
                        	mcolflag++;
                        	Ncols = atoix((char)c,&arg[1]);
                        	optind++;
	                } else if (arg[1] == 'e') {
	                        TABS(Etabn, Etabc);
	                        optind++;
				eiflag++;
	                } else if (arg[1] == 'i') {
                        	TABS(Itabn, Itabc);
                        	optind++;
				eiflag++;
                	} else if (arg[1] == 'n' || arg[1] == 'x') {
                        	++Lnumb;
                        	if ((Numw = intopt(&arg[1],&Nsepc)) <= 0)
                                	Numw = NUMW;
                        	optind++;
                	} else if (arg[1] == 's') {
                        	if (arg[2]) {
					if ((nb = mbtowc(&Sepc,&arg[2],mbcm)) <= 0) {
						perror("pr");
						exit(2);
					}
                        		if (arg[2+nb] != '\0') (void) usage();
				}
				else Sepc = wcTAB;
                        	optind++;
                	} else {
                        	c = getopt(argc,argv,"adFfh:l:mo:prtw:");
                        	switch(c) {
                                	case 'a':
                                        	if (Multi != 'm')
                                                	Multi = 'a';
                                        	break;
                                	case 'd':
                                        	Dblspace = 2;
                                        	break;
                                	case 'f':
                                	case 'F':
                                        	++Formfeed;
                                        	break;
                                	case 'h':
                                        	Head = optarg;
                                        	break;
                                	case 'l':
                                        	Length = atoix((char)c,optarg);
                                        	break;
                                	case 'm':
                                        	Multi = 'm';
                                        	break;
                                	case 'o':
                                        	Offset = atoix((char)c,optarg);
                                        	break;
                                	case 'p':
                                        	++Pause;
                                        	break;
                                	case 'r':
                                        	Report = 0;
                                        	break;
                                	case 't':
                                        	Margin = 0;
                                        	break;
                                	case 'w':
                                        	Linew = atoix((char)c,optarg);
                                        	break;
                                	case -1:
                                        	break;
                                	default :
                                        	(void) usage();
                        	} /* end of switch */
                	} /* end of else */
		} else break;
        } while (c != -1);
        if (mcolflag && Multi == 'm') (void) usage();
        if (Length == 0) Length = LENGTH;
        if (Length <= Margin) Margin = 0;
        Plength = Length - Margin/2;
        if (Multi == 'm') Ncols = argc - optind;
        switch (Ncols) {
        case 0:
                Ncols = 1;
        case 1:
                break;
	default:
       		if (Etabn == 0) /* respect explicit tab specification */
       			Etabn = DEFTAB;
        	if (Itabn == 0)
        		Itabn = DEFTAB;
        }
	if (Ncols == 1)
		Linew = INT_MAX;  /* allow extremely long lines for output */
	else if (Linew == 0)
		Linew = Sepc == 0 ? LINEW : 512;
        if (Lnumb) {
                int numw;

                if (Nsepc == wcTAB) {
                        if(Itabn == 0)
                                        numw = Numw + DEFTAB - (Numw % DEFTAB);
                        else
                                        numw = Numw + Itabn - (Numw % Itabn);
                }else {
                                numw = Numw + ((c = wcwidth(Nsepc)) > 0 ? c : 0);
                }
                Linew -= (Multi == 'm') ? numw : numw * Ncols;
        }
        if ((Colw = (Linew - Ncols + 1)/Ncols) < 1)
                die(MSGSTR(WDTHLARGE,"width too large"));
        if (Ncols != 1 && Multi == 0) {
                UNS buflen = ((UNS)(Plength/Dblspace + 1))*2*(Linew+1)*sizeof(wchar_t);
                Buffer = (wchar_t *)getspace(buflen);
                Bufend = &Buffer[buflen];
                Colpts = (COLP)getspace((UNS)((Ncols+1)*sizeof(*Colpts)));
        }
        if (Ttyout && (Pause || Formfeed) && !istty(stdin))
                Ttyin = fopen(TTY);
	for (optind; optind < argc; ++optind)
                if (Multi == 'm') {
                       	if (Nfiles >= NFILES - 1)
                               	die(MSGSTR(TOOMANYF, "too many files"));
			if (mustopen(argv[optind], &Files[Nfiles++]) == NULL)
				++nfdone;       /* suppress printing */
		} else {
			if (print(argv[optind]))
				fclose(Files->f_f);
			++nfdone;
		}
        if (!nfdone)    /* no files named, use stdin */
                print(NOSFILE); /* on GCOS, use current file, if any */
        errprint();     /* print accumulated error reports */
        exit(Error);
        /* NOTREACHED */
}

/*
 * NAME: intopt
 *                                                                    
 * FUNCTION: get num and char and get the ascii values
 */  
static intopt(char *argv, wchar_t *optp)
{
        char    c;
        int     num = -1;

        c = *argv;
        if (argv[1] != '\0')
		if (!isdigit(argv[1])) {
			if ((num = mbtowc(optp,&argv[1],mbcm)) <= 0) {
				perror("pr");
				exit(2);
			}
			argv += num + 1;
			num = (*argv != '\0') ? atoix(c,argv) : -1;
		} else num = atoix(c,&argv[1]);
        return (num);
}

/*
 * NAME: print
 *                                                                    
 * FUNCTION: print header for next page
 */  
static print(name) char *name;
{
        static int notfirst = 0;
        char *date = NULL, *head = NULL;
        short len;
        wchar_t wc;

        if (Multi != 'm' || Nfiles == 0)
                if (mustopen(name, &Files[Nfiles]) == NULL)
                        return(0);
        if (Buffer)
          ungetwc(Files->f_nextc, Files->f_f);
        if (Lnumb) Lnumb = 1;

        /* This is the main loop, putpage is where the buffer information
	   is printed out  */

        for (Page = 0; ; putpage()) {
           if (__WC == _WEOF) 
	      break;
	   /* Fill up the buffer */
           if (Buffer) 
	      nexbuf();
           Inpos = 0;
           if (get(0) == _WEOF) 
	      break;
           fflush(stdout);
           if (++Page >= Fpage) {
              if (Ttyout && (Pause || Formfeed && !notfirst++)) {
                 PROMPT(); /* prompt with bell and pause */
                 while ((wc = getwc(Ttyin)) != _WEOF && wc != '\n') ;
              }
              if (Margin == 0) 
	         continue;
              if (date == NULL)
                 date = GETDATE();
              if (head == NULL) head = Head != NULL ? Head :
                             Nfiles < 2 ? Files->f_name : nulls;
 
					  fputs("\n\n",stdout);
              Nspace = Offset;
              putspace();
			  printf(HEAD);
           }
        }
        __WC = '\0';
        return (1);
}


/*
 * NAME: putpage
 *                                                                    
 * FUNCTION: print out the current page
 */  
static putpage()
{
        int colno;

        for (Line = Margin/2; FOREVER; get(0)) {
           for (Nspace = Offset, colno = Outpos = 0; __WC != wcFF; ) {
              if (Lnumb && __WC != _WEOF
                      && ((colno == 0 && Multi == 'm') || Multi != 'm')) {
                 if (Page >= Fpage) {
                    putspace();
                    printf("%*ld%wc", Numw,
                       Buffer ? Colpts[colno].c_lno++ : Lnumb, Nsepc);
                 }
                 ++Lnumb;
              }
	      /* This loop will print out the characters to stdout. put
		 puts a character from the buffer to stdout and get gets
		 a character from the buffer */
              for (Lcolpos = 0, Pcolpos = 0;
                   __WC != wcNEWLINE && __WC != wcFF && __WC != _WEOF;) {
                 put(__WC);
                 get(colno);
              }
              if (__WC == _WEOF)
                 break;
              if(++colno == Ncols)
                 break;
              if (__WC == wcNEWLINE && get(colno) == _WEOF)
                 break;
              if (Sepc)
                 put(Sepc);
              else if ((Nspace += Colw - Lcolpos + 1) < 1)
                 Nspace = 1;
           }
           if (__WC == _WEOF) {
              if (Margin != 0)
                 break;
              if (colno != 0)
                 put(wcNEWLINE);
              return;
           }
           if (__WC == wcFF)
              break;
           put(wcNEWLINE);
           if (Dblspace == 2 && Line < Plength)
              put(wcNEWLINE);
           if (Line >= Plength)
              break;
        }
        if (Formfeed)
           put(wcFF);
        else
           while (Line < Length)
              put(wcNEWLINE);
}

/*
 * NAME: nexbuf
 *                                                                    
 * FUNCTION: build next line
 */  

static nexbuf()
{
        wchar_t *s = Buffer;
        COLP p = Colpts;
        int i, j, bline = 0;
	wchar_t wc;

        for ( ; ; ) {
           p->c_ptr0 = p->c_ptr = s;
           if (p == &Colpts[Ncols])
              return;
         
           (p++)->c_lno = Lnumb + bline;

	   /* This is the loop where the characters are read from  */
	   /* the file						   */

           for (j = (Length - Margin)/Dblspace; --j >= 0; ++bline)
              for (Inpos = 0; ; ) {
	         errno = 0;
                 wc = fgetwc(Files->f_f);

		 /* If the character is an EOF, is it the real EOF or was   */
		 /* an illegal character found (for the current locale).    */

                 if (wc == _WEOF) {
		
		    /* If there is an illegal character, get it out of the  */
		    /* file and put a #? in the buffer (if there is space   */
		    /* left in the column). 				    */

		    if (errno == EILSEQ) {
		       fgetc(Files->f_f);
		       if (Inpos < Colw - 1) {
		          *s++ = '#';
                          if (s >= Bufend)
                             die(MSGSTR(PBOVRFL,"page-buffer overflow"));
		          *s++ = '?';
                          if (s >= Bufend)
                             die(MSGSTR(PBOVRFL,"page-buffer overflow"));
		       }
		       Inpos += 2;
		       Error++;
		   }

		   /* Real EOF or error condition from fgetwc that can't   */
		   /* be fixed.  Setup everything for the end of a file    */
 
	           else {
                      for (*s = _WEOF; p <= &Colpts[Ncols]; ++p)
                         p->c_ptr0 = p->c_ptr = s;
                      balance(bline);
                      return;
		   }
                }
                else {

		   /* The character was "gotten" ok and processing should   */
		   /* proceed as normal.			            */


		   /* According to POSIX.2 Draft 11, if a character is not  */
		   /* printable, it is still put in the buffer, but it is   */
		   /* not used in the computation of the column width or    */
		   /* or page length. This will cause pr to look busted     */
		   /* when there are character's that are marked unprintable*/
		   /* in the locale, but are really printable because of    */
		   /* font set, etc. IE In the C locale, the characters     */
		   /* above 127 are marked as unprintable, but they will    */
		   /* show up when sent to stdout.			    */

                   if ((i = wcwidth(wc)) > 0)
                      Inpos += i;
                   if (Inpos <= Colw || wc == wcNEWLINE) {
                      *s = wc;
                      ++s;
                      if (s >= Bufend)
                         die(MSGSTR(PBOVRFL,"page-buffer overflow"));
                   }
                   if (wc == wcNEWLINE)
                      break;
               
                   switch (wc) {
                      case '\b': if (Inpos == 0)
                                 --s;
                      case ESC: if (Inpos > 0)
                                --Inpos;
                   }
                }
	     }
          }
}

/*
 * NAME: balance
 *                                                                    
 * FUNCTION: line balancing for last page
 */  

static balance(bline) 
{
        wchar_t *s = Buffer;
        COLP p = Colpts;
        int colno = 0, j, c, l, flag;

        c = bline % Ncols;
        l = (bline + Ncols - 1)/Ncols;
        bline = 0;
        do {
           for (j = 0; j < l; ++j)
              while (*s++ != '\n');
           (++p)->c_lno = Lnumb + (bline += l);
           p->c_ptr0 = p->c_ptr = s;
           if (++colno == c) 
	      --l;
        } while (colno < Ncols - 1);
}

/*
 * NAME: get
 *                                                                    
 * FUNCTION: build next column
 */  
static wchar_t get(colno)
{
        static int peekc = 0;
        COLP p;
        FILS *q;
        wchar_t wc;
        short len;

        if (peekc) { 
           peekc = 0;
           wc = Etabc;
        }
        else
          if (Buffer) {
             p = &Colpts[colno];
             if (p->c_ptr >= (p+1)->c_ptr0)
                wc = _WEOF;
             else
                wc = *p->c_ptr++;
          }
          else {
             wc = (q = &Files[Multi == 'a' ? 0 : colno])->f_nextc;
             if (wc == _WEOF) {
               for (q = &Files[Nfiles];
                  --q >= Files && q->f_nextc == _WEOF; ) ;
               if (q >= Files)
                  wc = wcNEWLINE;
             }
             else
                q->f_nextc = getwc(q->f_f);
          }

          if (Etabn != 0 && wc == Etabc) {
             ++Inpos;
             peekc = ETABS;
             wc = wcSPACE;
          }
          else if (wc != _WEOF) {
		 if (mbcm == 1)
		    len = isprint(wc) ? 1 : -1;
		 else
		    len = wcwidth(wc);
		 if (len > 0)
		    Inpos += len;
		 else {
		       switch (wc) {
			  case '\b':
			  case ESC:
				    if (Inpos > 0) --Inpos;
				       break;
			  case '\f':
				     if (Ncols == 1) break;
					wc = wcNEWLINE;
			  case '\n':
			  case '\r':
				     Inpos = 0;
		       }
		 }
	  }
          __WC = wc;
          return __WC;
}

/*
 * NAME: put
 *                                                                    
 * FUNCTION: put out char c into the buffer, but check for special characters
 *           first.
 */  

static put(wc)
wchar_t wc;
{
  int move, space;

  switch (wc) {
     case  ' ':
                if (Ncols < 2 || Lcolpos < Colw) {
                   ++Nspace;
                   ++Lcolpos;
                }
                return;
     case '\t':
                if (Itabn == 0) {
                   move = DEFTAB - (Lcolpos % DEFTAB);
                   break;
                }
                if (Lcolpos < Colw) {
                   move = Itabn - ((Lcolpos + Itabn) % Itabn);
                   move = (move < Colw-Lcolpos) ? move : Colw-Lcolpos;
                   Nspace += move;
                   Lcolpos += move;
                }
                return;
     case '\b':
                if (Lcolpos == 0)
                   return;
                if (Nspace > 0) { 
                   --Nspace;
                   --Lcolpos;
                   return;
                }
                if (Lcolpos > Pcolpos) {
                   --Lcolpos;
                   return;
                }
     case ESC:
                move = -1;
                break;
     case '\n':
                ++Line;
     case '\r':
     case '\f':
                Pcolpos = Lcolpos = Nspace = Outpos = 0;
     default:
		if (mbcm == 1)
		   move = isprint(wc) ? 1 : 0;
		else
		   move = wcwidth(wc);
		move = move > 0 ? move : 0;
   }
   if (Page < Fpage)
      return;
   if (Lcolpos > 0 || move > 0)
      Lcolpos += move;
   if (Nspace > 0)
      putspace();

   if ((Ncols<2 && (Outpos + move) < Linew) || Lcolpos <= Colw || wc == Sepc) {
      putwchar(wc);
      Outpos += move;
      Pcolpos = Lcolpos;
   } else if (move > 1){ /* for painful multi-byte alignment, D-37433 */
      space = Colw - Pcolpos;
      Outpos += space;
      Lcolpos = Pcolpos += space;
      for (space; space > 0; space--)
	  putchar(' ');
   }
}

/*
 * NAME: putspace
 *                                                                    
 * FUNCTION: put out white space (tab or space or specified char)
 */  

static putspace()
{
        int nc, flag;

        for ( ; Nspace > 0; Outpos += nc, Nspace -= nc) {
           nc = Itabn - Outpos % Itabn;
           flag = (Itabn > 0 && Nspace >= nc && eiflag);
           if (flag)
              putwchar(Itabc);
           else {
              nc = 1;
              putchar(' ');
           }
        }
}

/*
 * NAME: atoix
 *                                                                    
 * FUNCTION: convert character string of digits to an integer
 *
 **
 * Defect #166422:
 * Added check for negative numbers.
 * This prevents options that require numeric arguments
 * from using the -column option as an argument.
 **
 */  
static atoix(c,cp)
char c; char *cp;
{
        int     numval;

        numval = (int)strtol(cp,&cp,10);
        if (*cp == '\0')
		if ( numval <= INT_MAX ) {
			if (numval < 0) {
				(void) badnum(c);
				/* NOTREACHED */
			}
                	return(numval);
		} else {
			fprintf(MSGSTR(BADVAL,
			"pr: Value %d can not be greater than %d for -%c"),
			numval, INT_MAX, c);
			exit(1);
		}
			
        else (void) badnum(c);
}

/*
 * NAME: mustopen
 *                                                                    
 * FUNCTION: open a file
 */  
/* Defer message about failure to open file to prevent messing up
   alignment of page with tear perforations or form markers.
   Treat empty file as special case and report as diagnostic.
*/

static FILE *mustopen(s, f) char *s; FILS *f;
{
        char *empty, *cantopen;
        if (*s == '-' && s[1] == '\0' || *s == '\0') {
           f->f_name = STDINNAME();
           f->f_f = stdin;
        } 
	else if ((f->f_f = fopen(f->f_name = s, "r")) == NULL) {
            cantopen = MSGSTR(CANTOPEN,"Cannot open file %s.");
            s = (char *) getspace((UNS)(strlen(cantopen) + strlen(f->f_name)+1));
            sprintf(s, cantopen, f->f_name);
        }
        if (f->f_f != NULL) {
           if ((f->f_nextc = getwc(f->f_f)) != _WEOF || Multi == 'm')
              return (f->f_f);
           empty = MSGSTR(EMPTYF,"%s -- empty file"); 
           sprintf(s = (char *)getspace((UNS)(strlen(f->f_name) + 1 + strlen(empty))), empty, f->f_name); 
           fclose(f->f_f);
        }
        Error = 1;
        if (Report)
           if (Ttyout) { /* accumulate error reports */
              Lasterr = Lasterr->e_nextp = (ERR *)getspace((UNS)sizeof(ERR));
              Lasterr->e_nextp = NULL;
              Lasterr->e_mess = s;
           } 
	   else { /* ok to print error report now */
              cerror(s);
              putc('\n', stderr);
           }
           return ((FILE *)NULL);
}

/*
 * NAME: getspace
 *                                                                    
 * FUNCTION: get more space from memory
 */  
static ANY *getspace(n) UNS n;
{
        ANY *t;

        if ((t = (ANY *)malloc((size_t)n)) == NULL) die(MSGSTR(NOCORE,"out of space")); 
        return (t);
}

/*
 * NAME: die
 *                                                                    
 * FUNCTION: print error messages and exit
 */  
static die(s) char *s;
{
        ++Error;
        errprint();
        cerror(s);
        putwc('\n', stderr);
        exit(1);
}

/*
 * NAME: onintr
 *                                                                    
 * FUNCTION: on interrupt print error messages and exit
 */  
static onintr(void)
{
        ++Error;
        errprint();
        INTREXIT(1);
}

/*
 * NAME: errprint
 *                                                                    
 * FUNCTION:  print accumulated error reports 
 */  
static errprint() 
{
        fflush(stdout);
        for ( ; Err != NULL; Err = Err->e_nextp) {
            cerror(Err->e_mess);
            putc('\n', stderr);
        }
        done();
}

static badnum(c)
char c;
{
        fprintf(stderr,MSGSTR(BADNUM,"pr: bad number for option: %c\n"),c);
        (void) usage();
}

static usage()
{
        fprintf(stderr,MSGSTR(USAGE,
	"Usage: pr [+Page][[-Column [-a]] | -m][-dFrt]\n\
	\t  [-e[Character][Gap]][-h Header][-i[Character][Gap]][-l Lines]\n\
	\t  [-n[Character][Width]][-o Offset][-s[Character]][-w Width]\n\
	\t  [-x[Character][Width]][-fp][File ...]\n"));
        exit(1);
}
