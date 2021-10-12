static char sccsid[] = "@(#)64	1.7  src/bos/diag/util/uformat/udclass.c, dsauformat, bos41J, 9511A_all 3/10/95 18:02:49";
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: data_rand
 *		dclass_info
 *		disp_percent_done
 *		disp_rd_buf
 *		erase
 *		int_hand
 *		read_blks
 *		write_dsk
 *		write_patt
 *		write_rand
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
#include <stdlib.h>
#include <sys/devinfo.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include<sys/scsi.h>
#include<sys/scdisk.h>

#include <diag/diag.h>
#include <diag/scsi_atu.h>
#include "diag/da.h"
#include "diag/da_rc.h"
#include "diag/dcda_msg.h"
#include "diag/diag.h"
#include "diag/diag_exit.h"
#include "diag/diago.h" 
#include "diag/dascsi.h"
#include "sys/cfgodm.h"

#include "uformat.h"
#include "ufmt_msg.h"

/* these external declarations are to tell the compiler that these
 * variables have been declared outside of this module.
 */
extern int disp_menu();
extern void clean_up();
extern DRIVE_INFO drive_info;
extern int errno;
extern int is_working;
extern char * operation;
extern SCSI_TUTYPE tucb;
extern struct da_menu dmnu;

static int pass;
nl_catd cdep_catd;
/* these external declarations are to tell the compiler that these
 * variables have been declared outside of this module.
 */
extern nl_catd diag_catopen(char *name, int oflag);
extern int fdes;
extern unsigned int maxblock;
extern unsigned int blocklength;
extern uchar tu_buffer[MAX_SEND];


int disp_rd_buf(int );
int write_rand(int ,long );
int write_dsk(int , long );
void erase(char *);

static char * buf_wr;
static long b_rd_wr;
static char * buf_rd;
static struct devinfo info;	/* device information from IOCINFO ioctl */
static long size_of_drive;

/*
 * initialize data structures where structures are as in uformat.h
 *
 */

struct data_dclass dclass_data = {
	0,{0xffffffff,0x00000000,0xa5},'N'
};


struct read_blk read_data = {
	0,1
};


/* ^L */
/*
 * NAME: dclass_info
 *
 * FUNCTION:  Get information on what patterns to write to disk.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *		0 - for good completion
 *		-1 - For a failure to read blocks
 */

