/* @(#)74	1.2.2.5  src/bos/diag/util/uformat/uformat.h, dsauformat, bos411, 9428A410j 2/7/94 16:05:25 */
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: Header
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_UFORMAT
#define _H_UFORMAT

#include        "ufmt_msg.h"

/*
 * define the three possible operating modes
 */

#define UFORMAT  1
#define UCERTIFY 2
#define UDCLASS  3

#define CAB_MODEL 0x2010043


/*
 * common menu numbers
 */

#define MENU_ONE        1
#define MENU_TWO        2
#define MENU_THREE      3
#define MENU_FOUR       4
#define MENU_FIVE       5
#define MENU_SIX        6
#define MENU_SEVEN      7
#define MENU_EIGHT      8
#define MENU_NINE       9
#define MENU_TEN        10
#define MENU_ELEVIN     11
#define MENU_TWELVE     12
#define MENU_THIRTEEN   13

/* serial redwing LED number */
#define S_REDWING       0x870
#define	DIRECT_BUS_ATTACH 0x949

int             certify_after;  /* certify after format ( 0 or 1 )       */

/*
 * da_menu is a structure holds information related to display a menu or
 * message. Variables in this structure are explained the catalog message
 * server.
 *
 */

struct da_menu {
	char           *catfile;/* pointer to the catalog file name      */
	int             setnum; /* the set number in the catalog file    */
	int             msgnum; /* msg number in the set number  */
	int             menunum;/* menu number                           */
};

#define NO_PATTERNS (3)

struct data_dclass{
	int num; 	   	/* number of patterns to write */
	unsigned long pattern[NO_PATTERNS];		/* patterns */

	char rand;		/* write random pattern(y/n) */
};

struct read_blk{
	unsigned long lba;
	int how_many;
};


#define PATTERNS "ff,c0,a5,aa,00,a0"
#define NO_OF_WRITES "0,1,2,3"
#define YN "N,Y"
#define DEF_PATTERN "00"
#define BLKS_TO_READ "1,2,3,4,5,6,7,8,9,10"
#define ENTRIES 5
#define NAME_SIZE 128
#define DIFF_REP	(1.0) /* At what percentage does the appl. report progress*/
#define KEY (tu_buffer[2] & 0x0f)
#define CODE ((tu_buffer[12] << 8) | tu_buffer[13])
#define REG	1
#define DIAG 2
#define WRITE_ONCE 1
#define WRITE_COMPLEMENT 2

char * operation;

struct bad_block{
	int total;
	unsigned long cylinder;
	unsigned long head;
	unsigned long sector;
	struct bad_block * next;
};

void disp_percent_done(unsigned long,unsigned long);

/* MAX_SEND is the max. number of bytes to send.  In this case, its 128 blocks*/
#define MAX_SEND  128*512 

/* MENU_BASE is the first menu written */
#define MENU_BASE 0x0802540
/* define 1 GB */
#define ONE_GB    (1024*1024*1024)

#endif /* _H_UFORMAT */
