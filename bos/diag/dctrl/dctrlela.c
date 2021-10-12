static char sccsid[] = "@(#)11	1.13  src/bos/diag/dctrl/dctrlela.c, dctrl, bos411, 9428A410j 7/1/93 13:01:45";
/*
 * COMPONENT_NAME: (CMDDIAG) DIAGNOSTICS
 *
 * FUNCTIONS: dctrlela
 *	      get_enddate
 *	      test_ela_path
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                     

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <nl_types.h>
#include <sys/errids.h>
#include <diag/diag.h>
#include <diag/da.h>
#include <diag/diago.h>
#include <diag/diag_exit.h>
#include <diag/class_def.h>	/* object class data structures	*/ 
#include <diag/tmdefs.h>
#include "dctrl_msg.h"
#include "dctrl.h"

#define DATE_SIZE 11
#define CRITERIA_SIZE 256
#define da_exit_key 	(da_rc.field.user == DA_USER_EXIT)
#define da_cancel_key 	(da_rc.field.user == DA_USER_QUIT)

/* Global Variables */
int			ela_index = 0;
diag_dev_info_ptr_t	*ela_tested; 
char	startdate[DATE_SIZE];		/* mmddHHMMyy */
char	enddate[DATE_SIZE];		/* mmddHHMMyy */
union {
	unsigned	char	all;
	da_returncode_t		field;
} da_rc;

/* External Variables */
extern int			lerror;
extern int			lmflg;
extern diag_dev_info_ptr_t	*All;
extern diag_dev_info_ptr_t	*test_array;
extern int			num_All;
extern int			num_dev_to_test;

/* FUNCTION PROTOTYPES */
int dctrlela(void);
int get_enddate(char *, char *, int);
int test_ela_path(int);

/* Called Functions */
extern diag_dev_info_t *pop_device();

/*  */
/* NAME: dctrlela
 *
 * FUNCTION: Search error log entries for devices to test
 * 
 * NOTES: This unit controls execution of those Diagnostic Applications
 *	associated with the devices found in the error log. It performs
 *	the following functions:
 *	1.  Initialize the starting and ending dates for error log searches.
 *	2.  Searches the error log for matching entries.
 *	3.  When an entry is found, call the DA.
 *
 * RETURNS: 0
 *	DIAG_ASL_CANCEL : 
 *	DIAG_ASL_EXIT   :
 */

int dctrlela(void)
{
	int 	rc = DIAG_ASL_OK;
	int	i, j, i_cnt;
	int	dev_idx[512];
	unsigned dev_err[512];
	int	dev_index;
	char	last_device[NAMESIZE];
	char	*tmp;
	int	opflag;
	struct	errdata 	err_data;
	char	criteria[CRITERIA_SIZE];	/* errpt cmd and options */

	if ( ela_tested == NULL )
		ela_tested = (diag_dev_info_ptr_t *)
			calloc( num_All, sizeof( ela_tested[0] ));
	ela_tested[0] = NULL;
	ela_index = 0;
	lmflg = LOOPMODE_NOTLM;

	/* get the present time - this will be the default ending date */
	get_enddate(startdate, enddate, DATE_SIZE);

	/* make a list of all entries in the error log */
	sprintf( criteria, "-s %s -e %s -T %s -d H",
			    startdate, enddate, PERM );
	opflag = INIT;
	last_device[0] = '\0';
	i = 0;
	while ((i < 512) && error_log_get(opflag, criteria, &err_data))
	  {
	  opflag = SUBSEQ;
	  if ( *((char *)strchr(err_data.resource, '\n')) )
	  	*((char *)strchr(err_data.resource, '\n')) = '\0';
	  for (j = strlen(err_data.resource)-1; j >= 0 ; j--)
	    if (err_data.resource[j] == ' ') err_data.resource[j] = '\0';

	  /* Time saver:  check device name against previous device name */
	  if (!strncmp(last_device, err_data.resource,
			strlen(err_data.resource))) continue;
	  strncpy(last_device, err_data.resource, NAMESIZE);

	  /* if device not found in CuDv - skip */
	  /* (may happen if device was removed) */
	  for (dev_index = 0; dev_index < num_All; dev_index++)
	    if (!strcmp(last_device, All[dev_index]->T_CuDv->name))
	      break;
	  if (dev_index == num_All) continue;

	  /* if device not supported by diagnostics, OR    */
	  /* device is missing, OR device has been deleted */
	  /* from test list, then skip			   */
	  if ((All[dev_index]->T_PDiagDev == NULL) ||
	      (All[dev_index]->T_CuDv->chgstatus == MISSING) ||
	      (All[dev_index]->T_CDiagDev->RtMenu == RTMENU_DDTL)) 
	    continue;

	  dev_idx[i] = dev_index;
	  dev_err[i] = err_data.err_id;
	  i++;
	  }
	i_cnt = i;
	error_log_get(TERMI, NULL, NULL);

	for (i = 0; i < i_cnt; i++)
	  {
	  dev_index = dev_idx[i];

	  /* if device has already been tested - skip   */
	  if (All[dev_index]->flags.device_tested == DIAG_TRUE)
	    continue;

	  /* If device has been replaced, skip */
	  if (dev_err[i] == (unsigned )ERRID_REPLACED_FRU)
	    {
	    All[dev_index]->flags.device_tested = DIAG_TRUE;
	    continue;
	    }

	  rc = test_ela_path(dev_index);
	  if (rc == DIAG_ASL_EXIT) break;
	  }

	/* call disp_fru if any errors */
	if (lerror) disp_fru();

        /* display menu goal if present */
        disp_mgoal();

	/* exit with exit code */
	return(rc);
}

