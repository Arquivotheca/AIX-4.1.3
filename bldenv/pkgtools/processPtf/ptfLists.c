static char sccsid[] = "@(#)48  1.8     src/bldenv/pkgtools/processPtf/ptfLists.c, pkgtools, bos41J, 9512A_all 3/6/95 10:46:59";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: addInfo
 *		addNewPtfInfo
 *		checkInternalTable
 *		createEmptyLists
 *		createList
 *		createListFiles
 *		getValidPtf
 *		searchListFile
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
#include <sys/param.h>
#include "ptf.h"

extern char *validPtfPrefix;
extern char *ptfoptFile;

/*-----------------------------------------------------------------------
| Get the list information for a ptf.  If the list files for the 	|
| ptf already exist, add the information to the current files.		|
| Otherwise, copy the list files into the current directory and we	|
| will add to them.							|
-----------------------------------------------------------------------*/
void
createListFiles (char *filesetDir, char *lastPtf, char *top, 
		char *internalTable, int fakePtfFlag)
{
	FILE	*fp;
	char	lastPtfDir[MAXPATHLEN+1];

	strcpy (lastPtfDir, filesetDir);
	strcat (lastPtfDir, "/");
	strcat (lastPtfDir, lastPtf);

	createList (lastPtfDir, "aparsList", NULL, NULL, fakePtfFlag);
	createList (lastPtfDir, "filenamesList", NULL, NULL, fakePtfFlag);
	createList (lastPtfDir, "ifreqsList", top, internalTable, fakePtfFlag);
	createList (lastPtfDir, "coreqsList", top, internalTable, fakePtfFlag);
	createList (lastPtfDir, "prereqsList", top, internalTable, fakePtfFlag);
}
	
/*-----------------------------------------------------------------------
| Read the list files for the last ptf and add the entries to the	|
| current list file if they are not already there.			|
-----------------------------------------------------------------------*/
void
createList ( char *lastPtfDir, char *listFile, char *top, 
		char *internalTable, int fakePtfFlag)
{
	char	fileName[MAXPATHLEN+1];
	char	oldListFile[MAXPATHLEN+1];
	char	buf[BUFSIZE];
	char	cmd[BUFSIZE];
	FILE	*oldListFp, *newListFp;
	int	badPtf=0;

	strcpy (fileName, lastPtfDir);
	strcat (fileName, "/");
	strcat (fileName, listFile);

	if ( !(oldListFp = fopen (fileName, "r")) )
		return;

	newListFp = openFile (listFile, "a");

	while ( stripComments (oldListFp, buf) != EOF )
	{
		if ( ! searchListFile (listFile, buf) )
		{
		    /*------------------------------------------------
		    | Check for fake PTFs in ifreqs, coreqs, and     |
		    | prereqs Lists.  If listFile is 'aparsList' or  |
		    | 'filenamesList' then the value of top is NULL  |
		    | here, and we don't need to do the PTF check.   |
		    ------------------------------------------------*/
		    if ( top != (char *) NULL && !(fakePtfFlag) )
		    {
		       if ( ! getValidPtf(buf, top, internalTable) )
			    badPtf++;
		    }
		    fputs (buf, newListFp);
		    fputs ("\n", newListFp);
		}
	}
	fclose (newListFp);
	fclose (oldListFp);

	if ( !(fakePtfFlag) && badPtf)
	{
		/*---------------------------------------------------------
		| Update the reqsList file by replacing the fake PTF with |
		| the next valid PTF (found by getValidPtf).		  |
		---------------------------------------------------------*/
		strcpy(oldListFile, fileName);
		strcat(oldListFile, ".old");	
		sprintf(cmd, "cp %s %s 2>/dev/null", fileName, oldListFile);
		system(cmd);
	
		oldListFp = openFile (oldListFile, "r");
		newListFp = openFile (fileName, "w");
		while ( stripComments (oldListFp, buf) != EOF )
		{
			getValidPtf(buf, top, internalTable);
			fputs(buf, newListFp);
			fputs("\n", newListFp);
		}

		fclose(oldListFp);
		fclose(newListFp);
		sprintf(cmd, "rm %s 2>/dev/null", oldListFile);
		system(cmd);
	}
}

