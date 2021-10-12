static char sccsid[] = "@(#)15	1.1  src/bos/usr/lib/pios/trans1/IBM-1046.c, cmdpios, bos411, 9428A410j 11/4/93 16:26:37";

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

/*** 1046.c - builds Stage 1 translate table for 1046 (Arabic) ***/

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

#define CODEPAGE  "IBM-1046"   /* translate table file to be created */

#include "piostruct.h"


/*******************************************************************************
*               Table to Translate Code Points for Code Page 8859-6(Arabic)    *
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
/* 80 (128) */  0x2B8, 0x9E, 0xF6,0x2A7,0x2A8,0x2A9,0x2AA,0x2AB,
/* 88 (136) */     CP, 0xC4, 0xB3, 0xC1, 0xBF, 0xC0, 0xDA, 0xD9,
/* 90 (144) */  0x2AC,0x2AD,0x2A0,0x2AE,0x2AF,0x2B0,0x2A2,0x29D,
/* 98 (152) */  0x2A3,0x29F,0x295,0x2A4,0x2A6,0x278,0x2B1,0x27A,
/* A0 (160) */   0xff,0x2B3,0x2B4,0x2B5, 0xcf,0x2b6,0x2B7,0x28A,
/* A8 (168) */  0x28B,0x28C,0x28D,0x28E,0x22c,0xf0,0x28F,0x290,
/* B0 (176) */  0x27F,0x280,0x281,0x282,0x283,0x284,0x285,0x286,
/* B8 (184) */  0x287,0x288,0x291,0x22d,0x292,0x293,0x289,0x22e,
/* C0 (192) */  0x294,0x22f,0x230,0x231,0x232,0x233,0x252,0x235,
/* C8 (200) */  0x236,0x237,0x238,0x239,0x23a,0x23b,0x23c,0x23d,
/* D0 (208) */  0x23E,0x23F,0x240,0x241,0x242,0x243,0x244,0x245,
/* D8 (216) */  0x246,0x247,0x248,0x29E,0x27C,0x27D,0x27E,0x296,
/* E0 (224) */  0x249,0x24a,0x24b,0x24c,0x24d,0x24e,0x24f,0x2B9,
/* E8 (232) */  0x251,0x252,0x253,0x254,0x255,0x256,0x257,0x258,
/* F0 (240) */  0x259,0x25a,0x25b,0x297,0x298,0x299,0x27B,0x2A5,
/* F8 (248) */  0x277,0x2B2,0x279,0x29A,0x29B,0x2A1,0x250,   CP
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
