static char sccsid[] = "@(#)12	1.17.2.11  src/bos/diag/dctrl/dctrlsup.c, dctrl, bos41J, 9520A_all 5/8/95 09:56:21";
/*
 * COMPONENT_NAME: (CMDDIAG) Diagnostic Controller
 *
 * FUNCTIONS:
 *	stack_devices
 *	pop_device
 *	more_tests
 *	putdainput
 *	clr_class
 *	resource_desc
 *	copy_text
 *	sibling_check
 *	find_dev
 *	find_next_non_dependent
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
 */                                                                   

#include <stdio.h>
#include <nl_types.h>
#include <signal.h>
#include <cf.h>
#include "dctrl_msg.h"
#include "dctrl.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "diag/tmdefs.h"
#include "diag/tm_input.h"
#include "diag/class_def.h"
#include "sys/cfgdb.h"

/* GLOBAL VARIABLES */
#define NOT_DELETED_DTL( VAR1, VAR2 ) 				            \
	(								    \
		( /**/VAR1[/**/VAR2]->T_CDiagDev->RtMenu == RTMENU_DEF )    \
	)

#define SUPPORTS_TESTS( VAR1, VAR2 )					    \
	(								    \
		( /**/VAR1[/**/VAR2]->T_PDiagDev->SupTests | 7 )            \
	)

diag_dev_info_ptr_t     *test_array=NULL;

/* Base system tests	*/
#define NUM_BASE_TESTS 4
char	*base_tests[] = {"dsys",		/* System Planar	*/
			 "dfpp",		/* Floating Point	*/
			 "dmem",		/* Memory		*/
			 "diop"			/* IoPlanar		*/ 
};

#define NUM_BASE_ISA_TESTS 2
char	*base_isa_tests[] = {"dresid",		/* System Planar, Memory*/
			     "dfpp"		/* Floating Point	*/
};

/* EXTERNALLY DEFINED VARIABLES */
extern int diag_mode;           /* diagnostic test mode         */
extern int advancedflg;
extern int exenvflg;
extern int systestflg;
extern int basetestflg;
extern int missingflg;
extern int regressflg;
extern int regr_setup;
extern int elaflg;
extern int deviceflg;
extern int moretesting;
extern int lmflg;
extern short lcount;
extern int lerror;
extern int consoleflg;
extern int num_All;
extern int volume_to_test;
extern char startdate[];
extern char enddate[];
extern diag_dev_info_t         *Top;
extern diag_dev_info_ptr_t     *DSMenu;
extern diag_dev_info_ptr_t     *New;
extern diag_dev_info_ptr_t     *All;
extern char *dadir;

/* CALLED FUNCTIONS */
extern char *substrg();
diag_dev_info_t  *pop_device();
diag_dev_info_t  *find_dev();
/*  */
/*
 * NAME: stack_devices 
 *
 * FUNCTION: create array of indexes into List pointing to devices
 *		to test.  The initial device, selected, is contained 
 *              within DSMenu. If diag was entered with the -d option to
 *		test a particular device, then selected is contained
 *		within All. 
 *
 * RETURNS:
 *	non-zero positive value indicating number of devices to test
 *	-1 if error occurred
 */

