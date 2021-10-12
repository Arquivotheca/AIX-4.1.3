static char sccsid[] = "@(#)19  1.1  src/bldenv/pkgtools/acfList.c, pkgtools, bos412, GOLDA411a 5/25/94 00:54:02";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: acfInit
 *		acfLoadFile
 *		acfAdd
 *		acfFindMember
 *		acfFindLib
 *              acfWriteACF
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
 * MODULE  : ACF List
 * PURPOSE : To manage the list of ACF entries created or read
 *           from a user supplied ACF.  This module hides all of
 *           the structures and necessary logic to manage the 
 *           list of ACFs.
 * NOTES   :
 *    - this module must be initialized via the acfInit function
 *      before it can be used.
 *    - A complete user supplied ACF can be loaded via the acfLoad
 *      function.
 *    - acfWriteACF will generate the lpp.acf file required by
 *      installp.
 ********************************************************************* 
 */

#include <stdlib.h>
#include <malloc.h>
#include <strings.h>
#include <stdio.h>
#include "ade.h"
#include "adeinventory.h"
#include "acfList.h"

/* 
 *-------------------------------------------------------------------
 * Unit level variables.  Note that this variables are 
 * accessable only from with in this unit.
 *-------------------------------------------------------------------
 */
static List	acfList ;	/* List control block for the ACF list	*/

/*
 *-------------------------------------------------------------------
 *  Local function definitions
 *-------------------------------------------------------------------
 */


/*
 *********************************************************************
 * NAME: acfInit
 *                                                                    
 * DESCRIPTION: 
 *      Initializes the static unit level variables needed to 
 *      manage the ACF list.
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      The list control block is initialized and is empty.
 *
 * PARAMETERS: 
 *      none.
 *
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      nothing (cannot fail).
 *********************************************************************
 */
void
acfInit()
{
    listInit(&acfList) ;
}

/*
 *********************************************************************
 * NAME: acfLoadFile
 *                                                                    
 * DESCRIPTION: 
 *      "Loads" the entries from the specifed ACF file into
 *      the list.
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      The acfList contains elements for each of the entries
 *      from the specified file.
 *
 * PARAMETERS: 
 *      fName - full path of the acf file to load.
 *
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      ADE_SUCCESS = successfully loaded all entries from the file
 *      ADE_FAILURE = could not load the entries.  Error messages
 *                are generated detailing the error.
 *********************************************************************
 */
int
acfLoadFile(char *fName)
{
    int		i ;
    int		rc = ADE_SUCCESS ;	/* Rtn Code - initially success	*/
    char	line[MAXBUFFER + 1] ;	/* space for line from acf file	*/
    FILE	*fp ;			/* file ptr for acf file	*/
    char	*member ;		/* ptr to member in input line	*/
    char	*lib ;			/* ptr to library in input line	*/


    if ((fp = fopen(fName, "r")) != NULL)
    {
       /* 
	*-----------------------------------------------------
	* Got the ACF, so load the entries.  Skip blank
	* lines and comment lines (lines beginning with #).
	*-----------------------------------------------------
	*/
	while (fgets(line, MAXBUFFER, fp) != NULL)
	{
	    for (i = 0; isspace(line[i]); i++) ; /* skip leading blanks	*/

	    if ((line[i] != '\n') && (line[i] != '#'))
	    {
	       /* 
		*----------------------------------------------------
		* Member is 1st on line followed by white space.
		* Lib is next; the \n in its token list chops
		* off the new line.
		*----------------------------------------------------
		*/
		member = strtok(line, " \t") ;
		lib    = strtok((char*)NULL, " \t\n") ;
		if (acfAdd(member, lib) == (ACFEntry*)NULL)
		{
		    rc = ADE_ERROR ;
		}
	    }
	}
	fclose(fp) ;
    }
    else /* Could not open ACF file */
    {
	rc = ADE_ERROR ;
    }
    return(rc) ;
}

/*
 *********************************************************************
 * NAME: acfAdd
 *                                                                    
 * DESCRIPTION: 
 *      Creates and adds an ACF element to the acfList.  
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      New element created (malloc'ed), initialized, and added
 *      to the acfList.
 *
 * PARAMETERS: 
 *      member - input; string containing the full path of the member.
 *      lib    - input; string containing the full path of the library.
 *
 * NOTES: 
 *      - Input parameters may be null.
 *      - xmalloc will fatally exit if it cannot malloc the space.
 *
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      Address of the acfEntry if successful.
 *********************************************************************
 */
