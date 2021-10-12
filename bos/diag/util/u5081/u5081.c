static char sccsid[] = "@(#)11	1.10.4.6  src/bos/diag/util/u5081/u5081.c, dsau5081, bos41J, 9523C_all 6/7/95 11:00:05";
/*
 *   COMPONENT_NAME: DSAU5081
 *
 *   FUNCTIONS: BuildCommand
 *		BuildSelectionMenu
 *		ChangeDisplay
 *		FindGDHardware
 *		FindGttype
 *		PutUpMenu
 *		chk_asl_stat
 *		cleanup
 *		colorcards
 *		main
 *		software_error
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

#include	<stdio.h>
#include	<string.h>
#include	<fcntl.h>
#include	<nl_types.h>
#include	<memory.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<locale.h>

#include	"diag/diago.h"
#include	"diag/diag.h"
#include	"diag/diag_exit.h"
#include	"diag/tm_input.h"
#include	"diag/tmdefs.h"
#include	"diag/class_def.h"
#include	"diag/da.h"
#include	"diag/atu.h"
#include	"diag/dcda_msg.h"
#include	"u5081.h"
#include	"u5081_msg.h"


struct	msglist	GenericTitle[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_STANDBY },
		{ (int) NULL, (int) NULL }
	};

struct	msglist	NoDisplaysFound[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_NDF },
		{ (int) NULL, (int) NULL }
	};

struct	msglist	SoftwareError[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_SWE },
		{ (int) NULL, (int) NULL }
	};

struct	msglist	ColorPatterns[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_RED },
		{ UDISP_MSGS, UDISP_GREEN },
		{ UDISP_MSGS, UDISP_BLUE },
		{ UDISP_MSGS, UDISP_BLACK },
		{ UDISP_MSGS, UDISP_WHITE },
		{ UDISP_MSGS, UDISP_9x9GRID },
		{ UDISP_MSGS, UDISP_9x11GRID },
		{ UDISP_MSGS, UDISP_COLORBAR },
		{ UDISP_MSGS, UDISP_PATTERNS },
		{ (int) NULL, (int) NULL }
	};


struct	msglist	SGAColorPatterns[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_RED },
		{ UDISP_MSGS, UDISP_GREEN },
		{ UDISP_MSGS, UDISP_BLUE },
		{ UDISP_MSGS, UDISP_BLACK },
		{ UDISP_MSGS, UDISP_WHITE },
		{ UDISP_MSGS, UDISP_9x9GRID },
		{ UDISP_MSGS, UDISP_9x11GRID },
		{ UDISP_MSGS, UDISP_COLORBAR },
		{ UDISP_MSGS, UDISP_BOX },
		{ UDISP_MSGS, UDISP_FOCUS },
		{ UDISP_MSGS, UDISP_PATTERNS },
		{ (int) NULL, (int) NULL }
	};

struct	msglist	MonoPatterns[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_BLACK },
		{ UDISP_MSGS, UDISP_WHITE },
		{ UDISP_MSGS, UDISP_9x9GRID },
		{ UDISP_MSGS, UDISP_9x11GRID },
		{ UDISP_MSGS, UDISP_PATTERNS },
		{ (int) NULL, (int) NULL }
	};

struct	msglist	TestDescription[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_TEST },
		{ (int) NULL, (int) NULL }
	};

struct	msglist	x_warn[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_X_WARN },
		{ (int) NULL, (int) NULL }
	};

struct	msglist	NoSA[]=
	{
		{ UDISP_MSGS, UDISP_GT },
		{ UDISP_MSGS, UDISP_NO_SA },
		{ (int) NULL, (int) NULL }
	};

struct	teststruct
{
	ulong	pdid;
	struct	CuDv	*cudv;
	struct	PDiagAtt	*pdiagatt;
};

struct	teststruct	TestList[4];
struct	teststruct	*DisplayToTest;

static	ASL_SCR_TYPE	menutypes= DM_TYPE_DEFAULTS;
static	ASL_SCR_INFO	*selectionlist; 
 
char	path[256];
char	rcs[10];
char	consoleflag[10];
char	*com[6];
int	tuhandlesints=FALSE;

int	init = FALSE;
int	DisplaysFound;
nl_catd	catd = CATD_ERR;		/* Pointer to the catalog file   */
ulong	PhysDevID;
struct CuAt	*getattr();
extern  nl_catd diag_catopen(char *, int);
extern	char	*diag_cat_gets();
extern	nl_catd	diag_device_catopen();
extern	char 	*diag_device_gets();

