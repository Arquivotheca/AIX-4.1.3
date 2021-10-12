static char sccsid[] = "@(#)18	1.13.3.6  src/bos/diag/util/umkdskt/umkdskt.c, dsaumkdskt, bos411, 9428A410j 3/3/94 17:54:39";
/*
 *   COMPONENT_NAME: dsaumkdskt 
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		Usystem
 *		check_drive
 *		cons_current
 *		create_config_diskette
 *		create_sig
 *		disp_consdef_menu
 *		disp_menu
 *		disp_refresh_menu
 *		disp_sel_menu
 *		drive_type
 *		edit_consdef
 *		format_diskette
 *		genexit
 *		int_handler
 *		main
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <odmi.h> 
#include <diag/diago.h>
#include <diag/diag.h>
#include <diag/diag_define.h>
#include <locale.h>
#include <sys/access.h>
#include <sys/param.h> 
#include <sys/mode.h>
#include "umkdskt_msg.h"


#define MENU_TOP	0x802020	/* Top menu */
#define MENU_INSERT	0x802021	/* Insert Diskette menu */
#define MENU_COMPLETE	0x802022	/* Format complete */
#define MENU_ERROR	0x802023	/* Error menu for format */
#define MENU_NO_DRIVE	0x802024	/* Can't format this dskt on this drive*/
#define MENU_CFG_OK	0x802025	/* Config diskette created */
#define MENU_MODCONS	0x802026	/* Edit console config file */
#define MENU_MREFRESH	0x802027	/* Edit refresh rate config file */
#define MENU_FORMATTING	0x802028	/* Formatting menu */
#define MENU_CFG_NO_CHG	0x802029	/* Config diskette not created */
#define MENU_INSERT_CONS	0x802030	/* Insert a formatted diskette menu */
#define MENU_CFG_ERROR	0x802031	/* Error menu for config*/

/* this dir is used if the DIAGDATA env. var. does not exist. */
#define UMKDSKT_DIR    "/etc/lpp/diagnostics/data"  
#define SYSTEM_CONSDEF_DIR "/etc"
#define       BUFLEN          32      /* temp buffer length */
#define CHANGED -100
#define NOT_CHANGED -99
char current[BUFLEN]; /* cons_current() temp buffer */

char * data_dir;      /* directory where diagnostics data is */

/* EXTERNAL VARIABLES	*/

nl_catd		fdes;				/* catalog file descriptor  */
extern ASL_SCR_TYPE dm_menutype;

/* menu 802020 */
struct msglist msg_top[] =
    {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_FORMAT_1MB},
	{MKDSKT_SET, MKDSKT_FORMAT_2MB},
	{MKDSKT_SET, MKDSKT_FORMAT_4MB},
	{MKDSKT_SET, MKDSKT_CONFIG},
	{MKDSKT_SET, MKDSKT_SELECT_DISKETTE},
	{(int )NULL, (int )NULL}
    };
ASL_SCR_INFO menu_top[DIAG_NUM_ENTRIES(msg_top)];

/* menu 0x802021 */
struct msglist msg_insert[] =
    {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_REMOVE},
	{MKDSKT_SET, MKDSKT_INSERT},
	{MKDSKT_SET, MKDSKT_ENTER},
	{(int )NULL, (int )NULL}
    };
ASL_SCR_INFO menu_insert[DIAG_NUM_ENTRIES(msg_insert)];

/* menu 0x802022 */
struct msglist msg_complete[] =
    {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_FORMAT_COMPLETE},
	{MKDSKT_SET, MKDSKT_ENTER2},
	{(int )NULL, (int )NULL}
    };
ASL_SCR_INFO menu_complete[DIAG_NUM_ENTRIES(msg_complete)];


/* menu 0x802023 */
struct msglist msg_error[] =
   {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_FORMAT_BAD},
	{MKDSKT_SET, MKDSKT_ENTER2},
	{(int )NULL, (int )NULL}
   };
ASL_SCR_INFO menu_error[DIAG_NUM_ENTRIES(msg_error)]; 

/* menu 0x802024 */
struct msglist msg_no_drive[] =
   {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_NO_DRIVE},
	{(int )NULL, (int )NULL}
   };
ASL_SCR_INFO menu_no_drive[DIAG_NUM_ENTRIES(msg_no_drive)];

/* menu 0x802025 */
struct msglist msg_cfg_ok[] =
   {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_CFG_OK},
	{MKDSKT_SET, MKDSKT_ENTER2},
	{(int )NULL, (int )NULL}
   };
ASL_SCR_INFO menu_cfg_ok[DIAG_NUM_ENTRIES(msg_cfg_ok)]; 


