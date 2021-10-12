static char sccsid[] = "@(#)51 1.7.1.6 src/bos/usr/ccs/bin/fpr/fpr.c, cmdprog, bos41J, 9521B_all 5/26/95 09:45:20";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, flush, gettext, init, nospace, savech
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  	fpr.c:  Converts ASA carriage-control characters (as from
 *	Fortran-program output) to their UNIX equivalents before
 *	writing to standard output. This version supports Kanji and
 *	NLS (which includes standard ASCII).
 *      The ASA control characters are:
 *      space	1 line before
 *      0	2 lines before
 *	1	New page before
 *	+	no advance before (overprint)
 *
 *      The rather ingenious method for support of both overprinting
 *      and the "+" control character is almost impossible to implement
 *      in a robust fashion for nls and kji; consequently this version
 *      assumes that the printer is capable of handling the appropriate
 *      backspaces; i.e. backspaces are retained in the text line. If
 *      the following line starts with "+", then the previous line is
 *      appended with as many backspaces as needed for moving the
 *      pointer to lines start, and the line is appended. Crude, but it
 *      works.
*/

#if defined(NLS) || defined(KJI)
#define NLSKJI

#include <NLchar.h>
#endif

#include <stdio.h>
/*
 * Added for "setlocale" line following main():
*/
#include <locale.h>
#include <nl_types.h>

#ifdef MSG
#include "fpr_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_FPR, n, s)
#else
#define MSGSTR(n,s) s
#endif

#define BLANK ' '
#define TAB '\t'
#define NUL '\000'
#define FF '\f'
#define BS '\b'
#define CR '\r'
#define VTAB '\013'
#define EOL '\n'

#define TRUE 1
#define FALSE 0

#define MAXCOL 170
#define TABSIZE 8

#ifndef NLSKJI
#define INITWIDTH 8

typedef
  struct column
    {
      int count;
      int width;
      char *str;
    }
  COLUMN;

COLUMN *line;
#endif

char cc;
char saved;
int length;
char *text;
int highcol;
int maxpos;
int maxcol;

extern char *malloc();
extern char *calloc();
extern char *realloc();

FILE	*input = stdin;
char	*cmd_name;


main(argc, argv)
int	argc;
char	*argv[];
{

    int errflg = 0;

    cmd_name = argv[0];
    setlocale(LC_ALL,"");
#ifdef MSG
    catd = catopen(MF_FPR, NL_CAT_LOCALE);
#endif
    if (argc == 1)
	work("");
    else
    {
        if (!strcmp(*++argv, "--"))
        {
            if (--argc == 1)
                work("");
            else
                argv++;
        }
	while (--argc > 0)
	{
	    if ((input = fopen(*argv, "r")) == NULL)
	    {
	        fprintf(stderr, MSGSTR(BADCC3,
	        "%s: cannot open input file %s\n"), cmd_name, *argv); /*MSG*/
	        errflg = 1;
	    }
            else
	        work(*argv);
            argv++;
	} /* while */
    } /* else */

    exit(errflg);
} /* main */

