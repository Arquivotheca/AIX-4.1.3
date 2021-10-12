static char sccsid[] = "@(#)02	1.6  src/bos/diag/util/udiagmon/udiagmon.c, dsaudiagmon, bos411, 9430C411a 7/26/94 14:44:21";
/*
 * COMPONENT_NAME: (DSAUDIAGMON) DIAGNOSTIC UTILITY
 *
 * FUNCTIONS: 	
 *      	main
 *		add
 *		delete
 *		modify
 *		get_time
 *		int_handler
 *		genexit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <nl_types.h>
#include <locale.h>
#include <errnotify.h>

#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include "diag/diag_define.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/tmdefs.h"
#include "diag/diagodm.h"

#include "udiagmon_msg.h"


                      /* checks if PDiagAtt's value field = 1                 */
#define test_val(val) (((int)atoi(val) & SUPTESTS_PERIODIC_MODE)? TRUE:FALSE)

#define UDIAGMON_MENU1	0x802150
#define UDIAGMON_MENU2	0x802151
#define UDIAGMON_MENU3	0x802152
#define UDIAGMON_MENU4	0x802153
#define UDIAGMON_MENU5	0x802154
#define UDIAGMON_MENU6	0x802155
#define UDIAGMON_MENU7	0x802156
#define UDIAGMON_MENU8	0x802157
#define UDIAGMON_MENU9	0x802158
#define UDIAGMON_MENU10	0x802159
#define UDIAGMON_MENU11	0x802250

#define	DISPLAY	1
#define	ADD	2
#define	DELETE	3
#define	MODIFY	4

#define	DIAGELA	"diagela"

/* Global variables */
nl_catd		fdes;		/* catalog file descriptor	*/
int 		num_All;        /* number of devices in All              */
int 		num_Top;        /* number of devices in Top              */
nl_catd 	fdes;           /* catalog file descriptor               */

int 		save_cdd_flag;
char 		udiag_lck_file[255];
int  		udiag_lck_fd;

diag_dev_info_t     *Top = -1;  /* pointer to all device data structures */
diag_dev_info_ptr_t *Allp;      /* array containing devices in All       */
diag_dev_info_ptr_t *DTL_disp;  /* array containing devices in test list */

static  struct Class *Nclp;

/* First menu displayed to show instructions	*/
struct	msglist	instructions[] = {
	{ UDIAGMON_SET, UDIAGMON_MENU_1 },
	{ (int) NULL, (int) NULL	},
};
ASL_SCR_INFO	first_menu[ DIAG_NUM_ENTRIES(instructions) ];

/* Second menu to display selection 		*/
/* This menu contains the ENABLE error log analysis selection */
struct	msglist	en_selection[] = {
	{ UDIAGMON_SET, UDIAGMON_TITLE },
	{ UDIAGMON_SET, UDIAGMON_SELECTION1 },
	{ UDIAGMON_SET, UDIAGMON_SELECTION2 },
	{ UDIAGMON_SET, UDIAGMON_SELECTION3 },
	{ UDIAGMON_SET, UDIAGMON_SELECTION4 },
	{ UDIAGMON_SET, UDIAGMON_SELECTION5 },
	{ UDIAGMON_SET, UDIAGMON_LASTLINE },
	{ (int) NULL, (int) NULL	},
};

/* This menu contains the DISABLE error log analysis selection */
struct	msglist	dis_selection[] = {
	{ UDIAGMON_SET, UDIAGMON_TITLE },
	{ UDIAGMON_SET, UDIAGMON_SELECTION1 },
	{ UDIAGMON_SET, UDIAGMON_SELECTION2 },
	{ UDIAGMON_SET, UDIAGMON_SELECTION3 },
	{ UDIAGMON_SET, UDIAGMON_SELECTION4 },
	{ UDIAGMON_SET, UDIAGMON_SELECTION6 },
	{ UDIAGMON_SET, UDIAGMON_LASTLINE },
	{ (int) NULL, (int) NULL	},
};

