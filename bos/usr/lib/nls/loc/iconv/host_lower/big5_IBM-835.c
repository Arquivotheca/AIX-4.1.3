static char sccsid[] = "@(#)19  1.8  src/bos/usr/lib/nls/loc/iconv/host_lower/big5_IBM-835.c, cmdiconv, bos411, 9428A410j 4/11/94 17:45:05";
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
#include "big5-host.h"
#define DEFAULTHO1   0xfe
#define DEFAULTHO2   0xfd

static size_t _iconv_exec (_LC_host_lower_iconv_t *cd,
                                uchar_t** inbuf, size_t *inbytesleft,
                                uchar_t** outbuf, size_t *outbytesleft)
{
        uchar_t  *in, *out;      /* pointer to in/out buffers */
        uchar_t  *e_in, *e_out;  /* point the end of in/out buffers */
        ushort_t *keycode;
        int             i, f_idx, found;
        int             l_idx, r_idx, idx;
        int             err_flag=FALSE;
        unsigned int tmpindex;
        ushort_t keybuffer, tmpcode;
        uchar_t  code1,code2;


        if ((cd == NULL) || (cd == -1)) {
                errno = EBADF; return -1;
        }
        if (inbuf == NULL) return 0;
        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
        keycode = &keybuffer;
       /* printf("go here\n");*/
        while (in < e_in) {

/*             while( in[0] <= 0x7F )    SBCS     Deleted by Debby */
/*              {                                                  */
/*               *out++ = *in++;                                   */
/*               if(in >= e_in)                                    */
/*               break;                                            */
/*              }                                                  */

                if (e_in - in < 2 )
                {
                        err_flag = TRUE;
                        errno = EINVAL;

                        if (in[0] <= 0x7F)      /* added by Debby */
                           errno = EILSEQ;

                        break;
                }
                /* High byte: 81-FE, 81-A0 is UDC area */
                if (in[0] < 0x81 || in[0] == 0xff)
                {
                        errno = EILSEQ;
                        err_flag = TRUE;
                        break;
                }
                /* Low byte: 40-7E, 81-FE, 81-A0 is UDC */
                if (!((in[1] >= 0x40 && in[1] <= 0x7e) ||
                      (in[1] >=0x81 && in[1]  <=0xa0 ) ||
                      (in[1] >=0xa1 && in[1]  <=0xfe ) ) )
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

               *((uchar_t *)keycode) = in[0];
               *((uchar_t *)keycode + 1) = in[1];
                found = 0;
                l_idx = 0;
                r_idx = TABLE_SIZE1-1;
                /*printf("go next %x\n", *keycode);*/
                while (r_idx >= 0 && l_idx <= TABLE_SIZE1-1 && l_idx <= r_idx) {
                   /*printf("%d %d %d\n", l_idx, r_idx, idx);*/
                   idx = (l_idx + r_idx) / 2 ;
                   if ((*keycode >= FROM_TAB[idx][0]) && (*keycode <= FROM_TAB[idx][1]))
                   {
                      found = 1;
                      break;
                   }
                   if (*keycode > FROM_TAB[idx][1])
                      l_idx = idx + 1;
                   if (*keycode < FROM_TAB[idx][0])
                      r_idx = idx - 1;
                } /* endwhile */
                if (found)
                {
                   tmpindex= (unsigned int)(FROM_TAB[idx][2] + *keycode- FROM_TAB[idx][0]);
                   tmpcode = TO_TAB[tmpindex];
                   out[0] = tmpcode >> 8 ;
                   out[1] = tmpcode & 0x00ff ;
                }
                else
                {
                  out[0]=DEFAULTHO1;
                  out[1]=DEFAULTHO2;
                }

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

