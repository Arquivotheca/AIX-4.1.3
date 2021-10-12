static char sccsid[] = "@(#)22	1.9.1.15  src/bos/diag/sysx/sysxtest.c, dsysx, bos41J, 9509A_all 2/17/95 13:25:13";
/*
 * COMPONENT_NAME: DSYSX System Exerciser
 *
 * FUNCTIONS: 	run_sysx
 *		get_time
 *		update_time
 *		count_loops
 *		gen_sysx_table
 *		sysx_tests
 *		chk_test
 *		parse_status
 *		add_device_to_list
 *		find_child
 *		find_dev
 *		wait_for_interaction
 *		init_time
 *		kill_all 
 *		resource_desc
 *		copy_text
 *		start_test
 *		stop_tests
 *		deldainput
 *		putdainput
 *		child_status
 *		check_cancel_exit
 *		disp_standby
 *		disp_standby_loop
 *		disp_ts_menu
 *		disp_tm_menu
 *		disp_sysx_results 
 *		disp_sysx_runtime
 *		disp_sysx_ntf
 *		disp_fru
 *		add_to_problem_menu
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */                                                                   

#include <stdio.h>
#include <cur02.h>
#include <nl_types.h>
#include <signal.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/processor.h>
#include <sys/ppda.h>
#include "sysx_msg.h"
#include "diag/dcda_msg.h"
#include "diag/diag.h"
#include "diag/da.h"
#include "diag/tmdefs.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "diag/class_def.h"

#ifndef	SYSX_INTERACTION
#define	SYSX_INTERACTION	SYSX_MEDIA
#endif
#define da_exit_key	(da_rc.field.user==DA_USER_EXIT)
#define da_cancel_key	(da_rc.field.user==DA_USER_QUIT)
#define da_cont		(da_rc.field.more==DA_MORE_CONT)
#define menu_exit_key	(rc==DIAG_ASL_EXIT)
#define menu_cancel_key	(rc==DIAG_ASL_CANCEL)
#define TEST_SUPTEST(x)	(table->dev_ptr->T_PDiagDev->SupTests & x)
#define TEST_TYPE(x)	(table->dev_ptr->T_PDiagDev->SysxFlg & x)
#define MAX(x, y)	((x) > (y)) ? (x) : (y)
#define MIN(x, y)	((x) < (y)) ? (x) : (y)

#define MAX_LOOPS	25		/* maximum loops */ 
#define SLEEP_TIME	60		/* time to wait for DA to start */
#define SHORTEST	1		/* Min # of secs. for any DA */
#define LONGEST		32767		/* Max # of secs. for any DA */ 

typedef struct dev_table_info_s { 
	diag_dev_info_t    	*dev_ptr;
	int			pid;		/* pid of DA		*/
	short 			error_count;	/* # of errors generated */
	short			loop_count;	/* # of times run so far */
	short			max_loops;	/* # of times to run	*/
	short			restart_cnt;	/* # of loops before restart */
	short			selected;	/* if test selected     */
	int			start_time;	/* time DA started	*/
	int			status;         /* status from DA 	*/
	cpu_t			cpuid;		/* processor number	*/
} dev_table_t;

/* GLOBAL VARIABLES */
int 	num_dev_to_test;		/* maximum number of devices to test */
int	seconds;
int	serial_flg;
int	loopmodeflg = LOOPMODE_NOTLM;
int	Scroll = 0;			/* scroll result in loop_mode */
char	command[512];
char	*com;
cpu_t	tab_processor[MAXCPU];		/*				*/

nl_catd cfg_fdes;
union {
	unsigned char	all;
	da_returncode_t	field;
} da_rc;
dev_table_t *head_table;

/* EXTERNALLY DEFINED VARIABLES */
extern int systestflg;
extern int advancedflg;
extern int exenvflg;
extern int consoleflg;      
extern int diag_mode;          		/* diagnostic test mode         */
extern int lmflg;         		/* loop mode flag               */
extern int loopmode;			/* test to be performed in loop */
extern short lcount;			/* loop count   		*/
extern short lerror;			/* number of errors in loop     */
extern int num_All;
extern diag_dev_info_ptr_t *All;
extern nl_catd fdes;
extern char *dadir;
extern int media_flg;
extern nl_catd diag_catopen(char *, int);

/* LOCAL FUNCTION PROTOTYPES */
int run_sysx(void);
short get_time(dev_table_t *);
void update_time(dev_table_t *, int);
void count_loops(int, dev_table_t *);
void gen_sysx_table(dev_table_t *);
int sysx_tests(void);
int chk_test(dev_table_t *);
int parse_status(dev_table_t *);
int add_device_to_list(int, diag_dev_info_ptr_t *);
int find_child(int, diag_dev_info_ptr_t *);
diag_dev_info_t *find_dev(char *);
int wait_for_interaction(dev_table_t *, int *);
void init_time(dev_table_t *);
void kill_all(void);
int resource_desc(char *, dev_table_t *);
int copy_text(int, char *, char *);
int start_test(dev_table_t *);
int stop_tests(void);
int deldainput(int );
int putdainput(dev_table_t *);
int child_status(dev_table_t *);
int check_cancel_exit(void);
int disp_standby(void);
int disp_standby_loop(void);
int disp_sysx(void);
int disp_ts_menu(dev_table_t *table);
int disp_tm_menu(int *);
int disp_sysx_results(void);
int disp_sysx_runtime(void);
int disp_sysx_ntf(void);
int disp_fru(dev_table_t *);
int add_to_problem_menu(dev_table_t *, char *, ASL_SCR_INFO *, int *, char *, int *);
int load_diag_kext(char *);

/* EXTERNAL FUNCTION DECLARATIONS */
short  convert(short);
extern char *substrg();
extern void *malloc();
extern char *diag_cat_gets(nl_catd, int, int);
extern struct CuAt *getattr(char *, char *, int, int *);

/*  */

/* NAME: init_tab_processor
 *
 * FUNCTION: initialize tab_processor with process number
 * 
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */

void init_tab_processor(void)
{
int i;
cpu_t	p;
	for(i=0;i<MAXCPU;i++)
	{
		p = i;
		if(bindprocessor(BINDPROCESS,getpid(),p) == 0 )
			tab_processor[i]=p;
		else	tab_processor[i]= -1;		
	}
}

/* NAME: next_processor
 *
 * FUNCTION:
 * 
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: index of next_processor available in tab_processor
 *
 */

static cpu_t	next_processor()
{

int p;
static int	ix = 0;


	if(ix < 0) ix = MAXCPU -1;
	p=(ix + 1) % MAXCPU;

	for(;p!=ix;p = (p+1) % MAXCPU)
		if(tab_processor[p] != -1) break;

	ix=p;
	return(tab_processor[p]);
}

/*  */
/*
 * NAME: run_sysx  
 *
 * FUNCTION: main test loop for system exerciser
 *
 * NOTES:
 *
 * RETURNS:
 *
 */
