static char sccsid[] = "@(#)46  1.14     src/bldenv/pkgtools/processPtf/processPtf.c, pkgtools, bos41J, 9513A_all 3/27/95 13:43:17";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: addAparInfo
 *		checkPtfOptions
 *		checkSize
 *		createInfoFiles
 *		getLastPtf
 *		getNextPtf
 *		getFileset
 *		getPtfInfo
 *		main
 *		processPtf
 *		stripComments
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

#define internaltable	"/HISTORY/internalvrmfTable"
#define ptfoptionsfile	"/HISTORY/ptfoptions"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "ptf.h"

extern char 	*optarg;
extern int      errno;
char		*validPtfPrefix = "U4";

/*--------------------------------------------------------------
| This is the usage function. All the command line parameters  |
| are optional.                                                |
--------------------------------------------------------------*/
void
usage()
{
  fprintf(stderr, Usage, commandName);
  exit (-1);
}

/*---------------------------------------------------------------
| ptfoptFile is used in main, addListReqs, addPtfOptions and	|
| checkPtfOptions.						|
---------------------------------------------------------------*/
char	*ptfoptFile; 
char    *commandName = NULL;

main (int argc, char **argv)
{
	char	*buildType; /* to specify the build type*/
	char	*buildCycle; /* to specify the build cycle*/
	char	*top;/* to store the environment var TOP*/
	char	*ode_tools;/* to store the environment var ode_tools*/
	char	currentPtf[PTFLEN]; /* to store the current ptf number*/
	char	fileset[FILESETLEN]; /* tp store the fileset name for the ptf*/
	char	filesetVRMF[VRMFSIZE];/* to store the fileset's vrmf number*/
	vrmftype vrmfentry; /* to store the vrmf entry for the fileset*/
	char	specialPtfType[PTFLEN];/* to mark the special type ptfs*/
	char	fileName[MAXPATHLEN+1];/* to store the filename modified by a ptf*/
        char    *internalTable;/* pointer to internal vrmf table*/
        int    overrideFlag; /* to specify the override vrmf number*/
	int 	arg;
	char    *ptr;
	int	fakePtfFlag=FALSE;

	/*-----------------------------------------------------------
	| initialize the variable.                                  |
	------------------------------------------------------------*/
	specialPtfType[0] = NULL;
	currentPtf[0] = NULL;
	fileset[0] = NULL;
	filesetVRMF[0] = NULL;
      
	commandName = getCommandName( argv[0] );

        /*---------------------------------------------------------
	| parse the command line and read the version, release    |
	| mod and fix level into the vrmfentry.                   |
	----------------------------------------------------------*/
	while ( ( arg = getopt (argc, argv, "v:r:m:f:")) != EOF)
	{
	switch (arg)
		{
		case 'v':
			strcpy(vrmfentry.version, optarg);
		break;
		case 'r':
			strcpy(vrmfentry.release, optarg);
			break;
		case 'm':
			strcpy(vrmfentry.mod, optarg);
			break;
		case 'f':
			strcpy(vrmfentry.fix, optarg);
			break;
		case '?':
			usage();
			break;
		}
	}
    
	/*---------------------------------------------------------------------
	| If anything was given on the command line then make sure that all   |
	| the flags are specified. If neither of them is specified then set   |
	| the override flag to false else set it to true. If any of the flags |
	| is specified but not all then exit with a fatal error as we need    |
	| the complete vrmf.                                                  |
	----------------------------------------------------------------------*/
	if (strlen(vrmfentry.version) || strlen(vrmfentry.release) ||
			strlen(vrmfentry.mod) || strlen(vrmfentry.fix))
		if (strlen(vrmfentry.version) && strlen(vrmfentry.release) &&
			strlen(vrmfentry.mod) && strlen(vrmfentry.fix))
			overrideFlag = TRUE;
		else
                        usage();
	else
		overrideFlag = FALSE;

	/*-------------------------------------------------------------------
	| Get the environment variables TOP, BLDCYCLE and ODE_TOOLS. If     |
	| TOP is not set then give a fatal error. ODE_TOOLS and BLDCYCLE    |
	| may or may not be set.                                            |
	-------------------------------------------------------------------*/
	if ( !(buildType = getEnvVar ("BUILD_TYPE")) )
		fatal (BuildTypeNotSet);
	if ( !(top = getEnvVar ("TOP")) )
		fatal (SelfixTopNotSet);
	buildCycle = getEnvVar ("BLDCYCLE");
	ode_tools = getEnvVar ("ODE_TOOLS");

	/*-----------------------------------------------
	| Set up some path names for later use.		|
	-----------------------------------------------*/
	ptfoptFile = xmalloc (strlen(top) + strlen(ptfoptionsfile) + 1);
	sprintf(ptfoptFile, "%s%s", top, ptfoptionsfile);

	internalTable = xmalloc (strlen(top) + strlen(internaltable) +
				 strlen(buildCycle) + 2);
	sprintf(internalTable, "%s%s.%s", top, internaltable, buildCycle);

	getPtfInfo (currentPtf, fileset, specialPtfType);

	if ( strncmp(currentPtf, validPtfPrefix, strlen(validPtfPrefix)))
		fakePtfFlag = TRUE;
#ifdef DEBUG
	printf ("ptf is %s, fileset is %s\n",
		currentPtf,fileset);
#endif
	/*---------------------------------------------------------------
	| We do not check the return code from checkPtfOptions here.	|
	| Since checkPtfOptions is used to check either the ptfoptions 	|
	| file or the internal vrmf table, an error exists only if the 	|
	| PTF is not found in either of these files.  Right now we only	|
	| want to verify the info in ptfoptions.			|
	---------------------------------------------------------------*/
	
	checkPtfOptions (currentPtf, fileset, filesetVRMF, NULL);

	processPtf (fileset, top, overrideFlag,
                    specialPtfType, internalTable, buildCycle, fakePtfFlag);
	createInfoFiles (top);

	/*-------------------------------
	| Delete any empty files.	|
	-------------------------------*/
	checkSize ("lpp.doc");
	sprintf (fileName, "%s.prereq",fileset);
	checkSize (fileName);
	exit (0);
}
	 
