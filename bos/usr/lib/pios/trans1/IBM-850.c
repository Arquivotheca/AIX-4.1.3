static char sccsid[] = "@(#)81	1.1  src/bos/usr/lib/pios/trans1/IBM-850.c, cmdpios, bos411, 9428A410j 5/14/91 03:55:17";

/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** IBM-850.c - builds Stage 1 translate table for Code Page 850 ***/

/*
   This is a Stage 1 translate table for tranlate table format type number 1
   (256 short integers, one for each code point for the input data stream).  The
   table is used for stage 1 code point translation from the input data stream
   code points to code points for the intermediate code page.  The
   intermediate code page code points will be translated to code points for the
   printer's code page during Stage 2 translation.

   Each entry in the table can be one of three values:
      (1) the code point for the intermediate code page,
      (2) the defined value "CP", which indicates that the input character's
	  code point is to be "copied"; i.e., the character's code point in the
	  intermediate code page is the same as its code point in the code page
	  for the input data stream, or
      (3) the defined value "RM", which indicates that the input character
	  should be "removed" (i.e., discarded).

   To build the translate table file, do the following:
      (1) Verify that the table values and the value defined for "CODEPAGE"
	  below are correct.
      (2) Verify that the name of this file is xxx.c, where "xxx" is the value
	  defined below for "CODEPAGE".
      (3) cc xxx.c  (where "xxx" is the value defined below for "CODEPAGE")
      (4) a.out dirname  (where "dirname" is the path name of the
			  directory where the file is to be created)
*/

#define CODEPAGE  "IBM-850"   /* translate table file to be created */

#include "piostruct.h"


/*******************************************************************************
*               Table to Translate Code Points for Code Page 850               *
*                 to Code Points for the Intermediate Code Page                *
*******************************************************************************/

short table[256] = {
/*
		  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
		--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
*/
/* 00 (000) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 10 (016) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 20 (032) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 30 (048) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 40 (064) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 50 (080) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 60 (096) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 70 (112) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 80 (128) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* 90 (144) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* A0 (160) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* B0 (176) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* C0 (192) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* D0 (208) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* E0 (224) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
/* F0 (240) */   CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP
};


/*******************************************************************************
*                     Program To Write the Table To a File                     *
*******************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define HEADER "PIOSTAGE1XLATE00"  /* file header */

main(argc, argv)
unsigned argc;
char *argv[];
{
extern int errno;
int fildes;
int fmt_type = 1;
char filename[200];

/* Construct File Name */
if (argc < 2) {
    fprintf(stderr,"ERROR: directory path for output file must be specified\n");
    return(1);
}
strcpy(filename, argv[1]);
strcat(filename, "/");
strcat(filename, CODEPAGE);

/* Open the File */
if ((fildes = open(filename, O_CREAT | O_WRONLY, 0664)) < 0)
{
    fprintf(stderr,"ERROR: Unable to open file \"%s\", errno = %d\n",
      filename, errno);
    return(1);
}

/* Write the Data to the File */
if (write(fildes, HEADER, sizeof(HEADER) - 1) < 0
      || write(fildes, &fmt_type, sizeof(fmt_type)) < 0
      || write(fildes, table, sizeof(table)) < 0) {
    fprintf(stderr,"ERROR writing to file \"%s\", errno = %d\n",
      filename, errno);
    (void) close(fildes);
    return(1);
}
(void) close(fildes);
return(0);
}
