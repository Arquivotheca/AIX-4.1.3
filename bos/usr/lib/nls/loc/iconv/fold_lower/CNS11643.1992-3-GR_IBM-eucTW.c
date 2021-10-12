static char sccsid[] = "@(#)50  1.4  src/bos/usr/lib/nls/loc/iconv/fold_lower/CNS11643.1992-3-GR_IBM-eucTW.c, cmdiconv, bos411, 9428A410j 6/1/94 02:51:28";
/*
 *   COMPONENT_NAME: CMDICONV
 *
 *   FUNCTIONS: _iconv_close
 *              _iconv_exec
 *              init
 *              instantiate
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <iconv.h>
#include "fold_lower.h"

static  size_t  _iconv_exec (_LC_fold_lower_iconv_t *cd,
        uchar_t** inbuf,  size_t *inbytesleft,
        uchar_t** outbuf, size_t *outbytesleft)
{
        uchar_t         *in, *out, *e_in, *e_out;
        int             err_flag=FALSE;


        if ((cd == NULL) || (cd == -1)) {
                errno = EBADF; return -1;
        }
        if (inbuf == NULL) return 0;
        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
/*      while (1) {                                 @V42 */
        while (in < e_in) {                      /* @V42 */
                if (e_in - in < 2) {
                        if (e_in - in == 1) {
                                if ((in[0] < 0xa1 || 0xfe < in[0]) &&
                                    (in[0] < 0x21 || 0x7e < in[0])) {
                                        errno = EILSEQ; err_flag = TRUE;
                                }
                                else {
                                        errno = EINVAL; err_flag = TRUE;
                                }
                        }
                        break;
                }
                if ((in[0] < 0xa1 || in[1] < 0xa1 || 0xfe < in[0] || 0xfe < in[1]) &&
                    (in[0] < 0x21 || in[1] < 0x21 || 0x7e < in[0] || 0x7e < in[1])) {
                        errno = EILSEQ; err_flag = TRUE;
                        break;
                }
                if (e_out - out < 4) {
                        errno = E2BIG; err_flag = TRUE;
                        break;
                }
                out[0] = 0x8E;
                out[1] = 0xA3;
                out[2] = (in[0] | 0x80);
                out[3] = (in[1] | 0x80);
                in += 2;
                out += 4;
        }   /* while */
        *inbuf = in;
        *outbuf = out;
        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        if (err_flag) return  -1;
        else return 0;
}

static  int     _iconv_close (iconv_t cd)
{
        if ((cd != NULL) && (cd != -1)) {
                free(cd);
                return 0;
        }
        else {
                errno = EBADF;
                return -1;
        }
}

static  _LC_fold_lower_iconv_t  *init (
        _LC_core_iconv_t        *core_cd,
        char                    *toname,
        char                    *fromname)
{
        _LC_fold_lower_iconv_t  *cd;

        if ((cd = (_LC_fold_lower_iconv_t *) malloc (
                sizeof (_LC_fold_lower_iconv_t))) == NULL)
                return (_LC_fold_lower_iconv_t*)-1;
        cd->core = *core_cd;
        return cd;
}

_LC_core_iconv_t        *instantiate (void)
{
        static _LC_core_iconv_t cd;

        cd.hdr.__magic   = _LC_MAGIC;
        cd.hdr.__version = _LC_VERSION;
        cd.hdr.__type_id = _LC_ICONV;
        cd.hdr.__size    = sizeof (_LC_core_iconv_t);
        cd.init          = (_LC_core_iconv_t*(*)())init;
        cd.exec          = (size_t(*)())_iconv_exec;
        cd.close         = (int(*)())_iconv_close;
        return &cd;
}
