static char sccsid[] = "@(#)49  1.14  src/bldenv/pkgtools/size.c, pkgtools, bos412, GOLDA411a 6/23/94 13:06:37";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: getInSize
 *		getDirEnt
 *		getLibInfo
 *		inSize
 *		initHashTable
 *		outSize
 *		updateSize
 *		updateTmpSize
 *		checkInSize
 *		addToHashTable
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <swvpd.h>
#include "adeinventory.h"
#include "ade.h"
#include "acfList.h"


/* 
 *------------------------------------------------------
 * Declare functions within this unit.
 *------------------------------------------------------
 */
void		addToHashTable (int, char *, off_t, off_t);
void            getInSize(char *, int, int, char *, char *);
void		inSize(FILE *, int, int, char *);
void		initHashTable();
void		updateSize(char *, off_t, char, int);
void		updateTmpSize( off_t );
DirEntry	*getDirEnt();

static int	libUpdate (InsEntry *insentry, off_t size, 
			   char *type, ACFEntry *acf) ;
static off_t	getLibInfo (char *, char);
static off_t	getLibSize(char *libName) ;

/* 
 *------------------------------------------------------
 * Extern variables to this unit, but
 * directly accessed within this unit.
 *------------------------------------------------------
 */
extern	List 	*dirList;
extern	FILE 	*opsize_ptr;
extern	char	**msgArray ;		/* ptr to error msg table	*/

/* 
 *------------------------------------------------------
 * Global variables defined by this unit.
 *------------------------------------------------------
 */
LibEntry	*libTable[LIBHASHTABLE];

/*
 *-------------------------------------------------------
 * The following file name variable is at module
 * to support better error messages.  
 *-------------------------------------------------------
 */
static char InSizeFile[MAXPATHLEN+1];


void 
getInSize( char *lppOption, int rFlag, int DFlag, char *lppname, char *outputDir)	
{	
	FILE *insize_ptr;

	if ( strlen(outputDir) )
		sprintf(InSizeFile,"%s/%s.insize",outputDir,lppOption);
	else
		sprintf(InSizeFile,"./%s.insize",lppOption);

	if ( insize_ptr=fopen(InSizeFile,"r") )
		inSize(insize_ptr, rFlag, DFlag, lppname);
}

void inSize(FILE *fp, int rFlag, int DFlag, char *lppname)
{
   char newLine[MAXBUFFER+1];
   char value1[20];
   char value2[20];
   char *ptr;
   int rc=0;
   int found=0;
   DirEntry *dirEntry;
   DirEntry *dirListEntry;

   while (stripComments(fp, newLine) != EOF) 
   {
      value1[0]  = '\0';			/* Init value1.    */
      dirEntry  = getDirEnt();
      bzero(value2,20);
      sscanf(newLine,"%s %s %s", dirEntry->Dir, value1, value2);
      switch (value1[0])
      {
         case '+':
         case '-':
            dirEntry->inputOperator = value1[0];
	    ptr = value1 + 1;
	   /* 
	    *------------------------------------------------------------
	    * If only operator is present, can't calculate size
	    * requirements. 
	    *------------------------------------------------------------
	    */
	    if (*ptr != '\0')
	    {
		if (strspn(ptr,"0123456789")==(strlen(value1)-1))
		{
		    dirEntry->insize = (off_t) atoi(ptr);
		    dirEntry->tempsize = (off_t) atoi(value2);
		}
		else 
		{
		    warning (msgArray[InvalidSizeInsize], InSizeFile, newLine);
		    continue;
		}
	    }
            else /* No value given after operator */
            {
		warning (msgArray[NoValueForOperator], InSizeFile, newLine);
	        continue;
	    }
	    break;
         default:
            dirEntry->inputOperator = '\0';
	    if (strspn(value1,"0123456789")==(strlen(value1))) 
            {
	       dirEntry->insize = (off_t) atoi(value1);
               dirEntry->tempsize = (off_t) atoi(value2);
	    }
	    else 
            {
	       warning (msgArray[InvalidSizeInsize], InSizeFile, newLine);
               continue;
            }
      }
      if ( rc = checkInSize(dirEntry, rFlag, DFlag))
      {
         rc = 0;
         continue;
      }

      listRewind(dirList);
      while ((!listEnd(dirList)) && (!found))
      {
         dirListEntry = (DirEntry *) listGet(dirList);

         /* If we have a match, adjust the sizes as required by the
            insize entry.  TMP space will always be added.         */

         if (!strcmp(dirEntry->Dir, dirListEntry->Dir))
         {
            dirListEntry->tempsize += dirEntry->tempsize;

            dirListEntry->inputOperator = dirEntry->inputOperator;

            if ((dirEntry->inputOperator == '+') || (dirEntry->inputOperator == '-'))
               dirListEntry->insize = dirEntry->insize;
            else
            {
               if ((dirEntry->insize - dirListEntry->totsize) > 0)
                  dirListEntry->totsize = dirEntry->insize;
            }

            ++found;
         }
      }

      if (!found)
         listAppend(dirList, dirEntry);
      else
         found = 0;

   }	/* Done processing input lines. */

   fclose(fp);
}