/*  */
/*
 * NAME: main (u5081)
 *                                                                    
 * FUNCTION: This function displays the list of display adapters in the 
 *           system for testing and then passes control to the application
 *	     program of the selected display to display test patterns.    
 *                                                                    
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) This function modifies the following global structures
 *			and variables:
 *		Initializes the catd variable.
 *
 * RETURNS: 
 *	0 if successful and 1 if fails.     	
 */  

main(argc,argv) 
int	argc;
char	*argv[];
{

	void	cleanup();
	int	OpenDevice();
	void	BuildSelectionMenu();
	int	PutUpMenu();
	char	*FindGttype();

	int	rc;
	int	src = 0;
	int	wrc;
	int	child_status;
	int	dev_speed=0;  /* refresh rate the adapter is set to for */
                          /* adapters that support multiple refresh rates */
	char	*tmp;
	char	*newvalue=NULL;

	setlocale(LC_ALL,"");
	catd = diag_catopen( MF_U5081,0 ) ;

	(void)chk_asl_stat( diag_asl_init(NULL) );
	init = TRUE;

	(void)PutUpMenu( GENERICTITLE );

	if ( init_dgodm() == ERR_FILE_OPEN )
	{
		software_error();
	}

	/* check to see if X is running. If so, cleanup */
	if(strcmp((char * )getenv("X_DIAG"),"1") == 0)
	{
		(void)PutUpMenu( X_WARNING );
		cleanup();
		exit(0);
	}

	if ((DisplaysFound = FindGDHardware()) == -1)
	{
		software_error();
	}

	if ( DisplaysFound == 0 )
	{
		(void)PutUpMenu( NODISPLAYSFOUND );
		cleanup();
		exit(0);
	}

	while (TRUE)
	{
		BuildSelectionMenu(&TestList[0],DisplaysFound,&dev_speed);

		rc = PutUpMenu( DISPLAYSELECTION );

		if ( rc < 0 || rc > DisplaysFound )
			break;

		DisplayToTest = &TestList[rc-1];

		while ( TRUE )
		{

	/* The display adapter application puts up all the screens */
			if ((strcmp(DisplayToTest->pdiagatt->attribute,"NoScreen")) == 0)
			{
				/* For multiple adapters of the same type */
					if ((strcmp(DisplayToTest->pdiagatt->value, "*")) == 0)
						newvalue = FindGttype(DisplayToTest->cudv->name, &dev_speed);
					colorcards(DisplayToTest, dev_speed, newvalue, src);
					/* since the utility is putting up its own
					   menu's, just break out of this while */
					break;
			}

	/* For regular color cards */
			else if ((strcmp(DisplayToTest->pdiagatt->attribute,"Color")) == 0)
			{
					src = PutUpMenu( COLORPATTERNS );
					if ( src < 0 )
					{
						rc = src;
						break;
					}
					if((rc=PutUpMenu( TESTDESCRIPTION )) !=0)
					{
						if ( rc == -2 )
							rc = 0;
						break;
					}
			/* For multiple adapters of the same type */
					if ((strcmp(DisplayToTest->pdiagatt->value, "*")) == 0) 
						newvalue = FindGttype(DisplayToTest->cudv->name, &dev_speed);
					rc = colorcards(DisplayToTest, dev_speed, newvalue, src);
			}
	
	/* Color cards with 50 mm. box and Focus test patterns. */
			else if ((strcmp(DisplayToTest->pdiagatt->attribute,"Focus")) == 0)
			{
					src = PutUpMenu( SGACOLORPATTERNS );

					if ( src < 0)
					{
						rc = src;
						break;
					}
					if((rc=PutUpMenu( TESTDESCRIPTION )) !=0)
					{
						if ( rc == -2 )
							rc = 0;
						break;
					}
			/* For multiple adapters of the same type. */
					if ((strcmp(DisplayToTest->pdiagatt->value, "*")) == 0) 
						newvalue = FindGttype(DisplayToTest->cudv->name, &dev_speed);
					rc = colorcards(DisplayToTest, dev_speed, newvalue, src);
			}

	/* Mono Color Cards */
			else if ((strcmp(DisplayToTest->pdiagatt->attribute,"Mono")) == 0)
			{

					src = PutUpMenu( MONOPATTERNS );
					if ( src < 0 )
					{
						rc = src;
						break;
					}
					rc=PutUpMenu( TESTDESCRIPTION );
					if (rc != 0 )
					{
						if ( rc == -2 )
							rc = 0;
						break;
					}
		/* For multiple adapters of the same type */
					if ((strcmp(DisplayToTest->pdiagatt->value, "*")) == 0)
						newvalue = FindGttype(DisplayToTest->cudv->name, &dev_speed);
					rc = colorcards(DisplayToTest, dev_speed, newvalue,src);
				}
			else
				software_error();	


			if ( rc < 0 )
				break;

		}  /* while end */
		if(rc == -3)
			break;

		if ( rc >= EXIT || rc == -1 )
			software_error();

	}  /* while end */

	cleanup();

	exit(0);

}  /* main end */