/*---------------------------------------------------------------
| Read wk_ptf_pkg file and fill in ptf, fileset and vrmf.	|
| Verify correctness of the file according to the following	|
| rules:							|
| 1.  pmp_ptf, pkg_ptf, opp_ptf, C_ptf, enh_ptf should only 	|
|     have one entry						|
| 2.  all other types should only have one fileset listed	|
|     per entry							|
| 3.  all entries must have same fileset name (also only for	|
|     non-special case ptfs)					|
| The expected format of wk_ptf_pkg is:				|
|								|
| PTF#|APARS|FILENAME|OPTION:VRMF|IFREQS|COREQS|PREREQS|	|
| The 4th field may or may not have ":VRMF". It will be given   |
| only in cases where we want to override the default           |
| mechanism for calculating the vrmf number.                    |
| Example:							|
| U412143|IX37982 IX38009|/bin/bsh|bos.obj.bsh|U412409|||	|
| OR                                                            |
| U412143|IX37982|/bin/bsh|bos.obj.bsh:4.1.0.3|U412409|||	|
---------------------------------------------------------------*/
void
getPtfInfo ( char *ptfName, char *fileset, char *specialPtfType )
{
	FILE	*ptfPkgFp;
	char	buf[BUFSIZE];
	char	fileName[MAXPATHLEN+1];
	char	*ptr ;

	ptfPkgFp = openFile ( "./wk_ptf_pkg", "r");

	while ( stripComments (ptfPkgFp, buf) != EOF )
	{
	    ptr = strtok (buf, "|");
	    if ( !strlen (ptfName) )
		strcpy (ptfName, ptr);
	    else
	    {
		if ( strcmp (ptfName, ptr) )
		    fatal (MultiplePtfs);
	    }

	    /*-------------------------------------------
	    | Skip over apars field.			|
	    -------------------------------------------*/
	    ptr = strtok (NULL, "|");

	    /*-----------------------------------------------------------
	    | Set flag if special case cum ptf.                         |
	    -----------------------------------------------------------*/
	    ptr = strtok (NULL, "|");
	    if ( strstr(ptr, "cum_ptf") )
		strcpy (specialPtfType, ptr);
	    else
		strcpy (fileName, ptr);

	    ptr = strtok (NULL, "|");
 
	    if ( strlen (specialPtfType) )
	    {
		while ( stripComments(ptfPkgFp, buf) != EOF )
		    fatal (MultipleEntries);
	    }
	    /*-------------------------------------------
	    | Get the fileset for the ptf 		|
	    -------------------------------------------*/
	    getFileset (ptr, fileset, fileName);
	}

	fclose (ptfPkgFp);
}

