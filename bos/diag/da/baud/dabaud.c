static char sccsid[] = "@(#)70  1.1  src/bos/diag/da/baud/dabaud.c, foxclub, bos411, 9435B411a 8/26/94 13:30:42";
/*
 *   COMPONENT_NAME: dabaud
 *
 *   FUNCTIONS: all_init
 *              check_asl_stat
 *              clean_up
 *              disp_scrn
 *              ela
 *              int_handler
 *              main
 *              report_frub
 *              set_up_sig
 *              test_tu
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*************************************************************************
 *     File: dabaud.c                                                    *
 *************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/errids.h>
#include <sys/types.h>
#include <locale.h>
#include <fcntl.h>

#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/diag_exit.h"
#include "diag/diagodm.h"
#include <sys/cfgodm.h>
#include <string.h>

#include "baud_exectu.h"
#include "dabaud.h"

struct          tm_input tm_input;    /* info. from dc to this program     */
int             rc=0;                 /* to hold functions return codes    */
static  nl_catd         catd;         /* file descriptor for catalog file  */
static short    odm_flag=0;
static int asl_flag = ASL_NOT_INITIALIZED;
static int initialized = NO;
static char * name = NULL;            /* device desc. to use for screen display */

extern          nl_catd diag_catopen(char *, int);
extern          unsigned long exectu(char *device_name, TUTYPE *tucb_ptr );
extern          getdainput();
extern          addfrub();
extern          insert_frub();

void main()
{
  setlocale(LC_ALL,"");
  all_init();

  /* initialize interrupt handler structure       */
  set_up_sig();

  /* get the description of the card */
  if(name == NULL)
      name = get_dev_desc(tm_input.dname);

  /* display initial screen depending on loopmode */
  if (NOTLM || ENTERLM)
      disp_scrn(TITLE);
  else
      disp_scrn(LOOPMODE_MSG_ID);

  /* if running in LOOPMODE_EXITLM then           */
  /* clean up and exit                                            */
  if( EXITLM )
  {
      clean_up();
  }

  if( PD_MODE || REPAIR_MODE )
  {
      check_asl_stat((int)READ_KBD);

      rc = load_diag_dd(tm_input.dname);
      sleep(4);
      if(rc == 0)
        {
        rc = test_tu(TU_OPEN_BAUD);    /* Diagex & hardware initialization */
        sleep(1);
        if(rc == 0)
              {
              initialized = YES;
              if(IPL)
                  {
                  test_tu(TU_ORD_IPL);
                  }
              else
                  {
                  test_tu(TU_ORD_ADV);
                  };
              initialized = NO;
              rc = test_tu(TU_CLOSE_BAUD);   /* hardware cleanup   */
              sleep(2);
              if(rc != 0)
                  clean_up();
              }
        else
              {
              clean_up();
              };
        unload_diag_dd(tm_input.dname);
        sleep(4);
        }
      else
        {

        rc = 0x55;
        };
   }

  /* if running problem determination, or error log analyis       */
  /* running error log analysis                                   */
  if( (DA_CHECKRC_STATUS() == DA_STATUS_GOOD) &&
          (PD_MODE || ELA_MODE) &&
          (NOTLM || ENTERLM) )
{
     /* For analyze the device apps's error log; */
     /* currently, device drv. for baud is not ready --> not to run ela */
     /*     ela (); */
  }

  /* cleaning up and exit DA      */
  clean_up();

} /* End of Main */

/*
 * NAME: clean_up
 *
 * FUNCTION: close file descripters and clean up ASL, ODM, etc. and then exit
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static void clean_up()
{

        if(initialized)
        {
                initialized = NO;
                test_tu(TU_CLOSE_BAUD);
                sleep(2);
        }

        /* terminate odm */
        if(odm_flag != ERROR_FOUND)
                term_dgodm();

        if(catd != CATD_ERR)
                catclose (catd);

        if ( asl_flag == ASL_INITIALIZED)
          {
                diag_asl_clear_screen();
                diag_asl_quit(NULL);
        }
        DA_EXIT ();
} /* clean_up */

/*
 * NAME: disp_scrn
 *
 * FUNCTION: displays menus
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  the return code is the choice which was made, either 1 or 0.
 *                       This is only valid for the screens that ask a question but the
 *                       DIAG_ITEM_SELECTED(menutypes) is returned regardless and it should
 *                       be ignored by the calling function if it is not needed.
 *
 */

#define SUBS 4

