static char sccsid[] = "@(#)06	1.22.2.23  src/bos/diag/dctrl/dctrl.c, dctrl, bos41J, 9514A_all 3/29/95 17:38:44";
/*
 * COMPONENT_NAME: (CMDDIAG) Diagnostic Controller
 *
 * FUNCTIONS:   main
 *              genexit
 *              int_handler
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <nl_types.h>
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/tmdefs.h"
#include "dctrl_msg.h"
#include "dctrl.h"
#include <locale.h>
#include "diag/diag_define.h"

#define   menu_end_key   (rc==DIAG_ASL_EXIT)
#define   menu_quit_key  (rc==DIAG_ASL_CANCEL)

/* GLOBAL VARIABLES     */
int showleds      = 1;            /* Display led in no console mode     */
int systestflg    = SYSTEM_FALSE; /* -s option, test whole system       */
int advancedflg   = ADVANCED_FALSE; /* -A option, advanced diagnostics  */
int customerflg   = DIAG_FALSE;   /* -C option, Customer Diagnostics    */
int deviceflg     = DIAG_FALSE;   /* -d option, test specific device    */
int unattendedflg = DIAG_FALSE;   /* -c option, unattended mode         */
int missingflg    = DIAG_FALSE;   /* -a option, check missing parts     */
int next_diskette = DIAG_FALSE;   /* insert next diskette selection     */
int regressflg    = DIAG_FALSE;   /* -r option, regression check        */
int regr_setup    = 0;            /* -r option, setup value             */
int pretest       = 0;            /* -D option, pretest value           */
int exenvflg      = EXENV_IPL;    /* execution modes                    */
int elaflg        = DIAG_FALSE;   /* error log analysis                 */
int displayflg    = DIAG_FALSE;   /* test displays before using console */
int ttymode       = DIAG_FALSE;   /* unattended mode - use NLprintf     */
int diag_mode     = DMODE_PD;     /* contains diag. mode selection      */
int moretesting   = DIAG_FALSE;
int present_dm_menu = DIAG_FALSE;
int present_tm_menu = DIAG_FALSE;
int diskette_based  = DIAG_FALSE; /* executing off diskette             */
int diag_ipl_source  = IPLSOURCE_DISK;  /* cdrom, tape diagnostics mode */
int diag_source  = IPLSOURCE_DISK;  /* diags code from cdrom or disk	*/
int consoleflg      = CONSOLE_TRUE;
int asl_init_good   = CONSOLE_FALSE;
int srn_generated   = DIAG_FALSE; /* has gen_rpt been called with a srn */
int num_diskettes = 0;            /* number of diskettes in package     */
int num_DSM_devices = 0;          /* number of devices in selection menu*/
int num_Top = 0;                  /* number of devices in Top array     */
int num_All = 0;                  /* number of devices in All array     */
char lockfile [255];		  /* lock file name			*/
int  lockfd = -1;		  /* lock file descriptor		*/
int  lockpid;			  /* lock file locking process		*/
unsigned int mach_model;          /* machine model code as returned by ROS*/
int model_code;                   /* model code                         */
diag_dev_info_t     *Top = NULL;  /* pointer to all device data structures */
diag_dev_info_ptr_t *DSMenu = NULL; /* Diagnostic Selection Menu list   */
nl_catd              fdes;        /* catalog file descriptor            */
int		diag_report=0;	  /* Flag to log symptom strings.	*/
int             current_volume=0;   /* current backup volume for tape diag */
char            *dadir = NULL;    /* Path to DA Directory               */
char		*datadir = NULL;  /* Path to data directory		*/
char            *diskenv = NULL;  /* diskette environment variable      */
char		*bootdev = NULL;  /* Name of boot device (tape).	*/
char            devicename[NAMESIZE] = {'\0'};


/* EXTERNALLY DEFINED VARIABLES */
extern int      optind;         /* defined by getopt()                  */
extern char     *optarg;        /* defined by getopt()                  */
extern int      lerror;
extern int      errno;          /* global error number                  */

/* CALLED FUNCTIONS */
void int_handler(int);           /* interrupt handler                   */
int diag_exec_source();
void unload_diag_kext();
int ipl_mode();
int testmissing();
int testsystem();
int run_pretests();
int testnormal();
int disp_da_output();
int disp_dc_error();
int disp_dm_menu();             /* display diagnostic mode selection */
unsigned int get_cpu_model(int *);

diag_dev_info_t *init_diag_db();
diag_dev_info_ptr_t *gen_ds_menu();
extern nl_catd diag_catopen(char *, int);
extern char *diag_cat_gets();

#define DEFAULT_EMSG1 "Another diagnostic session is active (pid %d)\n\
Only one session can be active at any time.\n\
If no diagnostic session is present, the lock file\n\
/etc/lpp/diagnostics/data/dctrl.lck can be removed.\n"

