static char sccsid[] = "@(#)51	1.19  src/bos/usr/bin/lsfont/lsfont.c, cmdlft, bos411, 9428A410j 5/3/94 07:59:25";

/*
 * COMPONENT_NAME: (CMDLFT) Low Function Terminal Commands
 *
 * FUNCTIONS:  main, create_list, print_list
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
	Display a list of font information available on the lft.

  FUNCTIONS:
	create_list() - create a linked list of font information for each
	font available to the lft.

	print_list() - display the font id, file name, family, encoding, 
	glyph size, and font name for each font currently used by the lft.
  ------------*/

#define SUCCESS 0
#define FAIL -1
#define FONTSIZE 50
#define LFTNUMFONTS 8   /* Should not be changed unless the LFTNUMFONTS */
			/* in lft.h changes.  Included here to avoid 	*/
			/* including lft.h and all of the other dependencies */

#define Bool unsigned /* needed for aixfont.c */

#include <locale.h>
#include "lftcmds.h"
#include "lftcmds_msg.h"
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <odmi.h>
#include <sys/stat.h>
#include <sys/aixfont.h>
#include <stdio.h>


struct _font_info 
{
	char	filename[ATTRVALSIZE];
	char	glyph_size[6];
	char	encoding[FONTSIZE];
	struct	_font_info *next;
} *font_info = NULL;
int lflag = FALSE;
int fn_size=4, fe_size=8;


main(int argc, char *argv[])
{

	setlocale(LC_ALL,"");
	/*------------
	  Open the message facilities catalog.
	  Retrieve the program name.
	  ------------*/
	catd=catopen("lftcmds.cat", NL_CAT_LOCALE);
	strcpy(program, MSGSTR(LSFONT, LSFONT_PGM, M_LSFONT_PGM));

	/* Verify correct options were specified */
	if (! (argc == 1 || (argc == 2 && strcmp(argv[1],"-l") == 0)) )
	{
		if ( argc == 2 )
			fprintf(stderr,MSGSTR(COMMON, BADFLAG, M_BADFLAG),
					program,argv[1]);
		fprintf(stderr,MSGSTR(LSFONT, LSFONT_USAGE, M_LSFONT_USAGE));
		exit(FAIL);
	}
	if ( argc == 2 )
		lflag = TRUE;

	/*------------
	  Create the font list
	  ------------*/
	if ( create_list() != SUCCESS)
		exit(FAIL);

	/*------------
	  print them out
	  ------------*/
	if ( print_list() != SUCCESS)
		exit(FAIL);

	/*------------
	  Close the message facilities catalog
	  ------------*/
	catclose(catd);

	exit(SUCCESS);
}

/************************************************************************
  The create_list function will create a linked list containing information 
  about each available lft font.
************************************************************************/