/*-----------------------------------------------------------------------
| Extract the fileset and vrmf for an entry from a buffer.              |
| If there are multiple fileset in the buffer they will be              |
| separated by a " " and an error should be generated.			|
|									|
| If fileset is empty, this is the first entry so copy	                |
| the information.  Otherwise, do a compare and generate an error if	|
| the fileset does not match.  These type of updates should	        |
| only affect one fileset.					        |
|                                                                       |
| If there is ":" in the fileset field from wk_ptf_pkg file             |
| then we need to discard everything after the ":" to get the right     |
| fileset name. Example :                                               |
| bos.rte:04.01.0000.0000                                               |
| This would be the case when a new vrmf number is specified  with      |
| the fileset field for the ptf in the                                  |
| ptf_pkg.bldcycle file instead of using the default mechanism to       |
| calculate the new vrmf number.                                        |
-----------------------------------------------------------------------*/
void
getFileset (char *buf, char *fileset, char *fileName)
{
	char	*ptr;

	/*-------------------------------------------------------
	| stripBlanks returns the next non-space character	|
	| in the string or NULL.				|
	-------------------------------------------------------*/
	if ( stripBlanks (buf) )
		fatal (MultipleFilesetsForFile, fileName, "wk_ptf_pkg");

	if (ptr = strchr(buf, ':'))
		*ptr = NULL;

        if ( strlen(fileset) )
        {
		if ( strcmp (buf, fileset) )
		    fatal (MultipleFilesetForPtf);
        }
        else
		strcpy (fileset, buf);
}

/*-----------------------------------------------------------------------
| Verify that the $TOP/HISTORY/ptfoptions file has the same fileset	|
| for this ptf that is in the ptf_pkg file.  The expected format of the	|
| ptfoptions file is:							|
| 									|
| PTF# fileset1,fileset2... vrmf						|
| 									|
| where only packaging type ptfs may have multiple fileset.		|
|									|
| If fileset is NULL we are just checking to see if a ptf is in the	|
| ptfoptFile.  In either case return 0 if the ptf is found and 1 if not.|
| If the file name is set then we search the internal vrmf Table else   |
| we search the ptfoptions file.
|									|
| NOTE:  ptfoptFile is a global variable.				|
-----------------------------------------------------------------------*/
int
checkPtfOptions (char *currentPtf, char *fileset, char *vrmf, char *filename)
{
	FILE	*ptfoptFp;
	char	buf[BUFSIZE];
	char	*ptr;
	char	*foption;
	char	*fvrmf;
	int	match=0;

        if ( strlen(filename))
	    ptfoptFp = openFile (filename, "r");
        else
	    ptfoptFp = openFile (ptfoptFile, "r");

	/*---------------------------------------------------------------
	| search the file for ptfnumber. If found then increment        |
	| match and break.                                              |
	----------------------------------------------------------------*/
	while ( stripComments(ptfoptFp, buf) != EOF )
	{
		if ( strncmp (buf, currentPtf, strlen(currentPtf)) )
		    continue;

		match++;
		break;
	}

	fclose (ptfoptFp);
        /*----------------------------------------------------------------
	| If a match for the ptfnumber was found in the file then break  |
	| the rest of the line into fileset and vrmf number.              |
	----------------------------------------------------------------*/
	if ( match )
	{
		/*----------------------------------------------------------------
		| We should always find the fileset with the ptf number.          |
		| If one was not found then give fatal error.                    |
		----------------------------------------------------------------*/
		if ( (ptr = strpbrk (buf, " \t")) == NULL )
		    fatal (FilesetNotFound,currentPtf,ptfoptFile);

		++ptr;
		/*----------------------------------------------------------------
		| break the rest of the line into fileset and vrmf number.        |
		----------------------------------------------------------------*/
                foption = strtok(ptr, " \t");
                fvrmf = strtok(NULL, " \t");
		/*------------------------------------------------------------------
 		| If we are searching the internal table then we should always     |
		| find the vrmf number. If one was not found then give fatal error.|
		| If we are searching the ptfoptions file then we may or may       |
		| may not find the vrmf number.                                    |
		-------------------------------------------------------------------*/
                if ( strlen(filename) && ! strlen(fvrmf))
		    fatal (VRMFNotFound,currentPtf,ptfoptFile);
		/*------------------------------------------------------------------
		| If fileset was given to us then we want to make sure that it      |
		| matches with the ptf's fileset in ptfoptions file.                |
		-------------------------------------------------------------------*/
		if ( strlen (fileset) && strcmp (foption, fileset) )
		    fatal (PtfoptionsMismatch, currentPtf, ptfoptFile, fileset); 
		else
		/*------------------------------------------------------------------
		| If fileset was not given then return the fileset and vrmf number.|
		| Also return zero.                                                |
		------------------------------------------------------------------*/
		    if ( !strlen (fileset) )
                      {
                       strcpy(fileset, foption);/* fileset is output parameter*/
                       strcpy(vrmf, fvrmf);/* vrmf is output parameter*/
                      }
		return (0);
	}
	else
		return (1);
}

