static char sccsid[] = "@(#) 93 1.25.1.6 src/bos/usr/lpp/bosinst/blvset/blvset.c, bosinst, bos411, 9433B411a 94/08/09 15:44:49";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: find_blv, read_menu, os_level
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:	blvset.c
 *
 * FUNCTION: 	This low level command places or retrieves system
 *		setting information in the second sector of the
 *		boot logical volume.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	The blvset program is a command that can be executed from
 *	a shell environment.
 *
 *	Flags:	-d device	Required - specifies on which device the
 *					boot logical volume resides.
 *			-- either --
 *		-p 		Required - puts the file information
 *					into the second sector of the blv.
 *			---- or ----
 *		-g 		Required - gets the information from the
 *					second sector of blv to stdout.
 *
 * Syntax: blvset -d device -p [menu|level]
 *         blvset -d device -g [menu|level]
 *
 * Changes: For release 3.2 the language and keyboard options were combined into
 *          one locale option, and the timezone field was droped, thus only five
 *          fields are put (-p) or retreived (-g).
 *          
 *
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/bootrecord.h>
#include "blvset.h"


#define USAGE "\nUsage:\t%s -d device -p [menu | level] | -g [menu | level]\n"
#define MENU 1
#define DATE 3
#define LEVEL 4
#define PAD_3_2 "3.2 pad string:~!~#$%@@%$#~!~" /* 29 char long pad string to */
                                                /* be placed in blvset.pad    */

#define PAD_4_1 "4.1 pad string:@%$#~!~~!~#$%@" /* 29 char long pad string to */
                                                /* be placed in blvset.pad    */

#define PAD_4_1_1 "4.1.1 pad str :@%$#~!~~!~#$%@" /* 29 char long pad string   */
                                                /* to be placed in blvset.pad */

unsigned int find_blv();		/* find blv start from ipl record */
int read_menu ();                       /* rean the menu block */
char * os_level ();                     /* get level of os */
int rc;					/* function call return code */
int fd;                                 /* file descriptor for physical vol */
int blv_start;				/* starting PSN of blv		*/
off_t offset;

/* options */
char    *bdevice;			/* boot device */
int	dataset;			/* data group id (MENU | LEVEL) */

/*
 *  main entry point.
 */
