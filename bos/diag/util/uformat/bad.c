static char sccsid[] = "@(#)63	1.2  bad.c, dsauformat, bos410 8/2/93 15:02:05";
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: 
 *		dsp_bad_blks
 *		glist
 *		initcmd
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include<stdio.h>
#include<fcntl.h>
#include<sys/scsi.h>
#include<sys/scdisk.h>
#include<errno.h>
#include"uformat.h"
#include"diag/scsi_atu.h" 



struct command{
	unsigned int data_length;/* larger of data to be received and transmitted */
	unsigned int timeout;
	uchar flags;
	uchar cmd_length;
	uchar cmd[12];
	unsigned int d_len;
	uchar data[512];
};

void glist(char * ,int );
void initcmd(struct command tu);
static struct bad_block * list = NULL;
char * first; /* ptr to first bad block */
extern SCSI_TUTYPE tucb;

/* set up command structures to be issued to drives */
/* this is the command structure for a reassign */
struct command tu_reas = {
	8, 120, 0, 6, {0x07, 0, 0, 0, 0, 0},8,{0,0,0,0x4,0,0,0,0}
};


/* this is the command structure to get the grown defect list */
struct command tu_bad = {
	512, 120, 1, 10, {0x37, 0, 0xd, 0, 0, 0, 0, 2, 0, 0},0,{0}
};


/* this is the structure to get the sense data from the drive */
struct command tu_sense = {
	28, 30, 1, 6, {0x3, 0, 0, 0, 0x1C, 0},0,{0}
};


extern int fdes;

/* ^L */
/*
 * NAME: dsp_bad_blks
 *
 * FUNCTION: find out if there are bad blocks and display them
 *
 * EXECUTION ENIVROMENT
 *
 * RETURNS:   
 *        	0 - for good completion
 *              NONE 0 - for a failure
 */

int dsp_bad_blks(char * dev_name)
{
	long tmp;
	int i,cdb;
	char file_name[50];
	int length;
	int count;
	extern uchar tu_buffer[MAX_SEND];


	/* issue the SCSI command to get the bad block list */
	initcmd(tu_bad);
	i = tu_test(fdes,SCATU_USER_DEFINED);

	/* if a check condition occurs, see if it is a unit attention and if so,
		try again */
	if(i == SCATU_CHECK_CONDITION)
	{
		/* issue a request sense and get the data */
		i = tu_test(fdes,SCATU_REQUEST_SENSE);
		if(i == 0)
		{
			/* if this is a 0x06 which is a unit attention, reissue the get 
				grown defect list command 
			*/
			if(tucb.scsiret.sense_key == 0x06)
			{
				initcmd(tu_bad);
				i = tu_test(fdes,SCATU_USER_DEFINED);
			}
			else if(tucb.scsiret.sense_key == 0x05)
			/* if 0x05 is the key, then this command is not supported by 
				the drive */
			{
				/* if command not supported, display menu and get a response as
					to whether you should continue */
				i = disp_menu(CMD_NOT_SUP);
				/* if the response is a yes to continue, return 0 otherwise,
					cleanup */
				if(i == 1)
				{
					return(0);
				}
				else
					clean_up();
			}
		}

	}

	if(i != 0)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/* call glist function which puts the data into a format which can be 
		displayed */
	glist(tucb.scsitu.data_buffer,tucb.scsitu.data_length);
	/* display the bad blocks */
	display_bad_blocks(first);

	free(tucb.scsitu.data_buffer);
	return(0);
} /* end of dsp_bad_blks(char * dev_name) */


/* ^L */
/*
 * NAME: initcmd 
 *
 * FUNCTION: Load tucb structure to pass as a user defined call to the TU's.
 *
 * EXECUTION ENIVROMENT
 *
 * RETURNS:
 *
 */
void initcmd(struct command tu)
{
	int i;

	tucb.scsitu.data_length = tu.data_length;

	/* set up buffer to the amount of data you need to get */
	tucb.scsitu.data_buffer = (char *) malloc(tucb.scsitu.data_length);
	if(tucb.scsitu.data_buffer == NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/* set up timeout and flags */
	tucb.scsitu.cmd_timeout = tu.timeout;    /* timeout in seconds */
	tucb.scsitu.flags = tu.flags;
	tucb.scsitu.command_length = tu.cmd_length;

	/* load the command data block with the command */

	memcpy(tucb.scsitu.scsi_cmd_blk,tu.cmd,tucb.scsitu.command_length);

	/* load the buffer with the data that the command requires */

	memcpy(tucb.scsitu.data_buffer,tu.data,tu.d_len);

} /* end of initcmd */

/* ^L */
/*
 * NAME: glist
 *
 * FUNCTION:  puts the bad block data into a format to be displayed 
 *
 * EXECUTION ENIVROMENT
 *
 * RETURNS:
 *
 */
void glist(char * data,int d_length)
{
	int i,j;
	int count=0;
	int length;
	int tmp;

	/* length of the bad block map */
	length = data[2] << 8 | data[3];
	if(length > d_length)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/* skip the header information and get the rest of the info. */
	for(i=4;i<length+4;i++)
	{
		/* start the linked list which holds the list of bad blocks*/
		if(list == (struct bad_block *)NULL)
		{
			list = (struct bad_block *) calloc(1,sizeof(struct bad_block) );
			/* keep track of the starting block */
			first = (char *) list;
		}
		else
		{
			/* add another structure to the list */
			list->next = (struct bad_block *)calloc(1,sizeof(struct bad_block));
			if(list->next == (struct bad_block *) NULL)
			{
				disp_menu(UNKNOWN_ERR);
				clean_up();
			}
			list = list->next;
		}

		/* set up total number of structures and point the next to NULL */
		list->total = length/8;
		list->next = NULL;

		/* keep a count of the number of structures */
		count ++;
		for(j=1,tmp=(data[i]<<16);j<3;j++)  /* byte 4 from defect data returned */
		{
			i++;
			switch (j)
			{
				case 1: 	/* byte 5 from defect data returned */
					tmp += (data[i]<<8);	
					break;
				case 2: 	/* byte 6 from defect data returned */
					tmp += data[i];
					break;
			}
		}

		/* 
		 * The first bit of data is the cylinder 
		 */
		list->cylinder = tmp;

		/* find the next piece of data and this is the head */
		i++;
		tmp = data[i]; 	/* byte 7 from defect data returned */
		list->head = tmp;
		i++;

		/* then the next useful data is the sector */
		for(j=1,tmp=(data[i]<<24);j<4;j++)	/* byte 8 from defect data returned */
		{
			i++;
			switch (j)
			{
				case 1: 	/* byte 9 from defect data returned */
					tmp += (data[i]<<16);	
					break;
				case 2: 	/* byte 10 from defect data returned */
					tmp += (data[i]<<8);	
					break;
				case 3: 	/* byte 11 from defect data returned */
					tmp += data[i];
					break;
			}
		}
		list->sector = tmp;
	}

} /* end of glist */
