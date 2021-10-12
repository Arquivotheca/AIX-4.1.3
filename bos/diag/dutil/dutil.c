static char sccsid[] = "@(#)96	1.18.2.17  src/bos/diag/dutil/dutil.c, dutil, bos41J, 9520A_all 5/16/95 10:33:58";
/*
 *   COMPONENT_NAME: DUTIL
 *
 *   FUNCTIONS: build_SA_menu
 *              check_diskettes
 *              disp_sup_menu
 *              genexit
 *              get_dskt_list
 *              int_handler
 *              main
 *              p_error
 *              prompt_for_diskette
 *              get_disp_selection_cond
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <signal.h>
#include <nl_types.h>
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/class_def.h"            /* object class data structures */
#include "diag/tmdefs.h"
#include "dutil_msg.h"
#include <locale.h>

/*
 * NAME: main
 *
 * FUNCTION: Display Diagnostic Utility Menu
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: This unit displays the Diagnostic Utility Menu and controls
 *        execution of the utilities. It performs the following functions:
 *      1.  Check that msgkey entry was given as input argument.
 *      2.  Initialize the ODM.
 *      3.  Build menu display list according to data in DSMOptions object class
 *      4.  Display menu and wait for response.
 *      5.  Call appropiate function according to response.
 *      what bits / data structures, etc it manipulates.
 *
 * RETURNS: NONE
 *
 */

/* LOCAL DEFINES */
#define SUP_DSKT        "S"
#define DO_NOT_READ_ALL 0
#define READ_ALL        1
#define COMMIT_KEY      (rc == DIAG_ASL_COMMIT)
#define CANCEL_KEY      (rc == DIAG_ASL_CANCEL)
#define EXIT_KEY        (rc == DIAG_ASL_EXIT)
#define CHK_CALLOC      { if (pmem == NULL) { \
                          diag_asl_msg("Memory allocation failed.\n"); \
                          genexit(1); } \
                        }

/* GLOBAL VARIABLES */
char    *disks_not_present;
char    **tmp_dskt_file;
char    *dskt_list;
char    current_volume[5];              /* Current diskette read        */
char    menu_key[5];                    /* input argument menu key type */
char    diagdir[256];                   /* default diagnostics path     */
char    **action_ptr;                   /* list of actions to perform      */
char    **diskette_ptr;                 /* holds diskette where command is */
int     max_dskt;                       /* Total # of diskettes         */
int     sup_dskt = 0;                   /* # of supplemental dskts      */
int     diskette_based = DIAG_FALSE;    /* running on diskette          */
int	diag_ipl_source=IPLSOURCE_DISK; /* Media used to IPL diags.    */
int	diag_source=IPLSOURCE_DISK; 	/* Media where diags code reside. */
int     memory_size;                    /* Memory size in K from CuAt   */
nl_catd fdes;                           /* catalog file descriptor      */
char	*bootdev;			/* name of boot device		*/
int     exenvflg = EXENV_IPL;   	/* execution mode               */
char	*altdir;

/* EXTERNAL VARIABLES */
extern char *optarg;
extern ASL_SCR_TYPE dm_menutype;

/* EXTERNAL FUNCTIONS */
extern char *getenv(char *);
extern char *diag_cat_gets();
extern nl_catd diag_catopen(char *, int);
extern void *malloc(int);
extern void *calloc(unsigned, unsigned);
extern char *chkdskt(char *);
extern char *strcpy(char *, char *);
extern char *strncpy(char *, char *, int);
extern char *strstr(char *, char *);
extern int strlen(char *);
extern int restore_files(char *);

/* FUNCTION PROTOTYPES */
int build_SA_menu(int, int *, ASL_SCR_INFO *);
void p_error(int, char *);
void int_handler(int);
void genexit(int exitcode);
void get_dskt_list(void);
void check_diskettes(void);
int disp_sup_menu(void);
int prompt_for_diskette(char *, int);

