static char sccsid[] = "@(#)60  1.6  src/bos/diag/util/udsense/udsense.c, dsaudsense, bos41J, 9512A_all 3/9/95 21:10:23";
/*
 * COMPONENT_NAME: (DSAUDSENSE) DIAGNOSTIC UTILITY
 *
 * FUNCTIONS:
 *              main
 *              int_handler
 *              genexit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <signal.h>
#include <nl_types.h>
#include <sys/stat.h>
#include <locale.h>
#include <string.h>
#include "diag/diag.h"
#include "diag/diago.h"
#include "udsense_msg.h"
#include "diag/diag_define.h"

#define UDSENSE_MENU    0x802900
#define UDERRPT1_MENU   0x802901
#define UDERRPT2_MENU   0x802902
#define unused3         0x802903
#define unused4         0x802904
#define STANDBY_MENU    0x802905
#define OUT_OF_MEM_ERR  0x802906
#define CMDMISSING_MENU 0x802907
#define ODM_RUN_ERR     0x802908
#define NO_DATA_MENU    0x802909

/* DIAG_BIN_VAR is the environment variable set by
 * Diagnostics.
 */

#define DIAG_BIN_VAR "DIAGNOSTICS"


#define OPT1 "-d H"        /* Error report - summary */
#define OPT2 "-a -d H"     /* Error report - detail */

/* Global variables */
nl_catd         fdes;           /* catalog file descriptor      */
char            *outbuf;
char            *errbuf;

/* PATH_MAX is defined in sys/limits.h */
char            diag_path[PATH_MAX];   /* diagnostics tools bin directory path */
char            run_cmd[PATH_MAX];     /* The command to be executed */

/* Extern variables */
extern ASL_SCR_TYPE dm_menutype;
extern nl_catd diag_catopen(char *, int);

/* EXTERNAL FUNCTIONS */
extern char     *diag_cat_gets();

/* FUNCTION PROTOTYPES */
void int_handler(int);
void genexit(int);

/*  */
/* NAME: main
 *
 * FUNCTION: Display hardware errpt output
 *
 * NOTES: This unit displays the 'Hardware Error Report' menu and
 *        controls execution of the response.  It performs the following
 *        functions:
 *      1.  Initialize the ODM.
 *      2.  Display menu and wait for response.
 *      3.  Call appropriate function according to response.
 *
 * RETURNS: 0
 *
 */

