static char sccsid[] = "@(#)95	1.7  src/bos/diag/da/corvette/dacorv.c, dascsi, bos41J, 9511A_all 3/9/95 13:17:20";
/*
 *   COMPONENT_NAME: DASCSI
 *
 *   FUNCTIONS: clean_up
 *		clean_up_task
 *		diag_io_menu
 *		do_task
 *		ela
 *		get_adp_type
 *		initialize_cons_odm
 *		int_handler
 *		main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>
#include	<fcntl.h>
#include	<memory.h>
#include	<limits.h>
#include	<nl_types.h>
#include	<sys/types.h>
#include	<signal.h>
#include	<sys/termio.h>
#include	<sys/errno.h>
#include        <sys/cfgodm.h>
#include        <sys/devinfo.h>
#include	<sys/buf.h>
#include	<sys/errids.h>
#include	<locale.h>

#include	"diag/diag.h"
#include	"diag/diago.h"
#include	"diag/tm_input.h"
#include	"diag/tmdefs.h"
#include	"diag/da.h"
#include	"diag/da_rc.h"
#include	"diag/diag_exit.h"
#include	"dacorv_msg.h"
#include	"common.h"
#include	"dacorv.h"
#include	"Adapters.h"
#include	"corv_atu.h"
#include	"debug.h"

struct	task	*get_tasks();
int	odm_flg=0;	/* odm flag is set when odm initialized		*/
int	asl_flg=0;	/* asl flag is set when asl initialized		*/
int	clean_tu=0;	/* flag indicator to execute clean up tus	*/
struct	tm_input	tm_input;/* DA input environment data		*/
nl_catd	catd=CATD_ERR;	/* pointer to the catalog file			*/ 
extern	int	exectu();/* test unit exectu				*/
void    int_handler(int);/* interrupt handler routine 			*/


/*
 * NAME: main()
 *
 * FUNCTION: Main driver for SCSI Diagnostic application and the download
 *	     microcode for this adapter. This function will call several
 *	     functions depends on the environment.
 *	     To run the diagnostics application the name of this program
 *	     must be dacorv otherwise this program will attempt to download
 *	     microcode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS:
 */