/* menu 0x802026 */
/* menu created in function disp_consdef_menu() */

/* menu 0x802027 */
/* menu created in function */

/* menu 0x802028 */
struct msglist msg_formatting[] =
    {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_FORMATTING},
	{MKDSKT_SET, MKDSKT_STANDBY},
	{(int )NULL, (int )NULL}
    };
ASL_SCR_INFO menu_formatting[DIAG_NUM_ENTRIES(msg_formatting)];

/* menu 0x802029 */
struct msglist msg_cfg_no_chg[] =
   {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_CFG_NO_CHG},
	{MKDSKT_SET, MKDSKT_ENTER2},
	{(int )NULL, (int )NULL}
   };
ASL_SCR_INFO menu_cfg_no_chg[DIAG_NUM_ENTRIES(msg_cfg_no_chg)]; 

/* menu 0x802030 */
struct msglist msg_insert_cons[] =
    {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_REMOVE},
	{MKDSKT_SET, MKDSKT_INSERT_CONS},
	{MKDSKT_SET, MKDSKT_ENTER},
	{(int )NULL, (int )NULL}
    };
ASL_SCR_INFO menu_insert_cons[DIAG_NUM_ENTRIES(msg_insert_cons)];

/* menu 0x802031 */
struct msglist msg_cfg_error[] =
   {
	{MKDSKT_SET, MKDSKT_TITLE},
	{MKDSKT_SET, MKDSKT_CFG_BAD},
	{MKDSKT_SET, MKDSKT_ENTER2},
	{(int )NULL, (int )NULL}
   };
ASL_SCR_INFO menu_cfg_error[DIAG_NUM_ENTRIES(msg_cfg_error)]; 

struct	PdAt *pdat;
struct  listinfo obj_info;

extern char * getenv(char *);
extern char *diag_cat_gets();
extern  struct CuAt *getattr(char *dev,char *attr,int all,int *count);
extern nl_catd diag_catopen(char *, int);
int create_config_diskette(void);
void int_handler(int); 
void genexit(int);
int disp_menu(int,struct msglist *,ASL_SCREEN_CODE,ASL_SCR_INFO *,int );
int disp_sel_menu( int *);
int Usystem(char *,char *);
int format_diskette(int);
int check_drive(int);
char *cons_current(char *, char *, char *);
void edit_consdef(char *, char *, char *);
int drive_type(void);
int disp_consdef_menu(void);
int disp_refresh_menu(void);
FILE *fopen(char *, char*);

/* return code from create_disk ()*/
#define DONE	-1
#define	RETRY	-2

#define NO	1	/* first selection item */
#define YES	2	/* 2nd selection item */


/****************************************************************
* NAME: main
*
* FUNCTION: Facilitate building diagnostic utility diskettes from installed 
*	    AIX system. 
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	genexit()
*
* RETURNS:
*	Exits with 0	(=no problems)
*	Exits with 1	(exit due to signal)
****************************************************************/

main()
{
  int selection;
  int status = -1;
  int dskts;
  struct sigaction act;

	setlocale(LC_ALL,"");

	/* set up interrupt handler	*/
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, (struct sigaction *)NULL);
	sigaction(SIGHUP, &act, (struct sigaction *)NULL);
	sigaction(SIGQUIT, &act, (struct sigaction *)NULL);

	/* open the catalog file containing the menus */
	fdes = diag_catopen(MF_UMKDSKT, 0);

	/* initialize the ODM	*/
	odm_initialize();

	/* initialize ASL    	*/
	diag_asl_init("NO_TYPE_AHEAD");

	/* get the diagnostics data dir. */
	data_dir = (char *) getenv("DIAGDATADIR");
	if(data_dir == NULL)
		data_dir = UMKDSKT_DIR;

	/* Obtain info for consdef flow_disp attribute.	*/

	pdat=get_PdAt_list(PdAt_CLASS, 
		    "uniquetype=tty/rs232/tty and attribute=flow_disp",
		    &obj_info, 1, 1);

	/* if drive does not exist, exit */
	status = check_drive((int) 1);
	if(status != 0)
		genexit( 1 );

	do
	{
		int dp;

	  	status = disp_sel_menu( &dp);
		if(status == DIAG_ASL_COMMIT || status == DIAG_ASL_ENTER) 
		{

			switch(dp)
			{
				case 1:
				case 2:
				case 3:
					status = format_diskette(dp);
					break;
				case 4:
					status = create_config_diskette();
				default:
					break;

			}

		}
	  } 
	  while (status != DIAG_ASL_EXIT && status != DIAG_ASL_CANCEL );

	genexit(0);
}



