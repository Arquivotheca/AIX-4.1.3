static char sccsid[] = "@(#)10  1.2  src/bos/usr/lib/nls/loc/uconv/UCS_ct.c, ils-zh_CN, bos41J, 9514A_all 3/28/95 15:12:32";
/*
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		_ascii_from_ucs
 *			MatchCharset
 *			MoveOneUTFChar
 *			WriteEscSeq
 *			UpdateExtSegLen
 *			_iconv_from_ucs
 *			_uconv_from_ucs
 *			_UCS_ct_init
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
#include "UCSTBL.h"
#include "ct.h"


/*
 *   NAME:	_ascii_from_ucs
 *
 *   FUNCTION:	Conversion from UCS-2 to ASCII.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static size_t	_ascii_from_ucs (
	uchar_t **in_buf,  size_t *in_left, 
	uchar_t **out_buf, size_t *out_left) {

	UniChar		*ucs_buf;
	UniChar		*in_ptr,  *in_buf_end;
	uchar_t		*out_ptr, *out_buf_end;
	size_t		errfl = 0;

	/*
	 *	Change byte order of input UCS-2 characters if little endian.
	 */

	ucs_buf = (UniChar*) *in_buf;
	in_buf_end  = (in_ptr  = ucs_buf)  +  *in_left/sizeof(UniChar);
	out_buf_end = (out_ptr = *out_buf) +  *out_left;

	while (in_ptr < in_buf_end) {

		if ((*in_ptr > 0x007f) ||
		    (*in_ptr < 0x001f)) {
			errno = EILSEQ;
			errfl = (size_t)-1;
			break;
		}
		if (out_ptr > out_buf_end) {
			errno = E2BIG;
			errfl = (size_t)-1;
			break;
		}
		*(out_ptr ++) = (uchar_t)*in_ptr;
		in_ptr++;

	}
	*in_left  = *in_buf + *in_left - (uchar_t*)in_ptr;
	*in_buf  = (uchar_t*)in_ptr;
	*out_buf  = out_ptr;
	*out_left = out_buf_end - out_ptr;

	if ( (*in_left == 1) && (errfl == 0)) {
		errno = EINVAL;		/* The last UCS-2 char is truncated */
		errfl = (size_t)-1;
	}
	return errfl;
}

/*
 *   NAME:	MatchCharset
 *
 *   FUNCTION:	Using a CharsetDB, it matches the 1st char in inbuf
 *		to a charset inside the CharsetDB.
 *
 *		NOTE: It is assumed that *ALL* charset iconv conversions
 * 		      are stateless.  This means that the same inbuf
 *		      may be converted again.
 *
 *   RETURNS:   ctCharsetDesc top	if match found
 *		ctCharsetDesc NULL 	if no match
 *		ctCharsetDesc -1	if error
 */
ctCharsetDesc MatchCharset(
	ctCharsetDesc	top,
	const uchar_t*	inbuf, 
	size_t		inbytesleft)
{
	uchar_t		tmp[1];
	uchar_t*	out = tmp;
	size_t		out_len = 1;
	size_t		nsubs;

	if (top->func )
		nsubs = (*(top->func))(
			&inbuf, &inbytesleft, &out, &out_len);
	else    nsubs = iconv (top->conv,
			&inbuf, &inbytesleft, &out, &out_len);

	
	if (nsubs == (size_t)-1) {
		if (errno == EILSEQ) 
			/* Charset does not match */
			return (ctCharsetDesc)NULL;
		else if (errno == EINVAL)
			/* Charset recognized, but tmp buf too small */
			return top;
		else
			/* Some other error */
			return (ctCharsetDesc)-1;
	}
	return top;
}
/*
 *   NAME:	NextCharset
 *
 *   FUNCTION:	Searches for the next Charset that has:
 *		a. valid iconv function
 *		b. EscSeq fits into outbuf...
 *
 *   RETURNS:	cd->current is modified 
 * 		0       - No Match found, cd->current = NULL
 *		1       - Match found, cd->current is reset
 *		-1      - Error, cd->current = NULL
 */
