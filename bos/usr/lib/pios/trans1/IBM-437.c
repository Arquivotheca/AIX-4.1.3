static char sccsid[] = "@(#)80	1.1  src/bos/usr/lib/pios/trans1/IBM-437.c, cmdpios, bos411, 9428A410j 5/14/91 03:55:16";

/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 28
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

/*** IBM-437.c - builds Stage 1 translate table for Code Page 437   ***/

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

#define CODEPAGE  "IBM-437"   /* translate table file to be created */

#include "piostruct.h"


/*******************************************************************************
*               Table to Translate Code Points for Code Page 437               *
*                 to Code Points for the Intermediate Code Page                *
*******************************************************************************/

short table[256] = {
/*
		    0     1     2     3     4     5     6     7
		    8     9     A     B     C     D     E     F 
		----- ----- ----- ----- ----- ----- ----- -----
*/
/* 00 (000) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 08 (008) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 10 (016) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 18 (024) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 20 (032) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 28 (040) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 30 (048) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 38 (056) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 40 (064) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 50 (072) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 50 (080) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 58 (088) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 60 (096) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 68 (104) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 70 (112) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 78 (120) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 80 (128) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 88 (136) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 90 (144) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* 98 (152) */     CP,   CP,   CP, 0xbd,   CP, 0xbe,0x120,   CP,
/* A0 (160) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,   CP,
/* A8 (168) */     CP,0x121,   CP,   CP,   CP,   CP,   CP,   CP,
/* B0 (176) */     CP,   CP,   CP,   CP,   CP,0x122,0x123,0x124,
/* B8 (184) */  0x125,   CP,   CP,   CP,   CP,0x126,0x127,   CP,
/* C0 (192) */     CP,   CP,   CP,   CP,   CP,   CP,0x128,0x129,
/* C8 (200) */     CP,   CP,   CP,   CP,   CP,   CP,   CP,0x12a,
/* D0 (208) */  0x12b,0x12c,0x12d,0x12e,0x12f,0x130,0x131,0x132,
/* D8 (216) */  0x133,   CP,   CP,   CP,   CP,0x134,0x135,   CP,
/* E0 (224) */  0x136,   CP,0x137,0x138,0x139,0x13a,   CP,0x13c,
/* E8 (232) */  0x13d,0x13e,0x13f,0x140,0x141,0x142,0x143,0x144,
/* F0 (240) */  0x145,   CP,0x146,0x147,0x148,0x149,   CP,0x14a,
/* F8 (248) */     CP,0x14b,   CP,0x14c,0x14d,   CP,   CP,   CP
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