int run_sysx(void)
{
	int			rc;		/* return code 		*/
	dev_table_t		*table;		/* list of devices 	*/


	/* generate a table of devices to test */
	head_table = (dev_table_t *)calloc(num_All, sizeof(head_table[0]));
	gen_sysx_table (head_table);
	init_tab_processor();

	do {
		/* clear out FRU object classes for another pass */
		clr_class("FRUB");
		clr_class("FRUs");

		table = head_table;
		while (table->dev_ptr)
		  {
		  table->max_loops = 0;
		  table->loop_count = 0;
		  table->error_count = 0;
		  table->start_time = 0;
		  table->pid = 0;
		  table->status = 0;
		  table->selected = FALSE;
		  table->cpuid = -1;
		  table++;
		  }
		/* present selection menu of devices to test */
		rc = disp_ts_menu(head_table);
		if (menu_exit_key || menu_cancel_key) return(rc);
		/* present test method selection: loopmode or not	*/
		rc = disp_tm_menu(&loopmodeflg);
		if (menu_exit_key || menu_cancel_key) return(rc);

		/* execute all the selected diagnostic applications */
		do
		  {
		  count_loops(FALSE, (dev_table_t *)NULL);
		  rc = sysx_tests();
		  if (menu_exit_key) return(rc);
		  rc = disp_sysx_runtime();
		  if (menu_exit_key) return(rc);
		  } while (!menu_cancel_key);

		table = head_table;
        	while (table->dev_ptr) {
			if(!table->selected) {
				table++;
				continue;
			}
			if (table->error_count) {
				rc = disp_sysx_results();
				break;
			}
			table++;
		}
		stop_tests();

	} while (!menu_exit_key);

	table = head_table;
	while (table->dev_ptr)
	  {
	  lerror = MAX(lerror, table->error_count);
	  table++;
	  }

	return (rc);
}

/*  */
/* NAME: get_time
 *
 * FUNCTION: Determine time of test to nearest second
 * 
 * NOTES: The SysxTime field in CDiagDev holds the
 *	time (in seconds) it takes for a DA to test
 *	the device.  Since this field is initialized
 *	to 0, a default value is used to start with.
 *	Thereafter, the DA is timed and the resulting
 *	value is written out to the CDiagDev object class.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: # of seconds to run a particular DA.
 */
short get_time(dev_table_t *table)
{
  short time_val;

	time_val = table->dev_ptr->T_CDiagDev->SysxTime;
	if (time_val <= 0) time_val = SHORTEST;
	return(time_val);
}

/*  */
/* NAME: update_time
 *
 * FUNCTION: Update the CDiagDev with an accurate time for
 *	     how long it took to run a DA
 * 
 * NOTES: See notes on get_time()
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None
 */
void update_time(dev_table_t *table, int new_time)
{
	if (new_time > LONGEST)  new_time = LONGEST;
	if (new_time < SHORTEST) new_time = SHORTEST;
	table->dev_ptr->T_CDiagDev->SysxTime = (short )new_time;
	return;
}

/*  */
/* NAME: count_loops
 *
 * FUNCTION: Determine the number of times each test should run
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 
 */
void count_loops(int update, dev_table_t *utable)
{
  dev_table_t *table;
  short fastest;
  short slowest;
  short long_test;
  short serial_time;
  static short max_slow;

	/****************************************************************
	* Determine the fastest and slowest running DAs.  Also		*
	* determine if there are any "long" tests (a "long" test	*
	* should generally only be run once).  If there are DAs which	*
	* must run serially (e.g., keyboard, mouse, & tablet) determine	*
	* the total time to run all serial tests.			*
	****************************************************************/
	fastest = LONGEST;
	slowest = 0;
	long_test = 0;
	serial_time = 0;

	table = head_table;
	while (table->dev_ptr)
	  {
	  if (!update) table->restart_cnt = table->max_loops;
	  fastest = MIN(fastest, get_time(table));
	  slowest = MAX(slowest, get_time(table));
	  if (TEST_TYPE(SYSX_LONG)) long_test++;
	  if (TEST_TYPE(SYSX_ALONE)) 
	    serial_time += get_time(table);
	  table++;
	  }

	if (long_test == 0)
	  slowest = MAX(slowest, MAX_LOOPS*fastest);
	slowest = MAX(slowest, serial_time);

	/****************************************************************
	* If this call is for an update, make sure the # of loops is	*
	* updated for the DA passed in.  Else, if the call is not for	*
	* for an update OR the slowest DA is faster than it used to be	*
	* then update the # of loops for all DAs.			*
	****************************************************************/
	if (update)
	  {
	  table = utable;
	  if (TEST_TYPE(SYSX_ALONE)) 
	    table->max_loops = table->restart_cnt + slowest/serial_time;
	  else
	    table->max_loops = table->restart_cnt + slowest/get_time(table); 
	  }

	if (!update || (slowest < max_slow))
	  {
	  table = head_table;
	  while (table->dev_ptr)
	    {
	    if (TEST_TYPE(SYSX_ALONE)) 
	      table->max_loops = table->restart_cnt + slowest/serial_time;
	    else
	      table->max_loops = table->restart_cnt + slowest/get_time(table); 
	    table++;
	    }
	  }

	max_slow = slowest;
	return;
}

/*  */
/* NAME: gen_sysx_table
 *
 * FUNCTION: generate a table of devices to run system exerciser on
 * 
 * NOTES: This list will exclude any drawers, memory, and devices
 *        which are not supported by diagnostics.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */

void gen_sysx_table(dev_table_t *table)
{
	int	i;

	num_dev_to_test = 0;
	for (i=0; i < num_All; i++)
	  if (add_device_to_list(i, All))
	    {
	    table->dev_ptr = (diag_dev_info_t *)All[i];	
	    num_dev_to_test++;
	    table++;
	    }
	table->dev_ptr = (diag_dev_info_t *) NULL;
	return;
}

/*  */
/* NAME: sysx_tests
 *
 * FUNCTION: test all devices 
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:	DIAG_ASL_EXIT, when exit key entered
 *		DIAG_ASL_CANCEL, when cancel key entered
 *
 */