int NextCharset(
	_LC_ucs_ct_iconv_t* cd,
	const uchar_t**	inbuf,
	size_t*		inbytesleft,
	uchar_t**	outbuf,
	size_t*		outbytesleft)
{
	ctCharsetDesc prev;
	ctCharsetDesc next;

	if ( *inbytesleft == 0 ) {
		cd->current = (ctCharsetDesc) NULL;
		return 0;
	}

	if ( cd->current == (ctCharsetDesc)NULL ) {
		prev = NULL;
		next = cd->top;
	}
	else {
		prev = cd->current;
		next = cd->current->next;
	}

	/*
	 * Assume we'll not find another 
	 */
	cd->current = NULL;

	while ( next )  {

		if (next->conv == (iconv_t)NULL && 
		      next->func == (ctUCS_Func)NULL ) {

					   /* 
                                            * @map modifier is needed to assure
                                            * no extra mapping is done by UCSTBL
                                            * that is not defined by the .ucmap
                                            */
		   uchar_t map_modifier[] = "@map=none,sub=no"; 
		   uchar_t name[256];

		   memcpy( name, next->escape.name, strlen(next->escape.name)+1);
		   strcat( name, map_modifier);

		   if ((next->conv = iconv_open ( name, DEF_UCS_NAME)) 
			    == (iconv_t)-1) {
			/*
			 * Remove this Charset entry from DB...
			 */
			ctCharsetDesc tmp;
			tmp = next->next;
			if ( !next->free )
				free(next);

			if ( prev )
				prev->next = tmp;
			else
				cd->top = tmp;
			next = tmp;
			continue;
		   }
		}
		if ( EscSeqLen(next) > *outbytesleft ) {
			if ( MatchCharset( next,
					   *inbuf,
					   *inbytesleft) == next ) {
				errno = E2BIG;
				return (size_t)-1;
			}
		}
		else {
			cd->current = next;
			return 1;
			break;
		}
		prev = next;
		next = next->next;
	}
	return 0;
}

/*
 *   NAME:	UpdateExtSegLen
 *
 *   FUNCTION:	Update len in Esc. Seq header
 *
 *   RETURNS:	none
 */
void UpdateExtSegLen(
	ctCharsetDesc	current,
	uchar_t*	start, 
	uchar_t*	end)
{
	size_t len ;

	start   += current->escape.len ;
        len = end - start - 2;
	*start++ = len / 128 | 0x80;
	*start   = len % 128 | 0x80;
}


/*
 *   NAME:	WriteEscSeq
 *
 *   FUNCTION:	write length within Extended Segment header
 *
 *   RETURNS:	none
 */
void WriteEscSeq(
	ctCharsetDesc	current,
	uchar_t*	start, 
	uchar_t*	end)
{
	size_t len;

	memcpy (start, current->escape.str, current->escape.len);
	len = current->escape.seglen + 2;
	if (current->escape.seg == NULL) {
		return;;
	}

	/*
	 *	Make an extended segment.
	 */
	UpdateExtSegLen( current, start, end);
	start += current->escape.len + 2;
	memcpy(start, current->escape.seg, current->escape.seglen);
}

/*
 *   NAME:	MoveOneUTFChar
 *
 *   FUNCTION:	Write in Esc. Seq header
 *
 *   RETURNS:	none
 */
size_t MoveOneUTFChar(
	const uchar_t**	inbuf, 
	size_t*		inbytesleft,
	uchar_t**	outbuf, 
	size_t*		outbytesleft)
{
	UniChar*	ucs_buf = (UniChar*)*inbuf;
	size_t		ucs_size = 1;
	uchar_t		tmpbuf[3] ;
	uchar_t*	tmp = tmpbuf;
	size_t		n = 3;


	if ( __iconv_ucs2utf( &ucs_buf, &ucs_size, &tmp, &n ) == -1 ) {
		errno = EINVAL;
		return (size_t)-1;
	}

	n = 3 -n;
	if ( n > *outbytesleft ) {
		errno = EINVAL;
		return (size_t)-1;
	}
	memcpy( *outbuf, tmpbuf, n);
	*inbuf += sizeof(UniChar);
	*outbuf += n;
	*inbytesleft -= sizeof(UniChar);
	*outbytesleft -= n;
	return n;
}


/*
 *   NAME:	_iconv_from_ucs
 *
 *   FUNCTION:	Conversion from UCS-2 (character based interface).
 *
 *   RETURNS:	>= 0    - Number of non-identical conversions performed.
 *		-1      - Error.
 **********************************************************************
	Pseudo-code of iconv_from_ucs

	until end of inbuf

	   for all Charsets 		( Handle as GR always !!! )
		if ( iconv )
			iconv w/ current cd
		else 
			ascii_from_ucs
		if ( something converted ) {
			Write EscSeq area, if needed
			performance optimize ... move cur Charset to be 2nd 
			UCSExtendedSeq = False;
			Set cd->current to re-start at cd->top.
		}
		Find next Charset that has iconv & matches charset of
			1st character in inbuf.
			*** The match charset became important when you
			*** consider cases about Esc Seq not fitting...
		if Charset found and Escape Seq needed
			skip forward, write later
		}
	   }
	   if ( cd->current ) break;

	   Nothing converted, write out UTF extended segment
	   If not first UCS char in same segment
		append UTF char to end of outbuf & update Ext. Seg. length

	   Change current to top Charset
	   if Escape Seq needed
		skip forward, write later

 ***********************************************************************
 *
 */

