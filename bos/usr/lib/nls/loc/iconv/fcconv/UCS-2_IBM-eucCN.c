static char sccsid[] = "@(#)87	1.1  src/bos/usr/lib/nls/loc/iconv/fcconv/UCS-2_IBM-eucCN.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:49:36";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: _iconv_close
 *		_iconv_exec
 *		_iconv_init
 *		instantiate
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************* START OF MODULE SPECIFICATION ***********************/
/*                                                                         */
/* MODULE NAME:        UCS-2_IBM-eucCN                                     */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert Data Encoding in Unicode to IBM-eucCN       */
/*                                                                         */
/* FUNCTION:           _iconv_exec         : Convert Data Encoding in One  */
/*                                           Codeset to Another            */
/*                                                                         */
/*                     _iconv_close        : Free Memory Allocated for     */
/*                                           Converter early               */
/*                                                                         */
/*                     _iconv_init         : Allocate Memory for Specific  */
/*                                           Converter Descriptor, and     */
/*                                           Initialize It                 */
/*                                                                         */
/*                     instantiate         : Initialize Core Converter     */
/*                                           Descriptor                    */
/*                                                                         */
/* MODULE TYPE:        C                                                   */
/*                                                                         */
/* COMPILER:           AIX C                                               */
/*                                                                         */
/* AUTHOR:             Dong Meiting, Wang Ying                             */
/*                                                                         */
/* STATUS:             Simplified Chinese Iconv Version 1.0                */
/*                                                                         */
/* CHANGE ACTIVITY:    None                                                */
/*                                                                         */
/********************* END OF SPECIFICATION ********************************/

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "iconv.h"
#include "iconvP.h"
#include "fcconv.h"
#include  <limits.h>
#include "ucs-euccn.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in Unicode  Codeset to             */
/*               IBM-eucCN Codeset.                                       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : ret_value                                                */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int _iconv_exec (_LC_fcconv_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out;      /* Point to in/out buffers */
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             ret_value;      /* Hold the return value */
        unsigned short  *outtmp, outbuf1;
	unsigned short  *ucs2in, ucs2;
	short 		indx, start, middle, end; /* Index variable */

 /*
 Initialize specific converter descriptor failed
 */
        if (!cd) {
    		errno = EBADF; 
		return NULL;
	};

	if (!inbuf) return 0;

        /* Assign begin/end address of input buffer to in/e_in pointer */
       	e_in = (in = *inbuf) + *inbytesleft; 

        /* Assign begin/end address of output buffer to in/e_in pointer */
       	e_out = (out = *outbuf) + *outbytesleft; 

       	ret_value = 0;

	ucs2in = &ucs2;
	outtmp = &outbuf1;

       	while (in < e_in) {
 /*
  Convert character in ISO8859-1
 */
	    while((in[0] == 0x0) && (in[1] <= 0xaf)) {
		if(in[1] == 0xa5) 
		    out[0] = 0x5c;
		else if(in[0] == 0xaf)
		    out[0] = 0x7e;
		else if(in[0] == 0xa3 || in[0] == 0x5c || in[0] == 0x7e)
		    out[0] = 0x20;
		else
       		    out[0] = in[1];
       		in  += 2;
       		out += 1;
      		if(in >= e_in) break;
            };
 /*
   Process input/output buffer errors
 */
            if (e_in - in < 2){   
	       	errno = EINVAL;
              	ret_value = -1;
              	break;
            };

            if (e_out - out < 2) { 
	       	errno = E2BIG;
               	ret_value = -1;
                break;
            };
/*
  Process Chinese and glaphic words one by one
*/

	    *(unsigned char*)ucs2in = in[0];
	    *((unsigned char*)ucs2in + 1) = in[1];

/*
 Chinese and Graphic(including 31 IBM-selected characters, but exclude User
 defined characters
*/
	    if((*ucs2in < 0xe000) || (*ucs2in > 0xe56e)){
	        start = HEADER;
	        end = TRAILER; 
	        middle = (end + start)/2; 
	        while((*ucs2in!=ucstocn[middle][0])&&(start< end)){
	    	    if(*ucs2in > ucstocn[middle][0])
		        start = middle + 1;
		    else
		    if(*ucs2in < ucstocn[middle][0]) 
		        end = middle-1;
		    middle = (end + start)/2;
	        };/* End of while for seraching */

	        if(*ucs2in == ucstocn[middle][0]) 
		    *outtmp = ucstocn[middle][1];
	        else
		    *outtmp = DEFAULTCN;

		out[0] = *(unsigned char*)outtmp;
		out[1] = *((unsigned char*)outtmp+1);
		out += 2;
		in  += 2;
	     }else
/* 
 User defined Characters 
*/
	        if((*ucs2in >= 0xe000) && (*ucs2in <= 0xe56e)) {
  	            if ((*ucs2in >= 0xe000) && (*ucs2in <= 0xe0a3)) {
	 	        indx = (unsigned short)(*ucs2in - 0xe000);
		        *outtmp = udc_index[indx]; 
    		    }
    		    else if ((*ucs2in >= 0xe0a4) && (*ucs2in <= 0xe2d7)) {
   		        *(unsigned char*)outtmp=(char)((*ucs2in-0xe0a4)/ 94 )+0x00aa;
        	        *((unsigned char*)outtmp+1)=(char)((*ucs2in-0xe0a4) %94)+0x00a1;
    	            }
    	    	    else if ((*ucs2in >= 0xe2d8) && (*ucs2in <= 0xe2dc)) {
  		        *outtmp=0xd7fa+(*ucs2in-0xe2d8);
    	            }
    		    else if ((*ucs2in >= 0xe2dd) && (*ucs2in <= 0xe56e)) {
   		        *(unsigned char*)outtmp=(char)((*ucs2in-0xe2dd)/94 )+0x00f8;
        	        *((unsigned char*)outtmp+1)=(char)((*ucs2in-0xe2dd) %94)+0x00a1;
    		    }
	            out[0] = *(unsigned char*)outtmp;
	            out[1] = *((unsigned char*)outtmp + 1);
	            out += 2;
	            in  += 2;
	        } 
/*
 Invalid characters
*/
		else{
	     	    errno = EILSEQ;
               	    ret_value = -1;
              	    break;
	        };

	};/* End of While */

        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        return ret_value;

}/* End of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for UCS-2_IBM-eucCN                */
/*               Converter Descriptor                                     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd                                                       */
/* OUTPUT      : none                                                     */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static void     _iconv_close(iconv_t cd)
{
        if (cd)
                free(cd);
        else
                errno = EBADF;
}

