static char sccsid[] = "@(#)49	1.1  src/bos/usr/lib/nls/loc/iconv/PRC.LocalSolution/IBM-836_UTF-8.c, ils-zh_CN, bos41J, 9509A_all 2/19/95 23:51:55";
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
#include <stdlib.h>
#include <iconv.h>
#include <iconvP.h>
#include "host_lower.h"
#include "ebc-asc.h"


static int _iconv_exec (_LC_host_lower_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out;      /* pointer to in/out buffers */
        unsigned char   *e_in, *e_out;  /* point the end of in/out buffers */
        int             err_flag = FALSE;      /* hold the return value */

        if (!cd) {
                errno = EBADF; return NULL;
        }
        if (!inbuf) return 0;
        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
        while (in < e_in) {
           if( in[0] == 0x0e ) {
                errno = EILSEQ;
                err_flag = TRUE;
                break;
           }
           if (in[0] >= 0xff) {
                errno = EILSEQ;
                err_flag = TRUE;
                break;
           }
           if (out >= e_out) {
                errno = E2BIG;
                err_flag = TRUE;
                break;
           }
	   if(table[*in]==0xaf||table[*in]==0xa3||table[*in]==0xac||table[*in]==0xa5){
	        if(table[*in]==0xaf){
	             out[0] = 0xc2;
	             out[1] = 0xaf;
		     out += 2;
		}else
		if(table[*in]==0xa3){
		     out[0] = 0xc2;
		     out[1] = 0xa3;
		     out += 2;
		}else
		if(table[*in]==0xac){
		     out[0] = 0xc2;
		     out[1] = 0xac;
		     out += 2;
		}else
		if(table[*in]==0xa5){
		     out[0] = 0xc2;
		     out[1] = 0xa5;
		     out += 2;
		}
           }else
                *out++ = table[*in];
	   in += 1;
        }
        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        if ( err_flag ) return -1;
	else return 0;
}

static void     _iconv_close(iconv_t cd)
{
        if (cd)
                free(cd);
        else
                errno = EBADF;
}

static _LC_host_lower_iconv_t   *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_host_lower_iconv_t  *cd;

        if (!(cd = (_LC_host_lower_iconv_t *) malloc(sizeof(_LC_host_lower_iconv_t))))
                return (NULL);
        cd->core = *core_cd;
        return cd;
}

_LC_core_iconv_t        *instantiate(void)
{
        static _LC_core_iconv_t cd;

        cd.hdr.__magic   = _LC_MAGIC;
        cd.hdr.__versi