ACFEntry*
acfAdd(char	*member,
       char	*lib)
{
    ACFEntry	*acfEntry = NULL ;	/* Ptr to new ACF Entry		*/


    if (acfFindMember(member) == NULL)
    {
	acfEntry = (ACFEntry*)xmalloc(sizeof(ACFEntry)) ;

	if (member[0] != '.')
	{
	    acfEntry->member = (char*)xmalloc(strlen(member) + 2) ;
	    sprintf(acfEntry->member, ".%s", member) ;    
	}
	else
	{
	    acfEntry->member = (char*)xmalloc(strlen(member) + 1) ;
	    strcpy(acfEntry->member, member) ;
	}

	if (lib[0] != '.')
	{
	    acfEntry->lib = (char*)xmalloc(strlen(lib) + 2) ;
	    sprintf(acfEntry->lib, ".%s", lib) ;    
	}
	else
	{
	    acfEntry->lib = (char*)xmalloc(strlen(lib) + 1) ;
	    strcpy(acfEntry->lib, lib) ;
	}

	acfEntry->built = FALSE ;
	listInsert(&acfList, acfEntry) ;
    }
    return(acfEntry) ;
}

/*
 *********************************************************************
 * NAME: acfFindMember
 *                                                                    
 * DESCRIPTION: 
 *      Searches the ACF List for an entry with the specified
 *      member in its member string.
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      Current index into the ACF list points to the found entry.
 *
 * PARAMETERS: 
 *      Member	- input; exact string to find in the member field.
 *
 * NOTES: 
 *    - All members in the acf Entry list should be preceeded with
 *      a dot (.).  This is ensured by the acfAdd routine.  Since we
 *      cannot be sure that the input member to find starts with a
 *      dot, we must adjust our strcmp accordingly.
 *
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      Address of the ACF entry containing the specified member,
 *	    if found.
 *      NULL if the member is not found.
 *********************************************************************
 */
ACFEntry*
acfFindMember(char	*member)
{
    ACFEntry	*acfEntry = NULL ;	/* Last entry rtn from list	*/
    int		found = FALSE ;		/* Start out as not found	*/


    listRewind(&acfList);
    while ((!listEnd(&acfList)) && (!found))
    {
	acfEntry = (ACFEntry*)listGet(&acfList);
	if (member[0] != '.')
	{
	    found = strcmp(&acfEntry->member[1], member) == 0 ? TRUE : FALSE ;
	}
	else
	{	
	    found = strcmp(acfEntry->member, member) == 0 ? TRUE : FALSE ;
	}
    }

    if (found == FALSE)
    {
	acfEntry = NULL ;
    }
    return(acfEntry) ;
}

/*
 *********************************************************************
 * NAME: acfFindLibrary
 *                                                                    
 * DESCRIPTION: 
 *      Searches the ACF List for an entry with the specified
 *      member in its library string.
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      Current index into the ACF list points to the found entry.
 *
 * PARAMETERS: 
 *      Member	- input; string to find in the library field.
 *
 * NOTES: 
 *    - All members in the acf Entry list should be preceeded with
 *      a dot (.).  This is ensured by the acfAdd routine.  Since we
 *      cannot be sure that the input member to find starts with a
 *      dot, we must adjust our strcmp accordingly.
 *
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      Address of the ACF entry containing the specified library,
 *	    if found.
 *      NULL if the library is not found.
 *********************************************************************
 */
ACFEntry*
acfFindLib(char	*lib)
{
    ACFEntry	*acfEntry = NULL ;	/* Last entry rtn from list	*/
    int		found = FALSE ;		/* Start out as not found	*/


    listRewind(&acfList);
    while ((!listEnd(&acfList)) && (!found))
    {
	acfEntry = (ACFEntry*)listGet(&acfList);
	if (lib[0] != '.')
	{
	    found = strcmp(&acfEntry->lib[1], lib) == 0 ? TRUE : FALSE ;
	}
	else
	{	
	    found = strcmp(acfEntry->lib, lib) == 0 ? TRUE : FALSE ;
	}
    }

    if (found == FALSE)
    {
	acfEntry = NULL ;
    }
    return(acfEntry) ;
}
#ifdef GET_ROUTINES

