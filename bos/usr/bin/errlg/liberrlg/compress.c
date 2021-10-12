static char sccsid[] = "@(#)84    1.1  src/bos/usr/bin/errlg/liberrlg/compress.c, cmderrlg, bos411, 9428A410j  4/2/93  17:21:08";

/* COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: iscompressed, uncompress, compress 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SIZE 256

int compress();
int uncompress();
int iscompressed();

/*
 *
 * NAME: iscompressed
 *
 * FUNCTION: This function checks to see whether or not a file
 *           is compressed.
 *
 * RETURNS:  1 - file is compressed
 *           0 - file is not compressed
 *
 */

int iscompressed(file)
char *file;
{
char Tempfile[256];
struct stat *statbuf;

	  strcpy(Tempfile, file);
	  strcat(Tempfile, ".Z");
	  if ((stat(Tempfile, &statbuf)) == 0)
		  return (1);
	  return (0);
}


/*
 * NAME: uncompress
 *
 * FUNCTION: This function uncompresses a given file.  You should use
 *	     iscompressed() before calling this to verify that the file
 *           is in compressed format.	
 *
 * RETURNS:
 *           -1  uncompress failed
 *            0  uncompress worked
 *
 */

int uncompress(file)
char *file;
{
char buf[SIZE];

	   sprintf(buf,"uncompress %s >/dev/null 2>&1", file);
	   if (system(buf)) {
		   return (-1);
	   }
	   return (0);
}


/*
 * NAME: compress
 *
 * FUNCTION: This function compresses a given file. 
 *
 * RETURNS:
 *           -1  compress failed
 *            0  compress worked
 *
 */

int compress(file)
char *file;
{
char buf[SIZE];

	 sprintf(buf,"compress %s >/dev/null 2>&1", file);
	 if (system(buf)) {
		 return (-1);
	 }
	 return (0);
}

