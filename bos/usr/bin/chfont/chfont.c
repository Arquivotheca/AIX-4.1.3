static char sccsid[] = "@(#)41	1.27  src/bos/usr/bin/chfont/chfont.c, cmdlft, bos411, 9431A411a 7/27/94 13:42:50";

/*
 * COMPONENT_NAME: (CMDLFT) Low Function Terminal Commands
 *
 * FUNCTIONS:  main
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
  Creates a custom_font attribute stored in the CuAt for the current default
  display. The font will be loaded at the next system IPL
  ------------*/

#define SUCCESS 0
#define FAIL -1
#define LFTNUMFONTS 8   /* Should not be changed unless the LFTNUMFONTS */
			/* in lft.h changes.  Included here to avoid 	*/
			/* including lft.h and all of the other dependencies */

#define Bool unsigned	/* needed in aixfont.h */

#include <locale.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/aixfont.h>
#include "lftcmds.h"
#include "lftcmds_msg.h"
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <odmi.h>
#include <sys/lft_ioctl.h>



main(int argc, char *argv[])
{

	int rc, i, x, id;		/* temporary variables */
	int nbr_fonts;			/* number of fonts found in ODM */
	char crit[60];			/* temp string for ODM queries */
	char default_disp[NAMESIZE];	/* string for default display name */
	struct CuAt *cuat;		/* struct to hold CuAt info */
	lft_query_t lft_query;		/* struct to hold lft query info */

	/*------------
	  Open the message facilities catalog.
	  Retrieve the program name.
	  ------------*/
        setlocale(LC_ALL,"");
	catd=catopen("lftcmds.cat", NL_CAT_LOCALE);
	strcpy(program, MSGSTR(CHFONT, CHFONT_PGM, M_CHFONT_PGM));

	/* This command has to be run from a lft */
	if ( ioctl(0, LFT_QUERY_LFT, &lft_query) == -1)
	{
		fprintf(stderr,MSGSTR(COMMON, NOT_LFT, M_NOT_LFT),program);
		exit(FAIL);
	}

	/* Verify a valid parameter is there */
	if( argc != 2 || (strlen(argv[1]) != 1) || (!isdigit((int)*argv[1])) ) 
	{
		if( argc > 2 )
			fprintf(stderr,MSGSTR(COMMON,BADFLAG,M_BADFLAG),
					program, argv[2]);
		fprintf(stderr,MSGSTR(CHFONT, CHFONT_USAGE, M_CHFONT_USAGE));
		exit(FAIL);
	}

	/* Initialize the ODM database for access */
	if( (rc = odm_initialize()) < 0 )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_INIT, M_ODM_INIT), program);
		exit(FAIL);
	}

	/* find out how many fonts are stored in the ODM */
	for( nbr_fonts = i = 0; i < LFTNUMFONTS; i++ )
	{
		sprintf(crit, "font%d_path", i+1);
		if( (cuat = getattr("lft0",crit,FALSE, &x)) == NULL)
		{
			fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ, M_ODM_NOOBJ),
					program, "getattr()", "PdAt",crit);
			odm_terminate();
			exit(FAIL);
		}
		if( cuat->value[0] != '\0' )
			nbr_fonts++;
	}
	
	/* Verify the font id passed in points to a valid font */
	id = atoi( argv[1] );
	if ( id < 0 || id >= nbr_fonts )
	{
		fprintf(stderr,MSGSTR(CHFONT, CHFONT_ID, M_CHFONT_ID),id);
		odm_terminate();
		exit(FAIL);
	}
			
	/* get the name of the default display to change the font for */
	if( (cuat = getattr("lft0","default_disp",FALSE, &x)) == NULL)
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ, M_ODM_NOOBJ), program,
					"getattr()", "PdAt", "default_disp");
		odm_terminate();
		exit(FAIL);
	}
	
	/* make sure a default display is found in the CuAt */
	strcpy( default_disp, cuat->value );
	if ( default_disp[0] == '\0' )
	{
		fprintf(stderr,MSGSTR(COMMON, NO_DFLT, M_NO_DFLT), program);
		odm_terminate();
		exit(FAIL);
	}

	/* get the current custom font for the default display */
	if( (cuat = getattr(default_disp,"custom_font",FALSE, &x)) == NULL)
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ, M_ODM_NOOBJ), program,
					"getattr()", "PdAt", "custom_font");
		odm_terminate();
		exit(FAIL);
	}
	
	/* copy in id of new custom font and store it back in the CuAt */ 
	sprintf(cuat->value,"%d",id);
	if( putattr( cuat ) == -1 )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_FAIL, M_ODM_NOOBJ), program,
				"putattr()", "CuAt", "custom_font");
		odm_terminate();
		exit(FAIL);
	}

	odm_terminate();  /* close down the ODM */
	system("savebase");	/* flush out the changes to ODM */
	exit(SUCCESS);
}
 