/**************************************************************************/
/* FUNCTION    : _iconv_init                                              */
/* DESCRIPTION : Initialize UCS-2_IBM-eucCN  Converter-Specific           */
/*               Descriptor                                               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : core_cd, toname, fromname                                */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static _LC_fcconv_iconv_t       *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_fcconv_iconv_t      *cd;

        if (!(cd = (_LC_fcconv_iconv_t *) malloc(sizeof(_LC_fcconv_iconv_t))))
                return (NULL);
        cd->core = *core_cd;
        return cd;
}

/**************************************************************************/
/* FUNCTION    : instantiate                                              */
/* DESCRIPTION : Initialize Core Converter Descriptor , It is program     */
/*               Entry Point                                              */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : void                                                     */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
_LC_core_iconv_t        *instantiate(void)
{
        static _LC_core_iconv_t cd;

        cd.hdr.__magic   = _LC_MAGIC;
        cd.hdr.__version = _LC_VERSION;
        cd.hdr.__type_id = _LC_ICONV;
        cd.hdr.__size    = sizeof (_LC_core_iconv_t);
        cd.init          = (_LC_core_iconv_t*(*)())_iconv_init;
        cd.exec          = (size_t(*)())_iconv_exec;
        cd.close         = (int(*)())_iconv_close;
        return &cd;
}