static	size_t			_UCS_ct_exec(
	_LC_ucs_ct_iconv_t*	cd,
	const uchar_t**		inbuf,  
	size_t*			inbytesleft,
	uchar_t**		outbuf, 
	size_t*			outbytesleft) 
{

	uchar_t*		p;
	size_t			n;
	size_t			nsubs;
	uchar_t*		utfExtSeg;
	size_t			utfExtSegLen;
	size_t 			ret_value = 0;

        const uchar_t         	*ucs_buf, *ucs_start;


	if ((cd == NULL) || (cd == (_LC_ucs_ct_iconv_t*)-1)) {
		errno = EBADF;
		return (size_t)-1;
	}
	if (inbuf == NULL) {
		if (_ctUCS_reset_state (cd, outbuf, outbytesleft))
			return (size_t)0;
		else	return (size_t)-1;
	}

        if (cd->modifier.swap_endian.source) {
                if ((ucs_buf = (uchar_t*)malloc (*inbytesleft)) == NULL) {
                        errno = ENOMEM;
                        return (size_t)-1;
                }
                _ctUCS_b_swap ((uchar_t*)ucs_buf, (uchar_t*)*inbuf, 
					*inbytesleft);
		ucs_start = ucs_buf;
        }
        else {
		ucs_start = 
		ucs_buf = *inbuf;
	}

	utfExtSegLen = 0;
	p = *outbuf;
	n = *outbytesleft;

	if (cd->current == (ctCharsetDesc) NULL) 
		cd->current = cd->top;

	while ( *inbytesleft ) {
		ctCharsetDesc current = cd->current;

		for( current=cd->current; current ; ) {
			uchar_t* save_p = p;
			if (current->func ) 
				nsubs = (current->func)(
					&ucs_buf, inbytesleft, &p, &n);
			else	nsubs = iconv (current->conv,
					&ucs_buf, inbytesleft, &p, &n);

			if (p != save_p) {
				/*
				 *	Something was converted. Put out escape
				 *	sequence of current code set at the top
				 *	of converted character string.
				 */
				if ( save_p != *outbuf )
					WriteEscSeq( current, *outbuf, p);
				*outbuf = p;
				*outbytesleft = n;
				
				/*
				 * Restart with top Charset 
				 */
				cd->current = NULL;
				utfExtSegLen = 0;
			}

			/*
			 * If error ... except EILSEQ ... check out
			 */
			if ( nsubs == (size_t) -1 ) {
				if ( errno != EILSEQ ) {
					ret_value = (size_t)-1;
					break;
				}
			}
			/* 
			 * Last charset Conversion complete, lookf for next charset or C0
			 */

			while ( *inbytesleft >= sizeof(UniChar) 
				&& *((UniChar*)ucs_buf) < (UniChar)0x0020) {
				if ( *((UniChar*)ucs_buf) != (UniChar)'\t'
				     && *((UniChar*)ucs_buf) != (UniChar)'\n') {
					errno = EILSEQ; 
					ret_value = (size_t)-1; 
					break;
				}
				if (*outbytesleft == 0){
					errno = E2BIG;
					ret_value = (size_t)-1; 
					break;
				}
				*(*outbuf)++ = (uchar_t)*((UniChar*)ucs_buf);
   				ucs_buf += sizeof(UniChar);
				(*inbytesleft) -= sizeof(UniChar);
				(*outbytesleft)--;
				/* Restart with top Charset */
				cd->current = NULL;
				utfExtSegLen = 0;
			}
			if ( ret_value != 0 ) break;

			/* 
			 * Ok, new Charset needed... lets go fishing...
			 * Criteria for new Charset 
		   	 *  a. has conversion descriptor
			 *  b. EscSeq header fits in outbuf...
			 */
			if ( NextCharset( cd, 
				     &ucs_buf, inbytesleft,
				     outbuf, outbytesleft) == -1 ) {
				ret_value = (size_t)-1;
				break;
			}
			if ( current = cd->current ) {
				p = *outbuf + EscSeqLen(cd->current);
				n = *outbytesleft - EscSeqLen(cd->current);
			}
		}


		if ( !*inbytesleft ) {
			break;
		}

		/*
		 *      All Charset's exhausted....
 		 * 	Put out UTF Extended Segment ...
		 */
		if ( utfExtSegLen == 0 ) {
			if ( EscSeqLen(&utfCharset) > *outbytesleft ) {
				errno = E2BIG ;
				ret_value = (size_t)-1;
				break;
			}
			utfExtSeg = *outbuf;
			WriteEscSeq( &utfCharset, 
				 	 *outbuf, 
					 *outbuf + EscSeqLen(&utfCharset));
			*outbuf += EscSeqLen(&utfCharset);
			*outbytesleft -= EscSeqLen(&utfCharset);
		}
		/* else outbuf/outbytesleft already set for append */
		
		n = MoveOneUTFChar( &ucs_buf, inbytesleft, 
				    outbuf, outbytesleft);
		if ( n == (size_t)-1 ) { 
			errno = E2BIG;
			ret_value = (size_t)-1;
			break;
		}

		utfExtSegLen += n;
		UpdateExtSegLen( &utfCharset, utfExtSeg, *outbuf );

		/* 
		 * Reset to top Charset for next go round 
		 */
		cd->current = NULL;
		if ( NextCharset( cd, 
			     &ucs_buf, inbytesleft,
			     outbuf, outbytesleft) == -1 ) {
			ret_value = (size_t)-1;
			break;
		}
		if ( cd->current ) {
			p = *outbuf + EscSeqLen(cd->current);
			n = *outbytesleft - EscSeqLen(cd->current);
		}
	}
	*inbuf += ucs_buf - ucs_start;
	return ret_value;
}