work(filename)
char	*filename;
{

  register int ch;
  register char ateof;
  register int i;
  register int errorcount;

  init();
  errorcount = 0;
  ateof = FALSE;

  ch = getc(input);
  if (ch == EOF)
    exit(0);

  if (ch == EOL)
    {
      cc = NUL;
      ungetc((int) EOL, input);
    }
  else if (ch == BLANK)
    cc = NUL;
  else if (ch == '1')
    cc = FF;
  else if (ch == '0')
    cc = EOL;
  else if (ch == '+')
    cc = NUL;
  else if (ch == '-')
    {
      putchar(EOL);
      cc = EOL;
    }
  else
    {
      errorcount = 1;
      cc = NUL;
    }

  while ( ! ateof)
    {
#ifdef NLSKJI
      gettext(length);
#else
      gettext();
#endif
      ch = getc(input);
      if (ch == EOF)
	{
	  flush();
	  ateof = TRUE;
	}
      else if (ch == EOL)
	{
	  flush();
	  cc = NUL;
	  ungetc((int) EOL, input);
	}
      else if (ch == BLANK)
	{
	  flush();
	  cc = NUL;
	}
      else if (ch == '1')
	{
	  flush();
	  cc = FF;
	}
      else if (ch == '0')
	{
	  flush();
	  cc = EOL;
	}
      else if (ch == '-')
	{
	  flush();
          putchar(EOL);
	  cc = EOL;
	}
      else if (ch == '+')
	{
#ifdef NLSKJI
	  if (highcol > maxpos)
	    {
	      maxpos = highcol + 10;
	      text = realloc(text, (unsigned) maxpos);
	      if (text == NULL)
		      nospace();
	    }
          for (i = 0; i < highcol; i++)
          /* GH 02/28/90 A17732 fpr/asa can't handle records 72-96 long*/
            {
               if (length >= maxpos)
                 {
                   maxpos = length + 10;
                   text = realloc(text, (unsigned) maxpos);
                   if ( text == NULL)
                      nospace();
                 }
               text[length++] = BS;
            }
	    highcol = 0;
#else
	  for (i = 0; i < length; i++)
	    savech(i);
#endif
	}
      else
	{
	  errorcount++;
	  flush();
	  cc = NUL;
	}
    }

  if (errorcount == 1)
    fprintf(stderr, MSGSTR(BADCC1, "%s: %s: Illegal carriage control - 1 line.\n"),
	cmd_name, filename); /*MSG*/
  else if (errorcount > 1)
    fprintf(stderr, MSGSTR(BADCC2, "%s: %s: Illegal carriage control - %d lines.\n"),
	cmd_name, filename, errorcount); /*MSG*/

} /* work */



init()
{
#ifndef NLSKJI
  register COLUMN *cp;
  register COLUMN *cend;
#endif
  register char *sp;


  length = 0;
  maxpos = MAXCOL;
  sp = malloc((unsigned) maxpos);
  if (sp == NULL)
    nospace();
  text = sp;
  highcol = -1;
#ifdef NLSKJI
  highcol = 0;
#else
  highcol = -1;
  maxcol = MAXCOL;
  line = (COLUMN *) calloc(maxcol, (unsigned) sizeof(COLUMN));
  if (line == NULL)
    nospace();
  cp = line;
  cend = line + (maxcol-1);
  while (cp <= cend)
    {
      cp->width = INITWIDTH;
      sp = calloc(INITWIDTH, (unsigned) sizeof(char));
      if (sp == NULL)
        nospace();
      cp->str = sp;
      cp++;
    }
#endif
}


#ifdef NLSKJI
gettext(ii)
int ii;
#else
gettext()
#endif
{
  register int i;
  register char ateol;
  register int ch;
  register int pos;

#ifdef NLSKJI
  i = ii;
#else
  i = 0;
#endif
  ateol = FALSE;

  while ( ! ateol)
    {
      ch = getc(input);
      if (ch == EOL || ch == EOF)
	ateol = TRUE;
      else if (ch == TAB)
	{
	  pos = (1 + i/TABSIZE) * TABSIZE;
	  if (pos > maxpos)
	    {
	      maxpos = pos + 10;
	      text = realloc(text, (unsigned) maxpos);
	      if (text == NULL)
		nospace();
	    }
	  while (i < pos)
	    {
	      text[i] = BLANK;
	      i++;
#ifdef NLSKJI
	      highcol++;
#endif
	
	    }
	}
      else if (ch == BS)
	{
	  if (i > 0)
	    {

#ifdef NLSKJI
	      if (i >= maxpos)
	         {
	           maxpos = i + 10;
	           text = realloc(text, (unsigned) maxpos);
	           if (text == NULL)
		      nospace();
	         }
	      highcol--;
	      text[i++] = ch;
#else
	      i--;
	      savech(i);
#endif
	    }
	}
      else if (ch == CR)
	{
	  while (highcol > 0)
	    {
#ifdef NLSKJI
	      if ((i + highcol) > maxpos)
		 {
		   maxpos = i + highcol +10;
	           text = realloc(text, (unsigned) maxpos);
	           if (text == NULL)
		      nospace();
	         }
	      highcol--;
	      text[i++] = BS;
#else
	      i--;
	      savech(i);
#endif
	    }
	}
      else if (ch == FF || ch == VTAB)
	{
	  flush();
	  cc = ch;
	  i = 0;
	}
      else
	{
	  if (i >= maxpos)
	    {
	      maxpos = i + 10;
	      text = realloc(text, (unsigned) maxpos);
	      if (text == NULL)
		nospace();
	    }
	  text[i] = ch;
	  i++;
#ifdef NLSKJI
 	  highcol++;
	  if (NCisshift(ch))
	    {
	      ch = getc(input);
	      if (i >= maxpos)
	        {
	          maxpos = i + 10;
	          text = realloc(text, (unsigned) maxpos);
	          if (text == NULL)
		     nospace();
	        }
	      text[i] = ch;
	      i++;
#ifdef KJI
	      highcol++;
#endif
	    }
#endif
	}
    }
  length = i;
}


