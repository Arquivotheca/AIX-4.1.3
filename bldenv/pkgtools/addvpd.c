static char sccsid[] = "@(#)07  1.4  src/bldenv/pkgtools/addvpd.c, pkgtools, bos41J, 9512A_all 3/16/95 09:50:14";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: addToVpd
 *		fatal
 *		getChecksum
 *              initVpdPath
 *              usage
 *		warning
 *              xmalloc
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

#include <swvpd.h>
#include <sys/param.h>
#include <errno.h>
#include <varargs.h>
#include "stanza.h"
#include "addvpd.h"

extern char *	optarg;

/*-----------------------------------------------------------------------
| The addvpd command reads an input stanza file and adds an entry	|
| to the vital products database (vpd) for each file entry in the	|
| input file.								|
-----------------------------------------------------------------------*/

main (int argc, char **argv)
{
	int arg, rc=0;
	char *stanzaFile, *lppname, *objDir;
	char vpdPath[MAXPATHLEN];
	char stanza[STANZASIZE];
	char	value[LENSIZE];
	FILE *fp;
	inv_t inv_entry;
	short lpp_id=0;

	objDir = NULL;
	stanzaFile = NULL;
	lppname = NULL;

	while ( (arg = getopt (argc, argv, "d:f:l:")) != EOF )
	{
	    switch (arg)
	    {
		case 'd':
			objDir = optarg;
			break;
		case 'f':
			stanzaFile = optarg;
			break;
		case 'l':
			lppname = optarg;
			break;
		case '?':
			usage ();
	    }
	}

	/*---------------------------------------
	| Verify command line parameters.	|
	---------------------------------------*/

	if ( !strlen (stanzaFile) || !strlen (lppname) || !strlen (objDir) )
	{
		fprintf (stderr,Missing_Opt,COMMANDNAME);
		usage ();
	}

	initVpdPath (objDir);

	/*-------------------------------
	| Get the lpp id from the vpd.	|
	-------------------------------*/
	if (vpdreslpp_name(lppname,&lpp_id) != VPD_OK)
		fatal (No_LPP_Id, lppname);

	if ( (fp = fopen(stanzaFile, "r")) == NULL )
		fatal (File_Open_Failed,stanzaFile,errno);

	/*---------------------------------------------------------------
	| Only add stanzas which contain "apply" on the class line to	|
	| the vpd.  Inventory only entries do not contain apply and	|
	| should not be in the vpd.					|
	---------------------------------------------------------------*/
	while ( (rc = readStanza (fp, stanza, STANZASIZE)) == 0 )
	{
		memset ((void *) &inv_entry, 0, sizeof inv_entry);
		if ( !(rc = getEntry ("class",stanza,value)) && strlen (value) )
		{
		    if ( !strstr (value, "apply") )
			continue;
		}
		else
		    if ( rc == OVERFLOW )
			fatal (Class_Overflow, stanza, LENSIZE);
		    else
			fatal (No_Class, stanza);
		addToVpd (&inv_entry, stanza, vpdPath, lpp_id);
	}

	if ( rc == OVERFLOW )
	    fatal (Stanza_Overflow, STANZASIZE, stanza);
	    
	exit (0);
}

void
usage ()
{
	fprintf (stderr,Usage,COMMANDNAME);
	exit (-1);
}

/*---------------------------------------------------------------
| Initialize and open the vpd with the location provided by	|
| the -d parameter.						|
---------------------------------------------------------------*/

void
initVpdPath ( char *objDir )
{
	if (vpdremotepath(objDir) != VPD_OK)
	    fatal (VPD_Open_Failed);
	else vpdremote();
}

/*-----------------------------------------------------------------------
| Fill in the inv_entry struct with size, checksum, link and symlink	|
| information from the current stanza. If an entry already exists	|
| for a file, display a warning message and do not do a addvpd.		|
-----------------------------------------------------------------------*/

