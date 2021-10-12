static char sccsid[] = "@(#)83	1.1  src/bos/usr/lib/nls/loc/iconv/fcconv/IBM-sbdCN_IBM-eucCN.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:49:28";
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
/* MODULE NAME:        IBM-sbdCN_IBM-eucCN                                 */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert data encoding in IBM-sbdCN to IBM-eucCN     */
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
/********************* END OF SPECIFICATION ********************************/
#include <stdio.h>
#include <stdlib.h>
#include "iconv.h"
#include "fcconv.h"
/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in IBM-eucCN Codeset to            */
/*               IBM-sbdCN Codeset                                        */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : ret_value                                                */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int _iconv_exec (_LC_fcconv_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
     unsigned char   *in, *out;      /* point to in/out buffers */
     unsigned char   *e_in, *e_out;  /* point to the end of in/out buffers */
     int             ret_value;      /* hold the return value */

     if (!cd) {
           /*************************************************
             Initialize specific converter descriptor failed
            *************************************************/
         errno = EBADF; return NULL;
     }

     if (!inbuf) return 0;
 /*******************
  Initialize Pointers
  *******************/
     e_in = (in = *inbuf) + *inbytesleft;
     e_out = (out = *outbuf) + *outbytesleft;

     ret_value = 0;

 /*******************************
  Begin to process various errors
  *******************************/
     while(in < e_in){ 
          /*      if (e_in - in < 2) {
                        if (e_in - in == 1) {
                                if (in[0] < 0xa1 || in[0] == 0xff)
                                      {
                                        errno = EILSEQ; ret_value = -1;
                                      }
                                else
                                      {
                                        errno = EINVAL; ret_value = -1;
                                      }
                        }
                        break;
                }
*/

          if (in[0] < 0xa1 || in[0] > 0xfe) {
              errno = EILSEQ;
              ret_value = -1;
              break;
          }

          if (in[1] < 0xa1 || in[1] > 0xfe) {
              errno = EILSEQ;
              ret_value = -1;
              break;
          }

          if (e_out - out < 2) { 
	      errno = E2BIG;
              ret_value = -1;
              break;
          }

 /************************************************
  Convert Characters in EUC CS1 of CN to IBM-sbdCN
  ************************************************/
          out[0] =  in[0] | 0x80;
          out[1] =  in[1] | 0x80;
          in += 2;
          out += 2;
      };

      *inbytesleft = e_in - in;
      *outbytesleft = e_out - out;
      *inbuf = in;
      *outbuf = out;
      return ret_value;
}

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for IBM-eucCN_IBM-sbdCN            */
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
/* DESCRIPTION : Initialize IBM-eucCN_IBM-sbdCN Converter-Specific        */
/*               Descriptor                                               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : core_cd, toname, fromname                                */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static _LC_fcconv_iconv_t       *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
     _LC_fcconv_iconv_t      *cd;

     if (!(cd = (_LC_fcconv_iconv_t *) malloc(sizeof(_LC_fcconv_iconv_t))))
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