/****************************************************************
* NAME: format_diskette
*
* FUNCTION: format the diskette
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	  disp_menu()
*
* RETURNS:
*	1 - some type of failure
*	0 - Success
*
****************************************************************/
int format_diskette(int type)
{
	int rc;
	int size;
	int sectors;
	char options[64];

	/* set the size of the diskette */
	switch(type)
	{
		case 1:
			/* size is 1 MB */
			size = 1;
			break;
		case 2:
			/* size is 2 MB */
			size = 2;
			break;
		case 3:
			/* size is 4 MB */
			size = 4;
			break;
		default:
			break;
	}
	rc = check_drive(size);

	if(rc != 0)
		return( 1 );


	/* put up insert diskette menu */
	rc = disp_menu(MENU_INSERT,msg_insert,ASL_DIAG_KEYS_ENTER_SC,menu_insert,size);
	if (rc != DIAG_ASL_ENTER)
	{
		return( rc );
	}

	/* put up formatting menu */
	rc = diag_display(MENU_FORMATTING, fdes, msg_formatting , DIAG_IO, 
			ASL_DIAG_OUTPUT_LEAVE_SC, &dm_menutype, menu_formatting);

	/* the number of sectors is the number of MB times 9 */
	sectors = size * 9;

	/* input is redirected so that you don't have to display messages from
	 * the format routine */
	sprintf(options,"-d /dev/rfd0.%d  < /dev/null",sectors);
	rc = Usystem(FORMAT,options);

	if(rc == 0)
	{
		/* put up successful completion menu */
		rc = disp_menu(MENU_COMPLETE,msg_complete,ASL_DIAG_KEYS_ENTER_SC,menu_complete,size);
	}
	else
	{
		/* put up failure completion menu */
		rc = disp_menu(MENU_ERROR,msg_error,ASL_DIAG_KEYS_ENTER_SC,menu_error,size);
	}
		

	return( 1 );
}


/****************************************************************
* NAME: check_drive
*
* FUNCTION: check the drive size
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	  disp_menu()
*
* RETURNS:
*	DONE	(=no problems)
*	RETRY	(=user want to retry the write operation when it fails)
*
****************************************************************/
int check_drive(int type)
{
	int rc = DONE; /* default to continue */

	rc = drive_type();
	/* a return of 1 indicates a 4 MB drive so you can format anything so
	 * return(0) OR if 2MB drive and wanting to format a 1 MB or 2 MB, 
	 * return(0) */
	if(rc == 1 || (rc == 0 && type != 4) )
		return(0);
	else 
	{
		rc = disp_menu(MENU_NO_DRIVE,msg_no_drive,ASL_DIAG_ENTER_SC,
			menu_no_drive,type);
		return(-1);
	}
	
}


/****************************************************************
* NAME: int_handler 
*
* FUNCTION: In case of an interrupt, this routine is called.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	genexit()
*
* RETURNS:
*	None
****************************************************************/
void int_handler(int sig)
{
	diag_asl_clear_screen();
	genexit(1);
}

/****************************************************************
* NAME: genexit 
*
* FUNCTION: Exit the ASL menu mode.  Relinquish all acquired
*	    ODM data.  Close the NL catalog "umkdskt.cat".
*	    Exit to process that invoked umkdskt. 
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	None
*
* RETURNS:
*	None
****************************************************************/
void genexit(int exitcode)
{

	
	/* remove the files which you have created */
	chdir(data_dir);
	Usystem(RM,"-f .signature CONSDEF REFRESH"); 

	if( (pdat != (struct PdAt *)NULL) &&
	    (pdat != (struct PdAt *)-1) )
		odm_free_list(pdat, &obj_info);

	diag_asl_quit();
	odm_terminate();
	catclose(fdes);
	exit(exitcode);
}

