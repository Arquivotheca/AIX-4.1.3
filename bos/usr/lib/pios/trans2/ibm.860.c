static char sccsid[] = "@(#)34	1.1  src/bos/usr/lib/pios/trans2/ibm.860.c, cmdpios, bos411, 9428A410j 6/11/91 10:28:43";

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

/*** ibm.860.c ***/

/*
   This is a Stage 2 translate table for translation from code points for the
   intermediate code page (generated by Stage 1 translation) to code points
   for the printer's code page.  There is one entry in the table for each
   intermediate code page code point, beginning at intermediate code page code
   point zero and ending with the highest intermediate code page code point
   that can be translated to a printer code page code point for this particular
   printer code page.

   Each entry in the translate table must be of the form

      { charvalue | CP | SC  [,index1 [,index2] ] }

   where | vertical bars separate mutually exclusive choices, and [] brackets
   surround options.  The values that may be specified are:

      charvalue - the code point for the printer code page
      CP - copy the intermediate code page code point to the printer code page
	   code point (i.e., use the same code point)
      SC - use a substitute character (can't print the character)

      index1 - index into cmdnames array that specifies a command to be output
	       BEFORE the code point is output.
      index2 - index into cmdnames array that specifies a command to be output
	       AFTER the code point is output.

   To build the translate table file, do the following:
      (1) Verify that the table values and the value defined for "CODEPAGE"
	  below are correct.
      (2) Verify that the name of this file is xxx.c, where "xxx" is the value
	  defined below for "CODEPAGE".
      (3) cc xxx.c  (where "xxx" is the value defined below for "CODEPAGE")
      (4) a.out dirname  (where "dirname" is the path name of the
			  directory where the file is to be created)
*/

#define CODEPAGE "ibm.860" /* name of the translate table file to be created */

#include "piostruct.h"



/*******************************************************************************
*         Table to Translate Code Points for the Intermediate Code Page        *
*                  to Code Points for the Printer's Code Page                  *
*******************************************************************************/
struct transtab table[] = {

/*        0        1        2        3        4        5        6        7
	  8        9        A        B        C        D        E        F
*/
/* 00*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 08*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 10*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 18*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 20*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 28*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 30*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 38*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 40*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 48*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 50*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 58*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 60*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 68*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 70*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 78*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* 80*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{SC}    ,{CP}    ,{SC}    ,{CP}    ,
/* 88*/ {CP}    ,{SC}    ,{CP}    ,{SC}    ,{SC}    ,{CP}    ,{SC}    ,{SC}    ,
/* 90*/ {CP}    ,{SC}    ,{SC}    ,{CP}    ,{SC}    ,{CP}    ,{SC}    ,{CP}    ,
/* 98*/ {SC}    ,{SC}    ,{CP}    ,{SC}    ,{CP}    ,{SC}    ,{SC}    ,{SC}    ,
/* A0*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* A8*/ {CP}    ,{SC}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,
/* B0*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{0x86}  ,{0x8f}  ,{0x91}  ,
/* B8*/ {SC}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{0x9b}  ,{SC}    ,{CP}    ,
/* C0*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{0x84}  ,{0x8e}  ,
/* C8*/ {CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{SC}    ,
/* D0*/ {SC}    ,{SC}    ,{0x89}  ,{SC}    ,{0x92}  ,{SC}    ,{0x8b}  ,{SC}    ,
/* D8*/ {SC}    ,{CP}    ,{CP}    ,{CP}    ,{CP}    ,{SC}    ,{0x98}  ,{CP}    ,
/* E0*/ {0x9f}  ,{CP}    ,{0x8c}  ,{0xa9}  ,{0x94}  ,{0x99}  ,{CP}    ,{SC}    ,
/* E8*/ {SC}    ,{0x96}  ,{SC}    ,{0x9d}  ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,
/* F0*/ {0x2d}  ,{CP}    ,{SC}    ,{SC}    ,{SC}    ,{SC}    ,{CP}    ,{SC}    ,
/* F8*/ {CP}    ,{SC}    ,{CP}    ,{SC}    ,{SC}    ,{CP}    ,{CP}    ,{CP}    ,
/*100*/ {SC}    ,{0x01,1},{0x02,1},{0x03,1},{0x04,1},{0x05,1},{0x06,1},{0x07,1},
/*108*/ {0x08,1},{0x09,1},{0x0a,1},{0x0b,1},{0x0c,1},{0x0d,1},{0x0e,1},{0x0f,1},
/*110*/ {0x10,1},{0x11,1},{0x12,1},{0x13,1},{0x14,1},{0x15,1},{0x16,1},{0x17,1},
/*118*/ {0x18,1},{0x19,1},{0x1a,1},{0x1b,1},{0x1c,1},{0x1d,1},{0x1e,1},{0x1f,1},
/*120*/ {0x9e}  ,{SC}    ,{0xb5}  ,{0xb6}  ,{0xb7}  ,{0xb8}  ,{0xbd}  ,{0xbe}  ,
/*128*/ {0xc6}  ,{0xc7}  ,{0xcf}  ,{0xd0}  ,{0xd1}  ,{0xd2}  ,{0xd3}  ,{0xd4}  ,
/*130*/ {0xd5}  ,{0xd6}  ,{0xd7}  ,{0xd8}  ,{0xdd}  ,{0xde}  ,{0xe0}  ,{0xe2}  ,
/*138*/ {0xe3}  ,{0xe4}  ,{0xe5}  ,{0xe6}  ,{0xe7}  ,{0xe8}  ,{0xe9}  ,{0xea}  ,
/*140*/ {0xeb}  ,{0xec}  ,{0xed}  ,{0xee}  ,{0xef}  ,{0xf0}  ,{0xf2}  ,{0xf3}  ,
/*148*/ {0xf4}  ,{0xf5}  ,{0xf7}  ,{0xf9}  ,{0xfb}  ,{0xfc}

};


/*******************************************************************************
*                Command Names Specified by the Translate Tables               *
*******************************************************************************/
char cmdnames[][2] = {
{'c', '8'},                     /* index 0 - select the code page */
{'e', 'b'},                     /* index 1 - next byte is graphic, not control*/
};


/*******************************************************************************
*                     Program To Write the Table To a File                     *
*******************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define HEADER "PIOSTAGE2XLATE00"  /* file header */

int num_commands = sizeof(cmdnames) / 2;

main(argc, argv)
unsigned argc;
char *argv[];
{
extern int errno;
int fildes;
int fmt_type = 1;
char filename[500];

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
      || write(fildes, &num_commands, sizeof(num_commands)) < 0
      || write(fildes, cmdnames, sizeof(cmdnames)) < 0
      || write(fildes, table, sizeof(table)) < 0) {
    fprintf(stderr,"ERROR writing to file \"%s\", errno = %d\n",
      filename, errno);
    (void) close(fildes);
    return(1);
}
(void) close(fildes);
return(0);
}