int sysx_tests(void)
{
  int		rc;
  int		DA_cnt;
  dev_table_t	*table;

	rc = DIAG_ASL_OK;
	seconds = 0L;
	DA_cnt = 0;
	serial_flg = DIAG_FALSE;	

	/****************************************************************
	* Search the table for any device that uses the diagex kernel   *  
	* extension. If found, load it once.                         	*
	****************************************************************/
	table = head_table;
	while (table->dev_ptr)
	  {
	  if TEST_SUPTEST(SUPTESTS_DIAGEX)
	    	load_diag_kext(table->dev_ptr->T_CuDv->PdDvLn_Lvalue);
	  table++;
	  }

	/****************************************************************
	* Start the media DAs.  Each one will prompt for media.  When	*
	* when user responds with media (or decides not to use media)	*
	* then TMInput will be released.				*
	****************************************************************/
	consoleflg = TRUE;
	table = head_table;
	lmflg=LOOPMODE_ENTERLM;
	while (table->dev_ptr && !menu_exit_key && !menu_cancel_key)
	  {
	  if (TEST_TYPE(SYSX_INTERACTION) && chk_test(table) && !table->loop_count)
	    {
	    if (advancedflg)	media_flg = TRUE;
	    start_test(table);
	    rc = wait_for_interaction(table, &DA_cnt);
	    serial_flg = FALSE;
	    DA_cnt++;
	    }
	  table++;
	  }

	/****************************************************************
	* Start the rest of the DAs.  If one finishes, check its exit	*
	* status, then start it next time around.  Keep looping through	*
	* the list of DAs until all DAs have stopped (DA_cnt == 0) AND	*
	* no DAs can be started during the next pass.			*
	****************************************************************/
	consoleflg = FALSE;
	table = head_table;
	/* init for display stanby_loop		*/
	seconds = 0L;
	Scroll = 0;
	while (table->dev_ptr && !menu_exit_key && !menu_cancel_key)
	  {
	  if (table->pid)
	    {
	    if (child_status(table)) DA_cnt--;
	    }
	  else if (chk_test(table))
	    {
	    lmflg=LOOPMODE_INLM;
	    if(!TEST_TYPE(SYSX_INTERACTION) && table->loop_count == 0)
			lmflg=LOOPMODE_INLM;
	    start_test(table);
	    init_time(table);
	    DA_cnt++;
	    }

          if( loopmodeflg == LOOPMODE_ENTERLM )	disp_standby_loop();
	  else	disp_standby();
	  rc = check_cancel_exit();
	  table++;
	  if (table->dev_ptr == (diag_dev_info_t *)NULL)
	    {
	    table = head_table;
	    if (DA_cnt == 0)
	      while (table->dev_ptr)
	        {
		if (chk_test(table)) break;
		else		     table++;
	        }
	    }
	  }

	/* make sure all processes are stopped */
	kill_all();

	/* Set the max # of loops equal to the loop	*/
	/* count.  They should be the same already	*/
	/* unless the testing was aborted early.	*/ 
	table = head_table;
	while (table->dev_ptr)
	  {
	  table->max_loops = table->loop_count;
	  table++;
	  }

	return(rc);
}

/* NAME: stop_tests
 *
 * FUNCTION: run once test on all INTERACTION devices in LOOPMODE_EXITLM
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:	DIAG_ASL_EXIT, when exit key entered
 *		DIAG_ASL_CANCEL, when cancel key entered
 *
 */
int stop_tests(void)
{
  int		rc;
  int		DA_cnt=0;
  dev_table_t	*table;

	/* Start DA with INTERACTION in LOOPMODE_EXITLM */
	rc = DIAG_ASL_OK;
	lmflg=LOOPMODE_EXITLM;
	consoleflg = TRUE;
	table = head_table;
	while (table->dev_ptr && !menu_exit_key && !menu_cancel_key)
	  {
	  if(table->max_loops)	table->max_loops = table->loop_count + 1;

	  if (TEST_TYPE(SYSX_INTERACTION) && chk_test(table))
	    {
	    start_test(table);
	    rc = wait_for_interaction(table, &DA_cnt);
	    serial_flg = FALSE;
	    DA_cnt++;
	    }
	  table++;
	  }

	/* make sure all processes are stopped */
	kill_all();
	return(rc);
}

/*  */
/* NAME: chk_test
 *
 * FUNCTION: Determine whether a particular test should be run 
 * 
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  0 (= do not run test) 
 *	     1 (= run test)
 */
int chk_test(dev_table_t *table)
{

	if (TEST_TYPE(SYSX_NO)) return(0);
 	if (table->pid) return(0);
	/* If loopmode, don't check for max_loops	*/ 
	if ( loopmodeflg == LOOPMODE_NOTLM
	 && table->loop_count >= table->max_loops)	return(0);
	if (TEST_TYPE(SYSX_ALONE) && serial_flg) return(0);
	/* Test only selected devices			*/
	if(table->selected==FALSE) return(0);
	return(1);
}

/*  */
/*
 * NAME: parse_status
 *
 * FUNCTION: 
 *
 * NOTES:
 *
 * RETURNS:
 */

int parse_status(dev_table_t *table)
{
	static struct fru_bucket frub;

	/* convert status to something readable */
	da_rc.all = table->status >> 8;
	
	if ( ( da_rc.field.status == DA_STATUS_BAD ) || 
	     ( da_rc.field.error  == DA_ERROR_OTHER ) || 
	     ( table->status & 0xFF) )
		table->error_count++;

	/* Software error - generate a FRU bucket for display */
	if ((da_rc.field.error == DA_ERROR_OTHER) || 
	    (table->status & 0xFF)) {
		strcpy( frub.dname, table->dev_ptr->T_CuDv->name);
		frub.ftype = FRUB1;
		frub.rcode = table->dev_ptr->T_Pdv->led;
		frub.sn = DC_SOURCE_SOFT;
		frub.rmsg = DC_SOFT_MSG;
		frub.frus[0].conf = 0;
		addfrub( &frub );
	}
	return(0);
}

/*  */
/* NAME: add_device_to_list
 *
 * FUNCTION: search database for a child of input device
 * 
 * NOTES: This list will exclude any drawers, memory, and devices
 *        which are not supported by diagnostics.
 *	  Also, any device that has been excluded from the test list,
 *        or marked as MISSING will not be tested.
 *	  Also, any proc? which is not AVAILABLE
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 - device is not testable
 *          1 - device is a testable end device
 *
 */

int add_device_to_list(int idx,
		    diag_dev_info_ptr_t *List)
{
	char	*name;
	char	*class;
	int	rc;

	class = substrg(PCLASS, List[idx]->T_CuDv->PdDvLn_Lvalue);

	/* do not test memory */
	if ( !strcmp(class, CLASS_MEMORY) ) return(0);

	/* do not test any missing device */
	if (List[idx]->T_CuDv->chgstatus == MISSING) return(0);

	/* do not test any device named proc? which is not AVAILABLE */
	if (!strncmp(List[idx]->T_CuDv->name,"proc",4)
		&& (List[idx]->T_CuDv->status & AVAILABLE)== 0) return(0);
	
        /* do not test any device which is not supported by diagnostics */
        if (!List[idx]->T_PDiagDev) return(0);

	/* do not test any device which is not supported by sysx */
	if (List[idx]->T_PDiagDev->SysxFlg & SYSX_NO) return(0);
	
	/* do not test any device deleted from the test list */
	if ((List[idx]->T_CDiagDev) &&
	    (List[idx]->T_CDiagDev->RtMenu == RTMENU_DDTL)) return(0);

	/* do not test any device which does not have a DA */
	if (!strlen(List[idx]->T_PDiagDev->DaName)) return(0);

	/* do test any device which has attached devices not testable */
	if (strlen(List[idx]->T_PDiagDev->AttDType)) return(1);

	rc = find_child(idx, List);
	return(rc);
}

/*  */
/* NAME: find_child 
 *
 * FUNCTION: search database for a child of input device
 * 
 * NOTES: This list will exclude any drawers, memory, and devices
 *        which are not supported by diagnostics.
 *	  Also, any device that has been excluded from the test list,
 *        or marked as MISSING will not be tested.
 *	  Also, any proc? which is not AVAILABLE
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 - device is not testable
 *          1 - device is a testable end device
 *
 */

