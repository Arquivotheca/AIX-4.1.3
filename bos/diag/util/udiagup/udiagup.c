static char sccsid[] = "@(#)64	1.6.1.11  src/bos/diag/util/udiagup/udiagup.c, dsaudiagup, bos411, 9428A410j 4/8/94 11:12:03";
/*
 *   COMPONENT_NAME: DSAUDIAGUP
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		FREE_STR
 *		disp_menu
 *		genexit
 *		int_handler
 *		main
 *		update_hardfile
 *		determine_install_devices
 *		build_menu
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <odmi.h>
#include <sys/cfgdb.h>
#include <sys/access.h>
#include <locale.h>
#include <diag/class_def.h>
#include <diag/diago.h>
#include <diag/diag.h>
#include "udiagup_msg.h"
#include "udiagup.h"

#define MENU_TOP	0x802160	/* Top menu */
#define MENU_SELECT	0x802161	/* Select device menu */    
#define MENU_INSERT	0x802162	/* Insert Diskette menu */
#define MENU_MORE	0x802163	/* More update? */
#define MENU_NOUP	0x802164	/* No update */
#define MENU_NONE	0x802165	/* No device found */

/* menu 0x802160 */
struct msglist msg_top[] = 
    {
	{DIAGUP_SET, DIAGUP_TITLE},
	{DIAGUP_SET, DIAGUP_PROLOG_NEW},
	{DIAGUP_SET, DIAGUP_END},
	{(int )NULL, (int )NULL}
    };
ASL_SCR_INFO menu_top[DIAG_NUM_ENTRIES(msg_top)];

/* menu 0x802161 */
struct msglist msg_select[] =
    {
	{DIAGUP_SET, DIAGUP_TITLE},
	{DIAGUP_SET, DIAGUP_SELECT},
	{(int )NULL, (int )NULL}
    };
ASL_SCR_INFO menu_select[DIAG_NUM_ENTRIES(msg_select)];

/* menu 0x802162 */
struct msglist msg_insert[] =
    {
	{DIAGUP_SET, DIAGUP_TITLE},
	{DIAGUP_SET, DIAGUP_INSERT},
	{DIAGUP_SET, DIAGUP_ENTER},
	{(int )NULL, (int )NULL}
    };
ASL_SCR_INFO menu_insert[DIAG_NUM_ENTRIES(msg_insert)];

/* menu 0x802164 */
struct msglist msg_noup[] =
   {
	{DIAGUP_SET, DIAGUP_TITLE},
	{DIAGUP_SET, DIAGUP_YES},
	{DIAGUP_SET, DIAGUP_NO},
	{DIAGUP_SET, DIAGUP_NOUP},
	{(int )NULL, (int )NULL}
   };
ASL_SCR_INFO menu_noup[DIAG_NUM_ENTRIES(msg_noup)];

/* menu 0x802163 */
struct msglist msg_more[] =
   {
	{DIAGUP_SET, DIAGUP_TITLE},
	{DIAGUP_SET, DIAGUP_YES},
	{DIAGUP_SET, DIAGUP_NO},
	{DIAGUP_SET, DIAGUP_MORE},
	{(int )NULL, (int )NULL}
   };
ASL_SCR_INFO menu_more[DIAG_NUM_ENTRIES(msg_more)];

/* menu 0x802165 */
struct msglist msg_none[] =
   {
	{DIAGUP_SET, DIAGUP_TITLE},
	{DIAGUP_SET, DIAGUP_NONE},
	{DIAGUP_SET, DIAGUP_EXIT},
	{(int )NULL, (int )NULL}
   };
ASL_SCR_INFO menu_none[DIAG_NUM_ENTRIES(msg_none)];

/* global values */
nl_catd		fdes;				/* catalog file descriptor  */

#define	NO	2	/* first selection item */
#define YES	1	/* 2nd selection item */

#define DISKETTE	0
#define TAPE		1
	 
#define FREE_STR(x) if (x != (char *)NULL) free(x)

/*GLOBAL VARIABLES 	*/

short	installp_invoked=0;
struct dev{
	int d_type;
	char *d_name;
        }devices[16];	/* store device names */

/* FUNCTION PROTOTYPES */
void int_handler(int); 
void genexit(int);
int update_hardfile(int);
int disp_menu(int,struct msglist *,ASL_SCREEN_CODE,ASL_SCR_INFO *,int,int);
int determine_install_devices();
int build_menu(int *,int);

/* EXTERNAL VARIABLES	*/
extern nl_catd diag_catopen(char *, int);
extern ASL_SCR_TYPE dm_menutype;
extern char *chkdskt(char *);
extern char *strchr(char *, char);

/****************************************************************
* NAME: main
*
* FUNCTION: Update the hardfile based diagnostics from diagnostic   
*	    diskette.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	disp_menu()
*	determine_install_devices()
*	build_menu()
*	update_hardfile()
*	genexit()
*	
* RETURNS:
*	Exits with 0	(=no problems)
****************************************************************/

