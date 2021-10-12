static char sccsid[] = "@(#)70	1.14.1.2  src/bos/diag/util/uformat/kazfor.c, dsauformat, bos411, 9428A410j 12/10/92 16:22:16";
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		base_path
 *		check_ioctl
 *		check_rc
 *		check_rc1
 *		check_rc2
 *		check_rc_quit
 *		clean_up
 *		clear_pvid
 *		main
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <nl_types.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <time.h>
#include <sys/types.h>
#include <sys/devinfo.h>
#include <sys/badisk.h>
#include <sys/ioctl.h>
#include <sys/cfgodm.h>
#include <errno.h>

#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/diag_exit.h"
#include "kazfor_msg.h"


struct msglist destroy[] = {
				{ASK,ASK_TITLE},
				{ASK,ASK_YES},
				{ASK,ASK_NO},
				{ASK,ASK_ACTION},
				(int)NULL
};

struct msglist long_3[] = {
				{HOUR_3,HOUR_3_TITLE},
				{HOUR_3,HOUR_3_YES},
				{HOUR_3,HOUR_3_NO},
				{HOUR_3,HOUR_3_ACTION},
				(int)NULL
};

struct msglist confirm[] = {
				{CHECK,CHECK_TITLE},
				{CHECK,CHECK_YES},
				{CHECK,CHECK_NO},
				{CHECK,CHECK_ACTION},
				(int)NULL
};

struct msglist v_confirm[] = {
				{VERIFY,VERIFY_TITLE},
				{VERIFY,VERIFY_YES},
				{VERIFY,VERIFY_NO},
				{VERIFY,VERIFY_ACTION},
				(int)NULL
};

/*
struct msglist intx21_or_not[] = {
				     {Q_INTX21_OR_NOT,Q_INTX21_OR_NOT_TITLE},
				     {Q_INTX21_OR_NOT,Q_INTX21_OR_NOT_YES},
				     {Q_INTX21_OR_NOT,Q_INTX21_OR_NOT_NO},
				     {Q_INTX21_OR_NOT,Q_INTX21_OR_NOT_ACTION},
				     (int)NULL
};

struct msglist intv24_or_not[] = {
				     {Q_INTV24_OR_NOT,Q_INTV24_OR_NOT_TITLE},
				     {Q_INTV24_OR_NOT,Q_INTV24_OR_NOT_YES},
				     {Q_INTV24_OR_NOT,Q_INTV24_OR_NOT_NO},
				     {Q_INTV24_OR_NOT,Q_INTV24_OR_NOT_ACTION},
				     (int)NULL
};

struct msglist intv35_or_not[] = {
				     {Q_INTV35_OR_NOT,Q_INTV35_OR_NOT_TITLE},
				     {Q_INTV35_OR_NOT,Q_INTV35_OR_NOT_YES},
				     {Q_INTV35_OR_NOT,Q_INTV35_OR_NOT_NO},
				     {Q_INTV35_OR_NOT,Q_INTV35_OR_NOT_ACTION},
				     (int)NULL
};
*/

static ASL_SCR_INFO q_destroy[DIAG_NUM_ENTRIES(destroy)];
static ASL_SCR_INFO q_confirm[DIAG_NUM_ENTRIES(confirm)];
static ASL_SCR_INFO q_v_confirm[DIAG_NUM_ENTRIES(v_confirm)];
static ASL_SCR_INFO q_long_3[DIAG_NUM_ENTRIES(long_3)];
/*
static ASL_SCR_INFO q_intx21_or_not[DIAG_NUM_ENTRIES(intx21_or_not)];
static ASL_SCR_INFO q_intv24_or_not[DIAG_NUM_ENTRIES(intv24_or_not)];
static ASL_SCR_INFO q_intv35_or_not[DIAG_NUM_ENTRIES(intv35_or_not)];
*/
static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

struct badisk_ioctl_parms badisk_ioctl_parms; 
struct badisk_ioctl_parms arg_for; 
struct badisk_ioctl_parms arg_prog; 
struct badisk_cstat_blk arg_stat; 
struct devinfo device_info;