int dclass_info(char * dev_name)
{
	int rc;					/* return code */
	int count;				/* counter */
	char dev_path[50];
	int nbytes;				/* number of bytes to write */
	int i,j;

	cdep_catd = diag_catopen(dmnu.catfile, 0);

	drive_info.percent_complete = 0;
	pass = 0;
	operation = (char *) malloc(NAME_SIZE);
	if(operation == (char *) NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}


	/* 
	 * display list of bad blocks
	 */
	rc = dsp_bad_blks(dev_name);
	if(rc != 0)
	{
		clean_up();
	}


	/* 
	 * display warning
	 */
	rc = disp_menu(D_CLASS_DISK);
	if(rc == 2)
	{
		clean_up();
	}

	/* 
	 * get size of disk 
	 */
	rc = tu_test(fdes,SCATU_READ_CAPACITY);
	if(rc != 0)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/* get the size of the drive in blocks */
	maxblock = (unsigned) tu_buffer[0] << 24 |
	    (unsigned) tu_buffer[1] << 16 |
	    (unsigned) tu_buffer[2] << 8 |
	    (unsigned) tu_buffer[3];

	blocklength = (unsigned) tu_buffer[4] << 24 |
	    (unsigned) tu_buffer[5] << 16 |
	    (unsigned) tu_buffer[6] << 8 |
	    (unsigned) tu_buffer[7];

	size_of_drive = maxblock + 1;

	/* nbytes is the amount of data which can be passed to the drive */
	nbytes = MAX_SEND;

	buf_wr = (char *) malloc((size_t)nbytes);
	buf_rd = (char *) malloc((size_t)nbytes);

	if(buf_wr == (char *) NULL || buf_rd == (char *) NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	memset(buf_wr,1,MAX_SEND);
	memset(buf_rd,0,MAX_SEND);

	/* read data or write pattern */
	rc = disp_menu(MENU_READ_OR_WRITE);
        if (rc == 1)  /* read data */
 	{
		rc = disp_read_info();
		if(rc != DIAG_ASL_COMMIT)
			clean_up();
		/* Read data */
		rc = read_blks(dev_name,read_data.how_many,read_data.lba,25*1024);
		if(rc != 0)
		{
			disp_menu(MENU_TERMINATED);
			clean_up();
		}
	}
	else if (rc == 2) /* write pattern */
	{
		rc = display_pattern();
		if(rc != DIAG_ASL_COMMIT)
			clean_up();
	}
	else 
		clean_up();

	return(0);
} /* end of dclass_info */

/* ^L */
/*
 * NAME: write_patt
 *
 * FUNCTION:  write patterns to disk
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *		0 - for good completion
 *		-1 - For a failure to read blocks
 */
int write_patt(char * dev_name)
{
	int i,j,k,length;
	int shift;
	unsigned long data_pattern;
	long request;
	char patt[8];
	int rc;
	/* these external declarations are to tell the compiler that these
	 * variables have been declared outside of this module.
	 */
	extern uchar tu_buffer[MAX_SEND];/* buffer for write and request sense   128 blocks */

	is_working = 1;

	/* write number of pattern time to disk */
	for(i=0;i<dclass_data.num;i++)
	{
		data_pattern = dclass_data.pattern[i];

		for(j=0;j<WRITE_ONCE;j++)
		{
			pass ++;

			sprintf(operation, 
			    (char *) diag_cat_gets(cdep_catd,DUFORMAT_SET,PATTERN), 
			    pass,(i+1) );


			drive_info.percent_complete = 0;

			/* put data_pattern into a string so that it can be broken down
				 * into 1 byte increments.
				 */
			sprintf(patt,"%x",data_pattern);

			length = strlen(patt)/2;

			if(length >= 1)
				shift = length-1;
			else 
				shift = 0;

			request = MAX_SEND;

			for(k=0;k<request;k++)
			{
				/* this puts one byte of the pattern into the buffer at
					 * a time.  If the length of the pattern is 1, then there
					 * is no shift.  If the pattern is 2 bytes, then to get the
					 * higher order byte first, the pattern must be right 
					 * shifted by 1 byte.  This can be extrapolated to patterns
					 * of more than 2 bytes.
					 */
				buf_wr[k] = data_pattern >> (shift * 8);
				if(length > 0)
				{
					if(shift == 0)
					{
						shift = length-1;
					}
					else
						shift --;
				}
				else 
					shift = 0;
			}

			/* copy data pattern to tu_buffer for scsi write*/
			memcpy(tu_buffer,buf_wr,MAX_SEND);


			rc=write_dsk(fdes,size_of_drive);
			if (rc)
			{
				/* 
					 * display screen that says write failed
					 */
				disp_menu(MENU_WRITE_FAILED);
                                clean_up();

			}


		}

	} /* End of for loop */

	is_working = 0;
	return(0);

} /*write_patt */

/* ^L */
/*
 * NAME: data_rand
 *
 * FUNCTION:  write random patterns to disk
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *		0 - for good completion
 *		-1 - For a failure to read blocks
 */

int data_rand(char * dev_name)
{

	int rc;

	pass ++;
	sprintf(operation,
	    (char *) diag_cat_gets(cdep_catd,DUFORMAT_SET,RANDOM), pass);
	drive_info.percent_complete = 0;

	/*
	 * write random data to drive
	 */
	if(dclass_data.rand == 'Y')
	{
		rc=write_rand(fdes,size_of_drive);
		if(rc)
		{
			/* put up menu that says this op. failed */
			disp_menu(MENU_WRITE_FAILED);
                        clean_up();
		}
	}

	return(0);

} /* data_rand */



/* ^L */
/*
 * NAME: write_dsk
 *
 * FUNCTION: Write pattern to whole disk
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *		0 - for good completion
 *		-1 - For a failure to read blocks
 */
int write_dsk(int fdes, long blocks)
{
	unsigned long nblocks;
	int rc;
	int size;
	int write_type;
	short reassign_block;
 	short error_type;
	short recovery_count;

	/* these external declarations are to tell the compiler that these
	 * variables have been declared outside of this module.
	 */
	extern int curblock;
	extern int maxbyte;

	nblocks = MAX_SEND/512;
	size = blocks;
	drive_info.percent_complete = 0;
	b_rd_wr = 0;
	curblock = 0;

	/* if the size of the drive is greater than 1 GB */
	/* size is in blocks so ONE_GB is divided by 512 */
	if (size >= ONE_GB/512)
		write_type=SCATU_WRITE_EXTENDED;
	else 
		write_type=SCATU_WRITE;

	while(blocks > 0)
	{

		if(nblocks > blocks)
			nblocks = blocks;

		/* number of blocks remaining is the number of (blocks minus bytes 
		 * written divided by 512) 
		 */
		blocks -= nblocks;
		maxbyte = nblocks*512;

		rc = tu_test(fdes,write_type);

		/* the next block to write to is calculated by adding the number of
		 * blocks(=number of bytes written/512) written.
		 */
		curblock += nblocks;

		if (rc != 0)
		{
			if (rc == SCATU_CHECK_CONDITION){
				rc=tu_test(fdes, SCATU_REQUEST_SENSE);
		        	recovery_count = ( (((unsigned)tu_buffer[16]) << 8) +
			       	        ((unsigned)tu_buffer[17]) );
				if(rc == 0){
       					reassign_block=0;
       					error_type=0;
					process_sense_data(
				    	tucb.scsiret.sense_key & 0x0f,
				    	tucb.scsiret.sense_code, recovery_count,
				   	 &reassign_block, &error_type);

					if (reassign_block) 
						error_type=UNRECOVERED_DATA_ERROR;
					/* Update count of errors based on error type */
					switch(error_type){
					case RECOVERED_EQUIP_ERROR:
						++drive_info.rec_equ_check_errors;
						break;
					case RECOVERED_DATA_ERROR:
						++drive_info.rec_data_errors;
 						if (drive_info.rec_data_errors > 
                                                    (5*(maxblock*blocklength)/1000000)) {
							disp_menu(DATA_ERR);
							clean_up();
  						}
						break;
					case UNRECOVERED_EQUIP_ERROR:
						++drive_info.unrec_equ_check_errors;
						if(drive_info.unrec_equ_check_errors > 0) {
							disp_menu(EQUIP_ERR);
							clean_up();
						}
						break;
					case UNRECOVERED_DATA_ERROR:
						++drive_info.unrec_data_errors;
			    			if (drive_info.unrec_data_errors > 0) {
							disp_menu(DATA_ERR);
							clean_up();
						}
						break;
					}
					b_rd_wr += nblocks; /* blocks read or written */

					disp_percent_done(b_rd_wr, size);
				}
				else {  /* scatu request sense failed */
                                	return(-1);     
				}
			}
			else {  /* scatu write failed */
 				return(-1);
  			}
		}
		else
		{
			b_rd_wr += nblocks; /* blocks read or written */

			disp_percent_done(b_rd_wr, size);
		}

	}

	return(0);
} /* write_dsk */


/* ^L */
/*
 * NAME: write_rand
 *
 * FUNCTION: Write random data over the whole disk.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *		0 - for good completion
 *		-1 - For a failure to read blocks
 */
int write_rand(int fdes,long blocks)
{
	unsigned long nblocks;
	int seed;
	char * mem;
	int i,j;
	long numb;
	int size;
	int write_type;
        short reassign_block;
        short error_type;
	short recovery_count=0;

	/* these external declarations are to tell the compiler that these
	 * variables have been declared outside of this module.
	 */
	extern uchar tu_buffer[MAX_SEND];
	extern int curblock;
	extern int maxbyte;

	/*
	 * Set percentage of drive written to 0
	 */
	drive_info.percent_complete = 0;
	b_rd_wr = 0;
	curblock = 0;
	is_working = 1;

	/* number of blocks which can be passed to the drive */
	/* This is the MAX_SEND/512 */
	nblocks = MAX_SEND/512;

	/*
	 * get memory
	 */
	mem = (char *) malloc(MAX_SEND + 23);
	if(mem == (char *)NULL)
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/*
	 * set random seed
	 */
	srandom(getpid());

	/* set size to number of blocks on drive */
	size = blocks;

	/* if size > 1 GB then use write_extended command */
	/* ONE_GB is divided by 512 because size is in blocks */
	if (size >= ONE_GB/512)
		write_type=SCATU_WRITE_EXTENDED;
	else 
		write_type=SCATU_WRITE;

	/* while there are bytes left to write write data */
	while(blocks > 0)
	{
		if(nblocks > blocks)
			nblocks = blocks;

		/* number of blocks remaining is reduced by the number of blocks written */
		blocks -= nblocks;
		maxbyte = nblocks*512;


		/* get random numbers to write to drive */
		for(i=0;i<nblocks*512-3;i+=4)
		{
			numb = random() & 0xff;
			for(j=0;j<4;j++)
			{
				mem[j+i] = numb & 0xff;
				numb = numb >> 1;
			}


		}

		/* set up buffer and write data to drive */
		memcpy(tu_buffer,mem,MAX_SEND);
		j = tu_test(fdes, write_type);

		/* adjust curblock to indicate that data has been written */
		curblock += nblocks;

		if(j != 0)
		{
			if (j == SCATU_CHECK_CONDITION){
				j=tu_test(fdes, SCATU_REQUEST_SENSE);
		        	recovery_count = ( (((unsigned)tu_buffer[16]) << 8) +
			       	        ((unsigned)tu_buffer[17]) );
				if(j == 0){
       					reassign_block=0;
       					error_type=0;
					process_sense_data(
				    	tucb.scsiret.sense_key & 0x0f,
				    	tucb.scsiret.sense_code, recovery_count,
				   	 &reassign_block, &error_type);

					if (reassign_block) 
						error_type=UNRECOVERED_DATA_ERROR;
					/* Update count of errors based on error type */
					switch(error_type){
					case RECOVERED_EQUIP_ERROR:
						++drive_info.rec_equ_check_errors;
						break;
					case RECOVERED_DATA_ERROR:
						++drive_info.rec_data_errors;
 						if (drive_info.rec_data_errors > 
                                                    (5*(maxblock*blocklength)/1000000)) {
                                                	disp_menu(DATA_ERR);
							clean_up();
						}
						break;
					case UNRECOVERED_EQUIP_ERROR:
						++drive_info.unrec_equ_check_errors;
						if(drive_info.unrec_equ_check_errors > 0) {
                                                	disp_menu(EQUIP_ERR);
							clean_up();
						}	
						break;
					case UNRECOVERED_DATA_ERROR:
						++drive_info.unrec_data_errors;
			    			if (drive_info.unrec_data_errors > 0) {
                                                	disp_menu(DATA_ERR);
							clean_up();
						}	
						break;
					}
					b_rd_wr += nblocks; /* blocks read or written */

					disp_percent_done(b_rd_wr, size);
				}
				else {  /* scatu request sense failed */
                                	return(-1);     
				}
			}
			else {  /* sactu write failed */
                          	return(-1);
			}
		}
		else
		{
			/* b_rd_wr is the number of blocks written */
			b_rd_wr += nblocks;

			disp_percent_done(b_rd_wr,size);

		}


		fflush(stdout);

	}

	free(mem);
	is_working = 0;
	return(0);

} /* write_rand */

/* ^L */
/*
 * NAME: read_blks
 *
 * FUNCTION: Read a certain number of blocks starting at a specific location.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *		0 - for good completion
 *		-1 - For a failure to read blocks
 */

int read_blks(char * dev_name,int num_of_blocks, int start,int max_request)
{
	int nblocks;
	int blocks;
	int rc;
	int rc1;
   	short reassign_block;
	short error_type;
	short recovery_count=0;

	/* these external declarations are to tell the compiler that these
	 * variables have been declared outside of this module.
	 */
	extern uchar tu_buffer[MAX_SEND];/* buffer for write and request sense  */
	extern int curblock;
	extern int maxbyte;

	/* set up variables for number of bytes to read/offset/starting block */
	nblocks = max_request/512;
	blocks = num_of_blocks;
	curblock = start;

	/* if starting block is past the end of the disk, put up menu */
	if(start > size_of_drive)
	{
		disp_menu(NO_SUCH_BLOCK);

		return(0);
	}

	/* if ending block is past end of disk only put up as much data as can
	 * be obtained 
	 */
	if((start + num_of_blocks) > size_of_drive)
		blocks = size_of_drive - start;


	/* read in all of the blocks requested */
	while(blocks > 0)
	{

		rc1 = 0;
		/* bytes to read is calculated by taking the number of blocks left times
		 * 512 
		 */
		if(nblocks > blocks)
			nblocks = blocks;

		/* remaining blocks are calculated by subtracting from previous number 
		 * of blocks to read.
		 */
		blocks -= nblocks;
		maxbyte = nblocks*512;

		/* malloc enough memory to hold data that is read in */
		buf_rd = (char *)malloc(MAX_SEND + 1);
		if(buf_rd == (char *) NULL)
		{
			disp_menu(UNKNOWN_ERR);
			clean_up();
		}
		rc = tu_test(fdes,SCATU_READ_EXTENDED);
		memcpy(buf_rd,tu_buffer,maxbyte);

		/*increment curblock to the starting block plus number of blocks read */
		curblock += nblocks;

		if(rc != 0)
		{
			if (rc == SCATU_CHECK_CONDITION){
				rc=tu_test(fdes, SCATU_REQUEST_SENSE);
		        	recovery_count = ( (((unsigned)tu_buffer[16]) << 8) +
			       	        ((unsigned)tu_buffer[17]) );
				if(rc == 0){
       					reassign_block=0;
       					error_type=0;
					process_sense_data(
				    	tucb.scsiret.sense_key & 0x0f,
				    	tucb.scsiret.sense_code, recovery_count,
				   	 &reassign_block, &error_type);

					if (reassign_block) 
						error_type=UNRECOVERED_DATA_ERROR;
					/* Update count of errors based on error type */
					switch(error_type){
					case RECOVERED_EQUIP_ERROR:
						++drive_info.rec_equ_check_errors;
						break;
					case RECOVERED_DATA_ERROR:
						++drive_info.rec_data_errors;
 						if (drive_info.rec_data_errors > 
                                                    (5*(maxblock*blocklength)/1000000)) {
							disp_menu(DATA_ERR);
							clean_up();
						}
						break;
					case UNRECOVERED_EQUIP_ERROR:
						++drive_info.unrec_equ_check_errors;
						if(drive_info.unrec_equ_check_errors > 0) {
							disp_menu(EQUIP_ERR);
							clean_up();
						}
						break;
					case UNRECOVERED_DATA_ERROR:
						++drive_info.unrec_data_errors;
			    			if (drive_info.unrec_data_errors > 0) {
							disp_menu(DATA_ERR);
							clean_up();
						}
						break;
					}
				}
				else {  /* scatu request sense failed */
 					disp_menu(MENU_WRITE_FAILED);
                               	 	clean_up();
				}
			}
			else {  /* scatu read failed */
 				disp_menu(MENU_WRITE_FAILED);
                                clean_up();
			}
		}

		disp_rd_buf(num_of_blocks);

		free(buf_rd);

	} /* End of while loop */


	return(0);
} /* read_blks */

/* ^L */
/*
 * NAME: disp_percent_done
 *
 * FUNCTION: Display the percent of the operation that has completed
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */
void
disp_percent_done(unsigned long b_rd_wr,unsigned long data_to_process)
{
	double percent_done;

	percent_done = b_rd_wr;
	percent_done /= data_to_process;
	percent_done *= 100;

	if( (percent_done - drive_info.percent_complete) > DIFF_REP)
	{
		drive_info.percent_complete = percent_done;
		disp_menu(CDEP_PERCENT_DONE);
	}

} /* disp_percent_done */

/* ^L */
/*
 * NAME: int_hand
 *
 * FUNCTION: interrupt handler cleans up following an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */

void
int_hand()
{
	clean_up();
} /* int_hand */


/**/
/*
 * NAME: disp_rd_buf()
 *                                                                    
 * FUNCTION:  Display's menu with read buffer
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 *	
 */
int disp_rd_buf(int blks)
{
	int i;
	int count,count2;
	ASL_SCR_INFO * menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
	int ind = 1;


	/* allocate memory for the the menu */
	menuinfo = (ASL_SCR_INFO *) calloc(ind+2 ,sizeof(ASL_SCR_INFO) );

	if(menuinfo == (ASL_SCR_INFO *) NULL )
	{
		disp_menu(UNKNOWN_ERR);
		clean_up();
	}

	/* get the message from the catalog */
	menuinfo[0].text = (char *)diag_cat_gets(cdep_catd,DUFORMAT_SET,MENU_READ_BLK);
	for( i=0 ; i < ind;i++)
	{

		menuinfo[i+1].text = (char *) malloc(blks * 512 * 2 + 10);
		if(menuinfo[i+1].text == (char *)NULL)
		{
			disp_menu(UNKNOWN_ERR);
			clean_up();
		}

		for(count = 0;count < blks*512;count++)
			sprintf(&menuinfo[i+1].text[count*2],"%.2x",buf_rd[count]);
	}

	menutype.max_index = ind + 1 ;

	/* display menu */
	diag_display(MENU_BASE + 0x31,0,NULL,DIAG_IO,ASL_DIAG_KEYS_ENTER_SC, &menutype,menuinfo);


	free(menuinfo);

}

/**/
/*
 * NAME: erase()
 *                                                                    
 * FUNCTION:  Erase function.  Determines order functions are called
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 *	
 */
void erase(char * dev_name)
{
	int num_patt;
	int rc;
	/* these external declarations are to tell the compiler that these
	 * variables have been declared outside of this module.
	 */
	extern struct CuDv    *cudv_selected;
	extern int pvid_destroyed;


	/* get information from user as to what patterns to write and/or how
	 * much data to read.
	 */

	rc =  dclass_info(dev_name);
	if(rc != 0)
	{
		disp_menu(MENU_TERMINATED);
	}

	/* This is so we can tell later whether any patterns were supposed to
		have been written.
	*/
	num_patt = dclass_data.num;
	/* 
	 * if data patterns to write is > 0 then do write 
	 */
	if(dclass_data.num > 0)
	{
		rc = write_patt(dev_name);
		if(rc != 0)
		{
			disp_menu(MENU_TERMINATED);
			clean_up();
		}
		else
			pvid_destroyed = 1;

	}

	/* write the random data if it has been requested to do so. */
	rc = data_rand(dev_name);
	if(rc != 0)
	{
		disp_menu(MENU_TERMINATED);
		clean_up();
	}
	else	
		pvid_destroyed = 1;


	/* suggest that the drive be re-formatted */
	if(dclass_data.rand == 'Y' || num_patt > 0)
		disp_menu(MENU_SUGGEST_FORMAT);

} /* erase() */