/* Menu to show resources currently in the periodic test */
/* list.						 */
struct	msglist device_in_test_list[] = {
	{ UDIAGMON_SET, UDIAGMON_DISPLAY },
	{ UDIAGMON_SET, UDIAGMON_SCROLL },
	{ (int) NULL, (int) NULL },
};

/* Menu to indicate concurrent ELA has been enabled	*/
struct	msglist ela_enabled[] = {
	{ UDIAGMON_SET, UDIAGMON_ELA_ENABLED },
	{ UDIAGMON_SET, UDIAGMON_CONTINUE },
	{ (int) NULL, (int) NULL	},
};
ASL_SCR_INFO	ela_enabled_menu[ DIAG_NUM_ENTRIES(ela_enabled) ];

/* Menu to indicate concurrent ELA has been disabled	*/
struct	msglist ela_disabled[] = {
	{ UDIAGMON_SET, UDIAGMON_ELA_DISABLED },
	{ UDIAGMON_SET, UDIAGMON_CONTINUE },
	{ (int) NULL, (int) NULL	},
};
ASL_SCR_INFO	ela_disabled_menu[ DIAG_NUM_ENTRIES(ela_disabled) ];

/* Extern variables */
extern 	ASL_SCR_TYPE 	dm_menutype;
extern 	nl_catd		diag_catopen(char *, int);

/* EXTERNAL FUNCTIONS */
extern 	char		*diag_cat_gets();

/* FUNCTION PROTOTYPES */
int	display_test_list(int, int, ASL_SCREEN_CODE, struct msglist *, int *);
void 	resource_desc(int, char *, diag_dev_info_t *);
void 	copy_text(int, char *, char *);
int 	add(void);
int 	delete(void);
void 	int_handler(int);
void 	genexit(int);
int 	get_time();

/*  */
/* NAME: main
 *
 * FUNCTION: Manage periodic test list and Concurrent Error Log
 *	     Analysis program.
 * 
 * RETURNS: 0
 *
 */