int		fail;
int		pvid_destroyed=0;
int	        format_opt;
int             rc;        	/* to hold functions return codes    */
nl_catd         fdes;        	/* file descriptor for catalog file  */
int             fd;        	/* file descriptor for opening DD    */
int		state;		/* state of the device */
struct CuDv  *cudv_selected;	/* struct for ODM retrieve  */

extern nl_catd diag_catopen(char *, int);

char	*DEVICE = (char *) 0;
char	*DEVICE_NAME = (char *) 0;
char	*LOCATION = (char *) 0;

main(argc,argv,envp)
int argc;
char **argv;
char **envp;
{
    char       *base_path();
    void	clear_pvid();
    int         i;			/* loop counter */
    int         r;
    int		err_count;
    int		percent;
    long	last_block;
    long	maxerr_count;
    long	max_rba;
    long	ver_step;

    setlocale(LC_ALL,"");
    if( argc != 4 ) {
	exit();
    }
    DEVICE = (char *)malloc(1024);
    LOCATION = (char *)malloc(1024);
    DEVICE_NAME = (char *)malloc(1024);
    strcpy( DEVICE, argv[1] );
    strcpy( LOCATION, argv[2] );
    strcpy( DEVICE_NAME, base_path(DEVICE) );
    format_opt = atoi( argv[3] );
    if( format_opt < 1 || format_opt > 4 ) {
       exit();
    }

    init_dgodm();        /* initialize ODM */
    diag_asl_init(ASL_INIT_DEFAULT);
    fdes = diag_catopen(MF_KAZFOR,0);

    /* Attempt to configure the device if it is not already configured */
    state = configure_device(DEVICE_NAME);
    if (state == -1) {
        rc = diag_msg(0x802249,fdes,CONFIG_FAIL,CONFIG_FAIL_TITLE,DEVICE_NAME);
        clean_up();
    }

    /* Try to open the Device Driver. If it fails, quit  */
    if ((fd = openx(DEVICE, O_RDWR, 0, 1)) < 0) {
        rc = diag_msg(0x802249,fdes,OPEN_FAIL,OPEN_FAIL_TITLE,DEVICE_NAME);
        clean_up();
    }

    err_count = 0;
    percent = 0;
    fail = 0;

    switch (format_opt) {
        case 2 : rc = diag_display(0x802441, fdes, long_3, DIAG_IO,
                                   ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, 
                                   q_long_3);
                 check_rc_quit(rc);
                 if (rc == DIAG_ASL_COMMIT)
                     if (DIAG_ITEM_SELECTED(menutype) == 2)
                         clean_up();

        case 3 :
        case 1 : rc = diag_display(0x802440, fdes, destroy, DIAG_IO,
                                   ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, 
                                   q_destroy);
                 check_rc_quit(rc);
                 if (rc == DIAG_ASL_COMMIT)
                     if (DIAG_ITEM_SELECTED(menutype) == 2)
                         clean_up();

                 if ((format_opt == 1) || (format_opt == 3))
                     badisk_ioctl_parms.format_options = BAFUPI | BAUSDM;
                 else
                     badisk_ioctl_parms.format_options = BAPESA | BAFUPI | BAUSDM;
                 check_ioctl(ioctl(fd,BAFORMAT,&badisk_ioctl_parms)); /*format*/
		 pvid_destroyed = 1;
                 i = 0;
                 r = -1;
                 while (ioctl(fd,BACCSTAT,&arg_stat) != 0) {
                     if (i != r) {
                         rc = diag_msg_nw(0x802442, fdes, STANDBY, STITLE,
                                          DEVICE_NAME, LOCATION, i);
                         if (r != -1) 
                             i = r;
                     }
                     else
                         rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,FALSE,NULL);
                     check_rc1(rc);
                     check_ioctl(ioctl(fd,BAFMTPROG,&arg_prog));
                     r = arg_prog.curr_cylinder;
                 }
                 if ((arg_stat.cmd_status == 0x01)||
                    (arg_stat.cmd_status == 0x03)|| 
                    (arg_stat.cmd_status == 0x05)) {
                     if (format_opt != 3) {
                         rc = diag_msg(0x802242,fdes,COMPLETE,COMPLETE_TITLE,
                                       DEVICE_NAME);
                     }
                 }
                 else {
                     fail = 1;
		     if((format_opt == 2) || (format_opt == 3))
	                     rc = diag_msg(0x802243,fdes,FAIL,
				FAIL_TITLE,DEVICE_NAME);
		     else
       		              rc = diag_msg(0x802243,fdes,FAIL,
				FAIL_TITLE2,DEVICE_NAME);
                 }
                 if ((format_opt != 3) || (fail == 1))
                     break;
        case 4 : rc = diag_msg_nw(0x802246,fdes,VER_STDBY,VTITLE,DEVICE_NAME,
                                  LOCATION);
                 check_rc2();
                 rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,FALSE,NULL);
                 check_rc2();
                 badisk_ioctl_parms.milli_secs = 200000;
                 badisk_ioctl_parms.diag_test = BADIAG_READV;
                 check_ioctl(ioctl(fd,BADIAGTEST,&badisk_ioctl_parms));
                 if (ioctl(fd,BAWAITCC,&badisk_ioctl_parms)!=0) {
                     check_ioctl(ioctl(fd,BAABORT,&arg_for));
                     rc = diag_msg(0x802244,fdes,V_ABORT,V_ABORT_TITLE,DEVICE_NAME);
                     clean_up();
                 }
                 check_ioctl(ioctl(fd,BACCSTAT,&arg_stat));
                 if ((arg_stat.cmd_status == 0x01) ||
                     (arg_stat.cmd_status == 0x03) ||
                     (arg_stat.cmd_status == 0x05)) {
                     rc = diag_msg(0x802447,fdes,V_COMPLETE,V_COMPLETE_TITLE,
                                   DEVICE_NAME);
                 }
                 else {
                     rc = diag_msg(0x802245,fdes,V_FAIL,V_FAIL_TITLE,DEVICE_NAME);
                     clean_up();
                 }
                 break;
        default: break;
    } /* end switch format_opt */

    clean_up();

} /* end main */