/*  */
main(int argc, char *argv[])
{
        int     c;                      /* holds char returned from getopt */
        int     index;                  /* index into action_ptr array     */
        int     order;                  /* order of DSMOption retrieval    */
        int     rc;                     /* holds return status             */
        int     status;                 /* holds return status             */
        int     child_status;           /* holds return status from child  */
        char    *flag_ptr[3];           /* command list passed to execvp   */
        char    crit[80];               /* object class search string crit */
        char    *str;                   /* points to environment strings   */
        char    command[256];           /* holds full path of command      */
        char    arguments[128];         /* holds arguments from obj class data */
        char    *curdisk;               /* points to current diskette      */
        char    *dadir;                 /* points to DA directory          */
        short   valid = DIAG_FALSE;     /* flag indicating valid argument  */
        struct sigaction act;           /* interrupt handler structure     */
        struct DSMOptions *T_DSMOptions;/* ptr to DSMOptions data          */
        struct listinfo o_info;
        struct  CuAt    *attobj;
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
        ASL_SCR_INFO *menuinfo;
	char	*volume;
	char	tmp[5];
        void    *pmem;
        char str_tmp[256];      /* temporary variable to hold environment variable */

        setlocale(LC_ALL,"");

        /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* open catalog file */
        fdes = diag_catopen(MF_DUTIL,0);

        /* get execution mode */
        exenvflg = ipl_mode(&diskette_based);

        diag_asl_init("default");

        /* initialize the ODM data base */
        if (init_dgodm() == -1)
                p_error(ERROR1, argv[0]);

	/* determine source of IPL by looking at the environment variable. */
	if((str = getenv("DIAG_IPL_SOURCE")) != (char *)NULL)
		diag_ipl_source=atoi(str);

	if((str = getenv("DIAG_SOURCE")) != (char *)NULL)
		diag_source=atoi(str);

	if(diag_source==IPLSOURCE_CDROM)
		altdir = getenv("ALT_DIAG_DIR");

        /* get directory path to use for command execution of diag's */
        if ((str = getenv("DIAG_UTILDIR")) == (char *)NULL ) {
                if ((str = getenv("DIAGNOSTICS")) == (char *)NULL)
                        sprintf(diagdir, "%s/bin/", DIAGNOSTICS);
                else
                        sprintf(diagdir, "%s/bin/", str);
        }
        else
                sprintf(diagdir, "%s/", str);

        /* get da directory path */
        if( (dadir =(char *)getenv("DADIR")) == NULL )
                dadir = DEFAULT_DADIR;

	/* get name of boot device */

	bootdev = (char *)getenv("BOOTDEV");

        /* Validate the argument before displaying the menu.    */
        while ((c = getopt(argc,argv,"m:")) != EOF) {
                switch (c)  {
                        case 'm' :
                                strncpy(menu_key,optarg,sizeof(menu_key));
                                valid = DIAG_TRUE;
                                break;
                }
        }

        if ( !valid )
                p_error(ERROR5, argv[0] );

        /* Determine the # of Service Aids to be added to menu */
        sprintf(crit,"msgkey = '%s'", menu_key);
        T_DSMOptions = (struct DSMOptions *)diag_get_list(DSMOptions_CLASS,
			crit, &o_info, 20, 1);

        /* Find out memory size in machine from CuAt */
	/* Maybe a libdiag routine must be called to */
	/* obtain actual memory size information.    */

        attobj = (struct CuAt *)getattr("sys0", "realmem", FALSE, &rc);
        if(attobj == (struct CuAt *)NULL)
                memory_size=0;
        else
                memory_size=(int)strtol(attobj->value,(char **)NULL, 0);

        /* Allocate memory for menu */
        pmem = calloc(o_info.num, sizeof(ASL_SCR_INFO));
        CHK_CALLOC
        menuinfo = (ASL_SCR_INFO *)pmem;

        /* Allocate memory for diskette names */
        pmem = calloc(o_info.num, sizeof(char *));
        CHK_CALLOC
        diskette_ptr = (char **)pmem;

        /* Allocate memory for executable names */
        pmem = calloc(o_info.num, sizeof(char *));
        CHK_CALLOC
        action_ptr = (char **)pmem;
        diag_free_list(T_DSMOptions, &o_info);

        /* Add Title to menu (order=-1) */
        index = 0;
        rc = build_SA_menu(-1, &index, menuinfo);
        if (rc != 1) p_error(ERROR4, "DSMOptions");

        /* Add standard SA's to menu (order=1,2,3...) */
        order = 1;
        while ((rc = build_SA_menu(order, &index, menuinfo)) != 0)
          {
          if (rc != 1) p_error(ERROR4, "DSMOptions");
          order++;
          }

        /* Add conditional SA's to menu (order=99) */
        rc = build_SA_menu(99, &index, menuinfo);
        if (index < 2)          /* No SA's found in data base */
          p_error(ERROR4, "DSMOptions");

        /* Add selective fix SA's to menu (order=0) */
        rc = build_SA_menu(0, &index, menuinfo);
        if (index < 2)          /* No SA's found in data base */
          p_error(ERROR4, "DSMOptions");

        /* Add instruction line to menu (order=-2) */
        rc = build_SA_menu(-2, &index, menuinfo);
        if (rc != 1) p_error(ERROR4, "DSMOptions");

        menutype.max_index = --index;  /* index was incremented once too much */
        menutype.cur_index = 1;

        rc = DIAG_ASL_COMMIT;
        while (!CANCEL_KEY && !EXIT_KEY) {
		rc = diag_display(0x802001, fdes, NULL,
                        DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                        &menutype, menuinfo);

                /* if a menu selection was made, extract the command
                 * and arguments from the object list, and execute it
                 */
		if (COMMIT_KEY)
                {
	                  index = DIAG_ITEM_SELECTED(menutype);
       	                  sscanf(action_ptr[index], "%s %s", command, arguments);
                          flag_ptr[0] = command;
			  if ( strlen(arguments) )
                          	flag_ptr[1] = arguments;
			  else
                          	flag_ptr[1] = NULL;
                          flag_ptr[2] = NULL;

			  if (diag_ipl_source != IPLSOURCE_DISK){
                    		rc = process_diskettes(command,
						diskette_ptr[index]);
				if(rc == -1)
					continue;
			  } else
				/* Running from hard file and off the */
				/* CDROM, and the file is not present */
				/* could be a SA from supplemental.   */

				if((diag_source == IPLSOURCE_CDROM) &&
					   !file_present(command)){
					/* Obtain the command, which is the */
					/* token following the last '/'.    */

					str=(char *)strrchr(command, '/');
					sprintf(command, "%s/bin%s", altdir, str);
				}

                  	  /*
                   	   * if this is the pg command, change the
			   * environment variable to
                      	   * NULL so that no input can be used to access
			   * the filesystems.
                   	   */
                  	   if(strstr(command,"/bin/pg") != NULL)
                  	   {
                        	str = getenv("SHELL");
                        	/* save original SHELL definition */
                        	sprintf(str_tmp,"SHELL=%s",str);
                        	/* set SHELL env. variable to NULL */
                        	putenv("SHELL=");
                	  }

                	  if (COMMIT_KEY)
                   		 status = diag_asl_execute(command, flag_ptr, &child_status);
                  	  if (child_status & 0xFF)
                   	 	 p_error(ERROR2, command);
                  }

                  /*
                   * if this is the pg command, change the environment 
		   * variable to what it was before.
                   */

                  if(strstr(command,"/bin/pg") != NULL)
                  {
                        /* reset SHELL env variable */
                        putenv(str_tmp);
                  }

                /*
                 * Make sure that the arguments have been cleared before calling
                 * the next program, which may not be expecting arguments...
                 */
                arguments[0]='\0';

        }
        for (index = 0; action_ptr[index]; index++)
                free (action_ptr[index]);
        genexit(0);
}