int find_child( int idx,
	        diag_dev_info_ptr_t *List)
{
	int search_more_children = 0;
	int found_testable_child = 0;
	char *name;

	name = List[idx]->T_CuDv->name;
	for ( ++idx; idx < num_All; idx++ )
		/* search for a child device */
		if(!strcmp(name, List[idx]->T_CuDv->parent)) {
			search_more_children = 0;

        		/* continue search if child device is missing */
        		if (List[idx]->T_CuDv->chgstatus == MISSING)
				;
        		/* continue search if device is not supported by diagnostics */
			else if (!List[idx]->T_PDiagDev)
				search_more_children = 1;
        		/* continue search if device is not supported by sysx */
        		else if (List[idx]->T_PDiagDev->SysxFlg & SYSX_NO) 
				search_more_children = 1;
			/* continue search if device is deleted from the test list */
        		else if ((List[idx]->T_CDiagDev) && (List[idx]->T_CDiagDev->RtMenu == RTMENU_DDTL))
				;
        		/* continue search if device does not have a DA */
        		else if (!strlen(List[idx]->T_PDiagDev->DaName))
				search_more_children = 1;
			else
				found_testable_child = 1;
			if ( search_more_children ) {
				if( find_child ( idx, List ) == 0 )
					return (0);
			}
			else if (found_testable_child == 1)
				return (0);
		}
	return(1);
}

/*  */
/*
 * NAME: find_dev
 *
 * FUNCTION: Searches the All array for a device 
 *	     matching 'device_name'
 *
 * NOTES:
 *
 * RETURNS:
 *	diag_dev_info_t *      : pointer to device 	
 *	diag_dev_info_t * NULL : if device not found	
 */

diag_dev_info_t *find_dev(char *device_name)
{
	int	i;

	for ( i = 0; i < num_All; i++) 
		if(!strcmp(device_name,All[i]->T_CuDv->name))
			break;

	return((i==num_All) ? (diag_dev_info_t *)NULL : All[i]);
}

/* NAME: wait_for_interaction
 *
 * FUNCTION: 
 * 
 * NOTES:
 *
 * RETURNS: DIAG_ASL_OK
 *	    DIAG_ASL_EXIT if media DA was cancelled.
 */
int wait_for_interaction(
	dev_table_t *table, 
	int *DA_cnt)
{
  int rc;
	/* wait for end of preparation pass	*/
	/* this pass is not counted as a loop	*/

	child_status(table);
	(*DA_cnt)--;
	if (da_exit_key || da_cancel_key) return(DIAG_ASL_EXIT);
	return(DIAG_ASL_OK);
}

/* NAME: init_time
 *
 * FUNCTION: 
 * 
 * NOTES:
 *
 * RETURNS:
 */

void init_time(dev_table_t *table) 
{
  struct timeval loc_time;
  struct timezone loc_tz;

	gettimeofday(&loc_time, &loc_tz);
	table->start_time = loc_time.tv_sec;
	table->loop_count++;
}

/* NAME: kill_all 
 *
 * FUNCTION: kill all active processes 
 * 
 * NOTES:
 *
 * RETURNS: 
 */
void kill_all(void)
{
	dev_table_t *table;
	int pid, status;

	table = head_table;
	while (table->dev_ptr)
	  {
	  if (table->pid) kill(table->pid, SIGTERM);
	  table++;
	  }

	table = head_table;
	while (table->dev_ptr)
	  {
	  if (table->pid)
	    {
	    pid = (int )waitpid((pid_t )table->pid, &status, 0);
	    deldainput(table->pid);
	    if (pid == 0) genexit(-1);
	    table->pid = 0;
	    }
	  table++;
	  }
	return;
}

/*   */
/* NAME: resource_desc
 *
 * FUNCTION: Build a text string describing the resource and its location.
 * 
 * NOTES:
 *	This function takes as input a line type which indicates the type
 *	of display line to build.  
 *
 * RETURNS: 0
 */

int resource_desc(char *string,
		  dev_table_t *table)
{

	char	*tmp;

	sprintf(string, "%-16s %-16.16s ", 
		table->dev_ptr->T_CuDv->name,
		table->dev_ptr->T_CuDv->location);
	tmp = string + strlen(string);
	copy_text( strlen(string), tmp, table->dev_ptr->Text);
	return(0);
}

/* NAME: copy_text
 *
 * FUNCTION: 
 * 
 * NOTES:
 *
 * RETURNS: 0
 */

