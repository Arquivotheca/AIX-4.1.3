static char sccsid[] = "@(#)91	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-eucCN_IBM-936.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:25";
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
/* MODULE NAME:        IBM-eucCN_IBM-937.c                                 */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert Data Encoding in PC550                      */
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
#include "euccn-55.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in IBM-eucCN Codeset to            */
/*               PC550 Codeset.                                           */
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
        unsigned short  euc2, pc55, *euc2in, *euctmp;
        unsigned short  tmpbuf;
	unsigned short  indx;       /* Used to locate the positon in a array */
	short  		start, middle, end;

 /*
  Initialize specific converter descriptor failed
 */
       	if (!cd) {
   	    errno = EBADF; 
	    return NULL;
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

	euc2in = &tmpbuf;  

       	ret_value = 0;

       	while (in < e_in) { 
 /*
  Process ASCII characters
 */ 
	    while(in[0] <= 0x7e){
                out[0] = in[0];
	        out += 1;
	        in  += 1; 
		if(in >= e_in) break;
            };
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

            if (in[0] < 0xa1 || in[0] == 0xff){
                errno = EILSEQ;
                ret_value = -1;
                break;
            };

            if (in[1] < 0xa1 || in[1] == 0xff){
                errno = EILSEQ;
                ret_value = -1;
                break;
            };

  /*
   Process Chinese and Graphic characters
  */ 
            *((unsigned char *)euc2in) = in[0];
            *((unsigned char *)euc2in +1) = in[1];
  /*
   Locate Chinese characters
  */ 
            if ((*euc2in >= 0xb0a1) && (*euc2in <= 0xd7f9)){
              	indx = (unsigned short)((in[0] - 0xb0) * 94 + (in[1] - 0xa1)) ;
              	pc55 = cnto937chin1[indx];
	    }
	    else
            if ((*euc2in >= 0xd8a1 ) && (*euc2in <= 0xf7fe)){
              	indx = (unsigned short)((in[0] - 0xd8) * 94 + (in[1] - 0xa1)) ;
               	pc55 = cnto937chin2[indx];
	    }
	    else
  /*
   Locate Graphic characters
  */ 
            if ((*euc2in >= 0xa1a1 ) && (*euc2in <= 0xa1fe)){
              	indx = in[1] - 0xa1;
                pc55 = cnto937graphy1[indx];
            }
	    else
            if ((*euc2in >= 0xa2b1 ) && (*euc2in <= 0xa2e2)){
               	indx = in[1] - 0xb1;
               	pc55 = cnto937graphy2[indx];
	    } 
	    else
            if((*euc2in >= 0xa2e5 ) && (*euc2in <= 0xa2ee)){
              	indx = in[1] - 0xe5;
               	pc55 = cnto937graphy3[indx];
	    }
	    else
            if ((*euc2in >= 0xa2f1 ) && (*euc2in <= 0xa2fc)){
              	indx = in[1] - 0xf1;
               	pc55 = cnto937graphy4[indx];
	    }
	    else
            if ((*euc2in >= 0xa3a1 ) && (*euc2in <= 0xa4f3)){
              	indx = (unsigned short)((in[0] - 0xa3) * 94 + (in[1] - 0xa1)) ;
               	pc55 = cnto937graphy5[indx];
	    }
	    else
            if ((*euc2in >= 0xa5a1 ) && (*euc2in <= 0xa5f6)){
            	indx = in[1] - 0xa1;
               	pc55 = cnto937graphy6[indx];
	    }
	    else
            if ((*euc2in >= 0xa6a1 ) && (*euc2in <= 0xa6b8)){
              	indx = in[1] - 0xa1 ;
                pc55 = cnto937graphy7[indx];
	    }
	    else
            if ((*euc2in >= 0xa6c1 ) && (*euc2in <= 0xa6d8)){
              	indx = in[1] - 0xc1;
               	pc55 = cnto937graphy8[indx];
	    }
	    else
            if ((*euc2in >= 0xa7a1 ) && (*euc2in <= 0xa7c1)){
              	indx = in[1] - 0xa1;
              	pc55 = cnto937graphy9[indx];
	    } 
	    else
            if ((*euc2in >= 0xa7d1 ) && (*euc2in <= 0xa7f1)){
             	indx = in[1] - 0xd1 ;
              	pc55 = cnto937graphy10[indx];
	    } 
	    else
            if ((*euc2in >= 0xa8a1 ) && (*euc2in <= 0xa8aa)){
              	indx = in[1] - 0xa1;
              	pc55 = cnto937graphy11[indx];
	    }
	    else
            if ((*euc2in >= 0xa8c5 ) && (*euc2in <= 0xa8e9)){
              	indx = in[1] - 0xc5;
              	pc55 = cnto937graphy12[indx];
	    }
	    else
            if ((*euc2in >= 0xa9a4 ) && (*euc2in <= 0xa9ef)){
               	indx = in[1] - 0xa4;
              	pc55 = cnto937graphy13[indx];
	    } 
  /*
   Locate IBM Selected Characters
  */ 
            else if((*euc2in >= 0xfedc) && (*euc2in <= 0xfefe)){
               	indx = in[1] - 0xe0;
               	pc55 = cnto937graphy13[indx];
	    }
  /*
   Convert User-defined characters
  */
            else{
		pc55 = DEFAULTPC;
	    }

  /*
  Convert Characters Located Before
  */
            out[0] = (pc55 >> 8);
            out[1] = (pc55 & 0x00ff);
            in  += 2;
            out += 2;
	    continue;

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