/****************************************************************
* NAME: disp_menu
*
* FUNCTION: Construct and display the menu of menu_no
*
* EXECUTION ENVIRONMENT:
*
* NOTES: menu_no - the menu number of displaying menu
*	 msg     - message list to be displayed
*	 scr_type- screen type used in diag_display()
*	 menu    - menu list to be constructed
*	 diskt_no- optional argument; used when menu_no is MENU_STANDBY
*		   or MENU_INSERT
* RETURNS:
*	 same return code of diag_display()
****************************************************************/
int disp_menu(int menu_no,struct msglist *msg,ASL_SCREEN_CODE scr_type,
		ASL_SCR_INFO *menu,int type)
{
	char entered_value[2];
	char *format_p = NULL;
	char temp[80];
	int rc;
	char drive_size[5];

	rc = diag_display(NULL, fdes,msg, DIAG_MSGONLY, NULL,
               			&dm_menutype, menu);

	switch( menu_no ) {
	case    MENU_INSERT:
			sprintf(temp,menu[2].text,type); 
			menu[2].text = temp;
			break;
	case    MENU_NO_DRIVE: 
			/* fill in the size of the diskette */
			switch(type)
			{
				case 4:
					strcpy(drive_size ,"2.88");
					break;
				default:
					strcpy(drive_size ,"1.44");
				     break;
			}
			format_p = (char *) malloc((strlen(menu[1].text) + 10));
			sprintf(format_p,menu[1].text,drive_size); 
			menu[1].text = format_p;

			break;
		
	case  MENU_COMPLETE: 
	case 	MENU_TOP:
	case	MENU_ERROR: /* do nothing */
	default:
			break;
	}
	
	rc = diag_display(menu_no, fdes, NULL , DIAG_IO, scr_type,
               			&dm_menutype, menu);
	if(format_p != NULL)
		free(format_p);

	return(rc);
}


/****************************************************************
* NAME: disp_sel_menu
*
* FUNCTION: Display the menu of selecting diskette 
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	NONE
*
* RETURNS:
*	the same as diag_display()
*	set the value of disk_no to the selected diskette
*
****************************************************************/
disp_sel_menu( int * dp)
{
	int 	i, rc, line=0;
	char	*buffer;
      static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

      /* allocate space for enough entries */
      rc = diag_display(MENU_TOP, fdes, msg_top, DIAG_IO,
                                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                                &restype, menu_top);
	  
	if ( rc == DIAG_ASL_COMMIT || rc == DIAG_ASL_ENTER) 
		*dp = DIAG_ITEM_SELECTED(restype);

	return(rc);
}

/****************************************************************
* NAME: disp_refresh_menu
*
* FUNCTION: Display/modify monitor refresh rate configuration file
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	NONE
*
* RETURNS:
*	the same as diag_display()
*
****************************************************************/
int disp_refresh_menu(void)
{
	int 	rc;
      static 	ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

	ASL_SCR_INFO    *dialog_info;
	int	line = 0;
	char	buf[2 * MAXPATHLEN];
	char	buf2[MAXPATHLEN];
	char	buf3[MAXPATHLEN];
	FILE	*fp;
	int	i;


	/* allocate space for 1 entry + 2 info lines*/
	dialog_info = (ASL_SCR_INFO *) calloc(10,
		sizeof(ASL_SCR_INFO));
	if (dialog_info == (ASL_SCR_INFO *)NULL)
		return(-1);

	/* set the title line in the array   */
	dialog_info[line++].text = diag_cat_gets(fdes, 
		MKDSKT_SET, MKDSKT_TITLE);

	/* set the refresh rate line */
	dialog_info[line].text = diag_cat_gets(fdes, 
		MKDSKT_SET, MKDSKT_RREFRESH);
	dialog_info[line].op_type = ASL_RING_ENTRY;
	dialog_info[line].entry_type = ASL_NO_ENTRY;
	dialog_info[line].required = ASL_NO;

	/* get default value */
	strcpy(buf3, "60");

	dialog_info[line].data_value = buf3;
	dialog_info[line].entry_size = BUFLEN - 1;

	/* get legal values */
	/* use these defaults if not listed in refresh.add */
	if (!strcmp(buf3, "60"))
		strcpy(buf2, "60,77");
	else
		strcpy(buf2, "77,60");

	dialog_info[line++].disp_values = buf2;

	/* set instruction line */
	dialog_info[line].text = diag_cat_gets(fdes, 
		MKDSKT_SET, MKDSKT_REFRESH);

	restype.max_index = line;

	for (rc = DIAG_ASL_OK; rc != DIAG_ASL_COMMIT && 
		rc != DIAG_ASL_CANCEL && rc != DIAG_ASL_EXIT;)
		rc = diag_display(MENU_MREFRESH, fdes, NULL, 
			DIAG_IO, ASL_DIAG_DIALOGUE_SC,
			&restype, dialog_info);

	if (rc == DIAG_ASL_COMMIT) 
	{

		if(dialog_info[1].changed != ASL_YES)
			return(NOT_CHANGED);

		/* Check for refresh.add existence first */
		sprintf(buf, "%s/REFRESH",  data_dir );

		if (access(buf, E_ACC))  { 
			chdir(data_dir);
			strcpy(buf,"odmget -q \"attribute=refresh_rate\" PdAt > REFRESH");
			system(buf);
		}

		/* write new REFRESH file */
sprintf(buf, "%s 's/^[ 		]*deflt.*=.*/	deflt = \"%s\"/' %s/REFRESH > %s/REFRESH.%d", SED, buf3, data_dir,  data_dir,  getpid());

			sprintf(buf2, "%s/REFRESH.%d", data_dir,  getpid());

			if (!system(buf)) {
				sprintf(buf3, "%s/REFRESH", data_dir);
				if (rename(buf2, buf3)) {
				/* display this menu if the renaming of files fails */
					rc = disp_menu(MENU_CFG_ERROR,msg_cfg_error,
						ASL_DIAG_KEYS_ENTER_SC,menu_cfg_error,0);
					if(rc == ASL_CANCEL)
						return(1);
					else
						return(rc);
				}
			}
			else {
			/* display this menu if the system call to create file 
			 * fails
			 */
					rc = disp_menu(MENU_CFG_ERROR,msg_cfg_error,
						ASL_DIAG_KEYS_ENTER_SC,menu_cfg_error,0);
					if(rc == ASL_CANCEL)
						return(1);
					else
						return(rc);
			}
			unlink(buf2);

			return(CHANGED);
	}

	return(rc);
}

