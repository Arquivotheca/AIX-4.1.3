static char sccsid[] = "@(#)12	1.1  src/bos/usr/lib/nls/loc/iconv/fold_lower/CNS11643.1986-1_UTF-8.c, ils-zh_CN, bos41B, 9504A 12/19/94 15:16:06";
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
/* MODULE NAME:        CNS11643.1986-1_utf8.c                              */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert Data Encoding in UTF to CNS11643.1986-1     */
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
#include "cns1-ucs.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in Unicode  Codeset to             */
/*               CNS11643.1986-1 Codeset.                                 */
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
        unsigned char   *in, *out, *outtmp;      /* Point to in/out buffers */
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             ret_value;      /* Hold the return value */
	int  		utflen;
	wchar_t         ucs2in;
	unsigned short  *cns2in, cns2;
	short		middle, begin, end;

        if (!cd) {
   	    errno = EBADF; 
	    return NULL;
	};

	if (!inbuf) return 0;

        /* Assign begin/end address of input buffer to in/e_in pointer */
        e_in = (in = *inbuf) + *inbytesleft;  

        /* Assign begin/end address of output buffer to in/e_in pointer */
        e_out = (out = *outbuf) + *outbytesleft;

	cns2in = &cns2;

       	ret_value = 0;

	(void)setlocale(LC_ALL, "zh_CN.utf-8" );
       	while (in < e_in) {
	    /*if(in[0] == 0x0a || in[0] == 0x00){
                if (e_out - out < 1) { 
	       	    errno = E2BIG;
                    ret_value = -1;
                    break;
		}
	        *out++ = *in++;
		if(e_in <= in)
		   break;
	        continue;
	    };*/

            if (e_in - in < 2) {
                 if (e_in - in == 1) {
                      if (in[0] < 0x21 || 0x7e < in[0]){
                           errno = EILSEQ;
			   ret_value = -1;
                       }else{
                           errno = EINVAL; ret_value = -1;
                       }
                 }
                 break;
            }

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

            /*if (e_in - in < 2){
	       	errno = EINVAL;
                ret_value = -1;
                break;
            };*/

            if (e_out - out < MB_CUR_MAX) { 
	       	errno = E2BIG;
                ret_value = -1;
                break;
            };

	    *(unsigned char *)cns2in = in[0];
	    *((unsigned char *)cns2in + 1) = in[1];

	    begin = BEGIN;
	    end   = END;
	    middle= (begin + end) / 2;
	    while(*cns2in != cns1toucs[middle][0] && begin <= end){
		if(*cns2in > cns1toucs[middle][0])
		    begin = middle + 1;
		if(*cns2in < cns1toucs[middle][0])
		    end   = middle - 1;
	        middle = (begin + end ) / 2;
	    }
	    if(*cns2in == cns1toucs[middle][0])
		ucs2in = cns1toucs[middle][1];
	    else
		ucs2in = DEFAULTUCS;
	    
	    utflen = wctomb(out, ucs2in);
	    in += 2;
	    out+=utflen;
		
	};/* End of While */

        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        return ret_value;

}/* End of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for CNS11643.1986-1_utf8           */
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
/* DESCRIPTION : Initialize CNS11643.1986-1_utf8  Converter-Specific      */
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