main(argc,argv)
int	argc;	/* number of argument passed to the DA			*/
char	**argv;	/* argument passed to the application			*/
{
	struct  msglist goals[]=	/* Menu goal msglist		*/
	{
		{ 0,		0 		},
        };
	struct  task *ar_tasks;	/* pointer to task structure		*/
	char	task_string[1024];
	char	*erd_type_crit[]=/* Error data type priority search crit.*/
			{ "_ISR",""};
	
	int	num_task=0;	/* Number of tasks found		*/
	int	adapter;	/* adapter type flag			*/
	int	rc;
	int	i=0;
	int	cur_index;	/* index to task array.			*/
	int	old_index;	/* previous index to task array.	*/
	int	erp_return;	/* flag to break out of a loop		*/
	int	task_ret=0;	/* task execution return code. 		*/
	int	offset=0;
	short	application=MCODE;
	int	isr;
	char	*fname=NULL;	/* name of the application		*/
	char	*dadir=NULL;	/* diagnostics directory name		*/
	struct	sigaction	invec;	/* interrupt handler structure	*/

	setlocale(LC_ALL,"");

	/* initialize interrupt handler structure       */
	invec.sa_handler =  int_handler;
	sigaction(SIGINT, &invec, (struct sigaction *) NULL );
	sigaction( SIGTERM, &invec, (struct sigaction *) NULL );
	sigaction( SIGQUIT, &invec, (struct sigaction *) NULL );
	sigaction( SIGKILL, &invec, (struct sigaction *) NULL );

	(void)diag_db(START);

	/* determine DA Directory Path from environment variable */
        if ( (dadir = (char *)getenv("DADIR")) == NULL )
                dadir = DEFAULT_DADIR;

	fname= calloc(strlen(dadir)+strlen("/dacorv")+2,sizeof(char));
	(void)sprintf(fname,"%s%s",dadir,"/dacorv");
	(void)diag_db(STRING,2,"argv[0] ",argv[0],"fname",fname);

	if (!strcmp(argv[0], fname))
		application=DA_CORV;
	else
		if(argc<3 )
		{
			/* this condition only happen the users select	*/
			/* download microcode and nor args.		*/
			(void)diag_db(DEC,1,"number of argument",argc);
			DA_SETRC_ERROR(DA_ERROR_OTHER);
                        DA_EXIT();
		}

	/* initialize the odm, set exit defaults and call diag asl init	*/

	initialize_cons_odm(application,argv[1]);

	/* get the adapter type such as DE, SE or Integrated		*/
	adapter=get_adp_type();
	switch(adapter)
	{
		case DE:
			offset=0x100;
			break;
		case INTEGRATED:
			offset=0x200;
			break;
		case UNKOWN_ADAPT:
			offset=0x300;
			break;
		case TURBO_DE:
			offset=0x400;
			break;
		default:
			offset=0;
			break;
	}

	/* update FRUs offset to point to Appropriate FRUs.	*/
	for(i=0;i<(sizeof(frub)/sizeof(struct fru_bucket));i++)
		frub[i].rcode+=offset;
 

	if (!strcmp(argv[0], fname))
	{
		/* DA got called 	*/
		if(tm_input.dmode == DMODE_PD && strlen(tm_input.child1) )
			(void)get_task_data("VSCSI_TASK",&task_string);
		else
			(void)get_task_data("DA_TASK",&task_string);
	}
	else
		/* Download microcode got called */
		rc=get_task_data("MCODE_TASK",&task_string);
 
	/* assign the task sequence into a matrix of tasks */

 
	(void)diag_db(STRING,1,"get_tasks got task_string",task_string);

	ar_tasks=get_tasks(task_string,&num_task);
 
	(void)diag_db(DEC,1,"number of tasks",num_task);

	/* Sequence must start from the begining */
 
	free(dadir);
	if(argc>=3)
	{
		dadir= calloc(strlen(argv[2])+2,sizeof(char));
		sprintf(dadir,"%s",argv[2]);
	}
	cur_index=0;
	while ( cur_index < num_task)
	{
		/* Check the task environment and compare it to tm_input */
		if ( env_task(ar_tasks[cur_index].task_code) == TRUE)
		{
			isr=0xffff;
			(void)diag_db(DEC,1,"do_task is called index",cur_index);
			
			task_ret=do_task(ar_tasks[cur_index],&isr,dadir);
			(void)diag_db(DEC,1,"do_task  returned",task_ret);

			old_index=cur_index; /* save the current index	*/

			erp_return=do_erd(ar_tasks,&cur_index,isr,
				&erd_type_crit[0],&frub[0],
				&goals[0],2);
			if ( erp_return == EXIT_NOW )
				break;
			erp_return=do_erd(ar_tasks,&cur_index,task_ret,
				&erd_type_crit[0],&frub[0],
				&goals[0],2);
			if ( erp_return == EXIT_NOW )
				break;

			/* set clean_tu flag only when running a TU	*/
			if(ar_tasks[cur_index].task_code[0] == '4' ||
			    ar_tasks[cur_index].task_code[0] == '8' )
				clean_tu =TRUE;

			/* Current index == old_index means the TU did 	*/
			/* fail or if so did not have a jump instruction*/
			/* in the ERD.					*/

			if(old_index==cur_index)
				cur_index++;
		}
		else
			/* Task does not run in this mode */
			cur_index++; /* increment the index. */

	}
	if(clean_tu)
		clean_up_task();
	clean_up();

	DA_EXIT();
}
 
/*
 * NAME: initialize_cons_odm()
 *
 * FUNCTION: Designed to initialize the odm, get TM input and to
 *           initialize ASL.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */


initialize_cons_odm(flag,name)
short	flag;
char	*name;
{
	DA_SETRC_STATUS(DA_STATUS_GOOD);
	DA_SETRC_USER(DA_USER_NOKEY);
	DA_SETRC_ERROR(DA_ERROR_NONE);
	DA_SETRC_TESTS( DA_TEST_FULL );
	DA_SETRC_MORE(DA_MORE_NOCONT);

	init_dgodm();
	odm_flg=TRUE;
	if(flag == DA_CORV)
	{
		if(getdainput(&tm_input))
		{
			clean_up();
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			DA_EXIT();
		}
	}
	else
	{
		/* set a fake tm_input for the microcode download */
		(void)memset(&tm_input,0,sizeof(struct tm_input));
		strcpy(tm_input.dname,name);
		diag_db(STRING,1,"tm_input.dname",tm_input.dname);
	}
		
	if ( tm_input.console == CONSOLE_TRUE )
	{
		if(tm_input.loopmode ==  LOOPMODE_INLM)
			diag_asl_init("DEFAULT") ;
		else
			diag_asl_init("NO_TYPE_AHEAD") ;
		asl_flg=TRUE;
	}
	catd = (nl_catd)diag_catopen( MF_DACORV,0) ;
}
 
