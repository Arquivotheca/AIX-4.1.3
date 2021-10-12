static char sccsid[] = "@(#)87	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-936_UTF-8.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:18";
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
/* MODULE NAME:        IBM-937_utf8.c                                      */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert Data Encoding in PC5550                     */
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
#include "pc5550-ucs.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in PC5550 Codeset to               */
/*               utf8 Codeset.                                            */
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
        int             utflen;
        unsigned short  *pc55in, pc55; 
	wchar_t		ucs2in;
	unsigned short  begin, end, middle;

       	if (!cd) {
    		errno = EBADF; return NULL;
	};

	if (!inbuf) return 0;

        /* Assign begin/end address of input buffer to in/e_in pointer */
        e_in = (in = *inbuf) + *inbytesleft; 

        /* Assign begin/end address of output buffer to in/e_in pointer */
        e_out = (out = *outbuf) + *outbytesleft; 

        /* Allocate space for pc55in pointer */
	pc55in = &pc55;           

	/* Initialize return value */
        ret_value = 0;

	/* Setup locale for destination codeset*/
	(void)setlocale(LC_ALL, "UTF2" );

	/* 
	   Use bisect algorithm to search for the location of current code in two-dimension in
	   pc5550-ucs.h.
	   If found it, then assign correspondent pc code to outbuf; otherwise assign default
	   ucs code(0x3000) to outbuf.
	*/

        while (in < e_in) { 
  /******************************
   Convert singal-byte characters
   ******************************/
	    /* Convert singal byte */
	    if(in[0] < 0x7E){
		 if(e_out - out <1){
	      	     errno = E2BIG;
                     ret_value = -1;
                     break;
                 };
                 *out++=*in++;
		 continue;
	    }
	    if(in[0] == 0x7E||in[0] == 0x80||in[0]==0xFD){
	 	 if(e_out - out <2){
	     	      errno = E2BIG;
                      ret_value = -1;
                      break;
                 };
		 if(in[0]==0x7E) utflen = wctomb(out, 0x00AF);
		 if(in[0]==0x80) utflen = wctomb(out, 0x00A3);
		 if(in[0]==0xFD) utflen = wctomb(out, 0x00AC);
		 out += utflen;
		 in  += 1;
		 continue;
	    };
	    if(in[0] == 0xFE||in[0] == 0xFF){
		 if(e_out - out <1){
	      	     errno = E2BIG;
                     ret_value = -1;
                     break;
                 };
	 	 if(in[0]==0xFE)  *out++ = 0x5C;
	 	 if(in[0]==0xFF)  *out++ = 0x7E;
		 in += 1;
		 continue;
            };

            /* Process various error used for double byte*/ 
            if (e_in - in < 2){   
	      	errno = EINVAL;
                ret_value = -1;
                break;
            };

            if (e_out - out < MB_CUR_MAX){ 
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

            /* Begin conversion of HANZI and Graphys */
            *((unsigned char *)pc55in) = in[0];
            *((unsigned char *)pc55in +1) = in[1];

	     begin = BEGIN;
	     end   = END;
	     middle=(BEGIN + END) / 2;

	     while(*pc55in!=pctoucs[middle][0] && begin<=end){
	         if(*pc55in > pctoucs[middle][0])
		     begin = middle + 1;
		 else
		 if(*pc55in < pctoucs[middle][0])
		     end = middle - 1;
		  middle = (begin + end) / 2;
	    }

	    if(*pc55in != pctoucs[middle][0])
	          ucs2in = DEFAULTUCS;
	    else
	          ucs2in = pctoucs[middle][1];
	    
	    utflen = wctomb(out, ucs2in);
	    in  += 2;
	    out += utflen;
		    
	};/* End of While */

        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        return ret_value;

}/* End of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for utf8_IBM-937                   */
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
/* DESCRIPTION : Initialize utf8_IBM-937 Converter-Specific               */
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


