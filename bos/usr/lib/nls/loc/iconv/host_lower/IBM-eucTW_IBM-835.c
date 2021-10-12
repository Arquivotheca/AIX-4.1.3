static char sccsid[] = "@(#)14  1.8  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-eucTW_IBM-835.c, cmdiconv, bos411, 9428A410j 4/11/94 17:44:54";
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
#include "euc-host.h"
#define  DEFAULTHO1  0xfe
#define  DEFAULTHO2  0xfd

static size_t _iconv_exec (_LC_host_lower_iconv_t *cd,
                                uchar_t** inbuf, size_t *inbytesleft,
                                uchar_t** outbuf, size_t *outbytesleft)
{
        uchar_t   *in, *out;      /* pointer to in/out buffers */
        uchar_t   *e_in, *e_out;  /* point the end of in/out buffers */
        ushort_t  tmp,indx;
        ushort_t  host,euc2,*euc2in;
        uchar_t   tmp2[5];
        int             i;
        int             err_flag=FALSE;


        if ((cd == NULL) || (cd == -1)) {
                errno = EBADF; return -1;
        }
        if (inbuf == NULL) return 0;
        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
        euc2in = &tmp;
        while (in < e_in) {
               if (in[0] == 0x8e)  /* 4 bytes euc in 8ea2 or 8ead */
                {
/* === Below : modify sequence by Kathy === */
                        if (e_in - in < 4)
                        {
                                err_flag = TRUE;
                                errno = EINVAL;
                                break;
                        }
                        if (!(in[1] >= 0xa2 && in[1] <= 0xb0) )
                          {

                              errno = EILSEQ;
                              err_flag = TRUE;
                              break;

                        }
                       else if (in[2] < 0xa1 || in[2] == 0xff)
                        {
                              errno = EILSEQ;
                              err_flag = TRUE;
                              break;
                        }
                       else if (in[3] < 0xa1 || in[3] == 0xff)
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
              /*        else {     */
                          *((uchar_t *)euc2in) = in[2];
                          *((uchar_t *)euc2in +1) = in[3];

                          if (( in[1] == 0xa2)&&(*euc2in <= 0xf2c4 )){
                           indx = (ushort_t)((in[2] - 0xa1)*94 + (in[3]-0xa1)) ;
                           host = e2hp2[indx];
                           out[0] = (host >> 8);
                           out[1] = (host & 0x00ff);
                           in += 4;
                           out += 2;
                           continue;
                        }
                        if (( in[1] == 0xac)&&(*euc2in  <= 0xe2fe)){
                           indx = (ushort_t)((in[2] - 0xa1)*94 + (in[3]-0xa1)) ;
                           host = e2hudc[indx];
                           out[0] = (host >> 8);
                           out[1] = (host & 0x00ff);
                           in += 4;
                           out += 2;
                           continue;
                        }
                        if (( in[1] == 0xad)&&(*euc2in <= 0xa4cb)){
                           indx = (ushort_t)((in[2] - 0xa1)*94 + (in[3]-0xa1)) ;
                           host = e2hsbd[indx];
                           out[0] = (host >> 8);
                           out[1] = (host & 0x00ff);
                           in += 4;
                           out += 2;
                           continue;
                        }
                           out[0] = DEFAULTHO1 ;
                           out[1] = DEFAULTHO2 ;
                      /*     err_flag = TRUE;  */
                        out += 2;
                        in += 4;
                        continue;
                  /*      }*/ /* end of if else for searching */
                } /* end of 4 bytes if */
                else {   /* 2 bytes EUC in range a1-fe, a1-  fe */
/* === Below : modify sequence by Kathy === */
                        if ( e_in - in < 2 )
                        {
                                errno = EINVAL;
                                err_flag = TRUE;
                                                /* skip all sbcs add by Debby */
                                if (in[0] < 0xa1 || in[0] == 0xff)
                                   errno = EILSEQ;
                                break;
                        }
                       if (in[0] < 0xa1 || in[0] == 0xff)
                        {
                            errno = EILSEQ;
                            err_flag = TRUE;
                            break;
                        }
                        else if (in[1] < 0xa1 || in[1] == 0xff)
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
                 /*     else {   */
                        /*  euc 2 bytes   */

                       *((uchar_t *)euc2in) = in[0];
                       *((uchar_t *)euc2in +1) = in[1];


                        if ( *euc2in <= 0xc2c1 ) {
                        for (i=0; i<684; i++)
                          {
                            euc2 = e2hsymb[i][0];
                            host = e2hsymb[i][1];
                            if (( in[0] == (euc2 >>8 )) &&
                                ( in[1] == (euc2 & 0x00ff)))
                                {
                                out[0] = (host >> 8);
                                out[1] = (host & 0x00ff);
                                in += 2;
                                out += 2;
                                break;
                                }
                          } /* end of for loop */
                          if (i <= 683 ) continue;
                        }  /* end if 0xc2c1 check */

                        if ((*euc2in >= 0xc4a1 ) && (*euc2in <= 0xfdcb)){
                           indx = (ushort_t)((in[0] - 0xc4)*94 + (in[1]-0xa1)) ;
                           host = e2hp1[indx];
                           out[0] = (host >> 8);
                           out[1] = (host & 0x00ff);
                           in += 2;
                           out += 2;
                           continue;
                        }
                        else {
                        out[0] = DEFAULTHO1 ;
                        out[1] = DEFAULTHO2 ;
               /*         err_flag = TRUE;    */
                        in += 2;
                        out += 2;
                        } /* end of else for searching */
                }/* end of else of 2 byte */

        }     /* while */

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

_LC_core_iconv_t        *instantiate(void) {

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