static int disp_scrn(int menu_type)
{
        ASL_SCR_TYPE menutypes= DM_TYPE_DEFAULTS; /* might use this later */
        int rc,i;
        char mode_str[64];
        int base_num;
        int menu_num;
        char  * subs[SUBS];

        if( asl_flag == ASL_NOT_INITIALIZED)
        {
                return(0);
        }

        for(i=0;i<SUBS;i++)
        {
                subs[i] = (char *) malloc(128);
        }

        strcpy(subs[0],name);
        strcpy(subs[1],tm_input.dname);
        strcpy(subs[2],tm_input.dnameloc);

        base_num = FOXCLUB * 0x1000;

        if(ADVANCED)
        {
                strcpy(mode_str,diag_cat_gets(catd,MSG_SET,ADVANCED_MSG_ID));
                menu_num = base_num + 1;
        }
        else
        {
                strcpy(mode_str,diag_cat_gets(catd,MSG_SET,CUSTOMER_MSG_ID));
                menu_num = base_num + 2;
        }

        switch(menu_type)
        {
                case TITLE:

                        /* display title/standby menu */
                        if(ADVANCED)
                                rc = diag_display_menu(ADVANCED_TESTING_MENU,menu_num,
                                                                subs,tm_input.lcount,tm_input.lerrors);
                        else
                                rc = diag_display_menu(CUSTOMER_TESTING_MENU,menu_num,
                                                                subs,tm_input.lcount,tm_input.lerrors);

                        check_asl_stat(rc);
                        break;

                case LOOPMODE_MSG_ID:
                        menu_num = base_num + 3;


                        rc = diag_display_menu(LOOPMODE_TESTING_MENU,menu_num,
                                                                subs,tm_input.lcount,tm_input.lerrors);
                        check_asl_stat(rc);
                        break;

                default:
                                break;
        }
        check_asl_stat((int)READ_KBD);

        /* free memory */
        for(i=0;i<SUBS;i++)
                free(subs[i]);
        /*
         * if item chosen, return which item was selected.
         */
        return(DIAG_ITEM_SELECTED(menutypes));

} /* disp_scrn */

/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void
int_handler(int sig)
{
        if ( tm_input.console == CONSOLE_TRUE )
                diag_asl_clear_screen();
        clean_up();
}

/*
 * NAME: set_up_sig
 *
 * FUNCTION: Set up signal handler calls
 *
 * RETURNS: NONE
 */
void set_up_sig()
{
        struct  sigaction   invec;  /* interrupt handler structure     */

        /* initializing interrupt handler */
        invec.sa_handler =  int_handler;
        sigaction( SIGINT, &invec, (struct sigaction *) NULL );
        sigaction( SIGTERM, &invec, (struct sigaction *) NULL );
        sigaction( SIGQUIT, &invec, (struct sigaction *) NULL );
        sigaction( SIGRETRACT, &invec, (struct sigaction *) NULL );

        return;
}   /* end of set_up_sig() */

/*
 * NAME: all_init
 *
 * FUNCTION: Initialize things like ODM, ASL, and sets default exit values.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static void all_init()
{
        int rc;

        /*..............................*/
        /* General Initialization       */
        /*..............................*/
        /* building defined data structure for foxglove */
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        DA_SETRC_ERROR(DA_ERROR_NONE);
        DA_SETRC_USER(DA_USER_NOKEY);
        DA_SETRC_TESTS(DA_TEST_FULL);
        DA_SETRC_MORE(DA_MORE_NOCONT);

        /* initialize odm and get dainput       */
        odm_flag = init_dgodm();
        if (odm_flag == ERROR_FOUND || getdainput (&tm_input) != 0)
        {
                DA_SETRC_STATUS(DA_ERROR_OTHER);
                clean_up ();
        }

        /* running under console */
        if ( CONSOLE_INSTALLED )
        {
                /* initialize asl               */
                if(INLM)
                        rc = diag_asl_init ( "DEFAULT" );
                else
                        rc = diag_asl_init("NO_TYPE_AHEAD");

                if(rc == ERROR_FOUND)
                {
                        DA_SETRC_STATUS(DA_ERROR_OTHER);
                        clean_up ();
                }
                else
                        asl_flag = ASL_INITIALIZED;

                catd = diag_catopen (CATALOG,0);
        }
} /* all_init */

/*
 * NAME: check_asl_stat
 *
 * FUNCTION: checks ASL status and/or return code
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static void check_asl_stat(int rc)
{
        char buf[64];

        if( (! CONSOLE_INSTALLED ) )
        {
                return;
        }

        if(rc == READ_KBD)
                rc = diag_asl_read ( ASL_DIAG_LIST_CANCEL_EXIT_SC, 0,buf) ;

        switch ( rc)
        {
                case ASL_CANCEL:
                        DA_SETRC_USER (DA_USER_QUIT);
                        clean_up();
                        break;

                case ASL_EXIT:
                        DA_SETRC_USER (DA_USER_EXIT);
                        clean_up();
                        break;
                default:
                        break;

        }
        return;
}  /* check_asl_stat    */

/*
 * NAME: report_frub
 *
 * FUNCTION: Adds FRU bucket to database
 *
 * EXECUTION ENVIRONMENT:
 *
 * INPUTS:  The address of the FRU structure to add to the FRU DB.
 *
 * RETURNS: NONE
 */

