/* @(#)57	1.2  src/bos/usr/ccs/lib/libim/aixmapping.c, libim, bos411, 9428A410j 6/5/91 11:04:20 */
/*
 * COMPONENT_NAME :	LIBIM - AIX Input Method
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
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

/*
 * Copyright 1985, 1987, Massachusetts Institute of Technology
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "im.h"
#include "imP.h"

/*
 * IMAIXMapping()
 */

char	*IMAIXMapping(IMMap m, unsigned int keysym, unsigned int state,
	int *nbytes)
{
	if (!m) {
		*nbytes = 0;
		return NULL;
	}

	if (IsModifierKey(keysym)) {
		*nbytes = 0;
		return NULL;
	}

	if (state & Mod5Mask) {
		if (IsKeypadKey(keysym))
			state ^= ShiftMask;
		state &= ~Mod5Mask;
	}

	_IMMapKeysym(m->immap, &keysym, &state);
	m->output.len = 0;
	if (_IMAIXMapping(m->immap, keysym, state, &m->output,
		&m->dead_state, &m->accum_state, &m->accumulation) ==
		IMInputNotUsed)
		(void)_IMSimpleMapping(m->immap, keysym, state, &m->output);
	*nbytes = m->output.len;
	return m->output.data;
}

/*
 * IMSimpleMapping() does a simple keysym/state to string mapping.
 */

char	*IMSimpleMapping(IMMap m, unsigned int keysym, unsigned int state,
	int *nbytes)
{
	if (!m) {
		*nbytes = 0;
		return NULL;
	}

	if (state & Mod5Mask) {
		if (IsKeypadKey(keysym))
			state ^= ShiftMask;
		state &= ~Mod5Mask;
	}

	_IMMapKeysym(m->immap, &keysym, &state);
	m->output.len = 0;
	(void)_IMSimpleMapping(m->immap, keysym, state, &m->output);
	*nbytes = m->output.len;
	return m->output.data;
}

/*
 *	Rebind the specified string to the specified keysym/state.
 */

int	IMRebindCode(IMMap m, unsigned int keysym, unsigned int state,
	unsigned char *str, int nbytes)
{
	if (!m)
		return False;
	return _IMRebindCode(m->immap, keysym, state, str, nbytes);
}

/*
 *	IMUseKeymap
 */

IMMap	IMUseKeymap(char *path)
{
	IMMap	m;

	if (!(m = (IMMap)malloc(sizeof (IMMapRec))))
		return NULL;
	if (!(m->immap = _IMOpenKeymap(path))) {
		free(m);
		return NULL;
	}
	m->output.data = NULL;
	m->output.len = 0;
	m->output.siz = 0;
	m->dead_state = XK_VoidSymbol;
	m->accum_state = 0;
	m->accumulation = 0;
	return m;
}

IMMap	IMInitializeKeymap(IMLanguage lang)
{
	IMMap	m;

	if (!(m = (IMMap)malloc(sizeof (IMMapRec))))
		return NULL;
	if (!(m->immap = _IMInitializeKeymap(lang))) {
		free(m);
		return NULL;
	}
	m->output.data = NULL;
	m->output.len = 0;
	m->output.siz = 0;
	m->dead_state = XK_VoidSymbol;
	m->accum_state = 0;
	m->accumulation = 0;
	return m;
}

void	IMFreeKeymap(IMMap m)
{
	if (m) {
		_IMCloseKeymap(m->immap);
		if (m->output.data) 
			free((char *)m->output.data);
		free((char *)m);
	}
	return;
}