/*-----------------------------------------------------------------------
| Process the insize line using the rules for root, share, and usr as	|
| follows:								|
| If -r flag ignore all .insize entries beginning with /usr.		|
| If !-r flag (usr processing) ignore all .insize entries that do not	|
|    begin with /usr.							|
| if -D flag ignore all entries that do not begin with /usr/share.	|
| NOTE:  A non-zero return code will indicate to the caller that	|
| the entry should be ignored.						|
-----------------------------------------------------------------------*/
checkInSize(DirEntry *dirEntry, int rFlag, int DFlag)
{
   if ( !strncmp(dirEntry->Dir, "/usr/share", strlen ("/usr/share")) &&
	DFlag)
		return (0);

   if ( !strncmp(dirEntry->Dir, "/usr", strlen ("/usr")) &&
	!DFlag && !rFlag )
		return (0);

   if ( rFlag && strncmp(dirEntry->Dir, "/usr", strlen ("/usr")) )
		return (0);

   return -1;
}

/*-----------------------------------------------------------------------
| Print out the size file, looping thru the linked list of directories  |
| The size should never be 0 or negative.                               |
-----------------------------------------------------------------------*/

void
outSize(int updtFlag, char *tcbFile, char *lppOption, int bootFlag, int rFlag)
 {
	DirEntry	*dirEntry;
	int		updateSpace=0;
	struct stat	statBuf;
	int		tmpFlag=0;

	/*---------------------------------------------------------------
	| If this is an update the size of the reposDir and security	|
	| need to be adjusted.	The security dir is only updated if	|
	| there are any tcb entries, which is why we stat the tcb file.	|
	---------------------------------------------------------------*/
	if (updtFlag)
	{
	    updateSpace = adjustReposDir (lppOption);
	    if ( stat (tcbFile, &statBuf) == -1 )
		statBuf.st_size = 0;
	}

	/* Loop through	the "dirList" and write	size information. */
	listRewind(dirList);
	while (!listEnd(dirList)) 
	{
		dirEntry = (DirEntry *)	listGet(dirList);
		/*-------------------------------------------------------
		| Repository and security directories have not been	|
		| converted to blocks yet.				|
		-------------------------------------------------------*/
		if ( !strcmp(dirEntry->Dir, reposDir) )
		{
		    if (updtFlag)
			dirEntry->totsize += (off_t) updateSpace;
		    dirEntry->totsize = (off_t) toblocks(dirEntry->totsize);
		}
		if ( !strcmp(dirEntry->Dir, secDir) )
		{
			/*---------------------------------------
			| Only update security dir if size > 0.	|
			---------------------------------------*/
			if (updtFlag && statBuf.st_size)
			    dirEntry->totsize += (off_t) updateSpace;
			dirEntry->totsize = (off_t) toblocks(dirEntry->totsize);
		}

  		if ( !strcmp(dirEntry->Dir, "/tmp") && bootFlag && !rFlag)
  		{
  			/*---------------------------------------
  			| Add to /tmp space for bootable lpps.  | 
  			---------------------------------------*/
  			dirEntry->tempsize += (off_t) toblocks(7500000);
  			tmpFlag++;
  		}
  		

		/*-----------------------------------------------
		| SAVESPACE is only used for updates.		|
		-----------------------------------------------*/
		if ( !strcmp (dirEntry->Dir, savespaceDir) && !updtFlag )
		    continue;
		if (dirEntry->inputOperator == '-')
		       {
			if (dirEntry->totsize == 0)
			   continue;
		        if ((dirEntry->totsize - dirEntry->insize) > 0) 
		           dirEntry->totsize -= dirEntry->insize;
                       }
		else if	(dirEntry->inputOperator == '+')
			dirEntry->totsize += dirEntry->insize;
		else if	(dirEntry->totsize < dirEntry->insize)
			dirEntry->totsize = dirEntry->insize;

		/* Now it is time to append this entry to the size file. */

		if (dirEntry->tempsize <= 0) 
		    fprintf(opsize_ptr, "%s %d\n", dirEntry->Dir,
			dirEntry->totsize);
		else 
		    fprintf(opsize_ptr, "%s %d %d\n", dirEntry->Dir,
			dirEntry->totsize, dirEntry->tempsize);
       }	/* Done looping through the "dirList". */
  
  	/*-------------------------------------------------------------------
  	| If this lpp is bootable, extra temporary space is needed in /tmp. |
  	| This was added above if /tmp was in dirList; if not, add it now.  |
  	-------------------------------------------------------------------*/
  	if (bootFlag && !tmpFlag && !rFlag)
  	        fprintf(opsize_ptr, "%s %d %d\n", 
			"/tmp", 0, (off_t) toblocks(7500000));
}

