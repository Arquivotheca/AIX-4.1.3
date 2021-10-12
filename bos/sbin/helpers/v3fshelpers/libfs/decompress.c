static char sccsid[] = "@(#)46	1.5  src/bos/sbin/helpers/v3fshelpers/libfs/decompress.c, cmdfs, bos411, 9428A410j 4/25/94 14:57:25";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: 	valid_algorithm
 *		valid_compression
 * 		decompress
 *		loadmodule
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <libfs/libfs.h>
#include "jfsc.h"
#include <sys/sysconfig.h>
#include <sys/access.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/statfs.h>

#define MASK(x)	((x & 0x0000FFF) >> 8)
static struct compmethod *comp;

extern int encode_decode(int, int, caddr_t, size_t, caddr_t, size_t);
extern int getcompent(struct compmethod **);

/*
 *  NAME:	valid_algorithm(algorithm)
 *
 *  PARAMETERS: algorithm - specifies compression algorithm to use
 *
 *  FUNCTION: 	Tests if the specified compression algorithm is valid.
 *
 *  RETURN VALUES
 *	1 if invalid compression algorithm, 0 if valid compression 
 *	algorithm specified.
 */
int
valid_algorithm(int algorithm)
{
	struct compmethod *cptr;

	if (algorithm == NO_COMPRESS)
		return(0);

	if (comp == (struct compmethod *)NULL)
		if (loadmodule(&comp))
			return(1);

	for (cptr = comp; cptr; cptr = cptr->next)
	{
		if (cptr->type == algorithm)
			return(0);
	}
	return(1);
}

/*
 *  NAME:	valid_compression(name)
 *
 *  PARAMETERS: name 	- compression algorithm name in text format
 *
 *  FUNCTION: 	Tests if the specified compression is valid and returns
 *		compression type.
 *
 *  RETURNS:	Compression algorithm type or -1 if invalid compression name
 */
int
valid_compression(char *name)
{
	struct compmethod *cptr;
	struct statfs st;
	int    rc = -1;

	if (name == (char *)NULL)
		return(rc);

	if (comp == (struct compmethod *)NULL)
		if (loadmodule(&comp))
			return(rc);

	/*
	 * Loop thru compression names.
	 */
	for (cptr = comp; cptr; cptr = cptr->next)
	{
		if (!strcmp(name, cptr->name))
		{
			rc = cptr->type;
			break;
		}
	}
	return(rc);
}
	

/*
 *  decompress(int algorithm, char *input, char *output, int nbytes)
 *      algorithm:  specifies compression algorithm to use.
 *	input:     ptr to data to be decompressed.
 *	output:    ptr to buffer for uncompressed input data.
 *	nbytes:    length of the compressed data in input.
 *
 *  FUNCTION
 *      if the specified compression algorithm is valid then call the
 *	appropriate decompression routine to .
 *
 *  RETURN VALUES
 *      -1 if invalid compression algorithm is specified, othereise, the
 *	return value of the decompression routine is returned.
 */
int
decompress(int algorithm,
           char *input,
           char *output,
           int nbytes)
{
	struct compmethod *cptr;

	if (comp == (struct compmethod *)NULL)
		if (loadmodule(&comp))
			return(-1);

	return(encode_decode(COMP_DECODE,algorithm,input,4096,output,nbytes));
}

/* 
 * NAME:	loadmodule(comp)
 *
 * FUNCTION:	Loads user level portion of the compress kernel extension
 *		and sets function pointers to encode/decode functions.
 * 
 * RETURNS:	 0 	- load succeeded
 *		-1 	- load failed
 */
static int 
loadmodule(struct compmethod **comp)
{
	if (load(COMP_USERLEVEL_PATH, 1, "/usr/lib") == NULL)
		return(-1);

	if (getcompent(comp))
		return(-1);

	return(0);
}