/*
* NAME: clean_up
*                                                                    
* FUNCTION: Closing file descriptors and return to diagnostic controller.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and tu_test.
*                                                                   
* RETURNS: NONE
*/

clean_up()
{
    if (fd > 0)
        close(fd);
    /* Clean up data base if performed format operation  */
    if((format_opt != 4) && pvid_destroyed)
	clear_pvid(DEVICE_NAME);
    initial_state(state,DEVICE_NAME);
    diag_asl_quit();    /* close ASL */
    term_dgodm();       /* close ODM */
    if(DEVICE != (char *) 0)
	free( DEVICE );
    if(LOCATION != (char *) 0)
	free( LOCATION );
    if(DEVICE_NAME != (char *) 0)
	free( DEVICE_NAME );
    exit(0);
} /* end clean_up */


/*
* NAME: check_rc1
*                                                                    
* FUNCTION: Checks if the user has entered the Esc or Cancel key while a screen
*           is displayed.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: rc, the input parameter
*/

int 
check_rc1(rc)
    int             rc;			/* user's input */
{
    if ((rc == DIAG_ASL_CANCEL) || (rc == DIAG_ASL_EXIT)) {
        rc = diag_display(0x802444, fdes, confirm, DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, 
                          q_confirm);
    
        if (rc == DIAG_ASL_COMMIT)
             switch (DIAG_ITEM_SELECTED(menutype)) {
                 case 1: check_ioctl(ioctl(fd,BAABORT,&arg_for));
                         rc = diag_msg(0x802343,fdes,CANCEL,CANCEL_TITLE,
                                       DEVICE_NAME);
                         clean_up();
                         break;
                 case 2: rc = diag_msg(0x802345,fdes,RESUME,RESUME_TITLE,
                                       DEVICE_NAME);
                         break;
                 default:break;
             }
    }
    return (rc);
} /* end check_rc1 */

