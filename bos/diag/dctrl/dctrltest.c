static char sccsid[] = "@(#)13	1.27.2.29  src/bos/diag/dctrl/dctrltest.c, dctrl, bos41J, 9514A_all 3/29/95 14:37:52";
/*
 * COMPONENT_NAME: (CMDDIAG) Diagnostic Controller
 *
 * FUNCTIONS:   testnormal
 *              testdevice
 *              testsystem
 *              testone
 *              run_pretests
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
#include <nl_types.h>
#include <signal.h>
#include <ctype.h>
#include <sys/mdio.h>
#include "diag/dcda_msg.h"
#include "diag/diag.h"
#include "diag/da.h"
#include "diag/tmdefs.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "diag/class_def.h"
#include "dctrl_msg.h"
#include "dctrl.h"
#include "diag/diag_define.h"

#define   da_exit_key      (da_rc.field.user==DA_USER_EXIT)
#define   da_cancel_key    (da_rc.field.user==DA_USER_QUIT)
#define   da_cont          (da_rc.field.more==DA_MORE_CONT)
#define   menu_exit_key    (rc==DIAG_ASL_EXIT)
#define   menu_cancel_key  (rc==DIAG_ASL_CANCEL)
#define	  TAPE_ERROR_LED   0xA43
#define	  TAPE_READ_LED	   0xA40


/* GLOBAL VARIABLES */
int     lmflg;                          /* loop mode flag               */
short   lcount;                         /* loop count                   */
int     lerror;                         /* number of errors in loop     */
int     num_dev_to_test;                /* number of devices to test    */
int     memory_da_tested;               /* memory da has been run flag  */
int     basetestflg;                    /* base system test             */
char    command[512];
char    *com;
union {
        unsigned char   all;
        da_returncode_t field;
} da_rc;

/* EXTERNALLY DEFINED VARIABLES */
extern int diag_mode;                   /* diagnostic test mode         */
extern int consoleflg;
extern int missingflg;
extern int next_diskette;
extern int advancedflg;
extern int customerflg;
extern int exenvflg;
extern int systestflg;
extern int deviceflg;
extern int pretest;
extern int ttymode;
extern int displayflg;
extern int diag_ipl_source;
extern int diag_source;
extern int present_dm_menu;
extern int present_tm_menu;
extern int num_All;
extern unsigned int mach_model;
extern nl_catd fdes;
extern diag_dev_info_ptr_t     *All;
extern diag_dev_info_ptr_t     *test_array;
extern char devicename[];
extern char *dadir;
extern char *bootdev;
extern int showleds;

/* CALLED FUNCTIONS */
int              disp_dc_error();
diag_dev_info_t  *pop_device();
diag_dev_info_t  *find_dev();
int		restore_files(char *);

/* FUNCTION PROTOTYPES */
int testnormal(void);
int testdevice(int, int);
void set_LED_buff(diag_dev_info_t *);
void fill_LED_buffer(short *, int, int);
int blink_LED(short);
void show_LED(short, int);
int testone(int *);
void testsystem(void);
int run_pretests(void);
int report_fru_byled(void);
int load_diag_kext(char *);
short convert_LED_value(char, int);
void search_alt_device(int *);

/*  */
/*
 * NAME: testnormal
 *
 * FUNCTION: main test loop for devices
 *
 * NOTES:
 *
 * RETURNS:
 *
 */

