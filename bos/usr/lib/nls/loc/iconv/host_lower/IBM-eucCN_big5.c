static char sccsid[] = "@(#)94	1.1  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-eucCN_big5.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:04:31";
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
/* MODULE NAME:        IBM-eucCN_BIG5.c                                    */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert data encoding in IBM-eucCN to BIG5          */
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
#include "euccn-big5.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in IBM-eucCN Codeset to            */
/*               BIG5 Codeset                                             */
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
      unsigned char   *in, *out;   /* Point to in/out buffers */
      unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
      int             ret_value;      /* Hold the return value */
      unsigned short  *ucs, *euc2in, *outtmp;
      unsigned short  tmpbuf, outbuf1, ucsbuf;
      unsigned short  indx;       /* Used to locate the positon in a array */
      short	      middle, start, end;
      int             indx1, flag;
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
      euc2in = &tmpbuf;
      outtmp = &outbuf1;
      ucs = &ucsbuf;
      ret_value = 0;
      while (in < e_in) { 
 /*
  Convert Characters in EUC CS0 of CN
 */ 
           if( in[0] <= 0x7F ) {
		if(e_out - out < 1){
                    errno = E2BIG;
                    ret_value = -1;
                    break;
           	};
               	*out++ = *in++;
                if(in >= e_in) break;
		continue;
           };
 /*
  Handle Various Aspect May Encounter
 */ 
           if (e_in - in < 2) {   
	       errno = EINVAL;
               ret_value = -1;
               break;
           };

           if (e_out - out < 2) { 
               errno = E2BIG;
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
			
  /*
   Process Chinese and Graphic characters one by one
  */
           *((unsigned char *)euc2in) = in[0];
           *((unsigned char *)euc2in +1) = in[1];
           if ((*euc2in >= 0xb0a1) && (*euc2in <= 0xf7fe)) {
  /*
   Convert One Hanzi of CN to BIG5
  */ 
               indx = (unsigned short)((in[0]-0xb0)*94+(in[1]-0xa1)) ;
               *ucs = cntobig5han1[indx];
	       if(*(unsigned char*)ucs == 0x0) {  
                    if (e_out - out < 2) { 
		        errno = E2BIG;
                        ret_value = -1;
		        statue = 1;
                        break;
                    };
                    if(statue == 1||statue == 0) {
		        out[0] = LEFTMARK1;
		        out[1] = LEFTMARK2;
		        out += 2;
			statue = 0;
		    };
		    indx1 = 0;
		    flag = 0;
		    while(indx1 <= MANYEND){
			if(*euc2in == many[indx1][0] && statue !=3 ){
		             *ucs = many[indx1][1];
                             if (e_out - out < 2) { 
		                 errno = E2BIG;
                                 ret_value = -1;
			         statue = 2;
                                 break;
                             };
                             if (statue==2||statue==0) {
		                  out[0] = *(unsigned char*)ucs;
		                  out[1] = *((unsigned char*)ucs+1);
		                  out += 2;
				  statue = 0;
			      }
				  indx1 += 1;
				  flag = 1;
		        } else{
				if(flag == 1){
                    		    if (e_out - out < 2) { 
		        	        errno = E2BIG;
                        	        ret_value = -1;
					statue == 3;
                        	        break;
                    		    };
                                    if (statue == 0||statue==3){ 
		    		        out[0] = RIGHTMARK1;
		    		        out[1] = RIGHTMARK2;
		    		        out += 2;
				        in += 2;
				        statue = 0;
                                    };
				    break;
				}else{
				    indx1 += 1;
				    flag = 0;
				}
	    		    }
	              }
		      if( statue==2||statue==3) break;
	        }else{
		     out[0] = *(unsigned char*)ucs;
		     out[1] = *((unsigned char*)ucs+1);
		     out += 2;
	             in += 2;
               };
	    }
	    else
	    if((*euc2in >= 0xa1a1) && (*euc2in <= 0xa869)) {
  /*
   Convert Graphic of CN to BIG5
  */
	       start = HEADER;
	       end = TRAILER; 
	       middle = (end + start)/2; 
	       while((*euc2in != euctobig5non[middle][0])&&(start <= end)) {
	    	    if((*euc2in) > euctobig5non[middle][0]) 
			start = middle + 1;
		    else
		    if((*euc2in) < euctobig5non[middle][0]) 
	                end = middle-1;
		    middle = (end + start)/2;
		};/* End of while for seraching */

		if((*euc2in) != euctobig5non[middle][0]) 
			*ucs = DEFAULTBIG5;
		else
		if((*euc2in) == euctobig5non[middle][0]) 
		    *(unsigned short*)ucs = euctobig5non[middle][1];

	        out[0] = *(unsigned char*)ucs;
	        out[1] = *((unsigned char*)ucs+1);
	        out += 2;
	        in  += 2;
	  }
  /*
   Convert Characters Rested
  */
	  else {
		*ucs = DEFAULTBIG5;
	        out[0] = *(unsigned char*)ucs;
	        out[1] = *((unsigned char*)ucs+1);
	        out += 2;
	        in  += 2;
	  };
     }; /* The end of while */

     *inbytesleft = e_in - in;
     *outbytesleft = e_out - out;
     *inbuf = in;
     *outbuf = out;
     return ret_value;

}

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for IBM-eucCN_BIG5                 */
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
/* DESCRIPTION : Initialize IBM-eucCN_BIG5 Converter_Specific             */
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