stack_devices(List, num_List, selected)
diag_dev_info_ptr_t	*List;
int			num_List;
int			selected;
{
	int i;
	int curr; 
	int index;
	int dindex;
	int parent_index;
	int enclosure_index;
	int counter;
	int da_base_cnt;
	char *parent;
	char *device;
	char *enclosure;
	char **da_base_ptr;
	char file[255];

	/* allocate space for test array if not already done */
	if ( test_array == NULL )
		test_array = (diag_dev_info_ptr_t *)
			calloc(num_All+1,sizeof(test_array[0]));

	if ( systestflg == SYSTEM_TRUE ) { 
		for ( dindex=0, index=0; dindex < num_List; dindex++) {
		    List[dindex]->flags.device_tested = DIAG_FALSE;
		    if((List[dindex]->T_CDiagDev->RtMenu == RTMENU_DEF) &&
			(List[dindex]->T_CuDv->chgstatus != MISSING) &&
			/* skip non available processors */
			( strncmp(List[dindex]->T_CuDv->name, "proc", 4) ||
			  List[dindex]->T_CuDv->status == AVAILABLE ) )
				test_array[index++] = List[dindex];
		}
		test_array[index] = 0;
		return(index);
	}
	else if ( basetestflg == SYSTEM_TRUE ) {
		/* stack up base system da's */
		if ( has_isa_capability() ) {
			da_base_cnt = NUM_BASE_ISA_TESTS;
			da_base_ptr = base_isa_tests;
		}
		else {
			da_base_cnt = NUM_BASE_TESTS;
			da_base_ptr = base_tests;
		}

		for ( i=0, index=0; i < da_base_cnt; i++ )
			for ( dindex=0; dindex < num_List; dindex++) {
				if( (List[dindex]->T_CDiagDev->RtMenu == 
								RTMENU_DEF) &&
				    (List[dindex]->T_CuDv->chgstatus !=
						 MISSING)&&
				    (!strncmp(List[dindex]->T_PDiagDev->DaName,
						da_base_ptr[i], 4)) &&
				    /* skip non available processors */
				    ( strncmp(List[dindex]->T_CuDv->name,
						"proc", 4) ||
				      List[dindex]->T_CuDv->status ==
						AVAILABLE ) ) {
					List[dindex]->flags.device_tested = 
								DIAG_FALSE;
					test_array[index++] = List[dindex];
				}
			}	
		test_array[index] = 0;
		return(index);
	}

	/* Translate selected into a pointer in List if option -d */
	/* was not entered on command line.                       */
	/* also do not do this if in missingopt or ela testing    */
	if ( !deviceflg && !missingflg && !elaflg ) {
        	for ( index=0; index < num_List; index++ )
			if ( !strcmp(DSMenu[selected]->T_CuDv->name,
                             	     List[index]->T_CuDv->name) )
				break;
        	if ( index == num_List )
			return(0);
		else selected = index;
	}
	/* Enter the selected device as the first device to test */
	test_array[0] = List[selected];
	test_array[1] = 0;
	List[selected]->flags.device_tested = DIAG_FALSE;
	counter = 1;

	/* only stack one device if loopmode was chosen */
	if (lmflg != LOOPMODE_NOTLM && List[selected]->T_PDiagDev != NULL &&
				    strlen(List[selected]->T_PDiagDev->DaName))
		return(counter);

	/* if supposed to test sibling next                   */
	/*        search database for device with same parent */
	/*        also try to get non-base sibling */
	curr = selected;
	if (List[curr]->T_PDiagDev->DNext == DIAG_SIB) {
		index = 0;
		parent = List[curr]->T_CuDv->parent;
		while ( strcmp(parent, List[index]->T_CuDv->parent) ||
		        !strcmp(
			     substrg(PCLASS,List[index]->T_CuDv->PdDvLn_Lvalue),
			     CLASS_DISK)||
	   			 ( curr == index) )
			if (++index >= num_List)
				break;
		/* if a valid index to device - then add it in */
		if (index < num_List) {
			if( (List[index]->T_PDiagDev->SupTests | 7 ) &&
		            (List[index]->T_CDiagDev->RtMenu == RTMENU_DEF) &&
			    (List[index]->T_CuDv->chgstatus != MISSING)) {
				test_array[counter] = List[index];
				List[index]->flags.device_tested =
					 DIAG_FALSE;
				counter++;
			}
		}
	}

	/* while the device has a parent to test              */	
	/*    check for device to test next		      */
	/*    if parent                                       */
	/*        search database for parent as Device Name   */

	while ( strcmp(List[curr]->T_CuDv->parent,"")) { 
		parent_index = 0;
		parent = List[curr]->T_CuDv->parent;
		device = List[curr]->T_CuDv->name;

		/* find index into array for parent */
		while (strcmp(parent, List[parent_index]->T_CuDv->name))
			if (++parent_index >= num_List)
				return(counter);

		/* add in the parent to the test array if device is supported */
		if (List[parent_index]->T_PDiagDev) {
			if ( SUPPORTS_TESTS( List, parent_index)  && 
			     NOT_DELETED_DTL( List, parent_index) ){ 
				test_array[counter] = List[parent_index];
				List[parent_index]->flags.device_tested = 
					DIAG_FALSE;
				counter++;
				if (lmflg != LOOPMODE_NOTLM && 
				 strlen(List[parent_index]->T_PDiagDev->DaName))
					return(counter);
			}	
		}
		curr = parent_index;
	}
	test_array[counter] = 0;
	return ( counter );
}
/*   */
/*
 * NAME: pop_device 
 *
 * FUNCTION: returns pointer to next device in test array list
 *
 * RETURNS:
 *	diag_dev_info_t *device - next device to test
 *
 */

