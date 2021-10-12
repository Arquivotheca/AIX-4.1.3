static char sccsid[] = "@(#)02	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/UTF-8_IBM-936.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:47";
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
/* MODULE NAME:        utf8_IBM-937.c                                      */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert Data Encoding in PC5550                     */
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
#include "ucs-pc5550.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in utf8 to PC5550 Codeset.         */
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
	wchar_t		ucs2in;
	unsigned short  utflen;
        unsigned short  *pc55, pctmp;
	unsigned short  begin, end, middle;

       	if (!cd) {
   	    errno = EBADF; return NULL;
	};

	if (!inbuf) return 0;

        /* Assign begin/end address of input buffer to in/e_in pointer */
       	e_in = (in = *inbuf) + *inbytesleft;  

 	/* Assign begin/end address of output buffer to in/e_in pointer */
       	e_out = (out = *outbuf) + *outbytesleft;

	/* Allocate a temporary buffer to hold pc5550 code*/
	pc55 = &pctmp;

	/* Initialize a return value */
       	ret_value = 0;

	/* Set destination codeset environment */
	(void)setlocale(LC_ALL, "UTF2" );

       	while(in < e_in){ 
  	    /* Convert ASCII Characters */
	    if(*in <= 0x7e){
		if(e_out - out < 1){
	       	errno = E2BIG;
                ret_value = -1;
                break;
                };
		switch(*in){
		    case 0x5C:
			      *out = 0xFE;
			      break;
		    case 0x7E:
			      *out = 0xFF;
			      break;
		    default:
			      *out = *in;
			      break;
		}
                out += 1;
		in  += 1;
		continue;
            };

  	    /* Handle various error case */ 
            if (e_in - in < mblen(in, MB_CUR_MAX)){   
		errno = EINVAL;
                ret_value = -1;
                break;
            };

            if (e_out - out < 2){ 
	       	errno = E2BIG;
                ret_value = -1;
                break;
            };

  	    /* Convert utf8 to ucs-2 using mbtowc() */
	    utflen = mbtowc(&ucs2in, in, MB_CUR_MAX);

	   /* Locate Hanzi and Graphy */
	    begin =  BEGIN;
	    end   =  END;
	    middle= (begin + end) / 2 ;

	   /* 
	      Use bisect algorithm to search the two-dimension array structure in
	      ucs-pc5550.h file. In this array, the first element is ucs code, the
	      second element is correspondent pc5550 code.
	      If can't find ucs code pattern, assign a default value(0x8140) to outbuf, 
	      otherwise assign the found pc code to output.
	      If the obtained pc code is a special code, then use switch clause to handle it
	   */
	    while(ucs2in!=ucstopc[middle][0]&&begin<=end){
		if(ucs2in > ucstopc[middle][0])
		    begin = middle + 1;
		else
		if(ucs2in < ucstopc[middle][0])
		    end = middle - 1;
		middle = (begin + end) / 2 ;
	    }
	    if(ucs2in!=ucstopc[middle][0])
		*pc55 = DEFAULTPC;
	    else 
		*pc55 = ucstopc[middle][1];
	    switch(*pc55){
		case 0x00AF:
		case 0x00A3:
		case 0x00AC:
			    *out = *((unsigned char*)pc55 + 1);
			    out += 1;
			    in += utflen;	
			    break;
            	default:
			    out[0] = *(unsigned char*)pc55;
	    	 	    out[1] = *((unsigned char*)pc55 + 1); 
	    		    out += 2;
	    		    in  += utflen;
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
/* DESCRIPTION : Free Memory Allocated for utf8_IBM-937                   */
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
/* DESCRIPTION : Initialize utf8_IBM-937 Converter-Specific          */
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