/*
 *   NAME:      _UCS_ct_init
 *
 *   FUNCTION:  Initialize for UCS8 to ct conversion
 *
 *   RETURNS:   This function returns pointer to a conversion descriptor to
 *              the variable pointed by 'cd' argument as described in XPG4.
 *              If failed, (_LC_ucs_ct_iconv_t*)-1 is returned.
 *
 *   NOTE:      This function will be replaced when UCS_ct_init is
 *              implemented.
 */

static  _LC_ucs_ct_iconv_t      *_UCS_ct_init (
        _LC_core_iconv_t        *core,
        uchar_t                 *t_name,
        uchar_t                 *f_name) {

        _LC_ucs_ct_iconv_t*     cd;
        EscTblTbl*              ett;
	EscTbl*			tmp;

        if      (strncmp (t_name, "ct"   ,2) == 0) ett = _iconv_ct_ett;
        else if (strncmp (t_name, "fold7",5) == 0) ett = _iconv_fold7_ett;
        else if (strncmp (t_name, "fold8",5) == 0) ett = _iconv_fold8_ett;
        else return (_LC_ucs_ct_iconv_t*)-1;

	/*
	 * Initialize iconv conversion descriptor
	 */

        cd = _ctUCS_init( core, t_name, f_name, ett);
	cd->top->func = _ascii_from_ucs;

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

        return cd;
}


#ifdef UCS2_SUPPORT
/*
 *   NAME:      UCS_ct_init
 *
 *   FUNCTION:  Initialize for UCS-2 to ct conversion
 *
 *   RETURNS:   This function returns pointer to a conversion descriptor to
 *              the variable pointed by 'cd' argument as described in XPG4.
 *              If failed, (_LC_ucs_ct_iconv_t*)-1 is returned.
 *
 *   NOTE:      Codeset name of the 'unicode' or 'ISO10646' is not passed as
 *              a string, but is indicated by empty string.
 */

static  _LC_ucs_ct_iconv_t      *UCS_ct_init (
        _LC_core_iconv_t        *core,
        uchar_t                 *t_name,
        uchar_t                 *f_name) {

        _LC_ucs_ct_iconv_t      *cd;

        return cd;
}
#endif /* UCS2_SUPPORT */



/*
 *   NAME:      instantiate
 *
 *   FUNCTION:  Instantiation method of this converter.
 *
 *   RETURNS:   Pointer to the descriptor.
 */

_LC_core_iconv_t        *instantiate(void) {

        static _LC_core_iconv_t core;

        core.hdr.__magic   = _LC_MAGIC;
        core.hdr.__version = _LC_VERSION | _LC_ICONV_MODIFIER;
        core.hdr.__type_id = _LC_ICONV;
        core.hdr.__size    = sizeof (_LC_core_iconv_t);
        core.init          = (_LC_core_iconv_t*(*)())_UCS_ct_init;
        core.close         = (int(*)())_ctUCS_close;
        core.exec          = (size_t(*)())_UCS_ct_exec;

        return &core;
}