diag_dev_info_t *
pop_device(cdevice)
int 			cdevice;
{
	return ( test_array[cdevice] );
}
/*   */
/*
 * NAME: more_tests 
 *
 * FUNCTION: check if more testing is needed due to some
 *		resources that need to be freed.
 *
 * NOTES: Check the da_exit_tests to see what the DA actually tested. If
 *        less than the highest, then display more testing needed menu.
 *
 * RETURNS:
 *     0 - if all supported tests were run or in system test mode
 *     1 - if less than all supported tests were run
 */

more_tests(device)
diag_dev_info_t  *device;
{
	moretesting = DIAG_FALSE;

	/* if in system test - just return without checking */
	if (systestflg)
		return(0);

	/* if device open error - present menu */
	if( device->flags.device_driver_err == DIAG_TRUE ) {
		moretesting = DIAG_TRUE;
		return(1);
	}

	/* if state was good and no tests were run - return */
	if(device->T_CDiagDev->State == STATE_GOOD && 
	   device->T_CDiagDev->TstLvl == TSTLVL_NOTEST )
		return(0);

	/* if device just supports MORP - return without checking */
	if( (device->T_PDiagDev->SupTests & 7) == 0)
		return(0);

	/* else check each level */
	if(device->T_PDiagDev->SupTests & device->T_CDiagDev->TstLvl
						& TSTLVL_FULL) 
		return(0);
	if(device->T_PDiagDev->SupTests & device->T_CDiagDev->TstLvl
						& TSTLVL_SUB) 
		return(0);
	if(device->T_PDiagDev->SupTests & device->T_CDiagDev->TstLvl
						& TSTLVL_SHR) 
		return(0);
	moretesting = DIAG_TRUE;
	return(1);
}

/*   */
/* NAME: putdainput
 *
 * FUNCTION: This unit populates the TMInput Object Class with data used by
 *		the diagnostic applications.
 * 
 * RETURNS:
 *	0  - put successful
 *	-1 - put unsuccessful
 */

