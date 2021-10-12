static char sccsid[] = "@(#)61	1.11  src/bos/usr/bin/errlg/errpt/vpdcfg.c, cmderrlg, bos411, 9439C411e 9/30/94 17:27:59";
/*
 * COMPONENT_NAME: CMDERRLG system error logging and reporting facility
 *
 * FUNCTIONS: 	main
 *              build_vpd
 *		copy_vpd
 *		get_byte
 *		mergetext
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*.
   This file was adapted from the file R2/cmd/diag/lscfg/lscfg.c. Changes
   have been made to build_vpd procedure so that it will accomodate a  
   pointer to the vpd data and output buffer to be printed. Other routines
   originally present in lscfg.c have been omitted because on non-use in 
   this environment. It is important to remember the dependency on NLS.
   The message file lscfg.msg will be used for message but does not
   necessarily hav to be shipped.  This program could use the cmdlscfg.cat
   originally shipped by  the owners of lscfg.c.  One more point, the 
   'key' structure below may need to be updated often, when new descriptors
   are added.  
.*/
#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include <ras.h>
#include <diag/class_def.h>
#include <diag/diagvpd.h>
#include <lscfg_msg.h>

/* Do not use the vpdcfg.cat catalog. Use the lscfg.cat catalog. It contains
   the original information used to describe the descriptor fields in VDP data.
*/
#define MF_LSCFG "lscfg.cat" 
#define DELETED_DEVICE '-'
#define NOT_SUPPORTED  '*'
#define TESTABLE_DEV   '+'
#define MAN_ENTERED	TRUE
#define NOT_MAN_ENTERED	FALSE
#define ODM_FAILURE -1
#define MAX_DEV_TEXT 256
#define DEV_TEXT_FMT "%c %-16.16s  %-16.16s  "
#define VPD_HDR_FMT  "  %-16.16s  %-16.16s  %s\n\t\n"
#define LINE_LENGTH 75  /* change this if output is not acceptable */

/* Globally Defined Variables */
nl_catd 	fdes;

/* function to retrieve binary data from VPD buffer */
int		get_byte();

#define USER_SPECIFIC "Z"

/* Called Functions */
extern 	char	*malloc();
extern 	nl_catd catopen( char *, int );

/*
 * NAME: build_vpd
 *                                                                    
 * FUNCTION: Construct vpd data for each device
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */


int build_vpd(CuVPD_vpd_ibm, CuVPD_vpd_user, vpd_text)
char *CuVPD_vpd_ibm;
char *CuVPD_vpd_user;
char **vpd_text;
{
        int bsz = 2 * VPDSIZE;
        int rc = 0;

	fdes = catopen( MF_LSCFG,NL_CAT_LOCALE);
       
  	/* allocate output buffer */
        *vpd_text = (char *)calloc(1,bsz);

	/* put VPD in single buffer */

	/* IBM's entered data. */
	if (CuVPD_vpd_ibm)  
		copy_vpd(CuVPD_vpd_ibm,vpd_text,&bsz,NOT_MAN_ENTERED);

	/* The users manually entered data. */
	if (strcmp(CuVPD_vpd_user,"NONE") ) {
		copy_vpd(CuVPD_vpd_user,vpd_text,&bsz,MAN_ENTERED);
	}


        return(rc);

}

/*
 * NAME: get_byte
 *                                                                    
 * FUNCTION: Convert binary data from buffer into displayable characters
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE
 */

get_byte( length, bufptr, vpd_data_buffer )
int length;
char *bufptr, *vpd_data_buffer;
{
	int i;

	for ( i=0; i < length; i++ ) {
		sprintf(vpd_data_buffer,"%02X",*bufptr++);
		vpd_data_buffer += 2;
	}
	*vpd_data_buffer = '\0';
}

/*
 * NAME: free_vbuf 
 *                                                                    
 * FUNCTION: free previously allocated VPDBUF pointers
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE
 */

free_vbuf(vbuf)
VPDBUF *vbuf;
{
int	cnt;
	for(cnt=0; cnt < vbuf->entries; cnt++)
		free(vbuf->vdat[cnt]);
}