main()
{
	int selection;
  	int status;
	int num_devices;
  	struct sigaction act;
	int flag;
	setlocale(LC_ALL,"");

	/* set up interrupt handler	*/
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, (struct sigaction *)NULL);

	/* initialize ASL    	*/
	diag_asl_init("NO_TYPE_AHEAD");

	/* open the catalog file containing the menus */
	fdes = diag_catopen(MF_UDIAGUP, 0);

	/* initialize the ODM	*/
	odm_initialize();
	flag = 0;	/* no substitution in the messages */
	selection = 0;
	status = disp_menu(MENU_TOP,msg_top,ASL_DIAG_ENTER_SC,menu_top,flag,
			selection);
	
	if (status == DIAG_ASL_ENTER){
		num_devices = determine_install_devices();
		if (num_devices == 0){
			status = disp_menu(MENU_NONE,msg_none,ASL_DIAG_ENTER_SC,
					menu_none,flag,selection);
			status = DIAG_ASL_EXIT;
		}
	}

	if (status == DIAG_ASL_ENTER){
	   do{
	      do{
		selection = 0;
		if (status == DIAG_ASL_ENTER || status == DIAG_ASL_CANCEL){	
			if (num_devices > 1){
			    do{		
				status = build_menu(&selection,num_devices);
				if (status == DIAG_ASL_ENTER){
				flag = 1;
				status = disp_menu(MENU_INSERT,msg_insert,
					  ASL_DIAG_ENTER_SC,menu_insert,
			       		  flag,selection);
				}
				else
					break;
			    } while (status == DIAG_ASL_CANCEL);
			}
			else {
				flag = 1;
				status = disp_menu(MENU_INSERT,msg_insert,
					  ASL_DIAG_ENTER_SC,menu_insert,
			       		  flag,selection);
			}
		}
		if (status == DIAG_ASL_ENTER || status == DIAG_ASL_EXIT) 
			break;
		if (status == DIAG_ASL_CANCEL){
			flag = 0;
			status = disp_menu(MENU_TOP,msg_top,ASL_DIAG_ENTER_SC,
					menu_top,flag,selection);
			if (status == DIAG_ASL_CANCEL)
				genexit(0);
		}
	      } while (status != DIAG_ASL_EXIT || status != DIAG_ASL_CANCEL);
		if (status == DIAG_ASL_ENTER){
		   do{
			diag_asl_clear_screen();
			asl_restore();
			status = update_hardfile(selection);
			diag_asl_clear_screen();
			asl_reset();
			switch(status){
			case 0:
		    	    status = disp_menu(MENU_MORE,msg_more,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					menu_more,flag,selection);
			    if (status == DIAG_ASL_COMMIT &&
			        DIAG_ITEM_SELECTED(dm_menutype)==YES)
					status=DIAG_ASL_ENTER;
			    else if (status == DIAG_ASL_COMMIT &&
			        DIAG_ITEM_SELECTED(dm_menutype)==NO )
					status=DIAG_ASL_EXIT;
			    break;
			default:
		    	    status = disp_menu(MENU_NOUP,msg_noup,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					menu_noup,flag,selection);
			    if (status == DIAG_ASL_COMMIT &&
			        DIAG_ITEM_SELECTED(dm_menutype)==YES)
					status=DIAG_ASL_ENTER;
			    else if (status == DIAG_ASL_COMMIT &&
			        DIAG_ITEM_SELECTED(dm_menutype)==NO )
					status=DIAG_ASL_EXIT;
			    break;
			}
			if (status == DIAG_ASL_ENTER)
				break;
		
			if (status == DIAG_ASL_CANCEL){
				flag = 1;
				status = disp_menu(MENU_INSERT,msg_insert,
					  ASL_DIAG_ENTER_SC,menu_insert,
			       	  		flag,selection);
				if (status == DIAG_ASL_CANCEL)
					break;
			}
		    } while (status != DIAG_ASL_EXIT);		
		}
	
	  } while (status != DIAG_ASL_EXIT);
	} 
	genexit(0);
}


/*********************************************************************
* NAME: update_hardfile
*
* FUNCTION: process hard file updates
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*	return code from installp command
********************************************************************/
int update_hardfile(choice)
int choice;
{
int rc;
char string[80];
char *buffer;
	buffer = malloc(256);	
	
	printf(diag_cat_gets(fdes, DIAGUP_SET, DIAGUP_PLS_STANDBY));
	installp_invoked=1;
	
	if (devices[choice].d_type == DISKETTE)
		sprintf(string,"/usr/sbin/installp -qa -d /dev/r'%s' -X all",
				devices[choice].d_name);
	else
		sprintf(string,"/usr/sbin/installp -qa -d /dev/'%s'.1 -X all",
				devices[choice].d_name);
	rc = system(string);
	installp_invoked=0;
	FREE_STR(string);
	free(buffer);
	return(rc);
}