int testnormal(void)
{
        int rc;
        int finished = 0;
        int exitrc = 0;
        int dchosen = 0;            /* diagnostic test chosen       */
        int cdevice;
        int bdevice;
        int next;
        diag_dev_info_t         *dev_ptr;
        int xdiag=DIAG_FALSE;
        char *xptr;
        char xchar[2];

        lcount = 0;

        while (!finished){
                /* Display selection menu if not in system test mode */
                /*  and not testing a particular device.             */
                basetestflg = 0;
                if ( !systestflg && !deviceflg ) {
                        rc = disp_ds_menu(&dchosen);
                        if ( menu_exit_key || menu_cancel_key)
                                break;
                }
                else if ( deviceflg )
                        if(rc = testone(&dchosen))
                                return(rc);

                if((diag_ipl_source!=IPLSOURCE_DISK) && (next_diskette==DIAG_TRUE)){
			rc = prompt_for_diskette(fdes);
			if (menu_exit_key)
				break;
			else if (menu_cancel_key){
				break;
			}
                        /* Exit controller with return code set to 98   */
			/* This is done to return control back to rc.boot */
			/* which will turn around and invoke the controller */
			/* again to regenerate DSMenu and Top.              */
			genexit(DIAG_EXIT_DCTRL_RELOAD);
		}

                if ( ( (present_dm_menu == DIAG_TRUE) || deviceflg)
                                && !(diag_mode & DMODE_ELA) ){
                        rc = disp_dm_menu(&diag_mode);
                        if (menu_exit_key)
                                break;
                        else if (menu_cancel_key){
                                if ( deviceflg )
                                        break;
                                if ( systestflg )
                                        systestflg = 0;
                                continue;
                        }
                }
                present_dm_menu = DIAG_TRUE;

                /* if Advanced mode and standalone from hardfile     */
                /*                                 - support looping */
                /* if using hidden -l flag         - support looping */
                if ( (present_tm_menu == DIAG_TRUE) ||
                   ((exenvflg == EXENV_STD) && (advancedflg == ADVANCED_TRUE) &&
                    !deviceflg && (diag_ipl_source == IPLSOURCE_DISK) )) {
                        rc = disp_tm_menu(&lmflg);
                        if (menu_exit_key)
                                break;
                        else if (menu_cancel_key) {
                                if ( systestflg )
                                        systestflg = 0;
                                continue;
                        }
                }
                else lmflg = LOOPMODE_NOTLM;

                cdevice = lerror = 0;
                if ((bdevice=num_dev_to_test=stack_devices(All,
                                        num_All,dchosen)) < 0) {
                        disp_dc_error( ERROR7, NULL);
                        return(-1);
                }

                /* check for device that supports ELA in conc mode only */
                dev_ptr = pop_device(cdevice);
                if ((exenvflg == EXENV_CONC) && (diag_mode != DMODE_PD) &&
                                !systestflg && !basetestflg && consoleflg &&
                                (dev_ptr->T_PDiagDev->Conc == 2) ) {
                        rc = disp_ela_pd_menu(dev_ptr);
                        if (menu_exit_key)
                                break;
                        else if (menu_cancel_key)
                                continue;
                }

                while (!finished){
                        /* if an error occurs in system checkout, skip the
                         * children of the failing device and go on to the
                         * next device.
                         */
                        if ( cdevice == 0 ) {
                                memory_da_tested = DIAG_FALSE;
                                dev_ptr = pop_device(cdevice);
                        }
                        else if ( systestflg &&
                                   (dev_ptr->T_CDiagDev->State == STATE_BAD)) {
                                find_next_non_dependent(--cdevice,&next,
                                                        test_array,
                                                        num_dev_to_test);
                                cdevice = next;
                                dev_ptr = pop_device(cdevice);
                        }
                        /* else get the next device in the list to test */
                        else
                                dev_ptr = pop_device(cdevice);

                        /* If in concurrent mode, see if invoked from X-window */
                        if (exenvflg == EXENV_CONC) {
                                if((xptr = (char *) getenv("X_DIAG")) == (char *) NULL)
                                        xdiag = DIAG_FALSE;
                                else {
                                        strncpy(xchar, xptr, 1);
                                        if (xchar[0] == '0')
                                                xdiag = DIAG_FALSE;
                                        else
                                                xdiag = DIAG_TRUE;
                                }
			} else
				xdiag = DIAG_FALSE;

                        /* If invoked from X-window, do not test HFT devices */
                        if (dev_ptr && ((dev_ptr->T_PDiagDev->SupTests & SUPTESTS_HFT)!=0) &&
                          (xdiag == DIAG_TRUE)) {
                                if (ttymode)
                                        disp_no_test_printf(ERROR13, dev_ptr->T_CuDv->name);
                                cdevice++;
                                if (cdevice >= bdevice){
                                        if (lmflg==LOOPMODE_ENTERLM) {
                                                cdevice = 0;
                                                if( ++lcount < 0)
                                                        lcount = 0;
                                                lmflg=LOOPMODE_INLM;
                                        }
                                        else if (lmflg==LOOPMODE_INLM) {
                                                cdevice = 0;
                                                if( ++lcount < 0)
                                                        lcount = 0;
                                        }
                                        else finished = (lmflg==LOOPMODE_NOTLM)
                                                || (lmflg==LOOPMODE_EXITLM);
                                }
                                continue;
                        }


                        /* test the device */
                        if((exitrc = testdevice(cdevice,DIAG_FALSE)) == -1)
                                return(exitrc);

                        /* if not looping or if first pass of a loop and
                         * the user has not pressed the exit key - then
                         * check to see if highest level test was run.
                         */
                        if (((lmflg==LOOPMODE_NOTLM)||(lmflg==LOOPMODE_ENTERLM))
                                        && !(da_cancel_key || da_exit_key)
                                        &&  ( cdevice != num_dev_to_test)
                                        &&  consoleflg ) {
                                while ( more_tests(dev_ptr) )   {
                                        /* display pr menus, free up resource */
                                        rc = disp_more_tests(dev_ptr);
                                        if (menu_exit_key){
                                                finished = 1;
                                                break;
                                        }
                                        else if (menu_cancel_key) {
                                                finished = 1;
                                                rc = 1;
                                                break;
                                        }
                                        else if((exitrc = testdevice(cdevice,
                                                         DIAG_FALSE))==-1)
                                                return(exitrc);
                                }
                        }

                        if (finished || da_exit_key){
                                finished = 1;
                                break;
                        }

                        if ((da_cancel_key) && (lmflg!=LOOPMODE_EXITLM) ) {
                                if ((lmflg==LOOPMODE_ENTERLM) ||
                                    (lmflg==LOOPMODE_INLM))  {
                                        if (lmflg==LOOPMODE_ENTERLM)
                                                bdevice = cdevice;
                                        cdevice = 0;
                                        if( ++lcount < 0)
                                                lcount = 0;
                                        lmflg = LOOPMODE_EXITLM;
                                }
                                else {
                                        finished = 1;
                                        break;
                                }
                        }

                        else if (da_cont || systestflg || basetestflg ||
                                                (lmflg != LOOPMODE_NOTLM))  {
                                cdevice++;
                                if (cdevice >= bdevice){
                                        if (lmflg==LOOPMODE_ENTERLM) {
                                                cdevice = 0;
                                                if( ++lcount < 0)
                                                        lcount = 0;
                                                lmflg=LOOPMODE_INLM;
                                        }
                                        else if (lmflg==LOOPMODE_INLM) {
                                                cdevice = 0;
                                                if( ++lcount < 0)
                                                        lcount = 0;
                                        }
                                        else finished = (lmflg==LOOPMODE_NOTLM)
                                                || (lmflg==LOOPMODE_EXITLM);
                                }
                        }
                        else finished = (lmflg==LOOPMODE_NOTLM) ||
                                       ((lmflg==LOOPMODE_EXITLM) &&
                                        (cdevice == bdevice));
                }
        }
        return((menu_exit_key || menu_cancel_key || da_exit_key) ? 1 : exitrc);
}