/*
 *********************************************************************
 * NAME: Calculate Size
 *                                                                    
 * DESCRIPTION: 
 *      Function to calculate size required by installp to
 *      process this file during install/update.
 *                                                                   
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      The dirList will be modified to contain a
 *      new or updated entry.
 *
 * PARAMETERS: 
 *      insentry - input; the inslist entry that is
 *                 currently being processed.
 *      filesize - input; the size of the file identified by 
 *      	   insentry.
 *      updtFlag - input; true if current build is an update
 *      	   build.
 *      userACF  - input; True if user supplied an ACF file

 * NOTES: 
 *      For regular files, size is the size of the file itself.
 *      
 *      For links, the length of the link name is calculated;
 *      if it is >48, then a block is added to the directory
 *      space.
 *      
 *      For library members, special processing is required.
 *      Library size calculations are passed on to another
 *      routine.  Only the detection of this condition is
 *      performed in this function.
 *      There are 2 types of library member updates.
 *      1) Install time updates:
 *         These are currently only used for compatability 
 *         (ie. X11R3); however, it could be used more in the
 *         future.  In this case, the user MUST supply an ACF
 *         (Archive Control File).
 *      2) Updates to "normal" libraries:
 *         This situation is detected by the updtFlag AND the
 *         file being processed containing the string "inst_updt"
 *         in its full path name.
 *
 * DATA STRUCTURES: 
 *      The dirList linked list of directory objects is
 *      updated with the size required by the file being
 *      processed.
 *
 * RETURNS: 
 *      nothing is returned.  All errors are worked around.
 *********************************************************************
 */
void
calculateSize (InsEntry	*insentry, 
	       off_t	filesize, 
	       int	updtFlag,
	       int	userACF)
{
    char 	DirName[MAXPATHLEN+1];
    ACFEntry	*acf = NULL ;		/* only used if user supplied an ACF */


    getDirectoryName (DirName, insentry->object_name, insentry->type);

    if ( !chktype(insentry) )
    {
       /* 
	*------------------------------------------------------------
	* If the user supplied an ACF, we must see if the current
	* inslist entry is in that ACF.  If it is, the size must
	* be calculated for a library update.
	* 
	* If no match is found in the ACF list, do "normal"
	* processing as described in function prolog.
	*------------------------------------------------------------
	*/
        if ((userACF) &&
	    (acf = acfFindMember(insentry->object_name)) != NULL)
	{
	    libUpdate(insentry, filesize, DirName, acf) ;
	}
	else if (updtFlag && strstr(insentry->object_name, "inst_updt"))
	{
	    libUpdate (insentry, filesize, DirName, acf) ;
	}
	else /* not a member, process as a "regular" file */
	{
	    updateSize (DirName, filesize, insentry->type, 0);
	    updateSize (savespaceDir, filesize, insentry->type, 0);
	}
    }

    if ( isaLink (insentry->type) )
	updateSize(DirName, strlen(insentry->object_name), insentry->type, 0);
}

