static char sccsid[] = "@(#)82	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-837_UCS-2.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:09";
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
/* MODULE NAME:        IBM-837_UCS-2.c                                     */
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
#include "host-ucs.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in UCS-2 Codeset to                */
/*               EBCDIC Codeset.                                          */
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
	unsigned short  *host2in, host2tmp;
	wchar_t		*ucs2in, ucsin;
	unsigned short  start, middle, end;


	if(!cd){
	    errno = EBADF; 
	    return NULL;
	};

	if (!inbuf) return 0;

        /* Assign begin/end address of input buffer to in/e_in pointer */
	e_in = (in = *inbuf) + *inbytesleft; 

	/* Assign begin/end address of output buffer to in/e_in pointer */
	e_out = (out = *outbuf) + *outbytesleft;

	/* Initialize return value */
	ret_value = 0;

	/* Allocate temporary buffer to hold the EBCDIC code */
	host2in = &host2tmp;
	ucs2in  = &ucsin;

	/* Perform code conversion */
	while (in < e_in) {

	    /* Handle DBCS switch off designator */
	    if(in[0] == 0x0f){
	        ret_value = -1;
	        break;
	    };

	    /* Process errors used for DBCS */
            if (e_in - in < 2){
                errno = EINVAL;
                ret_value = -1;
                break;
            };

            if (e_out - out < MB_CUR_MAX){
                errno = E2BIG;
                ret_value = -1;
                break;
            };

            /* Begin conversion of hanzi and graphy characters */
            *((unsigned char *)host2in) = in[0];
            *((unsigned char *)host2in +1) = in[1];

	    /* 
	      First use bisect algorithm to locate
            */
	    if(*host2in >= 0x4040 && *host2in <= 0x6c9f){
                start  = BEGIN;
                end    = END;
                middle = (start + end) / 2;
                while((*host2in != hosttoucs[middle][0])&&(start <= end)){
                    if(*host2in > hosttoucs[middle][0])
                        start = middle + 1;
                    else
                    if(*host2in < hosttoucs[middle][0])
                        end = middle - 1;
                    middle = (end + start)/2;
                };

                if(*host2in != hosttoucs[middle][0])
	    	    *ucs2in = DEFAULTUCS;
	        else
                    *ucs2in = hosttoucs[middle][1];
	    }else if(*host2in >= 0x7641 && *host2in <= 0x7ffe){
                start  = BEGIN_UDC;
                end    = END_UDC;
                middle = (start + end) / 2;
                while((*host2in != udc[middle][0])&&(start <= end)){
                    if(*host2in > udc[middle][0])
                        start = middle + 1;
                    else
                    if(*host2in < udc[middle][0])
                        end = middle - 1;
                    middle = (end + start)/2;
                };

                if(*host2in != udc[middle][0])
	    	    *ucs2in = DEFAULTUCS;
	        else
                    *ucs2in = udc[middle][1];
	    }else
		*ucs2in = DEFAULTUCS;

	    out[0] = *(unsigned char*)ucs2in;
	    out[1] = *((unsigned char*)ucs2in + 1);
	    out += 2;
	    in  += 2;
    };/* End of While */

    *inbytesleft = e_in - in;
    *outbytesleft = e_out - out;
    *inbuf = in;
    *outbuf = out;
    return ret_value;

}/* End of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for utf2_IBM-937                   */
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
/* DESCRIPTION : Initialize utf2_IBM-937 Converter-Specific               */
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