/*
* NAME: check_rc2
*                                                                    
* FUNCTION: Checks if the user has entered the Esc or Cancel key while a screen
*           is displayed.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: rc, the input parameter
*/

int 
check_rc2(rc)
    int             rc;			/* user's input */
{
    if ((rc == DIAG_ASL_CANCEL) || (rc == DIAG_ASL_EXIT)) {
        rc = diag_display(0x802449, fdes, v_confirm, DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, 
                          q_v_confirm);
    
        if (rc == DIAG_ASL_COMMIT)
             switch (DIAG_ITEM_SELECTED(menutype)) {
                 case 1: check_ioctl(ioctl(fd,BAABORT,&arg_for));
                         rc = diag_msg(0x802448,fdes,V_CANCEL,V_CANCEL_TITLE,
                                       DEVICE_NAME);
                         clean_up();
                         break;
                 case 2: rc = diag_msg(0x802346,fdes,V_RESUME,V_RESUME_TITLE,
                                       DEVICE_NAME);
                         break;
                 default:break;
             }
    }
    return (rc);
} /* end check_rc2 */

/*
* NAME: check_rc
*                                                                    
* FUNCTION: Checks if the user has entered the Esc or Cancel key while a screen
*           is displayed.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: rc, the input parameter
*/

int 
check_rc(rc)
    int             rc;			/* user's input */
{
    if ((rc == DIAG_ASL_CANCEL) || (rc == DIAG_ASL_EXIT)) {
        rc = diag_msg(0x802344,fdes,CANCEL,CANCEL_TITLE,
                      DEVICE_NAME);
        clean_up();
    }
    return (rc);
} /* end check_rc */

/*
* NAME: check_rc_quit
*                                                                    
* FUNCTION: Checks if the user has entered the Esc or Cancel key while a screen
*           is displayed.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: rc, the input parameter
*/

int 
check_rc_quit(rc)
    int             rc;			/* user's input */
{
    if ((rc == DIAG_ASL_CANCEL) || (rc == DIAG_ASL_EXIT)) {
        rc = diag_msg(0x802347,fdes,QUIT,QUIT_TITLE,DEVICE_NAME);
        clean_up();
    }
    return (rc);
} /* end check_rc_quit */

/*
* NAME: check_ioctl
*                                                                    
* FUNCTION: Checks if the ioctl call is successful. At this point, no checking
*           is necessary.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: None
*/

int 
check_ioctl(arg)

	int arg;
{
}

/*
 * NAME: base_path
 *
 * FUNCTION: Return a pointer into the input string that points to the
 *	last word.  Words are separated by "/".
 *	Used to determine execution program name.
 *
 * RETURNS: pointer to last word in input string.
 */

char           *
base_path(source)		/* just get base of the name	 */
	char           *source;
{
	int             source_len;	/* length of source */
	char           *dest;	/* will point to base of name */

	source_len = strlen(source);
	dest = source + source_len - 1;
	if (*dest == '/')
		return (0);
	if (source_len < 2)
		return (dest);
	while (--source_len && (*(dest - 1) != 'r'))
		--dest;
	return (dest);
}
/*  */
/*
 * NAME:  clear_pvid
 *
 * FUNCTION: Perform clean up of data base to clear pvid
 *
 * NOTES: 	This routine must be able to page fault
 *
 * RETURNS: None
 *
*/
void
clear_pvid(device_name)
char	*device_name;
{
	struct	CuAt *pvidattr;
	int	howmany;


	if((pvidattr=(struct CuAt *)getattr(device_name,"pvid", FALSE, &howmany)
			)==NULL)
		return;
	strcpy(pvidattr->value,"none");
	(void)putattr(pvidattr);
	return;

}
