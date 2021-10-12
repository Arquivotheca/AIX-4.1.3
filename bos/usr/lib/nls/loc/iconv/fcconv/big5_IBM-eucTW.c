static char sccsid[] = "@(#)64  1.9  src/bos/usr/lib/nls/loc/iconv/fcconv/big5_IBM-eucTW.c, cmdiconv, bos411, 9428A410j 5/13/94 04:38:31";
/*
 *   COMPONENT_NAME: cmdiconv
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
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <stdio.h>
#include <iconv.h>
#include <iconvP.h>
#include "fcconv.h"
#include "big5-euc.h"
#define DEFAULTHO1    0xfd
#define DEFAULTHO2    0xfe

static        size_t  _iconv_exec (_LC_fcconv_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char  *in, *out;      /* pointer to in/out buffers */
        unsigned char  *e_in, *e_out;  /* point the end of in/out buffers */
        unsigned short *keycode;
        int             i, f_idx, found;
        int             l_idx, r_idx, idx;
        unsigned int    tmpindex;
        unsigned short  keybuffer;
        unsigned int    *tmpcode;
        unsigned char   code1,code2;
        int             err_flag=FALSE;

        if ((cd == NULL) || (cd == -1)) {
                errno = EBADF; return -1;
        }
        if (inbuf == NULL) return 0;
        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
        keycode = &keybuffer;

        while (in < e_in) {
               if (in[0] <= 0x7f)                      /* @V42 */
               {                                       /* @V42 */
/*               while( in[0] <= 0x7F )                   @V42 */
/*               {                                        @V42 */
                  if (e_out - out < 1)                 /* @V42 */
                  {                                    /* @V42 */
                        errno = E2BIG;                 /* @V42 */
                        err_flag = TRUE;               /* @V42 */
                        break;                         /* @V42 */
                  }                                    /* @V42 */
                   *out++ = *in++;
/*                 if(in >= e_in)                         @V42 */
/*                  break;                                @V42 */
/*               }                                        @V42 */
                 continue;                             /* @V42 */
               }                                       /* @V42 */

                if (e_in - in < 2 )
                {

                        errno = EINVAL;
                        err_flag = TRUE;
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

                *((unsigned char *)keycode) = in[0];
                *((unsigned char *)keycode + 1) = in[1];

                found = 0;
                l_idx = 0;
                r_idx = TABLE_SIZE1-1;
                while (r_idx >= 0 && l_idx <= TABLE_SIZE1-1 && l_idx <= r_idx) {
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
                   tmpcode = &(TO_TAB[tmpindex]);
                   if (*((unsigned char *)tmpcode) == 0x8e)
                   {
                       if (e_out - out < 4)
                       {
                              errno = E2BIG;
                              err_flag = TRUE;
                              break;
                       }
                       out[0] = *((unsigned char *)tmpcode);
                       out[1] = *((unsigned char *)tmpcode+1);
                       out[2] = *((unsigned char *)tmpcode+2);
                       out[3] = *((unsigned char *)tmpcode+3);
                       out += 4;
                   }
                   else
                   {
                       if (e_out - out < 2)
                       {
                              errno = E2BIG;
                              err_flag = TRUE;
                              break;
                       }
                       out[0] = *((unsigned char *)tmpcode+2);
                       out[1] = *((unsigned char *)tmpcode+3);
                       out += 2;
                   } /* endif */
                }
                else
                {
                  out[0]=DEFAULTHO1;
                  out[1]=DEFAULTHO2;
                  out += 2;
                }

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

static _LC_fcconv_iconv_t       *init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_fcconv_iconv_t      *cd;

        if ((cd = (_LC_fcconv_iconv_t *) malloc(sizeof(_LC_fcconv_iconv_t))) == NULL)
                return (_LC_fcconv_iconv_t*)-1;
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
        cd.init          = (_LC_core_iconv_t*(*)())init;
        cd.exec          = (size_t(*)())_iconv_exec;
        cd.close         = (int(*)())_iconv_close;
        return &cd;
}