/*   */
/*
 * NAME: testdevice
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS:
 *      0 : no error
 *     -1 : error
 *
 */


int testdevice(
        int cdevice,
        int enclda_flag)                        /* enclosure da name flag           */
{

#define DA_RC_DEFAULT 01                /* good status, more cont, no tests */

        diag_dev_info_t *device;
        int     child_status;
        int     fd, rc = 0;
	int	diagex_rc;
	char	*altdir;
        char    *errbuf;
        char    *options[2];
        char    criteria[128];
        char    new_DA[128];
        static struct   fru_bucket frub;
	char cmd[256];

        if ( cdevice == num_dev_to_test)
                return(0);

        if ((device = pop_device(cdevice)) == (diag_dev_info_t *)NULL ) {
                disp_dc_error( ERROR1, "");
                return(-1);
        }

        /* return if this is not a supported device */
        if ( device->T_PDiagDev == NULL )
                return(0);

        /* Set up path to DA directory */
        strcpy(command,dadir);
        com = command + strlen(dadir);
        *(com++) = '/';

        /* populate the TMInput object class for the DA */
        if (putdainput(cdevice))  {
                disp_dc_error( ERROR6, "TMInput");
                return(-1);
        }

        if (enclda_flag)
                strcpy(com, device->T_PDiagDev->EnclDaName);
        else {
                /************************************************************
                 If the DA name starts with an '*' then we must locate the
                 name of the proper executable.  
                ************************************************************/
                if (device->T_PDiagDev->DaName[0] == '*')
                  {
			if ( (determine_DA( &new_DA, device )) == 0 )
                  		strcpy(com, new_DA);
                  }

                /*******************************
                 else DA name is name of executable
                *******************************/
                else {
                  strcpy(com, device->T_PDiagDev->DaName);
		}
        }

        /* if running pretest, but there is no DA, return */
        if ( (pretest || displayflg) && !file_present(command) )
                return(0);

        /* verify that there is a da to call */
        da_rc.all = DA_RC_DEFAULT;

        if ( strlen( device->T_PDiagDev->DaName ) && ( missingflg ||
             !(!strcmp( device->T_PDiagDev->DaName, "dmemory") &&
                (memory_da_tested == DIAG_TRUE) ) ) )  {
                if (!strcmp( device->T_PDiagDev->DaName, "dmemory"))
                        memory_da_tested = DIAG_TRUE;

                /* set LEDs */
                set_LED_buff(device);

                /* Run slibclean to avoid lots of memory fragmentation */
                /* and help stand alone package .                      */
                if(diag_ipl_source != IPLSOURCE_DISK)
		    {
				sprintf(cmd,"%s 1>/dev/null 2>&1",SLIBCLEAN);
                        (void)system(cmd);
		    }

		/* If running in stand alone, and the file does not    */
		/* exist and it is on supplemental diskettes, then     */
		/* prompt the user to insert the diskette.	       */

		if( (diag_ipl_source != IPLSOURCE_DISK) &&
			!file_present(command) &&
			(!strcmp(device->T_PDiagDev->Diskette,"S") ||
			 !strcmp(device->T_PDiagDev->Diskette,"3S")) ){

			rc = prompt_for_diskette(fdes);
			if (menu_exit_key)
				return (0);
			else if (menu_cancel_key){
				return (0);
			}
                        /* Exit controller with return code set to 98   */
			/* This is done to return control back to rc.boot */
			/* which will turn around and invoke the controller */
			/* again to regenerate DSMenu and Top.              */
			genexit(DIAG_EXIT_DCTRL_RELOAD);
		}

                /* If running off TAPE, restore files needed to run DA */
                if( (diag_ipl_source == IPLSOURCE_TAPE) && 
			!file_present(command) ){
			if(consoleflg)
				diag_msg_nw(READING_DSKT, fdes, DISKETTE3, 
					MSG_DISK_6, bootdev);
			else if (systestflg && !consoleflg)
				setleds(TAPE_READ_LED);

                        rc=restore_files(device->T_PDiagDev->Diskette);
			setleds(0xfff);
			if(rc != 0)
				if(consoleflg){
					(void)diag_msg(READING_DSKT, fdes,
					   	DISKETTE3, MSG_DISK_7);
					return(-1);
				} else if (systestflg && ! consoleflg){
					setleds(TAPE_ERROR_LED);
					while(1)
					  ;
				}
		}
		/* Check to see if diagnostic application requires the 
		   loading of the diagnostic kernel extension. If so
		   load it before invoking the diagnostic app.
		*/
                if (device->T_PDiagDev->SupTests & SUPTESTS_DIAGEX)
		{
			diagex_rc=load_diag_kext(device->T_CuDv->PdDvLn_Lvalue);
			if ( diagex_rc )
				return (-1);
		}
		/* If running from hard file using the CDROM diagnostics */
		/* and the file is not present, then it may be on the    */
		/* hard file. Check value of ALT_DIAG_DIR, and rebuild   */
		/* path of command.					 */

		if( (diag_source == IPLSOURCE_CDROM) &&
				!file_present(command))

	                if((altdir = (char *) getenv("ALT_DIAG_DIR")) != 
					(char *) NULL)

			        /* Set up path to Alternate DA directory */

				sprintf(command, "%s/da/%s", altdir, com);

		/* Set up argv arguments for execute */
		options[0] = command;
		options[1] = (char *) NULL;
		options[2] = (char *) NULL;
                if (consoleflg){
                        rc = diag_asl_execute(command,options,&child_status);
                        if ( (child_status & 0xFF) || (rc < 0) ) {
                                da_rc.all = 0;
                                da_rc.field.error = DA_ERROR_OTHER;
                                switch (device->T_PDiagDev->SupTests) {
                                        case (SUPTESTS_SHR):
                                             da_rc.field.tests = DA_TEST_SHR;
                                             break;
                                        case (SUPTESTS_SUB):
                                             da_rc.field.tests = DA_TEST_SUB;
                                             break;
                                        case (SUPTESTS_FULL):
                                             da_rc.field.tests = DA_TEST_FULL;
                                             break;
                                        default:
                                             da_rc.field.tests = DA_TEST_FULL;
                                             break;
                                }
                        }
                        else
                                da_rc.all = child_status >> 8;
                }
                else {
                        /* if ttymode */
                        if ( ttymode )
                                printf("%s %s\n",
                                        diag_cat_gets(fdes, INFOSET, TMSGT),
                                         device->T_CuDv->name);
                        rc = diag_execute(command, options, &child_status);
                        if ((child_status & 0xFF) || (rc < 0)) {
                                da_rc.all = 0;
                                da_rc.field.error = DA_ERROR_OTHER;
                        }
                        else
                                da_rc.all = child_status>>8;

                        /* Turn LED off  */
                        show_LED( 0xFFF, 0 );
                }



                if ( da_rc.all == DIAG_BADRC) return(-1);
        }


        /* check status field for any hardware problems */
        switch ( da_rc.field.status )  {
                case    DA_STATUS_BAD:
                        device->T_CDiagDev->State = STATE_BAD;
                        lerror ++;
                        break;
                case    DA_STATUS_GOOD:
                        if ((lmflg==LOOPMODE_NOTLM)||(lmflg==LOOPMODE_ENTERLM))
                                device->T_CDiagDev->State = STATE_GOOD;
                        break;
        }

        /* set the test level that the da accomplished */
        switch ( da_rc.field.tests )  {
                case    DA_TEST_FULL:
                        device->T_CDiagDev->TstLvl = TSTLVL_FULL;
                        break;
                case    DA_TEST_SUB:
                        device->T_CDiagDev->TstLvl = TSTLVL_SUB;
                        break;
                case    DA_TEST_SHR:
                        device->T_CDiagDev->TstLvl = TSTLVL_SHR;
                        break;
                default: /* DA_TEST_NOTEST */
                        device->T_CDiagDev->TstLvl = TSTLVL_NOTEST;
        }

        device->T_CDiagDev->More   = da_rc.field.more;
        device->flags.device_tested = DIAG_TRUE;

        /* if no software errors were encountered, then indicate that the
         * device was actually tested */
        switch ( da_rc.field.error )  {
                case DA_ERROR_NONE:
                        if ( !systestflg && !basetestflg )
                                device->Asterisk = 'T';
                        device->flags.device_driver_err = DIAG_FALSE;
                        break;
                case DA_ERROR_OPEN:             /* additional testing   */
                        /* present device busy menu                     */
                        device->flags.device_driver_err = DIAG_TRUE;
                        device->T_CDiagDev->State = STATE_GOOD;
                        device->T_CDiagDev->TstLvl = TSTLVL_NOTEST;
                        break;
                case DA_ERROR_OTHER:            /* disp predefined SRN */
                        device->flags.device_driver_err = DIAG_FALSE;
                        if ( device->T_CDiagDev->State != STATE_BAD ) {
                                device->T_CDiagDev->State = STATE_BAD;
                                lerror++;       /* increment error count */
                        }
                        strcpy( frub.dname, device->T_CuDv->name);
                        frub.ftype = FRUB1;
                        frub.rcode = device->T_Pdv->led;
                        frub.sn = DC_SOURCE_SOFT;
                        frub.rmsg = DC_SOFT_MSG;
                        frub.frus[0].conf = 0;
                        addfrub( &frub );
                        break;
        }
        return(0);
}