/*-----------------------------------------------------------------------
| Add the wk_ptf_pkg information to the list files.  The expected	|
| format is:								|
|									|
| PTF#|APARS|FILENAME|OPTION:V.R.M.F|IFREQS|COREQS|PREREQS|		|
| The ":V.R.M.F" in the 4th field is optional and may or may not        |
| be given. ":V.R.M.F" will be given only in cases where we want to     |
| override the default vrmf.                                            |
| Example:								|
| U412143|IX37982 IX38009|/bin/bsh|bos.obj.bsh|U412409|||		|
| OR                                                                    |
| U412143|IX37982 IX38009|/bin/bsh|bos.obj.bsh:4.1.0.3|U412409|||		|
-----------------------------------------------------------------------*/
void
addNewPtfInfo ( )
{
	FILE	*wkFp, *ifreqFp, *coreqFp, *prereqFp, *aparFp, *fileFp;
	char	*ptr = NULL;
	char	buf[BUFSIZE];
	char	aparbuf[BUFSIZE];

	wkFp = openFile ("./wk_ptf_pkg", "r");
	ifreqFp = openFile ("ifreqsList", "a");
	coreqFp = openFile ("coreqsList", "a");
	prereqFp = openFile ("prereqsList", "a");
	aparFp = openFile ("aparsList", "a");
	fileFp = openFile ("filenamesList", "a");

	/*---------------------------------------------------------------
	| This function uses our own modified version of strtok.	|
	| The "real" strtok skips leading separators so it does not	|
	| return an empty field.  Our requisite fields may be empty in	|
	| which case we just don't add any data to the list.		|
	---------------------------------------------------------------*/
        buf[0] = 0;
	while ( stripComments (wkFp, buf) != EOF )
	{
		ptr = ptfstrtok (buf, "|");
		ptr = ptfstrtok (NULL, "|");
		addInfo (ptr, aparFp, "aparsList");
		ptr = ptfstrtok (NULL, "|");
		if (strcmp(ptr, "cum_ptf"))
		{
		    addInfo (ptr, fileFp, "filenamesList");
		}
		ptr = ptfstrtok (NULL, "|");

		/*-------------------------------------------------------
		| These fields may not exist or they may be empty.	|
		-------------------------------------------------------*/
		if ( ptr = ptfstrtok (NULL, "|") )
		    addInfo (ptr, ifreqFp, "ifreqsList");
		if ( ptr = ptfstrtok (NULL, "|") )
		    addInfo (ptr, coreqFp, "coreqsList");
		if ( ptr = ptfstrtok (NULL, "|") )
		    addInfo (ptr, prereqFp, "prereqsList");
	}
	fclose (ifreqFp);
	fclose (aparFp);
	fclose (prereqFp);
	fclose (fileFp);
	fclose (coreqFp);
}


/*-----------------------------------------------------------------------
| Add a space separated list of entries to the given list file if they	|
| are not already there.  Can't use strtok because the caller is	|
| using it.								|
-----------------------------------------------------------------------*/
void
addInfo (char *buf, FILE *listFp, char *listFile)
{
	char	*ptr, *lastPtr;
	char	entry[BUFSIZE];
	int	len;

	if ( !strlen(buf) )
		return;
	lastPtr = buf;
	do
	{
		ptr = strchr (buf, ' ');
		if ( ptr )
		{
			*ptr = ',';
			len = ptr - lastPtr;
			strncpy (entry, lastPtr, len);
			entry[len] = '\0';
		}
		/*---------------------------------------
		| At the end of the list.		|
		---------------------------------------*/
		else
			strcpy (entry, lastPtr);
		/*-------------------------------------------
		| Force this entry to be written to the file,|
		| so that the next call to searchListFile    |
		| function will be able to keep subsequent   |
		| duplicates from showing up.                |
		--------------------------------------------*/
		if ( !searchListFile (listFile, entry) )
		{
			fputs (entry, listFp);
			fputs ("\n", listFp);
			fflush (listFp);
		}
		if ( ptr ) 
		{
                        lastPtr = ptr;
                        while ( *lastPtr++ == ' ' );
		}

	}
	while ( ptr );
}
			

/*-----------------------------------------------------------------------
| Search a list file for an entry.  Return 1 if the entry is found,	|
| otherwise return 0.							|
-----------------------------------------------------------------------*/
int
searchListFile ( char *listFileName, char *entry )
{
	FILE	*fp;
	char	buf[BUFSIZE];

	if ( !(fp = fopen (listFileName, "r")) )
		return (0);

	while ( stripComments(fp, buf) != EOF )
	{
		if ( !strcmp (buf, entry) )
		{
		    fclose (fp);
		    return (1);
		}
	}
	fclose (fp);
	return (0);
}

