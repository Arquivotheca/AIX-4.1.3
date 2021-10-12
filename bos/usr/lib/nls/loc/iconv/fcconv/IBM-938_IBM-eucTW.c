static char sccsid[] = "@(#)60  1.9  src/bos/usr/lib/nls/loc/iconv/fcconv/IBM-938_IBM-eucTW.c, cmdiconv, bos411, 9428A410j 5/13/94 04:34:33";
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
#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <iconvP.h>
#include "fcconv.h"
#include "55-euc.h"
#define DEFAULTEUC1   0xfd
#define DEFAULTEUC2   0xfe

static        size_t  _iconv_exec (_LC_fcconv_iconv_t *cd,
                                uchar_t** inbuf, size_t *inbytesleft,
                                uchar_t** outbuf, size_t *outbytesleft)
{
        uchar_t   *in, *out;      /* pointer to in/out buffers */
        uchar_t   *e_in, *e_out;  /* point the end of in/out buffers */
        ushort_t  tmp;
        ushort_t  tmp55in;
        ushort_t  ps55,*ps55in, euc2;
        unsigned long   euc4;
        int             i,indx;
        int             err_flag=FALSE;


        if ((cd == NULL) || (cd == -1)) {
                errno = EBADF; return -1;
        }
        if (inbuf == NULL) return 0;
        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
        ps55in = &tmp55in;

        while (in < e_in) {
               if (in[0] <= 0x7f)                      /* @V42 */
               {                                       /* @V42 */
/*               while( in[0] <= 0x7F )           SBCS    @V42 */
/*               {                                        @V42 */
                  if (e_out - out < 1)                 /* @V42 */
                  {                                    /* @V42 */
                        errno = E2BIG;                 /* @V42 */
                        err_flag = TRUE;               /* @V42 */
                        break;                         /* @V42 */
                  }                                    /* @V42 */
                  *out++ = *in++;
/*                if(in >= e_in)                          @V42 */
/*                break;                                  @V42 */
/*               }                                        @V42 */
                 continue;                             /* @V42 */
               }                                       /* @V42 */
/* === Below : modify sequence by Kathy === */
/*              if(in >= e_in) break;                      @V42 */
                if (e_in - in < 2 )
                {       errno = EINVAL;
                        err_flag = TRUE;
                                  /*skip 0x80 0xfd and 0xfe added by Debby */
                        if( !( in[0] >=0x81 && in[0] <=0xfc ) )
                             errno = EILSEQ;
                        break;
                }
                if( !( in[0] >=0x81 && in[0] <=0xfc ) )
                {
                 errno = EILSEQ;
                 err_flag = TRUE;
                 break;
                }
                if( !( in[1] >=0x40 && in[1] <=0x7e ) &&
                    !( in[1] >=0x80 && in[1] <=0xfc )    )
                {
                 errno = EILSEQ;
                 err_flag = TRUE;
                 break;
                }

                if (e_out - out < 2)
                {       errno = E2BIG;
                        err_flag = TRUE;
                        break;
                }
/* === */
                /*  initial out */
               /*  out[0]=0x00;  */
               /*  out[1]=0x00;  */
               *((uchar_t *)ps55in) = in[0];
               *((uchar_t *)ps55in + 1) = in[1];

                /* CNS symbols */
                if ((  *ps55in <= 0x8b61 )||
                     ((*ps55in == 0xa09c) || (*ps55in == 0xa94a) ||
                      (*ps55in == 0xb3b7) || (*ps55in == 0xbaa3) ||
                      (*ps55in == 0xbab3)) )
                      /* 5 IBM unique symbols */
                   {

                if (  *ps55in <= 0x8b61 ) {
                   for (i=0; i<684; i++)
                     {
                       ps55 = ps2esymb[i][0];
                       euc2 = ps2esymb[i][1];
                       if (( in[0] == (ps55 >> 8)) &&
                           ( in[1] == (ps55 & 0x00ff)) )
                           {
                             out[0] = (euc2 >> 8) ;
                             out[1] = (euc2 & 0x00ff) ;
                             in += 2;
                             out += 2;
                             break;
                          }
                     } /* end of for loop */
                     if (i <= 683) continue;
                   }   /* end if 8b5a */

                     for (i=0; i<325; i++)
                         {
                          ps55 = ps2esbd[i][0];
                          euc4 = ps2esbd[i][1];
                          if (( in[0] == (ps55 >> 8) ) &&
                              ( in[1] == (ps55 & 0xff)) )
                             {
                             if (e_out - out < 4)
                             {       errno = E2BIG;
                                     err_flag = TRUE;
                                     break;
                             }
                                out[0] = (euc4 >> 24);
                                out[1] = (euc4 >> 16);
                                out[2] = (euc4 >> 8);
                                out[3] = (euc4 & 0x000000ff);
                                in += 2;
                                out += 4;
                                break;
                             }
                         } /* end of for loop */

                     if (err_flag) break;
                      if ( i <= 324) continue;
                   }  /* end if 0x4959 and 5 symb check */


                   /* CNS1 primary */

                if (*ps55in >= 0x8c40 && *ps55in <= 0xa8ca) {
                   indx = ( in[0] - 0x8c)*188 + (in[1] - 0x40) ;

                   /* 2nd byte of 5550 code can't conain 0x7f*/
                   /* 188 already exclude 0x7f */

                   if (in[1] > 0x7f) --indx ;

                   euc2 = ps2ep1[indx];
                   out[0] = euc2 >> 8 ;
                   out[1] = euc2 & 0x00ff ;
                   in += 2;
                   out+=2;
                   continue;
                }
                /* CNS2 secondary */
                if (*ps55in >= 0xa940 && *ps55in <= 0xd1c6) {
                   indx = ( in[0] - 0xa9)*188 + (in[1] - 0x40) ;
                   if (in[1] > 0x7f) --indx ;

                   euc4 = ps2ep2[indx];
                   if (e_out - out < 4)
                   {       errno = E2BIG;
                           err_flag = TRUE;
                           break;
                   }
                   out[0] = (euc4 >> 24);
                   out[1] = (euc4 >> 16);
                   out[2] = (euc4 >> 8);
                   out[3] = (euc4 & 0x000000ff);
                   in += 2;
                   out+=4;
                   continue;
                }
                /*udc */
                if (*ps55in >= 0xdb40 && *ps55in <= 0xfbfc) {
                   indx = ( in[0] - 0xdb)*188 + (in[1] - 0x40) ;
                   if (in[1] > 0x7f) --indx ;
                   euc4 = ps2eudc[indx];
                   if (e_out - out < 4)
                   {       errno = E2BIG;
                           err_flag = TRUE;
                           break;
                   }
                   out[0] = (euc4 >> 24);
                   out[1] = (euc4 >> 16);
                   out[2] = (euc4 >> 8);
                   out[3] = (euc4 & 0x000000ff);
                   in += 2;
                   out+=4;
                   continue;
                }
                     out[0]= DEFAULTEUC1;
                     out[1]= DEFAULTEUC2;
                 /*    err_flag = TRUE;   */
                     out+=2;
                     in += 2;
        } /* end of while */
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