main(int argc, char *argv[])
{
int 	status;
int	rc;
int 	rc1;
char	*args[3];

int	enabled_ela;
struct 	msglist *selection;
struct 	listinfo result;

int 	lockfd;
char 	lockfile[255];

char 	diagela[255];	/* Full pathname of diagela program */


	setlocale(LC_ALL, "");

	/* catalog name is the program name with .cat extension */
	diag_asl_init("NO_TYPE_AHEAD");
	fdes = diag_catopen(MF_UDIAGMON, 0);

	lock_self();

	init_dgodm();

	/* set up interrupt handlers, here! Will take care of closing
	   odm,asl,lock file etc. on interrupts.
	*/
	handle_signals();

	/* Display first menu of Service Aid. This menu explains what */
	/* the Service Aid's functions are.				      */

	status = diag_display(UDIAGMON_MENU1, fdes, instructions, DIAG_IO,
			ASL_DIAG_ENTER_SC, &dm_menutype, first_menu);
	if(status == DIAG_ASL_CANCEL || status == DIAG_ASL_EXIT)
		genexit(0);

	/* Initialize the data structures for all resources.   */

	if ((Top = init_diag_db( &num_Top )) == (diag_dev_info_t *) -1 )
			genexit(1);

	/* get list of all devices to be included in diag test selection */
	Allp = generate_All( num_Top, Top , &num_All);

	/* parallely, create a Device Test List for periodic testing */
	DTL_disp = (diag_dev_info_ptr_t *)
		   calloc(num_All+5, sizeof(DTL_disp[0]));

	/* May have to go through DTL_disp to pick only the devices we  */
	/* want to test periodically.					*/

	/* check errnotify object class to get ela status 		*/
	enabled_ela = FALSE;
        if ((Nclp = odm_mount_class("errnotify")) == (struct Class *)-1) {
                Nclp = 0;
        }
        if (Nclp != 0)
                enabled_ela = (get_errnotify_list(Nclp,"en_name=diagela",
                        &result, MAX_EXPECT, 1) == NULL)? FALSE : TRUE;
	sprintf(diagela,"%s/%s", DEFAULT_UTILDIR, DIAGELA);

	status = DIAG_ASL_OK;
	while (status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT) {

		/* display top menu.This menu has the selection */
		/* for what operation the user wants to perform */

		selection = (enabled_ela)? &dis_selection[0] : &en_selection[0];

		status = diag_display(UDIAGMON_MENU2, fdes, selection, DIAG_IO,
				ASL_DIAG_LIST_CANCEL_EXIT_SC, NULL, NULL);
		if(status == DIAG_ASL_COMMIT){
			switch(DIAG_ITEM_SELECTED(dm_menutype))
			{
			case 1: /* Add a resource to the periodic test list */
				add();
				break;
			case 2: /* Delete resource from  periodic test list */
				delete();
				break;
			case 3: /* Modify time to test a resource  */
				modify();
				break;
			case 4: /* Display the periodic test list  */
				display_test_list(DISPLAY,
					UDIAGMON_MENU5, 
					ASL_DIAG_LIST_CANCEL_EXIT_SC, 
					device_in_test_list, &rc);
				break;
			case 5: /* Enable Concurrent Error Log Analysis */
				if (enabled_ela == FALSE)
				{
					args[0]= DIAGELA;
					args[1]="ENABLE";
					args[2]= (char *) NULL;

					rc = diag_asl_execute(diagela, args, 
									&rc1);
					if( (rc1 & 0xFF) || (rc < 0) )
						rc = diag_hmsg(fdes, 
							UDIAGMON_SET,
							NO_ENABLE, NULL);
					else
					{
						rc = diag_display(
						UDIAGMON_MENU6, fdes,
						ela_enabled, DIAG_IO,
			    	 		ASL_DIAG_ENTER_SC, 
						&dm_menutype, ela_enabled_menu);

						enabled_ela = TRUE;
					}
				}
				else
				{ /* Disable Concurrent Error Log Analysis */

					args[0]= DIAGELA;
					args[1]="DISABLE";
					args[2]= (char *) NULL;

					rc = diag_asl_execute(diagela, args, 										&rc1);
					if( (rc1 & 0xFF) || (rc < 0) )
						rc = diag_hmsg(fdes, 
						UDIAGMON_SET,
						NO_DISABLE, NULL);
					else
					{
						rc = diag_display(
						UDIAGMON_MENU7, fdes,
						ela_disabled, DIAG_IO,
		    	 			ASL_DIAG_ENTER_SC, 
						&dm_menutype,ela_disabled_menu);

						enabled_ela = FALSE;
					}
				}
				break;
			default: /* Software error */
				break;
			}
		}

	} /* end of while loop */
	if ((status == DIAG_ASL_CANCEL) && save_cdd_flag )
	{
		int  rc;

		save_cdd_flag = FALSE;

		rc = diag_hmsg(fdes, UDIAGMON_SET, SAVE_CANCEL, NULL);
		
		if (rc != DIAG_ASL_CANCEL)
			save_cdd_flag = TRUE;
	}

	if (lockfd >= 0)
		unlink(lockfile);

	genexit(0);

} /* end of main() function */

/*  */
/*
 * NAME: init_sigaction
 *                                                                    
 * FUNCTION: Register an action routine for a signal.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS: NONE
 *
 */  

init_sigaction(sig, sact, handler)
int 	sig;
struct 	sigaction *sact;
void 	(handler)();
{
short 	i = sizeof(struct sigaction);
char 	*sp = (char *)sact;
	
	/* init sigaction structure */
	for (; (i--);)
		*(sp+i) = 0;

	sact->sa_handler = handler;
	sigaction(sig, sact, (struct sigaction *)NULL);
}

/*  */
/*
 * NAME: handle_signals
 *                                                                    
 * FUNCTION: handle all the most familiar signals.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS: NONE
 *
 */  