/*  */
/*
 * NAME: build_SA_menu
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: # of DSMOption stanzas that meet the specified criteria
 */

int build_SA_menu(
        int order,
        int *index,
        ASL_SCR_INFO *menuinfo)
{
  int i1, count, idx;
  int	condition;
  int	mask;
  int	diskette;
  char *str, *dskt;
  char crit[80];
  char com1[256];
  char com2[128];
  char flag[128];
  nl_catd temp_fdes;
  struct DSMOptions *T_DSMOptions;
  struct listinfo o_info;

        sprintf(crit,"msgkey='%s' AND order=%d", menu_key, order);
        T_DSMOptions = (struct DSMOptions *)diag_get_list(DSMOptions_CLASS,
			crit, &o_info, 1, 1);

        if ((int )T_DSMOptions == -1)
          p_error(ERROR4, "DSMOptions");

	/* build a bit mask consisting of all the conditions required
	   to put a service aid on the menu selection, then use the &
	   operator to see if the given condition for the current
	   entry in PDiagAtt is satisfied. If it is not, do not put the
	   Service Aid on the selection list.
	   - Bit positions  16 8 4 2 1
		             | | | | |
			     | | | | |_______Can only be run in Service Mode
			     | | | |_________Can only be run from hardfile
			     | | |___________Can only be run on MP machine
			     | |_____________Can only be run if there is ISA bus
			     |_______________Can only be run on non-RSPC systems

           If more conditions are needed, then change the line below
	   to have the new bit shifted one more position.
	*/

	mask =  (!is_rspc_model()<<4)                    |
		(has_isa_capability()<<3)                | 
		(is_mp()<<2)                             |
	       	((diag_ipl_source == IPLSOURCE_DISK)<<1) |
	       	(exenvflg == EXENV_STD);

        count = o_info.num;
        idx = *index;
        for (i1=0; i1<count; i1++)
          {
          /*
            Determine the full path of the command to execute
          */

          if (T_DSMOptions[i1].action[0] == '/')
            strcpy(com1, T_DSMOptions[i1].action);
          else
            {
            strcpy(com1, diagdir);
            strcat(com1, T_DSMOptions[i1].action);
            }

          /*
           Strip off any flags to get just the command and use it to
           to determine if the file is present.
	  */

          sscanf(com1, "%s %s", com2, flag);
          dskt = T_DSMOptions[i1].Diskette;

	  /* if diskette field is of the form X00X, then adjust value */
	  if ( (strlen(dskt) == 4) &&
	       ((dskt[0] == '1') || (dskt[0] == '2') || ( dskt[0] == '3')) )
		diskette = atoi(dskt+1);
	  else
		diskette = atoi(dskt);

	  condition = 0;
          /* Do not add to menu if:
               -running from standalone package AND the SA is not
                supported on standalone (dskt field=0.)
                               OR
               -command is not present AND running from hard file
          */

          if ( ((diag_ipl_source != IPLSOURCE_DISK) && (diskette==0)) ||
               (!file_present(com2) && (diag_ipl_source == IPLSOURCE_DISK)) )
                continue;

          /* if this entry has the first character set to 2, the PDiagAtt
	     should be queried to obtain the conditions under which the
	     SA can be put on the selection menu.
	  */
	  if ( (strlen(dskt) == 4) && ((dskt[0] & '2') == '2') )
		condition = get_disp_selection_cond(T_DSMOptions[i1].action);


	  if ((condition & mask) != condition)
		continue;

          diskette_ptr[idx] = (char *)malloc(strlen(dskt) + 1);
          strcpy(diskette_ptr[idx], dskt);
          action_ptr[idx] = (char *)malloc(strlen(com1) + 1);
          strcpy(action_ptr[idx], com1);

          temp_fdes = diag_catopen(T_DSMOptions[i1].catalogue, 0);
          str = diag_cat_gets(temp_fdes, T_DSMOptions[i1].setid,
                                T_DSMOptions[i1].msgid);
          menuinfo[idx].text = malloc(strlen(str) + 1);
          strcpy(menuinfo[idx].text, str);
          catclose(temp_fdes);
          idx++;
          }

        diag_free_list(T_DSMOptions, &o_info);
        *index = idx;

        /*
         Return the # of SA's found in the database
        */

        return(count);
}