/*
 * NAME: BuildCommand
 *                                                                    
 * FUNCTION: This function builds the command arguments that will be 
 *		passed with the diag_asl_execute command.
 *                                                                    
 * RETURNS: To the calling routine;
 *		0 if successful.
 *		-1 if call fails.
 */  
BuildCommand(commandname,tu,speed)
char	commandname[256];
int	tu;
int	speed;
{
	char	tmp[5];
	char * tmp_ptr;

	memset(path,0x00,sizeof(path));
	if(commandname[0] != '/')
		sprintf(path,"%s/bin/",getenv("DIAGNOSTICS"));

	/* find the occurrence of the first space. This assumes there is just
	 * one space between the command and the argument */

	tmp_ptr = strchr(commandname,' ');
	/* if there is a flag to pass, then the tu is not passed.  But a 
	 * flag is. Also check to see if there is parameter.  If no
	 parameter, assumet name is no longer than 9 bytes.*/
	if(tmp_ptr == (char *)NULL)
	{
		sprintf(rcs,"%d",tu);
		strncat(path,commandname,10);
	}
	else	
	{
		strncat(path,commandname,tmp_ptr - commandname);
		sprintf(rcs,"%s",tmp_ptr);
	}


	com[0] = commandname;
	/* com[1] is filled in the buildselection menu function */

	com[2] = rcs;
	sprintf(consoleflag,"%d",tuhandlesints);
	com[3] = consoleflag;
	sprintf(tmp,"%d",speed);
	com[4] = tmp;
	com[5] = NULL;
}

/*
 * NAME: FindGDHardware
 *                                                                    
 * FUNCTION: This function 
 *	        gets a list of all the available display adapters on the 
 *		system upto a maximum of four and puts the information 
 *		of the adapters in the array TestList. 	
 *                                                                   
 * (NOTES:) The maximum number of display adapters in the system is 4.
 *
 * (RECOVERY OPERATION:) 
 *		Software errors, such as failures to open, are handled by
 *		displaying error message, clean up  and returning to the 
 *		calling routine.
 *
 * (DATA STRUCTURES:) This function modifies the following global structures
 *			and variables:
 *			initializes the array TestList
 * RETURNS: To the calling routine;
 *		the number of available displays in the system.
 */  