/*-----------------------------------------------------------------------
| Generate the list files for this ptf and set up requisite information.|
-----------------------------------------------------------------------*/
void
processPtf (char *fileset, char *top, int overrideFlag, char *specialPtfType, 
	char *internalTable, char* buildCycle, int fakePtfFlag)
{
	char	filesetDir[MAXPATHLEN+1];
	char	cumsListFile[MAXPATHLEN+1];
	char	ptfsListFile[MAXPATHLEN+1];
	char	lastCumPtf[PTFLEN];
	char	lastFilesetPtf[PTFLEN];

	lastCumPtf[0] = '\0';
	lastFilesetPtf[0] = '\0';

	/*-------------------------------------------------------
	| Get the last cumptf from cumsList file and last	|
	| fileset ptf from ptfsList file for this fileset.	|
	-------------------------------------------------------*/
	getDirName (fileset, top, filesetDir);
	strcpy (cumsListFile, filesetDir);
	strcat (cumsListFile, "/cumsList");
	strcpy (ptfsListFile, filesetDir);
	strcat (ptfsListFile, "/ptfsList");
	getLastPtf (cumsListFile, lastCumPtf);
	getLastPtf (ptfsListFile, lastFilesetPtf);

#ifdef DEBUG
	printf ("last cum ptf is %s\n", lastCumPtf);
	printf ("last fileset ptf is %s\n", lastFilesetPtf);
#endif

	/*---------------------------------------------------------------
	| Get the list files from the last fileset ptf.  If this	|
	| is a cum, also include the list files from the last cum ptf.	|
	---------------------------------------------------------------*/
	createEmptyLists ();
	if ( strlen (lastCumPtf) && !strcmp (specialPtfType, "cum_ptf") )
		createListFiles (filesetDir, lastCumPtf, top, 
				internalTable, fakePtfFlag);
	if ( strlen (lastFilesetPtf) )
		createListFiles (filesetDir, lastFilesetPtf, top, 
				internalTable, fakePtfFlag);

	/*---------------------------------------------------------------
	| Add the information from wk_ptf_pkg into the list files.	|
	---------------------------------------------------------------*/
	addNewPtfInfo ();

	addRequisites (fileset, specialPtfType, 
                        lastCumPtf, overrideFlag, top, internalTable, buildCycle);
}

/*-----------------------------------------------------------------------
| Get the last ptf if one exists from the list file specified for this	|
| fileset.  The list file is located in the fileset directory.	|
-----------------------------------------------------------------------*/
void
getLastPtf (char *listFile, char *lastPtf)
{
	char	buf[BUFSIZE];
	FILE	*listFileFp;

	if ( ! (listFileFp = fopen(listFile, "r")) )
	    return;

	while ( stripComments (listFileFp, buf) != EOF )
		strcpy (lastPtf, buf);
}