main (int argc, char *argv)
{
    FILE            *in_file;		/* stdin	*/
    FILE            *out_file;		/* stdout	*/
    struct blvset   blvset;		/* Information record for blv */
    int             flg, pflag, gflag, errflg, flgcnt;
    extern char     *optarg;
    char	    *execname;			/* name of file exec'ed */
    
    /*
     *
     * Interpret options and input files.
     *
     */
    execname = argv[0];
    flg = gflag = pflag = errflg = flgcnt = 0;

#ifdef _DEBUG
    fprintf(stderr, "	BLVSET: entering getopt loop: line %d\n",__LINE__);
#endif

    while ((flg = getopt(argc, argv, "d:g:p:")) != EOF)
    {

#ifdef _DEBUG
	fprintf(stderr, "	BLVSET: in getopt while loop: line %d\n",__LINE__);
#endif

	switch(flg)
	{
	    case 'd': 
	        bdevice = optarg;    /* device with blv */
		break;

	    case 'g': 
		/* If the options to -g were not either menu or level, flag */
		/* an error. */
		if(	strcmp(optarg,"menu") &&
			strcmp(optarg,"level") &&
			strcmp(optarg,"date")	)
		    errflg++;

		if(!strcmp(optarg,"menu"))
		    dataset = MENU;
		else if(!strcmp(optarg,"level"))
		    dataset = LEVEL;
		else if(!strcmp(optarg,"date"))
		    dataset = DATE;

		out_file = stdout;    /* get settings from blv */

		if (pflag)
		    errflg++;
		else
		    gflag++;

		break;
		
	    case 'p': 
		if(strcmp(optarg,"menu") && strcmp(optarg,"level"))
		    errflg++;

		if(!strcmp(optarg,"menu"))
		    dataset = MENU;
		else if(!strcmp(optarg,"level"))
		    dataset = LEVEL;

		in_file = stdin;     /* put settings into blv */

		if (gflag)
		    errflg++;
		else
		    pflag++;
		break;
		
	    case '?':
	    default:
		errflg++;
		break;
	    }
	flgcnt++;
    } /* while */

#ifdef _DEBUG
    fprintf(stderr, "	BLVSET: left getopt while loop: line %d\n",__LINE__);
    fprintf (stderr, "   BLVSET: dataset = %d\n", dataset);
#endif

    if (errflg || !flgcnt)
    {
	fprintf(stderr, USAGE, argv[0]);
	exit(1);
    }
	
    /*
     *
     * Open input file and read settings information into the blv
     * record. This is the -p option.
     *
     */
    if (pflag) 
    {
	switch (dataset)
	{
	    case MENU:
	    case LEVEL:

#ifdef _DEBUG
	    fprintf(stderr, "	BLVSET: putting menu: line %d\n",__LINE__);
#endif

		blvset.menu.locale[0] = '\0';
		blvset.menu.console[0] = '\0';
		blvset.menu.inst_dev[0] = '\0';
		blvset.menu.targ_dev[0] = '\0';
		blvset.menu.ipl_dev[0] = '\0';

		/* blvset.menu.locale will contain all four locale values. */
		fgets(blvset.menu.locale, RECSIZE, in_file);

		sprintf (blvset.menu.pad1,"%d", time((time_t )0));
		strcpy (blvset.menu.pad2, PAD_4_1_1);
		break;
	}
	fclose( in_file );	/* close stdin file	 */
    }
    
    /*
     *
     * Open disk device containing the blv record sector.
     *
     */

#ifdef _DEBUG
    fprintf(stderr, "	BLVSET: opening blv 'file': line %d\n",__LINE__);
#endif

    fd = open(bdevice, O_RDWR);
    if (fd < 0)
    {
	fprintf(stderr, "\nDevice open failure: %s\n", bdevice);
	exit(2);
    }
    /*
     *
     * Find and seek blv location on disk.
     *
     */
    switch (dataset)
    {
	case MENU:
	case LEVEL:
	case DATE:
	    blv_start = find_blv(fd) * UBSIZE;
	    break;
    }

#ifdef _DEBUG
    fprintf(stderr,"IPL record blv group %d start: %d\n", dataset, blv_start);
#endif

    offset = lseek(fd, blv_start, 0);
    if (offset < 0) 
    {
	fprintf(stderr, "\nSeek error!\n");
	exit(3);
    }

    /*
     *
     * Read current information from blv disk sector and direct
     * to stdout. This is the -g flag option.
     *
     */
    else if (gflag) 
    {

#ifdef _DEBUG
	fprintf(stderr, "	BLVSET: getting: line %d\n",__LINE__);
#endif

	switch (dataset)
	{
	    case MENU:
	    case DATE:
	        rc = read_menu (fd, &blvset.menu);
		if (rc < 0)
		{
		    fprintf(stderr, "\nBLV record read failed! %d\n", fd);
		    exit(4);
		}
		else
		{
		    if (dataset == MENU)
		    {
			fputs(blvset.menu.locale, out_file);
			fputs(blvset.menu.console, out_file);
			fputs(blvset.menu.inst_dev, out_file);
			fputs(blvset.menu.targ_dev, out_file);
		    }
		    else
		    {
			time_t epoch_time;
			epoch_time = atoi(blvset.menu.pad1);
			fprintf(out_file,"%s", ctime(&epoch_time));
		    }
		}
		break;
    	    /* Return level of OS on the system. */
	    case LEVEL:
		printf("%s\n", os_level(fd) );
		break;
	}

	fclose( out_file );	/* close stdout file	 */
    }
    /*
     *
     * Write blv record to blv first sector. This is the -p flag option.
     *
     */
    else
    {

#ifdef _DEBUG
	fprintf(stderr, "	BLVSET: putting: line %d\n",__LINE__);
	fprintf(stderr, "BLVSET: pad2: %s", blvset.menu.pad2);
#endif

	switch (dataset)
	{
	    case MENU:
	    case LEVEL:
	        rc = write(fd, (char *) &blvset.menu, sizeof(struct menu));
		break;
		
	}

	if (rc < 0)
	{
	    fprintf(stderr, "\nBLV record write failed! %d\n", fd);
	    exit(5);
	}

#ifdef _DEBUG
	fprintf(stderr,"blv record %s section written at: %d, returned %d\n",
		dataset == MENU ? "menu" : "network", blv_start,rc);
#endif

    }
    close(fd);		/* close blv disk device */
    exit(0);
}

/*****************************************************************************/

unsigned int find_blv( int fd )
{
    int rc;
    unsigned int offset;
    IPL_REC ipl_rec;		/* ROS IPL record */
    
    offset = lseek(fd, 0, 0); 	/* find ipl record */

#ifdef _DEBUG
    fprintf(stderr, "	BLVSET: in find_blv: line %d\n",__LINE__);
#endif

    if (offset < 0) 
    {
	fprintf(stderr, "\nSeek error!\n");
	exit(6);
    }

    rc = read(fd, (char *) &ipl_rec, sizeof(IPL_REC));
    if (rc < 0) 
    {
	fprintf(stderr, "\nIPL record read failed! %d\n", fd);
	exit(7);
    }
    if (ipl_rec.IPL_record_id != IPLRECID) 
    {
      fprintf(stderr, "\nIPL record read failed! %d\n", fd);
      exit(8);
    }
    
    /*
     * Return location of the blv second sector.
     */
    offset = ipl_rec.boot_lv_start + 1;

#ifdef _DEBUG
    fprintf(stderr,"BLV PSN start: %d\n", offset);
#endif

    return(offset);
}

