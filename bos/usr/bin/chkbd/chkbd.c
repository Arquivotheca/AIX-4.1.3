static char sccsid[] = "@(#)43	1.19.1.7  src/bos/usr/bin/chkbd/chkbd.c, cmdlft, bos41J, 9517B_all 4/28/95 10:05:40";

/*
 * COMPONENT_NAME: (CMDLFT) Low Function Terminal Commands
 *
 * FUNCTIONS:  main, parse
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*------------
  PURPOSE:
  Changes the default software keyboard map. The chkbd command will update
  the CuAt attribute, "swkb_path", with the pathname passed into the command.
  This new path will be used during the next IPL to load the new software
  keyboard map.

  FUNCTIONS:
  update_odm: Updates the ODM database for the software keyboard in the LFT
  ------------*/

#include <locale.h>
#include <fcntl.h>
#include "lftcmds.h"
#include <sys/cfgodm.h>
#include <odmi.h>
#include <sys/lft_ioctl.h>

#define SUCCESS 0
#define FAIL -1

main(argc, argv)
int argc;
char *argv[];
{
	char command[256];		/* command string */
	struct stat stbuf;		/* stat buffer for file check */
	char filename[256];		/* full pathname of input file */
	lft_query_t lft_query;		/* struct to hold lft query info */


	setlocale(LC_ALL,"");
	/*------------
	  Open the message facilities catalog.
	  Retrieve the program name.
	  ------------*/
	catd=catopen("lftcmds.cat", NL_CAT_LOCALE);
	strcpy(program, MSGSTR(CHKBD, CHKBD_PGM, M_CHKBD_PGM));

	/*------------
	  If no arguments are specified, display the command usage and exit.
	  ------------*/
	if (argc != 2) 
	{
		fprintf(stderr,MSGSTR(CHKBD, CHKBD_USAGE, M_CHKBD_USAGE));
		exit(FAIL);
	}
	/*------------
	  This command must be run from anything, NOT just an lft.
	  We don't care.
	*------------*/

	/*------------
	  If path is not absolute, concatenate it to the default directory
	  path otherwise just use the absolute path.
	  ------------*/
	if ( argv[1][0] != '/' )
	{
		strcpy(filename, "/usr/lib/nls/loc/");
		strcat(filename, argv[1]);
	}
	else
	{
		strcpy(filename, argv[1]);
	}

	/*------------
	  Verify the swkbd file exists
	  ------------*/
	if (stat(filename, &stbuf) == -1) 
	{
		fprintf(stderr,MSGSTR(CHKBD, CHKBD_NOSTAT, M_CHKBD_NOSTAT),
				filename);
		fprintf(stderr,MSGSTR(CHKBD, CHKBD_USAGE, M_CHKBD_USAGE));
		exit(FAIL);  
	}			 

	/*------------
	  Set the default software keyboard in the ODM.
	  ------------*/
	if ( update_odm("lft0", filename) != SUCCESS )
		exit(FAIL);

	/*------------
	  Close the message facilities catalog and exit
	  ------------*/
	catclose(catd);
	exit(SUCCESS);

} /* end main */




/*------------
  Update the ODM database entry for the software keyboard in the LFT
  ------------*/
update_odm(lname, swkb_path)
char *lname;			/* logical device name */
char *swkb_path;		/* new software keyboard path */
{
	int rc;			/* return code */
	struct CuAt	*cuat;	/* storage to get customized attribute into */

	/*------------
	  Initialize the ODM database for access
	  ------------ */
	if ( odm_initialize() < 0 )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_INIT, M_ODM_INIT), program);
		return(FAIL);
	}

	/*------------
	  Call generic getattr routine which will search for the
	  attribute value in customized first, then if not found,
	  will search predefined for the attribute value.
	  ------------*/
	if ( (cuat = getattr(lname, "swkb_path", FALSE, &rc)) == NULL )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ, M_ODM_NOOBJ), program,
				"getattr()","PdAt","attribute=swkb_path");
		odm_terminate();
		return(FAIL);
	}

	/*------------
	  Copy new software keyboard path into attribute object
	  ------------*/
	strcpy(cuat->value, swkb_path);

	/*------------
	  Put the new attribute object into ODM
	  ------------*/
	if ( putattr(cuat) == -1)
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_FAIL, M_ODM_FAIL), program,
				"putattr()","CuAt","swkb_path");
		odm_terminate();
		return(FAIL);
	}

	/* terminate ODM and return */
	odm_terminate();
	system("savebase");	/* flush out the changes to ODM */
	return(SUCCESS);
}
