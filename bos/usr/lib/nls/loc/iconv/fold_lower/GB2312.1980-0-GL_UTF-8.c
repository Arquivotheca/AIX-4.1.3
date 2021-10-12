static char sccsid[] = "@(#)22	1.1  src/bos/usr/lib/nls/loc/iconv/fold_lower/GB2312.1980-0-GL_UTF-8.c, ils-zh_CN, bos41B, 9504A 12/19/94 15:16:22";
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
/* MODULE NAME:        GB2312.1980-0_utf8.c                                */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert EUC Encoding to UTF8(Transform of Unicode)  */
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
#include "fold_lower.h"
#include "gb-ucs.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in GB2312.1980-0 Codeset to        */
/*               utf8(Transform of Unicode)                               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : ret_value                                                */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int _iconv_exec (_LC_fold_lower_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out;      /* Point to in/out buffers */
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             ret_value;      /* Hold the return value */
	wchar_t		ucs;
	int		utflen;
        unsigned short  euc2, *euc2in;
	unsigned short  indx;           /* Used to locate the positon in a array */
	unsigned short  *outtmp;
	short  		start, middle, end;

        if (!cd) {
            errno = EBADF; return NULL;
        };

        if (!inbuf) return 0;
 /*******************
  Initialize Pointers
  *******************/
 	e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;

	euc2in = &euc2;

        ret_value = 0;

	(void)setlocale(LC_ALL, "zh_CN.utf-8" );
        while (in < e_in ) { 
	    /*if(*in == 0x0a||*in == 0x00) {
               if(e_out - out < 1) {   
	       	  errno = E2BIG;
                  ret_value = -1;
                  break;
               };
               *out++ = *in++;
               if(in>=e_in) break;
	       continue;
            }*/

 	    /* Handle Errors */
            if (e_in - in < 2) {
                 if (e_in - in == 1) {
                      if (in[0] < 0x21 || 0x7e < in[0]){
                             errno = EILSEQ; 
			     ret_value = -1;
                      }else{
                             errno = EINVAL; 
			     ret_value = -1;
                      }
                 }
                 break;
           }

            if(e_out - out < MB_CUR_MAX) {   
	       	  errno = E2BIG;
                  ret_value = -1;
                  break;
            };

            if (in[0] < 0x21 || in[0] == 0x7f) {
                errno = EILSEQ;
                ret_value = -1;
                break;
            };

            if (in[1] < 0x21 || in[1] == 0x7f) {
                errno = EILSEQ;
                ret_value = -1;
                break;
            };

 	    /* Obtain One Input Word, Either Hanzi or Graphic */
            *((unsigned char *)euc2in) = in[0];
            *((unsigned char *)euc2in +1) = in[1];

            start  = BEGIN;
            end    = END;
            middle = ( start + end ) / 2;
            while( *euc2in != gbtoucs[middle][0] && start <= end ) {
                if( *euc2in < gbtoucs[middle][0] ) 
                     end = middle - 1;
	        else
                if( *euc2in > gbtoucs[middle][0] ) 
                     start = middle + 1;
                middle = ( start + end ) / 2;
           };

	   if(*euc2in == gbtoucs[middle][0])
                ucs = gbtoucs[middle][1];
 /*******************************************
  Locate User-Defined Hanzi and Graphic Words
  *******************************************/
           else if( in[0] >= 0x22 && in[0] <= 0x29 ) {
                start = UDC_HEADER;
                end = UDC_TRAILER;
                middle = ( start + end ) / 2;
                while( *euc2in != udc_index[middle] && start <= end ) {
                    if( *euc2in < udc_index[middle] ) 
                         end = middle - 1;
	            else
                    if( *euc2in > udc_index[middle] ) 
                         start = middle + 1;
                    middle = ( start + end ) / 2;
               };
               if(start > end) {
                   errno = EILSEQ;
		   ret_value = -1;
		   break;
               }
               else {
                   ucs = (0xe000 + (unsigned short)middle);
               }
	    } else if( in[0] >= 0x2a && in[0] <= 0x2f ) {
	         indx = (unsigned short)((in[0] - 0x2a)*94+(in[1] - 0x21));
                 ucs = (0xe0a4 + (unsigned short)indx);
	    } else if( in[0] == 0x57 ) {
	         indx = (unsigned short)((in[0] - 0x57)*94+(in[1] - 0x7a));
                 ucs = (0xe2d8 + (unsigned short)indx);
	    } else if(in[0] >= 0x78 && in[0] <= 0x7e) {
	         indx = (unsigned short)((in[0] - 0x78)*94+(in[1] - 0x21));
                 ucs = (0xe2dd + (unsigned short)indx);
	    } else {
	         errno = EILSEQ;
                 ret_value = -1;
                 break;
            };
  /***************** 
   Begin to convert all characters located before 
   *****************/
	    utflen = wctomb(out, ucs);
	    in += 2;
	    out+=utflen; 
       }; /* The end of while */
       *inbytesleft = e_in - in;
       *outbytesleft = e_out - out;
       *inbuf = in;
       *outbuf = out;
       return ret_value;
}

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for GB2312.1980-0_UTF8             */
/*               Converter Descriptor                                     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd                                                       */
/* OUTPUT      : none                                                     */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static void     _iconv_close(iconv_t cd)
{
        if (cd){
              free(cd);
	}
        else
                errno = EBADF;
}

/**************************************************************************/
/* FUNCTION    : _iconv_init                                              */
/* DESCRIPTION : Initialize GB2312.1980-0_UTF8 Converter-Specific         */
/*               Descriptor                                               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : core_cd, toname, fromname                                */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static _LC_fold_lower_iconv_t       *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_fold_lower_iconv_t      *cd;

        if (!(cd = (_LC_fold_lower_iconv_t *) malloc(sizeof(_LC_fold_lower_iconv_t))))
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
