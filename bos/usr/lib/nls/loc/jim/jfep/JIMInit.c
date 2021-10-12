static char sccsid[] = "@(#)05	1.5.1.2  src/bos/usr/lib/nls/loc/jim/jfep/JIMInit.c, libKJI, bos411, 9428A410j 9/29/93 21:42:54";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMInit
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <imlang.h>
#include "imjimP.h"		/* Japanese Input Method header file */

/*
 *	determine the code set based on the give "IMLanguage" string.
 */
static int	determine_codeset(IMLanguage lang)
{
	if (strstr(lang, JIM_CS_IBMeucJP))
		return JIM_EUC;
	if (strstr(lang, JIM_CS_IBM932))
		if (strstr(lang, JIM_CS_MODIFIER))
			return JIM_SJISDBCS;
		else
			return JIM_SJISMIX;
	if (strstr(lang, JIM_CS_IBM301))
		return JIM_SJISDBCS;
	if (strstr(lang, JIM_CS_UJIS))
		return JIM_EUC;
	if (strstr(lang, JIM_CS_SJIS))
		if (strstr(lang, JIM_CS_MODIFIER))
			return JIM_SJISDBCS;
		else
			return JIM_SJISMIX;
	if (strstr(lang, JIM_CS_eucJP_BY_LANG))
		return JIM_EUC;
	if (strstr(lang, JIM_CS_932_BY_LANG))
		if (strstr(lang, JIM_CS_MODIFIER))
			return JIM_SJISDBCS;
		else
			return JIM_SJISMIX;
	if (strstr(lang, JIM_CS_OLD_LANG))
		if (strstr(lang, JIM_CS_MODIFIER))
			return JIM_SJISDBCS;
		else
			return JIM_SJISMIX;
	return JIM_EUC;
}

/*
 *	JIMInitialize()
 */
IMFep	JIMInitialize(IMLanguage language)
{
	extern void	JIMClose();
	extern IMObject	JIMCreate();
	extern void	JIMDestroy();
	extern int	JIMProcess();
	extern int	JIMFilter();
	extern int	JIMLookup();
	extern int	JIMProcessAuxiliary();
	extern int	JIMIoctl();
	int	i;

	JIMFEP	fep;

	fep = (JIMFEP)malloc(sizeof(JIMfep));

	SetCurrentSDICTDATA(&(fep->sdictdata));
	SetCurrentUDICTINFO(&(fep->udictinfo));
	SetCurrentFDICTINFO(&(fep->fdictinfo));

	/*
	 *	initialize allocated above
	 */
	fep->common.imerrno = IMNoError;
	fep->common.iminitialize = JIMInitialize;
	fep->common.imclose = JIMClose;
	fep->common.imcreate = JIMCreate;
	fep->common.imdestroy = JIMDestroy;
	fep->common.improcess = JIMProcess;
	fep->common.improcessaux = JIMProcessAuxiliary;
	fep->common.imioctl = JIMIoctl;
	fep->common.imfilter = JIMFilter;
	fep->common.imlookup = JIMLookup;
	fep->jimver = JIM_VERSION;
	fep->codeset = determine_codeset(language);
	if (fep->codeset == JIM_EUC) {
		fep->cd = iconv_open(JIM_CS_IBMeucJP, JIM_CS_IBM932);
		if (fep->cd == -1 ) {
			free(fep);
			fep->common.imerrno = IMInitializeError;
			return NULL;
		}
	}
	else
		fep->cd = NULL;

	/*
	 *	set -1 to dictionary file descriptors
	 *			to indicate these files are not opened
	 */
	for ( i = 0; i < SDICT_NUM; i++ ) {
	    fep->sdictdata.sdictinfo[i].sdictname = NULL;
	    fep->sdictdata.sdictinfo[i].dsyfd = -1;
	    fep->sdictdata.sdictinfo[i].dsyseg = NULL;
	    fep->sdictdata.sdictinfo[i].sxesxe = NULL;
	}

	fep->udictinfo.udictname = NULL;
	fep->udictinfo.dusfd = -1;
	fep->udictinfo.indlen = 0;
	fep->udictinfo.mdemde = NULL;
	fep->udictinfo.uxeuxe = NULL;

	fep->fdictinfo.fdictname = NULL;
	fep->fdictinfo.dfzfd = -1;
	fep->fdictinfo.dfgdfg = NULL;

	/*
	 *	open iconv converter for Host (IBM Kanji) -> SJIS (IBM-932)
	 */
	OpenHostCodeConverter();

	/*
	 *	have keymap routine initialized
	 *	note: passes the given language as is.
	 */
	fep->immap = _IMInitializeKeymap(language);

	if (fep->immap == NULL) { /* initialization failed */
		free(fep);
		fep->common.imerrno = IMKeymapInitializeError;
		return NULL;
	}
	else
		return (IMFep)fep;
}