putdainput(cdevice)
int cdevice;
{

	diag_dev_info_t *device;
	diag_dev_info_t *last_device = NULL;
	int 		rc;
	static struct	TMInput	T_TMInput;
	diag_dev_info_t *child_dev;
	diag_dev_info_t *parent_dev;

	/* make sure this class is empty first */
	if ( (rc = diag_rm_obj(TMInput_CLASS, "" )) != -1)   {

                device = pop_device(cdevice);
		if ( cdevice > 0 )
                	last_device = pop_device(cdevice - 1);

		if (regressflg) 
		  T_TMInput.exenv = EXENV_REGR;	/* Regression Testing	*/
		else
		  T_TMInput.exenv = exenvflg;	/* Standard Exec. Envir.*/

		T_TMInput.advanced = advancedflg;

		/* If missingopt testing - perform option checkout mode */
		if ( missingflg )
			T_TMInput.system = SYSTEM_FALSE;

		/* else if base system tests perform system checkout  */
		if ( basetestflg == 1 )
			T_TMInput.system = systestflg;

		/* else if device to test is a sibling of the last device */
		/*	perform only option checkout tests                */
		else if ( (last_device != NULL) && !elaflg &&
			  !(strcmp(device->T_CuDv->parent,
					last_device->T_CuDv->parent)))
			T_TMInput.system = SYSTEM_TRUE;
						
		/* else if testing the ioplanar                           */
		/*	perform only non-interactive tests                */
		else if ( !(strncmp(device->T_PDiagDev->DaName, "diop", 4)) &&
				!deviceflg )
			T_TMInput.system = SYSTEM_TRUE;
						
		else if ( !missingflg )	
			T_TMInput.system = systestflg;

		if ( (exenvflg == EXENV_CONC) && !missingflg &&
		     (device->T_PDiagDev->Conc == DIAG_NO) ) 
			T_TMInput.dmode = DMODE_ELA;
		else if (regressflg)
			T_TMInput.dmode = regr_setup;
		else
			T_TMInput.dmode = diag_mode;

		T_TMInput.loopmode = lmflg;
		T_TMInput.lcount = lcount;
		T_TMInput.lerrors = lerror;
		T_TMInput.console = consoleflg;
		T_TMInput.pid = 0;

		/* set up defaults for children */
		T_TMInput.state1 = T_TMInput.state2 = 0;
		T_TMInput.child1[0] = T_TMInput.child2[0] = '\0';
		T_TMInput.childloc1[0] = T_TMInput.childloc2[0] = '\0';

		/* enter in ela search criteria */
		if ( ( T_TMInput.dmode == DMODE_ELA ) ||
		     ( T_TMInput.dmode == DMODE_PD ) ) {
                        get_enddate(startdate, enddate, 11);
                        sprintf(T_TMInput.date,"-s %s -e %s",
                                startdate, enddate );
                }


		strcpy(T_TMInput.dname, device->T_CuDv->name);
		/* location is expanded for ports	*/
		strcpy(T_TMInput.dnameloc, device->T_CuDv->location);

		/* enter parent information */
		strcpy(T_TMInput.parent, device->T_CuDv->parent);
		parent_dev = find_dev(device->T_CuDv->parent);
		if ( parent_dev != NULL )
			strcpy(T_TMInput.parentloc, 
				parent_dev->T_CuDv->location);
	
	 	/* if Option checkout - get status of 2 children, but only */
    		/* if they were just tested.  ie. get from test array */
		if ( systestflg == SYSTEM_FALSE && missingflg == 0){
		     if ( cdevice > 0 ){
                       	child_dev = pop_device(cdevice-1);
			/* if parents match, must be testing siblings */
			/* so do not fill out child status	      */
			if (strcmp(child_dev->T_CuDv->parent,  
				     device->T_CuDv->parent)){
				strcpy(T_TMInput.child1,
				       child_dev->T_CuDv->name);
				T_TMInput.state1 =
					child_dev->T_CDiagDev->State;
				strcpy(T_TMInput.childloc1,
					child_dev->T_CuDv->location);
                        }
                     }
		     if ( cdevice > 1 ){
                       	child_dev = pop_device(cdevice-2);
			/* if parents match, must be testing siblings */
			/* so do not fill out child status	      */
			if (strcmp(child_dev->T_CuDv->parent,  
				     device->T_CuDv->name)){
				strcpy(T_TMInput.child2,
				       child_dev->T_CuDv->name);
				T_TMInput.state2 =
					child_dev->T_CDiagDev->State;
				strcpy(T_TMInput.childloc2,
					child_dev->T_CuDv->location);
                        }
                     }
		}

		rc = diag_add_obj(TMInput_CLASS,&T_TMInput);
	}
	return( (rc == -1) ? -1: 0 );
}

