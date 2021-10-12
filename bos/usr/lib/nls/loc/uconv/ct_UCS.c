static char sccsid[] = "@(#)12  1.2  src/bos/usr/lib/nls/loc/uconv/ct_UCS.c, ils-zh_CN, bos41J, 9514A_all 3/28/95 15:12:36";
/*
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		_ct_UCS_init
 *			_ascii_to_ucs
 *			_ct_UCS_exec
 *			instantiate
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>
#include <iconv.h>
#include <iconvP.h>
#include <uconv.h>
#include "fcs.h"
#include "ct.h"

extern	int _ctUCS_close();
extern	int _iconv_close();
extern	size_t ct_UCS_exec();


/*
 *   NAME:	_ascii_to_ucs
 *
 *   FUNCTION:	Conversion from ASCII to UCS-2.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static
size_t	_ascii_to_ucs (
	uchar_t **in_buf,  size_t *in_left, 
	uchar_t **out_buf, size_t *out_left) {

	uchar_t		*in_ptr, *in_buf_end,
			*out_ptr, *out_buf_end;
	size_t		errfl = 0;


	in_buf_end  = (in_ptr  = *in_buf)  + *in_left;
	out_buf_end = (out_ptr = *out_buf) + *out_left;

	while (in_ptr < in_buf_end) {

		if ((*in_ptr > 0x7f) || (*in_ptr < 0x1f)) {
			errno = EILSEQ;
			errfl = (size_t)-1;
			break;
		}
		if ((out_buf_end - out_ptr) < sizeof(UniChar)) {
			errno = E2BIG;
			errfl = (size_t)-1;
			break;
		}
		*(out_ptr ++) = 0;
		*(out_ptr ++) = *(in_ptr ++);
	}
	
	*in_buf   = in_ptr;
	*out_buf  = out_ptr;
	*in_left  = in_buf_end  - in_ptr;
	*out_left = out_buf_end - out_ptr;

	return errfl;
}

/*
 *   NAME:      _ct_UCS_exec
 *
 *   FUNCTION:  Push ctCharsetDesc onto tail of cd's list of ctCharsetDesc
 *
 *   RETURNS:   ctCharsetDesc - malloc ok, return predecessor!
 *              Null          - Error.
 *
 */

static	ctCharsetDesc PushEscTbl (
	_LC_ucs_ct_iconv_t *cd, 
	EscTbl* et)
{
        ctCharsetDesc tmpCharset;
        ctCharsetDesc p = cd->top;

       	if ( (tmpCharset = (ctCharsetDesc) 
				malloc ( sizeof(ctCharsetDescRec))) == NULL) {
			return (ctCharsetDesc)NULL;
		}
	tmpCharset->conv = (iconv_t)NULL;
	tmpCharset->func = (ctUCS_Func)NULL;
	tmpCharset->next = (ctCharsetDesc)NULL;
	tmpCharset->free = True;
 	memcpy( &(tmpCharset->escape), et, sizeof(EscTbl));

	while (p->next) p = p->next;
	p->next = tmpCharset;
	return (p);
}


/*
 *   NAME:      _ct_UCS_exec
 *
 *   FUNCTION:  Conversion to ct to UCS-2.
 *
 *   RETURNS:   >= 0    - Number of non-identical conversions performed.
 *              -1      - Error.
 *
 */