int
FindGDHardware()
{
	struct	CuAt	*cuat;
	struct	PDiagAtt	*pdiagatt1;
	struct	CuDv	*cudv1;
	struct	listinfo	c_info;
	char	crit[100];
	int	i, j;
	int	how_many;
	int	numdisplays;
	int 	actdisplays = 0;

	/* Get a list of all predefined display adapters */	
	pdiagatt1 = get_PDiagAtt_list(PDiagAtt_CLASS, "DApp like u5081", &c_info, 10, 1);
	if (pdiagatt1 == (struct PDiagAtt *) -1)
		software_error();
	numdisplays = c_info.num;
	
	/* Get a list of predefined displays actually in system */
	for(i=0; i<numdisplays; i++)
	{
		sprintf(crit, "PdDvLn like %s/%s/%s AND status = %d", pdiagatt1->DClass, pdiagatt1->DSClass, pdiagatt1->DType, AVAILABLE);
		cudv1 = get_CuDv_list(CuDv_CLASS, crit, &c_info, 4,2);
		if (cudv1 == (struct CuDv *) -1)
		{
			software_error();	
		}
		if (cudv1 != NULL)
		{
			for(j=0; j<4 && j<c_info.num && actdisplays < 4; j++)
			{
/*				cuat = getattr(cudv1->name, "display_id", FALSE, &how_many);
				if (cuat == (struct CuAt *) -1)
				{
					software_error();	
				}

				PhysDevID = (ulong)strtol(cuat->value, (char **)NULL,16); 
				if(cuat != NULL && PhysDevID != 0) */
				{
					TestList[actdisplays].cudv = cudv1;
					TestList[actdisplays].pdiagatt = pdiagatt1;
					/* TestList[actdisplays].pdid = PhysDevID; */
					actdisplays++;
				}
				cudv1++;	
			} /* for */
		} /* if cudv1 != NULL */
		pdiagatt1++;
	} /* for end */
	return(actdisplays);
} /* FindGDHardware end */


	


/*  */
/*
 * NAME: FindGttype
 *
 * FUNCTION:
 *	This function decides which one of the Power Gtx graphics adapters
 * 	is available. It also finds out the refresh rate of the adapter.
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:) 
 *	Software errors are handled by calling the routine software_error to
 *	put up a message and clean up.
 * (DATA STRUCTURES: )
 *
 * RETURNS:
 *	Returns the name of the application program for the adapter.
 *
 */
char *FindGttype(dname,speed)
char	*dname;
int	*speed;
{
	struct	CuAt	*cuat;
	struct	CuAt	*cuat1;
	struct	PDiagAtt	*pdiagatt1;
	struct	listinfo	c_info;
	int	how_many;
	char	nvalue[16];
	char	crit[100];

        /*      Determine which type of graphics display adapter we are    */
        /*      testing with.                                              */

	if((cuat = getattr(dname, "subtype",FALSE, &how_many))
			== (struct  CuAt *) 0)
		software_error();
	sprintf(crit, "attribute like %s", cuat->value);	
	pdiagatt1 = get_PDiagAtt_list(PDiagAtt_CLASS, crit, &c_info,1,1);
	if ((pdiagatt1 == (struct PDiagAtt *) -1) || (pdiagatt1 == (struct PDiagAtt *) 0))
		software_error();
	strncpy(nvalue, pdiagatt1->value, 16);
	/* For adapters with multiple refresh rates */
	if (strcmp(pdiagatt1->rep, "Speed") == 0)
	{
		cuat1 = getattr(dname, "refresh_rate", FALSE, &how_many);
		if( cuat1 == (struct CuAt *) 0)
			software_error();
		*speed = atoi(cuat1->value);
	} /* if strcmp */
	else
		*speed = (int)NULL;
	odm_free_list(PDiagAtt_CLASS, &c_info);		

	return(nvalue);
}/* FindGttype end */

