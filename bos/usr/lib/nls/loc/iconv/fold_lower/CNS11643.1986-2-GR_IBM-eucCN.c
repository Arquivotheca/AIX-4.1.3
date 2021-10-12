static char sccsid[] = "@(#)14	1.2  src/bos/usr/lib/nls/loc/iconv/fold_lower/CNS11643.1986-2-GR_IBM-eucCN.c, ils-zh_CN, bos41J, 9514A_all 3/28/95 14:47:03";
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
/* MODULE NAME:        CNS11643.1986-2-GR_IBM-eucCN.c                      */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert data encoding in CNS2 to IBM-eucCN          */
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
#include "cns22-euccn.h"
/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in CNS2 Codeset to                 */
/*               IBM-eucCN Codeset.                                       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : err_flag                                                 */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int _iconv_exec(_LC_fold_lower_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out; /* Point to in/out buffers */
	unsigned short  *outtmp;
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             err_flag = FALSE;      /* Hold the return value */
        unsigned short  *euc2in, tmpbuf1, tmpbuf;
	short		middle, start, end;
	

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
		outtmp =  &tmpbuf1;


        	while (in < e_in) { 

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

                        if (e_out - out < 2) {
			    errno = E2BIG;
                            err_flag = TRUE;
                            break;
                        }

                        if (in[0] < 0xa1 || in[0] == 0xff) {
                            errno = EILSEQ;
                            err_flag = TRUE;
                            break;
                        }

                        if (in[1] < 0xa1 || in[1] == 0xff) {
                            errno = EILSEQ;
                            err_flag = TRUE;
                            break;
                        }
                	*((unsigned char *)euc2in) = in[0];
                	*((unsigned char *)euc2in +1) = in[1];
 /***************************
  Obtain One Input Hanzi word
  ***************************/
			if((*euc2in >= 0xa1a1) && (*euc2in <= 0xfefe)) {
			    start  = HEADER;
			    end    = TRAILER; 
			    middle = (end + start) / 2; 
 /********************
  Locate in data array
  ********************/
			    while((*euc2in != cns2toeuccnhan[middle][0])&&(start <= end)) {
			        if((*euc2in) > cns2toeuccnhan[middle][0]) {
					start = middle + 1;
				} 
				else
				if((*euc2in) < cns2toeuccnhan[middle][0]) {
					end = middle-1;
				}
				middle = (end + start)/2;
			    };
			   if((*euc2in) != cns2toeuccnhan[middle][0]) {
			       *(unsigned short*)outtmp = DEFAULTCN;
			   }

			   if((*euc2in) == cns2toeuccnhan[middle][0]) {
			       *(unsigned short*)outtmp = cns2toeuccnhan[middle][1];
			    }
		       }else{
                            errno = EILSEQ;
                            err_flag = TRUE;
                            break;
                         }
         	out[0] = *(unsigned char*)outtmp;
    		out[1] = *((unsigned char*)outtmp+1);
	    	out += 2;
    		in  += 2;
	    }/* The end of while */

        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        if ( err_flag ) return -1;
	return 0;
}
/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for CNS11643.1986-2-GR_IBM-eucCN      */
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
/* DESCRIPTION : Initialize CNS11643.1986-2-GR_IBM-eucCN Converter_Specific  */
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
