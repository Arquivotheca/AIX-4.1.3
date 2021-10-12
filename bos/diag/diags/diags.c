static char sccsid[] = "@(#)01  1.18.4.25  src/bos/diag/diags/diags.c, diagsup, bos41J, 9516A_all 4/17/95 13:59:28";
/*
 * COMPONENT_NAME: (CMDDIAG) DIAGNOSTIC SUPERVISOR
 *
 * FUNCTIONS:   main
 *              check_service
 *              call_invoke
 *              p_error
 *              int_handler
 *              genexit
 *              arg_array
 *              dspTERM
 *              dspLOGO
 *              init_asl
 *              check_term
 *              ensure_exit
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
#include <signal.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include <sys/scdisk.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tmdefs.h"
#include "diag/class_def.h"             /* object class data structures */
#include "diags_msg.h"
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#define DEFAULT_FSM 1
#define EXIT_ID 99

#define VERSION         "4.1.3"
#define DATE            "1982,1995"
#define VALID_SELECTION(x)      ( ((x >= 1) && (x <= max_selection)) || (x == EXIT_ID) ) ? 1 : 0

#define LOGO                    1
#define FUNCTION_SELECTION      2
#define DCTRL_COMMAND           "dctrl"
#define MISSING_OPT_FLAG        "-a"
#define TERM_INITIALIZED        0
#define TERM_NEEDS_INITIALIZED  1

/* Locally defined variables */
nl_catd fdes;                   /* catalog file descriptor      */
int     diskette_based;         /* diskette based flag          */
int     diag_ipl_source;        /* cdrom or tape diags mode     */
char    *dadir;                 /* DA on diskette 3             */
int     init_asl_flg = 0;       /* indicates if asl is init     */
int     quiet_mode = 0;         /* -u option given              */
char    term[255];              /* used to change TERM env var  */


/* Externally defined variables */
extern ASL_SCR_TYPE dm_menutype;        /* ASL display parameters       */
extern char *optarg;
extern int optind;
extern char *diag_cat_gets();
extern nl_catd diag_catopen(char *, int);
extern FILE *fopen(char *, char*);

/* FUNCTION PROTOTYPES */
int call_invoke(char *, char *);
void p_error(int, char *);
void int_handler(int);                  /* interrupt handler routine    */
void genexit(int);
int arg_array(char *[], int, char *);
int dspTERM(void);
int check_service(int);
int dspLOGO(void);
int dspFSM(void);
void show_LED(short, int);
int init_asl(void);
int check_term(char *);
int ensure_exit(int, int);
void errflag(void);

/*  */
/*
 * NAME: main
 *
 * FUNCTION: The Diagnostic Supervisor presents a menu containing selections
 *      a user may make to run diagnostics. It also accepts flags as input,
 *      allowing the user to specify operations to be performed directly.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Independent program with command line arguments, usually
 *      invoked by a user from a shell.
 *
 * NOTES: This unit performs the following functions:
 *      1.  If flags are present on command line
 *          a. Invoke dctrl with appropriate flags.
 *      2.  Open DSMenu and DSMOptions Object Classes to get menus.
 *      3.  Display Operating Instructions Menu and wait for response.
 *      4.  Display Function Selection Menu and invoke appropriate action.
 *      what bits / data structures, etc it manipulates.
 *
 * RETURNS:
 *      0 : no error
 */

