static char sccsid[] = "@(#)54	1.17  src/bos/usr/bin/mkfont/mkfont.c, cmdlft, bos411, 9433B411a 8/15/94 11:28:36";

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
  Create a new font in the ODM that lft can use on the next system ipl

  FUNCTIONS:
  verify_font() - Verifies the font path is valid then reads in font info
	to verify the font is not a variable width font.

  ------------*/

#define SUCCESS 0
#define FAIL -1

#define LFTNUMFONTS 8   /* Should not be changed unless the LFTNUMFONTS */
			/* in lft.h changes.  Included here to avoid 	*/
			/* including lft.h and all of the other dependencies */
#define Bool unsigned	/* needed in aixfont.c */

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

	int rc,i,x;		/* temporary variables */
	ulong value;
	char crit[60];
	struct CuAt *cuat;
	lft_query_t lft_query;


	/*------------
	  Open the message facilities catalog.
	  Retrieve the program name.
	  ------------*/
        setlocale(LC_ALL,"");
	catd=catopen("lftcmds.cat", NL_CAT_LOCALE);
	strcpy(program, MSGSTR(MKFONT, MKFONT_PGM, M_MKFONT_PGM));

	/* This command has to be run from a lft */
	if ( ioctl(0, LFT_QUERY_LFT, &lft_query) == -1)
	{
		fprintf(stderr,MSGSTR(COMMON, NOT_LFT, M_NOT_LFT),program);
		exit(FAIL);
	}

	/* Verify the parameter is there */
	if( argc != 2 )
	{
		if(argc > 2)
			fprintf(stderr,
				MSGSTR(COMMON,BADFLAG,M_BADFLAG),
                                program, argv[2]);
		fprintf(stderr,MSGSTR(MKFONT, MKFONT_USAGE, M_MKFONT_USAGE));
		exit(FAIL);
	}

	/* Verify the fontpath passed in points to a valid font file */
	if ( verify_font(argv[1]) != SUCCESS ) 
	{
		exit(FAIL);
	}
	
	/* Initialize the ODM database for access */
	if( (rc = odm_initialize()) < 0 )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_INIT, M_ODM_INIT), program);
		exit(FAIL);
	}

	/* find first unused font path for the lft in the odm */
	for( i = 1; i <= LFTNUMFONTS; i++ )
	{
		sprintf(crit, "font%d_path", i);
		if( (cuat = getattr("lft0",crit,FALSE, &x)) == NULL)
		{
			fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ, M_ODM_NOOBJ),
					program, "getattr()","PdAt",crit);
			odm_terminate();
			exit(FAIL);
		}
		if( cuat->value[0] == '\0' )
			break;
	}

	/* copy in name of new font and store it back in the CuAt */
	strcpy( cuat->value, argv[1] );
	if( putattr( cuat ) == -1 )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_FAIL, M_ODM_NOOBJ), program,
				"putattr()","CuAt",cuat->value);
		odm_terminate();
		exit(FAIL);
	}

	odm_terminate();  /* close down the ODM */
	system("savebase");	/* flush out the changes to ODM */
	exit(SUCCESS);
}
 
int verify_font( font_file )
char *font_file;
{
	FILE *font_fp;			/* font file descriptor */
	struct stat buffer;		/* buffer to hold font file info */
	aixFontInfoPtr aixfont;

	if( font_file[0] != '/' )
	{
		fprintf(stderr,MSGSTR(MKFONT,MKFONT_BADPATH,M_MKFONT_BADPATH),
				font_file);
		return(FAIL);
	}

	if( (font_fp = fopen(font_file, "r")) == NULL )
	{
		fprintf(stderr,MSGSTR(MKFONT,MKFONT_NOFILE,M_MKFONT_NOFILE),
				font_file);
		return(FAIL);
	}

	/* stat the font file to get necessary file information */
	if ( stat(font_file, &buffer) == -1 ) 
	{
 		fprintf(stderr,MSGSTR(MKFONT,MKFONT_NOSTAT,M_MKFONT_NOSTAT),
				font_file);
		return(FAIL);
	}

	/* Verify the font file size is ok */
	if( buffer.st_size < sizeof(aixFontInfo) ) 
	{
 		fprintf(stderr,MSGSTR(MKFONT,MKFONT_BADSIZE,M_MKFONT_BADSIZE),
				font_file);
		return(FAIL);
	}

	/* malloc a buffer to read in font info */
	if((aixfont = (aixFontInfoPtr) malloc(buffer.st_size)) == NULL)
	{
		fprintf(stderr,MSGSTR(COMMON,NO_SYS_MEM,M_NO_SYS_MEM),program);
		return(FAIL);
	}

	/* read in the font info */
	fseek(font_fp,0,0);
	if(fread(aixfont,buffer.st_size,1,font_fp) <= 0) 
	{
		fprintf(stderr,MSGSTR(MKFONT,MKFONT_NOREAD,M_MKFONT_NOREAD),
				font_file);
		return(FAIL);
	}

	/* Check for a variable width font.  LFT can't support it */
	if (aixfont->minbounds.characterWidth != 
			aixfont->maxbounds.characterWidth)
	{
		fprintf(stderr,MSGSTR(MKFONT,MKFONT_VARWIDTH,
				M_MKFONT_VARWIDTH));
		return(FAIL);
	}

	/* Sanity check on the size of the font passed in */
	if( aixfont->minbounds.characterWidth < 1   ||
			aixfont->minbounds.characterWidth > 100 )
	{
		fprintf(stderr,MSGSTR(MKFONT,MKFONT_BADINFO,M_MKFONT_BADINFO),
				font_file);
		return(FAIL);
	}
		
	return(SUCCESS);
}