#ifndef NLSKJI
savech(col)
int col;
{
  register char ch;
  register int oldmax;
  register COLUMN *cp;
  register COLUMN *cend;
  register char *sp;
  register int newcount;

  ch = text[col];
  if (ch == BLANK)
    return;

  saved = TRUE;

  if (col >= highcol)
    highcol = col;

  if (col >= maxcol)
    {
      oldmax = maxcol;
      maxcol = col + 10;
      line = (COLUMN *) realloc(line, (unsigned) maxcol*sizeof(COLUMN));
      if (line == NULL)
	nospace();
      cp = line + oldmax;
      cend = line + (maxcol - 1);
      while (cp <= cend)
	{
	  cp->width = INITWIDTH;
	  cp->count = 0;
	  sp = calloc(INITWIDTH, (unsigned) sizeof(char));
	  if (sp == NULL)
	    nospace();
	  cp->str = sp;
	  cp++;
	}
    }

  cp = line + col;
  newcount = cp->count + 1;
  if (newcount > cp->width)
    {
      cp->width = newcount;
      sp = realloc(cp->str, (unsigned) newcount*sizeof(char));
      if (sp == NULL)
	nospace();
      cp->str = sp;
    }
  cp->count = newcount;
  cp->str[newcount - 1] = ch;
}
#endif



flush()
{
  register int i;
  register int anchor;
  register int height;
  register int j;


  if (cc != NUL)
    putchar(cc);

  if ( ! saved)
    {
      i = length;
      while (i > 0 && text[i-1] == BLANK)
	i--;
      length == i;
      for (i = 0; i < length; i++)
	putchar(text[i]);
      putchar(EOL);
#ifdef NLSKJI
      length = 0;
      highcol = 0;
#endif
      return;
    }

#ifndef NLSKJI
  for (i =0; i < length; i++)
    savech(i);


  anchor = 0;
  while (anchor <= highcol)
    {
      height = line[anchor].count;
      if (height == 0)
	{
	  putchar(BLANK);
	  anchor++;
	}
      else if (height == 1)
	{
	  putchar( *(line[anchor].str) );
	  line[anchor].count = 0;
	  anchor++;
	}
      else
	{
	  i = anchor;
	  while (i < highcol && line[i+1].count > 1)
	    i++;
     	  for (j = anchor; j<= i; j++)
	    {
	      height = line[j].count - 1;
	      putchar(line[j].str[height]);
	      line[j].count = height;
     	    }
     	  for (j = anchor; j<= i; j++)
	    putchar(BS);		
    }
  }
#endif

  putchar(EOL);
  highcol = -1;
}



nospace()
{
  fprintf(stderr, MSGSTR(NOSPACE, "%s: Storage limit exceeded.\n"), cmd_name); /*MSG*/
  exit(1);
}
