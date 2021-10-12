static char sccsid[] = "@(#)82	1.2  src/bos/usr/lib/piosCN/lib/fontnames.c, ils-zh_CN, bos41J, 9512A_all 3/14/95 10:12:25";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: AddFontNamesName
 *		FreeFontNames
 *		MakeFontNamesRecord
 *
 *   ORIGINS: 16,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * $XConsortium: fontnames.c,v 1.1 91/05/10 16:51:51 keith Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 *
 */

#include	"fontmisc.h"
#include	"fontstruct.h"

void
FreeFontNames(pFN)
    FontNamesPtr pFN;
{
    int         i;

    if (!pFN)
	return;
    for (i = 0; i < pFN->nnames; i++) {
	xfree(pFN->names[i]);
    }
    xfree(pFN->names);
    xfree(pFN->length);
    xfree(pFN);
}

FontNamesPtr
MakeFontNamesRecord(size)
    unsigned    size;
{
    FontNamesPtr pFN;

    pFN = (FontNamesPtr) xalloc(sizeof(FontNamesRec));
    if (pFN) {
	pFN->nnames = 0;
	pFN->size = size;
	if (size)
	{
	    pFN->length = (int *) xalloc(size * sizeof(int));
	    pFN->names = (char **) xalloc(size * sizeof(char *));
	    if (!pFN->length || !pFN->names) {
	    	xfree(pFN->length);
	    	xfree(pFN->names);
	    	xfree(pFN);
	    	pFN = (FontNamesPtr) 0;
	    }
	}
	else
	{
	    pFN->length = 0;
	    pFN->names = 0;
	}
    }
    return pFN;
}

int
AddFontNamesName(names, name, length)
    FontNamesPtr names;
    char       *name;
    int         length;
{
    int         index = names->nnames;
    char       *nelt;

    nelt = (char *) xalloc(length + 1);
    if (!nelt)
	return AllocError;
    if (index >= names->size) {
	int         size = names->size << 1;
	int        *nlength;
	char      **nnames;

	if (size == 0)
	    size = 8;
	nlength = (int *) xrealloc(names->length, size * sizeof(int));
	nnames = (char **) xrealloc(names->names, size * sizeof(char *));
	if (nlength && nnames) {
	    names->size = size;
	    names->length = nlength;
	    names->names = nnames;
	} else {
	    xfree(nelt);
	    xfree(nlength);
	    xfree(nnames);
	    return AllocError;
	}
    }
    names->length[index] = length;
    names->names[index] = nelt;
    strncpy(nelt, name, length);
    nelt[length] = '\0';
    names->nnames++;
    return Successful;
}