void
updateSize(char *dirName, off_t fileSize, char type, off_t tempSize)
{
	off_t size=0, tmpBlocks=0;
	int found = 0;
	DirEntry	*dirEntry;

	/*---------------------------------------------------------------
	| If directory entry, use 8 (4K) as file size.	Don't convert	|
	| repository or security directory entries to blocks.  That	|
	| will be done when the entry is written to the size file.	|
	| That way we don't get an extra block for every link.		|
	---------------------------------------------------------------*/

	if ( (type == 'd') || (type == 'D') )
		size = 8;
	else
	{
		size = fileSize;
		if ( strcmp(dirName,secDir) && strcmp(dirName,reposDir) )
		{
			if ( size )
				size = (off_t) toblocks(size);
			if ( tempSize )
				tmpBlocks = (off_t) toblocks (tempSize);
		}
	}

	listRewind(dirList);
	while ((!listEnd(dirList)) && (! found))
	{
	    dirEntry = (DirEntry *)	listGet(dirList);

	    /* If we have a match, just add this file to the total size. */

	    if (!strcmp(dirEntry->Dir, dirName))
	    {
		/*-------------------------------------------------------
		| If this is a link then only add a block to the size	|
		| if the length of the link name > 48.			|
		-------------------------------------------------------*/
		if ( isaLink (type) || (type == 'H') )
		{
			if (fileSize > 48)
				size = 8;
			else
				return;
		}
		dirEntry->totsize += size;
		dirEntry->tempsize += tmpBlocks;
		++found;

	    }
	}

	if (! found) 
	{
	    /*-----------------------------------------------------------
	    | If this is a link then create an entry with a size	|
	    | of one block.						|
	    -----------------------------------------------------------*/
	    if ( isaLink (type) || (type == 'H') )
		size = 8;
	    dirEntry = getDirEnt();
	    strcpy(dirEntry->Dir, dirName);
	    dirEntry->totsize = size;
	    dirEntry->tempsize = tmpBlocks;
	    listAppend(dirList, dirEntry);
	}
}

DirEntry *
getDirEnt ()
{
	DirEntry *newDirEnt;

	newDirEnt = (DirEntry *) malloc( sizeof(DirEntry));
	newDirEnt->tempsize= 0;
	newDirEnt->insize = 0;
	newDirEnt->totsize = 0;
	newDirEnt->inputOperator = '\0';
	return (newDirEnt);
}

/*-----------------------------------------------------------------------
| Special updateSize function for /tmp for library updates.  Add 200    |
| blocks, if needed, to the size of the largest library in temp space.  |
-----------------------------------------------------------------------*/
void
updateTmpSize (off_t tempSize)
{
	off_t 		tmpBlocks;
	int   		found=0;
	DirEntry	*dirEntry;

	tmpBlocks= (off_t) toblocks (tempSize);
	listRewind(dirList);
	while ((!listEnd(dirList)) && (! found))
	{
	    dirEntry = (DirEntry *)	listGet(dirList);
	    if (!strcmp(dirEntry->Dir, "/tmp"))
	    {
		if (dirEntry->tempsize < (tmpBlocks + 200))
		    dirEntry->tempsize = (tmpBlocks + 200);
		found++;
	    }
	}
	
	if (! found)
	{
	    dirEntry = getDirEnt();
	    strcpy(dirEntry->Dir, "/tmp");
	    dirEntry->tempsize = (tmpBlocks + 200);
	    listAppend(dirList, dirEntry);
	}
}

