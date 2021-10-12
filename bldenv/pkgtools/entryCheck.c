static char sccsid[] = "@(#)02  1.5  src/bldenv/pkgtools/entryCheck.c, pkgtools, bos412, GOLDA411a 8/30/94 15:11:49";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: crossFileSys
 *		getFileSys
 *		idTable
 *		writeDb
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

#include <grp.h>
#include <pwd.h>
#include <sys/limits.h>
#include <sys/mode.h>
#include "ade.h"
#include "ins.h"

/*-----------------------------------------------------------------------
| Verify the uid and gid against the user provided table, if provided.	|
| If none was provided or the current uid and/or gid are not found,	|
| verify against the system uid and gid information.			|
-----------------------------------------------------------------------*/

idTable (InsEntry *inslist, FILE *id_ptr)
{
    static struct passwd *pass_ptr;
    static struct group *group_ptr;
    struct Table_Nam *tab_ptr;
    int idnum, groupnum;
    char idName[IDSIZE], groupName[IDSIZE];
    char idLine[ADE_BUFSIZE];
    int rc = 0;
    int uidFound=0, gidFound=0;

    if ( id_ptr )
    {
	rewind (id_ptr);
	/*-------------------------------------------------------
	| Look for the uid/gid in the user provided table file.	|
	-------------------------------------------------------*/

	tab_ptr = (struct Table_Nam *) malloc (sizeof (struct Table_Nam));
	while ( stripComments(id_ptr, idLine) != EOF )
	{
	    if (sscanf(idLine,"%d %s %d %s",&idnum,idName,&groupnum,groupName) != 4)
	    {
		inserror (invalidTableEnt, idLine);
		continue;
	    }
	    if ( idnum == inslist->uid )
		uidFound++;

	    if ( groupnum == inslist->gid )
		gidFound++;

	    if ( uidFound && gidFound )
		break;
	}

    }

    /*----------------------------------------
    | Try to validate using the system files |
    ----------------------------------------*/
    if (  !uidFound && (pass_ptr = getpwuid(inslist->uid)) == NULL )
    {
	inserror (uidNotFound, inslist->uid, inslist->object_name);
	rc = INS_ERROR;
    }
    else
    {
	if ( !uidFound )
	    strcpy(idName,pass_ptr->pw_name);
    }

    if ( !gidFound && (group_ptr = getgrgid(inslist->gid)) == NULL )
    {
	inserror (gidNotFound, inslist->gid, inslist->object_name);
	rc = INS_ERROR;
    }
    else
    {
	if ( !gidFound )
	    strcpy(groupName,group_ptr->gr_name);
    }

    /*----------------------------------------------------------
    | check tcb flag is set for set-uid root or set-gid system |
    ----------------------------------------------------------*/
    if ( (inslist->mode & S_ISUID) && (!strcmp (idName, "root")) &&
         (!inslist->tcbflag) )
    {
        warning (tcbFlagNotSetRoot, inslist->object_name);
    }
    if ( (inslist->mode & S_ISGID) && (!strcmp (groupName, "system")) &&
         (!inslist->tcbflag) )
    {
        warning (tcbFlagNotSetSys, inslist->object_name);
    }

    return rc;
}

/*-----------------------------------------------------------------------
| Check that hard links do not cross base file systems.  The base	|
| file systems are:  /, /tmp, /home, /var, /usr.			|
-----------------------------------------------------------------------*/

crossFileSys ( InsEntry *inslist )
{
    int i, rc = 0;
    char *objectFileSys;
    char *linkFileSys;

    /*-----------------------------------------------------------
    | Get base filesystem for the current inslist entry.	|
    -----------------------------------------------------------*/
    objectFileSys = getFileSys (inslist->object_name);

    for ( i=0; i<inslist->numHardLinks; i++ )
    {
    	/*---------------------------------------
    	| Get base filesystem for each link.	|
    	---------------------------------------*/
    	linkFileSys = getFileSys (inslist->hardLinks[i]);
    	if ( strcmp(linkFileSys, objectFileSys) )
    	{
            inserror (linkCrossFileSys, inslist->hardLinks[i], inslist->object_name);
            rc = 1;
    	}
    	/*-----------------------------------------------
    	| free space allocated by getFileSys()		|
    	-----------------------------------------------*/
    	free (linkFileSys);
    }

    free (objectFileSys);
    return rc;
}


/*-----------------------------------------------------------------------
| Return the file system name given a file or link name.		|
-----------------------------------------------------------------------*/

char *
getFileSys (char *objectName)
{
    char *fileSysName;
    char *ptr, *filesysptr;

    fileSysName = malloc (strlen(objectName) + 1);
    strcpy (fileSysName, objectName);

    /*-----------------------------------------------------------
    | Skip over the initial / to determine the file sys name	|
    | by putting null char at second / in name.			|
    -----------------------------------------------------------*/
    filesysptr = fileSysName;
    filesysptr++;
    if ( ptr = strchr (filesysptr, '/') )
	*ptr = '\0';

    /*-----------------------------------------------------------
    | If comparison fails we are in the root file system.	|
    -----------------------------------------------------------*/
    if ( strcmp(fileSysName,"/tmp") && strcmp (fileSysName,"/home") &&
	 strcmp(fileSysName,"/var") && strcmp (fileSysName,"/usr") )
        fileSysName[1] = '\0';

    return (fileSysName);
}

/*-----------------------------------------------------------------------
| Write an inslist entry to the output database file.  			|
-----------------------------------------------------------------------*/

void
writeDb (FILE *dbout_ptr, InsEntry *insentry)
{
    int i;
    char linktype[3];

    if (isupper (insentry->type))
	if (insentry->tcbflag == 'Y')
  	    fprintf(dbout_ptr, "%cT %d %d %o ",
				insentry->type,
				insentry->uid,
				insentry->gid,
				insentry->mode);
	else
	    fprintf(dbout_ptr, "%c %d %d %o ",
				insentry->type,
				insentry->uid,
				insentry->gid,
				insentry->mode);
    else /* lower case entry */
	if (insentry->tcbflag == 'Y')
	    fprintf(dbout_ptr, "%ct %d %d %o ",
				insentry->type,
				insentry->uid,
				insentry->gid,
				insentry->mode);
	else
	    fprintf(dbout_ptr, "%c %d %d %o ",
				insentry->type,
				insentry->uid,
				insentry->gid,
				insentry->mode);

    if ( isaLink(insentry->type) )	/* entry is a symbolic link */
	fprintf(dbout_ptr, "%s %s\n",insentry->object_name,insentry->target);
    else
    {
	fprintf(dbout_ptr, "%s\n", insentry->object_name);
	if (insentry->numHardLinks>0)  /* check for any hard links */
	    if ( isupper(insentry->type) )
	      	 if (insentry->tcbflag == 'Y')
		     strcpy(linktype,"HT");
		 else
		     strcpy(linktype,"H");
	    else	
	      	 if (insentry->tcbflag == 'Y')
		     strcpy(linktype,"ht");
		 else
		     strcpy(linktype,"h");
	for ( i=0; i < insentry->numHardLinks; i++ )
	    fprintf(dbout_ptr, "%s %d %d %o %s %s\n",
				linktype,
				insentry->uid,
				insentry->gid,
				insentry->mode,
				insentry->hardLinks[i],
				insentry->object_name);
    }
				
    fflush(dbout_ptr);
}