/****************************************************************
* NAME: disp_consdef_menu
*
* FUNCTION: Display/modify consdef configuration file
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	cons_current(), edit_consdef()
*
* RETURNS:
*	the same as diag_display()
*
****************************************************************/
int disp_consdef_menu(void)
{
	int 	rc;
	int i;
        static 	ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

	for (i = 0; i != 1; i ++) {
			char	buf0[2 * MAXPATHLEN];
			char	buf[MAXPATHLEN];
			char	buf2[BUFLEN];
			char	buf3[BUFLEN];
			char	buf4[BUFLEN];
			char	buf5[BUFLEN];
			char	buf6[BUFLEN];
			char	buf7[BUFLEN];
			char	buf8[BUFLEN];
			int	line = 0;
			ASL_SCR_INFO    *dialog_info;

			/* allocate space for 8 entries + 2 info lines*/
			dialog_info = (ASL_SCR_INFO *) calloc(10,
				sizeof(ASL_SCR_INFO));
			if (dialog_info == (ASL_SCR_INFO *)NULL)
				return(-1);

			/* set the title line in the array   */
			dialog_info[line++].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_TITLE);

			/* set the configurable lines */
/* The next line is not used at this time.  If we ever want to add checking of
the system consdef file, change the MACRO SYSTEM_CONSDEF_DIR.  This variable,
buf0, is passed to the cons_current function.
For now, we are just going to return the deflt for all cons_current calls 

M. Lew
*/
			sprintf(buf0, "%s/CONSDEF", SYSTEM_CONSDEF_DIR );


			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_CONN);
			dialog_info[line].op_type = ASL_RING_ENTRY;
			dialog_info[line].entry_type = ASL_NO_ENTRY;
			dialog_info[line].required = ASL_NO;
			strcpy(buf, cons_current("connection", "rs232", buf0));
			dialog_info[line].data_value = buf;
			if (!strcmp(buf, "rs232"))
				dialog_info[line].disp_values = "rs232,rs422";
			else
				dialog_info[line].disp_values = "rs422,rs232";
			dialog_info[line++].entry_size = BUFLEN - 1;

			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_LOC);
			dialog_info[line].op_type = ASL_LIST_ENTRY;
			dialog_info[line].entry_type = ASL_TEXT_ENTRY;
			dialog_info[line].required = ASL_NO;
			dialog_info[line].disp_values = "00-00-S1-00";
			strcpy(buf2, cons_current("location", "00-00-S1-00",
				 buf0));
			dialog_info[line].data_value = buf2;
			dialog_info[line++].entry_size = BUFLEN - 1;

			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_BAUD);
			dialog_info[line].op_type = ASL_LIST_ENTRY;
			dialog_info[line].entry_type = ASL_NUM_ENTRY;
			dialog_info[line].required = ASL_NO;
			dialog_info[line].disp_values = "9600";
			strcpy(buf3, cons_current("speed", "9600",  buf0));
			dialog_info[line].data_value = buf3;
			dialog_info[line++].entry_size = BUFLEN - 1;

			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_BITS);
			dialog_info[line].op_type = ASL_RING_ENTRY;
			dialog_info[line].entry_type = ASL_NO_ENTRY;
			dialog_info[line].required = ASL_NO;
			strcpy(buf4, cons_current("bpc", "8",  buf0));
			dialog_info[line].data_value = buf4;
			if (!strcmp(buf4, "8"))
				dialog_info[line].disp_values = "8,7";
			else
				dialog_info[line].disp_values = "7,8";
			dialog_info[line++].entry_size = BUFLEN - 1;

			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_SBITS);
			dialog_info[line].op_type = ASL_RING_ENTRY;
			dialog_info[line].entry_type = ASL_NO_ENTRY;
			dialog_info[line].required = ASL_NO;
			strcpy(buf5, cons_current("stops", "1",  buf0));
			dialog_info[line].data_value = buf5;
			if (!strcmp(buf5, "1"))
				dialog_info[line].disp_values = "1,2";
			else
				dialog_info[line].disp_values = "2,1";
			dialog_info[line++].entry_size = BUFLEN - 1;
			if( (pdat == (struct PdAt *)NULL) ||
			    (pdat == (struct PdAt *)-1) ){
			    strcpy(buf6,cons_current("flow_disp", "xon", buf0));
			    dialog_info[line].disp_values = "xon,dtr,rts";
			}else{
			    strcpy(buf6,cons_current("flow_disp",
				pdat->deflt, buf0));
			    dialog_info[line].disp_values = pdat->values;
			}
			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_XON);
			dialog_info[line].op_type = ASL_RING_ENTRY;
			dialog_info[line].entry_type = ASL_NO_ENTRY;
			dialog_info[line].required = ASL_NO;
			dialog_info[line].data_value = buf6;
			dialog_info[line++].entry_size = BUFLEN - 1;

			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_PARITY);
			dialog_info[line].op_type = ASL_RING_ENTRY;
			dialog_info[line].entry_type = ASL_NO_ENTRY;
			dialog_info[line].required = ASL_NO;
			strcpy(buf7, cons_current("parity", "none",  buf0));
			dialog_info[line].data_value = buf7;
			if (!strcmp(buf7, "none"))
				dialog_info[line].disp_values = 
					"none,space,even,odd";
			else
				dialog_info[line].disp_values = 
					"space,even,odd,none";
			dialog_info[line++].entry_size = BUFLEN - 1;

			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_TERM);
			dialog_info[line].op_type = ASL_LIST_ENTRY;
			dialog_info[line].entry_type = ASL_TEXT_ENTRY;
			dialog_info[line].required = ASL_NO;
			dialog_info[line].disp_values = "ibm3163";
			strcpy(buf8, cons_current("term", "ibm3163",  buf0));
			dialog_info[line].data_value = buf8;
			dialog_info[line++].entry_size = BUFLEN - 1;

			/* set instruction line */
			dialog_info[line].text = diag_cat_gets(fdes, 
				MKDSKT_SET, MKDSKT_CONSDEF);

			restype.max_index = line;
			restype.cur_index = 0;

			for (rc = DIAG_ASL_OK; rc != DIAG_ASL_COMMIT && 
				rc != DIAG_ASL_CANCEL && rc != DIAG_ASL_EXIT;)
				rc = diag_display(MENU_MODCONS, fdes, NULL, 
					DIAG_IO, ASL_DIAG_DIALOGUE_SC,
					&restype, dialog_info);

			if (rc == DIAG_ASL_COMMIT) {
			int j;
			int changed;
			for(j=1,changed=0;j<=8;j++)
			{
				if(dialog_info[j].changed == ASL_YES)
					changed ++;
			}

			if(changed == 0)
				return(NOT_CHANGED);
			else
			{
				/* write new consdef file */
				/* if we assume we are starting from scrach, we don't
				 * need the next 2 lines. 
				 */
				sprintf(buf0, "%s %s/CONSDEF %s/CONSDEF.%d 2>/dev/null", CP, data_dir, data_dir,  getpid());
				system(buf0);

				sprintf(buf0, "%s/CONSDEF.%d", 
					data_dir,  getpid());

				edit_consdef("connection", 
						dialog_info[1].data_value,buf0);
				edit_consdef("location", 
						dialog_info[2].data_value,buf0);
				edit_consdef("speed", 
						dialog_info[3].data_value,buf0);
				edit_consdef("bpc", 
						dialog_info[4].data_value,buf0);
				edit_consdef("stops", 
						dialog_info[5].data_value,buf0);
				edit_consdef("flow_disp", 
						dialog_info[6].data_value,buf0);
				edit_consdef("parity", 
						dialog_info[7].data_value,buf0);
				edit_consdef("term", 
						dialog_info[8].data_value,buf0);

				sprintf(buf, "%s/CONSDEF", data_dir);
				if (rename(buf0, buf)) {
				/* put up failure menu if rename fails */
					rc = disp_menu(MENU_CFG_ERROR,msg_cfg_error,
						ASL_DIAG_KEYS_ENTER_SC,menu_cfg_error,0);
					if(rc == ASL_CANCEL)
						return(1);
					else
						return(rc);
				}
				return(CHANGED);
			} /* end of else, section executed if something changed */
		     } /* end of if(rc == ASL_COMMIT) statement */
	}

	return(0);
}