/*
 *********************************************************************
 * NAME: Library Update (libUpdate)
 *                                                                    
 * DESCRIPTION: 
 *      Handles the special processing to determine the installp
 *      working space requirements for library (archive) updates.
 *                                                                   
 * PRE CONDITIONS: 
 *      Verification that the inslist entry is a library member
 *      that will be archived at install or update time has
 *      already been performed.
 *
 * POST CONDITIONS: 
 *      The dirList is modified.
 *
 * PARAMETERS: 
 *      finsentry - input; the inslist entry that is
 *                  currently being processed.
 *      memberSize- input; the size of the archive member 
 *      	    identified by finsentry.
 *      memberDirName- input; location of the member when installed
 *      	    on the machine before being put into the
 *      	    archive.
 *      acf       - input; pointer to acf entry for user supplied
 *      	    ACF file.  NULL if user is not supplying
 *      	    an ACF file, or the insentry was not found in
 *      	    that ACF file.
 * NOTES: 
 *      This function should only be called to handle either of
 *      the 2 library (archive) updates identified below.
 *      1) Install time updates:
 *         These are currently only used for compatability 
 *         (ie. X11R3); however, it could be used more in the
 *         future.  In this case, the user MUST supply an ACF
 *         (Archive Control File).  The acf entry parm identifies
 *         the member and its library.
 *      2) Updates to "normal" libraries:
 *         This situation is detected by the updtFlag AND the
 *         file being processed containing the string "inst_updt"
 *         in its full path name.  The commentary for "getLibInfo"
 *         describes the format of this path name.
 *         
 *      In either case, the size/space calculations are the
 *      same:
 *        /tmp -> the size of the largest library (archive) being
 *                updated + 200 blocks for installp processing.
 *        member directory -> 
 *        	  (specified for case 1 above, the inst_updt 
 *        	  directory for case 2); the sum of all updated
 *        	  members in temporary space.
 *        library directory-> 
 *        	  sum of all library sizes in permanent space.
 *                PLUS; the size of the lib + size of its members
 *        SAVESPACE ->
 *        	  size of all members ONLY.
 *
 * DATA STRUCTURES: 
 *      The dirList linked list of directory objects is
 *      updated with the size required by the file being
 *      processed.
 *
 * RETURNS: 
 *      ADE_SUCCESS - all processing completed successfully.
 *      ADE_ERROR   - The only error that can occur is that
 *      	  the library cannot be found in the ship trees.
 *********************************************************************
 */
static int
libUpdate (InsEntry 	*finsentry, 
	   off_t 	memberSize, 
	   char 	*memberDirName,
	   ACFEntry	*acf)
{
    int		rc = ADE_SUCCESS ;	/* Assume success for rtn code	*/
    int		i ;
    char	libName[MAXPATHLEN+1];
    char	libDirName[MAXPATHLEN+1];
    off_t	libSize = 0;
    int	totalLibSpace = 0;
    DirEntry	*dirEntry;
    ACFEntry	*acfEntry ;


    if (acf == NULL)
    {
       /* 
	*-----------------------------------------------------
	* Get the necessary info for archive updates, and
	* create/update the update ACF.
	*-----------------------------------------------------
	*/
	strcpy(libName, finsentry->object_name) ;
	if ((libSize = getLibInfo(libName, finsentry->type)) == -1)
	{
	   /* 
	    *----------------------------------------------------------
	    * Tell user that lib wasn't found; but continue
	    * with 0 for the lib size.
	    *----------------------------------------------------------
	    */
	    warning(msgArray[LibNotFound], libName, acfEntry->member) ;
	    libSize = 0 ;
	    rc = ADE_ERROR ;
	}
	getDirectoryName(libDirName, libName, finsentry->type) ;
	acfEntry = acfAdd(finsentry->object_name, libName) ;
    }
    else
    {
       /* 
	*-----------------------------------------------------
	* Get the necessary info from the user supplied acf.
        * NOTE that we must skip over the '.' at the 
        *      beginning of the library directory name
        *      (if it is present).  No '.' may be present
        *      in the size file which gets its entry name
        *      from this variable.
	*-----------------------------------------------------
	*/
	acfEntry = acf ;
	strcpy(libName, acf->lib) ;
	getDirectoryName(libDirName, libName, finsentry->type) ;
        if (libDirName[0] == '.')
        {
            strcpy(libDirName, &libDirName[1]) ;
        }
	if ((libSize = getLibSize(libName)) == -1)
	{
	   /* 
	    *----------------------------------------------------------
	    * Tell user that lib wasn't found; but continue
	    * with 0 for the lib size.
	    *----------------------------------------------------------
	    */
	    warning(msgArray[LibNotFound], libName, acfEntry->member) ;
	    libSize = 0 ;
	    rc = ADE_ERROR ;
	}
    }
    acfEntry->built = TRUE ;			/* indicate member is built */

    /*-----------------------------------------------
     | Update /tmp with size of largest library.
     -----------------------------------------------*/
    updateTmpSize ( libSize );

    /*-----------------------------------------------
     | Update member directory.
     -----------------------------------------------*/
    updateSize ( memberDirName, 0, NULL, memberSize );
    updateSize ( savespaceDir, memberSize, NULL, 0 );

    /*---------------------------------------------------------------
     | Update hash table for this library.  If this library is not
     | in the hash table updateHashEntry will return zero.
     | Otherwise return the accumulated size requirements for this
     | library which includes the space for the library itself plus
     | all of its updated members.
     ---------------------------------------------------------------*/
    totalLibSpace = updateHashEntry(libName, memberSize, libSize);

    listRewind(dirList);
    while ( !listEnd(dirList) )
    {
	dirEntry = (DirEntry *)     listGet(dirList);

	if ( strcmp(dirEntry->Dir, libDirName) )
	    continue;

	/*-----------------------------------------------------------
	 | If totalLibSpace is 0 this library is new.  Add space for
	 | it in the directory entry.  For a new library the
	 | tempspace is the libSize + memberSize.
	 -----------------------------------------------------------*/
	if ( ! totalLibSpace )
	{
	    dirEntry->totsize += toblocks (libSize);
	    totalLibSpace = libSize+memberSize;
	}
	totalLibSpace = toblocks (totalLibSpace);

	/*-----------------------------------------------------------
	 | If the library directory has an entry in dirList then
	 | replace temp space if totalLibSpace  is now > the temp
	 | space value that was already there.
	 -----------------------------------------------------------*/
	if ( dirEntry->tempsize < totalLibSpace )
	    dirEntry->tempsize = totalLibSpace;
	return(rc);
    }

    /*---------------------------------------------------------------
     | If we got here there was no entry for this lib directory.
     | Call updateSize to create one.
     ---------------------------------------------------------------*/
    updateSize (libDirName, libSize, NULL, libSize+memberSize);
    return(rc) ;
} /* END libUpdate */