/*-----------------------------------------------------------------------
| Get the next valid ptf from the list file specified for this	|
| fileset.  The list file is located in the fileset directory.	|
-----------------------------------------------------------------------*/
void
getNextPtf (char *listFile, char *Ptf, char *nextPtf)
{
	char	buf[BUFSIZE];
	FILE	*listFileFp;
	int	found=0;

	if ( ! (listFileFp = fopen(listFile, "r")) )
	    return;

	while ( stripComments (listFileFp, buf) != EOF )
	{
	    if ( found ) 
	    {
		if (! strncmp(buf, validPtfPrefix, strlen(validPtfPrefix)) ) 
	        {
	  	   strcpy (nextPtf, buf);
	    	   break;
	        }
	    }
	    else
	    {
	        if ( ! strncmp(buf, Ptf, strlen(Ptf)))
	           found++;	
	    }
	}

	fclose (listFileFp);
}

/*-----------------------------------------------------------------------
| Create the lpp.doc file.  For each apar in aparsList look in          |
| $TOP/HISTORY/memo_info and copy all text for the apar into            |
| the output files.			                                |
-----------------------------------------------------------------------*/
void
createInfoFiles (char *top)
{
	FILE	*memoinfoFp, *lppdocFp, *aparsFp;
	char	*memofileName[MAXPATHLEN+1];
	char	nextApar[BUFSIZE], text[BUFSIZE];
	struct stat statbuf;

	sprintf(memofileName, "%s/%s", top, "/HISTORY/memo_info");
	memoinfoFp = fopen (memofileName, "r");

	lppdocFp = openFile ("lpp.doc", "w");

	aparsFp = openFile ("aparsList", "r");
	while ( stripComments (aparsFp, nextApar) != EOF )
	{
	    if ( memoinfoFp )
	    {
		if ( stat (memofileName, &statbuf) == -1 )
			fatal (StatFailed, memofileName, errno);

		if ( statbuf.st_size != 0 )
		addAparInfo (lppdocFp, memoinfoFp, nextApar, "memo_info");
	    }
	}
	fclose (memoinfoFp);
	fclose (lppdocFp);
	fclose (aparsFp);
}
	
/*-----------------------------------------------------------------------
| Find the apar in the input file and copy the text for that apar into	|
| the output file.  All apar text is terminated by the "<@>" string.	|
| If the input file does not contain the delimiter string display a	|
| warning message.							|
-----------------------------------------------------------------------*/
int
addAparInfo (FILE *outFp, FILE *inFp, char *apar, char *fileName)
{
	char	buf[BUFSIZE];
	int	writeFlag=0;
	char	*ptr;
	int	formatFlag=0;

	rewind (inFp);
	while ( fgets (buf, BUFSIZE, inFp) )
	{
	    if ( strstr (buf, "<@>") && !formatFlag )
		formatFlag++;

	    if ( writeFlag )
	    {
		if ( strstr (buf, "<@>") )
		{
		    fputs ("\n",outFp);
		    return (0);
		}
		fputs (buf, outFp);
	    }

	    /*---------------------------------------------------
	    | Make sure apar name is not terminated by newline.	|
	    ---------------------------------------------------*/
	    if ( ptr = strchr (apar, '\n') )
		*ptr = '\0';
	    if ( strncmp (buf, apar, strlen (apar)) )
		continue;
	    else
	    {
		fputs (buf, outFp);
		writeFlag++;
	    }
	}
	if ( !formatFlag)
		warning (InvalidFileFormat, fileName);
	return (1);
}

/*---------------------------------------
| Strip comment lines as we go.         |
---------------------------------------*/
int
stripComments (FILE *insfp, char *line)
{
	char *num;
	char *ptr;

	while   ( (num = fgets(line,BUFSIZE,insfp)) != NULL )
	{
		/*-------------------------------------------------------
		| Position line[i] at first non-whitespace character.	|
		-------------------------------------------------------*/
		for ( ptr=line; isspace (*ptr); ptr++ )
		{
			if ( (*ptr == '\n') )
			    break;
		}
		if ( (*ptr != '#') && (*ptr != '\n') )
			break;
	}
			
	if (num == (char *) NULL)
		return (EOF);

	if ( num = strchr (line, '\n') )
		*num = (char *) NULL;
	return 0;
}

/*-----------------------------------------------
| Check size of file and unlink if empty.       |
-----------------------------------------------*/

void
checkSize ( char * fileName )
{
	struct stat statbuf;

	if ( stat (fileName, &statbuf) == -1 )
		fatal (StatFailed, fileName, errno);

	if ( statbuf.st_size == 0 )
		unlink (fileName);

	return;
}