/*  */
/*
 * NAME: p_error
 *
 * FUNCTION: Print an error message to the screen and call genexit to clean up.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: NONE
 */

void p_error(
        int err_msg,
        char *str)
{
        Perror(fdes, ESET, err_msg, str);
        genexit(1);
}

/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: NONE
 */

void int_handler(int sig)
{
        diag_asl_clear_screen();
        genexit(1);
}

/*  */
/*
 * NAME: genexit
 *
 * FUNCTION: Perform general clean up, and then exit with the status code.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      0 - no errors
 *      !=0 - error occurred
 */

void genexit(int exitcode)
{
        term_dgodm();
        diag_asl_quit();
        catclose(fdes);
        exit(exitcode);
}

/*  */
/*
 * NAME: get_dskt_list
 *
 * FUNCTION: Retrieves the list of diskettes that are used
 *              with the diagnostic diskette package after IPL.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: None
 */
extern int odmerrno;

void get_dskt_list(void)
{
  char list[256];
  char *comma, *list2;
  int i1, cnt;

        /*******************************************************
           Retrieve the list of diskettes that are part of the
           diagnostic diskette package (after IPL).  Then parse
           into individual diskette names.
        *******************************************************/
        get_diag_att("diag_dskt", "post_ipl_dskts", 's', &cnt, list);
        list2 = list;
        max_dskt = 1;
        while ((comma = strstr(list2, ",")) != (char *)NULL)
          {
          max_dskt++;
          list2 = comma + 1;
          }

        tmp_dskt_file = (char **)malloc(max_dskt*sizeof(char *));
        list2 = list;
        for (i1=0; i1<max_dskt-1; i1++)
          {
          comma = strstr(list2, ",");
          *comma = '\0';
          tmp_dskt_file[i1] = malloc(strlen(list2) + 1);
          strcpy(tmp_dskt_file[i1], list2);
          list2 = comma + 1;
          }
        tmp_dskt_file[i1] = malloc(strlen(list2) + 1);
        strcpy(tmp_dskt_file[i1], list2);

        disks_not_present = (char *)malloc(max_dskt*sizeof(char));

        return;
}