/*   */
/*
 * NAME: sibling_check   
 *
 * FUNCTION: Compares parents of two devices and returns a 0 if equal 
 *
 * NOTES:
 *
 * RETURNS:
 *	0 - if two devices are siblings
 */

sibling_check(first_dev, second_dev)
diag_dev_info_t *first_dev;
diag_dev_info_t *second_dev;
{
	if ((first_dev == (diag_dev_info_t *) NULL) || 
		(second_dev == (diag_dev_info_t *) NULL)) 
		return(1);

	/* if the two are siblings - return 0 */
	return(strcmp(first_dev->T_CuDv->parent,second_dev->T_CuDv->parent));
}

/*   */
/* NAME: find_next_non_dependent   
 *
 * FUNCTION: returns an index into List, which identifies the next_device 
 *           that is not dependent on starting_device. 
 *
 * NOTES: 
 *
 * RETURNS:
 *	0
 *
 */

find_next_non_dependent( current, next, List, max_devices )
int current;
int *next;
diag_dev_info_ptr_t *List;
int max_devices;
{

 	for (*next = current+1; *next < max_devices; (*next)++ ){
		/* does next device have the same parent */
		if (!strcmp(List[*next]->T_CuDv->parent,
			    List[current]->T_CuDv->parent))
				break;
		/* is next device a child */
		else if (!strcmp(List[*next]->T_CuDv->parent,
				 List[current]->T_CuDv->name)){
                        find_next_non_dependent(*next,next,List,max_devices);
			(*next)--;
			continue;
                }
                else break;
	}

	if ( *next > max_devices )
		*next = max_devices;
	return(0);

}
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

diag_dev_info_t *
find_dev(device_name)
char	*device_name;
{
	int	i;

	for ( i = 0; i < num_All; i++) {
		if(!strcmp(device_name,All[i]->T_CuDv->name))
			break;
	}

	return((i==num_All) ? (diag_dev_info_t *)NULL : All[i]);
}
/*   */
/* NAME: clr_class
 *
 * FUNCTION: This function clears Object Classes used by the DA's. 
 * 
 * RETURNS:
 *	0  - clear successful
 *	-1 - clear unsuccessful
 */

clr_class(classname)
char *classname;
{
	register int rc;

	if( !strcmp(classname,"FRUB") )
		rc = diag_rm_obj(FRUB_CLASS, "");
	else if( !strcmp(classname,"FRUs") )
		rc = diag_rm_obj(FRUs_CLASS, "");
	else if( !strcmp(classname,"MenuGoal") )
		rc = diag_rm_obj(MenuGoal_CLASS, "");
	else if( !strcmp(classname,"DAVars") )
		rc = diag_rm_obj(DAVars_CLASS, "");
	else if( !strcmp(classname,"TMInput") )
		rc = diag_rm_obj(TMInput_CLASS, "");
	else
		rc = -1;

	return( (rc >= 0) ? 0: -1 );

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

resource_desc(string, line_type, dev_ptr) 
char *string;
int  line_type;
diag_dev_info_t *dev_ptr;
{

	char	*tmp;

	switch ( line_type ) {
		case DIAG_SEL_TYPE:
			sprintf(string, "%-16s %-16.16s ", 
				dev_ptr->T_CuDv->name,
				dev_ptr->T_CuDv->location);
			break;
		case NTF_TYPE:
			sprintf(string, "- %-16s %-16.16s", 
				dev_ptr->T_CuDv->name,
				dev_ptr->T_CuDv->location);
			break;
	}

	tmp = string + strlen(string);
	copy_text( strlen(string), tmp, dev_ptr->Text);
	return(0);
}

/*   */
/* NAME: copy_text
 *
 * FUNCTION: 
 * 
 * NOTES:
 *
 * RETURNS: 0
 */

copy_text( string_length, buffer, text )
int	string_length;	/* current length of text already in buffer	*/ 
char	*buffer;	/* buffer to copy text into	*/
char 	*text;		/* text to be copied 		*/
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


