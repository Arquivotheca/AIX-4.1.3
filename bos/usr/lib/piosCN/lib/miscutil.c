static char sccsid[] = "@(#)92	1.1  src/bos/usr/lib/piosCN/lib/miscutil.c, ils-zh_CN, bos41J, 9507B 1/26/95 09:56:26";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CopyISOLatin1Lowered
 *		Xalloc
 *		Xfree
 *		Xrealloc
 *		register_fpe_functions
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * $XConsortium: miscutil.c,v 1.2 91/05/13 16:40:27 gildea Exp $
 * 
 * Copyright 1991 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission.  M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 */

#define XK_LATIN1
#include    <X11/keysymdef.h>
/* #include    <X11/Xmu/CharSet.h> */

/* make sure everything initializes themselves at least once */
#if !defined( X11R5BOS )
long serverGeneration = 1;

unsigned long *
Xalloc (m)
{
    return (unsigned long *) malloc (m);
}

unsigned long *
Xrealloc (n,m)
    unsigned long   *n;
{
    if (!n)
	return (unsigned long *) malloc (m);
    else
	return (unsigned long *) realloc ((char *) n, m);
}

Xfree (n)
    unsigned long   *n;
{
    if (n)
	free ((char *) n);
}
#endif /* X11R5BOS */
CopyISOLatin1Lowered (dst, src, len)
    char    *dst, *src;
    int	    len;
{
    register unsigned char *dest, *source;

    for (dest = (unsigned char *)dst, source = (unsigned char *)src;
	 *source && len > 0;
	 source++, dest++, len--)
    {
	if ((*source >= XK_A) && (*source <= XK_Z))
	    *dest = *source + (XK_a - XK_A);
	else if ((*source >= XK_Agrave) && (*source <= XK_Odiaeresis))
	    *dest = *source + (XK_agrave - XK_Agrave);
	else if ((*source >= XK_Ooblique) && (*source <= XK_Thorn))
	    *dest = *source + (XK_oslash - XK_Ooblique);
	else
	    *dest = *source;
    }
    *dest = '\0';
}
#if !defined( X11R5BOS )
register_fpe_functions ()
{
}
#endif