/*
 * NAME: do_task()
 *
 * FUNCTION: This function calls out several functions, it depends on the
 *	     tu_task string. The task string will have 4 characters string
 *	     as follow:
 *	     
 *               3 0 0 1
 *               | | | |
 *               | | |_|_____ Screen subtask action code or tu number.
 *               | |
 *               | |_________ Mode type
 *               |             0 - execute in all modes except when no console
 *               |                 DA should check for not to use the display.
 *               |             1 - execute in IPL pretest.
 *               |             2 - execute in StandAlone.
 *               |             3 - execute in Advanced mode.
 *               |             4 - execute in EnterLoop mode.
 *               |             5 - execute in InLoop and Advanced mode.
 *               |             6 - execute in ExitLoop mode.
 *               |             7 - execute in Customer mode.
 *               |             8 - execute in StandAlone and Advanced mode.
 *               |             9 - execute in ELA mode.
 *               |             A - execute in PD mode.
 *               |
 *               |___________ Task function code.( Range 0 to Z)
 *
 *				3 - Screen Menu calls.
 *				4 - Test Unit calls.
 *				8 - Test Unit calls.
 *				7 - ELA function call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: status of the excuted task.
 */

do_task(tu_task,isr,mcode_path)
struct	task	tu_task;
int	*isr;
char	*mcode_path;
{
	int	tu=0;
	int	return_code=0;
	CORV_TUTYPE	tucb;

	tu=atoi(&tu_task.task_code[2]);
	
	switch ( tu_task.task_code[0] )	/* first character in the task */
	{
		case '3':
			/* screen task					*/
			return_code= diag_io_menu(tu);
			break;
		case '8':	/* microcode task code			*/
		case '4':	/* diagnostics task code		*/
			/* test unit task				*/
			(void)memset(&tucb,0,sizeof(CORV_TUTYPE));
			tucb.ldev_name=(char *)calloc(sizeof(tm_input.dname)
				+1,sizeof(char));
			tucb.io_buff=(char *)calloc(strlen(mcode_path) + 3,
				sizeof(char));
			sprintf(tucb.ldev_name,"%s",tm_input.dname);
			if(tu_task.task_code[0]=='8')
				sprintf(tucb.io_buff,"%s",mcode_path);
			tucb.tu=tu;
			(void)diag_db(TU_CALL,&tucb);
			return_code=exectu(&tucb);
			(void)diag_db(TU_RET_DATA,return_code,&tucb);
			tucb.isr=tucb.isr>>4;
			if(tucb.isr == 3)
			{
				(void)diag_db(DEC,1,"tucb.io_buff ",tucb.io_buff);
				*isr=(((int)tucb.io_buff[0])<<4);
				(void)diag_db(DEC,1,"tucb.io_buff <<4",isr);
				*isr |= tucb.isr;
				(void)diag_db(DEC,1,"PTC +isr ",isr);
			}
			else
				*isr=tucb.isr;

			break;
		case '7':
			/* error log analysis task		*/
			ela();
			break;
		default:
			break;
	}

	return ( return_code);
}
 
/*
 * NAME: diag_io_menu()
 *
 * FUNCTION: Designed to interface with the users in such display 
 *	     message or read input from the keyboard.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: status of diag asl functions.
 */

diag_io_menu(menu_set)
int	menu_set;
{
	int	menu_num;
	int	rc=0;

	if(tm_input.console == CONSOLE_FALSE)
		return(0);

	switch(menu_set)
	{
	case 1:

		menu_num=0x890100;
		if(tm_input.loopmode == LOOPMODE_INLM)
		{
			/* display loop message				*/
			rc=diag_msg_nw(0x890102,catd,DCORV,DCORV_LOOP,
                                tm_input.dname,tm_input.dnameloc,tm_input.
                                lcount,tm_input.lerrors);

			break;
		}
		if(tm_input.advanced == ADVANCED_TRUE )
		{
			/* change the message number to advanced.	*/
			rc=diag_msg_nw(0x890102,catd,DCORV,DCORV_TITLE_ADV,
                                tm_input.dname,tm_input.dnameloc);
		}
		else
			rc=diag_msg_nw(0x890102,catd,DCORV,DCORV_TITLE,
                                tm_input.dname,tm_input.dnameloc);
		break;
		
	case 2:
		/* check for keyboard touch with no wait.		*/
		rc= diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,FALSE,NULL);
		break;
	default:
		rc=DIAG_ASL_OK;
		break;
	}
	return(rc);
}