main(int argc, char *argv[])
{
        int     setterm;        /* indicates whether TERM should be set */
        char    *term;          /* the environment variable TERM        */
        short   screen;         /* menu to display value                */
        int     option;         /* used to hold command args            */
        int     i;              /* used as indexing value               */
        int     rc;             /* used as indexing value               */
        int     no_controlling_console=0;
        int     child_status=0;
        int     status;         /* child's process return code          */
        int     selection;      /* hold's user selection from menu      */
        int     finished;       /* exit program flag                    */
        int     index;          /* used as index into menulist          */
        int     first_entry;    /* flag used to determine if first time */
        int     exenvflg;       /* execution environment                */
        int     deviceflg=0;    /* set if testing single device         */
        int     fastselection=0;/* set to user selection if from service*/
        int     Aflag = 0;      /* -A option is not given               */
        int     Cflag = 0;      /* -C option is not given               */
        int     looponly=DIAG_FALSE; /* Set if -l is only flag given    */
        char    *action_ptr[100]; /* points to action to perform        */
        char    crit[32];       /* object class search criteria         */
        char    *diagdir;       /* points to default diag path          */
        char    *com;           /* points to command to execute         */
        char    command[256];   /* holds full path of command           */
        char    sys_str[256];   /* holds command to be executed         */
        char    opts[256];      /* holds command args                   */
        char    version[10];    /* holds version of diagnostics         */
        int     end_string = 0; /* traverse opts                        */
        char    flags[40];      /* holds flags relating to action       */
        int     max_selection = 4;
        FILE    *fdout;
        struct sigaction act;
        struct msglist menulist[100]; /* holds menu data                */
        struct DSMenu *T_DSMenu;      /* pointer to DSMenu Obj Cls data */
        struct DSMOptions *T_DSMOptions; /* ptr to DSMOptions data      */
        struct CuDv       *cudv; /* ptr to CuDv data    */
        struct CuAt       *cuat; /* ptr to CuDv data    */
        struct listinfo m_info, o_info, c_info, a_info;
        char    *temp_ptr;
        char    *mode;
        ASL_SCR_INFO    logoinfo[4];
        static ASL_SCR_TYPE logotype = DM_TYPE_DEFAULTS;

        setlocale(LC_ALL,"");

        /* Check if standard out is associated with a terminal */
        if ( (rc = isatty(1)) == 0 )
                no_controlling_console = DIAG_TRUE;

        /* determine execution environment */
        exenvflg = ipl_mode( &diskette_based );
        if((mode = (char *)getenv("DIAG_IPL_SOURCE")) == NULL)
                diag_ipl_source=0;
        else
                diag_ipl_source=atoi(mode);

       /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* get environment variable TERM */
        if((term = (char *)getenv("TERM")) == NULL )
                setterm = TERM_NEEDS_INITIALIZED;
        else if(!strcmp(term, "dumb"))
                setterm = TERM_NEEDS_INITIALIZED;
        else
                setterm = TERM_INITIALIZED;

        first_entry = DIAG_TRUE;

        /* open catalog file */
        fdes = diag_catopen(MF_DIAGS, 0);

        /* initialize the ODM */
        init_dgodm();

	/* Check to see if diagnostics is supported on the system */
	if(!supports_diagnostics())
	{
        	printf("%s",diag_cat_gets(fdes,MESSAGES,ERROR6));
		exit( DIAG_EXIT_NO_DIAGSUPPORT );
	}

        /* get directory path to use for command execution of diag's */
        if((diagdir = (char *)getenv("DIAGNOSTICS")) == NULL )
                diagdir = DIAGNOSTICS;
        strcpy(command, diagdir);
        strcat(command, "/bin/");
        com = command + strlen(command);

        /* set up controlling console if invoked through rc.boot */
        cuat = (struct CuAt *)diag_get_list(CuAt_CLASS,
		"attribute=keylock and value=normal", &a_info,1,1);
        if ( !a_info.num ) {
                rc = setsid();
                fdout = fopen("/dev/console", "w");
                fclose( fdout );
        }
        else
                diag_free_list(cuat, &a_info);


        /* If invoked with any options - call dctrl with options */
        if ( argc != 1 )  {
                /* check command flags to determine if quiet mode */
                while ((option = getopt(argc, argv, "acCAlesd:r:")) != EOF ){
                        /* Set flags if loopmode is the only flag given */
                        /* If so, save the -l until the user selection  */
                        /* If more flags are used, call dctrl as normal */
                        if ( option == 'l' )
                                looponly = DIAG_TRUE;
                        else
                                looponly = DIAG_FALSE;
                        switch ((char) option) {
                                case 'a' :
                                        /* Set up option string */
                                        opts[end_string++] = '-';
                                        opts[end_string++] = 'a';
                                        opts[end_string++] = ' ';

                                        /* Search for any devices marked "MISSING" */
                                        cudv = (struct CuDv *)diag_get_list(CuDv_CLASS,
                                               "chgstatus = 3", &c_info,MAX_EXPECT,1);
                                        if ( c_info.num != 0 )  /* found some */
                                                break;          /* so break   */

                                        /* Search for any devices marked "NEW" */
                                        diag_free_list(cudv, &c_info);
                                        cudv = (struct CuDv *)diag_get_list(CuDv_CLASS,
                                               "chgstatus = 0", &c_info,MAX_EXPECT,1);
                                        if ( c_info.num == 0 ) {          /* if none   */
                                                genexit(DIAG_EXIT_GOOD);  /* then exit */
                                        }
                                        break;
                                case 'c' :
                                        quiet_mode = 1;
                                        opts[end_string++] = '-';
                                        opts[end_string++] = 'c';
                                        opts[end_string++] = ' ';
                                        break;
                                case 'd' :
                                case 'r' :
                                        opts[end_string++] = '-';
                                        opts[end_string++] = option;
                                        opts[end_string++] = '\0';
                                        strcat(opts, optarg);
                                        end_string = strlen( opts );
                                        if (*optarg == '-')
                                                errflag();
                                        opts[end_string++] = ' ';
                                        deviceflg = 1;
                                        break;
                                case '?' :
                                        errflag();
                                        break;
                                default :
                                        if ( option == 'A' ){
                                                if ( Cflag )
                                                        errflag();
                                                else
                                                        Aflag = 1;
                                        }
                                        if ( option == 'C' ){
                                                if ( Aflag )
                                                        errflag();
                                                else
                                                        Cflag = 1;
                                        }
                                        opts[end_string++] = '-';
                                        opts[end_string++] = option;
                                        opts[end_string++] = ' ';
                                        break;

                        }
                }
                /* if no controlling console and not running specific device,
                   run system checkout in NO CONSOLE mode   */
                if ( no_controlling_console && !deviceflg ) {
                        strncpy( com, DCTRL_COMMAND, strlen(DCTRL_COMMAND) );
                        sprintf(sys_str, "%s -s -c", command);
                        status=system(sys_str);
                        if ( exenvflg == EXENV_STD )
				show_LED(0xA99, 15);
                        genexit(status>>8);
                }
                if (end_string == 0)
                        errflag();
                if (setterm && !quiet_mode){
                        if (dspTERM())
                                genexit(DIAG_EXIT_GOOD);
                }
                if ( looponly == DIAG_FALSE ) {
                        opts[--end_string] = '\0';
                        strncpy( com, DCTRL_COMMAND, strlen(DCTRL_COMMAND) );
                        sprintf(sys_str, "%s %s", command, opts);
                        status=system(sys_str);
                        genexit(status>>8);
                }
        }
        /* if no controlling console run system checkout in NO CONSOLE mode */
        else if ( no_controlling_console ){
                strncpy( com, DCTRL_COMMAND, strlen(DCTRL_COMMAND) );
                sprintf(sys_str, "%s -s -c", command);
                status=system(sys_str);
                if ( exenvflg == EXENV_STD )
			show_LED(0xA99, 15);
                genexit(status>>8);
        }

        /* Check to see if invoked from service mode from the hardfile. */
        if ( (fastselection = check_service(exenvflg)) != 0 )
                /* If fastpath - skip the logo screen */
                screen = FUNCTION_SELECTION;
        else
                /* else first screen is always the logo screen */
                screen = LOGO;

        finished = DIAG_FALSE;
        if(diag_ipl_source != IPLSOURCE_DISK)
                max_selection = 3;

        while ( !finished )  {

                /* Set up screen search pattern */
                sprintf( crit, "order = %d", screen );
                T_DSMenu = (struct DSMenu *)diag_get_list(DSMenu_CLASS,
			crit, &m_info, 1, 1);
                if (T_DSMenu == (struct DSMenu *) -1 )
                        p_error(ERROR4, "DSMenu");

                /* Set up msgkey search pattern */
                sprintf(crit,"msgkey = '%s' and order = 1", T_DSMenu->msgkey );

                switch ( screen )  {
                case LOGO:      /* Logo and Operating Instructions Frame.  */
                        if (setterm == TERM_NEEDS_INITIALIZED){
                                (void) dspLOGO();
                                screen = FUNCTION_SELECTION;
                        }
                        else {
                                if (!init_asl_flg)
                                        init_asl();
                                T_DSMOptions = (struct DSMOptions *)diag_get_list(
                                                DSMOptions_CLASS,
                                                crit, &o_info, 1, 1);
                                if((T_DSMOptions==(struct DSMOptions *) -1) ||
                                   (T_DSMOptions==(struct DSMOptions *) NULL))
                                        p_error(ERROR4, "DSMOptions");
                                temp_ptr = diag_cat_gets(fdes,
                                                T_DSMOptions->setid,
                                                T_DSMOptions->msgid);
                                logoinfo[0].text = (char*)malloc(10 +
                                                        strlen(temp_ptr));
                                rc = get_diag_att("diag_dskt", "dskt_version",
                                                 's', &status, version);
                                if (rc < 0)
                                  sprintf(logoinfo[0].text, temp_ptr, VERSION, DATE);
                                else
                                  sprintf(logoinfo[0].text, temp_ptr, version, DATE);
                                logoinfo[1].text = diag_cat_gets(
                                                fdes,T_DSMOptions->setid,
                                                MSG_801001_1);
                                logoinfo[2].text = diag_cat_gets(
                                                fdes,T_DSMOptions->setid,
                                                MSG_801001_E);
                                logotype.max_index = 2;
                                selection = diag_display( 0x801001,
                                                fdes, NULL, DIAG_IO,
                                                ASL_DIAG_NO_KEYS_ENTER_SC,
                                                &logotype, logoinfo);
                                if (selection == DIAG_ASL_ENTER)
                                        screen = FUNCTION_SELECTION;
                                else if ((selection == DIAG_ASL_CANCEL) ||
                                         (selection == DIAG_ASL_EXIT)) {
                                        if(ensure_exit(setterm,exenvflg))
                                                finished = DIAG_TRUE;
                                }
                        }
                        break;

                case FUNCTION_SELECTION:
                        screen = LOGO;
                        if ( setterm & !fastselection ){
                                do {
                                        if ((fastselection=dspFSM())==EXIT_ID) {
                                                if(ensure_exit(setterm,exenvflg))
                                                        genexit(DIAG_EXIT_GOOD);
                                        }
                                        else
                                           if(VALID_SELECTION(fastselection))
                                                setterm = dspTERM();

                                } while (setterm == TERM_NEEDS_INITIALIZED);
                        }
                        else if (setterm == TERM_NEEDS_INITIALIZED) {
                            do {
                                        if ( setterm = dspTERM() )
                                                if(ensure_exit(setterm,exenvflg))
                                                        genexit(DIAG_EXIT_GOOD);
                                } while (setterm == TERM_NEEDS_INITIALIZED);
                        }

                        if (!init_asl_flg)
                                init_asl();
                        index = 0;
                        while(T_DSMOptions = (struct DSMOptions *)diag_get_list(
                                DSMOptions_CLASS, crit, &o_info, 1, 1))  {

                                /* Do not include System Exerciser in  */
                                /* function selection if:              */
                                /*    Running on a RSPC system  OR     */
                                /*    In Concurrent Mode        OR     */
                                /*    In Standalone Mode (NOT DISK)    */

                                if( ( is_rspc_model() ||
                                    (exenvflg == EXENV_CONC) ||
				    (diag_ipl_source != IPLSOURCE_DISK) ) &&
                                    (!strcmp(T_DSMOptions->action,"sysx")) ) {
                                        sprintf(crit,
                                                "msgkey = '%s' and order = %d",
                                                T_DSMenu->msgkey,
                                                T_DSMOptions->order+1 );
                                        continue;
                                }
                                menulist[index].setid = T_DSMOptions->setid;
                                menulist[index].msgid = T_DSMOptions->msgid;
                                action_ptr[index] = (char *)calloc(1,strlen(
                                                T_DSMOptions->action)+1);
                                strcpy(action_ptr[index],T_DSMOptions->action);
                                index++;
                                sprintf(crit, "msgkey = '%s' and order = %d",
                                   T_DSMenu->msgkey, T_DSMOptions->order+1 );
                        }
                        menulist[index].setid = (int)NULL;
                        if ( !fastselection )
                                selection = diag_display(0x801002, fdes,
                                                menulist, DIAG_IO,
                                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                                NULL, NULL);
                        else
                                selection = DIAG_ASL_COMMIT;
                        if (selection == DIAG_ASL_EXIT) {
                                if(ensure_exit(setterm,exenvflg))
                                        finished = DIAG_TRUE;
                        }

                        /* if a menu selection was made, extract the command
                         * and arguments from the object list, and execute it
                         */
                        else if ( selection == DIAG_ASL_COMMIT )  {
                                if ( !fastselection )
                                        selection = DIAG_ITEM_SELECTED(dm_menutype);
                                else
                                        selection = fastselection;
                                fastselection = 0;
                                sscanf(action_ptr[selection], "%s %s",
                                                        com, flags );

                                if ( ((selection == 1) || (selection == 3)) &&
                                                (exenvflg == EXENV_STD) &&
                                                (first_entry == DIAG_TRUE) ) {
                                        first_entry = DIAG_FALSE;
                                        sprintf(action_ptr[selection],
                                                "%s %s %s",
                                                com,          /* command */
                                                MISSING_OPT_FLAG,
                                                (selection==1) ? "-C":"-A");
                                        call_invoke(command,
                                                    action_ptr[selection]);
                                        sprintf(action_ptr[selection], "%s %s",
                                                com, flags);
                                }
                                if ((selection != 2)&&(looponly==DIAG_TRUE))
                                        sprintf(action_ptr[selection],
                                                "%s %s %s",
                                                com,          /* command */
                                                flags,        /* normal  */
                                                opts);        /* -l      */

                                child_status = call_invoke(command,
                                                         action_ptr[selection]);

                                /* If a supplemental diskette has just been */
                                /* processed, go back to function selection */
                                /* to allow controller to be invoked again  */

                                if((child_status>>8) == DIAG_EXIT_DCTRL_RELOAD)
                                        screen=FUNCTION_SELECTION;
                        }
                        for ( i = 0; i < index; i++ )
                                if (action_ptr[i])
                                        free ( action_ptr[i] );
                }
        }
        genexit(child_status>>8);
}