/****************************************************************
* NAME: cons_current
*
* FUNCTION: Get current /etc/consdef values.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	NONE
*
* RETURNS:	Current value for field (if defined in file), else deflt.
*
* NOTE: 10/15/93 This function has been disabled.  If there is a need to get
*       the system consdef and compare it, then the return(deflt) should
*	  be removed and the macro SYSTEM_CONSDEF_DIR should be changed to 
*   	  whatever the system directory is where the consdef file is located.
*
*/
char *cons_current(char *field, char *deflt, char *file)
{
	char	buf[MAXPATHLEN];
	FILE	*fp;
	int	i = 0;

/* The next line is causes a return of the deflt parameter always for right
now.  If we decide to change the SA to check the system configuraion file,
then this return should be deleted.  The rest of this function should still
be okay.

M. Lew

Also the file name passed in should be in lower case, i.e consdef, not
CONSDEF.

*/
	return(deflt);

	sprintf(buf, "%s '^[ 	]*%s=' %s | sed 's/.*=//'", GREP, field, file);

	if (fp = popen(buf, "r")) {
		for (; (current[i] = (char)getc(fp)) != (char)EOF && 
			current[i] != '\n' && i < BUFLEN; i++);
		current[i] = '\0';
		pclose(fp);
	}

	return(i ? current : deflt);
}