/*   */
/*
 * NAME: determine_DA
 *
 * FUNCTION: If the DA name starts with an '*' then we must locate the
 *           name of the proper executable.  Using the string following
 *           the '*' as an attribute , get the CuAt with the proper
 *           CuDv->name for this device.  Using the CuAt->value field
 *           as an attribute with the proper PDiagDev->DType for this
 *           device, get the PDiagAtt stanza whose value will be the
 *           proper executable name.  In case of error, get the
 *           "default" PDiagAtt stanza.
 *	
 * NOTES:
 *
 * RETURNS:
 *
 */

int
determine_DA( char *DA_name, diag_dev_info_t *device )
{
	int rc=0, cnt=0;
        struct CuAt *cuat;
	char crit[128];
 	struct  listinfo c_info, p_info;
        struct  CuDv    *parent_cudv;
        struct  PDiagAtt    *pdiagatt;


	/* set this to NULL if case of error */
	*(DA_name) = (char)NULL;

        if (!strncmp("parent_type", device->T_PDiagDev->DaName + 1, 11)) {

		/* first get the parent's Class/Subclass/Type */
		sprintf(crit, "name = %s", device->T_CuDv->parent);
		parent_cudv = (struct CuDv *)diag_get_list(CuDv_CLASS, crit,
                                &c_info, 1,2);
                if( (parent_cudv == (struct CuDv *)-1) ||
                    (parent_cudv == (struct CuDv *)NULL) )
                        return(-1);

                /* Now get PDiagAtt for the device under test */
                sprintf(crit,
                     "value=%s AND DType=%s AND DSClass=%s AND DClass=%s",
                        parent_cudv->PdDvLn_Lvalue,
			device->T_PDiagDev->DType,
			device->T_PDiagDev->DSClass,
			device->T_PDiagDev->DClass );
                pdiagatt = (struct PDiagAtt *)diag_get_list(PDiagAtt_CLASS,
                                crit, &p_info, 1, 1);

                if( (pdiagatt == (struct PDiagAtt *) -1) ||
                    (pdiagatt == (struct PDiagAtt *) NULL) )
			return(-1);
		strcpy(DA_name, pdiagatt->DApp);
	        diag_free_list(parent_cudv, &c_info);
                diag_free_list(pdiagatt, &p_info);
		return(0);
	}
        if (!strncmp("modelcode", device->T_PDiagDev->DaName + 1, 9))
                cuat = (struct CuAt *)getattr("sys0", "modelcode", 0, &cnt);
   	else
		/* might be different attribute like *_subtype */
        	cuat = (struct CuAt *)getattr(device->T_CuDv->name,
                                device->T_PDiagDev->DaName + 1, 0, &cnt);

        if (((void * )cuat == NULL) || (cnt != 1))
               	rc = -1;
        else
               	rc = get_diag_att(device->T_PDiagDev->DType,
                                cuat->value, 's', &cnt, DA_name);

	/* If no modelcode attribute, or error, get the default */
        if (rc < 0)
                rc = get_diag_att(device->T_PDiagDev->DType,
                                "default", 's', &cnt, DA_name);
       	if (rc < 0) return(-1);
	else return (0);

}

