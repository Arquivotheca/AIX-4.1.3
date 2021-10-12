static char sccsid[] = "@(#)79	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-837_IBM-1381.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:02";
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
/* MODULE NAME:        IBM-837_IBM-1381                                    */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert Data Encoding in DBCS                       */
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
#include "host-pcd.h"


/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in EBCDIC Multibytes to            */
/*               IBM-1381 Codeset.                                        */
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
        unsigned char   *in, *out;      /* Point to in/out buffers */
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             ret_value;      /* Hold the return value */
	unsigned short  *euc2in, euc2tmp, *host2in, host2tmp;
	short  		start, middle, end;

 /*
  Initialize specific converter descriptor failed
 */
	if(!cd) {
   	   errno = EBADF; return NULL;
	};

	if (!inbuf) return 0;
 /* 
  Assign begin/end address of input buffer to in/e_in pointer 
 */
       	e_in = (in = *inbuf) + *inbytesleft; 
 /* 
  Assign begin/end address of output buffer to in/e_in pointer 
 */
       	e_out = (out = *outbuf) + *outbytesleft;

       	ret_value = 0;

	euc2in = &euc2tmp;
	host2in = &host2tmp;

       	while (in < e_in) { 
  /*
   Convert Singal Byte Switch
  */
	    if(*in == 0x0f){
		errno = EILSEQ;
		ret_value = -1;
		break;
	     }

 /*
  Process input/output buffer 
 */ 
            if (e_in - in < 2){   
	       	errno = EINVAL;
              	ret_value = -1;
               	break;
            };

            if (e_out - out < 2){ 
	      	errno = E2BIG;
                ret_value = -1;
                break;
            };

 /*
  Process Chinese words one by one
 */ 
            *((unsigned char *)host2in) = in[0];
            *((unsigned char *)host2in +1) = in[1];
			
	    if(*host2in >= 0x4040 && *host2in <= 0x6c9f){
	        start  = HOST_START;
	        end    = HOST_TRAILER;
	        middle = (start + end) / 2; 

	        while((*host2in != hostpcd[middle][0])&&(start <= end)){
	            if((*host2in) > hostpcd[middle][0])
	                 start = middle + 1;
	            else
	            if((*host2in) < hostpcd[middle][0])
	    	         end = middle - 1;
		    middle = (end + start)/2;
	        };

	        if((*host2in) != hostpcd[middle][0]){
	            out[0] = DEFAULTPC;
	            out[1] = DEFAULTPC;
	            out += 2;
	            in  += 2;
		    continue;
	        };

	        if((*host2in) == hostpcd[middle][0]){
	            *euc2in = hostpcd[middle][1];
	            out[0] = *(unsigned char*)euc2in;
	            out[1] = *((unsigned char*)euc2in+1);
	            out += 2;
	            in  += 2;
	            continue;
	        };
	    }else if(*host2in>=0x7641&&*host2in<=0x7ffe){
	       start  = UDC_HEADER;
	       end    = UDC_TRAILER;
	       middle = (start + end) / 2; 
	       while((*host2in != udc[middle][0])&&(start <= end)) {
	    	    if((*host2in) > udc[middle][0]) {
			start = middle + 1;
		    } 
		    else
		    if((*host2in) < udc[middle][0]) {
	                end = middle-1;
		    }
		    middle = (end + start)/2;
		};/* End of while for seraching */

		if((*host2in) == udc[middle][0]) {
		    *(unsigned short*)euc2in= udc[middle][1];
		    *out = *(( unsigned char*)euc2in);
		    *(out+1) = *(( unsigned char*)euc2in + 1);
		    out += 2;
		    in  += 2;
		    continue;
		}else{
                    out[0] = DEFAULTPC1;
                    out[1] = DEFAULTPC2;
                    out += 2;
                    in  += 2;
                    continue;
		}
	    }else{
                errno = EINVAL;
                ret_value = -1;
                break;
	    }
        };/* End of While */

        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        return ret_value;

}/* End of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for IBM-eucCN_IBM-937              */
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
/* DESCRIPTION : Initialize IBM-eucCN_IBM-937 Converter-Specific          */
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