/*  */
/*
 * NAME: BuildSelectionMenu
 *                                                                    
 * FUNCTION: This function builds the menu to display the list of
 * 	display adpaters in the system and their locations. 
 *                                                                    
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *	Software errors are handled by calling the routine software_error to
 *	put up a message and clean up.
 *
 * (DATA STRUCTURES:) This function modifies the following global structures
 *			and variables:
 *	initializes the pointer variable selectionlist
 *
 * RETURNS: NONE
 *
 */  

void	BuildSelectionMenu(displaylist, numofdisplays, gt_sp)
struct	teststruct	*displaylist;
int	numofdisplays;
int	*gt_sp;
{
	char	msgstr[512];
	char	catlog[16];
	char	crit[100];
	char	*tmpbuf;
	char	*tmp;
	struct	teststruct	*disp_ptr;
	struct	CuAt		*cuat;
	struct	PdAt		*pdat;
	struct	listinfo	c_info, c_infop;
	int	mrc;
	int	i=0;
	short	setnum;
	short	msgnum;
	nl_catd	catdev;
	
	tmpbuf = (char *)malloc(numofdisplays*512);
	if( tmpbuf == (char *)NULL) 
		software_error();
	selectionlist = (ASL_SCR_INFO *)calloc(1,(numofdisplays+2)*sizeof(ASL_SCR_INFO));
	if( selectionlist == (ASL_SCR_INFO *)NULL)
		software_error();
	disp_ptr = displaylist; 
	/* Display the title */
	selectionlist[i].text = diag_cat_gets(catd, UDISP_MSGS, UDISP_GT);
	/* Build the adapter messages */
	for (i=1; i<numofdisplays+1 ; i++) 
	{
		strcpy(catlog, disp_ptr->cudv->PdDvLn->catalog);
		setnum =  disp_ptr->cudv->PdDvLn->setno;
		msgnum =  disp_ptr->cudv->PdDvLn->msgno;
		if (msgnum == 0)
		{ /* Multiple adapters of the same type */
			sprintf(crit, "name = %s AND type = T", disp_ptr->cudv->name);
			cuat = (struct CuAt *)get_CuAt_list(CuAt_CLASS, crit, &c_info, 1,1);
			if(cuat == (struct CuAt *) -1)
				software_error();
			if(c_info.num == 0)
			{
				sprintf(crit, "uniquetype = %s AND type = T", disp_ptr->cudv->PdDvLn->uniquetype);
				pdat = (struct PdAt *)get_PdAt_list(PdAt_CLASS, crit, &c_infop, 1,1);
				if(pdat == (struct PdAt *) -1)
					software_error();
				else if (c_infop.num == 1)
					msgnum = atoi(pdat->deflt);
				else
					software_error();
				odm_free_list(PdAt_CLASS, &c_infop);
			} /* if c_info.num == 0 in CuAt structure */
			else
				msgnum = atoi(cuat->value);
			odm_free_list(CuAt_CLASS, &c_info);			
		} /* if msgnum == 0 */
		catdev = diag_device_catopen(catlog, 0);	
		tmp = diag_device_gets(catdev, setnum, msgnum,"n/a"); 
		selectionlist[i].text = diag_cat_gets(catd, UDISP_MSGS, UDISP_DESCR);
		sprintf(msgstr, selectionlist[i].text, tmp, disp_ptr->cudv->location);
		strcpy(tmpbuf, msgstr);
		selectionlist[i].text = tmpbuf;
		tmpbuf = tmpbuf+strlen(tmpbuf)+1;
		selectionlist[i].non_select = ASL_NO;
		if((int)catdev > 0)
			catclose(catdev);
		++disp_ptr;
	} 
	selectionlist[i].text = diag_cat_gets(catd, UDISP_MSGS, UDISP_DISPLAYS);
	selectionlist[i].non_select = ASL_YES;

}  /* BuildSelectionMenu end */


