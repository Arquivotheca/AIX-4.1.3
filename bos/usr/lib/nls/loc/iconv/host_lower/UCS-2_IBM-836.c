static char sccsid[] = "@(#)98	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/UCS-2_IBM-836.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:40";
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
#include <stdio.h>
#include "iconv.h"
#include "iconvP.h"
#include "host_lower.h"
#include "ucs-hosts.h"

static int _iconv_exec (_LC_host_lower_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out;      /* pointer to in/out buffers */
        unsigned char   *e_in, *e_out;  /* point the end of in/out buffers */
        int             ret_value;      /* hold the return value */

        if (!cd) {
                errno = EBADF; return NULL;
        }
        if (!inbuf) return 0;
        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
        ret_value = 0;
		
        while (in < e_in) {

	   if(e_out - out < 2){
	        errno = E2BIG;
                ret_value = -1;
                break;
	   }

	   if (in[0] != 0x0){
	     	errno = EILSEQ;
               	ret_value = -1;
              	break;
	   };
      
           if((in[0]==0)&&(in[1]<=0xaf)&&(in[1]!=0xa8||in[1]!=0xa7||in[1]!=0xa4)){
               out[0] = table[in[1]];
	       in += 2;
	       out += 1;
	   }else{
	     	errno = EILSEQ;
               	ret_value = -1;
              	break;
	   };
	}
        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        return ret_value;
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
        cd.hdr.__version = _LC_VERSION;
        cd.hdr.__type_id = _LC_ICONV;
        cd.hdr.__size    = sizeof (_LC_core_iconv_t);
        cd.init          = (_LC_core_iconv_t*(*)())_iconv_init;
        cd.exec          = (size_t(*)())_iconv_exec;
        cd.close         = (int(*)())_iconv_close;
        return &cd;
}