#define DEFAULT_EMSG2 "Unable to create lock file:\n\
%s\n"

/*  */
/*
 * NAME: main
 *
 * FUNCTION: main program for the dctrl.  Processes the command line
 *           arguments.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This program executes as an application of the kernel.  It
 *      executes under a process, and can page fault.
 *
 * NOTES:
 *
 *      dctrl [-a] [-c] [-dname] [-e] [-rsetup] [-s] [-A] [-C] [-D] [-l]
 *
 *      -a      Perform missing device analysis.
 *      -c      Machine is unattended.  No prompts should be displayed.
 *      -dname  Test the named device.  The device must have already been
 *              configured.  The "name" parameter is the /dev name of the
 *              device.
 *      -e      Error Log Analysis.  Not documented for external use.  Checks
 *              the error log for device specified in "name" of the -d option.
 *      -rsetup Regression Testing.  Used in conjuntion with -c and -d options.
 *              The "setup" parameter designates the Application Test Unit
 *              number to run on device "name" specified by the -d option
 *      -s      Test the system.
 *
 *      -A      Advanced Diagnostics
 *      -C      Customer Diagnostics
 *      -D      Test the console before it is used.
 *      -l      Loop mode. (Hidden flag).
 *
 * RETURNS:
 *       0      No trouble found.
 *      -1      A problem was found.
 */

