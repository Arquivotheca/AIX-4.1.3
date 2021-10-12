static char sccsid[] = "@(#)88	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-PC_IBM-eucCN.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:19";
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
/* MODULE NAME:       IBM-PC_IBM-eucCN.c                                   */
/*                                                                         */
/* DESCRIPTIVE NAME:                                                       */
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

#include <stdlib.h>
#include <stdio.h>
#include "iconv.h"
#include "iconvP.h"
#include "host_lower.h"
#include "pc-euccn.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in IBM-PC Codeset                  */
/*               IBM-eucCN Codeset.                                       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : ret_value                                                */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int      _iconv_exec(_LC_host_lower_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out;      /* Pointer to in/out buffers */
        unsigned char   *e_in, *e_out;  /* Point the end of in/out buffers */
	unsigned short  *pc2in, pctmp, *euc2out, euctmp;
        int             ret_value;
	int		middle, start, end;

        if (!cd) {
            errno = EBADF; 
            return NULL;
        }

        if (!inbuf) return 0;

        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;

	pc2in  = &pctmp;
	euc2out = &euctmp;

        while (in < e_in) {
 /****************************
  Handle Singal Byte Character
  ****************************/
		if( in[0] <= 0x7f){
                    if (e_out - out < 1) {
                        errno = E2BIG; 
			ret_value = -1;
                        break;
                    }
		    if(e_in < in) break;
		    *out++ = *in++;
		     continue;
		}
			
 /**************************
  Handle Multibyte Character
  **************************/
                if (e_in - in < 2) {
                        errno = EINVAL; 
		        ret_value = -1;
                        break;
                }

                if (in[0] < 0x8c || 0xfe < in[0]) {
                        errno = EILSEQ; 
			ret_value = -1;
                        break;
                }
                if (in[1] < 0xa1 || 0xfe < in[1]) {
                        errno = EILSEQ; 
			ret_value = -1;
                        break;
                }
                if (e_out - out < 2) {
                        errno = E2BIG; 
			ret_value = -1;
                        break;
                }
 /****************
  Begin Converting
  ****************/
		*((unsigned char*)pc2in) = in[0];
		*((unsigned char*)pc2in + 1) = in[1];

		if(*pc2in >= 0x8ce0 && *pc2in <= 0x8cfe){
	            start = SBD_HEADER;
	            end   = SBD_TRAILER; 
	            middle= (end + start)/2; 
	            while((*pc2in != sbd[middle][0])&&(start <= end)) {
	    	         if((*pc2in) > sbd[middle][0]) {
			     start = middle + 1;
		         } 
		         else
		         if((*pc2in) < sbd[middle][0]) {
	                     end = middle-1;
		         }
		         middle = (end + start)/2;
		    };/* End of while for seraching */

		     if((*pc2in) == sbd[middle][0]) {
		         *(unsigned short*)euc2out= sbd[middle][1];
		         *out = *(( unsigned char*)euc2out);
		         *(out+1) = *(( unsigned char*)euc2out + 1);
		     }else{
                         errno = EILSEQ; 
			 ret_value = -1;
                         break;
                     }
		}else
		if(*pc2in >= 0x8da1 && *pc2in <= 0xa0fe){
	            start = UDC_HEADER;
	            end   = UDC_TRAILER; 
	            middle= (end + start)/2; 
	            while((*pc2in != udc[middle][0])&&(start <= end)) {
	    	         if((*pc2in) > udc[middle][0]) {
			     start = middle + 1;
		         } 
		         else
		         if((*pc2in) < udc[middle][0]) {
	                     end = middle-1;
		         }
		         middle = (end + start)/2;
		    };/* End of while for seraching */

		     if((*pc2in) == udc[middle][0]) {
		         *(unsigned short*)euc2out= udc[middle][1];
		         *out = *(( unsigned char*)euc2out);
		         *(out+1) = *(( unsigned char*)euc2out + 1);
		     }else{
                         errno = EILSEQ; 
			 ret_value = -1;
                         break;
		     }
		}else{
		         *out = *in;
		         *(out + 1) = *(in + 1);
		}
                in += 2;
                out += 2;
        }   /* end of while */

        *inbuf = in;
        *outbuf = out;
        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        return ret_value;
} /* end of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for IBM-PC_IBM-eucCN               */
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
/* DESCRIPTION : Initialize IBM-PC_IBM-eucCN Converter-Specific           */
/*               Descriptor                                               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : core_cd, toname, fromname                                */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static _LC_host_lower_iconv_t *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_host_lower_iconv_t *cd;

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
