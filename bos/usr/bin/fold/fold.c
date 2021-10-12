static char sccsid[] = "@(#)21  1.21  src/bos/usr/bin/fold/fold.c, cmdfiles, bos412, 9446C 11/14/94 16:48:34";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: fold 
 *
 * ORIGINS: 3, 18, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 *  $RCSfile: fold.c,v $ $Revision: 2.5 $ (OSF) $Date: 90/10/07 16:33:32 $
 */


#define _ILS_MACROS
#include "fold_msg.h" 
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * NAME:  fold [-bs] [-w Width] [File ...]
 * FUNCTION:  fold (wrap) long lines for limited-width output devices.
 *
 * Note: This code is compliant with POSIX 1003.2
 *
 */

#define MSGSTR(num,str) catgets(catd,MS_FOLD,num,str)

#define STREQ(s1,s2) (!strcmp(s1,s2))
#define SPECIAL -1

#define isblank(c)   is_wctype(c, _ISBLANK)
#define iswblank(c)  is_wctype(c, _ISBLANK)

typedef void (*func_ptr)();           /* func_ptr: pointer to a function.     */

static int      width = 80;             /* Width of output line (cols or bytes). */
static int      col, bytes;             /* Columns, bytes.                       */
static wint_t   wc;                     /* Current wide char (could be WEOF).    */
static int      ch;                     /* Current char (could be EOF).          */
static char     *cb;                    /* Char buffer pointer.                  */
static wchar_t  buffer[LINE_MAX+1];     /* Static buffer for char (or wchar_t).  */
static wchar_t  *wb;                    /* Wide char buffer pointer.             */
static size_t   mbcs;                   /* Multibyte code size (max bytes/char). */
static int      mbflag;                 /* Multibyte? Yes or no.                 */
static int      longline;               /* Found input line > LINE_MAX chars?    */
static nl_catd  catd;                   /* Message catalog descriptor.           */
static int      bflag;                  /* -b option?                            */
static int      sflag;                  /* -s option?                            */
static int      err;                    /* Retain knowledge of prior error (for  */
                                 /*   exit under unusual circumstances).  */

static void command_line(int argc, char *argv[]);
static void ps_none();                  /* Process single-byte, no options.      */
static void ps_b();                     /* Process single-byte, -b option.       */
static void ps_s();                     /* Process single-byte, -s option.       */
static void ps_bs();                    /* Process single-byte, -b & -s options. */
static void pm_none();                  /* Process multibyte, no options.        */
static void pm_b();                     /* Process multibyte, -b option.         */
static void pm_s();                     /* Process multibyte, -s option.         */
static void pm_bs();                    /* Process multibyte, -b & -s options.   */

/* "proc" is an array of pointers to file-processing functions.           */

static func_ptr proc[8] = {ps_none,
                    ps_b,
                    ps_s,
                    ps_bs,
                    pm_none,
                    pm_b,
                    pm_s,
                    pm_bs};
static func_ptr process;                /* Holds the pointer to the appropriate  */
                                 /*   function for processing files.      */

/*****************  
 * Main program. * 
 *****************/
 
main(int argc, char *argv[])
{
  int fn;                        /* File name (index into argv).          */
  int indexa;                     /* Index into proc[] array.              */

  /* Set the locale and open the appropriate message catalog.             */

  (void) setlocale(LC_ALL,"");
  catd = catopen(MF_FOLD, NL_CAT_LOCALE);

  /* Get the value of the current maximum byte length of a multibyte     */
  /*   character. If it is greater than 1, the program should follow the */
  /*   multibyte path. If the maximum display width is greater than 1,   */
  /*   the multibyte path will be followed regardless of the value of    */
  /*   MB_CUR_MAX.                                                       */

  mbcs = MB_CUR_MAX;
  mbflag = (mbcs > 1);
  err = 0;                       /* No errors yet.                       */

  command_line(argc,argv);       /* Parse the command line arguments.    */

  indexa = 0;                     /* The proper file-processing function  */
  if (mbflag)                    /*   is chosen based on three criteria: */
    indexa += 4;                  /*   whether the input is single-byte   */
  if (sflag)                     /*   or multibyte; the use of the -b    */
    indexa += 2;                  /*   option; and the use of the -s      */
  if (bflag)                     /*   option. An offset into the proc[]  */
    indexa += 1;                  /*   array is calculated and put into   */
  process = proc[indexa];         /*   "index."                           */

  if(argv[optind]==NULL)         /* No input files specified?            */
    (void) (*process)();         /* Then process stdin.                  */
  else
    for (fn=optind; fn < argc; fn++)    /* Otherwise, take each file one */
      if(!freopen(argv[fn],"r",stdin))  /*   at a time and associate it  */
      {                                 /*   with stdin.                 */
        fprintf(stderr,MSGSTR(BADFIL,"Cannot open file %s\n"),argv[fn]);
        err = 2;                 /* Error: Don't abort, just report it.  */
      }
      else
        (void) (*process)();     /* Opened OK; process the file.         */

  exit(err);                     /* Did any file fail to open? Exit with */
                                 /*   a non-zero return code.            */
}

