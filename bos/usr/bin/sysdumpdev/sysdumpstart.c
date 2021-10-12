static char sccsid[] = "@(#)70  1.4  src/bos/usr/bin/sysdumpdev/sysdumpstart.c, cmddump, bos411, 9428A410j 1/21/94 09:08:00";

/*
 * COMPONENT_NAME: CMDDUMP    system dump control and formatting
 *
 * FUNCTIONS: main for sysdumpstart
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * sysdumpstart command:
 *
 *	sysdumpstart -p
 *	sysdumpstart -s
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/err_rec.h>
#include <sys/dump.h> 
#include <sys/ioctl.h>
#include <sys/cfgodm.h>
#include <sys/errids.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <ras.h>
#include <cmddump_msg.h>
#include <errlg/SWservAt.h>

#define MCS_CATALOG "cmddump.cat"


#include <sys/devinfo.h>

#define VCPY(from,to_array) \
	strncpy(to_array,from,sizeof(to_array)); \
	to_array[sizeof(to_array)-1] = '\0';

extern optind;
extern char *optarg;

static int Dmpfd = -1;
static char *Pdevname = NULL;  /* pointer to the string that contain the current prim dump device */
static char *Sdevname = NULL;  /* pointer to the string that contain the current sec dump device */
static char *tPdevname = NULL; /* pointer to the string that contains the default prim dump device */
static char *tSdevname = NULL; /* pointer to the string that contains the default sec dump device */
static char *remote_fn = NULL;   /* pointer to the string of the remote filename*/
static char *Sdevarg = NULL;   /* pointer to the string of the input secondary dump device */

static char *Sysdump = "/dev/sysdump";
static char *SysNull = "/dev/sysdumpnull";

static int primaryflg = 0;
static int secondaryflg = 0;
static int forceflg = 0;
static int quietflg = 0;	/* -q don't print values after init by odm */

struct od_data {
    char *attr_name;
    struct SWservAt *swservp;
};

struct od_data od_list[] = {
#define TPRIMARY 0
    { "tprimary", NULL },
#define TSECONDARY 1
    { "tsecondary", NULL },
#define PRIMARY 2
    { "primary", NULL },
#define SECONDARY 3
    { "secondary", NULL } 
#define OD_LIST 4
};


static sysdumpstart();
static odminit();
static odmread();
static usage_start();

/*
 * NAME:     main
 * FUNCTION: Command line interface to sysdumpstart
 * INPUTS:   argc, argv
 * RETURNS:  None (exits)
 *
 */
main(argc,argv)
char *argv[];
{

	setlocale(LC_ALL,"");				/* set Locale */
	setprogname();					/* set Progname */
	catinit(MCS_CATALOG);			/* init message catalog */
	sysdumpstart(argc,argv);	/* no return */
}


static 
odminit()
{
    if (0 > odm_initialize()) {
	cat_fatal(CAT_ODMINIT, "Cannot initialize ODM.\n\t%s", errstr());
	exit(-1);
    }
}

static 
odmread()
{
    struct SWservAt *swservp;
    int i, cnt;

    for (i = 0; i < OD_LIST; i++) {
	swservp = (struct SWservAt *)ras_getattr(od_list[i].attr_name, 0, &cnt);
	Debug("odmread:[SWservAt] attr='%s' value='%s'\n", swservp->attribute,
	    swservp->value);
	if (!cnt) continue;
	if (PRIMARY == i)
	    Pdevname = stracpy(swservp->value);
	else if (SECONDARY == i)
	    Sdevname = stracpy(swservp->value);
	else if (TPRIMARY == i)
	    tPdevname = stracpy(swservp->value);
	else if (TSECONDARY == i)
	    tSdevname = stracpy(swservp->value);
	od_list[i].swservp = swservp;
    }
    if ((Pdevname == 0) && !quietflg)
	cat_eprint(CAT_ODM_PRIM,
	            "Cannot read primary dump device from ODM object class SWservAt.\n");
    if ((Sdevname == 0) && !quietflg)
	cat_eprint(CAT_ODM_SEC,
	            "Cannot read secondary dump device from ODM object class SWservAt.\n");
}

/*
 * NAME:     sysdumpstart
 * FUNCTION: start dump to primary or secondary device
 * INPUTS:   Command line argc and argv
 * RETURNS:  None (exits)
 *
 */
static 
sysdumpstart(argc,argv)
char *argv[];
{
	int c;
	char *primarydev;
	char *secondarydev;
	struct devinfo devinfo;
	struct dumpinfo dumpinfo;
        char   t[100];

	while((c = getopt(argc,argv,"psf")) != EOF) {
		switch(c) {
		case 'p':
			primaryflg++;
			break;
		case 's':
			secondaryflg++;
			break;
		case 'f':
			forceflg++;
			break;
		case 'H':
		default:
			usage_start();
		}
	}
	if(optind < argc)		/* no additional args allowed */
		usage_start();
	if(primaryflg && secondaryflg) {
		cat_eprint(CAT_DST_BOTH,
			"Cannot start a dump to both primary and secondary devices\n");
		usage_start();
	}
	/*
 	 * Open /dev/sysdump device.
 	 */
	if((Dmpfd = open(Sysdump,0)) < 0)
		cat_fatal(CAT_SYSDUMP,
			"Cannot open dump device %s.\n    %s",Sysdump,errstr());
	odminit();
	odmread();
	/*
 	 * Get the names of the current dump devices.
	 * They will be used in error messages.
 	 */
	if(ioctl(Dmpfd,IOCINFO,&devinfo) < 0)
		cat_fatal(CAT_IOCINFO,
		"Cannot read dump device information from %s.\n\t%s",Sysdump,errstr());
	primarydev   = Pdevname;
	secondarydev = Sdevname;
	/*
 	 * start the dump.
 	 */
	if(primaryflg) {
		if(ioctl(Dmpfd,DMPNOW_PRIM,0) < 0) {	/* start dump */
			cat_fatal(CAT_DMPNOW,
				"Cannot start dump to device %s.\n    %s\n",
					primarydev,errstr());
			exit(0);				/* exit */
		}
	} else if(secondaryflg) {
		if(!forceflg) {
			cat_print(CAT_PROMPT,"Insert into %s. Type <cr> when ready: ",
				secondarydev);
			fflush(stdout);					/* force the write */
			fflush(stdin);					/* empty any terminal input */
			getchar();						/* wait for '\n' */
		}
		if(ioctl(Dmpfd,DMPNOW_SEC,0) < 0) {	/* start dump */
			cat_fatal(CAT_DMPNOW,
				"Cannot start dump to device %s.\n    %s\n",
					secondarydev,errstr());
			exit(0);				/* exit */
		}
	} else
		usage_start();

	exit(0);
}
/*
 * NAME:     usage_start
 * FUNCTION: Output usage message for sysdumpstart and exit.
 * INPUTS:   None
 * RETURNS:  None (exits)
 */
static
usage_start()
{

        cat_eprint(CAT_USAGESTART,"\
Usage: %s -p | -s [-f]\n\
\n\
-p     Start a dump to the primary device.\n\
-s     Start a dump to the secondary device.\n\
-f     Force. Suppress prompt to make secondary dump device ready.\n",
                Progname);
        exit(1);
}

