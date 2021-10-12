static char sccsid[] = "@(#)09  1.8  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-835_IBM-eucTW.c, cmdiconv, bos411, 9428A410j 4/11/94 17:44:25";
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
#include <iconv.h>
#include <iconvP.h>
#include "host_lower.h"
/*#include "IBM.h" */
#include "host-euc.h"
#define  DEFAULTEUC1    0xfd
#define  DEFAULTEUC2    0xfe

static size_t _iconv_exec (_LC_host_lower_iconv_t *cd,
                                uchar_t** inbuf, size_t *inbytesleft,
                                uchar_t** outbuf, size_t *outbytesleft)
{
        uchar_t   *in, *out;      /* pointer to in/out buffers */
        uchar_t   *e_in, *e_out;  /* point the end of in/out buffers */
        ushort_t  tmphin;
        ushort_t  host,*hostin, euc2;
        unsigned long   euc4;
        int             i,indx,hb;
        int             err_flag=FALSE;

        if ((cd == NULL) || (cd == -1)) {
                errno = EBADF; return -1;
        }
        if (inbuf == NULL) return 0;

        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
        hostin = &tmphin;

        while (in < e_in) {
/* ===  Below : Modify the sequence by Kathy === */
                if (e_in - in < 2)
                {
                    errno = EINVAL;
                    err_flag = TRUE;
                    if(in[0]==0x0f)     /* skip 0x0f added by Debby */
                       errno = EILSEQ;
                    break;
                }
                if(in[0]==0x0f)
                 {
                    err_flag = TRUE;
                    errno = EILSEQ;
                    break;
                }
                if( in[0] == 0x40 && in[1] !=0x40 )
                {
                  errno = EILSEQ;
                  err_flag = TRUE;
                  break;
                }
               else if (( !( in[0] >= 0x41 && in[0] <= 0xfe )  ||
                     !( in[1] >= 0x41 && in[1] <= 0xfe ) ) &&
                     !( in[0] == 0x40 && in[1] == 0x40 ) )
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
/* ===  Above : Modify the sequence by Kathy === */

               *((uchar_t *)hostin) = in[0];
               *((uchar_t *)hostin + 1) = in[1];

                /* CNS symbols */
                if ((  *hostin <= 0x4959 ) ||
                     ((*hostin == 0x609d) || (*hostin == 0x694b) ||
                      (*hostin == 0x73b8) || (*hostin == 0x7aa4) ||
                      (*hostin == 0x7ab4)) )
                      /* 5 IBM unique symbols */
                   {

                if (  *hostin <= 0x4959 ) {
                   for (i=0; i<684; i++)
                     {
                       host = h2esymb[i][0];
                       euc2 = h2esymb[i][1];
                       if (( in[0] == (host >> 8)) &&
                           ( in[1] == (host & 0x00ff)) )
                           {
                             out[0] = (euc2 >> 8) ;
                             out[1] = (euc2 & 0x00ff) ;
                             in += 2;
                             out += 2;
                             break;
                           }
                     } /* end of for loop */
                     if (i <= 683) continue;
                   }   /* end if 4059 */

                     for (i=0; i<325; i++)
                         {
                          host = h2esbd[i][0];
                          euc4 = h2esbd[i][1];
                          if (( in[0] == (host >> 8) ) &&
                              ( in[1] == (host & 0xff)) )
                             {
/* === added by Kathy === */
                             if (e_out - out < 4)
                             {
                                 errno = E2BIG;
                                 err_flag = TRUE;
                                 break;
                             }
/* === */
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
                if (*hostin >= 0x4c41 && *hostin <= 0x68cb) {
                   indx = ( in[0] - 0x4c)*188 + (in[1] - 0x41) ;

                   /* 2nd byte of Host code can't conain 0x80*/
                   /* 188 already exclude 0x80 */

                   if (in[1] > 0x80) --indx ;

                   euc2 = h2ep1[indx];
                   out[0] = euc2 >> 8 ;
                   out[1] = euc2 & 0x00ff ;
                   in += 2;
                   out+=2;
                   continue;
                }
                /* CNS2 secondary */
                if (*hostin >= 0x6941 && *hostin <= 0x91c7) {
                   indx = ( in[0] - 0x69)*188 + (in[1] - 0x41) ;
                   if (in[1] > 0x80) --indx ;

                   euc4 = h2ep2[indx];
                   if (e_out - out < 4)
                   {
                       errno = E2BIG;
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
                if (*hostin >= 0xc241 && *hostin <= 0xe2fd) {
                   indx = ( in[0] - 0xc2)*188 + (in[1] - 0x41) ;
                   if (in[1] > 0x80) --indx ;
                   euc4 = h2eudc[indx];
                   if (e_out - out < 4)
                   {
                       errno = E2BIG;
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
                     out[0] = DEFAULTEUC1 ;
                     out[1] = DEFAULTEUC2 ;
                  /*
                     errno = EILSEQ;
                     err_flag = TRUE;
                   */
                     in += 2;
                     out +=2;
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