handle_signals()
{
int	sig;
struct 	sigaction siga;

	/* Handle only signals mostly familiar to the process */

	for (sig=1; sig <= SIGMAX; sig++)
	{
		switch(sig)
		{
			case SIGUSR1:
			case SIGUSR2:
			case SIGILL:
			case SIGQUIT:
			case SIGINT:
			case SIGABRT:
			case SIGTERM:
			case SIGHUP:
				init_sigaction(sig, &siga, int_handler);
			break;
			default:
			break;
		}

	}
}


/*  */
/*
 * NAME: get_time
 *                                                                    
 * FUNCTION: Present dialog box and prompt for time of day
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */  

int 
get_time(action, dev_ptr)
short 		action;
diag_dev_info_t *dev_ptr;
{

int 	rc,hour,min;
int 	line = 0;
char	entered_minute[3];
char	entered_hour[3];
char	string[256];
char	title[512];
char	time[5];

static char	valid_minute[] = { "00,05,10,15,20,25,30,35,40,45,50,55"};
static char	valid_hour[] = { "00,01,02,03,04,05,06,07,08,09,10,11,12,13,14,15,16,17,18,19,20,21,22,23" };

static ASL_SCR_TYPE 	menutype = DM_TYPE_DEFAULTS;
ASL_SCR_INFO    	*dialog_info;

        /* allocate space for at least 5 entries */
        dialog_info = (ASL_SCR_INFO *) calloc(5,sizeof(ASL_SCR_INFO) );
        if ( dialog_info == (ASL_SCR_INFO *) NULL)
                return(1);

     	resource_desc(action, string, dev_ptr);

        /* set the title line in the array   */
        sprintf(title,diag_cat_gets(fdes,UDIAGMON_SET,UDIAGMON_ADD), string,
		diag_cat_gets(fdes,UDIAGMON_SET,UDIAGMON_ENTERTIME_1));

	dialog_info[line++].text = title;

	/* hour prompt */
        dialog_info[line].text = diag_cat_gets(fdes, UDIAGMON_SET, 
							UDIAGMON_HOUR);
	sprintf(time, "%04d", dev_ptr->T_CDiagDev->Periodic);
	if(action == MODIFY)
	{
		entered_hour[0] = time[0]; 
		entered_hour[1] = time[1]; 
	}
	else
		entered_hour[0] = entered_hour[1] = '0';
	entered_hour[2] = '\0';
	dialog_info[line].op_type = ASL_RING_ENTRY;
	dialog_info[line].entry_type = ASL_NUM_ENTRY;
	dialog_info[line].required = ASL_YES;
	dialog_info[line].disp_values = valid_hour;
	dialog_info[line].data_value = entered_hour;
	dialog_info[line].entry_size = 2;
	line++;
	
	if(action == MODIFY)
	{
		entered_minute[0] = time[2]; 
		entered_minute[1] = time[3]; 
	}
	else
		entered_minute[0] = entered_minute[1] = '0';
	entered_minute[2] = '\0';
	dialog_info[line].op_type = ASL_RING_ENTRY;
	dialog_info[line].entry_type = ASL_NUM_ENTRY;
	dialog_info[line].required = ASL_YES;
	dialog_info[line].disp_values = valid_minute;
	dialog_info[line].data_value = entered_minute;
	dialog_info[line].entry_size = 2;
       	dialog_info[line++].text = diag_cat_gets(fdes, 
						UDIAGMON_SET, UDIAGMON_MINUTE);
        dialog_info[line].text = diag_cat_gets(fdes, UDIAGMON_SET, BLANK);
	menutype.max_index = line ;

	while (TRUE) 
	{
		rc = diag_display( UDIAGMON_MENU8, fdes, NULL, DIAG_IO,
			ASL_DIAG_DIALOGUE_SC, &menutype, dialog_info );

		if ( rc == DIAG_ASL_HELP )
			diag_hmsg(fdes, HELP_SET, HELP_MSG1, NULL);

		if ( rc == DIAG_ASL_COMMIT ) 
		{
			sprintf(time, "%02s%02s", dialog_info[1].data_value,
				dialog_info[2].data_value);
			dev_ptr->T_CDiagDev->Periodic=(short)atoi(time);
			dev_ptr->T_CDiagDev->Frequency=1;
			dev_ptr->Asterisk = '$';
			
			save_cdd_flag = TRUE;
			break;
		}
	        else if ( rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL )
			return(rc);
	}
	return (rc);
}
/*  */
/*
 * NAME: add
 *                                                                    
 * FUNCTION: Displays Resource menu and allow devices to be
 *           added to periodic test list.
 *                                                                    
 * EXECUTION ENVIRONMENT: This function can page fault.
 *                                                                   
 * RETURNS: 
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */  