/*-----------------------------------------------------------------------
| Create empty list files in the ptf directory.  This ensures that	|
| we do not use data from a previous run since the createLists function	|
| opens the files for append access.					|
-----------------------------------------------------------------------*/
void
createEmptyLists ()
{
	FILE	*fp;

	fp = openFile ("ifreqsList", "w");
	fclose (fp);
	fp = openFile ("aparsList", "w");
	fclose (fp);
	fp = openFile ("coreqsList", "w");
	fclose (fp);
	fp = openFile ("prereqsList", "w");
	fclose (fp);
	fp = openFile ("filenamesList", "w");
	fclose (fp);
}

/*---------------------------------------------------------------
| Check for 'fake' PTFs.  If Ptf is valid, simply returns true.	|
| If Ptf is an internal PTF, get next valid one and return in	|
| 'Ptf' 
---------------------------------------------------------------*/
int 
getValidPtf ( char *Ptf, 
		char *top, 
		char *internalTable )
{
	char filename[MAXPATHLEN+1];
	char fileset[MAXPATHLEN+1];
	char filesetName[MAXPATHLEN+1];
	char filesetDir[MAXPATHLEN+1];
	char newPtf[PTFLEN];	
	char vrmf[VRMFSIZE];	

	if ( ! strncmp (Ptf, validPtfPrefix, strlen(validPtfPrefix)) )
	{
	     /*--------------------------------------------------
	     | If Ptf is valid there is nothing to do		|
	     --------------------------------------------------*/
	     return 1;
	}
	else
	{
	     /*--------------------------------------------------
	     | Requisite PTF is a 'fake' PTF			|
	     --------------------------------------------------*/
	     newPtf[0] = NULL;
	     fileset[0] = NULL;
	     vrmf[0] = NULL;

	     /*--------------------------------------------------
	     | Get fileset for this ptf from ptfoptions file.	|
	     | If not in ptfoptions file, look in the internal  |
	     | vrmf table.					|
	     --------------------------------------------------*/
	     if ( checkPtfOptions (Ptf, fileset, vrmf, NULL) )
	     	if ( checkPtfOptions (Ptf, fileset, vrmf, internalTable) )
			fatal (ReqNotInPtfOptFile, Ptf, ptfoptFile);

	     strcpy(filesetName, fileset);

	     /*--------------------------------------------------
	     | Build full pathname for <fileset>/ptfsList 	|
	     --------------------------------------------------*/
	     replace_char (fileset, ".", "/" );
	     getDirName (fileset, top, filesetDir);
	     sprintf(filename, "%s/ptfsList", filesetDir);


	     /*--------------------------------------------------
	     | Get next valid ptf from <fileset>/ptfsList.      |
	     --------------------------------------------------*/
	     getNextPtf (filename, Ptf, newPtf);
	     if ( strlen(newPtf))
		strcpy(Ptf, newPtf);
	     else
	     {
	        /*--------------------------------------------------
		| If no valid ptf was found in <fileset>/ptfsList, |
		| perhaps this ptf is being packaged in the 	   |
		| current build cycle.  Use the fileset to get ptf |
		|          from the internal vrmf table.	   |
	        --------------------------------------------------*/
		checkInternalTable(newPtf, filesetName, internalTable);
		if ( strlen(newPtf) )
		    strcpy(Ptf, newPtf);
	     }

	     if ( ! strlen(newPtf))
	     {
	        /*----------------------------------------------------
	        | If no valid ptf was found in ptfsList and the ptf  |
	        | is not being packaged in this build cycle, use the |
	        | last cum ptf, and its fileset and vrmf.	     |
	        ----------------------------------------------------*/
		sprintf(filename, "%s/cumsList", filesetDir);
		getLastPtf(filename, newPtf);
		if ( strlen(newPtf))
		   strcpy(Ptf, newPtf);
		else
		   fatal(InvalidPtf, Ptf, fileset);
	     }

	     return 0;
	}
}


void
checkInternalTable(char *newPtf, 
		   char *fileset, 
		   char *internalTable)
{
	FILE *tableFp;
	char buf[BUFSIZE];
	char *ptf;
	char *foption;
	int  match=0;

	tableFp = openFile(internalTable, "r");
	while (stripComments(tableFp, buf) != EOF)
	{
		ptf = strtok(buf, " \t");
		foption = strtok(NULL, " \t");
		if (strcmp(foption, fileset))
		   continue;
		match++;
		break;
	}
	fclose(tableFp);

	if ( match )
		strcpy(newPtf, ptf);
	return;
}
	
