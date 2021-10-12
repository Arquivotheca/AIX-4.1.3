static char sccsid[] = "@(#)86	1.2  src/bos/usr/lib/pios/trans1/ISO8859-5.c, cmdpios, bos411, 9428A410j 5/28/91 14:06:29";

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

/** ISO8859-5.c - builds Stage 1 translate table for 8859-5(Cyrillic)**/

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

#define CODEPAGE  "ISO8859-5"   /* translate table file to be created */

#include "piostruct.h"


/*******************************************************************************
*               Table to Translate Code Points for Code Page 8859-5(Cyrillic)  *
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
/* 78 (120) */     CP,   CP,   CP,   CP,   CP,   CP,   CP, 0x5f,
/* 80 (128) */   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
/* 88 (136) */   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
/* 90 (144) */   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
/* 98 (152) */   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
/* A0 (160) */   0xff, 0xd3,0x1bf,0x1c0,0x1c1, 0x53, 0x49, 0xd8,
/* A8 (168) */   0x4a,0x1c2,0x1c3,0x1c4,0x1c5, 0xf0,0x1c6,0x1c7,
/* B0 (176) */   0x41,0x1c8, 0x42,0x137,0x1c9, 0x45,0x1ca,0x1cb,
/* B8 (184) */  0x1cc,0x1cd,0x1ce,0x1cf, 0x4d, 0x48, 0x4f,0x1d0,
/* C0 (192) */   0x50, 0x43, 0x54,0x1d1,0x13d, 0x58,0x1d2,0x1d3,
/* C8 (200) */  0x1d4,0x1d5,0x1d6,0x1d7,0x1d8,0x1d9,0x1da,0x1db,
/* D0 (208) */   0x61,0x1dc,0x1dd,0x1de,0x1df, 0x65,0x1e0,0x1e1,
/* D8 (216) */  0x1e2,0x1e3,0x1e4,0x1e5,0x1e6,0x1e7, 0x6f,0x1e8,
/* E0 (224) */   0x70, 0x63,0x1e9, 0x79,0x1ea, 0x78,0x1eb,0x1ec,
/* E8 (232) */  0x1ed,0x1ee,0x1ef,0x1f0,0x1f1,0x1f2,0x1f3,0x1f4,
/* F0 (240) */  0x1f5, 0x89,0x1f6,0x1f7,0x1f8, 0x73, 0x69, 0x8b,
/* F8 (248) */   0x6a,0x1f9,0x1fa,0x1fb,0x1fc, 0xf5,0x1fd,0x1fe
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