/*
 * NAME: cleanup
 *                                                                    
 * FUNCTION: This is the exit point for the diagnostic application.
 *		Any devices that were opened during the execution of the
 *		service aid are closed here.  If it was necessary
 *		to configure the device from within the diagnostic
 *		application then that device is unconfigured here.
 *                                                                    
 * (NOTES:) This function:
 *		Closes the message catalog if successfully opened.
 *		Closes the odm via the term_dgodm() call.
 *		Issues DIAG_ASL_QUIT if message catalog was opened.
 *
 * (RECOVERY OPERATION:) Software error recovery is handled by setting
 *		error flags and returning these to the diagnostic
 *		utility controller.
 *
 * (DATA STRUCTURES:) No global data structures/variables effected.
 *
 * RETURNS: NONE
 */  

void	cleanup()
 
{
  	
	(void)term_dgodm();
	 
	if ( catd != CATD_ERR )
		catclose( catd );

	if ( init == TRUE )
	{
		(void)diag_asl_quit( NULL );
	}

}  /* cleanup end */


/*
 * NAME: PutUpMenu
 *                                                                    
 * FUNCTION: This function:
 *	Displays one of eight possible menus to the display screen.  
 *                                                                    
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) Software error recovery is handled by the    
 *		chk_asl_stat function.
 *		
 *
 * (DATA STRUCTURES:) Does not effect global data structures/variables.
 *
 * RETURNS: This function returns to the caller if the display of the
 *	menu occurs without error.  If an error return occurs then the 
 *	exit is handled by the chk_asl_stat function.
 */  

int	PutUpMenu(msgtype)
int	msgtype;
{
	int rc=0;

	switch( msgtype )
	{
		case GENERICTITLE:
			(void)chk_asl_stat( diag_display( 0x802080,
					catd,GenericTitle,DIAG_IO,
					ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL ) );
			break;

		case NODISPLAYSFOUND:
			rc=chk_asl_stat( diag_display( 0x802085,
					catd,NoDisplaysFound,DIAG_IO,
					ASL_DIAG_KEYS_ENTER_SC, NULL,
					NULL ) );
			break;

		case TESTDESCRIPTION:
			menutypes.cur_index=1;
			rc=chk_asl_stat( diag_display( 0x802084,
					catd,TestDescription,DIAG_IO,
					ASL_DIAG_KEYS_ENTER_SC, NULL,
					NULL ) );
			break;

		case COLORPATTERNS:
			menutypes.cur_index=1;
			menutypes.cur_win_index=0;
			rc=chk_asl_stat(diag_display(0x802082,
					catd,ColorPatterns,DIAG_IO,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					&menutypes, NULL));
			if ( rc == 0 )
				rc =  menutypes.cur_index ;
			break;

		case MONOPATTERNS:
			menutypes.cur_index=1;
			menutypes.cur_win_index=0;
			rc=chk_asl_stat(diag_display(0x802083,
					catd,MonoPatterns,DIAG_IO,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					&menutypes, NULL));
			if ( rc == 0 )
				rc =  menutypes.cur_index ;
			break;

		case SGACOLORPATTERNS:
			menutypes.cur_index=1;
			menutypes.cur_win_index=0;
			rc=chk_asl_stat(diag_display(0x802087,
					catd,SGAColorPatterns,DIAG_IO,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					&menutypes, NULL));
			if ( rc == 0 )
				rc =  menutypes.cur_index ;
			break;

		case DISPLAYSELECTION:
			menutypes.cur_win_index=0;
			menutypes.cur_index=1;
			menutypes.max_index = (DisplaysFound + 1);

			rc=chk_asl_stat(diag_display(0x802081,catd, NULL,
				DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
				&menutypes, selectionlist));
			if ( rc == 0 )
				rc =  menutypes.cur_index ;
			break;

		case SOFTWARE_ERR:
			(void)chk_asl_stat( diag_display( 0x802086,
					catd,SoftwareError,DIAG_IO,
					ASL_DIAG_KEYS_ENTER_SC, NULL,
					NULL ) );
			break;

		case X_WARNING:
			(void)chk_asl_stat( diag_display( 0x802088,
					catd,x_warn,DIAG_IO,
					ASL_DIAG_KEYS_ENTER_SC, NULL,
					NULL ) );
			break;

		case NO_SA:
			(void)chk_asl_stat( diag_display( 0x802090,
					catd,NoSA,DIAG_IO,
					ASL_DIAG_KEYS_ENTER_SC, NULL,
					NULL ) );
			break;

		default:
			break;
	}
	return(rc);
}



