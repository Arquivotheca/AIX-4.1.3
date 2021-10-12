static char sccsid[] = "@(#)45	1.1  src/bos/usr/lib/nls/loc/iconv/fold_lower/UTF-8_GB2312.1980-0-GR.c, ils-zh_CN, bos41B, 9504A 12/19/94 15:17:03";
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
/* MODULE NAME:        UTF8_IBM-eucCN.c                                    */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert Data Encoding in UTF to IBM-eucCN           */
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
#include "fold_lower.h"
#include  <limits.h>
#include "ucs-euccn.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in Unicode  Codeset to             */
/*               IBM-eucTW Codeset.                                       */
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
        unsigned short  *outtmp, outbuf1;
	wchar_t         *ucs2in, ucs2;
	int  inlen, start, middle, end, indx;

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

	(void)setlocale(LC_ALL, "zh_CN.utf-8" );

        while (in < e_in) {

	    if(*in < 0x80){
	        errno = EILSEQ;
                ret_value = -1;
                break;
	    };

            if (e_in - in <  MB_CUR_MAX){   
	       	errno = EINVAL;
                ret_value = -1;
                break;
            };

            if (e_out - out < 2) { 
	        errno = E2BIG;
                ret_value = -1;
                break;
            };

	    inlen = mbtowc(ucs2in, in, MB_CUR_MAX);
/* 
 Begin conversion 
*/
	    if(wcsid(*ucs2in) == 1){
	        start = HEADER;
	        end   = TRAILER; 
	        middle = (end + start)/2; 
	        while((*ucs2in!=ucstogb[middle][0])&&(start<=end)){
	    	    if(*ucs2in > ucstogb[middle][0])
		        start = middle + 1;
	            else
		    if(*ucs2in < ucstogb[middle][0]) 
		        end = middle-1;
	            middle = (end + start)/2;
	        };/* End of while for seraching */

	        if(*ucs2in == ucstogb[middle][0]) 
	    	    *(unsigned short*)outtmp = ucstogb[middle][1];
    	     }else 
		if(wcsid(*ucs2in)==4||wcsid(*ucs2in)==5){
		    *outtmp = 0xa1f6;
	     }else{
	         errno = EILSEQ;
                 ret_value = -1;
                 break;
	     };
		out[0] = *(unsigned char*)outtmp;
		out[1] = *((unsigned char*)outtmp + 1);
		out += 2;
		in  += inlen;

    };/* End of While */

   *inbytesleft = e_in - in;
   *outbytesleft = e_out - out;
   *inbuf = in;
   *outbuf = out;
   return ret_value;

}/* End of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for UTF8_IBM-eucCN                 */
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
/* DESCRIPTION : Initialize UTF8_IBM-eucCN  Converter-Specific            */
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