/* command_line -- Handle all the command line options and arguments.    */

static void 
command_line(int argc, char *argv[])
{
  int arg;                       /* Return value from getopt().          */

  /* Handle all the command-line arguments using getopt(). The error     */
  /*   messages from getopt() are not disabled although the program      */
  /*   issues its own messages.                                          */

  while ((arg = getopt(argc,argv,"bsw:")) != SPECIAL)
  {
    switch(arg)
    { 
      case 'b': bflag = 1;       /* Set the -b flag.                     */    
                break;           
      case 's': sflag = 1;       /* Set the -s flag.                     */
                break;
      case 'w': width = (int)strtoul(optarg, &optarg, 10);
		if (*optarg != '\0') /* Width; valid number?             */
                {
                  fprintf(stderr,MSGSTR(ILLNUM,"Invalid number.\n"));
                  usage();       /* Bad command-line syntax.             */
                }
                break;
      default :
                usage();         /* Bad command-line syntax.             */
    }
  }
 
  if (!bflag && (width < 0 || width > LINE_MAX))
  {
    fprintf(stderr, MSGSTR(ILLNUM, "Invalid number.\n"));
    usage();
  }
  /* The hyphen (-) is not allowed as a way of indicating standard input. */

  if(argv[optind] && STREQ(argv[optind],"-"))
  {
    fprintf(stderr,MSGSTR(NOHYPH,"Cannot use '-' here.\n"));
    usage();                     /* Bad command-line syntax.             */
  }
}
 
/* ps_none() -- Process single-byte data, no command-line options.       */
 
static void 
ps_none()
{
  col = 1;                       /* Loop until eof forces a break.       */
  while ((ch = fgetc(stdin)) != EOF)
  {
    if (ch == '\t')
      col = (col + 8) & ~7;
    if (col > width && ch != '\b' && ch != '\n' && ch != '\r')
    {
      putc('\n',stdout);         /* Then print a newline and start over. */
      col = ch == '\t' ? 8 : 1;
    }
    putc(ch,stdout);             /* Output the character.                */
    switch (ch)                  /* No eof yet; adjust column-count      */
    {                            /*   based on the last character.       */
      case '\b': if(col > 1)     /* Backspace: Decrement column-count.   */
                   --col;
                 break;
      case '\n':                 /* Newline: Reset the column count.     */
      case '\r': col=1;          /* Carriage return: col. to beginning   */
                 break;
      default  : ++col;
    }
  }
}

/* ps_b() -- Process single-byte data with the -b option.                */
 
static void 
ps_b()
{
  bytes = 1;
  while ((ch = fgetc(stdin)) != EOF)
  {
    if (bytes > width && ch != '\n') /* Width about to be exceeded?      */
    {
      putc('\n',stdout);         /* Then print a newline and start over. */
      bytes = 1;
    }
    putc(ch,stdout);             /* Output the character.                */
    bytes = ch == '\n' ? 1 : bytes + 1;
  }
}
 
/* ps_s() -- Process single-byte data with the -s option.                */
 