/*   */
/*
 * NAME: set_LED_buff
 *
 * FUNCTION:  This routine converts the SRN/FRU data into a led
 *		structure for use by the set_LED function. 
 * NOTES:
 *
 * RETURNS:
 *
 */

void set_LED_buff(diag_dev_info_t *device)
{
        int i1, k1, k2;
        short lval;
        char *tmp;
        static short leds[18] = {
                                        0x8880, 0x1020, /* System Abend */
                                        0x0000, 0x0000, /* Abend Data   */
                                        0xAAA0,         /* New FRU      */
                                        0x1030, 0x8040, /*              */
                                        0x0000, 0xA010, /* FRU, LOC KEY */
                                        0x1000, 0x2000, /* LOC 1 & 2    */
                                        0x3000, 0x4000, /* LOC 3 & 4    */
                                        0x5000, 0x6000, /* LOC 5 & 6    */
                                        0x7000, 0x8000, /* LOC 7 & 8    */
                                        0xFFF0          /* Termination  */
                                };

        /* Check to see if there is a need to display the LEDs */
        if(showleds == 0)
                return;

        leds[7] = device->T_Pdv->led<<4;

        tmp = device->T_CuDv->location;
        for (i1=9, k1=0, k2=1; k1<strlen(tmp); k1++)
          if ((lval = convert_LED_value(*(tmp + k1), k2)) != -1)
            {
            leds[i1++] = lval;
            k2++;
            }
        leds[i1] = 0xFFF0;

	/* call set_LED function to write data to NVRAM, if applicable */
        fill_LED_buffer( &leds[0], sizeof (leds), consoleflg );

        return;

}