/*  */
/**************************************************************************
 * NAME: check_service
 *
 * FUNCTION: Check if booted in service mode from hardfile
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      0 - no fastpath option
 *      1 - run diagnostic routines
 *      2 - run service aids
 *      3 - run advanced diagnostic routines
 *      4 - run system exerciser
 *
 *************************************************************************/

int check_service(
        int exenvflg )
{
        FILE    *fd;
        int     selection = 0;
        char    *path = "/etc/lpp/diagnostics/data/fastdiag";
        char    *data = "0";

        /* if running in concurrent mode - return */
        if ( exenvflg == EXENV_CONC )
                return(0);

        /* Try to open the /etc/lpp/diagnostics/data/fastdiag file      */
        /* If error opening or reading - return with default    */
        if ((fd = fopen(path,"r+")) == (FILE *)NULL )
                return(0);

        /* scan the file looking for a valid number 1 to 4      */
        /* This must be at the beginning of the file            */
        fscanf( fd, "%d", &selection );

        /* rewind and set the file to read 0 for next time around */
        rewind(fd);
        fwrite( data, sizeof(data), 1, fd );

        fclose ( fd );

        /* If selection is a valid 1 to 4, return it            */
        /* Else default to 0                                    */
        if ( selection >=1 && selection <= 4 )
                return (selection);
        else
                return (0);

}
/*  */
/**************************************************************************
 * NAME: call_invoke
 *
 * FUNCTION: Set up calling arguments for diag_invoke
 *
 * RETURNS:
 *      0 - no errors
 *     -1 - error occurred
 *
 *************************************************************************/