static void 
ps_s()
{
  int  nc;                       /* Number of characters read so far.    */
  int  sc;                       /* Index of last space character.       */
  int  sl;                       /* String length of input line (bytes). */
  int  pos;                      /* Position at which to break line.     */
  int  overwidth;
  char save;                     /* Temporary variable.                  */

  cb = (char *) buffer;          /* Set cb to the start of the buffer.   */
  *cb = (char)NULL;              /* Ensure that we have a null string.   */

  for(;;)                        /* Loop until line is broken.           */
  {
    if(!*cb)                     /* Empty buffer? Then try to get a line */
    {
      cb = (char *) buffer;      /* Set cb to the start of the buffer.   */
      if(!fgets(cb, LINE_MAX, stdin)) /*   of input from stdin.          */
          break;                 /* EOF: Break and return to main().     */
      else
      {
        sl = strlen(cb);          /* Find the length of the string and   */
        longline = cb[sl-1]!='\n';/*   check whether the last byte       */
        if(!longline)             /*   is a newline; if so, change it to */
          cb[sl-1] = (char)NULL;  /*   a terminating NULL character.     */
      }
    }

    sl = strlen(cb);
    overwidth = FALSE;
    /* Loop until the column count reaches the specified width or the    */
    /*   end of the input line is reached.                               */
    for(col=1, nc=sc=0; col<=width && nc<sl; nc++)
    {
      switch(cb[nc])             /* Adjust column based on last char.    */
      {
        case '\b': if(col>1)     /* Backspace: Decrement counter by 1,   */ 
                     --col;      /*   but don't let it become negative.  */ 
                   break;
        case '\r': col = 1;      /* Carriage return: column to beginning */
                   break;
        case '\t': col = (col + 8) & ~7; /* Tab: Adjust to next tab */
                   if (col > width && width > 8)
                     overwidth = TRUE;
        default  : ++col;        /* Other: Just increment the counter.   */
      }
      if(isblank(cb[nc]))
        sc = nc;
    }

    if(col <= width)             /* Width not yet reached?    */
    {
      puts(cb);                  /* Then print the entire line.          */
      *cb = (char)NULL;          /* Empty the buffer.                    */
    }
    else                         /* Line is longer than "width".         */
    {                            /* Save the character at the break      */
      pos = isblank(cb[sc]) ? overwidth ? sc : sc + 1 : nc;
      save = cb[pos];            /*   position, stash a terminating NULL */
      cb[pos] = (char)NULL;      /*   there, and print out the string;   */
      puts(cb);                  /*   after that, move the entire tail   */
      cb[pos] = save;            /*   of the buffer to the head. We are  */
      cb += pos;                 /*   then ready for the next iteration. */
    }
  }
}
  
/* ps_bs() -- Process single-byte data with the -b and -s options. */
 
static void 
ps_bs()
{
  char save;                     /* Temporary variable.                  */
  int pos, sc = 0;

  if (!(cb = malloc(width + 1))) /* Set cb to the start of the buffer.   */
  {
    perror("malloc");
    exit(1);
  }
  bytes = 0;
  while ((ch = fgetc(stdin)) != EOF)
  {
    cb[bytes] = ch;
    if (bytes >= width && cb[bytes] != '\n')
    {
      pos = isblank(cb[sc]) ? sc + 1 : bytes;
      save = cb[pos];
      cb[pos] = (char) NULL;
      puts(cb);
      cb[pos] = save;
      strcpy(cb, cb + pos);
      bytes = strlen(cb) - 1;
    }
    if (isblank(cb[bytes]))
      sc = bytes;
    if (cb[bytes] == '\n')
    {
      cb[bytes] = NULL;
      puts(cb);
      bytes = 0;
    }
    else
      bytes++;
  }
}
   
/* pm_none() -- Process multibyte data with no command-line options. */
 
static void 
pm_none()
{
  col = 1;                       /* Loop until eof.                      */
  while ((wc = fgetwc(stdin)) != EOF)
  {                                                                        
    if (wc == L'\t')
      col = (col + 8) & ~7;
    if (col > width && wc != L'\b' && wc != L'\n' && wc != L'\r')
    {
      putwc(L'\n',stdout); /* Output a newline and reset count.*/
      col = ch == L'\t' ? 8 : 1;
    }
    putwc(wc,stdout);            /* Output the current character.        */
    switch (wc)                                                            
    {
      case L'\b': if(col > 1)    /* Decrement columns, but don't let it  */
                    --col;       /*   become negative.                   */
                  break;
      case L'\n':                /* Reset column count.                  */ 
      case L'\r': col=1;         /* Carriage return: col. to beginning   */
                  break;
      default   : ++col;
    }
  }
}

/* pm_b() -- Process multibyte data with the -b option. */
 
static void 
pm_b()
{
  char  mb[10];
  short bc;

  bytes = 0; 
  while ((wc = fgetwc(stdin)) != EOF)
  {                                                                        
    bc = wctomb(mb, (wchar_t)wc);
    bytes = wc == L'\n' ? 0 : bytes + bc;
    if (bytes > width && wc != L'\n') 
    {
      putwc(L'\n',stdout);              /*      print a newline         */
      bytes = bc;
    }
    putwc(wc, stdout);                  /* Output the character.        */
  }
}

/* pm_s() -- Process multibyte with the -s option. */
 