/*   */
/*
 * NAME: testone
 *
 * FUNCTION: Return device index for device to test 
 *
 * NOTES:
 *
 * RETURNS: 0
 *          -2 if running in X
 *          -3 if device is not found
 *	    -4 if device does not have PDiagDev
 *
 */

int testone(int *device_index)
{
        diag_dev_info_t *dev_ptr;
        int i, alt_device_index;
        int xdiag;
        char *xptr;
        char xchar[2];

        /* verify device is in test table and obtain it's index value */
        for(i=0; i < num_All; i++)
                if (!strcmp(devicename, All[i]->T_CuDv->name))
                        break;

        /* if not in table - search for 'alt_da' attribute. */
	/* Only when not found, will an error be returned.    */
	/* This algorithm is used to handle devices that are  */
	/* not logically grouped under the System Unit tree,  */
	/* instead, their parents have separate Config Rules. */

        if ( i >= num_All )  {
	  	search_alt_device(&alt_device_index);
		if(alt_device_index >= num_All){
	                disp_dc_error( ERROR2, devicename);
			return(-3);
		}
		i=alt_device_index;
        }

        /* if in concurrent mode, see if invoked from X-window */
        if (exenvflg == EXENV_CONC) {
                if ((xptr = (char *) getenv("X_DIAG")) == (char *) NULL)
                        xdiag=DIAG_FALSE;
                else {
                        strncpy(xchar, xptr, 1);
                        if(xchar[0] == '0')
                                xdiag = DIAG_FALSE;
                        else
                                xdiag = DIAG_TRUE;
                }
        }

	/* Check to see if device has diagnostics support */

	if( (All[i]->T_PDiagDev == (struct PDiagDev *)NULL) ||
	    (All[i]->T_PDiagDev == (struct PDiagDev *)-1) ) {
                disp_dc_error( ERROR17, NULL);
		return(-4);
	}

        /* If invoked from X-window & device not supported in X-window, exit */
        if (((All[i]->T_PDiagDev->SupTests & SUPTESTS_HFT) != 0) &&
                (xdiag == DIAG_TRUE)) {
                        if(ttymode)
                                disp_no_test_printf(ERROR13, devicename);
                        else
                                diag_emsg(fdes, ESET, ERROR14, devicename);
                        return(-2);
        }

        *device_index = i;
        return(0);
}


/*
 * NAME: testsystem
 *
 * FUNCTION: Performs system test
 *
 * NOTES:
 *
 * RETURNS: NONE
 *
 */

void testsystem(void)
{
        diag_mode = DMODE_REPAIR;
        testnormal();
        return;
}

/*  */
/*
 * NAME: run_pretests
 *
 * FUNCTION: Test the underlying displays.
 *
 * NOTES:
 *
 *      For each device in the concurrent configuration.
 *              If (PDiagDev.Pretest){
 *                      Stack Device/Device Path/Siblings.
 *                      Init LED Save Area for check stops.
 *                      Display LED (PdDv->led).
 *                      Call DA - no console mode.
 *                      Clear LED.
 *                      If (problem found){
 *                              While problem not isolated
 *                                      Advance to next device in stack
 *                                      Init LED Save Area.
 *                                      Display LED (PdDv->led).
 *                                      Call DA - no console mode.
 *                                      Clear LED.
 *                                      if (problem isolated){
 *                                              report FRU with LEDs.
 *                                      }
 *                      }
 *              }
 *
 * RETURNS:
 *      0 = NTF
 *      1 = A problem was found.
 *
 */

