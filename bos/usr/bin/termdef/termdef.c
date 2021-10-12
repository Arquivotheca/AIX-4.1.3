#ifndef lint
static char sccsid[] = "@(#)08	1.5  src/bos/usr/bin/termdef/termdef.c, cmdtty, bos411, 9428A410j 2/3/94 03:04:08";
#endif

/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main (termdef.c)
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*--------------------------------------------------------------*/
/*  FUNCTION : termdef command                                  */
/*                                                              */
/*  DESCRIPTION:                                                */
/*      Print a character string about the terminal.            */
/*                                                              */
/*  SYNTAX:                                                     */
/*      termdef -t                                              */
/*      termdef -c                                              */
/*      termdef -l                                              */
/*                                                              */
/*      where -t = prints terminal name                         */
/*            -c = print number of columns on the terminal      */
/*            -l = print number of rows on the terminal         */
/*                                                              */
/*--------------------------------------------------------------*/

#include <stdio.h>
#include <locale.h>
#include <nl_types.h>
#include "termdef_msg.h"

char *termdef(int fd, char c);

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_TERMDEF,num,str)

main(int argc, char *argv[])
{
  int c;

  setlocale(LC_ALL,"") ;
  catd = catopen(MF_TERMDEF, NL_CAT_LOCALE);
 
  if (argc > 2) 
	  usage();
  else if (argc == 1)
	  printf("%s\n", termdef(0, 't'));
  else {
    c = getopt(argc,argv,"tcl");
    switch (c) {
      case 't':
      case 'c':
      case 'l':
	      printf("%s\n", termdef(0, argv[1][1]));
	      break;
      default:
	      usage();
	    }
  }
  return 0;
}

usage()
{
  fprintf(stderr, MSGSTR(USAGE, "Usage: termdef [-t | -c | -l]\n"));
  exit(2);
}