/*
 * NAME: copy_vpd
 *                                                                    
 * FUNCTION: Construct vpd data for each device
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 0
 */


int copy_vpd( inbuf_ptr, ob_ptr, obsz, me_flg )
char	*inbuf_ptr;
char    **ob_ptr;
int     *obsz; /* size of outbuf_ptr */
int	me_flg;		/* manually entered flag	*/
{
	int		sz_to_be;
	int 	val;
	int 	cnt;
    char    *outbuf_ptr = *ob_ptr + strlen(*ob_ptr);
    char    *obptr = outbuf_ptr; /* beg. of outbuf_ptr[] */
	char 	*vpdvalue, *vpdptr, *tmp, *text;
	char	specific[126];
	VPDBUF	vbuf;
	char 	*vptr;
	static char *defaultvpd[] = {
		"Addressing Field............",
		"Adapter Type................",
		"Adapter Card ID.............",
		"Date Code...................",
		"Device Driver Level.........",
		"Diagnostic Level............",
		"Drawer Level................",
		"Displayable Message.........",
		"Drawer Unit.................",
		"EC Level....................",
		"Feature Code................",
		"FRU Number..................",
		"Loadable Microcode Ptr......",
		"Loadable Microcode Level....",
		"Location....................",
		"Manufacturer................",
		"Network Address.............",
		"Next Adapter VPD Ptr........",
		"Processor Component ID......",
		"Processor Identification....",
		"Part Number.................",
		"ROS Code on Adapter Ptr.....",
		"ROS Level and ID............",
		"System Unit Name............",
		"Read/Write Register Ptr.....",
		"Slot Location...............",
		"Serial Number...............",
		"Size........................",
		"Machine Type and Model......",
		"User Data...................",
		"VPD Extended Data Ptr.......",
		"Device Specific.(%c%c)........"};

	
	vpdvalue = malloc(VPDSIZE);

	parse_vpd(inbuf_ptr,&vbuf,0);
	for(cnt = 0; cnt < vbuf.entries; cnt++) {	
		memset(vpdvalue,0,VPDSIZE);
		vpdptr = vpdvalue;
		vptr = vbuf.vdat[cnt];
		if ( me_flg == TRUE ) {
			*vpdptr++ = 'M';
			*vpdptr++ = 'E';
		}
		strcat(vpdptr,(vptr+4));
		tmp = outbuf_ptr;
		val = get_msgoffset(vptr);

		text = catgets( fdes, SET0,val, defaultvpd[val-1] );
		if ( val == Z0_MSG ) {
			sprintf(specific,text,*(vptr+1),*(vptr+2));
			text = specific;
		}


		sz_to_be = strlen(*ob_ptr) + strlen(text) + 8 +
		           strlen(vpdvalue) + 1;
            if ( sz_to_be > *obsz) {
                    int new_size = ( sz_to_be/VPDSIZE + 1) * VPDSIZE;
                    char *new_ptr;
                    int ofset = abs(outbuf_ptr - *ob_ptr);

                    new_ptr = (char *)realloc((void *)*ob_ptr,new_size);
                    tmp = outbuf_ptr = &new_ptr[ofset];
                    *obptr = outbuf_ptr;
                    *obsz = new_size;
                    *ob_ptr = new_ptr;
            }

		sprintf(outbuf_ptr,"        %s", text);
			
		tmp += strlen(outbuf_ptr);
		mergetext( strlen(outbuf_ptr), tmp, vpdvalue );
		strcat(outbuf_ptr,"\n");
		outbuf_ptr += strlen(outbuf_ptr);
	}
	free_vbuf(&vbuf);
	free(vpdvalue);

	return(0);
}

get_msgoffset(vptr)
char *vptr;
{
	int cnt;
	int stop = Z0_MSG-AD_MSG;
	++vptr;
	for(cnt=0; cnt < stop ; cnt++) {
		if(!strncmp(vptr,vpd_data[cnt].keyword,2)) 
			return(cnt+AD_MSG);
	}
	return(cnt+AD_MSG);
}

/*
 * NAME: mergetext
 *                                                                    
 * FUNCTION: Adjust wraparound of text on screen
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 0
 */

