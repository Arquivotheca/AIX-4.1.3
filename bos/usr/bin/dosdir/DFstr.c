static char sccsid[] = "@(#)56	1.3  src/bos/usr/bin/dosdir/DFstr.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:55:58";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: _DFcmp _DFcpym _DFcpyn _DFlocc _DFsname _DFlen _DFskpc _DFcmpp 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Local copies of CS* routines used within libdos.
 * names are _DF* in this library.
 * All taken from version 5.1 of the corresponding function
 *
 *      _DFcmp()
 *      _DFcpym()
 *      _DFcpyn()
 *      _DFlocc()
 *      _DFsname()
 *      _DFlen()
 */

#include <IN/standard.h>

_DFcmp(s1, s2)
	register char *s1, *s2; {

	if (s1 && s2) {
		while (*s1 == *s2++)
			if (!*s1++)
				return 0;
		return (*s1 > *--s2)? 1: -1;
	} else  return 0;
}

char *
_DFcpym(dst, src, cnt)
	register char *dst, *src;
	register cnt; {

	if (dst && src) {
		while (cnt > 0) {
			if ((*dst++ = *src++) == 0)
				return dst - 1;
			--cnt;
		}
		*dst = 0;
	}
	return dst;
}

char *
_DFcpyn(dst, src, cnt)
	register char *dst, *src;
	register cnt; {

	if (dst && src) {
		while (cnt > 0) {
			*dst++ = *src++;
			--cnt;
		}
		*dst = 0;
	}
	return dst;
}

char *
_DFlocc(str, chr)
	register char *str;
	register chr; {
	register c;

	if (str) {
		while (c = *str++)
			if (c == chr)
				break;
		--str;
	}
	return str;
}
/*
 * _DFname
 *
 *      return "simple" part of pathname
 */


char *
_DFsname (name)
register char *name;
{
	register char *cp = name + _DFlen(name);

        /* ignore trailing slashes */

	while( --cp > name && *cp == '/'
#ifdef vms
                && cp[-1] != ':' && cp[-1] != ']'
#endif
					 )
	    ;
        ++cp;

        /* find last slash */

	while( cp > name )
	    if( *--cp == '/'
#ifdef vms
                    || *cp == ':' || *cp == ']'
#endif
			     )
	    {   ++cp;
		if( *cp == NUL )
		    return ".";
		break;
	    }
        return cp;
}

_DFlen(s)
	register char *s; {
	register char *p;

	if (!s)
		return 0;
	for (p=s; *p++; )
		;
	return (p - s) - 1;
}

char *
_DFskpc(str, chr)
	register char *str;
	register chr; {

	if (str && chr) {
		while (*str++ == chr)
			;
		--str;
	}
	return str;
}

_DFcmpp(s1, s2)
	register char *s1, *s2; {

	if (s1 && s2)
		while(*s1)
			if (*s1++ != *s2++)
				return (*--s1 > *--s2)? 1: -1;
	return 0;
}
