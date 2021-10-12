static char sccsid[] = "@(#)62	1.6  src/bos/diag/util/uformat/disp_patt.c, dsauformat, bos411, 9428A410j 2/7/94 16:05:17";
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: disp_blocks
 *		disp_read_info
 *		display_bad_blocks
 *		display_pattern
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include "diag/diag.h"
#include "diag/da.h"
#include "diag/da_rc.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "uformat.h"
#include "ufmt_msg.h"

extern char * diag_cat_gets();
extern struct da_menu dmnu;
int disp_blocks(char * );

/*
 * NAME: display_pattern
 *
 * FUNCTION: Display selections and accept changes
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */


int
display_pattern()
{
	/* these external declarations are to tell the compiler that these
 	* variables have been declared outside of this module.
 	*/
	extern struct data_dclass dclass_data;
	extern struct da_menu dmnu;
	extern nl_catd cdep_catd;
	int   	i, rc = -1;
	int	entry = 0, entry_size;
	char	buffer[512];
	ASL_SCR_INFO	*menuinfo;
	static	ASL_SCR_TYPE	menutype = DM_TYPE_DEFAULTS;



	/* allocate space for title, selection, last line, and vpd data */
	menuinfo = (ASL_SCR_INFO *) calloc(8, sizeof(ASL_SCR_INFO));
	if(menuinfo == (ASL_SCR_INFO *) NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/* put in the title and resource description */
	menuinfo[0].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,MENU_DCLASS_SELECT);

	/* set up menu for the number of entries */
	for(entry = 1;entry <= ENTRIES;entry++)
	{
		menuinfo[entry].entry_size = 2;
		menuinfo[entry].data_value = (char *) malloc(menuinfo[entry].entry_size + 1);
		if(menuinfo[entry].data_value == (char *) NULL)
		{
			disp_menu(UNKNOWN_ERR);
			clean_up();
		}


		menuinfo[entry].disp_values = (char *) malloc(512);

		if(menuinfo[entry].disp_values == (char *) NULL)
		{
			disp_menu(UNKNOWN_ERR);
			clean_up();
		}


		
		menuinfo[entry].multi_select = ASL_YES;
		menuinfo[entry].entry_type = ASL_HEX_ENTRY;


		/* set up the menu for each of the entries. */
		switch(entry)
		{
			/* this is the number of patterns to write */
		case 1:

			menuinfo[entry].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,NUM_OF_PATTERNS);
			strcpy(menuinfo[entry].disp_values,NO_OF_WRITES);
			menuinfo[entry].entry_size = 1;
			sprintf(menuinfo[entry].data_value,"%x",dclass_data.num);
			menuinfo[entry].op_type = ASL_RING_ENTRY;
			menuinfo[entry].entry_type = ASL_NO_ENTRY;
			break;

			/* case 2 is the first pattern to write */
		case 2:
			menuinfo[entry].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,PATTERN1);
			menuinfo[entry].entry_size = 8;
			sprintf(menuinfo[entry].disp_values,"%x",dclass_data.pattern[entry-2]);
			sprintf(menuinfo[entry].data_value,"%x",dclass_data.pattern[entry-2]);
			break;

			/* case 3 is the second pattern to write */
		case 3:
			menuinfo[entry].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,PATTERN2);
			menuinfo[entry].entry_size = 8;
			sprintf(menuinfo[entry].disp_values,"%x",dclass_data.pattern[entry-2]);
			sprintf(menuinfo[entry].data_value,"%x",dclass_data.pattern[entry-2]);
			break;

			/* case 4 is the third pattern to write */
		case 4:
			menuinfo[entry].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,PATTERN3);
			menuinfo[entry].entry_size = 8;
			sprintf(menuinfo[entry].disp_values,"%x",dclass_data.pattern[entry-2]);
			sprintf(menuinfo[entry].data_value,"%x",dclass_data.pattern[entry-2]);
			break;

			/* case 5 is whether to write a random pattern */
		case 5:
			strcpy(menuinfo[entry].disp_values,YN);
			menuinfo[entry].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,RAND_PATTERN);
			menuinfo[entry].data_value[0] = dclass_data.rand;
			menuinfo[entry].data_value[1] = 0; /* NULL terminate */
			menuinfo[entry].entry_size = 1;
			menuinfo[entry].entry_type = ASL_NO_ENTRY;
			menuinfo[entry].op_type = ASL_RING_ENTRY;
			break;

		default:
			return(DIAG_ASL_FAIL);
		}


	}

	/* put in the last line */
	menuinfo[ENTRIES+1].text = diag_cat_gets(cdep_catd, DUFORMAT_SET, OPTION_MODIFY);

	menutype.max_index = ENTRIES+1;
	menutype.multi_select = ASL_YES;
	menutype.screen_code = ASL_DIAG_DIALOGUE_SC;

	while ( rc != DIAG_ASL_CANCEL && rc != DIAG_ASL_EXIT && 
	    rc != DIAG_ASL_COMMIT ) {
		rc = diag_display(MENU_BASE + 0x30, cdep_catd, NULL, DIAG_IO, 
		    ASL_DIAG_DIALOGUE_SC, &menutype, menuinfo);
		if(rc == ASL_HELP)
		{
			if(menutype.cur_index <  5 && menutype.cur_index > 1)
				strcpy(buffer,diag_cat_gets(cdep_catd,DUFORMAT_SET,HELP));
			else
				strcpy(buffer,diag_cat_gets(cdep_catd,DUFORMAT_SET,NOHELP));

			asl_vnote(ASL_MSG,ASL_NO_LOG,buffer,"");

		}
	}


	/*
	 * If commit data, then assign data to structure for later access
	 */
	if(rc == DIAG_ASL_COMMIT)
	{
		dclass_data.num = strtol(menuinfo[1].data_value,(char * *) NULL, 16);
		dclass_data.pattern[0] = (unsigned long) strtoul(menuinfo[2].data_value,(char * *) NULL, 16);
		dclass_data.pattern[1] = (unsigned long) strtoul(menuinfo[3].data_value,(char * *) NULL, 16);
		dclass_data.pattern[2] = (unsigned long) strtoul(menuinfo[4].data_value,(char * *) NULL, 16);
		dclass_data.rand = menuinfo[5].data_value[0];
	}

	return(rc);
}

