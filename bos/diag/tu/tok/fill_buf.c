static char sccsid[] = "src/bos/diag/tu/tok/fill_buf.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:21";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
 *
 * FUNCTIONS: fill_buf
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Fill Buffer with Pattern

Module Name :  fill_buf.c
SCCS ID     :  1.5

Current Date:  6/20/91, 10:14:45
Newest Delta:  5/14/91, 16:20:42

This support function fills a buffer pointed to by a user supplied
char ptr. with a pattern (0x00 - 0x0f) or one from a pattern file.

*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "toktst.h"	/* note that his also includes hxihtx.h */

/*****************************************************************************

fill_buf

Function fills up the specified buffer (starting at "buf_ptr") with
a pattern of data.  If running with HTX and a "pattern_id" has been
specified as a keyword, function attempts to open file specified
and use the data therein for the pattern.  Else, function fills
in with a default pattern.

*****************************************************************************/

int fill_buf (register unsigned char *buf_ptr,
              register unsigned char *end_buf,
              TUTYPE *tucb_ptr)
   {
	register int i, cc;
	unsigned char pattern_char;
	register unsigned char *buf_start;
	int use_default;
	FILE *p_fp;

	use_default = 0;
	/*
	 * check if specified "pattern_id"
	 */
	if (tucb_ptr->header.mfg == INVOKED_BY_HTX)
	   {
		if (strcmp(tucb_ptr->token_s.pattern_id,"|none|") != 0)
		   {
			p_fp = fopen(tucb_ptr->token_s.pattern_id, "r");
			if (p_fp == NULL)
			   {
				/*
				 * return error....
				 */
				return(PAT_OPEN_ERR);
			   }
		   }
		else
			use_default = 1;
	   }
	else
		use_default = 1;
	
	buf_start = buf_ptr;
	if (!use_default)
	   {
		while (buf_ptr < end_buf)
		   {
			cc = fgetc(p_fp);
			if (cc == EOF)
			   {
				if (buf_start == buf_ptr)
				   {
					/*
					 * empty pattern file...return err
					 */
					fclose(p_fp);
					return(PAT_EMP_ERR);
				   }
				rewind(p_fp);
				continue;
			   }
			pattern_char = (unsigned char) cc;
			*buf_ptr = pattern_char;
			buf_ptr++;
		   }
		fclose(p_fp);
		return(0);
	   }

	/*
	 * ok, let's fill in the data portion of the frame with default.
	 * Use a pattern with the indexed value modulus 16.  That is,
	 * byte 14 will have a value of 14, byte 15 value 15,
	 * byte 16 value 0, byte 17 value 1, etc.
	 */
	i = (2 + (2 * NETADD_LEN)) % 16;
	while (buf_ptr < end_buf)
	   {
		*buf_ptr = (unsigned char) i;
		i = (i + 1) % 16;
		buf_ptr++;
	   }
	return(0);
   }