static	size_t	_ct_UCS_exec (
	_LC_ucs_ct_iconv_t *cd, 
	const uchar_t **inbuf,  size_t *inbytesleft, 
	uchar_t **outbuf, size_t *outbytesleft) {

	const uchar_t 	*in, *e_in;
	uchar_t		*out, *e_out;
	size_t		inlen, outlen, ret_value;
	int		i;
	EscTbl		*etbl;
						/* Used for swapping endian*/
	uchar_t		*ucs_buf = *outbuf;
	size_t		ucs_size = *outbytesleft;

	if ((cd == NULL) || (cd == (_LC_ucs_ct_iconv_t*)-1)) {
		errno = EBADF; return -1;
	}
	if (inbuf == NULL) {
		if (_ctUCS_reset_state (cd, NULL, NULL))
			return (size_t)0;
		else	return (size_t)-1;
	}

	while (*inbytesleft) {

		ctCharsetDesc current = cd->current;
		if (current->func) {
			ret_value = (*current->func) (
				inbuf, inbytesleft, outbuf, outbytesleft);
		}
		else {
			if (current->conv == NULL) {
				if ( (current->conv = iconv_open 	
					(DEF_UCS_NAME, 
					current->escape.name)) == (iconv_t)-1) {
					ret_value = (size_t)-1;
					break;
				}
			}
			ret_value = iconv (current->conv,
				inbuf, inbytesleft, outbuf, outbytesleft);
		}
		if (ret_value != -1) 
			break;  /* Conversion complete */
		else if (errno == E2BIG) 
			break;  /* charset ok, outbuf too small */
		/* else
		 *	          EINVAL 
		 *		  - charset ok, but check next byte - GL/GR? 
		 *                EILSEQ
		 *	          - charset not ok, but check next byte - GL/GR?
		 *		  ret_value = -1 & errno set 
		 */
		
		in = *inbuf;
		if (*in == 0x1b) {
			ctCharsetDesc last = cd->current;
			ctCharsetDesc current = cd->current;
			ctCharsetDesc incomplete_found = (ctCharsetDesc)NULL;
			EscTblTbl* ett = cd->ett;
			
			ret_value = 0;
			/* until found or unrecognized */
			for ( current = cd->top; 1 ;
						current = current->next) {
				size_t len;
			        if ( current == (ctCharsetDesc)NULL ) {

				    /* 
				     * iso2022.cfg exhausted... go fishing
				     * for other esc seq in cd->ett
				     */
				    EscTbl* et = 
					_iconv_FindWellKnownEscape( cd, &ett,
					    in, *inbytesleft);
				    if ( et ) {
					if ( et->len > *inbytesleft )
					    incomplete_found = (ctCharsetDesc)True;
					else if ( (current = PushEscTbl( cd, et))
						  == (ctCharsetDesc)NULL) 
					    break; /* something wrong */
				     /* else try returned current */
					continue;
				    }
				    else /* That's it - unknown -> EILSEQ */
					break;
				}    
				len = current->escape.len;
				if ( len > *inbytesleft )
					len = *inbytesleft;
				if (memcmp(*inbuf + 1, 
					   &current->escape.str[1],
					   len - 1))
					continue;
				if ( len != current->escape.len ) {
					incomplete_found = current;
					continue;
				}
				if (!current->escape.seg) {
					*inbuf += current->escape.len;
					*inbytesleft -= current->escape.len;
					if (current->escape.gl)
						cd->gl = current;
					else
						cd->gr = current;
					cd->current = current;
					break;
				}
				if (*inbytesleft < 2) {
					incomplete_found = current ; continue;
				}
				in = *inbuf + current->escape.len;
				inlen = (in[0] & 0x7f) * 128 + (in[1] & 0x7f);
				if (*inbytesleft < inlen + current->escape.len + 2){
					errno = EINVAL; ret_value = (size_t)-1;
					break;
				}
				/* The following is not needed... f.rojas
				if (inlen < current->escape.seglen){
					errno = EINVAL; ret_value = (size_t)-1;
					break;
				}
				*/
				if (memcmp(in + 2,
					current->escape.seg, current->escape.seglen))
					continue;
				in += current->escape.seglen + 2;
				inlen -= current->escape.seglen;
				out = *outbuf;
				outlen = *outbytesleft;
				if ( current == &utfCharset ) {
					if ( inlen > outlen ) {
						errno = EINVAL; 
						ret_value = (size_t)-1;
						break;
					}
					outlen /= sizeof(UniChar);
					ret_value = __iconv_utf2ucs( 
						&in, &inlen, 
						(UniChar*)&out, &outlen);
					outlen = *outbuf + *outbytesleft - out;
				}
				else {
				    if (current->conv == NULL) {
					if ( (current->conv = iconv_open 	
					      (DEF_UCS_NAME, 
					       current->escape.name)) == (iconv_t)-1) {
					    ret_value = (size_t)-1;
					    break;
				        }
				    }
				    ret_value = iconv (current->conv, &in, &inlen, &out, &outlen);
				}
				inlen = in - *inbuf;
				*inbuf = in;
				*inbytesleft -= inlen;
				*outbuf = out;
				*outbytesleft = outlen;
				break;
			}
			if (!current){
				if ( incomplete_found )
					errno = EINVAL;
				else
					errno = EILSEQ; 
				ret_value = (size_t)-1;
			}
			if ( ret_value != 0 ) break;
		}
		else if (cd->isctl[*in]) {  /* Test for C0/C1 characters */
			UniChar* ucs;
			ret_value = 0;
		        if (cd->isctl[*in] != 1) {
				errno = EILSEQ; ret_value = (size_t)-1; break;
			}
			if (sizeof(UniChar) > *outbytesleft ){
				errno = E2BIG; ret_value = (size_t)-1; break;
			}
			ucs = (UniChar*)*outbuf;
			*ucs++ = *(*inbuf)++;
			(*inbytesleft)--;
			*outbuf += sizeof(UniChar);
			*outbytesleft -= sizeof(UniChar);
		}
		else { /* Toggle GL/GR to see if conversion can proceed */
			if (*in < 0x80) {
				if (current == cd->gl)
					break;
				current = cd->gl;
			}
			else {
				if (current == cd->gr || 
					cd->gr == &utfInvalidCharset)
					break;
				current = cd->gr;
			}
		}
	}
	/*
	 *	Swap high & low bytes of UCS characters when the endian is
	 *	required LITTLE.
	 *
	 *	CAUSION: The following statements are hardware dependent.
	 */


	if (cd->modifier.swap_endian.target)
		_ctUCS_b_swap (NULL, ucs_buf, ucs_size - *outbytesleft);
	return ret_value;
}