/*  */
/*
 * NAME: disp_read_info
 *                                                                    
 * FUNCTION: Display read info for blocks to read.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */
int
disp_read_info()
{

	extern struct read_blk read_data;
	extern struct da_menu dmnu;
	int   rc = -1;
	int	entry = 0, entry_size;
	extern nl_catd cdep_catd;
	ASL_SCR_INFO	*menuinfo;
	static	ASL_SCR_TYPE	menutype = DM_TYPE_DEFAULTS;

	/* allocate space for title, selection, last line*/
	menuinfo = (ASL_SCR_INFO *) calloc(4, sizeof(ASL_SCR_INFO));
	if(menuinfo == (ASL_SCR_INFO *) NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/* put in the title and resource description */
	menuinfo[0].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,MENU_DCLASS_SELECT);


	for(entry=1;entry <= 2;entry++)
	{
		menuinfo[entry].data_value = (char *) malloc(menuinfo[entry].entry_size + 1);
		if(menuinfo[entry].data_value == (char *) NULL)
		{
			disp_menu(UNKNOWN_ERR);
			clean_up();
		}
		memset(menuinfo[entry].data_value,'\0',menuinfo[entry].entry_size+1);


		menuinfo[entry].disp_values = (char *) malloc(512);
		if(menuinfo[entry].disp_values == (char *) NULL)
		{
			disp_menu(UNKNOWN_ERR);
			clean_up();
		}

		memset(menuinfo[entry].disp_values,'\0',512);


		menuinfo[entry].entry_type = ASL_HEX_ENTRY;
	}

	/* 
	 * set up first entry 
	 */
	strcpy(menuinfo[1].disp_values,"0");
	menuinfo[1].entry_size = 8;
	menuinfo[1].required = ASL_YES;
	menuinfo[1].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,BLK_TO_READ);

	/* 
	 * default which lba to start at
	 */
	sprintf(menuinfo[1].data_value,"%x",read_data.lba);

	/* 
	 * set up entry #2
	 */
	menuinfo[2].entry_size = 3;
	menuinfo[2].required = ASL_YES;
	menuinfo[2].op_type = ASL_RING_ENTRY;
	menuinfo[2].text = diag_cat_gets(cdep_catd,DUFORMAT_SET,NUM_OF_BLKS_TO_READ);
	strcpy(menuinfo[2].disp_values,BLKS_TO_READ);

	/*
	 * default how many blocks to read up to 10
	 */
	sprintf(menuinfo[2].data_value,"%x",read_data.how_many);
	menuinfo[2].entry_type = ASL_NO_ENTRY;

	menuinfo[3].text = diag_cat_gets(cdep_catd, DUFORMAT_SET, OPTION_MODIFY);

	menutype.max_index = 3;

	do
	{
		rc = diag_display(0x802033, cdep_catd, NULL, DIAG_IO, 
		    ASL_DIAG_DIALOGUE_SC, &menutype, menuinfo);
	}	while ( (rc != DIAG_ASL_CANCEL && rc != DIAG_ASL_EXIT && 
	    rc != DIAG_ASL_COMMIT) || (rc == DIAG_ASL_FAIL) ) ;

	/*
	 * If commit data, then assign data to structure for later access
	 */
	if(rc == DIAG_ASL_COMMIT)
	{
		read_data.lba = (unsigned long) strtoul(menuinfo[1].data_value,(char * *)NULL, 16);
		read_data.how_many = strtol(menuinfo[2].data_value,(char * *)NULL, 16);
	}

	return(rc);

}/* end of disp_read_info() */