int copy_text(int string_length,	/* current length of text already in buffer	*/ 
	      char *buffer,		/* buffer to copy text into	*/
	      char *text)		/* text to be copied 		*/
{

	int	i;
	int 	space_count;
	int 	char_positions;

	/* determine if length of text string will fit on one line */
	char_positions = LINE_LENGTH - string_length;
	if ( char_positions < strlen(text))  {

		/* dont break the line in the middle of a word */
		if(text[char_positions] != ' ')
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

	return(0);
}

/*   */
/* NAME: start_test
 *
 * FUNCTION: 
 * 
 * NOTES:
 *
 * RETURNS: 0 if da not started
 *          1 if da sucessfully started
 */
int start_test(dev_table_t *table)
{
  int	pid;
  char	new_DA[128];
  char 	*com;
  char  *options[2];
  char	command[512];

	strcpy(command,dadir);
	com = command + strlen(dadir);
	*(com++) = '/';

	/************************************************************
         If the DA name starts with an '*' then we must locate the
         name of the proper executable.
        ************************************************************/
        if (table->dev_ptr->T_PDiagDev->DaName[0] == '*') {
                if ( (determine_DA( &new_DA, table )) == 0 )
                	strcpy(com, new_DA);
        }

        /*******************************
         else DA name is name of executable
        *******************************/
        else {
                strcpy(com, table->dev_ptr->T_PDiagDev->DaName);
        }

       /* Set up argv arguments for execute */
	options[0] = command;
	options[1] = (char *) NULL;

	/* fork off the da and save the pid */
	table->cpuid = next_processor() + 1;
	if ( (pid = fork()) == 0 ) {	
		/* populate the TMInput class */
		putdainput(table);	/* pid set by child proces	*/
		execv(command, options);	/* in the child process */
		exit(-1);
	}
	table->pid = pid;
	if (TEST_TYPE(SYSX_ALONE)) serial_flg = TRUE;
	return (1);

}

/* ^L  */
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
determine_DA( char *DA_name, dev_table_t *table )
{
        int rc=0, cnt=0;
        struct CuAt *cuat;
        char crit[128];
        struct  listinfo c_info, p_info;
        struct  CuDv    *parent_cudv;
        struct  PDiagAtt    *pdiagatt;


        /* set this to NULL in case of error */
        *(DA_name) = (char)NULL;

        if (!strncmp("parent_type", table->dev_ptr->T_PDiagDev->DaName + 1, 11)) {
                /* first get the parent's Class/Subclass/Type */
                sprintf(crit, "name = %s", table->dev_ptr->T_CuDv->parent);
                parent_cudv = (struct CuDv *)diag_get_list(CuDv_CLASS, crit,
                                &c_info, 1,2);
                if( (parent_cudv == (struct CuDv *)-1) ||
                    (parent_cudv == (struct CuDv *)NULL) )
                        return(-1);

                /* Now get PDiagAtt for the device under test */
                sprintf(crit,
                     "value=%s AND DType=%s AND DSClass=%s AND DClass=%s",
                        parent_cudv->PdDvLn_Lvalue,
                        table->dev_ptr->T_PDiagDev->DType,
                        table->dev_ptr->T_PDiagDev->DSClass,
                        table->dev_ptr->T_PDiagDev->DClass );
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
        if (!strncmp("modelcode", table->dev_ptr->T_PDiagDev->DaName + 1, 9))
                cuat = (struct CuAt *)getattr("sys0", "modelcode", 0, &cnt);
        else
                /* might be different attribute like *_subtype */
                cuat = (struct CuAt *)getattr(table->dev_ptr->T_CuDv->name,
                                table->dev_ptr->T_PDiagDev->DaName + 1, 0, &cnt);

        if (((void * )cuat == NULL) || (cnt != 1))
                rc = -1;
        else
                rc = get_diag_att(table->dev_ptr->T_PDiagDev->DType,
                                cuat->value, 's', &cnt, DA_name);

        /* If no modelcode attribute, or error, get the default */
        if (rc < 0)
                rc = get_diag_att(table->dev_ptr->T_PDiagDev->DType,
                                "default", 's', &cnt, DA_name);
        if (rc < 0) return(-1);
        else return (0);

}


/*   */
/* NAME: deldainput
 *
 * FUNCTION: This unit delete the TMInput Object Class which pid is pid
 * 
 * 
 * RETURNS: NONE
 */

int deldainput(int pid)
{
	char    crit[40];
	sprintf(crit,"pid=%d",pid);
	diag_rm_obj(TMInput_CLASS, crit);
}
/* NAME: putdainput
 *
 * FUNCTION: This unit populates the TMInput Object Class with data used by
 *		the diagnostic applications.
 * 
 * RETURNS:
 *	0  - put successful
 *	-1 - put unsuccessful
 */

int putdainput(dev_table_t *table)
{

	int 		rc;
	static struct	TMInput	T_TMInput;
	diag_dev_info_t *parent_dev;

	/* to be on the safe side */
	deldainput(getpid());

	T_TMInput.exenv = EXENV_SYSX;
	T_TMInput.advanced = advancedflg;
	T_TMInput.system = systestflg;
	T_TMInput.dmode = diag_mode;
	T_TMInput.loopmode = lmflg;
	T_TMInput.lcount = lcount;
	T_TMInput.lerrors = lerror;
	T_TMInput.console = consoleflg;

	/* set up defaults for children */
	T_TMInput.state1 = T_TMInput.state2 = 0;
	T_TMInput.child1[0] = T_TMInput.child2[0] = '\0';
	T_TMInput.childloc1[0] = T_TMInput.childloc2[0] = '\0';

	T_TMInput.date[0] = '\0';

	strcpy(T_TMInput.dname, table->dev_ptr->T_CuDv->name);

	/* location is expanded for ports	*/
	strcpy(T_TMInput.dnameloc, table->dev_ptr->T_CuDv->location);

	/* enter parent information */
	strcpy(T_TMInput.parent, table->dev_ptr->T_CuDv->parent);
	parent_dev = find_dev(table->dev_ptr->T_CuDv->parent);
	if ( parent_dev != NULL )
		strcpy(T_TMInput.parentloc, parent_dev->T_CuDv->location);
	else
		T_TMInput.parentloc[0] = '\0';
	
	T_TMInput.pid = getpid();	/* enter pid information	*/

	T_TMInput.cpuid = table->cpuid;

	rc = diag_add_obj(TMInput_CLASS,&T_TMInput);

	return( (rc == -1) ? -1: 0 );
}

/*   */
/* NAME: child_status 
 *
 * FUNCTION: 
 * 
 * RETURNS:	0, process is still active
 *		>0 , pid of inactive process
 */
int child_status(dev_table_t *table)
{
  int	savemask;
  int	pid = 0;
  int	status;
  int	new_time;
  struct timeval loc_time;
  struct timezone loc_tz;

	savemask = sigblock(1 << (SIGINT-1));

	/* if process was started, check if terminated */
	if (table->pid)
	  {
	  /* if preparation pass, wait for the process to be ended	*/
	  pid = (int )waitpid((pid_t )table->pid, &status,consoleflg?0:WNOHANG);
	  table->status = status;
	  if (pid > 0)
	    {
	    gettimeofday(&loc_time, &loc_tz);
	    new_time = loc_time.tv_sec - table->start_time;
	    /* no count in preparation pass		*/
	    if(consoleflg == FALSE)
	 	{
		update_time(table, new_time);
		count_loops(TRUE, table);
	 	}
	    if (TEST_TYPE(SYSX_ALONE)) serial_flg = FALSE;
	    deldainput(table->pid);
	    table->pid = 0;
	    parse_status(table);
	    }
	  }

	(void) sigsetmask(savemask);
	return (pid);
}

/*  */
/* NAME: check_cancel_exit
 *
 * FUNCTION: check for the Cancel/Exit key
 * 
 * NOTES: Check also for KEY_UP and KEY_DOWN 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:	DIAG_ASL_EXIT, when exit key entered
 *		DIAG_ASL_CANCEL, when cancel key entered
 *	        DIAG_ASL_OK	
 *
 */

int check_cancel_exit(void)
{
	int	rc,key;
	char	buf[128];
	static struct msglist can_menu[] = {
		{SET_SYSXL, MSG_S4},
		{(int )NULL, (int )NULL}
	};
	rc = diag_asl_read(ASL_DIAG_OUTPUT_LEAVE_SC,FALSE,buf);

	if ( rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL )
		diag_display ( 0x803007, fdes, can_menu, DIAG_IO,
			    ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL);

	/* Check for KEY_UP and KEY_DOWN		*/
	/* Set Scroll for disp_standby_loop function	*/
	/* Try to regenerate the key code: we only get low order byte */
	key = buf[0] + 0x100;
	if(key  == KEY_DOWN) {
		Scroll++; 
		seconds = 0L;
	}
	else if(key == KEY_UP && Scroll > 0) {
		Scroll--;
		seconds = 0L;
	}

	return(rc);
}
/*  */
/* NAME: disp_standby
 *
 * FUNCTION: display standby screen with timestamp
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */

int disp_standby(void)
{
	int	rc;
	char	timestamp[80];
	char	*standby;
	struct	timeval loc_time;
	struct	timezone loc_tz;
	static struct msglist sys_menu[] = {
		{SET_SYSXL, MSG_S3},
		{(int )NULL, (int )NULL}
	};
	static ASL_SCR_INFO menuinfo[ DIAG_NUM_ENTRIES(sys_menu) ];
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	gettimeofday(&loc_time, &loc_tz);
	if ((seconds == 0) || (loc_time.tv_sec - seconds) > 60) 
		seconds = loc_time.tv_sec;
	else
		return(DIAG_ASL_OK);

	/* put date and time into header */
	getdate(timestamp, sizeof(timestamp));
	rc = diag_display ( 0x803006, fdes, sys_menu, DIAG_MSGONLY,
			    NULL, &menutype, menuinfo);
	standby = (char *)malloc(2048);
	sprintf(standby,menuinfo[0].text,timestamp);
	free ( menuinfo[0].text );
	menuinfo[0].text = standby;
	rc = diag_display ( 0x803006, fdes, NULL, DIAG_IO,
			    ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menuinfo);

	free ( standby );
	return(rc);
}
/*  */
/* NAME: disp_standby_loop
 *
 * FUNCTION: display standby screen with timestamp in loop mode
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */

int disp_standby_loop(void)
{
	int	rc;
	dev_table_t 	*table;
        int     	line=0;
	int		s;
        char            *string;
        char            *free_string;
	char	timestamp[80];
	char	*standby;
	struct	timeval loc_time;
	struct	timezone loc_tz;
	static ASL_SCR_INFO    *menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	gettimeofday(&loc_time, &loc_tz);
	if(seconds == 0)	menutype.cur_index = 0;
	if ((seconds == 0) || (loc_time.tv_sec - seconds) > 4) 
		seconds = loc_time.tv_sec;
	else
		return(DIAG_ASL_OK);

	/* put date and time into header */
	getdate(timestamp, sizeof(timestamp));



        /* allocate space for enough entries */
	if(!menuinfo)
        	menuinfo = (ASL_SCR_INFO *)
                	calloc(1,(num_dev_to_test+5)*sizeof(ASL_SCR_INFO) );
        if ( menuinfo == (ASL_SCR_INFO *) NULL) {
                return(-1);
	}

        free_string = string = (char *)malloc(num_dev_to_test*132);
        if ( string == (char *) NULL)
                return(-1);

        /* set the title line in the array  */
	sprintf(string,diag_cat_gets(fdes, SET_SYSXL, MSG_S5),timestamp);

	menuinfo[line++].text = string;

	string = string + strlen(string)+1;

	/* Set Scroll			*/
	for(s=0,table = head_table;
		(Scroll < 0 || s<Scroll) && table->dev_ptr;) {
		table++;
		s++;
	}
	if(s!=Scroll && Scroll < 0 ) {
		Scroll = s -1;
		table--;
	}
	if(!table->dev_ptr) {
		table = head_table;
		s=0;Scroll=0;
	}

        /* read the entries from table, get the DName, location, and
           description and put them in the array        */
	do {
		if(table->selected) {
			sprintf(string,"%s\t\t%10d\t%10d",
				table->dev_ptr->T_CuDv->name,
				table->loop_count,
				table->error_count);
               		menuinfo[line++].text = string;
			string = string + strlen(string)+1;
		}
		table++;
		s++;
		if(!table->dev_ptr) {
			s=0;
			table=head_table;
		}
        } while(s != Scroll);


        /* now display screen */
        menutype.max_index = line;

	sprintf(string,diag_cat_gets(fdes, SET_SYSXL, MSG_S6));
	menuinfo[line++].text = string;
	string = string + strlen(string)+1;

        rc = diag_display(0x803006, fdes, NULL, DIAG_IO,
				ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menuinfo);
        free ( free_string );

	return(rc);
}

/*  */
/* NAME: disp_ts_menu
 *
 * FUNCTION: Display simultaneous diagnostic selection test menu.
 *
 * NOTES: The selection made is written into the 'selection' argument.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      -1              : error
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *
 */
disp_ts_menu(dev_table_t *table)
{
#define S_ALL	2
#define DEV1	S_ALL + 1
	int	rc;
        int	line = 0;
	int	all_device=0;
	int	nb_selected=0;
        int             dchosen;
	dev_table_t *tb;
        static char            *string;
        static ASL_SCR_INFO *resinfo;
        static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

	if(!resinfo) {
		resinfo = (ASL_SCR_INFO *)
		calloc(1,(num_dev_to_test+5)*sizeof(ASL_SCR_INFO) );
               	if ( resinfo == (ASL_SCR_INFO *) NULL)
                       	return(-1);

		string = malloc(num_dev_to_test*132);
		if ( (string == (char *) NULL) && (num_dev_to_test != 0))
			return(-1);

		/* set the title line in the array   */
		resinfo[line++].text =
			diag_cat_gets(fdes,SET_TSMENU, MSG_TSMENU_A);

		advancedflg=ADVANCED_FALSE;
		resinfo[line++].text = diag_cat_gets(fdes,SET_TSMENU, TSNORMAL);

		resinfo[line++].text = diag_cat_gets(fdes,SET_TSMENU, TSALLDEV);
        	for(tb=table;tb->dev_ptr;tb++)
		{
			resource_desc(string,tb);
			resinfo[line].text = string;
			resinfo[line].non_select = ASL_NO;
			string = string + strlen(string)+1;
			line++;
		}
		/* finally add the last line */
		resinfo[line].text = diag_cat_gets(fdes,
			SET_TSMENU, MSG_TSMENU_B);
                restype.max_index = line;
		resinfo[1].item_flag=' ';
	}
	do
	{
		resinfo[S_ALL].item_flag = all_device?'*':' ';
		for(dchosen=DEV1,tb=table;tb->dev_ptr;dchosen++,tb++)
		{
			resinfo[dchosen].item_flag=tb->selected?'*':' ';
		}
		rc = diag_display(0x803002, fdes, NULL, DIAG_IO,
			ASL_DIAG_LIST_COMMIT_HELP_SC,&restype, resinfo);
		if ( rc == DIAG_ASL_ENTER) {
                	dchosen = DIAG_ITEM_SELECTED(restype);
			if(dchosen == 1) {
				advancedflg=	advancedflg==ADVANCED_FALSE?
					ADVANCED_TRUE:ADVANCED_FALSE;
				resinfo[1].text = diag_cat_gets(fdes,SET_TSMENU,
					advancedflg?TSADVANCED:TSNORMAL);
			}
			else {
				if(dchosen == S_ALL)
					all_device = all_device?0:1;
				else {
					dchosen -= DEV1;
					tb=table + dchosen;
					tb->selected = tb->selected?0:1;
					nb_selected += tb->selected?1:-1;
				}
			}
		}
		if (rc == DIAG_ASL_HELP )
		{
			diag_hmsg(fdes, HELP_SET, HELP_SELECT, NULL);
			rc = DIAG_ASL_ENTER;
		} 
		if(rc == DIAG_ASL_COMMIT && all_device == 0 && nb_selected==0) {
			rc = DIAG_ASL_ENTER;
		}
	} while(rc == DIAG_ASL_ENTER);
	if(rc != DIAG_ASL_COMMIT)	return(rc);

	for(dchosen=DEV1,tb=table;all_device && tb->dev_ptr;dchosen++,tb++)
	{
		tb->selected=TRUE;
	}

	return(rc);
}
/*  */
/* NAME: disp_tm_menu
 *
 * FUNCTION: Display the Test Method Selection menu. Return the
 *              item selected.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *
 */

extern ASL_SCR_TYPE dm_menutype;

disp_tm_menu(int * selection)
{
        int     rc;
        static struct msglist menuTMETHOD[] = {
                {SET_TMETHOD, MSG_TMETHOD_0, },
                {SET_TMETHOD, MSG_TMETHOD_1, },
                {SET_TMETHOD, MSG_TMETHOD_2, },
                {SET_TMETHOD, MSG_TMETHOD_E, },
                (int)NULL,
        };

        *selection = LOOPMODE_NOTLM;

	if( loopmode == FALSE)	return(DIAG_ASL_COMMIT);
        rc = diag_display(0x803009, fdes, menuTMETHOD, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                NULL, NULL);

        if( rc == DIAG_ASL_COMMIT )
                if (DIAG_ITEM_SELECTED(dm_menutype) == 2)
                        *selection = LOOPMODE_ENTERLM;

        return(rc);
}

/*  */
/* NAME: disp_sysx_results
 *
 * FUNCTION: display list of devices tested with an * aside those with errors
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */

int disp_sysx_results (void)
{
	dev_table_t	*table;
        int             line=0;
	int 		selection;
        int             rc=DIAG_ASL_COMMIT;
        char            *dname;
        char            *string;
        char            *free_string;
	ASL_SCR_INFO    *resinfo;
	static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        resinfo = (ASL_SCR_INFO *)
                calloc(1,(num_dev_to_test+5)*sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        free_string = string = (char *)malloc(num_dev_to_test*132);
        if ( string == (char *) NULL)
                return(-1);

        /* set the title line in the array  */
	resinfo[line++].text = diag_cat_gets(fdes, SET_RESULTS, MSG_RES_2);

        /* read the entries from table, get the DName, location, and
           description and put them in the array        */
	table = head_table;
        while (table->dev_ptr) {
		if(!table->selected) {
			table++;
			continue;
		}
		resource_desc(string, table);
		if (table->error_count)
			resinfo[line].item_flag = '*';
               	resinfo[line++].text = string;
		string = string + strlen(string)+1;
		table++;
        }

        /* finally add the last line    */
        resinfo[line].text = diag_cat_gets(fdes, SET_RESULTS, MSG_RES_3);

        /* now display screen */
        restype.max_index = line;
	while (!menu_exit_key && !menu_cancel_key) {
        	rc = diag_display(0x803005, fdes, NULL, DIAG_IO,
			    ASL_DIAG_LIST_CANCEL_EXIT_SC, &restype, resinfo);

		table = head_table; /* restore table */
		if (rc == DIAG_ASL_COMMIT) {
			selection = DIAG_ITEM_SELECTED(restype);
			if (resinfo[selection].item_flag != '*')
				diag_hmsg(fdes, ESET, INFO1);	
			else {
				while (!table->selected) table++;
				while (--selection) {
					table++;
					while (!table->selected) table++;
				}
				disp_fru(table);
			}
		}
	}
        free (free_string);
        free (resinfo);

	return (rc);
}

/*  */
/* NAME: disp_sysx_runtime
 *
 * FUNCTION: display either those devices with errors or a NTF frame
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */

int disp_sysx_runtime(void)
{
	dev_table_t	*table;
	int		total_error=0;
        int             index;
        int             line=0;
        int             rc;
        char            *string;
        char            *free_string;
	ASL_SCR_INFO    *resinfo;
	static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

	/* check for any errors */
	table = head_table;
        while (table->dev_ptr) {
		if (table->error_count != 0)
			total_error++;
		table++;
	}

	/* if no errors - display No Trouble Found */
	if (!total_error)
		return(disp_sysx_ntf());

        /* else - allocate space for enough entries */
        resinfo = (ASL_SCR_INFO *)
                calloc(1, (total_error+5)*sizeof(ASL_SCR_INFO));
        if (resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        free_string = string = (char *)malloc(total_error*132);
        if (string == (char *) NULL)
                return(-1);

        /* set the title line in the array  */
	resinfo[line++].text = diag_cat_gets(fdes, SET_RESULTS, MSG_RES_1);

        /* read the entries from table, get the DName, location, and
           description and put them in the array        */
	table = head_table;
        while (table->dev_ptr) {
		if (table->error_count) {
			resource_desc(string,table);
               		resinfo[line++].text = string;
			string = string + strlen(string)+1;
		}
		table++;
        }

        /* finally add the last line    */
        resinfo[line].text = diag_cat_gets(fdes, SET_RESULTS, MSG_RES_E);

        /* now display screen */
        restype.max_index = line;
        rc = diag_display(0x803003, fdes, NULL, DIAG_IO,
			    ASL_DIAG_ENTER_SC, &restype, resinfo);

        free (free_string);
        free (resinfo);

	return(rc);
}

/*  */
/* NAME: disp_sysx_ntf
 *
 * FUNCTION: display no trouble found menu
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:	DIAG_ASL_EXIT, when exit key entered
 *		DIAG_ASL_CANCEL, when cancel key entered
 *
 */

int disp_sysx_ntf(void)
{
	int	rc;
	dev_table_t	*table;
        int             line=0;
	int 		selection;
        char            *dname;
        char            *string;
        char            *free_string;
	ASL_SCR_INFO    *resinfo;
	static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;


        resinfo = (ASL_SCR_INFO *)
                calloc(1,(num_dev_to_test+5)*sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        free_string = string = (char *)malloc(num_dev_to_test*132);
        if ( string == (char *) NULL)
                return(-1);

        /* set the title line in the array  */
	resinfo[line++].text = diag_cat_gets(fdes, SET_RESULTS, MSG_RES_0);

        /* read the entries from table, get the DName, location, and
           description and put them in the array        */
	table = head_table;
        while (table->dev_ptr) {
		if(!table->selected) {
			table++;
			continue;
		}
		resource_desc(string, table);
               	resinfo[line++].text = string;
		string = string + strlen(string)+1;
		table++;
        }

        /* finally add the last line    */
        resinfo[line].text = diag_cat_gets(fdes, SET_RESULTS, MSG_RES_E);

	/* now display screen */
	restype.max_index = line;
	rc = diag_display(0x803004, fdes, NULL, DIAG_IO,
			  ASL_DIAG_LIST_CANCEL_EXIT_SC, &restype, resinfo);
	return(rc);
}

/*   */
/*
 * NAME: disp_fru
 *
 * FUNCTION: This function will display a Problem Report Frame. The fru 
 * 		bucket data is obtained from the FRUB and FRUs Object
 *		Class.
 *
 * NOTES:
 *	Allocate space for number of possible entries to 
 *		((number of devices tested) * 8) + 5 overhead lines.
 * 	Determine FRUB to use 
 *	Put the title line and SRN line in the display.
 *	Fill in probable causes
 *	Add last line and display screen
 *	Call frub_write to write to error file
 *	Display screen and wait for response
 *	Clear FRUB and FRUs object classes
 *
 * RETURNS:
 *	DIAG_ASL_COMMIT
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 */

int disp_fru(dev_table_t *table)
{

	int		i;
        int             line=3;		/* first line to start with */
        int             rc;
	int 		msg_size;
	int		ftype;
        nl_catd         alt_fdes;       /* if DA has SRN in DA.cat */
        char            *string;
        char            *free_string;
        char            *tmp;
	char		title_buffer[256];
        char            dacat[64];
	char		err_buffer[256];
	char		crit[256];
	char		timestamp[80];
	struct FRUs	*T_FRUs;
	struct listinfo c_info;
	diag_dev_info_t	*fru_device;
	ASL_SCR_INFO    *resinfo;
	static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for enough entries */
        resinfo = (ASL_SCR_INFO *)
                calloc(1,128*sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        free_string = string = (char *)malloc(8192);
        if ( string == (char *) NULL)
                return(-1);

	cfg_fdes = diag_catopen(PORT_CAT,0);

	getdate(timestamp, sizeof(timestamp));
  	msg_size = add_to_problem_menu ( table, string, resinfo, &line,
				     timestamp, &ftype);

	if ( msg_size == -1 )
		return(-1);
	string += msg_size;

        /* set the title line in the array  */
        sprintf(title_buffer, "%s%s",
		  diag_cat_gets(fdes, DIAG_PROB_SETID, DIAG_PROB_TITLE),
		  timestamp);
	resinfo[0].text = title_buffer;

        /* fill in errors per loop  */
        tmp = diag_cat_gets(fdes, DIAG_PROB_SETID, DIAG_PROB_ERR);
        sprintf(err_buffer, tmp, table->loop_count,
			table->error_count);
	resinfo[1].text = err_buffer;

       	resinfo[2].text = diag_cat_gets(fdes, DIAG_PROB_SETID, DIAG_PROB_SRN);

        /* now fill in probable causes  */
        resinfo[line++].text = diag_cat_gets(fdes, DIAG_PROB_SETID, DIAG_PROB_CUS);

	sprintf(crit, "dname = '%s' and ftype = %d", 
			table->dev_ptr->T_CuDv->name, ftype);
	T_FRUs = (struct FRUs *)diag_get_list(FRUs_CLASS,crit,
			&c_info, MAX_EXPECT, 1);
	if ( T_FRUs == (struct FRUs *) -1)
		return (-1);
        for(i=0; i < c_info.num; i++)  {
		sprintf(string, "- %3d%%  %-16.16s  %-16.16s  ", 
				T_FRUs[i].conf,T_FRUs[i].fname, 
				T_FRUs[i].floc);
		tmp = string + strlen(string);
		fru_device = find_dev(T_FRUs[i].fname);

		/* if FRU is not in database */
		if ( fru_device == NULL ) {
			fru_device = find_dev(T_FRUs[i].dname);
                        if(fru_device->T_PDiagDev->Menu & DIAG_DA_SRN) {
                        	sprintf(dacat, "%s.cat", fru_device->T_PDiagDev->DaName);
                                alt_fdes = diag_catopen(dacat,0);
                                copy_text( strlen(string), tmp,
                                          diag_cat_gets(alt_fdes,
					  fru_device->T_PDiagDev->PSet,
                                          T_FRUs[i].fmsg));
                                catclose(alt_fdes);
                        }
                        else
				copy_text(strlen(string), tmp, 
					diag_cat_gets(cfg_fdes,
					fru_device->T_PDiagDev->PSet,
					T_FRUs[i].fmsg));
		}
		else  {
			copy_text( strlen(string), tmp, 
					   fru_device->Text);
		}
                resinfo[line++].text = string;
		string = string + strlen(string)+1;
	}

	/* add last line */
        resinfo[line].text = diag_cat_gets(fdes, SET_INVALID, MSG_E);

        restype.max_index = line;

        /* now display screen */
	if ( line > 3 ) {
		/* write all this to an error log file */
/*		gen_rpt(PROB_REPORT,restype,resinfo);	*/
        	rc = diag_display(PROB_REPORT, fdes, NULL, DIAG_IO, 
						ASL_DIAG_ENTER_SC,
						&restype, resinfo);
	}

	/* free up allocated buffers */
        free ( free_string );
        free ( resinfo );

	catclose(cfg_fdes);
	return (rc);
}

/*   */
/*
 * NAME: add_to_problem_menu 
 *
 * FUNCTION: 
 *
 * NOTES: May be more than one entry
 *
 * RETURNS:
 *	>0 : number of characters in buffer
 *	-1 : error reading FRUB Object Class
 */

int add_to_problem_menu(dev_table_t	*table,
			char 		*buffer,
			ASL_SCR_INFO 	*text,
			int 		*line_num,
			char 		*timestamp,
			int		*ftype)
{
	int 		current_line;
	int 		index;
	int		set;
        nl_catd         alt_fdes;
	char 		crit[256];
        char            dacat[64];
	char		*tmp;
	char		*tmp_buff;
	short		sn, rcode;
	struct FRUB	*T_FRUB;
	struct listinfo f_info;

	/* read entry from FRUB Object Class */
	*ftype = FRUB1;
	sprintf(crit, "dname = '%s' and ftype = %d", 
				table->dev_ptr->T_CuDv->name, FRUB1 );
	T_FRUB = (struct FRUB *)diag_get_list(FRUB_CLASS, crit,
			&f_info, MAX_EXPECT, 1);
	if ( T_FRUB == (struct FRUB *) -1)
		return (-1);

	/* if nothing found, then try the other fru bucket type */
	if ( f_info.num == 0 )  {
		*ftype = FRUB2;
		sprintf(crit, "dname = '%s' and ftype = %d", 
				table->dev_ptr->T_CuDv->name, FRUB2 );
		T_FRUB = (struct FRUB *)diag_get_list(FRUB_CLASS,crit,
				&f_info, MAX_EXPECT, 1);
		if ( T_FRUB == (struct FRUB *) -1)
			return (-1);
	}

	tmp_buff = buffer;
	for ( index = 0; index < f_info.num; index++)  {

		/* if sn is a -1, treat rcode as a 4 digit hex number 	*/
		if ( T_FRUB[index].sn == -1 ){
			rcode=convert_ffc(T_FRUB[index].rcode);
                	sprintf(tmp_buff,"      %4X:  ", rcode & 0xFFFF);
		/* else default is 3 digits separated by a dash */
		} else {
			sn=convert_ffc(T_FRUB[index].sn);
			if(T_FRUB[index].sn == DC_SOURCE_SOFT)
				rcode=convert_ffc(T_FRUB[index].rcode);
			else
				rcode=T_FRUB[index].rcode;

                	sprintf(tmp_buff,"  %03X-%03X:  ", sn, rcode);
		}
		tmp = tmp_buff + strlen(tmp_buff);

		/* if a controller generated error - set set to 1 */
		if ( T_FRUB[index].sn == DC_SOURCE_SOFT) 
		     	set = 1;
		else
			set = table->dev_ptr->T_PDiagDev->PSet;
                if( (table->dev_ptr->T_PDiagDev->Menu & DIAG_DA_SRN)
                        && (T_FRUB[index].sn != DC_SOURCE_SOFT) )
                {
                        sprintf(dacat, "%s.cat", table->dev_ptr->T_PDiagDev->DaName);
                        alt_fdes = diag_catopen(dacat,0);
                        copy_text( strlen(tmp_buff), tmp,
                                   diag_cat_gets(alt_fdes, set,
                                                 T_FRUB[index].rmsg));
                        catclose(alt_fdes);
                }
                else
			copy_text(strlen(tmp_buff), tmp, 
		          	diag_cat_gets(cfg_fdes, set, T_FRUB[index].rmsg));
		strcpy(timestamp,T_FRUB[index].timestamp);
		current_line = *line_num;
                text[current_line].text = tmp_buff;
		tmp_buff = tmp_buff + strlen(tmp_buff) + 1; 
		(*line_num)++;
        }
	catclose(cfg_fdes);
	if (T_FRUB) diag_free_list(T_FRUB, &f_info);
	return(tmp_buff - buffer);
}