int 
add(void)
{
int 	rc;
int	idx;

/* Menu to show resources that can be tested, i,e	*/
/* added to the periodic test list.			*/

static struct	msglist device_test_list[] = {
	{ UDIAGMON_SET, UDIAGMON_ADD_DISPLAY },
	{ UDIAGMON_SET, UDIAGMON_ADD_DISPLAY_LAST },
	{ (int) NULL, (int) NULL },
};

	do 
	{
		rc = display_test_list(ADD, UDIAGMON_MENU3, 
			ASL_DIAG_LIST_CANCEL_EXIT_SC, device_test_list, &idx);

		if ( rc == DIAG_ASL_COMMIT )
		{
#ifdef	DBUG
			diag_asl_msg("%s is selected\n", 
					DTL_disp[idx]->T_CuDv->name);
#endif

			get_time(ADD, DTL_disp[idx]);
		}
		else
			break;
	} while (TRUE); 

return( rc );
}
/*  */
/*
 * NAME: delete 
 *                                                                    
 * FUNCTION: Displays Delete Resource Menu
 *	     Allow all resources to be selected.
 *	     Upon exit clear the frequency and Periodic field
 *	     in the CDiagDev structure.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function can page fault.
 *                                                                   
 * RETURNS: 
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */  

int 
delete(void)
{
int 	rc;
int	idx;

/* Menu to show resources currently in the periodic test */
/* list. Also allow user to delete entry.		 */
static	struct 	msglist device_dlt_test_list[] = {
	{ UDIAGMON_SET, UDIAGMON_DEL_DISPLAY },
	{ UDIAGMON_SET, UDIAGMON_DEL_DISPLAY_LAST },
	{ (int) NULL, (int) NULL },
};

#ifdef	DBUG
	rc=diag_asl_msg("In delete function\n");
#endif
	rc = display_test_list(DELETE, UDIAGMON_MENU6,
		   ASL_DIAG_LIST_COMMIT_SC, device_dlt_test_list, &idx);

	if ( rc == DIAG_ASL_COMMIT ) 
	{
        	for(idx=0; idx < num_All; idx++)
	        	if (Allp[idx]->Asterisk == '@') {
				save_cdd_flag = TRUE;
	        		Allp[idx]->Asterisk = ' ';
				if ( Allp[idx]->T_CDiagDev != NULL ){
	             			Allp[idx]->T_CDiagDev->Periodic = 
							DEFAULT_TESTTIME;
				}
			}	
	}
}
/*  */
/*
 * NAME: modify
 *
 * FUNCTION: modify time of day when resources can be tested.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES:
 *
 * RETURN: DIAG_ASL_COMMIT
 *	   DIAG_ASL_CANCEL
 *	   DIAG_ASL_EXIT
 *
 */

int 
modify(void)
{
int 	rc;
int	idx;

/* Menu to show resources currently in the periodic test */
/* list. Also allow user to delete entry.		 */
static	struct 	msglist device_modify_test_list[] = {
	{ UDIAGMON_SET, UDIAGMON_MODIFY_DISPLAY },
	{ UDIAGMON_SET, UDIAGMON_MODIFY_DISPLAY_LAST },
	{ (int) NULL, (int) NULL },
};

#ifdef	DBUG
	rc=diag_asl_msg("In modify function\n");
#endif

	do
	{
		rc = display_test_list(MODIFY, UDIAGMON_MENU7,
		   ASL_DIAG_LIST_CANCEL_EXIT_SC, device_modify_test_list, &idx);

		if ( rc == DIAG_ASL_COMMIT ) 
			get_time(MODIFY, DTL_disp[idx]);
		else
			break;

	} while(TRUE);

return (rc);
}

