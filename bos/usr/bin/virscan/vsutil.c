static char sccsid[] = "@(#)25	1.2  src/bos/usr/bin/virscan/vsutil.c, cmdsvir, bos411, 9428A410j 4/11/91 18:42:56";
/*
 *   COMPONENT_NAME: CMDSVIR
 *
 *   FUNCTIONS: mask
 *		pr_hex
 *		pr_masked_hex
 *		get_line_from_sig_file
 *		dump_bsect
 *		beep_until_key_pressed
 *		pr_then_clear_line
 *		clear_line_if_required
 *		outp_progress_line
 *		mycmp
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Utility routines used by VIRSCAN.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "vstypes.h"
#include "vsdefs.h"
#include "vsutil.h"

/*
 * Imported global variables.
 */
extern int prevnamelen;
extern boolean quiet;
extern boolean verbose;
extern boolean cr_works;
extern boolean tolerance_allowed;
extern boolean more_tolerance_allowed;

#ifndef _AIX
#include "vsmsg.h"
extern char *message(unsigned int msg_ind);
#endif

/*
 * Mask a range of bytes with 0xFF.
 * This routine is used to hide signatures in the hash table from the
 * memory scan for viruses.
 */
void
mask(register byte *addr, register int count)
{
	if (count == 0) return;
	--count;
	do
   	{
      		addr[count] ^= 0xFF;
   	}
   	while (count--);
}

/*
 * Print an input range of bytes in hex format.
 */
void
pr_hex(byte *hex, int len)
{
   	int i;

   	for (i=0; i<len; ++i)
      	printf("%02.2X",hex[i]);
}

/*
 * Print an masked input range of bytes in hex format.
 * If signature is a complex signature (i.e. it contains wildcards) then
 * wildcard metacharacters are handled differently.
 */
void
pr_masked_hex(byte *masked_hex, register int len, boolean is_complex_sig)
{
  	register int i;
   	byte unmasked_val;

   	for (i=0; i<len; ++i)
   	{
      		if (is_complex_sig)
      		{
         		unmasked_val = masked_hex[i] ^ 0xFF;
         		if (unmasked_val == 0xFF)
            			printf("??");
         		else
            			printf("%02.2X", unmasked_val);
      		}
      		else
         		printf("%02.2X", masked_hex[i] ^ 0xFF);
   	}
}

/*
 * Get a non-comment line from the file pointed to by "file_ptr".
 * A comment line starts with a '*'.
 * Also ignores blank lines. (Strictly, lines with only a single '\n')
 */
char *
get_line_from_sig_file(char *buffer, int length, FILE *file_ptr)
{
   	do
   	{
      		if (fgets(buffer, length, file_ptr) == NULL)
         		return(NULL);
   	}
   	while (buffer[0] == '*' || buffer[0] == '\n');

   	return buffer;
}

#ifndef _AIX
/*
 * Hex dump a region of memory containing the image of a a boot sector.
 */
void
dump_bsect(byte *buffer, int size)
{
      	int i;

      	printf(message(HEX_DUMP_BS));
      	for (i=0; i<size; ++i)
      	{
         	printf("%02X", ((unsigned char *)buffer)[i]);
         	if ((i+1) % 32 == 0) printf("\n");
      	}
}
#endif

#if BOOTS
#include <conio.h>

/*
 * Beep until a key is pressed.
 */
void
beep_until_key_pressed(void)
{
   	time_t timeval;
   	time_t nexttimeval;

	#define kbhit() my_kbhit()
	extern int my_kbhit(void);
	#define getch() my_getch()
	extern int my_getch(void);

   	fprintf(stderr, MSGSTR(ONE_OR_MORE, MSG_ONE_OR_MORE));
   	fprintf(stderr, MSGSTR(PRESS_ANY_KEY, MSG_PRESS_ANY_KEY));
   	fprintf(stderr, cr_works ? "\r" : "\n");

   	time(&timeval);
   	while(!kbhit())
   	{
      		if (time(&nexttimeval) - timeval >= 1)
      		{
         		timeval = nexttimeval;
         		printf("\a");
      		}
   	}

   	getch();
}
#endif


/*
 * Clear line from end of parameter string to last column of last line
 * of progress report text.
 */
