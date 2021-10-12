static char sccsid[] = "@(#)83	1.15  src/bos/usr/bin/paste/paste.c, cmdfiles, bos412, 9446C 11/14/94 16:49:10";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 18, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 *
 * $RCSfile: paste.c,v $ $Revision: 1.7.2.3 $ (OSF) $Date: 92/03/09 21:28:25 $
 */


#include <sys/limits.h>
#include "paste_msg.h"
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MSGSTR(id,ds) catgets(catd, MS_PASTE, id, ds)
static nl_catd catd;

#define RUB  '\177'
#define wcNEWLINE '\n'
#define SPECIAL -1
static wchar_t del[LINE_MAX] = L"\t";

static int mbcm;
  
/*
 * NAME: paste file ...
 *       paste -d List file ...
 *       paste -s [-d List] file ...
 *                                                                    
 * FUNCTION: Merges the lines of several files or subsequent lines in 
 *           one file.
 *                                                                    
 * NOTES:  Reads standard input if - is given as a file.
 *         Default is to treat each file as a column and join them
 *         horizontally.
 *         -d List   List of delimiters to separate fields
 *         -s        Merges subsequent lines from the first file
 *                   horizontally
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *
 * RETURN VALUE DESCRIPTION: What this code returns (NONE, if it has no 
 *                           return value)
 */  

/* Note: This code is compliant with POSIX 1003.2 Draft 11. */

