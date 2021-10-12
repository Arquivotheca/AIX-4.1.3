static char sccsid[] = "@(#)86  1.5     src/bldenv/pkgtools/common/vrmf.c, pkgtools, bos41B, 412_41B_sync 11/18/94 16:43:06";
 /*   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: 
 *		getVRMF
 *		splitVRMF
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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "ptf.h"


/*-----------------------------------------------------------------------
| Get the version, release, mod and fix level from the fileset lpp_info file.	|
| This information is found in the second field of the second line	|
| of the lpp_info file.  For example, the following two lines are	|
| from the bos.dev.base.diag lpp_info file:				|
|	4 R I bos.dev.base						|
|	bos.dev.base.diag 04.01.0000.0000				|
| where vrmf will contain 04.01.0000.0000				|
-----------------------------------------------------------------------*/
void
getVRMF (char *fileset, char *top, char *vrmf, char *bldcycle)
{
	char	lppInfoFile[MAXPATHLEN+1];
	char	buf[BUFSIZE];
	FILE	*lppInfoFp;
	char	*ptr = NULL;
	char    *ptr1 = NULL;

	getDirName (fileset, top, lppInfoFile);
	strcat (lppInfoFile, "/lpp_info.");
	strcat (lppInfoFile, bldcycle);
	if ( !(lppInfoFp = fopen(lppInfoFile, "r")) )
	{
		ptr = strrchr(lppInfoFile, '.');
		*ptr = NULL;
		if ( !(lppInfoFp = fopen(lppInfoFile, "r")) )
			fatal (CouldNotOpenForVRMF, fileset, bldcycle);
        }

	/*-----------------------------------------------
	| Skip over first line of lpp_info file.	|
	-----------------------------------------------*/
	stripComments (lppInfoFp, buf);
	if ( stripComments (lppInfoFp, buf) == EOF )
		fatal (CouldNotGetVRMF, lppInfoFile);

	/*-----------------------------------------------
	| Copy second field into vrmf.			|
	-----------------------------------------------*/
	if ( !(ptr1 = strchr (buf, ' ')) )
		fatal (CouldNotGetVRMF, lppInfoFile);
	strcpy (vrmf, ptr1+1);

	fclose(lppInfoFile);
}

/*------------------------------------------------------------
| Split the vrm string into separate fields.		     |
| The vrm string format is <version>.<release>.<mod>.<fix>   |
-------------------------------------------------------------*/

void 
splitVRMF(char * fileset, char * vrmf, vrmftype * vrmfentry) 
{
	char * ptr;/* local pointer to go through vrmf string*/
	char * buf; /* to duplicate the vrmf as we do not want to modify the  original*/

	buf = strdup(vrmf);
	/*--------------------------------------------------------------
	| find the first "." and copy the preceding string in version  |
	---------------------------------------------------------------*/
	if ( !(ptr = strchr (buf, '.')) )
		fatal (InvalidVRMF,fileset, vrmf);
	*ptr = NULL;
	strcpy(vrmfentry->version, buf);

	/*--------------------------------------------------------------
	| find the second "." and copy the preceding string in release  |
	---------------------------------------------------------------*/
	buf = ptr + 1;
	if ( !(ptr = strchr (buf, '.')) )
		fatal (InvalidVRMF, fileset, vrmf);
	*ptr = NULL;
	strcpy(vrmfentry->release, buf);

	/*--------------------------------------------------------------
	| find the third "." and copy the preceding string in mod      |
	---------------------------------------------------------------*/
	buf = ptr + 1;
	if ( !(ptr = strchr (buf, '.')) )
		fatal (InvalidVRMF, fileset, vrmf);
	*ptr = NULL;
	strcpy(vrmfentry->mod, buf);

	/*----------------------------------------------
	| the remaining string is fix level            |
	-----------------------------------------------*/
	strcpy(vrmfentry->fix, ptr+1);

	/*----------------------------------------------
	| free the allocated space                     |
	-----------------------------------------------*/
	free(buf);
}