int call_invoke(
        char *progname,
        char *arguments)
{
        int     child_status;
        char    *flag_ptr[20];       /* points to command and arguments     */

        arg_array(flag_ptr, sizeof(flag_ptr)/sizeof(flag_ptr[0]),
                arguments);

        if( diag_asl_execute(progname, flag_ptr, &child_status) == -1 )
                Perror(fdes, MESSAGES, ERROR2, progname);
        return(child_status);
}

/*************************************************************************
 * NAME: p_error
 *
 * FUNCTION: Print error message to the screen
 *
 * RETURNS:  NONE
 *
 *************************************************************************/

void p_error(
        int err_msg,
        char *str)
{
        if (!init_asl_flg) init_asl();
        Perror(fdes, MESSAGES, err_msg, str);
        genexit(DIAG_EXIT_OBJCLASS_ERROR);
}

/**************************************************************************
* NAME: errflag
*
* FUNCTION: Print usage message to the screen
*
* NOTES:  Other local routines called --
*         genexit()
*
* RETURNS:  NONE
*
**************************************************************************/
void errflag(void)
{
        printf("%s",diag_cat_gets(fdes,MESSAGES,USAGE));
        genexit(DIAG_EXIT_USAGE);
}

/*  */
/**************************************************************************
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
**************************************************************************/
void int_handler(int sig)
{
        genexit(DIAG_EXIT_RESPAWN);
}