main (argc, argv)
int argc;
char *argv[];
{
        int c, rc;
        struct sigaction act;
        int     bc, index;
        char    *list2, *comma;
	char	mounted_cdrfs[128];
        char    list[256];

        setlocale(LC_ALL, "");

        while((c = getopt(argc,argv,"acd:elr:sACD:")) != EOF) {
                switch((char) c) {
                case 'a':
                        missingflg = DIAG_TRUE;
                        break;
                case 'c':
                        unattendedflg = DIAG_TRUE;
                        break;
                case 'd':
                        deviceflg = DIAG_TRUE;
                        strcpy(devicename, optarg);
                        break;
                case 'e':
                        diag_mode = DMODE_ELA;
                        break;
                case 'l':
                        present_tm_menu = DIAG_TRUE;
                        break;
                case 'r':
                        regressflg = DIAG_TRUE;
                        regr_setup = atoi(optarg);
                        break;
                case 's':
                        systestflg = SYSTEM_TRUE;
                        break;
                case 'A':
                        advancedflg = ADVANCED_TRUE;
                        break;
                case 'C':
                        customerflg = DIAG_TRUE;
                        break;
                case 'D':
                        displayflg = DIAG_TRUE;
                        pretest = atoi(optarg);
                        break;
                default :
                        exit(DIAG_EXIT_USAGE);
                }
        }

       /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

/*  */
        /* determine DA Directory Path from environment variable */
        if ( (dadir = (char *)getenv("DADIR")) == NULL )
                dadir = DEFAULT_DADIR;

        /* open the message catalog file: always done because trouble reports
           are logged to a file even if system unattended or no console */
        fdes = diag_catopen(MF_DCTRL, 0);

        /* initialize the ODM   */
        init_dgodm();

        /* Check to see if diagnostics is supported on the system */
	if(!supports_diagnostics())
	{
		printf("%s",diag_cat_gets(fdes,ESET,ERROR18));
		genexit( DIAG_EXIT_NO_DIAGSUPPORT );
	}

        /* if unattended mode, make sure checking a device or system test */
        if (unattendedflg && !(deviceflg || systestflg)) {
                printf("%s", diag_cat_gets( fdes, INFOSET, TMSGU ) );
                exit(DIAG_EXIT_GOOD);
        }

        /* determine execution mode */
        exenvflg = ipl_mode( &diskette_based );

        /* determine if led should be displayed in no console mode */
        /* If environment variable is not defined, default is to show leds */
        if( (diskenv = (char *)getenv("SHOWLEDS")) != NULL )
                showleds = atoi(diskenv);

        if((diskenv = (char *)getenv("DIAG_IPL_SOURCE")) == (char *)NULL)
		diag_ipl_source=0;
	else
		diag_ipl_source=atoi(diskenv);

	diag_source=diag_exec_source(&mounted_cdrfs);

	/* Get the value of DIAG_REPORT. This will dictates whether */
	/* or not symptom strings should be logged on errors.	    */

        if((diskenv = (char *)getenv("DIAG_REPORT")) == (char *)NULL)
		diag_report=0;
	else
		diag_report=atoi(diskenv);

	/* Obtain the boot device name. Used this name when displaying */
	/* menu during tape restore.				       */

	bootdev = (char *)getenv("BOOTDEV");

	/* Get the data directory value.			       */

	if((datadir = (char *)getenv("DIAGDATADIR")) == (char *)NULL)
		datadir = DIAGDATA;
		
        /* if testing underlying displays */
        if (displayflg)
                unattendedflg = DIAG_TRUE;
        else if ( unattendedflg )
                ttymode = DIAG_TRUE;
        else { /* test to see if there is a console     */
                if (init_asl() < 0){
                        systestflg = SYSTEM_TRUE;
                        asl_init_good = CONSOLE_FALSE;
                        unattendedflg = DIAG_TRUE;
                }
                else
                        asl_init_good = CONSOLE_TRUE;
        }
        if ( unattendedflg )
                consoleflg = CONSOLE_FALSE;

	/* If running from hardfile, only allow one instance of the dctrl
	   to run at once. 				        	 */

        if ((diag_ipl_source == IPLSOURCE_DISK) && (displayflg == DIAG_FALSE)
		&& (exenvflg == EXENV_CONC) ) {
                /* Disallow more than one copy of dctrl at a time. */
                /* DO NOT USE LOCK FILE IN PRETEST                 */
                sprintf(lockfile, "%s/dctrl.lck", datadir);
                for (; (lockfd = open(lockfile, O_CREAT | O_EXCL | O_WRONLY |
                                O_NSHARE, 0600)) < 0;) {

                        /* lockfile exists, make sure locking process does */
                        char    buf[255];

                        if ((lockfd = open(lockfile, O_RDWR)) > 0 &&
                                read(lockfd, &lockpid, sizeof(lockpid)) ==
                                sizeof(lockpid) && kill(lockpid, 0) < 0) {

                                /* locking process does not exist */
                                break;
                        }
                        /* Another process is running with locking process' PID.
                           Make sure lockfile isn't from a previous boot. */
                        sprintf(buf,
                                "%s -p%d|%s dctrl > /dev/null 2>&1",
                                PS, lockpid, GREP);
                        if (lockpid == getpid() || system(buf))
                                break;

                        /* print msg and exit */
                        lockfd = -1;
			sprintf(buf, "%d", lockpid);
			if(ttymode)
                        	printf(diag_cat_gets(fdes, ESET, ERROR15,
	       				DEFAULT_EMSG1), buf);
			else 
				if(asl_init_good && !displayflg){
					disp_dc_error(ERROR15, buf); 
                			diag_asl_quit();
				}
			/* exit here without going through genexit since */
			/* we have not done much.			 */

                        exit(DIAG_EXIT_BUSY);
                }
		if(lockfd < 0){
		/* Unable to create lock file */
			if(ttymode)
				printf(diag_cat_gets(fdes, ESET, ERROR16,
					DEFAULT_EMSG2), lockfile);
			else 
				if(asl_init_good && !displayflg){
					disp_dc_error(ERROR16, lockfile); 
                			diag_asl_quit();
				}
			/* exit here without going through genexit since */
			/* we have not done much.			 */
			exit(DIAG_EXIT_LOCK_ERROR);
		}
				   
                /* create lockfile */
                lockpid = getpid();
                lseek(lockfd, 0, SEEK_SET);
                write(lockfd, &lockpid, sizeof(lockpid));
                close(lockfd);
        }
        /* if unattended but there is a console - use printf to print */
        if ( ttymode )
                printf("%s", diag_cat_gets( fdes, INFOSET, TMSGS) );

        /* if performing system test, force customer mode testing */
        if (systestflg || customerflg){
                advancedflg = ADVANCED_FALSE;
                customerflg = 1;
        }
        else {
                advancedflg = ADVANCED_TRUE;
                customerflg = 0;
        };

        /* start out with the object classes cleaned out */
        if(clr_class("FRUB"))
                disp_dc_error( ERROR6, "FRUB");
        if(clr_class("FRUs"))
                disp_dc_error( ERROR6, "FRUs");
        if(clr_class("MenuGoal"))
                disp_dc_error( ERROR6, "MenuGoal");
        if(clr_class("DAVars"))
                disp_dc_error( ERROR6, "DAVars");

        /* initialize the data structures for the test devices  */
        if ((Top = init_diag_db( &num_Top )) == (diag_dev_info_t *) -1 )
                genexit(DIAG_EXIT_OBJCLASS_ERROR);

        /* get list of all devices to be included in diag selection menu */
        DSMenu = gen_ds_menu( num_Top, Top, &num_All,
                                &num_DSM_devices, exenvflg);

        /* get the machine model code */
        mach_model = get_cpu_model(&model_code);


        if (displayflg){
		if ( has_LED_support() ) {
                	/* pre-test certain devices before console can be used */
                	rc = run_pretests();
                	if (lerror)
                        	genexit(DIAG_EXIT_DEVICE_ERROR);
		}
                genexit(DIAG_EXIT_GOOD);
        }
        else if (missingflg){
                if (diag_ipl_source == IPLSOURCE_DISK)
                        rc = testmissing();
        }
        else if (systestflg) {
                /* perform system test */
                rc = testsystem();
        }
        else if (deviceflg) {
                /* test a particular device */
		rc=testnormal();
                if ( rc == -3)
                        genexit(DIAG_EXIT_NO_DEVICE);
		if ( rc == -4)
			genexit(DIAG_EXIT_NoPDiagDev);
        }
        else {
        	/* Run the new resources program */
		if (!displayflg)
			if (check_new_devices() == DIAG_ASL_OK)
				update_device_database();

                /* perform normal test mode */
                rc = disp_dm_menu(&diag_mode);
                if (menu_end_key || menu_quit_key)
                        genexit(DIAG_EXIT_GOOD);
                /* if problem determination - call ela first */
                if ((diag_mode==DMODE_PD) && (diag_ipl_source==IPLSOURCE_DISK)){
                        elaflg = DIAG_TRUE;
                        rc = dctrlela();
                        if (menu_end_key)
                                genexit(DIAG_EXIT_GOOD);
                        elaflg = DIAG_FALSE;
                }
                rc = 0;
                while (rc==0){
                        moretesting = DIAG_FALSE;
                        if((rc = testnormal() ) == -1 )  /* if system error */
                                rc = 0;                  /* set rc to 0     */
                        else if ( rc == 0 )              /* else display    */
                                disp_da_output();        /*   output        */
                        systestflg = SYSTEM_FALSE;
                }
                if (lerror)
                        genexit(DIAG_EXIT_DEVICE_ERROR);
                genexit(DIAG_EXIT_GOOD);
        }
        if ( !missingflg )
                disp_da_output();

        if (lerror)
                genexit(DIAG_EXIT_DEVICE_ERROR);
        else
                genexit(DIAG_EXIT_GOOD);
}
/*  */
/*
 * NAME:  genexit
 *
 * FUNCTION: general exit point for dctrl.
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */

genexit(exitcode)
int exitcode;
{
	char cmd[256];

        /* if ttymode */
        if ( ttymode ) {
		if ( exitcode == DIAG_EXIT_NoPDiagDev )
			printf("%s\n", diag_cat_gets( fdes, ESET, ERROR17) );
                else if ( (exitcode != DIAG_EXIT_GOOD) && (exitcode != DIAG_EXIT_NO_DEVICE) )
                        printf("%s", diag_cat_gets( fdes, INFOSET, TMSGP) );
                else if ( exitcode == DIAG_EXIT_NO_DEVICE) {
                        printf("%s\n", diag_cat_gets( fdes, ESET, ERROR2) );
                	exitcode = DIAG_EXIT_NO_DEVICE;
		}
                printf("%s", diag_cat_gets( fdes, INFOSET, TMSGE ));
        }

        /* save all changes to the Customized Diag Object Class */
        if (Top != (diag_dev_info_t *) -1 )  {
                if ( save_cdiag ( Top ,num_Top ) )
                        disp_dc_error( ERROR6, "CDiagDev");
        }

        /* remove any devices that were defined just for diagnostics */
        remove_new_device();

        /* clean up asl/curses/cur if initialized */
        if (asl_init_good && !displayflg)
                diag_asl_quit();

        /* Unload any diagnostic kernel extension needs unloading. */
        
	unload_diag_kext();
        sprintf(cmd,"%s 1>/dev/null 2>&1",SLIBCLEAN);
        (void) system(cmd);

        /* reset NVRAM LED buffer area */
        reset_LED_buffer();
        
	if(lockfd >= 0)
		unlink(lockfile);
        term_dgodm();
        catclose(fdes);
        exit( exitcode );
}
/*  */
/**************************************************************
* NAME: int_handler
*
* FUNCTION: In case of an interrupt, this routine is called.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       genexit()
*
* RETURNS:
*       None
****************************************************************/
void int_handler(int sig)
{
        if (asl_init_good && !displayflg)
        	diag_asl_clear_screen();


        genexit(DIAG_EXIT_INTERRUPT);
}

/*
 * NAME: init_asl
 *
 * FUNCTION:  This routine initializes asl. If an error, print msg and quit.
 *
 * NOTES:
 *
 * RETURNS:
 *
 */
int
init_asl()
{
        int rc;
        rc = diag_asl_init("default");
        if ( rc == DIAG_ASL_ERR_SCREEN_SIZE )
        {
                printf(diag_cat_gets(fdes,ESET,ERROR12));
                asl_init_good = CONSOLE_FALSE;
                term_dgodm();
                catclose(fdes);
                exit(DIAG_EXIT_SCREEN);
        }
        return(rc);
}