/**********************************************************************
* NAME: int_handler 
*
* FUNCTION: In case of an interrupt, this routine is called.
*	    (NOTE:  It was unclear whether this routine would
*	    be called with 1 argument, as indicated by the
*	    sigaction structure, or 3 arguments, as indicated
*	    in the documentation of system call sigaction().)
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	genexit()
*
* RETURNS:
*	None
*********************************************************************/
void int_handler(int sig)
{
	diag_asl_clear_screen();
	if(!installp_invoked)
		genexit(1);
}

/*******************************************************************
* NAME: genexit 
*
* FUNCTION: Exit the ASL menu mode.  Relinquish all acquired
*	    ODM data.  Close the NL catalog.
*	    Exit to process that invoked udiagup. 
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	None
*
* RETURNS:
*	None
*******************************************************************/
void genexit(int exitcode)
{
	diag_asl_quit();
	odm_terminate();
	catclose(fdes);
	exit(exitcode);
}

/**********************************************************************
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
* RETURNS:
*	 same return code of diag_display()
***********************************************************************/
int disp_menu(int menu_no,struct msglist *msg,ASL_SCREEN_CODE scr_type,
		ASL_SCR_INFO *menu,int flag,int choice)
{
int rc;
char *buffer;
char *temp;
	
	buffer = malloc(256);
	temp = malloc(2048);
	if (flag){
		if (devices[choice].d_type == TAPE)
			sprintf(buffer,diag_cat_gets(fdes,DIAGUP_SET,
				DIAGUP_TAPE));
		else
			sprintf(buffer,diag_cat_gets(fdes,DIAGUP_SET,
				DIAGUP_DISKETTE));		
		rc = diag_display(NULL, fdes,msg, DIAG_MSGONLY, NULL,
               			&dm_menutype, menu);
		switch(menu_no){
		case MENU_INSERT:
			sprintf(temp,menu[1].text,buffer,buffer);
			free(menu[1].text);
			menu[1].text = temp;
			break;
		case MENU_MORE:
		case MENU_NOUP:
			break;
			
		default:
			sprintf(temp,menu[1].text,buffer);
			free(menu[1].text);
			menu[1].text = temp;
			break;
		}
		rc = diag_display(menu_no, fdes, NULL , DIAG_IO, scr_type,
               			&dm_menutype, menu);
	}
	else {
		rc = diag_display(NULL, fdes,msg, DIAG_MSGONLY, NULL,
               			&dm_menutype, menu);

		rc = diag_display(menu_no, fdes, NULL , DIAG_IO, scr_type,
               			&dm_menutype, menu);
	}
	free(temp);
	free(buffer);
	return(rc);
}

/***********************************************************************
* NAME: determine_install_devices
*
* FUNCTION: Determine the devices that are connected to the machine
*
* EXECUTION ENVIRONMENT:
*
*
* RETURNS:
*	dev_count:number of devices
***********************************************************************/
int determine_install_devices()
{
  int index;
  int dev_count = 0;
  struct CuDv *cudv;
  struct listinfo c_info;

	cudv = get_CuDv_list(CuDv_CLASS,"status = 1 and chgstatus != 3",
		&c_info,1,1);
        
	if (cudv == (struct CuDv *) -1)
		return (-1);
	     
	for (index = 0; index < c_info.num; index++){
	 	if (!strncmp(cudv->PdDvLn_Lvalue,"tape",4)){
			devices[dev_count].d_type = TAPE;
			devices[dev_count].d_name = cudv->name;
			dev_count++;
		}  
		if (!strncmp(cudv->PdDvLn_Lvalue,"diskette",8)){	
			devices[dev_count].d_type = DISKETTE;
			devices[dev_count].d_name = cudv->name;
			dev_count++;
		}
		cudv++;
	}
	return(dev_count);

}

/*********************************************************************
* NAME: build_menu
*
* FUNCTION: Construct and display the menu by using the list 
*		of available devices 
*
* EXECUTION ENVIRONMENT:
*
*
* RETURNS:
*	same return code of diag_display()
*********************************************************************/
int build_menu(choice,count_devices)
int	*choice;
int     count_devices;
{
	int index;
	int rc;
	int line = 0;

 	ASL_SCR_INFO *resinfo;
	static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

	resinfo = (ASL_SCR_INFO *) calloc(16,sizeof(ASL_SCR_INFO));
	if ( resinfo == (ASL_SCR_INFO *) -1 )
		return(-1);
        
	resinfo[line++].text=(char *)diag_cat_gets(fdes, DIAGUP_SET,
					DIAGUP_TITLE);
	
	for( index = 0; index < count_devices; index++ )
		resinfo[line++].text = devices[index].d_name; 

	resinfo[line].text=(char *)diag_cat_gets(fdes, DIAGUP_SET,
					DIAGUP_SELECT);
	
	restype.max_index = line;
	restype.cur_index = 1;
	
	rc = diag_display(MENU_SELECT, fdes, NULL, DIAG_IO, 
				ASL_DIAG_LIST_CANCEL_EXIT_SC,
				&restype, resinfo);
	if ( rc == DIAG_ASL_COMMIT ){
		*choice = DIAG_ITEM_SELECTED(restype)-1;
		rc = DIAG_ASL_ENTER;
	}
	free(resinfo);
	return(rc);

}
