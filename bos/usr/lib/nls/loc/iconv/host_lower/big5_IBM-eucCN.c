static char sccsid[] = "@(#)05	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/big5_IBM-eucCN.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:54";
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
/* MODULE NAME:        BIG5_IBM-eucCN.c                                    */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert data encoding in BIG5 to IBM-eucCN          */
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
/* MODULE TYPE:        C                                                   */
/*                                                                         */
/* COMPILER:           AIX C                                               */
/*                                                                         */
/* AUTHOR:             Dong Meiting, Wang Ying                             */
/*                                                                         */
/* STATUS:             Simplified Chinese Iconv Version 1.0                */
/*                                                                         */
/* CHANGE ACTIVITY:    None                                                */
/*                                                                         */
/********************* END OF SPECIFICATION ********************************/

#include <stdio.h>
#include <stdlib.h>
#include "iconv.h"
#include "iconvP.h"
#include "host_lower.h"
#include "big5-euccn.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in BIG5 Codeset to                 */
/*               IBM-eucCN Codeset                                        */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : ret_value                                                */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int _iconv_exec (_LC_host_lower_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
     unsigned char   *in, *out;      /* Pointer to input/output buffers */
     unsigned char   *e_in, *e_out;  /* Point the end of input/output buffers */
     int             ret_value;      /* Hold the return value */
     unsigned short  tmpbig,indx ;
     unsigned short  big5,*big5in;
     unsigned short  start, middle, end;
     unsigned short  outcn1, *outtmp;

     if (!cd) {
         errno = EBADF; return NULL;
     }

     if (!inbuf) return 0;
 /*
  Initialize Various Pointers
 */
     e_in = (in = *inbuf) + *inbytesleft;
     e_out = (out = *outbuf) + *outbytesleft;
     ret_value = 0;
     big5in = &tmpbig;
     outtmp = &outcn1;

     while (in < e_in) {
 /*
  Convert Characters in EUC CS0 of CN
 */ 
          if( in[0] <= 0x7F ) {
	       if(e_out - out < 1){
                   errno = E2BIG;
                   ret_value = -1;
                   break;
               }
               *out++ = *in++;
               if(in >= e_in) break;
	       continue;
           }

/*
 Process Input/Output buffer
*/
          if (e_in - in < 2 ) {
              errno = EINVAL;
              ret_value = -1;
              break;
          }

          if (e_out - out < 2) {
              errno = E2BIG;
              ret_value = -1;
              break;
          }

          if (in[0] < 0xa1 || in[0] == 0xff) {
              errno = EILSEQ;
              ret_value = -1;
              break;
          }

 /*
  Process Chinese and Graphic characters one by one
 */ 
          *((unsigned char *)big5in) = in[0];
          *((unsigned char *)big5in + 1) = in[1];
/*
 Convert Hanzix Characters
*/ 
          if((*big5in >= 0xa440) && (*big5in <= 0xf9d5)) {
	      start = HEADER;
	      end = TRAILER; 
	      middle = (end + start)/2; 

	      while((*big5in != big5toeuccn[middle][0])&&(start <= end)) {
		   if(*big5in > big5toeuccn[middle][0]) {
		      start = middle + 1;
		   } 
		   else
		   if(*big5in < big5toeuccn[middle][0]) {
		      end = middle-1;
		   }
		   middle = (start + end) / 2;
	      };

	      if(*big5in != big5toeuccn[middle][0]) {
		  *outtmp = DEFAULTEUC;
	      };

	      if(*big5in == big5toeuccn[middle][0]) {
		  *outtmp = big5toeuccn[middle][1];
	       }
	    }
        else {
 /*
  Convert Graphic Characters
 */
	    if((*big5in >= 0xa140) && (*big5in <= 0xa17e)) {
	        indx = in[1] - 0x40;
	        *outtmp = big5tocnnon1[indx];
	    }
	    else
	    if((*big5in >= 0xa1a1) && (*big5in <= 0xa1fe)) {
	        indx = in[1] - 0xa1;
	        *outtmp = big5tocnnon2[indx];
	    }
	    else
	    if((*big5in >= 0xa240) && (*big5in <= 0xa27e)) {
	        indx = in[1] - 0x40;
	        *outtmp = big5tocnnon3[indx];
	    }
	    else
	    if((*big5in >= 0xa2a1) && (*big5in <= 0xa2fe)) {
	        indx = in[1] - 0xa1;
	        *outtmp = big5tocnnon4[indx];
	    }
	    else
	    if((*big5in >= 0xa340) && (*big5in <= 0xa37e)) {
	        indx = in[1] - 0x40;
	        *outtmp = big5tocnnon5[indx];
	    }
	    else
	    if((*big5in >= 0xa3a1) && (*big5in <= 0xa3bf)) {
	        indx = in[1] - 0xa1;
	        *outtmp = big5tocnnon6[indx];
	    }
	    else {
		*outtmp = DEFAULTEUC;
	    }
	}
	out[0] = *(unsigned char*)outtmp;
	out[1] = *((unsigned char*)outtmp + 1);
	out += 2;
	in  += 2;
    } 
    *inbytesleft = e_in - in;
    *outbytesleft = e_out - out;
    *inbuf = in;
    *outbuf = out;
    return ret_value;
}

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for BIG5_IBM-eucCN                 */
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
/* DESCRIPTION : Initialize BIG5_IBM-eucCN  Converter-Specific            */
/*               Descriptor                                               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : core_cd, toname, fromname                                */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static _LC_host_lower_iconv_t       *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
     _LC_host_lower_iconv_t      *cd;

     if (!(cd = (_LC_host_lower_iconv_t *) malloc(sizeof(_LC_host_lower_iconv_t))))
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