void
addToVpd (inv_t *inv_entry, char *stanza, char *vpdPath, short lpp_id)
{
    char	value[LENSIZE];
    char	*ptr;
    long	size, cksum;
    int		nameLen=0, rc=0;
    inv_t	get_entry;

    memset ((void *) value, 0, LENSIZE);

    inv_entry->lpp_id = lpp_id;

    /*-------------------------------------------
    | Get the file name from the stanza.	|
    -------------------------------------------*/
    ptr = strchr (stanza, ':');
    nameLen = ptr - stanza;

    /*-------------------------------------------------------------------
    | The following 128 is from the /usr/include/swvpd.h file.  If	|
    | that value (size of loc0 in inventory struct) changes then so	|
    | should this.							|
    -------------------------------------------------------------------*/
    strncpy (inv_entry->loc0, stanza, (nameLen < 128 ? nameLen: 128));

    inv_entry->private = 0;
    inv_entry->format = INV_FILE;

    if ( !getEntry ("size",stanza,value) && strlen (value) )
    {
	if (  !strcmp(value, "VOLATILE") )
	    size = 0;
	else
	{
	    errno = 0;
	    if ( !(size = strtol (value, NULL, 10)) && errno )
		fatal (No_Size, errno, stanza);
	}
        inv_entry->size = size;
    }
    else
	/*-----------------------------------------------
	| No size value in stanza.  Default to 0.	|
	| This is the case for directory entries.	|
	-----------------------------------------------*/
	inv_entry->size = 0;

    if ( !(rc = getEntry ("type",stanza,value)) )
    {
	if (  !strcmp(value, "SYMLINK") )
		inv_entry->file_type = 6;
    }

    if ( !(rc = getEntry ("links",stanza,value)) )
    {
	inv_entry->loc1 = xmalloc (strlen(value)+1);
	strcpy (inv_entry->loc1, value);
    }

    if ( rc == OVERFLOW )
	fatal (Link_Overflow, "LENSIZE", stanza);

    if ( !(rc = getEntry ("target",stanza,value)) )
    {
	inv_entry->loc2 = xmalloc (strlen(value) + 1);
	strcpy (inv_entry->loc2, value);
    }

    inv_entry->checksum = getChecksum (stanza, value);

    get_entry = *inv_entry;
    if ( vpdget (INVENTORY_TABLE, INV_LOC0, &get_entry) == VPD_OK )
	warning (Entry_Exists, inv_entry->loc0);
    else
	if ( vpdadd (INVENTORY_TABLE, inv_entry) != VPD_OK )
	    warning (VPD_Add_Failed,inv_entry->loc0);
}

/*-----------------------------------------------------------------------
| getChecksum returns the integer checksum value for this stanza.	|
| The format of a checksum attribute in the input file is		|
|	checksum = "11804     2 "					|
| so we need to extract the 11804 and convert it to integer.		|
| Directory entries do not have a checksum so default to 0.		|
-----------------------------------------------------------------------*/

getChecksum (char *stanza, char *value)
{
    int checksum=0;
    char cksumDigits[LENSIZE];
    char *ptr = value;

    if ( !getEntry ("checksum",stanza,value) && strlen (value) )
    {
	if ( strcmp(value, "VOLATILE") )
	{
	    /*-------------------------------------------
	    | Skip leading non digit (") character and	|
	    | copy.					|
	    -------------------------------------------*/
	    while ( !isdigit(*ptr) )
		ptr++;
	    strcpy (cksumDigits, ptr);

	    /*---------------------------------------------------
	    | Save only numeric characters in cksumDigits.	|
	    ---------------------------------------------------*/
	    ptr = cksumDigits;
	    while ( isdigit(*ptr) )
		ptr++;
	    *ptr = '\0';

	    errno = 0;
	    if ( !(checksum = strtol (cksumDigits, NULL, 10)) && errno )
		fatal (No_Checksum, errno, stanza);
	}
    }
    return (checksum);
}

/*---------------------------------------
| Malloc with error checking.		|
---------------------------------------*/

char *
xmalloc (int len)
{
        char    *cp;

        if (cp = malloc ((unsigned) len))
	{
                memset ((void *) cp, 0, len);
                return (cp);
        }
	else
	    fatal (Malloc_Error, errno);
}

/*-----------------------------------------------------------------------
| fatal accepts a variable length argument list which is a message	|
| format string and any arguments to the error message.  The arguments	|
| are combined and displayed to stderr and fatal exits with a -1.	|
-----------------------------------------------------------------------*/

void
fatal (va_alist)
{
	va_list ap;
	char *msgFormat;

	fprintf (stderr,"%s:  FATAL ERROR:\n",COMMANDNAME);

	va_start(ap);
	msgFormat = va_arg(ap, char *);
	vfprintf (stderr, msgFormat, ap);
	va_end(ap);

	fprintf (stderr,"\n\tTerminating.\n");
	exit (-1);
}
	
/*-----------------------------------------------------------------------
| warning accepts a variable length argument list which is a message	|
| format string and any arguments to the warning message.  The 		|
| arguments are combined and displayed to stderr.			|
-----------------------------------------------------------------------*/

void
warning (va_alist)
{
	va_list ap;
	char *msgFormat;
	int argno = 0;

	fprintf (stderr,"%s:  WARNING:\n",COMMANDNAME);

	va_start(ap);
	msgFormat = va_arg(ap, char *);
	vfprintf (stderr, msgFormat, ap);
	va_end(ap);
}
