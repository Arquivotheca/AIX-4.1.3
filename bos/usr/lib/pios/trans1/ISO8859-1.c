static char sccsid[] = "@(#)82	1.2  src/bos/usr/lib/pios/trans1/ISO8859-1.c, cmdpios, bos411, 9428A410j 5/28/91 14:06:05";

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

/*** ISO8859-1.c - builds Stage 1 translate table for 8859-1(Latin 1)***/

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

#define CODEPAGE  "ISO8859-1"   /* translate table file to be created */

#include "piostruct.h"


/*******************************************************************************
*               Table to Translate Code Points for Code Page 8859-1(Latin 1)   *
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
/* A0 (160) */   0xff, 0xad, 0xbd, 0x9c, 0xcf, 0xbe, 0xdd, 0xf5,
/* A8 (168) */   0xf9, 0xb8, 0xa6, 0xae, 0xaa, 0xf0, 0xa9, 0xee,
/* B0 (176) */   0xf8, 0xf1, 0xfd, 0xfc, 0xef, 0xe6, 0xf4, 0xfa,
/* B8 (184) */   0xf7, 0xfb, 0xa7, 0xaf, 0xac, 0xab, 0xf3, 0xa8,
/* C0 (192) */   0xb7, 0xb5, 0xb6, 0xc7, 0x8e, 0x8f, 0x92, 0x80,
/* C8 (200) */   0xd4, 0x90, 0xd2, 0xd3, 0xde, 0xd6, 0xd7, 0xd8,
/* D0 (208) */   0xd1, 0xa5, 0xe3, 0xe0, 0xe2, 0xe5, 0x99, 0x9e,
/* D8 (216) */   0x9d, 0xeb, 0xe9, 0xea, 0x9a, 0xed, 0xe8, 0xe1,
/* E0 (224) */   0x85, 0xa0, 0x83, 0xc6, 0x84, 0x86, 0x91, 0x87,
/* E8 (232) */   0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b,
/* F0 (240) */   0xd0, 0xa4, 0x95, 0xa2, 0x93, 0xe4, 0x94, 0xf6,
/* F8 (248) */   0x9b, 0x97, 0xa3, 0x96, 0x81, 0xec, 0xe7, 0x98
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