int create_list()
{

	int rc, i, x, y;		/* temporary variables */
	char crit[60];
	char filename[ATTRVALSIZE];
	FILE *font_fp;			/* font file descriptor */
	struct CuAt *cuat;		/* storage customized attributes */
	struct _font_info *linklst_ptr;	/* pointer to font structure */
	struct _font_info *tmp_ptr;	/* temp pointer to font structure */
	struct stat buffer;		/* buffer to hold font file info */
	aixFontInfoPtr aixfont;		/* structures to store font info */
	aixFontPropPtr aixprop;
	char *string_pool, *ptr;	/* pointer to the font string pool */


	/*------------
	  Initialize the ODM database for access
	  ------------*/
	if( (rc = odm_initialize()) < 0 ) 
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_INIT, M_ODM_INIT), program);
		return(FAIL);
	}

	/* loop thru all font paths stored in the ODM */
	for( i = 1; i <= LFTNUMFONTS; i++ )
	{
		sprintf(crit, "font%d_path", i);
		if( (cuat = getattr("lft0",crit,FALSE, &x)) == NULL)
		{
			fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ, M_ODM_NOOBJ), 
					program, "getattr()", "PdAt", crit);
			odm_terminate();
			return(FAIL);
		}
		strcpy(filename, cuat->value);

		/* if font path exist, read in font info into a linked list */
		if( filename[0] != '\0' )
		{
			if( (font_fp = fopen(filename, "r") ) == NULL )
			{
				fprintf(stderr,MSGSTR(LSFONT,LSFONT_NOFILE,
						M_LSFONT_NOFILE), filename);
				odm_terminate();
				return(FAIL);
			}

			/* malloc new structure for linked list */
			tmp_ptr = (struct _font_info*) 
					malloc( sizeof(struct _font_info));
			if ( tmp_ptr == NULL )
			{
				fprintf(stderr,MSGSTR(COMMON,NO_SYS_MEM,
						M_NO_SYS_MEM), program);
				odm_terminate();
				return(FAIL);
			}		

			if ( font_info == NULL ) 
			{
				font_info = linklst_ptr = tmp_ptr;
			}
			else 
			{
				linklst_ptr->next = tmp_ptr;
				linklst_ptr = linklst_ptr->next;
			}

			linklst_ptr->next = NULL;

			/* stat the font file to get file information */
			if ( stat(filename, &buffer) == -1 ) 
			{
				fprintf(stderr,MSGSTR(LSFONT,LSFONT_NOSTAT,
						M_LSFONT_NOSTAT), filename);
				odm_terminate();
				return(FAIL);
			}

			/* Verify the font file size is ok */
			if( buffer.st_size < sizeof(aixFontInfo) ) 
			{
				fprintf(stderr,MSGSTR(LSFONT,LSFONT_BADSIZE,
						M_LSFONT_BADSIZE),filename);
				odm_terminate();
				return(FAIL);
			}

			/* allocate memory to store font file info */
			aixfont = (aixFontInfoPtr) malloc( buffer.st_size );
			if ( aixfont == NULL )
			{
				fprintf(stderr,MSGSTR(COMMON,NO_SYS_MEM,
						M_NO_SYS_MEM), program);
				odm_terminate();
				return(FAIL);
			}

			/* seek to beginning of file and read it in */
			fseek(font_fp,0,0);
			if(fread(aixfont,buffer.st_size,1,font_fp) <= 0) 
			{
				fprintf(stderr,MSGSTR(LSFONT,LSFONT_NOREAD,
						M_LSFONT_NOREAD),filename);
				odm_terminate();
				return(FAIL);
			}

			/* Get to the properties */
			aixprop = (aixFontPropPtr) ((char *) aixfont +
					( BYTESOFFONTINFO(aixfont) +
					 BYTESOFCHARINFO(aixfont) +
					 BYTESOFGLYPHINFO(aixfont) ));

			/* Set string_pool to point past font property info */
			string_pool = (char *) aixprop + 
					BYTESOFPROPINFO(aixfont);
	  
			/* Go through the props searching for FONT property */
			for(x = aixfont->nProps; x ; x--, aixprop++) 
			{
				if(!strcmp(&string_pool[aixprop->name],"FONT"))
				{
					ptr = &string_pool[aixprop->value] + 
					 strlen(&string_pool[aixprop->value])-1;
					for(y = 2; y ; ptr--) 
					{
						if( *ptr == '-' )
							y--;
					}
	  				strcpy(linklst_ptr->encoding, ptr+2);
	  				y = strlen(linklst_ptr->encoding);
	  				if (fe_size < y ) fe_size = y;
					break;
				}
			}
			/* get the basename of the font file and copy it */
			ptr = filename + strlen(filename) - 1;
			while( *ptr != '/' ) ptr--;
			strcpy(linklst_ptr->filename, ++ptr);
			x = strlen(linklst_ptr->filename);
			if (fn_size < x) fn_size = x;

			sprintf(linklst_ptr->glyph_size,"%2dx%2d",
					aixfont->minbounds.characterWidth,
					aixfont->minbounds.ascent + 
					aixfont->minbounds.descent);

		} /* end of if(filename[0] != '\0') */

	} /* end of for loop of all fonts */

	odm_terminate();  /* close down the ODM */
}

/************************************************************************
  The print_list function prints the lsfont header and then traverses 
  the font linked list printing the information in their selected
  order. 
************************************************************************/


print_list()
{
	int i, id;		/* Temporary variables */
	char ss[30];

	fn_size+=2;
	if ( lflag == FALSE )
	{
		/* print header info from translated message file */
		printf( "\n\n");
		sprintf(ss, "%%-6s%%-%2ds%%-7s%%-%2ds\n", fn_size, fe_size);
		printf(ss, 
			MSGSTR(LSFONT,LSFONT_FONT,M_LSFONT_FONT),
			MSGSTR(LSFONT,LSFONT_FILE,M_LSFONT_FILE),
			MSGSTR(LSFONT,LSFONT_GLYPH,M_LSFONT_GLYPH),
			MSGSTR(LSFONT,LSFONT_FONT,M_LSFONT_FONT) );

		printf( ss,
			MSGSTR(LSFONT,LSFONT_ID,M_LSFONT_ID),
			MSGSTR(LSFONT,LSFONT_NAME,M_LSFONT_NAME),
			MSGSTR(LSFONT,LSFONT_SIZE,M_LSFONT_SIZE),
			MSGSTR(LSFONT,LSFONT_ENCODING,M_LSFONT_ENCODING) );

		/* print delimiter info */
		printf("%s","====  ");
		for(i=2;i<fn_size;i++) printf("%c",'=');
		printf("%s","  =====  ");
		for(i=0;i<fe_size;i++) printf("%c",'=');
		printf("\n");
	}
		
	/* print font information for each font in the list */
	sprintf(ss, "%%-6d%%-%2ds%%-7s%%-%2ds\n", fn_size, fe_size);
	for( id=0; font_info != NULL ;font_info = font_info->next, id++) 
	{
		printf( ss, id, font_info->filename,
				font_info->glyph_size, font_info->encoding);
	}

	if ( lflag == FALSE )
		printf("\n");

	return(SUCCESS);
}