main(argc, argv)
int argc;
char **argv;
{
        int i, j, k, eofcount, nfiles, maxline, glue;
        int delcount = 1;
        int onefile  = 0;
        wint_t wc ;
        wchar_t outbuf[LINE_MAX], l, t;
        wchar_t *p;
        FILE *inptr[OPEN_MAX];
        int arg;
	int status = 0; 			/* exit status */
	struct stat mbuf;
  
        (void) setlocale(LC_ALL,"");
        catd = catopen(MF_PASTE, NL_CAT_LOCALE);
        mbcm = MB_CUR_MAX;

        maxline = LINE_MAX - 2;
 
        while ((arg = getopt(argc,argv,"sd:")) != SPECIAL)
        {
          switch (arg)
          {
            case 's' : onefile++;
                         break;
            case 'd' : if((delcount = move(optarg, &del[0])) < 1)
                          {
                            fprintf(stderr,MSGSTR(M_NODEL,
                                  "paste: specify \\0 for null delimiter\n"));
                            exit(1);
                          }
                       break;
             default : fprintf(stderr,MSGSTR(M_USAGE,
                            "Usage: paste [-s] [-d List] file...\n"));
		       exit(1);
                       break;
          }
        }

        argv += optind;
        argc -= optind;

        if (argc <= 0)
        { 
              fprintf(stderr,MSGSTR(M_USAGE,
                  "Usage: paste [-s] [-d List] file...\n"));
              exit(1);
        }
 
        if ( ! onefile)
        {       /* not -s option: parallel line merging */
          for (i = 0; i<argc &&  i<OPEN_MAX; i++)
          {
            if ((argv[i][0] == '-') && (argv[i][1] == '\0'))
              inptr[i] = stdin;
            else
	      {
	       if (lstat(argv[i],&mbuf) <0 )
                  {
                    fprintf(stderr,MSGSTR(M_NOPEN, "paste: %s: cannot open\n"), argv[i]);
                    exit(1);
                   }
               if ((mbuf.st_mode & S_IFMT) == S_IFDIR)
                  {
                    fprintf(stderr,MSGSTR(M_DIR, "paste: %s: cannot open directory\n"), argv[i]);
                    exit(1);
                  }
                    
               inptr[i] = fopen(argv[i], "r");
              }
            if (inptr[i] == NULL)
            {
              fprintf(stderr,MSGSTR(M_NOPEN, "paste: %s: cannot open\n"), argv[i]);
              exit(1);
            }
          }
          if (argc > OPEN_MAX)
          {
              fprintf(stderr,MSGSTR(M_2MANY, "paste: too many files\n"));
              exit(1);
          }
                nfiles = i;
  
                 do
                 {
                        eofcount = 0;
                        p = &outbuf[0];
                        k = 0;
                        j = 0;  /* j is char offset on line */
                        for (i = 0; i < nfiles; i++)
                        {
                          while((inptr[i] != NULL) && (wc = getwc(inptr[i])), wc!='\n' && wc!=WEOF) {
                              *p++ = wc;
                              j++;
                              if (j >= maxline) {
                                  *p = 0;
                                  fputws(outbuf,stdout);
                                  p = &outbuf[0];
                                  j = 0;
                                  }
                              }
                              
                          if ((l = del[k]) != RUB) {
                              *p++ = l; 
                              j++; 
                              }
                          k = (k + 1) % delcount;

                          if((inptr[i]==NULL) || (wc == WEOF)) {
                            eofcount++;
                            if ((inptr[i] != NULL) && (inptr[i] != stdin))
                                 fclose(inptr[i]);
                            inptr[i] = NULL;
                          }
                        }

                        if (l != RUB)
                          *--p = '\n'; 
                        else
                          *p = '\n';
                        *++p = 0;
                        if (eofcount < nfiles)
                          fputws(outbuf, stdout);
                } 
                while (eofcount < nfiles);
        }
        else
        {        /* -s option: serial file pasting (old 127 paste command) */
          p = &outbuf[0];
          j = k = t = 0;
          glue = 0;
          for (i = 0; i < argc && i < OPEN_MAX; i++)
          {
            glue = 0;                       /* IBM bug fix */
            if (argv[i][0] == '-' && argv[i][1]==NULL)
              inptr[0] = stdin;
            else
	      {
	       if (lstat(argv[i],&mbuf) <0 )
                  {
                    fprintf(stderr,MSGSTR(M_NOPEN, "paste: %s: cannot open\n"), argv[i]);
                    status = 1;
                    continue;
                   }
               if ((mbuf.st_mode & S_IFMT) == S_IFDIR)
                  {
                    fprintf(stderr,MSGSTR(M_DIR, "paste: %s: cannot open directory\n"), argv[i]);
                    status = 1;
		    continue;
                  }
                    
               inptr[0] = fopen(argv[i], "r");
              }
            if (inptr[0] == NULL)
            {
              fprintf(stderr,MSGSTR(M_NOPEN, "paste: %s: cannot open\n"), argv[i]);
	      status = 1;
              continue;			/* skip to next file */
            }
          
            while((wc = getwc(inptr[0])) != WEOF)
            {
              if (j >= maxline)   /* ?? Refers to maxline; remove? */
              {
                t = *--p;
                *++p = 0; /* !!?? */
                fputws(outbuf, stdout);
                p = &outbuf[0];
                j = 0;
              }
              if (glue)
              {
                glue = 0;
                l = del[k];
                if (l != RUB)
                {
                  *p++ = l ;
                  t = l ;
                  j++;
                }
                k = (k + 1) % delcount;
              }
              if(wc != '\n')
              {
                *p++ = wc;
                t = wc;
                j++;
              }
              else
                glue++;
            }
            fclose(inptr[0]);
            if (t != '\n')
            {
              *p++ = '\n';
              j++;
            }
            if (j > 0)
            {
              *p = 0;
              fputws(outbuf, stdout);
              p = &outbuf[0];         /* IBM bug fix */
            }
          }
        }
        exit(status);        /* exit */
}

/*
 * NAME: move
 *                                                                    
 * FUNCTION: move one string to another checking for special characters
 *           along the way.
 *
 * RETURN: return the length of the copied string.
 */  
static move(char *from, wchar_t *to)
{
wchar_t wc;
int     i, len;

        i = 0;
        do
        {
          len = mbtowc(&wc,from,mbcm);
          from += len;
          i++;
          if (wc != '\\')
            *to++ = wc;
          else
          {
            len = mbtowc(&wc,from,mbcm);
            from += len;
            switch (wc)
            {
              case '0' : *to++ = RUB;
                         break;
              case 't' : *to++ = '\t';
                         break;
              case 'n' : *to++ = '\n';
                         break;
              default  : *to++ = wc;
                         break;
            }
          }
        } while (wc) ;
return(--i);
}