/*****************************************************************************/

int read_menu (int file, struct menu *menu_ptr)
{
    char   buff [sizeof (struct menu)];
    char   *index_ptr;
    int    rc;

    rc = read(file, (char *) buff, sizeof(struct menu));
    if (rc >= 0 )
    {
	int count;
	index_ptr = buff;
	memcpy (menu_ptr->locale, index_ptr, RECSIZE);
	
	/* 
	 * If the last RECSIZE bytes of buff contain
	 * the PAD_3_2 string then we have read a 3.2 blvset.menu
	 * block else this is a 3.1 blvset.menu block.
	 * In the latter case we must ignore the obsolete
	 * keyboard and time zone fields (see blvset.h).
	 */

	if ( (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_3_2) != 0) &&
	     (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_4_1) != 0) &&
	     (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_4_1_1) != 0)    )
	    index_ptr += RECSIZE * 2;         /* skip the 3.1 keyboard record */
	else
	    index_ptr += RECSIZE;             /* this is 3.2, 4.1 or 4.1.1 */

	memcpy (menu_ptr->console, index_ptr, RECSIZE);
	index_ptr += RECSIZE;
	memcpy (menu_ptr->inst_dev, index_ptr, RECSIZE);
	index_ptr += RECSIZE;
	memcpy (menu_ptr->targ_dev, index_ptr, RECSIZE);
	index_ptr += RECSIZE;
	memcpy (menu_ptr->ipl_dev, index_ptr, BIGRECSIZE);
	index_ptr += BIGRECSIZE;
	index_ptr = buff + sizeof(struct menu) - (RECSIZE * 2) ;
	memcpy (menu_ptr->pad1, index_ptr, RECSIZE);
	memcpy (menu_ptr->pad2, PAD_4_1_1, RECSIZE); /* last field no longer used */
    }
    return (rc);
}

/*
 * os_level:  returns the level of OS that was installed previously.
 */
char *os_level ( int file )
{
    char   buff [sizeof (struct menu)];
    char * level;
    int count, rc;
    off_t offset;

    rc = read(file, (char *) buff, sizeof(struct menu));
    if (rc >= 0 )
    {
	/* 
	 * If the last RECSIZE bytes of buff contain
	 * the PAD_4_1_1 string then we have read a 4.1 blvset.menu block.
	 * If the last RECSIZE bytes of buff contain
	 * the PAD_4_1 string then we have read a 4.1 blvset.menu block.
	 * If the last RECSIZE bytes of buff contain
	 * the PAD_3_2 string then we have read a 3.2 blvset.menu block.
	 * Else this is a 3.1 blvset.menu block.
	 */

#ifdef _DEBUG
	fprintf(stderr, "	BLVSET: get level: line %d\n",__LINE__);
	fprintf(stderr, "BLVSET: pad2: %s", &buff[sizeof(struct menu) - RECSIZE]);
#endif
	if (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_4_1_1) == 0)
	    level = "4.1.1";
	else if (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_4_1) == 0)
	    level = "4.1";
	else if (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_3_2) == 0)
	    level = "3.2";
	else
	{
	/* 
	 * If the PAD string is not 3.2 or 4.1, we need to check at offset 512
	 * from the beginning of the disk, since the data was sometimes saved
	 * there in 3.2 instead of at offset 512 from the beginning of the blv.
	 */
	    offset = lseek(fd, UBSIZE, 0);
	    if (offset < 0) 
	    {
		fprintf(stderr, "\nSeek error!\n");
		exit(3);
	    }
	    else
	    {
		rc = read(file, (char *) buff, sizeof(struct menu));
		if (rc >= 0 )
		{
		/* 
		 * If the last RECSIZE bytes of buff contain the
		 * PAD_4_1_1 string then we have read a 4.1.1 blvset.menu block.
		 * If the last RECSIZE bytes of buff contain
		 * the PAD_4_1 string then we have read a 4.1 blvset.menu block.
		 * If the last RECSIZE bytes of buff contain
		 * the PAD_3_2 string then we have read a 3.2 blvset.menu block.
		 * Else this is a 3.1 blvset.menu block.
		 */
		if (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_4_1_1) 
									== 0)
		    level = "4.1.1";
		else
		if (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_4_1) == 0)
		    level = "4.1";
		else
		if (strcmp (&buff[sizeof(struct menu) - RECSIZE], PAD_3_2) == 0)
		    level = "3.2";
		else
		{
		    char sysbuf[80];
		    sprintf(sysbuf,"lqueryvg -p %s -L | grep hd9var >&- 2>&-",
					bdevice);

		    if (system(sysbuf) == 0)
			    level = "3.2";
		    else
			    level = "3.1";
		}
		}
	    }
	}
    }

    return ( level );
}