int run_pretests(void)
{

        int i, j, rc, fbtype;
        diag_dev_info_t *dev_to_report, *curr_dev;

        lmflg = LOOPMODE_NOTLM;
        diag_mode = DMODE_REPAIR;
        deviceflg++;
        missingflg = 0;
        lerror = 0;

        for (i=0; (i < num_All) && (lerror == 0); i++){
                if ( (All[i]->T_PDiagDev) && (All[i]->T_CuDv->chgstatus != 3) &&
                    (  (pretest && (All[i]->T_PDiagDev->PreTest == pretest)) ||
                       (!pretest && All[i]->T_PDiagDev->PreTest) ||
		       ( strlen(devicename) && (!strcmp(devicename,All[i]->T_CuDv->name))
		       	    && ( All[i]->T_PDiagDev->PreTest ||
				 !strncmp(devicename, "tty", 3) ) ) ) )
		{
                        num_dev_to_test = stack_devices( All, num_All, i );
                        for ( j = 0; j < num_dev_to_test; j++ ){
                                if ((rc = testdevice( j, DIAG_FALSE )) == -1)
                                        return(-1);
                                if (da_rc.field.more == DA_MORE_NOCONT)
                                        break;
                        }
                }
        }

        if (lerror == 0)
                return(0);

        /* find appropriate device to report in stack */
        dev_to_report = (diag_dev_info_t *) NULL;
        for (j=0; j < num_dev_to_test; j++ ){
                curr_dev = test_array[j];
                if(curr_dev->flags.device_tested == DIAG_TRUE){
                        /* if curr_dev is not sibling of dev_to_report */
                        if (sibling_check(curr_dev, dev_to_report)) {
                                fbtype = FRUB1;
                                if (curr_dev->T_CDiagDev->State == STATE_BAD)
                                        dev_to_report = curr_dev;
                        }
                        else if (curr_dev->T_CDiagDev->State == STATE_GOOD)
                                fbtype = FRUB2;
                }
        }

        return(report_fru_byled());
}

/*  */
/*
 * NAME: report_fru_byled
 *
 * FUNCTION: Reports a FRUB by LEDS.
 *
 * NOTES:
 *
 *      Init LED Save Area.
 *      report FRU with LEDs.
 *
 * RETURNS:
 *      0 = NTF
 *      1 = A problem was found.
 *
 */
#define NO_SRN  0x1050
#define NEW_FRU 0xAAA0          /* Displayed as "ccc" */

int report_fru_byled(void)
{

        int i1, k1, k2, k3;
        int led_i;
        int zero  = 0;
        short lval;
        char rcode[5];
        char *tmp, crit[256];
        diag_dev_info_t *fru_device;
        struct FRUB *T_FRUB;
        struct FRUs *T_FRUs;
        struct listinfo fb_info;
        struct listinfo fs_info;
        short leds[32];

        /* get FRU Bucket Header; put source #/reason code in leds */
        T_FRUB = (struct FRUB *)diag_get_list(FRUB_CLASS, "", &fb_info, 1, 1);
        if (T_FRUB == (struct FRUB *) -1)
          return(-1);

        leds[0] = 0x8880;
        leds[1] = 0x1030;
        led_i = 2;
        for (i1=0; i1<fb_info.num; i1++)
          {
          if (i1 > 0)
            {
            if (led_i + 2 > 31) continue;
            leds[led_i++] = NEW_FRU;
            leds[led_i++] = 0x1030;
            }

          if (T_FRUB[i1].sn == -1)
            {
            if ((led_i + 5) > 31) continue;
            leds[led_i++] = NO_SRN;
            sprintf(rcode, "%04X", T_FRUB[i1].rcode);
            leds[led_i++] = convert_LED_value(rcode[0], 0);
            leds[led_i++] = convert_LED_value(rcode[1], 0);
            leds[led_i++] = convert_LED_value(rcode[2], 0);
            leds[led_i++] = convert_LED_value(rcode[3], 0);
            }
          else
            {
            if ((led_i + 2) > 31) continue;
            leds[led_i++] = T_FRUB[i1].sn << 4;
            leds[led_i++] = T_FRUB[i1].rcode << 4;
            }

          /* get individual FRUs - put their locations in leds */
          sprintf(crit, "dname=%s and ftype=%d",
                T_FRUB[i1].dname, T_FRUB[i1].ftype);
          T_FRUs = (struct FRUs *)diag_get_list(FRUs_CLASS, crit,
			&fs_info, 4, 1);
          if (T_FRUs == (struct FRUs *) -1)
            return(-1);

          for (k1=0; k1 < fs_info.num; k1++)
            {
            if (!(tmp = T_FRUs[k1].floc))
              {
              /* find device in customized */
              fru_device = find_dev(T_FRUs[k1].fname);
              if (fru_device) tmp = fru_device->T_CuDv->location;
              }
            if (!tmp) continue;

            if (led_i + strlen(tmp) + 1 > 31) break;
            leds[led_i++] = 0xA000 | (k1+1)<<4;
            for (k2=0, k3=1; k2<strlen(tmp); k2++)
              if ((lval = convert_LED_value(*(tmp + k2), k3)) != -1)
                {
                leds[led_i++] = lval;
                k3++;
                }
            }
          if (k1 < fs_info.num) break;
          }

        leds[led_i] = 0xFFF0;

	/* call set_LED function to write data to NVRAM, if applicable */
        fill_LED_buffer( &leds[0], sizeof (leds), DIAG_TRUE );

	/* No return expected - unless LED support is not supported */
        return( blink_LED(0x888) );

}

