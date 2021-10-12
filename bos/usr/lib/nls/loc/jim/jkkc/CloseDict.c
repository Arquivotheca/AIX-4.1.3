static char sccsid[] = "@(#)70	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/CloseDict.c, libKJI, bos411, 9428A410j 7/23/92 00:01:37";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       CloseSdict
 *                    CloseUdict
 *                    CloseFdict
 *
 * DESCRIPTIVE NAME:  CLOSE DICTIONARIES
 *                    
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       NONE (void)
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#include "_Kcmap.h"			/* dictionary information	*/
#include <sys/lockf.h>                  /* include for lockf()          */
#include "dict.h"			/* dictionary information	*/

/************************************************************************
 *	Colse System Dictionary
 ************************************************************************/
void	*CloseSdict(SDICTDATA dict)
{
	int	i;

	for (i = 0; i < SDICT_NUM; i++) {
		if (dict->sdictinfo[i].sdictname != NULL) {
			free(dict->sdictinfo[i].sdictname);
			dict->sdictinfo[i].sdictname = NULL;
		}
		dict->sdictinfo[i].dsyseg = NULL;
		if (dict->sdictinfo[i].sxesxe != NULL) {
			free(dict->sdictinfo[i].sxesxe);
			dict->sdictinfo[i].sxesxe = NULL;
		}
		if (dict->sdictinfo[i].dsyfd >= 0) {
			close(dict->sdictinfo[i].dsyfd);
			dict->sdictinfo[i].dsyfd = -1;
		}
	}
}


/************************************************************************
 *      Close User Dictionary
 ************************************************************************/
void	*CloseUdict(UDICTINFO dict)
{
	if (dict->uxeuxe) {
		free(dict->uxeuxe);
		dict->uxeuxe = NULL;
	}
	if (dict->mdemde) {
		free(dict->mdemde);
		dict->mdemde = NULL;
	}
	if (dict->udictname) {
		free(dict->udictname);
		dict->udictname = NULL;
	}

	if (dict->dusfd >= 0) {
		lseek(dict->dusfd, 0, 0);
		lockf(dict->dusfd, F_ULOCK, 0);
		close(dict->dusfd);
		dict->dusfd = -1;
	}
}


/************************************************************************
 *      Close FUZOKUGO Dictionary
 ************************************************************************/
void	*CloseFdict(FDICTINFO dict)
{
	if (dict->fdictname) {
		free(dict->fdictname);
		dict->fdictname = NULL;
	}
	if (dict->dfgdfg) {
		free(dict->dfgdfg);
		dict->dfgdfg = NULL;
	}
	if (dict->dfzfd >= 0) {
		close(dict->dfzfd);
		dict->dfzfd = -1;
	}
}