static void pm_s()
{
  int     nc;                    /* Number of characters read so far.    */
  int     sc;                    /* Index of last space character.       */
  int     sl;                    /* String length of input line (bytes). */
  int     pos;                   /* Position at which to break line.     */
  int     len;
  int     overwidth;
  wchar_t save;                  /* Temporary variable.                  */
                                                                           
  wb = (wchar_t *) buffer;       /* Set cb to the start of the buffer.   */
  *wb = (wchar_t) NULL;          /* Ensure that we have a null string.   */
                                 
  for(;;)                        /* Loop until line is broken.           */
  {                                                                        
    if(!*wb)                     /* Empty buffer? Then try to get a line */
    {
      wb = (wchar_t *) buffer;   /* Set cb to the start of the buffer.   */
      if(!fgetws(wb,LINE_MAX,stdin)) /* Get a line of input.           */
          break;                 /* Then break and return to main().     */
      else                                                                 
      {                                                                    
        sl = wcslen(wb);          /* Find the length of the string and   */
        longline = wb[sl-1]!=L'\n'; /*   check whether the last byte     */
        if(!longline)             /*   is a newline; if so, change it to */
          wb[sl-1] = (wchar_t)NULL; /*   a terminating NULL.             */
      }
    }

    /* Loop until the column count reaches the specified width or the    */
    /*   end of the input line is reached.                               */

    sl = wcslen(wb);
    overwidth = FALSE;
    for(col=1, nc=sc=0; col<=width && nc<sl; nc++)
    {
      switch(wb[nc])               /* Adjust column based on last char.    */
      {                        
        case L'\b': if(col>1)      /* Backspace: Decrement counter by 1,   */
                      --col;       /*   but don't let it become negative.  */
                    break;                                                   
        case L'\r': col = 1;       /* Carriage return: col. to beginning   */
                    break;
        case L'\t': col = (col + 8) & ~7;  /* Tab: Adjust to next tab */
                    if (col > width && width > 8)
                      overwidth = TRUE;
        default   : col += (len = wcwidth(wb[nc])) == -1 ? 1 : len; /* Other: Just increment the   */
                                            /*   counter.                  */ 
      }
      if (iswblank(wb[nc]))
        sc = nc;
    }                                                                         
    if (col > width + 1)           /* If multibyte char on boundary?       */
      nc--;
    if (col <= width)              /* Width not yet reached?               */ 
    {                                                                         
      putws(wb);                   /* Then print the entire line.          */ 
      *wb = (wchar_t)NULL;         /* Empty the buffer.                    */ 
    }                                                                         
    else                           /* Line is longer than "width".         */ 
    {                              /* Save the character at the break      */ 
      pos = iswblank(wb[sc]) ? overwidth ? sc : sc + 1 : nc;
      save = wb[pos];              /*   position, stash a terminating NULL */ 
      wb[pos] = (wchar_t)NULL;     /*   there, and print out the string;   */ 
      putws(wb);                   /*   after that, move the entire tail   */ 
      wb[pos] = save;              /*   of the buffer to the head. We are  */ 
      wb += pos;                   /*   then ready for the next iteration. */ 
    }
  }
}
  
/* pm_bs() -- Process multibyte with the -b and -s options. */
 
static void pm_bs()
{
  char mb[10];
  wchar_t save;                  /* Temporary variable.                  */
  int pos, nc = 0, sc = 0, sbytes = 0;
  short bc;

  if (!(wb = malloc(sizeof (wchar_t) * (width + 1))))
  {
    perror("malloc");
    exit(1);
  }
  bytes = 0;
  while ((wc = fgetwc(stdin)) != EOF)
  {
    wb[nc] = wc;
    bc = wctomb(mb, (wchar_t)wc);
    if (bytes + bc > width && wb[nc] != L'\n')
    {
      pos = iswblank(wb[sc]) ? sc + 1 : nc;
      bytes = iswblank(wb[sc]) ? bytes - sbytes : 0;
      save = wb[pos];
      wb[pos] = (wchar_t)NULL;
      putws(wb);
      wb[pos] = save;
      wcscpy(wb, wb + pos);
      nc -= pos;
    }
    if (iswblank(wb[nc]))
    {
      sc = nc;
      sbytes = bytes + bc;
    }
    if (wb[nc] == L'\n')
    {
      wb[nc] = (wchar_t)NULL;
      putws(wb);
      bytes = nc = sbytes = 0;
    }
    else
    {
      bytes += bc;
      nc++;
    }
  }
}

static usage()
{
    fprintf(stderr, MSGSTR(USAGE, "Usage: fold [-bs] [-w Width] [File...]\n"));
    exit(1);
}