/****************************************************************
* NAME: edit_consdef
*
* FUNCTION: Edit /etc/consdef
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	NONE
*
* RETURNS:	none
*
*/
void edit_consdef(char *field, char *value, char *file)
{
	char	buf[2 * MAXPATHLEN];

	/* Is the field already defined?   grep for [whitespace]*field */
	/* all error messages sent to /dev/null */
	sprintf(buf, "%s -q '^[ 	]*%s' %s 2>/dev/null", EGREP,field, file);
	if (!system(buf)) {
		/* already defined, edit in place */
		char	file2[MAXPATHLEN];

		strcpy(file2, file);
		strcat(file2, ".tmp");
		sprintf(buf, "%s 's/^[ 	]*%s=.*/%s=%s/' %s > %s", SED, field, field, value, file, file2);
		if (!system(buf))
			rename(file2, file);
		else {
			/* sed error */
			unlink(file2);

			/* put up failure menu because could not create file */
			disp_menu(MENU_CFG_ERROR,msg_cfg_error,
						ASL_DIAG_KEYS_ENTER_SC,menu_cfg_error,0);
			return;
		}
	}
	else {
		/* not already defined, append */
		FILE	*fp;

		if (fp = fopen(file, "a")) {
			fprintf(fp, "%s=%s\n", field, value);
			fclose(fp);
		}
		else {
			/* fopen error */
			disp_menu(MENU_CFG_ERROR,msg_cfg_error,
				ASL_DIAG_KEYS_ENTER_SC,menu_cfg_error,0);
			return;
		}
	}
}

/****************************************************************
* NAME: Usystem
*
* FUNCTION: Run odm_run_method while print out stderr by popup
*	    when error occurs
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	NONE
*
* RETURNS: status from odm_run_methods or -999 if diag_asl_msg
*	   is executed.
*
*/

Usystem(char *cmd,char *option)
{
char *new_out,*new_err;
int status;
int rc;

	status = odm_run_method(cmd,option,&new_out,&new_err);
	/* Show errors via popup windown, then set status to -999 */
	/* This will prevent the caller from putting up any       */
	/* additional popup menu.				  */
	if (status && (*new_err || *new_out) ) {
		status = -999;
		if (*new_err && *new_out)
			rc = diag_asl_msg("%s %s",new_err,new_out);
		else
			if (*new_err)
				rc = diag_asl_msg("%s",new_err);
			else
				if (*new_out)
					rc = diag_asl_msg("%s",new_out);

	}

	free(new_out);
	free(new_err);
	return(status);
}