/*-----------------------------------------------------------------------
| Given a member name extract the library name.  Find the library in	|
| the ship trees and return its size.					|
-----------------------------------------------------------------------*/

off_t
getLibInfo ( char *libName , char fitype)
{
    char		*ptr, *tmpptr, *fptr;


    /*-------------------------------------------------------
     | There are two cases to be considered:
     | In either case we want to just search for the library
     | name in the ship tree.
     | 1. Upper case entries.
     | Strip out inst_updt from the path and the member name
     | from the end to get the library name.
     | 2.Lower case entry.
     | /usr/lpp/<lppname>/<vrmf_level>/inst_root will
     | preceed the object name.
     | So strip every thing before the object name.
     | Also strip inst_updt and the member name from the
     | end to get the library name.
     -------------------------------------------------------*/
    if(isupper((int)fitype))
    {
	ptr = strrchr (libName, '/');
	*ptr = '\0';
	tmpptr = strstr (libName, "/inst_updt");
	*tmpptr = '\0';
	tmpptr += strlen("/inst_updt");
	strcat (libName, tmpptr);
    }
    else
    {
	ptr = strrchr (libName, '/');
	*ptr = '\0';
	tmpptr = strstr (libName, "/inst_updt");
	*tmpptr = '\0';
	tmpptr += strlen("/inst_updt");
	strcat (libName, tmpptr);
	fptr = strstr(libName, "/inst_root");
	fptr += strlen("/inst_root");
	strcpy (libName, fptr);
    }

    /*
     *---------------------------------------------------------
     * Now get and return the size of the lib.
     *---------------------------------------------------------
     */
    return(getLibSize(libName)) ;
}

/*
 *********************************************************************
 * NAME: Get Library Size (getLibSize)
 *                                                                    
 * DESCRIPTION: 
 *      Gets the size of the specified library.
 *                                                                   
 * PRE CONDITIONS: 
 *      none.
 * 
 * POST CONDITIONS: 
 *      none.
 * 
 * PARAMETERS: 
 *      libName - input; the full path of the library to be
 *      	  updated, used to find the lib in the 
 *      	  ship trees.
 * NOTES:
 *      Uses "findfile" to get the lib (and its size) from
 *      the ship trees.  Missing files are logged and a
 *      zero size returned.
 *
 * DATA STRUCTURES: 
 *      shipPaths - Passed in on the via the -s flag.  It is
 *                  passed to findfile.
 *      num_paths - Nmbr of paths in list of hipPaths.  It is
 *                  passed to findfile.
 * RETURNS: 
 *      Size of the library in the ship trees.
 *	-1 if the library is not found.
 *********************************************************************
 */
static off_t
getLibSize(char	*libName)
{
    Fileinfo	fileInfo ;	/* Ship file info structure		*/
    char	ckSum[512] ;	/* check sum of lib (rtn fr findfile)	*/


    fileInfo.file_fd = 0;
    strcpy(fileInfo.filename, libName);
    
    if (findfile(fileInfo.filename, shipPaths, num_paths,
		 fileInfo.ship_name, &fileInfo.f_st.st, ckSum) != 0)
    {
	warning(msgArray[FileNotFound], fileInfo.filename) ;
	return (-1);
    }
    return (fileInfo.f_st.st.st_size);
} /* END getLibSize */

