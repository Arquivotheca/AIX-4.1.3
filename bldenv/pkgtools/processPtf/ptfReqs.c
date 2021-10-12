static char sccsid[] = "@(#)47  1.5     src/bldenv/pkgtools/processPtf/ptfReqs.c, pkgtools, bos41B, 412_41B_sync 11/18/94 16:35:09";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: addTreeReqs
 *		addRequisites
 *		basePrereq
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
#include <sys/param.h>
#include "ptf.h"
#include "tree.h"

extern char     *ptfoptFile;
struct btree_node     *treeroot = NULL;

/*-----------------------------------------------------------------------
| Add requisite information to the <fileset>.prereq file.  If this is	|
| not a special type ptf we prereq the last cum ptf.  Also add		|
| requisites from ifreqsList, coreqsList and prereqsList.		|
-----------------------------------------------------------------------*/
void
addRequisites (char *fileset, 
	       char *specialPtfType,
	       char *lastCumPtf, 
	       int  overrideVRMF, 
	       char *top, 
	       char *internalTable, 
	       char *buildCycle)
{
	FILE 	*prereqFp;
	char	fileName[MAXPATHLEN+1];
	char	filesetDir[MAXPATHLEN+1];
	char	buf[BUFSIZE];
	char    *ptr = NULL;
	char    foption[FILESETLEN];
	char    cumVRMF[VRMFSIZE];
	char    ptfvrmfFile[VRMFSIZE];
	vrmftype vrmfentry;

	/*------------------------------------------------------------
	| a new fileset.prereq file is created each time.             |
	------------------------------------------------------------*/
	sprintf (fileName, "%s.prereq", fileset);
	prereqFp = openFile (fileName, "w");
	/*------------------------------------------------------------
	| If this is not a special type ptf                          |
	| and there is a last cumptf then write a prereq for the     |
	| last cum only if the fix level of the last cum was not     |
	| zero. If the fix level of the last cum ptf is zero then    |
	| we do not write the prereq as it is implicit.              |
	------------------------------------------------------------*/
	if ( !strlen (specialPtfType) && strlen (lastCumPtf) )
	{
            foption[0] = NULL;
            ptfvrmfFile[0] = NULL;
		/*---------------------------------------------------------
		| Find the vrmf of the last cum ptf from ptfoptions file. |
		| The ptfvrmfFile is null because checkPtfOptions will    |
		| search the ptfoptions file only if the filename passed  |
		| is NULL. We need to check only ptfoptions file as the   |
		| last cum ptf was packaged in the last build cycle and   |
		| we should find a match for it there.                    |
		----------------------------------------------------------*/
		if (checkPtfOptions(lastCumPtf, foption, cumVRMF, ptfvrmfFile))
			fatal(ReqNotInPtfOptFile, lastCumPtf,ptfoptFile);
		splitVRMF(foption, cumVRMF, &vrmfentry);
		/*-----------------------------------------------------------
		| write the requisite only if the fix level of the last cum |
		| is not zero.                                              |
		------------------------------------------------------------*/
		if (atoi(vrmfentry.fix)) 
		{
			fprintf (prereqFp, "*prereq %s %s\n", 
				 foption, 
				 cumVRMF);
                }
	}

	/*-------------------------------------------------------------------
	| add the requisites in the tree nodes for ifreqs, coreqs and       |
	| prereqs. This is done in order to keep the highest req type       |
	| and highest vrmf for an fileset.                                  |
	--------------------------------------------------------------------*/

	addTreeReqs ("ifreqsList", IFREQ, top, internalTable);
	addTreeReqs ("coreqsList", COREQ, top, internalTable);
	addTreeReqs ("prereqsList", PREREQ, top, internalTable);

	/*------------------------------------------------------------------
	| After adding traverse the tree and print the requisites in       |
	| fileset.prereq file.                                              |
	-------------------------------------------------------------------*/
	traverse (treeroot, prereqFp);

	fflush (prereqFp);

        /*------------------------------------------------------
        | The base prereq needs to be added for following      |
        | cases:                                               |
        | 1. If this is a fileset ptf and there is no last cum |
        |    then write a prereq for the base level only if    |
        |    mod level is not zero.                            |
        | 2. If this is the first cumptf then write a base     |
        |    prereq only if the mod level for the base is not  |
        |    zero.                                             |
        | 3. If this is a cumptf and there is a last cum then  |
        |    write the base prereq if the mod level for the    |
        |    base is not zero.                                 |
        ------------------------------------------------------*/

        if ( !strlen (specialPtfType) && !strlen (lastCumPtf) )
             basePrereq(prereqFp, fileset, top, buildCycle);

        if ( strlen (specialPtfType) && !strlen (lastCumPtf) )
             basePrereq(prereqFp, fileset, top, buildCycle);

        if ( strlen (specialPtfType) && strlen (lastCumPtf) )
             basePrereq(prereqFp, fileset, top, buildCycle);

	fclose (prereqFp);
}
	