/*  */ 
/*
 * NAME: display_test_list
 *
 * FUNCTION: display all resources selected to be periodically
 *	     tested.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES:
 *
 * RETURN: DIAG_ASL_COMMIT
 *	   DIAG_ASL_CANCEL
 *	   DIAG_ASL_EXIT
 *	   -1, if
 *
 */

int 
display_test_list(int type, 			/* DISPLAY, DELETE, or ADD  */
		  int menu_number,		/* menu number		    */
		  ASL_SCREEN_CODE screen_type,	/* asl screen type	    */
		  struct msglist *menulist,	/* menu title and lastlines */
		  int *selection)		/* Selection 		    */
{

int	i;
int	idx;
int	num_disp_devices;
int	rc = DIAG_ASL_OK;
int	line = 0;
int	cnt;
char	*string;
char	*free_string;

ASL_SCR_INFO    *resinfo;
static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for enough entries */
        resinfo = (ASL_SCR_INFO *)
                calloc(1,(num_All+5)*sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                genexit(1);

        free_string = string = (char *)malloc(num_All*132);
        if ( string == (char *) NULL)
                genexit(1);

        /* set the title line in the array   */
        resinfo[line++].text = diag_cat_gets(fdes, 
			menulist[0].setid, menulist[0].msgid);

        /* read the entries from Allp, get the DName, location, and
           description and put them in the array        */
	for (idx=0, num_disp_devices=0; idx < num_All; idx++)  
	{

		if ((Allp[idx]->T_PDiagDev == NULL) || 
		   (Allp[idx]->T_PDiagAtt == NULL) || 
		    (Allp[idx]->T_CuDv->chgstatus == MISSING) ||
		    (Allp[idx]->T_CDiagDev->RtMenu == RTMENU_DDTL))
			continue;

		if ((Allp[idx]->T_PDiagAtt != NULL) &&
		     (test_val(Allp[idx]->T_PDiagAtt->value) == FALSE)) 
			continue;

		/* For all except ADD, prepare list of entries with test times.
		*/
		if ((type != ADD) && (Allp[idx]->T_CDiagDev->Periodic != 
							DEFAULT_TESTTIME))
		     	resource_desc(type, string, Allp[idx]);
	  	else           /* For ADD, insert this entry into the list. */
		if ((type == ADD) && (Allp[idx]->T_CDiagDev->Periodic == 
							DEFAULT_TESTTIME))
			resource_desc(type,string,Allp[idx]);
		else
			continue;  /* anything else not to be shown in list */

                resinfo[line].text = string;
        	resinfo[line].non_select = ASL_NO;
		string = string + strlen(string) + 1;
		line++;
		DTL_disp[num_disp_devices] = Allp[idx];
		DTL_disp[num_disp_devices]->Asterisk = ' ';
		num_disp_devices++;
        }

        /* finally add the last line */
        resinfo[line].text = diag_cat_gets(fdes, 
				menulist[1].setid, menulist[1].msgid);
        restype.max_index = line;

	cnt = num_disp_devices; 
	if (type == ADD)
	{
		if (cnt == 0)
			rc = diag_hmsg(fdes, UDIAGMON_SET, NO_ADD, NULL);
	}
	else if (type == DELETE)
	{
		if (cnt == 0)
			rc = diag_hmsg(fdes, UDIAGMON_SET, NO_DEL, NULL);
	}
	else
	{
		if (cnt == 0)
			rc = diag_hmsg(fdes, UDIAGMON_SET, NO_DISPLAY, NULL);
	}

        /* now display menu */
	while (cnt) {
		/* flag the selected devices */
        	for (idx=0, line=1; idx < num_disp_devices; 
		     idx++, line++)
			if (DTL_disp[idx]->Asterisk == '@')
       				resinfo[line].item_flag = '@';
			else
       				resinfo[line].item_flag = ' ';
		rc = diag_display(menu_number, fdes, NULL, DIAG_IO,
					screen_type,
					&restype, resinfo);

		*selection = DIAG_ITEM_SELECTED(restype);
		--(*selection);

		if ((type == DISPLAY) 	|| (rc == DIAG_ASL_EXIT) 
					|| (rc == DIAG_ASL_CANCEL) 
					|| (rc == DIAG_ASL_COMMIT) 
					|| (num_disp_devices == 0))

			break;

		DTL_disp[*selection]->Asterisk = 
			(DTL_disp[*selection]->Asterisk == '@')? ' ': '@';
	}
        free ( free_string );
        free ( resinfo );

return (rc);
}
/*   */
/*
 * NAME: resource_desc
 *                                                                    
 * FUNCTION: Build a text string describing the resource and its location.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function can page fault.
 *                                                                   
 * NOTES:
 *	This function takes as input a line type which indicates the type
 *	of display line to build.  
 *
 * RETURNS: NONE 
 */  

void 
resource_desc(	int action,
		char *string,
		diag_dev_info_t *dev_ptr)
{

char	*tmp;
char 	time[5];

	if(action == ADD)
		sprintf(string, "%-16s %-16.16s ",
			dev_ptr->T_CuDv->name,
			dev_ptr->T_CuDv->location);
	else
	{
		sprintf(time, "%04d", dev_ptr->T_CDiagDev->Periodic);
		sprintf(string, "%-16s [%c%c:%c%c]          ", 
			dev_ptr->T_CuDv->name,
			time[0],time[1],time[2],time[3]);
	}
	tmp = string + strlen(string);
	copy_text( strlen(string), tmp, dev_ptr->Text);
}

/*   */
/* NAME: copy_text
 *
 * FUNCTION: 
 * 
 * NOTES:
 *
 * RETURNS: NONE
 */

void 
copy_text(int string_length,/* current length of text already in buffer */ 
		char *buffer,	 /* buffer to copy text into	*/
		char *text)	 /* text to be copied 		*/
{
int	i;
int 	space_count;
int 	char_positions;

	/* determine if length of text string will fit on one line */
	char_positions = LINE_LENGTH - string_length;

	if ( char_positions < strlen(text))  
	{
		/* dont break the line in the middle of a word */
		if(text[char_positions] != ' ' && text[char_positions+1] != ' ')
			while ( --char_positions )
			   	if( text[char_positions] == ' ')
					break;
		if ( char_positions == 0 )
			char_positions = LINE_LENGTH - string_length;

		for ( i = 0; i < char_positions; i++, buffer++, text++ )
			*buffer = ( *text == '\n' ) ? ' ' : *text; 
		*buffer++ = '\n';
		while ( *text == ' ' )   /* remove any leading blanks */
			text++;
		space_count = string_length;
		while ( space_count-- )
			*buffer++ = ' ';
		copy_text( string_length, buffer, text);
	}
	else
		sprintf(buffer, "%s", text);

}

/*  */
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

void 
int_handler(sig)
int 	sig;
{
	genexit(0);
}

/*  */
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

void 
genexit(exitcode)
int	exitcode;
{
int 	dpid;

	/* save all changes to the Customized Diag Object Class */
	if (save_cdd_flag == TRUE)
		if (Top != (diag_dev_info_t *) -1 )  
		{
			if ( save_cdiag ( Top, num_Top ) )
				Perror( fdes, UDIAGMON_SET, 
						DATABASE_ERR, "CDiagDev");
			else
			{
				/* send SIGUSR1 to Diagd daemon about
				   CDiagDev modifications. */
#ifdef DBUG
				diag_asl_msg("Changes to CDiagDev Saved to Disk\n");
#endif
				if ((dpid=get_diagd_pid()) != 0)
					kill(dpid, SIGUSR1);
			}
		}
	odm_terminate();
	diag_asl_quit();
	catclose(fdes);
	if (udiag_lck_fd >= 0)
		unlink(udiag_lck_file);
	exit(exitcode);
}

/*  */
/*
 * NAME: get_diagd_pid
 *                                                                    
 * FUNCTION: Gets the pid of the Periodic Diag daemon.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS: Daemon's pid.
 *
 */  

int
get_diagd_pid()
{
char 	*ddata_lockdir;
char 	*diag_lockdir;
char 	lockfile[255];
char 	buf[100];
int  	lockpid;
int  	lockfd;


	diag_lockdir = DEFAULT_UTILDIR;

        /* get directory path for Diagd lock file */
        if((ddata_lockdir = (char *)getenv("DIAGDATADIR")) == NULL )
                ddata_lockdir = DIAGDATA;

	sprintf(lockfile, "%s/diagd.lck", ddata_lockdir);
	
	while((lockfd = open(lockfile, O_RDWR)) > 0)
	{
		if (read(lockfd, &lockpid, sizeof(lockpid)) == sizeof(lockpid) 
					&& kill(lockpid, 0) < 0) 
		{
			/* locking process does not exist, Create one */
	 	 	break; 
		}
	
		/* Another process is running with locking process' PID.
		   Make sure lockfile isn't from a previous boot. 
		*/
		sprintf(buf, 
			"%s -p%d | %s diagd > /dev/null 2>&1", 
							PS, lockpid, GREP);
		if (system(buf))
			break;

#ifdef	DBUG
		diag_asl_msg("Diagd daemon,pid:%d\n",lockpid);
#endif
		close(lockfd);
		return lockpid;
	}
	/* Create and run the Periodic Diag daemon; this creates 
	   lockfile, stores its pid for us to communicate with it */

	if (lockfd > 0)
		close(lockfd);

	/* Diag daemon created; first time, the daemon will anyway 
	   read the CDiagDev, so no need to get its pid and signal.
	*/
	sprintf(buf, "%s/diagd", diag_lockdir);
	system(buf);

return 0;
}

/*  */
/*
 * NAME: lock_self
 *                                                                    
 * FUNCTION: creates a lock file 'udiagmon.lck' to avoid multiple sessions.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS: NONE
 *
 */  

int
lock_self()
{
int  	lockpid;
char 	*ddata_lockdir;

	/* Disallow more than one copy of Udiagmon at a time. */
        /* get directory path for Diagd lock file */
        if((ddata_lockdir = (char *)getenv("DIAGDATADIR")) == NULL )
                ddata_lockdir = DIAGDATA;

	sprintf(udiag_lck_file, "%s/udiagmon.lck", ddata_lockdir);

	for (; (udiag_lck_fd = open(udiag_lck_file, 
				O_CREAT | O_EXCL | O_WRONLY
                                | O_NSHARE, 0600)) < 0;) 
	{
		char    buf[255];

                if (errno != EEXIST) /* lock file does not exist, then... */
                {
                        /* print msg and exit */
			diag_emsg(fdes, UDIAGMON_SET, NO_UDIAGMON_LOCK);
			genexit(1);
                }

		/* udiag_lck_file exists, make sure locking process does */

		if ((udiag_lck_fd = open(udiag_lck_file, O_RDWR)) > 0 &&
			read(udiag_lck_fd, &lockpid, sizeof(lockpid)) ==
				sizeof(lockpid) && kill(lockpid, 0) < 0) 
		{

			/* locking process does not exist */
			break;
		}
		/* Another process is running with locking process' PID.
		Make sure udiag_lck_file isn't from a previous boot. */
		sprintf(buf,
			"/bin/ps -p%d|/bin/grep udiagmon > /dev/null 2>&1",
							lockpid);
		if (lockpid == getpid() || system(buf))
			break;

		/* print msg and exit */
		udiag_lck_fd = -1;
		diag_emsg(fdes, UDIAGMON_SET, NO_UDIAGMON, lockpid);
		diag_asl_quit();
		catclose(fdes);
		exit(1);
	}
	/* create udiag_lck_file */
	lockpid = getpid();
	lseek(udiag_lck_fd, 0, SEEK_SET);
	write(udiag_lck_fd, &lockpid, sizeof(lockpid));
	close(udiag_lck_fd);
}
