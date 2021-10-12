static char sccsid[] = "@(#)12  1.9  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-938_IBM-835.c, cmdiconv, bos411, 9428A410j 4/11/94 17:44:47";
/*
 *   COMPONENT_NAME:    CMDICONV
 *
 *   FUNCTIONS:         _iconv_exec
 *                      _iconv_close
 *                      init
 *                      instantiate
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <stdio.h>
#include <iconv.h>
#include <iconvP.h>
#include "host_lower.h"
#include "ebc-55.h"
#define DEFAULTHO1   0xfe
#define DEFAULTHO2   0xfd

static size_t      _iconv_exec(_LC_host_lower_iconv_t *cd,
                                uchar_t** inbuf, size_t *inbytesleft,
                                uchar_t** outbuf, size_t *outbytesleft)
{
        uchar_t   *in, *out;      /* pointer to in/out buffers */
        uchar_t   *e_in, *e_out;  /* point the end of in/out buffers */
        ushort_t  pccode;
        int             i;
        int             err_flag=FALSE;
        unsigned        tmp;


        if ((cd == NULL) || (cd == -1)) {
                errno = EBADF; return -1;
        }
        if (inbuf == NULL) return 0;
        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
        while (in < e_in) {
/* === Below : modify sequence by Kathy === */
                if (e_in - in < 2 )
                {
                        err_flag = TRUE;
                        errno = EINVAL;
                        if( !( in[0] >=0x81 && in[0] <=0xfc ) ) /* skip all SBCS */
                           errno = EILSEQ;
                        break;
                }
                if( !( in[0] >=0x81 && in[0] <=0xfc ) )
                {
                   errno = EILSEQ;
                   err_flag = TRUE;
                   break;
                }
                else  if( !( in[1] >=0x40 && in[1] <=0x7e ) &&
                          !( in[1] >=0x80 && in[1] <=0xfc )    )
                {
                   errno = EILSEQ;
                   err_flag = TRUE;
                   break;
                }
                if (e_out - out < 2)
                {
                        errno = E2BIG;
                        err_flag = TRUE;
                        break;
                }

                if (in[0] == 0x81 && in[1]== 0x40 )
                {
                        out[0] = 0x40;
                        out[1] = 0x40;
                }
               else if (in[0] >= 0x81 && in[0] <=0x8B )
                {
                        /* use table host<-> pc55 */
                        for(i=0;i<=1004;i++)
                         {
                            tmp=IBM3[i][0];
                            if (( in[0] == (tmp >> 8)) &&
                                ( in[1] == (tmp & 0x00ff)) )
                              {
                                tmp=IBM3[i][1];
                                out[0] = tmp >> 8;
                                out[1] = tmp & 0x00ff;
                                break;
                              }
                         }
                  }
                else if (in[0] >=0x8C && in[0] <=0xA8 )
                  {

                        out[0] = in[0] - 0x40;
                        out[1] = in[1] + 0x01;
                      if (  in[0] == 0xa8  && in[1] > 0xCA  )
                       {
                        out[0]= DEFAULTHO1;
                        out[1]= DEFAULTHO2;
                       }
                 }
                else if (in[0] >=0xA9 && in[0] <=0xD1 )
                {
                        out[0] = in[0] -0x40;
                        out[1] = in[1] +0x01;
                        if ( in[0] == 0xd1 && in[1] > 0xC6 )
                        {
                        out[0]= DEFAULTHO1;
                        out[1]= DEFAULTHO2;
                        }
                }
                else if (in[0] >=0xdb && in[0] <=0xfb )
                {
                        out[0] = in[0] - 0x19;
                        out[1] = in[1] + 0x01;
                        if ( in[0] == 0xfb && in[1] > 0xFC )
                        {
                        out[0]= DEFAULTHO1;
                        out[1]= DEFAULTHO2;
                        }
                }
/* === added by Kathy === */
                else
                {
                        out[0]= DEFAULTHO1;
                        out[1]= DEFAULTHO2;
                }
/* === */
             /*   else
                {
                 errno = EILSEQ;
                 err_flag = TRUE;
                 break;
                }
             */
                out += 2;
                in += 2;
        }
        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        if (err_flag) return  -1;
        else return 0;
}

static int     _iconv_close(iconv_t cd)
{
        if ((cd != NULL) && (cd != -1))
              {
                free(cd);
                return 0;
                }
        else
             {   errno = EBADF;
                return -1;
                }
}

static _LC_host_lower_iconv_t   *init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_host_lower_iconv_t  *cd;

        if ((cd = (_LC_host_lower_iconv_t *) malloc(sizeof(_LC_host_lower_iconv_t))) == NULL)
                return (_LC_host_lower_iconv_t*)-1;
        cd->core = *core_cd;
        return cd;
}

_LC_core_iconv_t        *instantiate (void) {

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