/*  */
/*
 * NAME: display_bad_blks
 *                                                                    
 * FUNCTION: Display bad blocks
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *  NUMBER OF BAD BLOCKS
 */
int display_bad_blocks(struct bad_block * bad_block)
{
	int i,j,rc;
	int total;
	extern nl_catd cdep_catd;
	char * list1;
	char tmp[512];
	ASL_SCR_INFO	*menuinfo;
	static	ASL_SCR_TYPE	menutype = DM_TYPE_DEFAULTS;

	total = bad_block->total;

	/* 
	 * if no bad blocks, state that there are no bad blocks
	 */
	if(bad_block->total == 0)
	{
		disp_menu(NO_BAD_BLOCKS);
	}
	else /* put blocks into list which can be displayed */
	{

		list1 = (char *) calloc((bad_block->total * 512) + 10, sizeof(char) );
		sprintf(&list1[0],diag_cat_gets(cdep_catd,DUFORMAT_SET,BAD_BLOCK_HEADER));
		for(i=0,j=strlen(list1);i< bad_block->total ; i++)
		{
			sprintf(tmp,"%d / %d / %d\n\0",bad_block->cylinder,bad_block->head,bad_block->sector);


			strncpy(&list1[j],tmp,strlen(tmp));
			j += strlen(tmp);

			bad_block = bad_block->next;
		}

		/* call function to display list */
		rc = disp_blocks(list1);
		if( rc == DIAG_ASL_CANCEL || rc == DIAG_ASL_EXIT)
		{
			clean_up();
		}

	}

	return(total);

} /* end of display_bad_blocks() */

/*
 * NAME: disp_blocks
 *                                                                    
 * FUNCTION: display the blocks which are bad.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *		Returns whatever is returned from diag_display 
 */
int disp_blocks(char * blk_list)
{

	extern nl_catd cdep_catd;
	ASL_SCR_INFO	*menuinfo;
	static	ASL_SCR_TYPE	menutype = DM_TYPE_DEFAULTS;
	char temp_buff[512];
	int rc;

	menuinfo = (ASL_SCR_INFO *)calloc(3,sizeof(ASL_SCR_INFO));
	if(menuinfo == (ASL_SCR_INFO *) NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/* title line */
	sprintf(temp_buff,
	    diag_cat_gets(cdep_catd,DUFORMAT_SET,BAD_BLOCKS_START),
	    operation);
	menuinfo[0].text = (char *)malloc(strlen(temp_buff)+1);
	if(menuinfo[0].text == (char *) NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}
	strcpy(menuinfo[0].text,temp_buff);

	/* bad blocks which were found */
	menuinfo[1].text = (char *)malloc(strlen(blk_list)+1);
	if(menuinfo[1].text == (char *) NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}
	strcpy(menuinfo[1].text,blk_list);

	/* put in the continue last line */
	menuinfo[2].text = diag_cat_gets(cdep_catd, DUFORMAT_SET, OPTION_CONTINUE);

	menutype.max_index = 2;

	rc = diag_display(MENU_BASE + 0x30, cdep_catd, NULL, DIAG_IO,
	    ASL_DIAG_KEYS_ENTER_SC, &menutype, menuinfo);

	return(rc);
}
