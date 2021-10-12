static char sccsid[] = "@(#)96	1.3  src/bos/usr/ccs/lib/libcfg/findmcode.c, libcfg, bos411, 9428A410j 9/19/91 15:00:39";
/*
 *
 * COMPONENT_NAME: (LIBCFG) FINDMCODE.C
 *
 * FUNCTIONS : 	findmcode
 * 
 * ORIGINS : 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * Unpublished Work
 * All Rights Reserved
 *
 * RESTRICTED RIGHTS LEGEND
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <dirent.h>
#include <cf.h>

/*
 * NAME : findmcode
 * 
 * FUNCTION :
 *	Finds the latest version of the specified microcode by searching
 *	through the directories indicated in the MCPATH environment
 *      variable.  Returns full pathname of the microcode if found,
 *      NULL if not.
 *
 * EXECUTION ENVIRONMENT:
 *	
 *
 * INPUT  : mfile    : base name of the microcode (cardid.LL if 
 * 		           VERSIONING is set in flag parm)
 *          mpath    : pointer to space to put fully qualified
 *                         microcode name.
 *          flag     : flag to control the type of search for the
 *          		   microcode.  Possible values are :
 *                         VERSIONING : look for microcode with the
 *                                      highest version number.
 *                         ABSOLUTE   : look only for a pure match
 *                         		of the name.
 *                         BASENAME   : look for a match of basename
 *                         		only (equivalent to a search
 *                         		for basename*).
 *          prompt   : microcode diskette name to prompt for if 
 *          		   the microcode is NOT found in the 
 *          		   directories - NOT CURRENTLY SUPPORTED!!!
 *
 * RETURNS: FALSE : could not find microcode file.
 *          TRUE  : Found microcode file & name is in space pointed
 *                    to by "file" input parameter.
 *
 * NOTE:
 *	The format for microcode file names is:
 *		cardid.LL.VV
 *	cardid is the hex id obtained from POS registers 0 and 1
 *	LL is revision level of the adapter from the LL field of
 *	   the VPD
 *	VV is version number of the microcode
 */

int
findmcode(mfile, mpath, flag, prompt)
    char	*mfile ;     	/* base mcode name (NOT including '.'       */
    char        *mpath ;      	/* ptr to space to put mcode file name.     */
    int		flag   ;     	/* flag defining type of search to do.      */
    char        *prompt;	/* NOT supported!			    */
{
#define SEPCHARS ":"

    struct dirent *fp;		/* ptr to dir entry being checked.	    */
    DIR		*dp;		/* ptr to open dir being searched.          */
    char	*dirlist ;      /* ptr to list of directories to search.    */
    char	*curdir  ;      /* ptr to name of current dir being searched*/
    int		len;		/* len of basename for initial file compare */
    int		myrc ;		/* save area for return code to be returned.*/
    int		rc;	
    ulong	cur_vers ;	/* unsigned long value of current version 
				   number  				    */
    ulong       tmp_vers ;      /* work space for converted version value
				   for comparison with "cur_vers".	    */
    char 	*tmp_str ; 	/* temp pointer used in string searches.    */ 
    char	curfile[256];   /* Current mcode filename being looked for. */
    char	defaultdirs[] = "/etc/microcode:/usr/lib/microcode" ;

/* BEGIN findmcode */

    myrc = FALSE ;
    cur_vers = 0  ;
   /* 
    *  Strip off the pathname if it is present and 
    *  setup search based on input flag value.	
    */
    if ((tmp_str = strrchr(mfile, '/')) != (char *)NULL)
	mfile = tmp_str + 1 ;

    strcpy(curfile, mfile)  ;
    if (flag == VERSIONING)
    {
	strcat(curfile,".");
    }
    len = strlen(curfile) ;

   /* 
    *  Set up the directory list.  Try using MCPATH.  If it is not
    *  found, use the default list.
    */
    if ((dirlist = getenv("MCPATH")) == (char *)NULL)
    {
	dirlist = defaultdirs ;
    }
   /*
    *  Got the dir list, now loop through the different
    *  directories looking for the specified mcode.
    */

    curdir = strtok(dirlist, SEPCHARS) ;
    while (curdir != (char *)NULL)
    {
	if ((dp = opendir(curdir)) != (DIR *)NULL) 
	{
	   /* 
	    * look through the directory entries and try to find 
      	    * the latest version microcode for this adapter level.
	    */
	    while ((fp = readdir(dp)) != (struct dirent *)NULL) 
	    {
		if (!strncmp(curfile,fp->d_name,len)) 
		{
		   /* 
		    * Got at least a partial match.  The rest depends
		    * on the type of search requested in the FLAG parm
		    */
		    switch (flag)
		    {
		      case VERSIONING:
		       /* 
			* Use an arithmatic comparison of versions
			* to support variable length version fields.
			* So, first must convert version number of the
			* directory's filename to ulong.  If conversion
			* cannot be done, no comparison is made.
			*/
			if ((tmp_str = strrchr(fp->d_name, '.')) != 
			    (char *)NULL)
			{
			    tmp_str++ ;
			    tmp_vers = strtoul(tmp_str, (char **)NULL, 16) ;
			    if ((tmp_vers > cur_vers) ||
				((tmp_vers == cur_vers) && (!myrc)))
			    {
			       /* 
				* Found a newer (or same) version of
				* microcode.  Set up return information.
				* 
				* NOTE the check for (!myrc) ensures that
				*      the first occurance of an exact 
				*      match is returned for VERSIONING
				*      as well as the other search types.
				*/
				sprintf(mpath, "%s/%s", curdir, fp->d_name) ;
				cur_vers = tmp_vers ;
				myrc = TRUE ;
			    }
			}
			break ;
		      case BASENAME:
			sprintf(mpath, "%s/%s", curdir, fp->d_name) ;
			myrc = TRUE ;
		        break ;
		      case ABSOLUTE:
		      default:		/* default unknown flags to ABSOLUTE */
		       /* 
			* For ABSOLUTE, the entire names must match,
			* so redo the comparison without length 
			* restrictions.
			*/
			if (!strcmp(curfile, fp->d_name))
			{
			    sprintf(mpath, "%s/%s", curdir, fp->d_name) ;
			    myrc = TRUE ;
			}
			break ;
		    } /* end switch on FLAG */
		}
		if ((myrc) && (flag != VERSIONING))
		{
		    /* we are done, since not checking for highest version */
		    break ;
		}
	    } /* end while loop thru dir entries */
	    closedir(dp);
	}
	if ((myrc) && (flag != VERSIONING))
	{
	    /* we are done, since not checking for highest version */
	    break ;
	}
	curdir = strtok((char *)NULL, SEPCHARS) ;
    }  /* end while loop thru directories */
    return(myrc);
} /* END findmcode */