/*************************************************************************
 * NAME: genexit
 *
 * FUNCTION: Perform general clean up, and then exit with the status code.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS:
 *      0 - no errors
 *     >0 - error occurred
 *
 *************************************************************************/

void genexit(int exitcode)
{
        term_dgodm();
        if (init_asl_flg)
                diag_asl_quit();
        catclose(fdes);
        exit(exitcode);
}

/*************************************************************************
 * NAME: arg_array
 *
 * FUNCTION: Convert character string to variable list array
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *     >0 - number of elements in array
 *
 *************************************************************************/

int arg_array(
        char *array[],
        int ar_size,
        char *string)
{
        int     j;
        char    *temp;

        for(j=0;  j < ar_size; j++) {
                array[j] = (char*) strtok(string, " \t\n");
                string = NULL;
                if( NULL == array[j] )
                        break;
        }
        return(j);
}
/*  */
/* NAME: dspFSM
 *
 * FUNCTION: Displays the Function Selection Menu.
 *           User enters an ID to advance.
 *           This routine is only called if the TERM environment variable
 *           is not set.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *     user's response
 *
 */
int dspFSM(void)
{
        int responsei;
        char response[255];

        if(diag_ipl_source != IPLSOURCE_DISK)
                printf( diag_cat_gets(fdes, SET_FS, MSG_FS_SA_PRINTF) );
        else
                printf( diag_cat_gets(fdes, SET_FS, MSG_FS_PRINTF) );
        gets(response);
        if (strlen(response))
                sscanf(response,"%d",&responsei);
        else
                return(DEFAULT_FSM);
        return(responsei);
}
/*  */
/*************************************************************************
 * NAME: dspTERM
 *
 * FUNCTION: Asks the user to enter a terminal type or Enter to return.
 *           Sets the environment variable TERM.
 *           This routine is only called if the TERM environment variable
 *           is not set.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *     1 = TERM still needs to be set
 *     0 = TERM does not need to be set.
 ************************************************************************/