static void report_frub(struct fru_bucket *frub_addr)
{
        int     rc;

        /* copy device name into the field device name of fru bucket */
        strcpy ( frub_addr->dname, tm_input.dname);

        /* update FRU Bucket with parent device name            */
        rc = insert_frub(&tm_input, frub_addr);
        if (rc != NO_ERROR)
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                return ;
        }

        /* add a FRU bucket;                                    */
        rc = addfrub(frub_addr);
        if (rc != NO_ERROR)
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                return;
        }

        DA_SETRC_STATUS (DA_STATUS_BAD);
        return;
} /* report_frub */

/*
 * NAME: test_tu
 *
 * FUNCTION: Run the specified test unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static int test_tu(int * numb)
{
        int rc;
        TUTYPE tucb;
        int index;
        char device_name[40];
        ERROR_DETAILS data;

        /* initialize rc and index to 0 */
        rc = 0;
        index = 0;

        while(numb[index] != (int) NULL && rc == NO_ERROR)
        {
                /* initialize tucb structure */
                tucb.header.loop = 1;
                tucb.error_log_count = 1;
                tucb.error_log = &data;
                tucb.header.tu = numb[index];
         /*     tucb.device_name = tm_input.dname;    */

                /* get name into correct format;  '/dev/baud0' example */
                sprintf(device_name, "/dev/%s", tm_input.dname);
                tucb.device_name = device_name;

                rc = exectu (tucb.device_name, &tucb);
                rc = rc & 0x0fff;
                if(rc != NO_ERROR)
                {
                   /* Report a specific fru depending on which test failed */
                   /* The index must match the fru bucket index */
                   switch (tucb.header.tu)
                   {
                      case TU_OPEN:
                             report_frub (&frus[0]);
                             break;
                      case TU_VPD_CHECK:
                             report_frub (&frus[1]);
                             break;
                      case TU_MCI_CHIP:
                             report_frub (&frus[2]);
                             break;
                      case TU_CODEC_TEST:
                             report_frub (&frus[3]);
                             break;
                      case TU_HTX_REC_PLAY:
                             report_frub (&frus[4]);
                             break;
                      case TU_CLOSE:
                             report_frub (&frus[5]);
                             break;
                      default:
                             break;
                   } /* end switch */
                }

                check_asl_stat ((int)READ_KBD);
                index++;
        }
    return(rc);
} /* test_tu */

/*
 * NAME: ela
 *
 * FUNCTION: perform error log analysis and take action if needed
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static void ela()
{
        int ela_rc , ela_get_rc;
        int error;
        char crit[128];
        struct errdata errdata_str;
        struct error_log * err_info;
        int index;

        /* initialize index */
        index = 0;

        while(errids[index] !=  (int) NULL)
        {
        /*..........................................................*/
            /* Look for an occurrence of any of the following errors in */
                /* the error log.  If one exists, exit_with_fru for ELA.    */
                /*..........................................................*/
        sprintf(crit,"%s -N %s -j %X", tm_input.date, tm_input.dname,
                                errids[index]);

        ela_get_rc = error_log_get(INIT,crit,&errdata_str);
        ela_rc = error_log_get(TERMI,crit,&errdata_str);

                if (ela_get_rc >0)
                {
                        /* if no FRU with that criteria get generic HW FRU callout */
                report_frub(&frus[13]);
                clean_up();

                }

                index++;
        }

        return;
} /* ela */

/*
 * NAME: load_diag_dd
 *
 * FUNCTION: Unloads normal device driver and loads diagnostics driver.
 *
 * EXECUTION ENVIRONMENT: Process
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: 0 if no error
 *
 * INPUT:  logical_device_name - global variable
 *
 * OUTPUT: None.
 */

 int  load_diag_dd(char *devname)
  {
  char name[20], *outbuf, *errbuf;
  int  retcode1, retcode2;

  outbuf = NULL;
  errbuf = NULL;

  /* get name into correct format;  '-l baud0' example */
  sprintf(name, "-l %s", devname);

  retcode1 =
    odm_run_method("/etc/methods/ucfgdevice", name, &outbuf, &errbuf);

  retcode2 =
    odm_run_method("/etc/methods/cfgdiagbaud", name, &outbuf, &errbuf);

  if(retcode2 && (retcode1 == 0))
    odm_run_method("/etc/methods/cfgbaud", name, &outbuf, &errbuf);

  return(retcode2);
 }
/*
 * NAME: unload_diag_dd
 *
 * FUNCTION: Unloads diagnostics device driver and loads normal driver.
 *
 * EXECUTION ENVIRONMENT: Process
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: 0 if no error
 *
 * INPUT:  logical_device_name - global variable
 *
 * OUTPUT: None.
 */

 int  unload_diag_dd(char *devname)
  {
  char name[20], *outbuf, *errbuf;
  int  retcode1, retcode2;

  outbuf = NULL;
  errbuf = NULL;

  /* get name into correct format;  '-l baud0' example */
  sprintf(name, "-l %s", devname);

  retcode1 =
    odm_run_method("/etc/methods/ucfgdiagbaud", name, &outbuf, &errbuf);

  retcode2 =
    odm_run_method("/etc/methods/cfgbaud", name, &outbuf, &errbuf);

  return(retcode2);
 }
