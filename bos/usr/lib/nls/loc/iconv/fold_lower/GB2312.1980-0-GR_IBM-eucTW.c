static char sccsid[] = "@(#)24	1.2  src/bos/usr/lib/nls/loc/iconv/fold_lower/GB2312.1980-0-GR_IBM-eucTW.c, ils-zh_CN, bos41J, 9514A_all 3/28/95 14:47:10";
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
/* MODULE NAME:        GB2312.1980-0-GR_IBM-eucTW.c                        */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert data encoding in EUC-CN to EUC-TW           */
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
#include <iconv.h>
#include <iconvP.h>
#include "fold_lower.h"
#include "euccn-euctw.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in GB2312.1980-0-GR Codeset to     */
/*               IBM-eucTW Codeset.                                       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : err_flag                                                 */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int _iconv_exec (_LC_fold_lower_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out;   /* Point to input and output buffers */
        unsigned char   *e_in, *e_out;  /* Point to the end of input and output buffers */
        int             err_flag = FALSE;   /* Hold the return value */
        unsigned short  *euc2in;
        unsigned short  inbuff, nonbuff, *nonbuf;
	unsigned short  indx;       /* used to locate the positon in a array */
	short  		middle, start, end;
	unsigned int    hanbuff, *hanbuf;
	int		flag;
	static short    statue;

        if (!cd) {
             errno = EBADF; return NULL;
        };

        if (!inbuf) return 0;
 /*
  Initialize Various Pointers
 */ 
 	e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
	euc2in = &inbuff;
	nonbuf = &nonbuff;
	hanbuf = &hanbuff;


        while (in < e_in) { 
 /*
  To convert characters in EUC CS0 set
 */
            if (e_in - in < 2) {
                    if (e_in - in == 1) {
                            if ((in[0] < 0xa1 || 0xfe < in[0]) &&
                                   (in[0] < 0x21 || 0x7e < in[0])){
                                 errno = EILSEQ; 
				 err_flag = TRUE;
                            }else{
                                 errno = EINVAL; 
				 err_flag = TRUE;
                            }
                    }
                    break;
            }

/*
 Process invalid characters
*/
            if (in[0] < 0xa1 || in[0] == 0xff) {
                 errno = EILSEQ;
                 err_flag = TRUE;
                 break;
            };

            if (in[1] < 0xa1 || in[1] == 0xff) {
                 errno = EILSEQ;
                 err_flag = TRUE;
                 break;
            };

            *((unsigned char *)euc2in) = in[0];
            *((unsigned char *)euc2in +1) = in[1];
 /*
  Process Chinese or Graphic characters
 */ 
            if ((*euc2in >= 0xb0a1) && (*euc2in <= 0xf7fe)) {
                indx = (unsigned short)((in[0]-0xb0)*94+(in[1]-0xa1)) ;
		*hanbuf = cntotwhan[indx];
 /*
  Convert Characters to CNS-1
 */
		if(*((unsigned char*)hanbuf+1) == 0x10) {
                    if (e_out - out < 2) { 
		        errno = E2BIG;
                        err_flag = TRUE;
                        break;
                    };
		    out[0] = *((unsigned char*)hanbuf+2);
		    out[1] = *((unsigned char*)hanbuf+3);
		    out += 2;
		    in  += 2;
		} else
 /*
  Convert characters to CNS-2
 */ 
		if(*((unsigned char*)hanbuf+1) == 0x20) {
                    if (e_out - out < 4) { 
		        errno = E2BIG;
                        err_flag = TRUE;
                        break;
                    };
		    out[0] = 0x8e;
		    out[1] = 0xa2;
		    out[2] = *((unsigned char*)hanbuf+2);
		    out[3] = *((unsigned char*)hanbuf+3);
		    out += 4;
		    in  += 2;
		}
 /*
  Convert one-to-many characters
 */
		else if(*((unsigned char*)hanbuf+1) == 0x0) {  
                    if (e_out - out < 2) { 
		        errno = E2BIG;
                        err_flag = TRUE;
			statue = 1;
                        break;
                    };
		    if (statue == 0 || statue == 1){
		        out[0] = LEFTMARK1;
		        out[1] = LEFTMARK2;
		        out += 2;
			statue = 0;
		     };
		        indx = 0;
		        flag = 0;
		    while(indx <= MANYEND){
			if(*euc2in == many[indx][0]){
		             *hanbuf =  many[indx][1];
		             if(*((unsigned char*)hanbuf+1) == 0x10) {
                                  if (e_out - out < 2) { 
		                      errno = E2BIG;
                                      err_flag = TRUE;
				      statue = 2;
                                      break;
                                  };
				  if (statue == 0 || statue == 2){
		                      out[0] = *((unsigned char*)hanbuf+2);
		                      out[1] = *((unsigned char*)hanbuf+3);
		                      out += 2;
				      statue = 0;
				  };
				      indx += 1;
				      flag = 1;
			      } else if(*((unsigned char*)hanbuf+1) == 0x20) {
                    		  if (e_out - out < 4) { 
		        	      errno = E2BIG;
                        	      err_flag = TRUE;
				      statue = 3;
                        	      break;
                    	          };
				  if (statue == 3||statue == 0){
		    		      out[0] = 0x8e;
		    		      out[1] = 0xa2;
		    	              out[2] = *((unsigned char*)hanbuf+2);
		    		      out[3] = *((unsigned char*)hanbuf+3);
		    		      out += 4;
				      statue = 0;
				 };
				  indx += 1;
				  flag = 1;
				}
			    }else{
				if(flag == 1){
                    		    if (e_out - out < 2) { 
		        	        errno = E2BIG;
                        	        err_flag = TRUE;
					statue = 4;
                        	        break;
                    		    };
		    		    out[0] = RIGHTMARK1;
		    		    out[1] = RIGHTMARK2;
		    		    out += 2;
				    in += 2;
				    statue = 0;
				    break;
				}else{
				    indx += 1;
				    flag = 0;
				}
	    		     }
			 }
		     }else{
		        if(e_out - out < 2) {
		            errno = E2BIG;
                            err_flag = TRUE;
                            break;
                        }
		        out[0] = DEFAULTTW1;
		        out[1] = DEFAULTTW2;
		        out += 2;
		        in  += 2;
		     }
		}else
 /*
  Convert Graphic Character in CS1
 */ 
		if((*euc2in >= 0xa1a1) && (*euc2in <= 0xa8e9)) {
                    if (e_out - out < 2) { 
		       	errno = E2BIG;
                       	err_flag = TRUE;
                       	break;
                    };
		    start = HEADER;
		    end = TRAILER; 
		    middle = (end + start)/2; 
		    while((*euc2in != cntotwnon[middle][0])&&(start <= end)) {
		   	if((*euc2in) > cntotwnon[middle][0]) 
			    start = middle + 1;
			else
			if((*euc2in) < cntotwnon[middle][0]) 
			    end = middle - 1;
			middle = (end + start)/2;
		    };/* End non-hanzix of while for seraching */

		    if(*euc2in == cntotwnon[middle][0]) {
		        *nonbuf = cntotwnon[middle][1];
			out[0] = *((unsigned char*)nonbuf);
			out[1] = *((unsigned char*)nonbuf+1);
			out += 2;
			in  += 2;
		    }else{
		        out[0] = DEFAULTTW1;
		        out[1] = DEFAULTTW2;
		        out += 2;
		        in  += 2;
		    }
	      }
	      else {
		    if(e_out - out < 2) {
		        errno = E2BIG;
                        err_flag = TRUE;
                        break;
                    }
 /*
  Process rested characters
 */ 
		    out[0] = DEFAULTTW1;
		    out[1] = DEFAULTTW2;
		    out += 2;
		    in  += 2;
		}
	}; /* End of while*/

 /*
  Adjust Various Pointer
 */
        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        if ( err_flag ) return -1;
	else return 0;

 }
 
 /**************************************************************************/
 /* FUNCTION    : _iconv_close                                             */
 /* DESCRIPTION : Free Memory Allocated for GB2312.1980-0-GR_IBM-eucTW     */
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
 /* DESCRIPTION : Initialize GB2312.1980-0-GR_IBM-eucTW Converter-Specific */
 /*               Descriptor                                               */
 /* EXTERNAL REFERENCES:                                                   */
 /* INPUT       : core_cd, toname, fromname                                */
 /* OUTPUT      : cd                                                       */
 /* CALLED      : iconv()                                                  */
 /* CALL        :                                                          */
 /**************************************************************************/
static _LC_fold_lower_iconv_t       *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_fold_lower_iconv_t      *cd;

        if (!(cd = (_LC_fold_lower_iconv_t *) malloc(sizeof(_LC_fold_lower_iconv_t))))
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