/*
 * NAME: ela()
 *
 * FUNCTION: This function designed to do error log analasis, on 
 *	     the adapter or the virtual device depends on the diagnostics
 *	     inputs.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS:
 */

ela()
{
	struct  msglist goal[]=	/* Menu goal msglist		*/
	{
		{ 0,		0 		},
        };
	struct	errdata	err_data;
	struct	task	ar_tasks[]=
		{
			{ "E001",0},	/* SCSI_ERR1 & errnum=1,5 or 0xA*/
			{ "E002",0},	/* SCSI_ERR2 		*/
			{ "E003",0},	/* SCSI_ERR3 		*/
			{ "E004",0},	/* SCSI_ERR10 		*/
			{ "E005",0},	/* SCSI_ERR10 		*/
			{ "E006",0},	/* SCSI_ERR10 vscsi0	*/
			{ "E007",0},	/* SCSI_ERR10 vscsi1	*/
		};
	char	srch_crit[128];
	char	*erd_type[]={""};
	int	option;
	int	errnum;
	int	index;
	int	rc=0;
	int	i;
	int	j;
	int	loop=0;
	char	dname[3][NAMESIZE];
	char	location[3][NAMESIZE];

	memset(dname,0,sizeof(dname));
	memset(location,0,sizeof(location));

	if(tm_input.dmode == DMODE_ELA)
	{
		/* when diagnostics is in ela mode then we must do ela	*/
		/* only on one device. Determine which device needs ela	*/
		/* if the child field is not empty then the DA must do	*/
		/* an ELA on the child otherwise do ELA on itself.	*/
		loop=1;
		strcpy(dname[0],strlen(tm_input.child1)==0 ? tm_input.dname :
			tm_input.child1 );
		strcpy(location[0],strlen(tm_input.childloc1)==0 ?
				tm_input.dnameloc : tm_input.childloc1 );
	}
	if(tm_input.dmode==DMODE_PD)
	{
		if(strlen(tm_input.child1))
		{
			/* This application got called by the controller*/
			/* when the user selected Adv. and PD and there	*/
			/* there is an entry in the error log for the	*/
			/* child.					*/
			strcpy(dname[0],tm_input.child1);
			strcpy(location[0],tm_input.childloc1);
			loop=1;
		}
		else
		{
			strcpy(dname[0],tm_input.dname);
			strcpy(location[0],tm_input.dnameloc);
			strcpy(dname[1],tm_input.child1);
			strcpy(location[1],tm_input.childloc1);
			strcpy(dname[2],tm_input.child2);
			strcpy(location[2],tm_input.childloc2);
			loop=3;
		}

	}
	option=INIT;
	for(i=0; i<loop && rc != EXIT_NOW;i++)
	{
		/* reset all the tasks retry count to 0			*/
		for (j=0;j< (sizeof(ar_tasks)/sizeof(struct task)); j++)
			ar_tasks[j].retry_count=0;

		/* assign search criteria string			*/
		sprintf(srch_crit,"%s -N %s",tm_input.date,dname[i]);
		while(option != TERMI)
		{
			rc = error_log_get(option,srch_crit,&err_data);
			if(rc<=0)
				break;
			(void)diag_db(STRING,1,"detail data ",
					err_data.detail_data);
			errnum=0;
			errnum|=((int)err_data.detail_data[8] ) << 24;
			errnum|=((int)err_data.detail_data[9] ) << 16;
			errnum|=((int)err_data.detail_data[10] ) << 8;
			errnum|=((int)err_data.detail_data[11] ) ;
			(void)diag_db(HEX,1,"error id",err_data.err_id);
			switch(err_data.err_id)
			{
			case	ERRID_SCSI_ERR1:
					index=0;
					break;
				case	ERRID_SCSI_ERR2:
					index=1;
					break;
				case	ERRID_SCSI_ERR3:
					index=2;
					break;
				case	ERRID_SCSI_ERR10:
					index=3;

					/* special case to read the ISR	*/
					/* values. This is only happen	*/
					/* on ascsi.			*/

					if(errnum==0xC &&
						err_data.detail_data[73]!= NULL)
					{
						index=4;
						errnum=err_data.detail_data[73];
						
					}

					/* another special case for the	*/
					/* vscsi device.		*/
					if(dname[i][0] == 'v')
					{
						if(location[i][7] == '0')
							index=5;
						else
							index=6;
					}

					break;
			}
			(void)diag_db(STRING,1,"devname",dname[i]);
			(void)diag_db(DEC,1,"devloc", *location[i],
				"index", index);
			rc=do_erd(ar_tasks,&index,errnum,
				&erd_type[0],&frub[0],&goal[0],1);
			if ( rc == EXIT_NOW )
				break;
			option=SUBSEQ;
		}
	}
	rc = error_log_get(TERMI,srch_crit,&err_data);
 
}
 