/*  */
/*
 * NAME: process_diskettes
 *
 * FUNCTION: Process supplemental diskettes before running Service Aid
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      None
 */

int process_diskettes(
        char *command,
        char *dskt_ptr)
{
  int   rc;
  int   i1;
  char  tmp[5];
  char  *vol_prompted;
  char  *volume;

	if ( (strlen(dskt_ptr) == 4) && ((dskt_ptr[0] & '1') == '1') ) {
	/* prompt for more supplemental diskette */
		   i1=1;
		   while( (dskt_ptr[i1] == '0') && (i1<strlen(dskt_ptr)) )
			i1++;
		   strcpy(tmp, dskt_ptr+i1);
		   volume=tmp;
                   rc = disp_sup_menu();
                   if (EXIT_KEY) return(rc);
	} else
		if( (strlen(dskt_ptr) == 4) &&
		       ((dskt_ptr[0] == '2') || ( dskt_ptr[0] == '3')) )
		{
		/* Adjust value of diskette */
		   i1=1;
		   while( (dskt_ptr[i1] == '0') && (i1<strlen(dskt_ptr)) )
			i1++;
	       	   strcpy(tmp, dskt_ptr+i1);
		   volume=tmp;
		} else
			volume=dskt_ptr;

        if (file_present(command))
          rc = DIAG_ASL_COMMIT;
        else
                if(diag_ipl_source==IPLSOURCE_TAPE){
            		diag_msg_nw(READING_DSKT, fdes, DSKSET,
				MSG_RESTORE_FILES, bootdev);
                        rc=restore_files(volume);
			if(rc != 0){
				(void)diag_msg(READING_DSKT, fdes, DSKSET,
					MSG_RESTORE_ERROR);
				return(-1);
			}
                        rc=DIAG_ASL_COMMIT;
                }
        return(rc);
}

/*  */
/*
 * NAME: disp_sup_menu
 *
 * FUNCTION: Display supplemental menu requesting diskette
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      0 - no errors
 *      DIAG_ASL_COMMIT -
 *      DIAG_ASL_EXIT -
 *      DIAG_ASL_CANCEL -
 */
int disp_sup_menu(void)
{
  int rc;
  int more;
  static struct  msglist sup[] = {
        { MSET1, MTITLE },
        { DSKSET, DIAGSA_NO },
        { DSKSET, DIAGSA_YES },
        { DSKSET, DIAGSA_SUPP },
        {(int )NULL, (int )NULL}
        };

        while (TRUE)
          {
          if (sup_dskt == 0)
            sup[3].msgid = DIAGSA_SUPP;
          else
            sup[3].msgid = DIAGSA_MORE_DSKT;

          rc = diag_display(0x802003, fdes, sup, DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC,
                        NULL, NULL);

          if ((COMMIT_KEY) && (DIAG_ITEM_SELECTED(dm_menutype) == 2))
            {
            rc = prompt_for_diskette(SUP_DSKT, DO_NOT_READ_ALL);
            if (COMMIT_KEY)
              sup_dskt++;
            else
              break;
            }
          else
            break;
          }
        return (rc);
}