/*
 *********************************************************************
 * NAME: acfGetFirst
 *                                                                    
 * DESCRIPTION: 
 *      Returns the ACF entry at the beginning of the list.
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      The current index into the ACF list points to the first
 *      entry in the list.
 *
 * PARAMETERS: 
 *      none.
 *
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      Address of the ACF entry at the head of the list if one exists.
 *      NULL if the list is empty, or an error occurs.
 *********************************************************************
 */
ACFEntry*
acfGetFirst()
{
    ACFEntry	*acfEntry = NULL ;


    listRewind(&acfList) ;
    if (!listEnd(&acfList))
    {	
	acfEntry = (ACFEntry*)listGet(&acfList) ;
    }
    return(acfEntry) ;
}

/*
 *********************************************************************
 * NAME: acfGetNext
 *                                                                    
 * DESCRIPTION: 
 *      Returns the ACF Entry that is next in the list.
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      The current index into the ACF list is modified.
 *
 * PARAMETERS: 
 *      none.
 *
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      Address of the ACF entry at the head of the list if one exists.
 *      NULL if the list is empty, there are no more elements in the 
 *         list, or an error occurs.
 *********************************************************************
 */
ACFEntry*
acfGetNext()
{
    ACFEntry	*acfEntry = NULL ;


    if (!listEnd(&acfList))
    {	
	acfEntry = (ACFEntry*)listGet(&acfList) ;
    }
    return(acfEntry) ;
}

/*
 *********************************************************************
 * NAME: acfDelete
 *                                                                    
 * DESCRIPTION: 
 *      Removes the specified ACF Entry from the list and frees
 *      the space associated with it.
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      Current index into the ACF list points to the entry following
 *      the one deleted.  The List may be empty after this operation.
 *
 * PARAMETERS: 
 *      acf	- pointer to the ACF entry to be removed.
 *
 * NOTES: 
 *      - space for the ACF entry AND the link control block is freed.
 * 
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      nothing; ignores case of Entry not in list.
 *********************************************************************
 */
void
acfDelete(ACFEntry *acf)
{
}
#endif /* GET_ROUTINES */

/*
 *********************************************************************
 * NAME: acfWriteACF
 *                                                                    
 * DESCRIPTION: 
 *      Loops through the ACF list and writes each entry to the
 *      "lpp.acf" file.
 *                                                                    
 * PRE CONDITIONS: 
 *      none.
 *
 * POST CONDITIONS: 
 *      Only affects unit level data.
 *      "lpp.acf" file is created and contains entries.
 *
 * PARAMETERS: 
 *      builtOnly - input; flag indicating (when TRUE) that only 
 *      	     entries with modified files are to be written.
 *
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *      No global data is affected.  Unit level data affected
 *	is described in POST CONDITIONS above.
 *
 * RETURNS: 
 *      SUCCESS if the lpp.acf file is successfully created/written.
 *      FAILURE if the lpp.acf file cannot be created/written.
 *********************************************************************
 */
int
acfWriteACF(int	builtOnly)
{
    ACFEntry	*acfEntry ;
    FILE	*fp	  ;
    int		rc = ADE_SUCCESS ;	/* Rtn Code; initially SUCCESS	*/
    

    listRewind(&acfList) ;
    if (!listEnd(&acfList))
    {
	if (NULL != (fp = fopen("./lpp.acf", "w")))
	{
	    while (!listEnd(&acfList))
	    {
		acfEntry = (ACFEntry*)listGet(&acfList) ;
	       /* 
		*-----------------------------------------------------
		* Always generate acf entry if builtOnly is FALSE,
		* otherwise acfEntry's built flag must also be TRUE.
		*-----------------------------------------------------
		*/
		if (!builtOnly || acfEntry->built)
		{
		    fprintf(fp, "%s %s\n", acfEntry->member, acfEntry->lib) ;
		}
	    }
	    fclose(fp) ;
	}
	else /* couldn't create 'lpp.acf' file */
	{
	    rc = ADE_ERROR ;
	}
    }
    return(rc) ;
}