/*
 * NAME: chk_asl_stat
 *                                                                    
 * FUNCTION: This function:
 *	Handles the return code from an asl or diag_asl procedure call.
 *                                                                    
 * (NOTES:) 
 *	This function acts on the return code from an asl or diag_asl
 *	procedure call.  It either returns to the calling routine or
 *	sets error flags and calls the cleanup() routine.
 *
 * (RECOVERY OPERATION:) Software error recovery is handled by setting
 *		error flags and returning these to the diagnostic
 *		utility controller.
 *
 * (DATA STRUCTURES:) Global DA_ERROR flag can be set in this function.
 *
 * RETURNS: This function returns to the calling procedure only if the 
 *		asl_okay flag or an undefined flag are set in the error 
 *		return code passed as an input parameter.
 */  

chk_asl_stat(returned_code)
long	returned_code;
{
	void	cleanup();

	switch ( returned_code )
	{
		case DIAG_ASL_FAIL:
			software_error();
		case DIAG_ASL_CANCEL:	
		{
			return(-2);
		}
		case DIAG_ASL_EXIT:
		{
			return(-3);
		}
		case DIAG_ASL_OK:
		default:
			return(0);
	}
}


/*
 * NAME: colorcards
 *                                                                    
 * FUNCTION: This function displays pattern selections of the applications 
 *	     that would run on a graphics card, after the user selects a 
 *	     pattern, this function would execute the application for that card.
 *                                                                    
 * (RECOVERY OPERATION:) Software error recovery is handled by 
 *			calling the software_error.
 *
 * RETURNS: This function returns to the calling procedure if no error 
 *	and calls software_error if error occurs.
 */  

colorcards(displaylist, speed, value, src)
struct	teststruct	*displaylist;
int	speed;
char *value; 
int src;
{

	int	rc=0;
	int	wrc=0;
	int	i=0;
	int	child_status;
	struct	teststruct	*disp_ptr;
	char	utilname[256]; 
	char 	*tmp;

	/* Check to see if there is a SA for this adapter */
	if (strcmp(DisplayToTest->pdiagatt->value, "NO_SA") == 0)
	{
		rc = PutUpMenu( NO_SA );
		return(rc);
	}


	disp_ptr = displaylist;
	for(i=0;i<1;i++) /* this is a dummy loop to branch out on an error */
	{
		/* For multiple adapters of same type */
		if((strcmp(disp_ptr->pdiagatt->value, "*") == 0))
			strcpy(utilname, value);
		else
			strcpy(utilname, disp_ptr->pdiagatt->value);

		BuildCommand(utilname,src,speed);

		/* Put device name as first command argument */
		tmp = (char *)calloc((strlen(disp_ptr->pdiagatt->DType)+2), sizeof(char *));
		sprintf(tmp, "%s", disp_ptr->cudv->name);
		com[1] = tmp;

		diag_asl_clear_screen(); 
		wrc=diag_asl_execute (path, com, &child_status);
		if (wrc == -1 || child_status & 0xFF)
			software_error();
		if ((child_status>>8) == -1)
			software_error();
	}
	return(rc);
}

/*
 * NAME: software_error
 *                                                                    
 * FUNCTION: This function:
 *	     Displays Software Error message and exit the program.
 *                                                                    
 * RETURNS: Exits with a code '1'.
 */  

software_error()
{
	(void)PutUpMenu(SOFTWARE_ERR);
	cleanup();
	exit(1);
}
