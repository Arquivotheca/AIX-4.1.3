static char sccsid[] = "@(#)82	1.1  src/bos/usr/lib/nls/loc/iconv/fcconv/IBM-eucCN_UTF-8.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:49:25";
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
/* MODULE NAME:        IBM-eucCN_UTF-8                                     */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert EUC Encoding to UTF8(Transform of Unicode)  */
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
#include "fcconv.h"
#include "euccn-ucs.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in IBM-eucCN Codeset to            */
/*               UTF8(Transform of Unicode)                               */
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
        unsigned char   *in, *out;      /* Point to in/out buffers */
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             ret_value;      /* Hold the return value */
        unsigned short  euc2, ucs, *euc2in;
        unsigned short  tmpbuf, tmpbuf1;
	unsigned short  indx;       /* Used to locate the positon in a array */
	unsigned short  *outtmp;
	short  		start, middle, end;

        	if (!cd) {
                	errno = EBADF; return NULL;
        	};

        	if (!inbuf) return 0;
 /*******************
  Initialize Pointers
  *******************/
 		e_in = (in = *inbuf) + *inbytesleft;
        	e_out = (out = *outbuf) + *outbytesleft;
		euc2in = &tmpbuf;
		outtmp = &tmpbuf1;

        	ret_value = 0;

        	while (in < e_in ) { 

			if(in[0] <= 0x7e) {
 /***********************************
  Convert Characters in EUC CS0 of CN 
  ***********************************/

                	    if (e_out - out < 1) {   
		        	 errno = E2BIG;
                        	 ret_value = -1;
                        	 break;
                	    };

                 	    *out++ = *in++;
                            if(in>=e_in) break;
			    continue;
                	}

 /*************
  Handle Errors
  *************/
                	if (e_in - in < 2) {   
		        	errno = EINVAL;
                        	ret_value = -1;
                        	break;
                	};

                	if (in[0] < 0xa1 || in[0] == 0xff) {
                        	errno = EILSEQ;
                        	ret_value = -1;
                        	break;
                	};

                	if (in[1] < 0xa1 || in[1] == 0xff) {
                        	errno = EILSEQ;
                        	ret_value = -1;
                        	break;
                	};

 /**********************************************
  Obtain One Input Word, Either Hanzi or Graphic
  **********************************************/
               	      *((unsigned char *)euc2in) = in[0];
                      *((unsigned char *)euc2in +1) = in[1];

                      if((*euc2in < 0xa1a1)||(*euc2in>0xfefe)){
		   	    errno = E2BIG;
                   	    ret_value = -1;
                   	    break;
                      }else{
                      start  = HEADER;
                      end    = TRAILER;
                      middle = ( start + end ) / 2;

                      while( *euc2in != gbtoucs[middle][0] && start <= end ) {
                           if( *euc2in < gbtoucs[middle][0] ) {
                                 end = middle - 1;
                           }
                           if( *euc2in > gbtoucs[middle][0] ) {
                                start = middle + 1;
                           }
                           middle = ( start + end ) / 2;
                      };
		      if(*euc2in == gbtoucs[middle][0]){
                           ucs = gbtoucs[middle][1];
		      }
 /*******************************************
  Locate User-Defined Hanzi and Graphic Words
  *******************************************/
                      else if( in[0] >= 0xa2 && in[0] <= 0xa9 ) {
                          start = UDC_HEADER;
                          end = UDC_TRAILER;
                          middle = ( start + end ) / 2;
                      while( *euc2in != udc_index[middle] && start <= end ) {
                           if( *euc2in < udc_index[middle] ) {
                                 end = middle - 1;
                           }
                           if( *euc2in > udc_index[middle] ) {
                                start = middle + 1;
                           }
                           middle = ( start + end ) / 2;
                      };
                      if(start > end) {
                            errno = EILSEQ;
			    break;
                      }
                      else {
                            ucs = (0xe000 + (unsigned short)middle);
                           }
		  }
		  else if( in[0] >= 0xaa && in[0] <= 0xaf ) {
		      indx = (unsigned short)((in[0] - 0xaa)*94+(in[1] - 0xa1));
                      ucs = (0xe0a4 + (unsigned short)indx);
		  }
		  else if( in[0] == 0xd7 ) {
		      indx = (unsigned short)((in[0] - 0xd7)*94+(in[1] - 0xfa));
                      ucs = (0xe2d8 + (unsigned short)indx);
		  }	
		  else
		    if(in[0] >= 0xf8 && in[0] <= 0xfe) {
		      indx = (unsigned short)((in[0] - 0xf8)*94+(in[1] - 0xa1));
                      ucs = (0xe2dd + (unsigned short)indx);
		    }
	    	  else {
		    errno = E2BIG;
                    ret_value = -1;
                    break;
                  };
  /***************** 
   Begin to convert all characters located before 
   *****************/
		    *(unsigned char*)outtmp = (ucs >> 8);
		    *((unsigned char*)outtmp+1) = (ucs & 0x00ff);

		    if(*outtmp >= 0x80 && *outtmp <= 0x7ff){
                	if (e_out - out < 2) {   
		        	errno = E2BIG;
                        	ret_value = -1;
                        	break;
                	};
		         out[0]=((*(unsigned char*)outtmp<<2)|((*((unsigned char*)outtmp+1)&0xc0)>>6)|0xc0);
			 out[1] = (*((unsigned char*)outtmp+1)&0x3f)|0x80;
			 out += 2;
			 in += 2;
		    }else{
                	if (e_out - out < 3) {   
		        	errno = E2BIG;
                        	ret_value = -1;
                        	break;
                	};
			 out[0] = ((*(unsigned char*)outtmp&0xf0)>>4)|0xe0;
			 out[1] = ((*(unsigned char*)outtmp&0x0f)<<2)|((*((unsigned char*)outtmp+1)&0xc0)>>6)|0x80;
			 out[2] = ((*((unsigned char*)outtmp+1)&0x3f)|0x80);
			 out += 3;
                       	 in += 2;
		    }
	       }
			
	}; /* The end of while */
        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        return ret_value;
}

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for IBM-eucCN_UTF8                 */
/*               Converter Descriptor                                     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd                                                       */
/* OUTPUT      : none                                                     */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static void     _iconv_close(iconv_t cd)
{
        if (cd){
              free(cd);
	}
        else
                errno = EBADF;
}

/**************************************************************************/
/* FUNCTION    : _iconv_init                                              */
/* DESCRIPTION : Initialize IBM-eucCN_UTF8  Converter-Specific            */
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

        cd.hdr.__magic = _LC_MAGIC;
        cd.hdr.__version = _LC_VERSION;
        cd.hdr.__type_id = _LC_ICONV;
        cd.hdr.__size = sizeof (_LC_core_iconv_t);
        cd.init          = (_LC_core_iconv_t*(*)())_iconv_init;
        cd.exec          = (size_t(*)())_iconv_exec;
        cd.close         = (int(*)())_iconv_close;
        return &cd;
}