/****************************************************************
* NAME: drive_type
*
* FUNCTION: find the drive type
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	  disp_menu()
*
* RETURNS:
*	1 for 4MB
*	0 for 2MB
*	-1 for no drive or some other error.
*
****************************************************************/
int drive_type(void)
{
      struct CuAt *cuat;
	struct CuDv *cudv;
      struct listinfo obj_info;
	char criteria[128];

   	sprintf(criteria,"name = fd0");

	/* check to assure there is a drive in the system. */
	cudv = get_CuDv_list(CuDv_CLASS,criteria,&obj_info,1,1);
      if (cudv == (struct CuDv *) NULL) {
              return(-1);
      }

	/* get the data to find out what type of drive is installed */
      cuat = get_CuAt_list(CuAt_CLASS,criteria,&obj_info,1,1);
      /* if error - return error */
      if (cuat == (struct CuAt *) - 1) {
              return(-1);
      }
      /* else if none found - return tatus for 2MB*/
      else if (cuat == (struct CuAt *) NULL) {
              return(0);
      }

      if ( strcmp(cuat->value,"3.5inch4Mb") == 0 ) {
              return(1);
	} else {
              return(0);
     }


} /* end drive_type() */

/*
 * NAME: create_config_diskette();
 *
 * FUNCTION: calls functions to create consdef and refresh diskette
 *
 * RETURNS: create_config_diskette
 *          
 */

int create_config_diskette(void)
{
	int rc = 0;
	char buf[128];
        FILE    *ptr;
	
	int changed=NOT_CHANGED;

	/* change directories to the working directory */
	chdir(data_dir);

	/* initialize buf variable to files which should be written out */
	sprintf(buf," .signature ");

	/* if consdef is created, add to buf so that it will be written to disk */
	rc = disp_consdef_menu();
 	if(rc == CHANGED)
	{
		strcat(buf," CONSDEF ");
		changed = CHANGED;
	}
	else 
	{
		if(rc == ASL_CANCEL)
			return(1);
		if(rc == ASL_EXIT)
			return(rc);
	}

	/* if refresh is created, add to buf so that it will be written to disk */
	rc = disp_refresh_menu();
 	if(rc == CHANGED)
	{
		strcat(buf," REFRESH ");
 		changed = CHANGED;
	}
	else 
	{
		if(rc == ASL_CANCEL)
			return(1);
		if(rc == ASL_EXIT)
			return(rc);
	}

	if(changed == NOT_CHANGED)
	{
		rc = disp_menu(MENU_CFG_NO_CHG,msg_cfg_no_chg,ASL_DIAG_KEYS_ENTER_SC,menu_cfg_no_chg,0);
		return( 1 );
	}

	if(create_sig() != 0)
	{
		rc = disp_menu(MENU_CFG_ERROR,msg_cfg_error,ASL_DIAG_KEYS_ENTER_SC,menu_cfg_error,0);
		if(rc == ASL_CANCEL)
			return(1);
		else
			return(rc);
	}

	/* ask the user to insert a diskette */
	rc = disp_menu(MENU_INSERT_CONS,msg_insert_cons,ASL_DIAG_KEYS_ENTER_SC,menu_insert_cons,2);
	if (rc != DIAG_ASL_ENTER)
	{
		return( rc );
	}

        /* first verify that we can write to the diskette */
        if ((ptr = fopen("/dev/rfd0", "wb")) == (FILE *)NULL ) {
                diag_asl_msg("%s",strerror(errno));
                fclose(ptr);
                return(1);
        }
        fclose(ptr);

	strcat(buf," | backup -iqv " );
	if(Usystem(LS,buf) == 0)
		rc = disp_menu(MENU_CFG_OK,msg_cfg_ok,ASL_DIAG_KEYS_ENTER_SC,menu_cfg_ok,0);

	return(1);
}

/*
 * NAME: create_sig
 *
 * FUNCTION: create signature file
 *
 * RETURNS: -1 for fail
 *           0 for ok
 */

create_sig()
{
	char tmp[128];
	int fd;
	int rc;
	extern int errno;

	/* set up the path name of the .signature file */
	sprintf(tmp,"%s/.signature",data_dir);

	/* open the file */
	fd = open(tmp,O_RDWR|O_CREAT,_S_IFREG|S_IWUSR|S_IRUSR);
	if(fd < 0)
	{
		rc = disp_menu(MENU_CFG_ERROR,msg_cfg_error,ASL_DIAG_KEYS_ENTER_SC,menu_cfg_error,0);
		return(-1);
	}

	/* write in the .signature string */
	strcpy(tmp,"/etc/diagconf");
	if(write(fd,tmp,strlen(tmp)) == -1)
	{
		rc = disp_menu(MENU_CFG_ERROR,msg_cfg_error,ASL_DIAG_KEYS_ENTER_SC,menu_cfg_error,0);
		return(-1);
	}

	close(fdes);
	return(0);
}
