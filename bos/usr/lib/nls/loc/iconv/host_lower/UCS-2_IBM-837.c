static char sccsid[] = "@(#)99	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/UCS-2_IBM-837.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:42";
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
/* MODULE NAME:        UCS-2_IBM-837.c                                     */
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
#include "ucs-host.h"

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
    wchar_t	    ucs2in, *ucsin;
    unsigned short  *host2in, hosttmp;
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

    /* Allocate temporary buffer to hold EBCDIC code */
    host2in = &hosttmp;
    ucsin   = &ucs2in;

    /* Perform code conversion.*/
       
    while (in < e_in) {

	/* Process various error used for DBCS */
        if (e_out - out < 2){
            errno = E2BIG;
            ret_value = -1;
            break;
        };

        if (e_in - in < 2){
            errno = EINVAL;
            ret_value = -1;
            break;
        };

	*(unsigned char *)ucsin = in[0];
	*((unsigned char*)ucsin + 1) = in[1];
	/* 
	   Use bisect algorithm to search for the EBCDIC code in ucs-host.h, where
           ucstohost[middle][0] is ucs code, ucstohost[middle][1] is pc code. If
	   find the ucs code, then assign correspond EBCDIC code to outbuf, otherwise
	   assign default value to outbuf.
	*/ 
	if(*ucsin <= 0x00A3){
	    errno = EILSEQ;
	    ret_value = -1;
	    break;
	};

	if((ucs2in >= 0x00A3&&ucs2in <= 0x9ffe)||(ucs2in >= 0xff01 && ucs2in <= 0xfffe)){
            start  = BEGIN;
            end    = END;
            middle = (start + end) / 2;
            while((ucs2in != ucstohost[middle][0])&&(start <= end)){
                if((ucs2in) > ucstohost[middle][0])
                     start = middle + 1;
                else
                if((ucs2in) < ucstohost[middle][0])
                     end = middle - 1;
                middle = (end + start)/2;
            };

            if((ucs2in) != ucstohost[middle][0])
	        *host2in = DEFAULTHOST;
	    else
                *host2in = ucstohost[middle][1];
	}else if(ucs2in >= 0xe000 && ucs2in <= 0xeffe){
            start  = BEGIN_UDC;
            end    = END_UDC;
            middle = (start + end) / 2;
            while((ucs2in != udc[middle][0])&&(start <= end)){
                if((ucs2in) > udc[middle][0])
                     start = middle + 1;
                else
                if((ucs2in) < udc[middle][0])
                     end = middle - 1;
                middle = (end + start)/2;
            };

            if((ucs2in) != udc[middle][0])
	        *host2in = DEFAULTHOST;
	    else
                *host2in = udc[middle][1];
	}else
	    *host2in = DEFAULTHOST;
         
	if(*host2in == 0x00A1||*host2in == 0x004A||*host2in == 0x005F||*host2in == 0x5B){
	    errno = EILSEQ;
	    ret_value = -1;
            break;
	}
	else{
	    out[0] = *(unsigned char*)host2in;
	    out[1] = *((unsigned char*)host2in + 1);
	    out += 2;
	}
	in += 2;
    };/* End of While */

    *inbytesleft = e_in - in;
    *outbytesleft = e_out - out;
    *inbuf = in;
    *outbuf = out;
    return ret_value;

}/* End of _iconv_exec */

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for utf2_IBM-937              */
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