/*  */
/*
 * NAME: search_alt_device
 *
 * FUNCTION: Obtain the new index to the device being tested.
 *	     
 * NOTES:  This function first searches the PDiagAtt attribute 'alt_da'
 *	   for the given device name. If the attribute does not exist,
 * 	   it will go to the device's parents, and get the attribute.
 *	   When an attribute is found, its value and the name of the
 *	   device having the attribute will be use to search the CuAt
 *	   object class for the name of the device, whose index will
 *	   be returned. This device has to be under the Sysplanar,
 *	   otherwise, this algorithm will not work.
 *	   Following is an example that relates to Disk Arrays:
 *	      1. diag -e -c -d hdisk0
 *	      2. hdisk0 has dar0 as the parent, which does not
 *               fall under the system unit tree (it has its own
 *		 ConfigRules).
 *	      3. dar0 has a PDiagAtt attribute of 'alt_da', whose
 *		 value is 'pri_controller'
 *	      4. get "name=dar0 and attribute=pri_controller" from
 *		 the CuAt.
 *	      5. Search All for the name taken from the value of
 *		 cuat found, then use the index as the alternate
 *		 index.
 *
 *	   When there is no CuDv, or no PDiagAtt found in the path,
 *	   the index will be set to num_All.
 *
 * RETURNS:
 *       index to the alternate device.
 *       num_All if not found.
 */

 void search_alt_device(int *alt_device_index)
{
	char	*psclass,*ptype,*pclass;
	char	crit[128];
	int	i,bc, rc;
	short	not_done=1;
	struct	listinfo c_info;
	struct	listinfo cuat_info;
	struct	listinfo p_info;
	struct	CuDv	*cudv;
	struct	CuAt	*cuat;
	struct	PDiagAtt *pdiagatt;

	*alt_device_index=num_All;
	sprintf( crit, "name = %s", devicename);

	do {
		cudv = (struct CuDv *)diag_get_list(CuDv_CLASS, crit,
				&c_info, 1,2);
		if( (cudv == (struct CuDv *)-1) ||
		    (cudv == (struct CuDv *)NULL) )
			return;
		ptype = (char *)substrg(PTYPE, cudv->PdDvLn_Lvalue);
		psclass = (char *)substrg(PSCLASS, cudv->PdDvLn_Lvalue);
		pclass = (char *)substrg(PCLASS, cudv->PdDvLn_Lvalue);

		/* Now get PDiagAtt given the ptype */
		sprintf(crit,
		     "attribute=%s AND DType=%s AND DSClass=%s AND DClass=%s",
			"alt_da", ptype, psclass, pclass);
		pdiagatt = (struct PDiagAtt *)diag_get_list(PDiagAtt_CLASS,
				crit, &p_info, 1, 1);

		/* If no PDiagAtt exists for the given device name */
		/* go to the parent to obtain PDiagAtt.            */

		if( (pdiagatt == (struct PDiagAtt *) -1) ||
		    (pdiagatt == (struct PDiagAtt *)NULL) )
			if(strcmp(cudv->parent, "")){
				sprintf(crit, "name = %s", cudv->parent);

				/* Prevent memory leak, free this cudv before */
				/* getting another one at the beginning of    */
				/* the do loop.				      */	
				diag_free_list(cudv, &c_info);
				continue; /* loop back to get ptype & psclass */
			} else
				not_done=0; /* Exit */

		else {

		/* If control gets here, we have obtained the pdiagatt. Now  */
		/* use this information to obtain information about the alt  */
		/* device via the CuAt.					     */	

			not_done=0;
			sprintf(crit, "name=%s AND attribute=%s", cudv->name,
				pdiagatt->value);
			/* Now free cudv, since the name is already obtained. */

			diag_free_list(cudv, &c_info);
			cuat = (struct CuAt *)diag_get_list(CuAt_CLASS, crit,
				&cuat_info, 1, 1);
			if( (cuat != (struct CuAt *)-1) &&
	    		    (cuat != (struct CuAt *)NULL) ) {
				for(i=0; i < num_All; i++)
             				if( (!strcmp(cuat->value,
						All[i]->T_CuDv->name))
						&& (All[i]->T_CuDv->chgstatus != 3) ){
					    *alt_device_index=i;
       	                		    break;
					}
				diag_free_list(cuat, &cuat_info);
			}					
			diag_free_list(pdiagatt, &p_info);
		}
	} while (not_done);

}
