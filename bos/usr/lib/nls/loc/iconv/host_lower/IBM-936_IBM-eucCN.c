static char sccsid[] = "@(#)86	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-936_IBM-eucCN.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:16";
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
/* MODULE NAME:        IBM-936_IBM-eucCN.c                                 */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert Data Encoding in PC550                      */
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
#include "iconv.h"
#include "iconvP.h"
#include "host_lower.h"
#include "55-euccn.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in PC550 Codeset to                */
/*               IBM-eucCN Codeset.                                       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : ret_value                                                */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int _iconv_exec (_LC_host_lower_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out;      /* Point to in/out buffers */
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             ret_value;      /* Hold the return value */
        unsigned short  *pc55in, *euctmp;
        unsigned short  pc55, euc;
	unsigned short  indx;       /* Used to locate the positon in a array */
	short  		header, middle, trailer;

 /*
  Initialize specific converter descriptor failed
 */
       	if (!cd) {
    	    errno = EBADF; 
	    return NULL;
	};

	if (!inbuf) return 0;
 /* 
  Assign begin/end address of input buffer to in/e_in pointer 
 */
        e_in = (in = *inbuf) + *inbytesleft; 
 /* 
  Assign begin/end address of output buffer to in/e_in pointer 
 */
        e_out = (out = *outbuf) + *outbytesleft; 

	pc55in = &pc55; 
	euctmp = &euc;

        ret_value = 0;

        while (in < e_in) { 
 /*
   Process Singal byte characters
 */
	    if(in[0] <= 0x7f){
		if(e_out - out < 1){
                    errno = E2BIG;
                    ret_value = -1;
                    break;
		}
                *out++ = *in++;
                if(in >= e_in) break;
		continue;
            };
 /*
  Various error process 
 */
            if (e_in - in < 2){   
	      	errno = EINVAL;
                ret_value = -1;
                break;
            };

            if (e_out - out < 2){ 
	      	errno = E2BIG;
                ret_value = -1;
                break;
            };

            if(in[0] < 0x81 || in[0] > 0xfa){
	      	errno = EILSEQ;
                ret_value = -1;
                break;
            }

            if(!(in[1] >= 0x40 && in[1] <=0x7e) && 
	       !( in[1] >=0x80 && in[1] <=0xfc )){
                errno = EILSEQ;
                ret_value = -1;
                break;
            }
  /*
   Process Chinese and Graphic characters
  */
            *((unsigned char *)pc55in) = in[0];
            *((unsigned char *)pc55in +1) = in[1];

	    if(*pc55in >= 0x8140 && *pc55in <= 0xfa7b){
	        header  = HEADER;
	        trailer = TRAILER;
	        middle  = (header + trailer) / 2;
	        while(*pc55in!=pctoeuccn[middle][0] && header<=trailer){
		    if(*pc55in < pctoeuccn[middle][0]){
		        trailer = middle - 1;
		    }
		    if(*pc55in > pctoeuccn[middle][0]){
		        header = middle + 1;
		    }
		    middle = (header + trailer) / 2;
		}

		if(*pc55in == pctoeuccn[middle][0])
		    *euctmp = pctoeuccn[middle][1];
		else
		    *euctmp = DEFAULTEUC;

		out[0] = *(unsigned char*)euctmp;
		out[1] = *((unsigned char*)euctmp + 1);
		out += 2;
		in  += 2;
	    }else{
	      	errno = EILSEQ;
                ret_value = -1;
                break;
            }
		    
	};/* End of While */

        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        return ret_value;

}/* End of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for IBM-eucCN_IBM-936              */
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
/* DESCRIPTION : Initialize IBM-eucCN_IBM-936 Converter-Specific          */
/*               Descriptor                                               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : core_cd, toname, fromname                                */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static _LC_host_lower_iconv_t       *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_host_lower_iconv_t      *cd;

        if (!(cd = (_LC_host_lower_iconv_t *) malloc(sizeof(_LC_host_lower_iconv_t))))
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


