/* static char sccsid[] = "@(#)31  1.2  src/bldenv/pkgtools/acfList.h, pkgtools, bos412, GOLDA411a 5/25/94 00:54:47" */
#ifndef _H_ACFLIST
#define _H_ACFLIST
/*
 *   COMPONENT_NAME: PKGTOOLS
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
 * MODULE  : ACF List Header File
 * PURPOSE : To define and provide the external interface to
 *           ACF List facility implemented in the acfList.c 
 *           unit.
 ********************************************************************* 
 */

/* 
 *--------------------------------------------------------------------
 *  The ACFEntry structure is the internal structure used to
 *  represent an ACF entry in the lpp.acf file used by installp
 *  to modify an existing library on an installed system.
 *  
 *  The ACF entry must identify the full path of the member to
 *  be modified in the library AND the full path of the library
 *  to be modified.  These paths are the paths to the files when
 *  they are installed on the target machine.
 *  
 *  The 'built' flag indicates whether the member identified is 
 *  being built during this build.  TRUE indicates that the file
 *  is being built (either updated or initially built).  The 
 *  lpp.acf file is only generated during update builds.  During
 *  these builds, only entries for members that are
 *  actually being built (updated) will be written to the 
 *  lpp.acf file.
 *--------------------------------------------------------------------
 */
typedef struct ACFentry
{
    int		built	;	/* TRUE if this entry is constructed	*/
    char	*member ;	/* Ptr to member path string		*/
    char	*lib    ;	/* Ptr to library path string		*/
} ACFEntry ;

/*
 *--------------------------------------------------------------------
 * Function prototypes for acfList interface routines
 *--------------------------------------------------------------------
 */
void	 acfInit() ;		        /* initialize the list stuff.	*/
int	 acfLoadFile(char *fName) ;	/* load acf entries from file	*/
ACFEntry *acfAdd(char *member,		/* create entry w/ given info	*/
		 char *lib) ;
ACFEntry *acfFindMember(char *member) ;	/* search list for member	*/
ACFEntry *acfFindlib(char *lib) ;	/* search list for lib		*/
ACFEntry *acfGetFirst() ;		/* get 1st entry in list	*/
ACFEntry *acfGetNext() ;		/* Get next entry in list	*/
void	 acfDelete(ACFEntry *acf) ;	/* destroy given ACF entry	*/
int	 acfWriteACF(int BuiltOnly) ;	/* create lpp.acf file		*/

#endif /* _H_ACFLIST */