/*
 * NAME: clean_up()
 *
 * FUNCTION: Designed to terminate odm, close catalog and quit asl.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: None
 */

clean_up()
{
	if ( odm_flg)
		term_dgodm();
	if (catd !=CATD_ERR)
		catclose(catd);
	if ( tm_input.console==CONSOLE_TRUE )
		diag_asl_quit(NULL);
	diag_db(END);
}
/*  */
/*
 * NAME: get_adp_type()
 *
 * FUNCTION: Looks at the led value found in the CuVPD structure in the odm
 *           data base. The function searches the Customized Devices Data
 *           base for the name passed in tm_input.dname. It then uses the
 *           link found in the PdDvLn entry to go to the Predefined Devices
 *           data base to retreive the value found in the led entry.
 *           This is neccessary to set up the test parameters for each
 *           fixed-disk type that may be tested and to create the correct
 *           fru bucket in the event a test indicates a drive failure.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns the led value if found or and error
 *          code if the data was not found.
 */

int get_adp_type()
{
        char                    odm_search_crit[40];   /* odm search criteria */
	char			*dg;
	char			*ell;
	char			dg_field[3];
	char			ell_field[5];
        struct CuVPD            *cuvpd;      /* ODM Customized VPD struct */
        struct listinfo         obj_info;


	(void)memset(odm_search_crit,0,sizeof(odm_search_crit));
       	sprintf( odm_search_crit, "name = %s", tm_input.dname );
       	cuvpd = get_CuVPD_list( CuVPD_CLASS, odm_search_crit,
			&obj_info, 1, 2 );
       	if ( (cuvpd == ( struct CuVPD * ) -1 ) ||
				  (cuvpd == ( struct CuVPD * ) NULL ) )
		/* if the vpd data is not present we must continue	*/
		/* return unkown adapter type. In the case of microcde	*/
		/* download this DA will get called from umcode if the 	*/
		/* vpd data is missing					*/

		return(UNKOWN_ADAPT);

	/* search for DG field then copy the next 2 characters after	*/

	dg=strstr(cuvpd->vpd,"DG");
       	if (dg == ((char *) NULL)) 
		return(UNKOWN_ADAPT);
	sprintf(dg_field,"%c%c",dg[3],dg[4]);
	(void)diag_db(STRING,1,"DG ",dg_field);
	(void)diag_db(DEC,1,"DG ",atoi(dg_field));

	/* if DE, check VPD LL field to determine if turbo adapter      */

	if (( atoi(dg_field)) == DE)
	{	
		ell=strstr(cuvpd->vpd,"LL");
       		if (ell == ((char *) NULL)) 
			return(UNKOWN_ADAPT);
		sprintf(ell_field,"%c%c%c%c",ell[3],ell[4],ell[5],ell[6]);
		if (( atoi(ell_field)) == DE_TURBO_LL)
			return(TURBO_DE);
	}

        return( atoi(dg_field));
}

/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executed only when there is interrupt.
 *
 * RETURNS: NONE
 */

void
int_handler(int sig)
{
	(void)diag_db(STRING,1,"Interrupt handler called:","cleaning up");
        if ( tm_input.console == CONSOLE_TRUE )
                diag_asl_clear_screen();
	if(clean_tu)
		clean_up_task();
        clean_up();
	DA_EXIT();
}


/*
 * NAME: clean_up_task()
 *
 * FUNCTION: Perform general test unit cleanups before exiting the DA.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

clean_up_task()
{
	int	num_task=0;
	int	rc;
	int	cur_index=0;
	char	task_string[1024];
	struct  task *ar_tasks;	/* pointer to task structure		*/
	/* this is a cleanup section does not have any error	*/
	/* reaction data					*/

	(void)get_task_data("CLEAN_UP",&task_string);
	(void)diag_db(STRING,1,"clean_up_task called:",task_string);
	ar_tasks=get_tasks(task_string,&num_task);
	while ( cur_index < num_task)
	{
		(void ) do_task(ar_tasks[cur_index],&rc);
		cur_index++;
	}
}