void
pr_then_clear_line(char *msg)
{
   	register int i;

   	printf("%s", msg);
   	for (i=strlen(msg); i<prevnamelen; ++i) printf(" ");
}

/*
 * Clear current display line, ready for new message.
 * Will detect whether or not all progress messages are displayed on the
 * same line.
 */
void
clear_line_if_required(void)
{
   	if (cr_works)
   	{
      		pr_then_clear_line("");
      		printf(" \r");
      		prevnamelen = strlen("");
   	}
}

/*
 * Output a progress line; works correctly whether or not all progress
 * messages are on the same line.
 */
void
outp_progress_line(char *msgbuf)
{
   	if (cr_works)
   	{
      		if (!verbose && !quiet)
      		{
         		pr_then_clear_line(msgbuf);
         		printf(" \r");
         		prevnamelen = strlen(msgbuf);
      		}
      		else
      		{
         		if (verbose)
            			printf("%s\n", msgbuf);
      		}
   	}
   	else
   	{
      		if (verbose)
         		printf("%s\n", msgbuf);
   	}
}

/*
 * Compare masked signature with a test range of bytes.
 * A variable number of mismatched bytes are allowed, depending on the
 * length of the signature.
 */
int
mycmp(byte *masked_signature,
      byte *test_addr,
      register int len,
      int *mismatched_bytes_count, /* Only valid if function rv == 0 */
      struct sigdats *sig)
{
   	register int i;
   	int tolerance = 0;            /* Default tolerance is zero */
   	int mismatch_count = 0;
   	boolean is_complex_sig = sig->is_complex_sig;

   	if (tolerance_allowed && 
	    len >= ((SIZEFRAG-3)+2) && /* Three bytes already matched */
            !is_complex_sig && !sig->disable_mutant_scan)
   	{
      		if (more_tolerance_allowed)
			/* 
			 * Already restricted, so don't need the MAX 
			 */
         		tolerance = (7*(len-(SIZEFRAG-3)))/8;  
      		else  
			/*
			 * Already restricted, so don't need the MAX 
			 */
         		tolerance = (2*(len-(SIZEFRAG-3)))/3;

      		for (i=0; i<len; ++i)
		#if 0
         		if((masked_signature[i] ^ (byte)0xFF) != test_addr[i] &&
             		   (!is_complex_sig ||
              		   (is_complex_sig && (masked_signature[i] != 0x00)) 
			   /* 0xFF^0xFF==0x00 */	))
		#else
         		if ((masked_signature[i] ^ (byte) 0xFF) != test_addr[i])
		#endif
         		{
            			if (++mismatch_count > tolerance)
            			{
               				return 1;
            			}
         		}

      		*mismatched_bytes_count = mismatch_count;
      		return 0;
   	}
   	else /* No tolerance allowed */
   	{
      	   for (i=0; i<len; ++i)
         		if((masked_signature[i] ^ (byte)0xFF) != test_addr[i] &&
             	   	   (!is_complex_sig || 
		   	   (is_complex_sig && (masked_signature[i] != 0x00))
			   /* 0xFF^0xFF==0x00 */	))
         		{
            			return 1;
         		}

      		*mismatched_bytes_count = mismatch_count;
      		return 0;
   	}
}

/*
 * Converts "\n", etc., in place in string.
 *
 */
void
convert_message(char *message)
{
   	char *src;
   	char *dst;

   	src = dst = message;	/* They start out pointing to same place */
   	while(*src != '\0')	/* For every element in message */
   	{
      	   if (*src == '\\')/* If meta character, then process next byte */
      	   {
         	switch(*(++src))	/* Point to next source byte */
         	{
		   case('n'):
               		*(dst++) = '\n'; /* Set target byte, update target. */
               		break;
            	   case('r'):
               		*(dst++) = '\r';	 /* carrage return */
               		break;
            	   case('a'):
               		*(dst++) = '\a';	/* beep */
               		break;
            	   case('\\'):
               		*(dst++) = '\\'; 	/* backslash */
               		break;
            	   default:
               		break;
         	}
         	++src;			/* Next source byte */
      	   }
      	   else
         	*(dst++) = *(src++);	/* Simply copy byte */
   	}
   	*dst = '\0';
}