main(int argc, char *argv[])
{
   int     status = -1;
   int     rc;
   int     selection;
   struct  stat buf;
   struct  sigaction act;
   struct  msglist menulist[] = {
      { MSET, MTITLE },
      { MSET, MOPT1 },
      { MSET, MOPT2 },
      { MSET, MLASTLINE },
      { (int )NULL, (int )NULL}
   };
   struct  msglist cmd_missing[] = {
      { MSET, MTITLE },
      { MSET, CMD_MISSING },
      { MSET, UDRET },
      { (int )NULL, (int )NULL}
   };
   struct  msglist no_data[] = {
      { MSET, MTITLE },
      { MERROR, NO_DATA },
      { MSET, UDRET },
      { (int )NULL, (int )NULL}
   };
   static ASL_SCR_INFO menu_cmd_missing[DIAG_NUM_ENTRIES(cmd_missing)];
   static ASL_SCR_INFO menu_no_data[DIAG_NUM_ENTRIES(no_data)];
   static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
   char  *path_ptr;  /* used to hold diagnostics environment variable */
   char  *lang_var;  /* used to hold language environment variable */
   char  outstr[15]; /* used for putenv call */
   char  *getenv();
   char  msgstr[512];


   /* Get environment variable DIAGNOSTICS or use default value */
   diag_path[0] = '\0';    /* init diagnostics buffer */
   /* Look for the sense path in the environment */
   path_ptr = getenv( DIAG_BIN_VAR );
   if ( path_ptr != NULL ) {
      /* save environment variable in our string variable */
      strcpy( diag_path, path_ptr );
   } else {
      /* environment variable path to user specified data directory
       * wasn't found. So use the default path defined in this program.
       */
      strcpy( diag_path, DIAGNOSTICS );
   }
   /* add bin directory to DIAGNOSTICS directory, or to default directory */
   /* it is important that NO whitespace is added after the "/bin/"
    * as the dsense command is appended to this path.
    */
   strcat(diag_path,"/bin/");

   setlocale(LC_ALL, "");

   /* set up interrupt handler     */
   act.sa_handler = int_handler;
   sigaction(SIGINT, &act, (struct sigaction *)NULL);

   /* catalog name is the program name with .cat extension */
   diag_asl_init("default");
   fdes = diag_catopen(MF_UDSENSE, 0);
   init_dgodm();

   /* Check to see if errpt command is available on the system */
   /* If not, then the SMIT package is missing.                */
   if (( rc = stat(ERRPT, &buf)) != 0 ){
      status = diag_display(CMDMISSING_MENU, fdes, cmd_missing,
         DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, &menutype,
         menu_cmd_missing);
      genexit(0);
   }
   status = DIAG_ASL_OK;
   while (status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT) {
      /* display top menu */
      status = diag_display(UDSENSE_MENU, fdes, menulist, DIAG_IO,
              ASL_DIAG_LIST_CANCEL_EXIT_SC, NULL, NULL);
      if (status == DIAG_ASL_COMMIT) {
         int     rc;
         int             menu;
         int             title;
         ASL_SCR_INFO    *menuinfo;
         ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;


         /* display warning popup message and process its result */
         rc = diag_asl_msg(diag_cat_gets( fdes, MWAITING, WAITING ),
                         diag_cat_gets(fdes, MSET, MTITLE) );
         switch (rc) {
            case DIAG_ASL_EXIT:
               /* exit means to exit back to AIX */
               status = DIAG_ASL_EXIT;
               break;
            case DIAG_ASL_CANCEL:
               /* cancel means to return to main menu */
               status = DIAG_ASL_OK;
               break;
            case DIAG_ASL_ENTER:
               /* enter means to continue with processing
                * so display results of previous command
                * displays the Menu Title,
                * UDCANCEL line, and the command's output stream. This
                * may take some time, during which the screen is blank.
                */

                diag_msg_nw(STANDBY_MENU, fdes, MSET, STANDBY);

                errbuf = NULL;
                switch (selection = DIAG_ITEM_SELECTED(dm_menutype)) {
                   case 1:
                        /* error summary */
                        menu = UDERRPT1_MENU;
                        title = UDERRSUM;
                        sprintf(run_cmd,"%s %s",ERRPT,OPT1);
                        break;
                   case 2:
                        /* error detail */
                        menu = UDERRPT2_MENU;
                        title = UDERRDTL;
                        sprintf(run_cmd, "%s %s", ERRPT, OPT2);
                        break;
                } /* end of switch statement */

                /* Get the error report information */
                rc = odm_run_method(run_cmd, "", &outbuf, &errbuf);

                /* Print out error message if error was encountered */
                if (rc == -1) {

                   ASL_SCR_INFO *menu_da;
                   ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
                   menu_da= (ASL_SCR_INFO *) calloc(1,(4*sizeof(ASL_SCR_INFO)));
                   if(menu_da == (ASL_SCR_INFO *) NULL) {   /* No free memory */
                        diag_asl_msg("Out of memory.  Program aborting.");
                        status = DIAG_ASL_EXIT;   /* Force exit */
                        break;
                   }
                   sprintf(msgstr, "%s", (char *) diag_cat_gets (fdes,
                        MSET, MTITLE));
                   menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
                   strcpy(menu_da[0].text, msgstr);

                   sprintf(msgstr, "%s", (char *) diag_cat_gets (fdes,
                        MERROR, ODM_RUN_ERROR));
                   menu_da[1].text = (char *) malloc (strlen(msgstr)+1);
                   strcpy(menu_da[1].text, msgstr);
                   sprintf(msgstr, menu_da[1].text, run_cmd);
                   free(menu_da[1].text);
                   menu_da[1].text = (char *) malloc (strlen(msgstr)+1);
                   strcpy(menu_da[1].text, msgstr);

                   sprintf(msgstr, "%s", (char *) diag_cat_gets (fdes,
                        MSET, UDRET));
                   menu_da[2].text = (char *) malloc (strlen(msgstr)+1);
                   strcpy(menu_da[2].text, msgstr);
                   menutype.max_index = 2;

                   diag_display(ODM_RUN_ERR, fdes, NULL,
                         DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, &menutype,
                         menu_da);
                   continue;
                }

                /*
                   If outstr is NULL, or if it does not contain any characters
                   besides blanks, tabs and carriage returns, put up a
                   popup saying that no hardware errors were found and
                   return to main menu.
                */
                if ((outbuf == (char *) NULL) ||
                   (strspn(outbuf, " \r\n\t") == strlen(outbuf))) {
                      sleep(2);  /* Make it look like we're busy */
                      diag_display(NO_DATA_MENU, fdes, no_data,
                            DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, &menutype,
                            menu_no_data);
                       continue;
                }

                /* allocate space for 3 entries */
                menuinfo = (ASL_SCR_INFO *)
                        calloc(3, sizeof(ASL_SCR_INFO));

                /* put in the title */
                menuinfo[0].text = diag_cat_gets(fdes, MSET, title);

                /* put in the command output */
                menuinfo[1].text = outbuf;

                /* add in the last line */
                menuinfo[2].text = diag_cat_gets(fdes, MSET, UDCANCEL);
                menutype.max_index = 2;
                rc = diag_display(menu, fdes, NULL, DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC,
                        &menutype, menuinfo);
                free(menuinfo);
                if (rc == DIAG_ASL_EXIT)
                        status = rc;
                break;
            default:
               /* same as DIAG_ASL_CANCEL */
               status = DIAG_ASL_OK;
               break;
         } /* end of switch statement */

         /* release memory buffers before proceeding */
         free(outbuf);
         if (errbuf)
            free(errbuf);
      } /* end of if(status ..)  statement */

   } /* end of while loop */
   genexit(0);

} /* end of main() function */

/*  */
/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up on receipt on an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 *
 */

void int_handler(int sig)
{
        genexit(1);
}


/*
 * NAME: genexit
 *
 * FUNCTION: Perform general clean up, and then exit with the status code.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 *
 */

void genexit(int exitcode)
{
        odm_terminate();
        diag_asl_quit();
        catclose(fdes);
        exit(exitcode);
}
/* end of udsense.c */