/*-----------------------------------------------------------------------
| Generate a prereq on the base level for this fileset.			|
-----------------------------------------------------------------------*/
void
basePrereq (FILE *prereqFp, char *fileset, char *top, char *buildCycle)
{
	char	 vrm[BUFSIZE];
	vrmftype vrmfentry;

	vrm[0] = NULL;

	/*-----------------------------------------------------------------
	| get the vrmf for the fileset from lpp_name.BLDCYCLE file.        |
	------------------------------------------------------------------*/
	getVRMF (fileset, top, vrm, buildCycle);
 
	/*------------------------------------------------------------------
	|Split the vrmf into version, release, mod and fix levels.         |
	-------------------------------------------------------------------*/
        splitVRMF(fileset, vrm, &vrmfentry);

	if (atoi(vrmfentry.mod))
	    fprintf (prereqFp, "*prereq %s %s.%s.%s.%s\n",
		fileset,
		skipLeadingZeros(vrmfentry.version),
		skipLeadingZeros(vrmfentry.release),
		skipLeadingZeros(vrmfentry.mod),
                skipLeadingZeros(vrmfentry.fix));
}

/*-----------------------------------------------------------------------
| This functions adds the requisites in a tree structure. Each tree node|
| has fileset name, requisite type and vrmf number for the fileset.       |
| New nodes are added to the tree is based upon the fileset value.       |
| This is done to keep the highest requisite type and highest vrmf      |
| number for each fileset.                                               |
-----------------------------------------------------------------------*/
void
addTreeReqs (char *listFile, reqtype reqType, char * top, char *internalTable)
{
	FILE	*listFp;
	char	buf[BUFSIZE];
	char	requisitePtf[BUFSIZE];
	char	reqFileset[FILESETLEN];
	char	reqvrmf[FILESETLEN];
	vrmftype vrmfentry;

	/*----------------------------------------------------------------
	| The ifreqsList, coreqsList or prereqsList files may or may 
	| nor exist.
	------------------------------------------------------------------*/
	if ( !(listFp = fopen (listFile, "r")) )
		return;

	/*----------------------------------------------------------------
	| For each requisite ptf number in the requisite file             
	------------------------------------------------------------------*/
	while ( stripComments (listFp, requisitePtf) != EOF )
	{
		reqFileset[0] = NULL;
		reqvrmf[0] = NULL;
		/*--------------------------------------------------------
		| First check to see if the requisite ptf is in the 
		| internal table file.   We will find the requisite 
		| ptf in the internal Table file only if it is also 
		| being packaged in this build cycle.  Then search 
		| the ptfoptions file. 
		| If the requisite ptf is not found in either files 
		| then give a fatal error and exit as we do not know 
		| which fileset this ptf belongs to.
		----------------------------------------------------------*/
		if ( checkPtfOptions(requisitePtf, reqFileset, reqvrmf, 
				     internalTable) )
		  if ( checkPtfOptions(requisitePtf, reqFileset, reqvrmf, 
				       NULL) )
		       fatal (ReqNotInPtfOptFile,
				requisitePtf, ptfoptFile);

                /*--------------------------------------------------------
		| split the vrmf string into version, release, mod and 
		| fix level
		----------------------------------------------------------*/
	        splitVRMF(reqFileset, reqvrmf, &vrmfentry);
                /*--------------------------------------------------------
		| We will store our requisites in a tree structure based 
		| upon the the fileset name. This done to keep track of 
		| the highest req type and vrmf number for each fileset. 
		| Once we are done with all the requisites for the 
		| current ptf, we will traverse the tree and write the 
		| requisites in the fileset.prereq file.
		| So search the tree and insert the new information at the
		| right place.
		----------------------------------------------------------*/
		insert(reqFileset, vrmfentry, reqType);
	}
	fclose (listFp);
}