int dspTERM(void)
{
        char    response[255];
        char    termfile[255];
        char    args[255];
        int     ignore_status;

        printf(diag_cat_gets(fdes, SET_TERM, MSG_TERM));
        gets( response );
        while (1) {
                if(!strlen(response))
                        break;
                if(diag_ipl_source != IPLSOURCE_DISK){
                        /* Stand alone mode, make sure current directory */
                        /* is /, before cpioing the terminfo file.       */
                        /* There is no need to restore current working   */
                        /* directory.                                    */
                        chdir("/");
                        if(diag_ipl_source == IPLSOURCE_TAPE){
                                sprintf(termfile,"usr/share/lib/terminfo/%c/%s",
                                          response[0], response);
                                sprintf(args,
                                "/usr/bin/cpio -iud < %s %s >/dev/null 2>&1",
                                "/usr/lpp/diagnostics/data/term.cpio",
                                termfile);
                                ignore_status = system(args);
                                if(ignore_status != 0)
                                         printf(diag_cat_gets(fdes, SET_TERM, MSG_NOTERM));
                                else{
                                        sprintf( term, "TERM=%s", response );
                                        return( putenv(term) );
                                }
                        } else {
                                sprintf( term, "TERM=%s", response );
                                return( putenv(term) );
                        }
                } else {
                        if ((check_term(&response[0])>0) ){
                                 sprintf( term, "TERM=%s", response );
                                 return( putenv(term) );
                        } else
                                 printf(diag_cat_gets(fdes, SET_TERM, MSG_NOTERM));
                }
                gets(response);
        }
        return( 1 );
}

/*  */
/* NAME: dspLOGO
 *
 * FUNCTION: Displays the logo screen.
 *           User presses Enter to advance.
 *           This routine is only called if the TERM environment variable
 *           is not set.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *     0 = success
 *
 */
int dspLOGO(void)
{
        char response[255];
        char    *logo;
        char    *temp_ptr;
        char    version[10];
        int     rc, cnt;

        /* get message and put in the version */
        temp_ptr = diag_cat_gets ( fdes, SET_OPI, MSG_LOGO );
        logo = (char*)malloc(10+strlen(temp_ptr));
        rc = get_diag_att("diag_dskt", "dskt_version", 's', &cnt, version);

        if (rc < 0)
          sprintf(logo, temp_ptr, VERSION, DATE);
        else
          sprintf(logo, temp_ptr, version, DATE);

        printf( logo );
        gets( response );

        return(0);
}
/*  */
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
int init_asl(void)
{
        int rc;
        rc = diag_asl_init("NO_TYPE_AHEAD");
        if ( rc == DIAG_ASL_ERR_SCREEN_SIZE )
        {
                printf(diag_cat_gets(fdes,MESSAGES,ERROR5,""));
                genexit(DIAG_EXIT_SCREEN);
        }
        init_asl_flg = 1;
        return(0);
}