mergetext( string_length, buffer, text )
int	string_length;	/* current length of text in buffer	*/
char	*buffer;	/* buffer to append text to		*/
char 	*text;		/* text to be appended			*/
{
	int	i;
	int 	space_count;
	int 	char_positions;

	/* determine if length of text string will fit on one line */
	char_positions = LINE_LENGTH - string_length;
	if ( char_positions < strlen(text))  {

		/* dont break the line in the middle of a word */
		if(text[char_positions] != ' ' && text[char_positions+1] != ' ')
			while ( --char_positions )
			   	if( text[char_positions] == ' ')
					break;
		if ( char_positions == 0 )
			char_positions = LINE_LENGTH - string_length;

		for ( i = 0; i < char_positions; i++, buffer++, text++ )
		  {
			if ( *text == '\n' )
			       *buffer = ' ';
		          else *buffer = *text;
		  }
		*buffer++ = '\n';
		while ( *text == ' ' )   /* remove any leading blanks */
			text++;
		space_count = string_length;
		while ( space_count-- )
			*buffer++ = ' ';
		mergetext( string_length, buffer, text);
	}
	else
		sprintf(buffer, "%s", text);

	return(0);
} 

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

static real_isprint(c)
char c;
{
	return((c>0x1F && c< 0x7F) ? TRUE: FALSE);
}

/*
 * NAME: null_ptrs 
 *                                                                    
 * FUNCTION: set array of VPDBUF pointers to nulls 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE 
 */

static null_ptrs(vbuf)
VPDBUF *vbuf;
{
int	cnt;

	for(cnt=0; cnt < 64; cnt++)
		vbuf->vdat[cnt] = NULL;
}

/*
 * NAME: bstrncpy
 *                                                                    
 * FUNCTION: replace nonprintable characters with spaces if convert flag true 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE
 */

static bstrncpy(dest,src,len,convert)
char *dest,*src;
int len;
int convert;
{
	while(len-- > 0) {
		*dest++ =(convert) ? ((real_isprint(*src)) ? *src : ' ') : *src;
		++src;
	}
	*dest = '\0';
}

/*
 * NAME: parse_vpd
 *                                                                    
 * FUNCTION: build null terminated ascii strings from vpd data 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE
 */

parse_vpd(vpd,vbuf,cflag)
char *vpd;
VPDBUF *vbuf;
int cflag;
{
int 	(*ptr) ();
int	length;
int 	blength=512;
int 	i;
int	tlen;
char 	key[2];
char 	*dat;
int 	valid_keywd;
int 	cnt=0;

	vbuf->entries=0;
	null_ptrs(vbuf);

	while ( vpd = (char *)memchr(vpd, '*' ,blength)) {
		valid_keywd = FALSE;
		key[0] = toupper( *(vpd+1) );
		key[1] = toupper( *(vpd+2) );
		tlen = *(vpd+3);
		tlen *= 2;

		for ( i=0; i < NVPDKEYS; i++ ) {
			if ( !strncmp(key, vpd_data[i].keyword, 2) )  {
				valid_keywd=TRUE;
				ptr = vpd_data[i].func;
				break;
			}
		}

		if(!valid_keywd) {
			if( *(vpd+tlen) == '*' || *(vpd+tlen) == '\0') { 
				valid_keywd=TRUE;
				ptr = NULL;
			}
		}

		if(valid_keywd) {
			dat = vbuf->vdat[vbuf->entries++] 
			    = (char *)malloc(tlen*2); 

			if(dat == (char *) 0 )
				return(-1);

			*(dat++) = *vpd;
			*(dat++) = *(vpd+1);		
			*(dat++) = *(vpd+2);
			*(dat++) = ' ';
			if(ptr != NULL) 
				(*ptr) (tlen-4,(vpd+4),dat);
			else
				bstrncpy(dat,(vpd+4),tlen-4,cflag);
			blength-=tlen;
		}
			
		else {
			tlen=1;
			--blength;
		}
	
		if(*(vpd+=tlen) == '\0') 
			break;
	}
	return(0);
}
