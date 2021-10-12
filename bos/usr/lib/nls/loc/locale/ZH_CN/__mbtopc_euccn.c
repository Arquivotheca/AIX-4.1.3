static char sccsid[] = "@(#)47	1.1  src/bos/usr/lib/nls/loc/locale/zh_CN/__mbtopc_euccn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:25:31";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __mbtopc_euccn
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
#include <stdlib.h>
#include <ctype.h>

#include "eucCN_ucs.h"

/* 
 * Convert a multi-byte string to process code for IBM_eucCN codeset
 *
 *    The algorithm for this conversion is:
 *        
 *    s[0] <= 0x7f:   PC = s[0]
 *    s[0] >= 0xa1 & s[0] <= 0xf7:  iconv(GB2312 --> UCS-2)  4e00-9fa8
 *    s[0] >= 0xf8 & s[0] <= 0xfe:  iconv(IBM-udcCN --> UCS-2) e000-f8ff
 */

size_t  __mbtopc_euccn(_LC_charmap_objhdl_t handle, wchar_t *pwc, char *s, size_t maxlen, int *err)
 {
	wchar_t dummy;
	unsigned short indx;
	int	start, end, middle;

	/**********
	  If s is NULL return 0
	 **********/
	if (s == (char *)NULL) return(0);

	/*********
	  If pwc is NULL, set it to dummy
	 *********/
        if (pwc == (wchar_t *)NULL) pwc = &dummy;

	/*********
	  Assume it is a bad character
	 *********/
	*err = 0;

	/*********
	  Single byte ( <=0x7f )
	 *********/
	if ((unsigned char)s[0] <= 0x7f) {
		if (maxlen < 1 ) {
		  *err = 1;
		  return(0);
		}
		else {	
		  *pwc = (wchar_t)s[0];
		  return(1);
		}
	}

	/********
	 GB2312 multi-byte
	 ********/
        
	else if ((unsigned char)s[0] >= 0xa1 && (unsigned char)s[0] <= 0xf7) {
             if (maxlen >= 2)  {
                  if ((unsigned char)s[0] == 0xa1 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xfe) {
                     indx = (unsigned short)((unsigned char)s[1] - 0xa1);
                     *pwc = (wchar_t)nonhanzi1[indx];
		     return(2);
                  }

                  else if ((unsigned char)s[0] == 0xa2 && (unsigned char)s[1] >= 0xb1 && (unsigned char)s[1] <= 0xe2) {
                     indx = (unsigned short)((unsigned char)s[1] - 0xb1);
                     *pwc = (wchar_t)nonhanzi2[indx];
		     return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa2 && (unsigned char)s[1] >= 0xe5 && (unsigned char)s[1] <= 0xee) {
                     indx = (unsigned short)((unsigned char)s[1] - 0xe5);
                     *pwc = (wchar_t)nonhanzi3[indx];
		     return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa2 && (unsigned char)s[1] >= 0xf1 && (unsigned char)s[1] <= 0xfc) {
                     indx = (unsigned short)((unsigned char)s[1] - 0xf1);
                     *pwc = (wchar_t)nonhanzi4[indx];
		     return(2);
		  }
 
                  else if ((unsigned char)s[0] == 0xa3 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xfe) {
                     indx = (unsigned short)((unsigned char)s[1] - 0xa1);
                     *pwc = (wchar_t)nonhanzi5[indx];
		     return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa4 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xf3) {
                     indx = (unsigned short)(94 + (unsigned char)s[1] - 0xa1);
                     *pwc = (wchar_t)nonhanzi5[indx];
		     return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa5 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xf6) {
                     indx = (unsigned short)((unsigned char)s[1] - 0xa1);
                     *pwc = (wchar_t)nonhanzi6[indx];
		     return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa6 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xb8) {
                      indx = (unsigned short)((unsigned char)s[1] - 0xa1);
                      *pwc = (wchar_t)nonhanzi7[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa6 && (unsigned char)s[1] >= 0xc1 && (unsigned char)s[1] <= 0xd8) {
                      indx = (unsigned short)((unsigned char)s[1] - 0xc1);
                      *pwc = (wchar_t)nonhanzi8[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa7 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xc1) {
                      indx = (unsigned short)((unsigned char)s[1] - 0xa1);
                      *pwc = (wchar_t)nonhanzi9[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa7 && (unsigned char)s[1] >= 0xd1 && (unsigned char)s[1] <= 0xf1) {
                      indx = (unsigned short)((unsigned char)s[1] - 0xd1);
                      *pwc = (wchar_t)nonhanzi10[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa8 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xba) {
                      indx = (unsigned short)((unsigned char)s[1] - 0xa1);
                      *pwc = (wchar_t)nonhanzi11[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa8 && (unsigned char)s[1] >= 0xc5 && (unsigned char)s[1] <= 0xe9) {
                      indx = (unsigned short)((unsigned char)s[1] - 0xc5);
                      *pwc = (wchar_t)nonhanzi12[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] == 0xa9 && (unsigned char)s[1] >= 0xa4 && (unsigned char)s[1] <= 0xef) {
                      indx = (unsigned short)((unsigned char)s[1] - 0xa4);
                      *pwc = (wchar_t)nonhanzi13[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] >= 0xb0 && (unsigned char)s[0] <= 0xd6 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xfe) {
                      indx = (unsigned short)(((unsigned char)s[0] - 0xb0)*94 + (unsigned char)s[1] - 0xa1);
                      *pwc = (wchar_t)hanzi1[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] == 0xd7 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xf9) {
                      indx = (unsigned short)(((unsigned char)s[0] - 0xb0)*94 + (unsigned char)s[1] - 0xa1);
                      *pwc = (wchar_t)hanzi1[indx];
		      return(2);
		  }

                  else if ((unsigned char)s[0] >= 0xd8 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xfe) {
                      indx = (unsigned short)(((unsigned char)s[0] - 0xd8)*94+((unsigned char)s[1] - 0xa1));
                      *pwc = (wchar_t)hanzi2[indx];
		      return(2);
		  }

        /************
	 IBM-udcCN multibyte ( 0xa2a1 - 0xf7fe )
 	 ************/
		  else if ((unsigned char)s[0] >= 0xa2 && (unsigned char)s[0] <= 0xa9 && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xfe) {

		      start = UDC_HEADER;  
		      end = UDC_TRAILER;
		      middle = ( start + end ) / 2;

		      while( *(unsigned short *)s != udc_index[middle] && start <= end ) {
		 	   if( *(unsigned short *)s < udc_index[middle] ) {
				 end = middle - 1;
		   		 middle = ( start + end ) / 2;
		      	   }
		      	   else {
			  	start = middle + 1;
			  	middle = ( start + end ) / 2;
		      	   }
		      }

		      /*********
		      otherwise it is an invalid multibyte
		      **********/
		      if(start > end) {
			    *err = -1;
			    return(0);
		      }		
		      else {
			    *pwc = (wchar_t)(0xe000 + middle);
			    return(2);
		      }
		  }

		  else if( s[0] >= 0xaa && s[0] <= 0xaf && (unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xfe) {
                      indx = (unsigned short)(((unsigned char)s[0] - 0xaa)*94+((
unsigned char)s[1] - 0xa1));
                      *pwc = (wchar_t)(0xe0a4 + indx);
                      return(2);
                  }

                  else if( s[0] == 0xd7 && (unsigned char)s[1] >= 0xfa && (unsigned char)s[1] <= 0xfe) {
                      indx = (unsigned short)(((unsigned char)s[0] - 0xd7)*94+((unsigned char)s[1] - 0xfa));
                      *pwc = (wchar_t)(0xe2d8 + indx);
                      return(2);
                  }
	     }
	     else {
		*err = 2;
		return(0);
	     }
	}

        /************
	 IBM-udcCN multibyte
 	 ************/
	else if ((unsigned char)s[0] >= 0xf8 && (unsigned char)s[0] <= 0xfd) {
             if (maxlen >= 2)  {
		if ((unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xfe) {
                   indx = (unsigned short)(((unsigned char)s[0] - 0xf8)*94+((unsigned char)s[1] - 0xa1));
		   *pwc = (wchar_t)(0xe2dd + indx);
		   return(2);
                } 
	     }
	     else {
		*err = 2;
		return(0);
	     }
	}

	else if ((unsigned char)s[0] == 0xfe) {
             if (maxlen >= 2)  {
		if ((unsigned char)s[1] >= 0xa1 && (unsigned char)s[1] <= 0xdf) {
                   indx = (unsigned short)((unsigned char)s[1] - 0xa1);
		   *pwc = (wchar_t)(0xe511 + indx);
		   return(2);
                } 

            /**************
            IBM-sbdCN special characters
	    **************/
		else if ((unsigned char)s[1] >= 0xe0 && (unsigned char)s[1] <= 0xfe) {
                     indx = (unsigned short)((unsigned char)s[1] - 0xe0);
                     *pwc = (wchar_t)ibmsbd[indx];
		     return(2);
 	        }
	     }
	     else {
		*err = 2;
		return(0);
	     }
	}


	/*********
	 If we are here invalid mb character
	 *********/

	 *err = -1;
	 return(0);
}	