/*
 * NAME: check_term
 *
 * FUNCTION: This function searches the defined terminal file for a match with
 *           the terminal type entered by the user.
 *
 * RETURNS:  1 if match is found
 *           0 if no match
 */

int check_term(char *term_type)
{
        struct stat info;
        char    termfile[255];

        sprintf(termfile,"/usr/share/lib/terminfo/%c/%s",term_type[0], term_type);
        if(stat(termfile, &info) != 0)
                return(0);
        else
                return(1);
}
/*  */
/*
 * NAME: ensure_exit
 *
 * FUNCTION: This function prompts the user to verify that they really want to
 *           do a system shutdown from Service Mode.
 *
 * RETURNS: 1 if user wants to shutdown
 *          0 if return to Diagnostics
 */

int ensure_exit(int setterm, int exenvflg)
{
        char    response[255];
        int     rc;
        char    *keypos;

        /* If not in service mode, exit */
        if(exenvflg != EXENV_STD)
                return(1);

        /* If we are in Maintenance Mode (or we do not know), exit */
        if((keypos = (char *)getenv("DIAG_KEY_POS")) == (char *)NULL)
                return(1);
        if(strncmp(keypos,"0",1)==0)
                return(1);

        /* If TERM=dumb, we need to scroll the query */
        if(setterm) {
                if( diag_ipl_source == IPLSOURCE_TAPE ) {
                        printf( diag_cat_gets(fdes, SET_SHUTDOWN, DIAG_EXIT_PRINTF) );
                        gets(response);
                        return(0);
                } else if (diag_ipl_source == IPLSOURCE_CDROM) {
			printf( diag_cat_gets(fdes, SET_SHUTDOWN, DIAG_EXIT_CDBOOT_PRINTF) );
			gets(response);
                        /* If response = 99 eject cd disc,else return to Diag */
			/* After cd is ejected, diags should be exited not    */
			/* going through genexit, to prevent the screen from  */
			/* being cleared.				      */
                        if((strlen(response)==2)&&(strncmp(response,"99",2)==0))
                                exit(0); /* equivalent to returning 1 */
                        else
                                return(0);

		}
                else {
                        printf( diag_cat_gets(fdes, SET_SHUTDOWN, MSG_EXIT_PRINTF) );
                        gets(response);
                        /* If response = 99 continue shutdown, else return to Diag */
                        if((strlen(response)==2)&&(strncmp(response,"99",2)==0))
                                return(1);
                        else
                                return(0);
                }
        }

        /* else, TERM is set and we can use ASL to prompt the user */
        else {
                if(diag_ipl_source == IPLSOURCE_TAPE) {
                        rc = diag_asl_msg(diag_cat_gets(fdes,SET_SHUTDOWN,DIAG_EXIT));
                        return(0);
                } else if(diag_ipl_source == IPLSOURCE_CDROM){
			rc = diag_asl_msg(diag_cat_gets(fdes,SET_SHUTDOWN,DIAG_EXIT_CDBOOT));
                        switch (rc) {
                        case DIAG_ASL_EXIT:
                        case DIAG_ASL_CANCEL:
                                /* EXIT and CANCEL mean return to Diagnostics */
                                return(0);
                                /* A 'break' here is unnecessary, and causes lint warning */
                        default:
                                /* Otherwise, eject the disc */
				/* After cd is ejected, diags should be exited not    */
				/* going through genexit, to prevent the screen from  */
				/* being cleared.				      */
                                exit(0); /* equivalent to returning 1 */
			}
		}
                else {
                        rc = diag_asl_msg(diag_cat_gets(fdes,SET_SHUTDOWN,MSG_EXIT));
                        switch (rc) {
                        case DIAG_ASL_EXIT:
                        case DIAG_ASL_CANCEL:
                                /* EXIT and CANCEL mean return to Diagnostics */
                                return(0);
                                /* A 'break' here is unnecessary, and causes lint warning */
                        default:
                                /* Otherwise, cancel the shutdown */
                                return(1);
                        } /* endswitch */
                }
        } /* endelse */
}
