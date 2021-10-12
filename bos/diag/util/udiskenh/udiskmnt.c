static char sccsid[] = "@(#)02	1.1  src/bos/diag/util/udiskenh/udiskmnt.c, dsaudiskenh, bos411, 9435A411a 8/18/94 13:52:45";
/*
 *   COMPONENT_NAME: DSAUDISKMNT
 *
 *   FUNCTIONS: main
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
#pragma options nosource
#include "udmutil.h"   /* Utility functions and types. */
#pragma options source

nl_catd cat_fd;

extern int diskcopy ();
extern int disp_alt ();

/*
 * NAME: main()
 *
 * FUNCTION: This procedure is the entry point for the Disk Maintenance 
 *      Service Aid.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is fork()ed and exec()ed either by the diagnostic
 *      controller or by the shell.
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * (RECOVERY OPERATION:) In the event of a software error that prevents
 *      this routine from functioning, control will be returned to the
 *      diagnostic controller after presenting screens to the user
 *      informing them of the error.
 *
 * RETURNS: NONE
 */

main () 
{
 /* handle Control-C like F10 (Exit) */
  struct sigaction act;

  /* entry panel */
  struct msglist mainlist[] = { 
                           {TITLES, MAIN_TITLE},
                           {SCROLLABLES, DISK_TO_DISK_COPY},
			   {SCROLLABLES, DISP_ALTER},
			   {INSTRUCTIONS, SELECT_INSTRUCTION},
                           NULL
  };
  ASL_SCR_INFO maininfo[DIAG_NUM_ENTRIES(mainlist)];
  ASL_SCR_TYPE maintype = DM_TYPE_DEFAULTS;
  int main_rc, rc;

  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  act.sa_handler = genexit;
  sigaction(SIGINT,&act,NULL);

  /* initialize locale environment and ODM and ASL */
  setlocale(LC_ALL, "");
  diag_asl_init("DEFAULT");
  init_dgodm();

  /* open the message catalog file */
  /* cat_fd=diag_catopen("./udiskmnt.cat", 0); */
  cat_fd=diag_catopen(MF_UDISKMNT, 0);

  /* Put up the main menu */
  main_rc = diag_display(MAIN_MEN, cat_fd, mainlist, DIAG_IO, 
                    ASL_DIAG_LIST_CANCEL_EXIT_SC, &maintype, maininfo);

  while (main_rc == ASL_COMMIT) {
      switch (maintype.cur_index) {
	  case 1: /* Disk to Disk Copy */
	      rc = diskcopy();
              #ifdef DBG
              fprintf (stderr,"RC from diskcopy = %d.\n",rc);
              #endif
	      break;
          case 2: /* Display/Alter Sector */
              rc = disp_alt();
              #ifdef DBG
              fprintf (stderr,"RC from disp_alt = %d.\n",rc);
              #endif
              break;
      } /* end switch */ 
      if (rc != ASL_EXIT) {
        /* Put up the main menu */
        main_rc = diag_display(MAIN_MEN, cat_fd, NULL, DIAG_IO, 
                    ASL_DIAG_LIST_CANCEL_EXIT_SC, &maintype, maininfo);
      }
      else main_rc = rc;  /* exit all the way out. */
  } /* end while */

  if (main_rc != DIAG_ASL_COMMIT &&
      main_rc != DIAG_ASL_EXIT   &&
      main_rc != DIAG_ASL_CANCEL) {
          #ifdef DBG
          fprintf(
          stderr, "Unexpected return code from diag_display %d.\n", 
          main_rc);
          #endif
          genexit(-1);
  }
  
  /* return to SERVICE AIDS SELECTION MENU when done */
  genexit(0);
} /* end main */