/*  */
/*
 * NAME: get_enddate
 *                                                                    
 * FUNCTION:  	This function obtains the current date and time.
 * 		It is returned in the buffer given as input.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS: 0
 */  

int get_enddate(
	char *start,
	char *end,
	int len)
{
	long	end_time_loc;
	long	start_time_loc;
	char	min[3];
	char	year[3];
	struct 	tm *date;

	/* NOTE: the start and end strings are generated in a	*/
	/* cumbersome manner because the SCCS software gets	*/
	/* confused when certain symbols, e.g. 2/27/93, are present	*/
	/* in the source code.  By splitting up the date conv-	*/
	/* ersions, we avoid this confusion.			*/

	/* get todays date and time. Make sure to include */
	/* errors logged within the past minute.	  */
	end_time_loc = time((time_t *)0) + 60;
	date = localtime(&end_time_loc);
	strftime(end, len, "%m%d%H", date);
	strftime(min, 3, "%M", date);
	strftime(year, 3, "%y", date);
	strcat(end, min);
	strcat(end, year);

	/* get yesterdays date and time.  Subtract a	*/
	/* 24-hour (=86400 seconds) period.		*/
	start_time_loc = end_time_loc - 86400;
	date = localtime(&start_time_loc);
	strftime(start, len, "%m%d%H", date);
	strftime(min, 3, "%M", date);
	strftime(year, 3, "%y", date);
	strcat(start, min);
	strcat(start, year);

	return(0);
}

/*   */
/* NAME: test_ela_path
 *
 * FUNCTION: Test the device and path.
 *
 * NOTES: 
 *
 * RETURNS:
 *	rc:
 *		DIAG_ASL_OK, normal return  
 *		DIAG_ASL_EXIT, when EXIT key entered 
 *		DIAG_ASL_CANCEL, when CANCEL key entered 
 *		-1, error occurred
 */

int test_ela_path(int device)
{

#define USE_DANAME 0

	diag_dev_info_t		*dev_ptr;
	int 			i, next, rc, cdevice;
	
	/* stack the devices to test */
        if (( num_dev_to_test = stack_devices(All, num_All, device)) < 0 ) {
		disp_dc_error( ERROR7, NULL);
		return(-1);
	}

	/* save test array to simplify processing of FRUs */ 
	for ( i = 0; i < num_dev_to_test; i++) {
		ela_tested[ela_index++] = test_array[i];
	}
	ela_tested[ela_index++] = NULL;
	ela_tested[ela_index + 1] = NULL;
 
	/* test path */	
        for (cdevice = 0; cdevice < num_dev_to_test; cdevice++) {
		dev_ptr = pop_device( cdevice );
        	if( ( rc = testdevice( cdevice, USE_DANAME ) ) == -1 )
 			return( -1 );
		if ( da_exit_key )
			return( DIAG_ASL_EXIT );
		if ( da_cancel_key )
			return( DIAG_ASL_CANCEL );
	 	if( dev_ptr->T_CDiagDev->State == STATE_BAD )
			dev_ptr->flags.defective_devfru = DIAG_TRUE;
	}

	return(DIAG_ASL_OK);
}