/*-----------------------------------------------------------------------
| Look for a library in the hash table.  If it is there, add the	|
| memberSize to the current total and return the value.  Otherwise,	|
| add the entry.  When you first create the entry the original library	|
| size should be included in the size value.  Returns 0 if the entry	|
| is new.								|
-----------------------------------------------------------------------*/

off_t
updateHashEntry ( char *libName, off_t memberSize, off_t libSize )
{
	LibEntry	*entryPtr;
	int		hashval;
	
	hashval = computeHashVal (libName);
	entryPtr = libTable[hashval];

	while ( entryPtr )
	{
		if ( !strcmp (entryPtr->libname, libName) )
		{
			entryPtr->libEntrySize += memberSize;
			return (entryPtr->libEntrySize);
		}
		entryPtr = entryPtr->nextLib;
	}

	addToHashTable (hashval, libName, memberSize, libSize);
	return (0);
}

/*-----------------------------------------------------------------------
| Add a library and its initial size requirements to the hash table.	|
-----------------------------------------------------------------------*/

void
addToHashTable (int hashval, char *libName, off_t memberSize, off_t libSize)
{
	LibEntry	*entryPtr, *nextEntry;

	entryPtr = (LibEntry *) malloc (sizeof (LibEntry));
	strcpy (entryPtr->libname, libName);
	entryPtr->libEntrySize = memberSize + libSize;
	entryPtr->nextLib = NULL;

	if ( ! (nextEntry = libTable[hashval]) )
		libTable[hashval] = entryPtr;
	else
	{
		while ( nextEntry->nextLib )
			nextEntry = nextEntry->nextLib;
		nextEntry->nextLib = entryPtr;
	}
}

/*-----------------------------------------------------------------------
| Initialize library hash table.  The table is used to track the amount	|
| of storage required for a library update.  A library update needs	|
| the size of the library itself plus the size of all members to be	|
| updated.								|
-----------------------------------------------------------------------*/

void
initHashTable()
{
	int	i;

	for (i=0; i<LIBHASHTABLE; i++)
		libTable[i] = NULL;
}

/*---------------------------------------------------------------
| Hash value is the integer sum of the characters in libname	|
| mod LIBHASHTABLE.						|
---------------------------------------------------------------*/

computeHashVal ( char *libname )
{
        int i, hashVal=0;

        for (i=0; libname[i]; i++)
                hashVal += libname[i];

        hashVal %= LIBHASHTABLE;
        return (hashVal);
}

/*-----------------------------------------------------------------------
| For updates the repository directory needs to add space for the	|
| following:								|
|	size of prereq file						|
|	size of fixinfo file						|
|	# of superseded ptfs from supersede file * sizeof (prod_t)	|
|	sizeof (hist_t)							|
|	sizeof (prod_t)							|
|	size of (lpp_t)							|
-----------------------------------------------------------------------*/

adjustReposDir (char *lppOption)
{
	char		prereqFile[MAXPATHLEN], supersedeFile[MAXPATHLEN];
	char		buf[MAXBUFFER];
	int		updateSpace = 0;
	struct stat	statBuf;
	FILE		*superFp;
	
	strcpy (prereqFile, lppOption);
	strcpy (supersedeFile, lppOption);
	strcat (prereqFile, ".prereq");
	strcat (supersedeFile, ".supersede");

	if ( stat (prereqFile, &statBuf) != -1 )
	    updateSpace += statBuf.st_size;

	if ( stat ("./fixinfo", &statBuf) != -1 )
	    updateSpace += statBuf.st_size;

	/*---------------------------------------------------------------
	| Each line contains 1 supersede ptf.  This is an automatically	|
	| generated file so it should not have any comments or blank	|
	| lines in it...						|
	---------------------------------------------------------------*/
	if ( (superFp = fopen (supersedeFile, "r")) != NULL )
	{
	    while ( fgets(buf, MAXBUFFER, superFp) != NULL )
		updateSpace += sizeof (prod_t);
	}

	updateSpace += sizeof (hist_t);
	updateSpace += sizeof (prod_t);
	updateSpace += sizeof (lpp_t);
	return (updateSpace);
}