/*  */
/*
 * NAME: prompt_for_diskette
 *
 * FUNCTION: Prompt user to insert diskette
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      0 - no errors
 *      DIAG_ASL_COMMIT -
 *      DIAG_ASL_EXIT -
 *      DIAG_ASL_CANCEL -
 */

int prompt_for_diskette(char *requested_volume, int all_dskt)
{
  int   rc;
  char  str1[256];
  char  str2[64];
  char  str3[256];
  char  *tmp;
  char  *vol;
  FILE  *fd;
  char  script[64];
  char  path[64];
  char  *outb=NULL;
  short create_file=0;
  char  *ebuf=NULL;

  static struct  msglist ntf[] = {
        { MSET1, MTITLE },
        { DSKSET, DIAGSA_REMOVE },
        { DSKSET, DIAGSA_INSERT },
        { DSKSET, DIAGSA_ENTER },
        { DSKSET, DIAGSA_CANCEL },
        {(int )NULL, (int )NULL}
        };
  static ASL_SCR_INFO ntfinfo[DIAG_NUM_ENTRIES(ntf)];
  static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        if (!strcmp(requested_volume,SUP_DSKT))
          {
          tmp = diag_cat_gets(fdes, DSKSET, MSG_DSKT_35);
          strcpy(str2, tmp);
          sprintf(script, "/etc/diagstartS >>$F1 2>&1");
          }

        rc = diag_display(NULL, fdes, ntf, DIAG_MSGONLY, NULL,
                        &menutype, ntfinfo);

        if ( !strcmp(requested_volume,SUP_DSKT) || all_dskt )
          sprintf(str1, ntfinfo[1].text, dskt_list);
        else
          sprintf(str1, ntfinfo[1].text, requested_volume);
        free(ntfinfo[1].text);
        ntfinfo[1].text = str1;

        sprintf(str3, ntfinfo[2].text, str2);
        free(ntfinfo[2].text);
        ntfinfo[2].text = str3;

        rc = diag_display(0x802002, fdes, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutype, ntfinfo);
        if (CANCEL_KEY || EXIT_KEY) return(rc);

        while (TRUE)
          {
          /* identify the current diskette volume */
          vol = chkdskt("DIAG");
          if (!strcmp(vol,requested_volume))
            {
            /* reading message */
            diag_msg_nw(READING_DSKT, fdes, DSKSET, MSG_DSKT_31, str2);
            if (!read_diskette(current_volume))
              {
              strcpy(current_volume,requested_volume);
              /* invoke the script to unpack and configure */
              rc = system(script);
              if (rc != 0)
                return(-1);
              /* write a file into /tmp with the current disk number.
                 This file should be 1-9 with a 0 length. */
              if (create_file) {
                   fd = fopen(path, "w");
                   fclose(fd);
              }
              break;
              }
            else
              vol = "B";
            }

          if (!strcmp(vol,"B"))
            {
            /* bad_diskette */
            rc = diag_hmsg(fdes, DSKSET, MSG_DSKT_32, str2);
            if (CANCEL_KEY || EXIT_KEY) return(rc);
            }
          else
            {
            /* wrong diskette */
            rc = diag_hmsg(fdes, DSKSET, MSG_DSKT_33, str2);
            if (CANCEL_KEY || EXIT_KEY) return(rc);
            }
          }

        return(DIAG_ASL_COMMIT);
}

/*  */
/*
 * NAME: get_disp_selection_cond
 *
 * FUNCTION: Obtain condition bits for putting a Service Aid
 *           on the selection menu.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      0 - no condition, always put the SA up.
 *      the value in the PDiagAtt stanza for the given sa.
 */

int get_disp_selection_cond(char *command)
{

	char criteria[512];
  	struct PDiagAtt *pdiag_att;
	struct listinfo o_info;
	int	cond;

	sprintf(criteria, 
	  "DType = USM AND attribute = sa_selection AND DApp = %s", command);
        pdiag_att = (struct PDiagAtt *)diag_get_list(PDiagAtt_CLASS, criteria,
				&o_info, 1, 1);
	if(pdiag_att == (struct PDiagAtt *)-1)
		return 0; /* No object found, assume no special conditions */

	cond = atoi(pdiag_att->value);
	diag_free_list(pdiag_att, &o_info);
	return(cond);
				
}
