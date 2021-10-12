/* @(#)64	1.9  src/bos/usr/ccs/lib/libim/imcore.c, libim, bos411, 9428A410j 1/11/94 04:16:58 */
/*
 * COMPONENT_NAME :	LIBIM
 *
 * FUNCTIONS :		AIX Input Method Library
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/***********************************************************
Copyright International Business Machines Corporation 1989

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "im.h"
#include "imP.h"
 
extern int	(*load(char *, unsigned int, char *))();

/*-----------------------------------------------------------------------*
*	GROVAL VARIABLES
*-----------------------------------------------------------------------*/
int	imerrno = IMNoError;

/*-----------------------------------------------------------------------*
*
*	IMQueryLanguage()
*		check to see if the specified module exists.
*
*-----------------------------------------------------------------------*/

unsigned int	IMQueryLanguage(IMLanguage language)
{
	char	*pathname;

	if (!(pathname = _IMAreYouThere(language, IMPOSTFIX)))
		return FALSE;
	free(pathname);
	return TRUE;
}

/*-----------------------------------------------------------------------*
*
*	IMInitialize()
*		load the specified module and initialize it.
*
*-----------------------------------------------------------------------*/

IMFep		IMInitialize(IMLanguage language)
{
	IMFep	imfep;
	IMFep	(*entry)(IMLanguage);
	char	*pathname;

	if (!(pathname = _IMAreYouThere(language, IMPOSTFIX)))
		return NULL;

	/*
	 *	try to load it.
	 */
	if (!(entry = (IMFep (*)(IMLanguage))load(pathname, 0, 0))) {
		imerrno = IMCouldNotLoad;
		free(pathname);
		return NULL;
	}
	free(pathname);

	if (!(imfep = (*entry)(language))) {		/* initialize */
		imerrno = IMInitializeError;
		(void)unload((int)entry);
		return NULL;
	}

	imfep->language = (char *)malloc((unsigned)strlen(language) + 1);
	(void)strcpy((char *)imfep->language, (char *)language);

	imfep->version = IMVersionNumber;		/* version stamp */

	return imfep;
}

/*-----------------------------------------------------------------------*
*
*	IMClose()
*		unload the module
*
*-----------------------------------------------------------------------*/

void	IMClose(IMFep imfep)
{
	int (*init)();

	if (imfep) {
		init = (int (*)())(imfep->iminitialize);
		free(imfep->language);
		(*imfep->imclose)(imfep);
		(void)unload(init);
	}
	return;
}

/*-----------------------------------------------------------------------*
*
*	IMCreate()
*
*-----------------------------------------------------------------------*/

IMObject	IMCreate(IMFep imfep, caddr_t imcallback, caddr_t udata)
{
	IMObject	im;

	if (!imfep) {
		imerrno = IMInvalidParameter;
		return NULL;
	}
	if (!(im = (*imfep->imcreate)(imfep, imcallback, udata)))
		imerrno = imfep->imerrno;
	return im;
}

/*-----------------------------------------------------------------------*
*
*	IMDestroy()
*
*-----------------------------------------------------------------------*/

void		IMDestroy(IMObject im)
{
	if (im)
		(void)(*im->imfep->imdestroy)(im);
}

/*-----------------------------------------------------------------------*
*
*	IMFilter()
*
*-----------------------------------------------------------------------*/

int	IMFilter(IMObject im, caddr_t key, caddr_t state, 
		caddr_t str, caddr_t len)
{
	if (!im) {
		imerrno = IMInvalidParameter;
		return IMError;
	}
	return (*im->imfep->imfilter)(im, key, state, str, len);
}

/*-----------------------------------------------------------------------*
*
*	IMLookupString()
*
*-----------------------------------------------------------------------*/

int	IMLookupString(IMObject im, caddr_t key, caddr_t state, 
		caddr_t str, caddr_t len)
{
	if (!im) {
		imerrno = IMInvalidParameter;
		return IMError;
	}
	return (*im->imfep->imlookup)(im, key, state, str, len);
}

/*-----------------------------------------------------------------------*
*
*	IMProcess()
*
*-----------------------------------------------------------------------*/

int	IMProcess(IMObject im, caddr_t key, caddr_t state, 
		caddr_t str, caddr_t len)
{
	int	r;			/* return code */

	if (!im) {
		imerrno = IMInvalidParameter;
		return IMError;
	}
	if ((r = (*im->imfep->improcess)(im, key, state, str, len)) == IMError)
		imerrno = im->imfep->imerrno;
	return r;
}

/*-----------------------------------------------------------------------*
*
*	IMProcessAuxiliary()
*
*-----------------------------------------------------------------------*/

int	IMProcessAuxiliary(IMObject im, caddr_t auxid, caddr_t button,
		caddr_t panel_row, caddr_t panel_col, 
		caddr_t item_row, caddr_t item_col,
		caddr_t *str, uint *len)
{
	int	r;			/* return code */

	if (!im) {
		imerrno = IMInvalidParameter;
		return IMError;
	}
	if ((r = (*im->imfep->improcessaux)(im, auxid, button, panel_row,
			panel_col, item_row, item_col, str, len)) == IMError)
		imerrno = im->imfep->imerrno;
	return r;
}

/*-----------------------------------------------------------------------*
*
*	IMIoctl()
*
*-----------------------------------------------------------------------*/

int	IMIoctl(IMObject im, caddr_t op, caddr_t arg)
{
	int	r;			/* return code */

	if (!im) {
		imerrno = IMInvalidParameter;
		return IMError;
	}
	if ((r = (*im->imfep->imioctl)(im, op, arg)) == IMError)
		imerrno = im->imfep->imerrno;
	return r;
}
