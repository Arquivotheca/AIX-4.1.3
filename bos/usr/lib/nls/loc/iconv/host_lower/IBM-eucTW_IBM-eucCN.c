static char sccsid[] = "@(#)95	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-eucTW_IBM-eucCN.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:33";
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
/* MODULE NAME:        IBM-eucTW_IBM-eucCN                                 */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert data encoding in EUC-TW to EUC-CN           */
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
#include "euctw-euccn.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in IBM-eucTW Codeset to            */
/*               IBM-eucCN Codeset.                                       */
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
        unsigned char   *in, *out; /* Point to the input and output buffer */
	unsigned short  *outtmp, *outtmp1, *outtmp2;/* Temporary buffer */ 
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             ret_value;      /* Hold the return value */
        unsigned short  *euc2in, tmpbuf;
	short  		middle, start, end;
	unsigned short  tmpbuf1;
	
        if (!cd) {
            errno = EBADF; return NULL;
        };

        if (!inbuf) return 0;
 /***************************
  Initialize Various Pointers
  ***************************/
 	e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
	euc2in = &tmpbuf;
	outtmp1 = &tmpbuf;
	outtmp =  &tmpbuf1;

        ret_value = 0;

        while (in < e_in) { 
 /***********************************
  Convert Characters in EUC CS0 of TW
  ***********************************/
	    if( in[0] <= 0x7F ) {
		 if(e_out - out < 1){
	              errno = E2BIG;
                      ret_value = -1;
                      break;
                  }
                 *out++ = *in++;
                 if(in >= e_in)
                 break;
		 continue;
            };

	    if(!(in[0] == 0x8e)) {
  /***********************************
   Convert Characters in EUC CS1 of TW
   ***********************************/
                 if (e_in - in < 2) {       
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
                  if (in[1] < 0xa1 || in[1] == 0xff) {
                      errno = EILSEQ;
                      ret_value = -1;
                      break;
                  }

                  *((unsigned char *)euc2in) = in[0];
                  *((unsigned char *)euc2in +1) = in[1];
 /************************
  Convert Hanzi Characters
  ************************/
		  if((*euc2in >= 0xc4a1) && (*euc2in <= 0xfdcb))
		  {
		      start = HEADER1;
		      end = TRAILER1; 
		      middle = (end + start)/2; 
		      while((*euc2in != euctwtoeuccnhan1[middle][0])&&(start <= end)) {
			    if((*euc2in) > euctwtoeuccnhan1[middle][0]) {
				start = middle + 1;
			    } 
			    else
			    if((*euc2in) < euctwtoeuccnhan1[middle][0]) {
				end = middle-1;
			    }
			    middle = (end + start)/2;
		       };

		       if((*euc2in) != euctwtoeuccnhan1[middle][0]) {
			    *(unsigned short*)outtmp = DEFAULTCN;
			}

			if((*euc2in) == euctwtoeuccnhan1[middle][0]) {
			    *(unsigned short*)outtmp = euctwtoeuccnhan1[middle][1];
			}
			    out[0] = *(unsigned char*)outtmp;
			    out[1] = *((unsigned char*)outtmp + 1);
			    out += 2;
			    in  += 2;
			    continue;
	           }
 /**************************
  Convert Graphic Characters 
  **************************/
	           if((*euc2in >= 0xa1a1) && (*euc2in <= 0xc3fe)) {
		        start = HEADER3;
		        end = TRAILER3; 
		        middle = (end + start)/2; 

		        while((*euc2in != euctwtoeuccnnon[middle][0])&&(start <= end)) {
		    	    if((*euc2in) > euctwtoeuccnnon[middle][0]) {
				start = middle + 1;
			    } 
			    else
			    if((*euc2in) < euctwtoeuccnnon[middle][0]) {
				end = middle-1;
			    }
			    middle = (end + start)/2;
			 };/* End non-hanz of while for seraching */

			 if((*euc2in) != euctwtoeuccnnon[middle][0]) {
			    *(unsigned short*)outtmp = DEFAULTCN;
			  };

			  if((*euc2in) == euctwtoeuccnnon[middle][0]) {
					
			     (*(unsigned short*)outtmp) = euctwtoeuccnnon[middle][1];
			   }
		     	}
			     out[0] = *((unsigned char*)outtmp);
			     out[1] = *((unsigned char*)outtmp+1);
			     out += 2;
			     in  += 2;
			     continue;
	        }
	        else
		{
 /***************************************
  Convert Characters in EUC CS2, CS3of TW
  ***************************************/
                     if (e_in - in < 4) {       
		         errno = EINVAL;
                         ret_value = -1;
                         break;
                     }
                     if (e_out - out < 2) {       
			 errno = E2BIG;
			 ret_value = -1;
			 break;
                     }
                     if ( !(in[1] >= 0xa2 && in[1] <= 0xb0) ) {
                         errno = EILSEQ;
                         ret_value = -1;
                         break;
                     }
                     if (in[2] < 0xa1 || in[2] == 0xff) {
                         errno = EILSEQ;
                         ret_value = -1;
                         break;
                     }
                     if (in[3] < 0xa1 || in[3] == 0xff) {
                         errno = EILSEQ;
                         ret_value = -1;
                         break;
                     }
		     if(in[1] == 0xa2) {
 /*****************************
  Convert Characters in EUC CS2
  *****************************/
                	 *((unsigned char *)euc2in) = in[2];
                	 *((unsigned char *)euc2in +1) = in[3];

			 if((*euc2in >= 0xa1a1) && (*euc2in <= 0xf9d5)) {
			     start = HEADER2;
			     end = TRAILER2; 
			     middle = (end + start)/2; 

			     while((*euc2in != euctwtoeuccnhan2[middle][0])&&(start <= end)) {
			  	    if((*euc2in) > euctwtoeuccnhan2[middle][0]){
					start = middle + 1;
				    } 
				    else
				    if((*euc2in) < euctwtoeuccnhan2[middle][0]){
					end = middle-1;
				    }
				    middle = (end + start)/2;
			     };/* End searching character in cs2 */

		       	     if((*euc2in) != euctwtoeuccnhan2[middle][0]) {
			    	*(unsigned short*)outtmp = DEFAULTCN;
			      };

			      if(*euc2in == euctwtoeuccnhan2[middle][0]) {
			         (*(unsigned short*)outtmp) = euctwtoeuccnhan2[middle][1];
			      }
			         out[0] = *(unsigned char*)outtmp;
			         out[1] = *((unsigned char*)outtmp+1);
			         out += 2;
			         in  += 4;
			         continue;
			  }
		      }
		      else
		      {
 /*****************************
  Convert Characters in EUC CS3
  *****************************/
			  if(in[1] == 0xad) {
			    	*(unsigned short*)outtmp = DEFAULTCN;
			         out[0] = *(unsigned char*)outtmp;
			         out[1] = *((unsigned char*)outtmp+1);
			         out += 2;
			         in  += 4;
			         continue;
			  }
			/* all other exceptions */
			  else {
                              errno = EILSEQ;
                              ret_value = -1;
                       	      break;
			  }
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
/* DESCRIPTION : Free Memory Allocated for IBM-eucTW_IBM-eucCN            */
/*               Converter Descriptor                                     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd                                                       */
/* OUTPUT      : none                                                     */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static void     _iconv_close(iconv_t cd)
{
    if(cd)
        free(cd);
    else
        errno = EBADF;
}

/**************************************************************************/
/* FUNCTION    : _iconv_init                                              */
/* DESCRIPTION : Initialize IBM-eucTW_IBM-eucCN Converter-Specific        */
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
