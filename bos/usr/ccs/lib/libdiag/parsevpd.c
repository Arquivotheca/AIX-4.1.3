static char sccsid[] = "@(#)41	1.4  src/bos/usr/ccs/lib/libdiag/parsevpd.c, libdiag, bos411, 9428A410j 6/28/91 11:44:11";
/*
 * COMPONENT_NAME: (LIBDIAG) Diagnostic Library
 *
 * FUNCTIONS:   parse_vpd
 *		free_vbuf
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <memory.h>
#include <nl_types.h>
#include "diag/diagvpd.h" 

/*  */
/*
 * NAME: real_isprint 
 *                                                                    
 * FUNCTION: test if value of character is printable 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: TRUE if c > 0x1F and < 0x7F 
 *	    FALSE otherwise
 */

int real_isprint(char c)
{
	return((c>0x1F && c< 0x7F) ? TRUE: FALSE);
}

/*  */
/*
 * NAME: null_ptrs 
 *                                                                    
 * FUNCTION: set array of VPDBUF pointers to nulls 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE 
 */

void null_ptrs(VPDBUF *vbuf)
{
int	cnt;

	for(cnt=0; cnt < 64; cnt++)
		vbuf->vdat[cnt] = NULL;
	return;
}

/*  */
/*
 * NAME: bstrncpy
 *                                                                    
 * FUNCTION: replace nonprintable characters with spaces if convert flag true 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE
 */

void bstrncpy(
	char *dest,
	char *src,
	int len,
	int convert)
{
	while(len-- > 0) {
		*dest++ =(convert) ? ((real_isprint(*src)) ? *src : ' ') : *src;
		++src;
	}
	*dest = '\0';
	return;
}

/*  */
/*
 * NAME: parse_vpd
 *                                                                    
 * FUNCTION: build null terminated ascii strings from vpd data 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE
 */
#define CAST int (*)(int, char *, char *)

int parse_vpd(
	char *vpd,
	VPDBUF *vbuf,
	int cflag)
{
int 	(*ptr)(int, char *, char *);
int	length;
int 	blength=512;
int 	i;
int	tlen;
char 	key[2];
char 	*dat;
int 	valid_keywd;
int 	cnt=0;

	vbuf->entries = 0;
	null_ptrs(vbuf);

	while (vpd = (char *)memchr(vpd, '*' ,blength)) {
		valid_keywd = FALSE;
		key[0] = toupper( *(vpd+1) );
		key[1] = toupper( *(vpd+2) );
		tlen = *(vpd+3);
		tlen *= 2;

		for (i=0; i < NVPDKEYS; i++) {
			if (!strncmp(key, vpd_data[i].keyword, 2))  {
				valid_keywd = TRUE;
				ptr = (CAST )vpd_data[i].func;
				break;
			}
		}

		if (!valid_keywd) {
			if (*(vpd+tlen) == '*' || *(vpd+tlen) == '\0') { 
				valid_keywd = TRUE;
				ptr = (CAST )NULL;
			}
		}

		if (valid_keywd) {
			dat = (char *)malloc(tlen*2); 
			if (dat == (char *)NULL) return(-1);
			else vbuf->vdat[vbuf->entries++] = dat;

			*(dat++) = *vpd;
			*(dat++) = *(vpd+1);		
			*(dat++) = *(vpd+2);
			*(dat++) = ' ';
			if (ptr != (CAST )NULL) 
				(*ptr)(tlen-4, (vpd+4), dat);
			else
				bstrncpy(dat, (vpd+4), tlen-4, cflag);
			blength -= tlen;
		}
			
		else {
			tlen = 1;
			--blength;
		}
	
		if (*(vpd+=tlen) == '\0') 
			break;
	}
	return(0);
}

/*  */
/*
 * NAME: free_vbuf 
 *                                                                    
 * FUNCTION: free previously allocated VPDBUF pointers
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE
 */

void free_vbuf(VPDBUF *vbuf)
{
int	cnt;
	for(cnt=0; cnt < vbuf->entries; cnt++)
		free(vbuf->vdat[cnt]);
	return;
}


/**/
/*
 * NAME: get_byte
 *                                                                    
 * FUNCTION: Convert binary data from buffer into displayable characters
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 0 
 *	   
 */

int get_byte(
	int len,
	char *inptr,
	char *outptr)
{
	int i;
	
	for(i=0; i < len; i++) {
		sprintf(outptr,"%02X",*inptr++);
		outptr += 2;
	}
	*outptr = '\0';
	return 0;
}