/*
 *   NAME:	_ct_UCS_init
 *
 *   FUNCTION:	Initialize for ct to UCS conversion
 *
 *   RETURNS:	This function returns pointer to a conversion descriptor to
 *		the variable pointed by 'cd' argument as described in XPG4.
 *		If failed, (_LC_ucs_ct_iconv_t*)-1 is returned.
 *
 *   NOTE:	This function will be replaced when ct_UCS_init is 
 *		implemented.
 */

static	_LC_ucs_ct_iconv_t	*_ct_UCS_init (
	_LC_core_iconv_t	*core,
	uchar_t			*t_name,
	uchar_t			*f_name) {

	_LC_ucs_ct_iconv_t	*cd;
        EscTblTbl               *ett;
        EscTbl*                 tmp;
        ctCharsetDesc 		p;

        if      (strncmp ("ct"   , f_name,2) == 0) ett = _iconv_ct_ett;
        else if (strncmp ("fold7", f_name,5) == 0) ett = _iconv_fold7_ett;
        else if (strncmp ("fold8", f_name,5) == 0) ett = _iconv_fold8_ett;
        else return (_LC_ucs_ct_iconv_t*)-1;

	cd = _ctUCS_init( core, t_name, f_name, ett);
	cd->isctl = ett->isctl;   /* Pick the 1st entry */
	cd->top->func = _ascii_to_ucs;

        /*
         * Initialize utfCharset
         */

        tmp = _iconv_FindWellKnownCharset( cd, "UTF-8", 5);
        utfCharset.next = (ctCharsetDesc)NULL;
        utfCharset.conv = (iconv_t)NULL;
        utfCharset.func = (ctUCS_Func)NULL;
        utfCharset.free = False;
        utfCharset.gl   = False;
        memcpy( &utfCharset.escape, tmp, sizeof(EscTbl));

	for ( p = cd->top; p && p->next ; p = p->next );
	p->next = &utfCharset;

	return cd;
}


#ifdef UCS2_SUPPORT
/*
 *   NAME:	ct_UCS_init
 *
 *   FUNCTION:	Initialize for ct to UCS-2 conversion
 *
 *   RETURNS:	This function returns pointer to a conversion descriptor to
 *		the variable pointed by 'cd' argument as described in XPG4.
 *		If failed, (_LC_ucs_ct_iconv_t*)-1 is returned.
 *
 *   NOTE:	Codeset name of the 'unicode' or 'ISO10646' is not passed as
 *		a string, but is indicated by empty string.
 */

static	_LC_ucs_ct_iconv_t	*ct_UCS_init (
	_LC_core_iconv_t	*core,
	uchar_t			*t_name,
	uchar_t			*f_name) {

	_LC_ucs_ct_iconv_t	*cd;

	return cd;
}

#endif



/*
 *   NAME:      instantiate
 *
 *   FUNCTION:  Instantiation method of this converter.
 *
 *   RETURNS:   Pointer to the descriptor.
 */

_LC_core_iconv_t	*instantiate(void) {

	static _LC_core_iconv_t core;

	core.hdr.__magic   = _LC_MAGIC;
	core.hdr.__version = _LC_VERSION | _LC_ICONV_MODIFIER;
	core.hdr.__type_id = _LC_ICONV;
	core.hdr.__size    = sizeof (_LC_core_iconv_t);
	core.init          = (_LC_core_iconv_t*(*)())_ct_UCS_init;
	core.close         = (int(*)())_ctUCS_close;
	core.exec          = (size_t(*)())_ct_UCS_exec;

	return &core;
}
