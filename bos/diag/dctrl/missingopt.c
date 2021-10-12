static char sccsid[] = "@(#)15	1.16.1.9  src/bos/diag/dctrl/missingopt.c, dctrl, bos41J, 9514A_all 3/29/95 17:38:42";
/*
 * COMPONENT_NAME: (CMDDIAG) Diagnostic Controller
 *
 * FUNCTIONS: testmissing
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include "dctrl_msg.h"
#include "dctrl.h"
#include "diag/dcda_msg.h"
#include "diag/diag.h"
#include "diag/da.h"
#include "diag/tmdefs.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "diag/class_def.h"

/* GLOBAL VARIABLES */
#define USE_DANAME 	0
#define USE_ENCLDANAME 	1
#define FRU_REPORTED   -1
#define NO_NEW_DEVICES -2

#define da_nocont     	( da_rc.field.more == DA_MORE_NOCONT )
#define da_exit_key 	( da_rc.field.user == DA_USER_EXIT )
#define da_cancel_key 	( da_rc.field.user == DA_USER_QUIT )
#define PARENT_CHILD( VAR1, VAR2 )					      \
	( !strcmp(All[/**/VAR1]->T_CuDv->name,All[/**/VAR2]->T_CuDv->parent) )
#define DETECTABLE( VAR1, VAR2 )					      \
	( All[/**/VAR1]->T_Pdv->detectable == /**/VAR2 )
#define CHGSTATUS( VAR1, VAR2 )						      \
	( All[/**/VAR1]->T_CuDv->chgstatus == /**/VAR2 )
#define SETSTATUS( VAR1, VAR2 )						      \
	( All[/**/VAR1]->T_CuDv->chgstatus = /**/VAR2 )

int 			num_new_devices;
diag_dev_info_ptr_t     *New;   
diag_dev_info_ptr_t	*missing_tested;
int			missing_index = 0;
union {
	unsigned char all;
	da_returncode_t	field;
} da_rc;

/* LOCAL FUNCTION PROTOTYPES */
DIAG_ASL_RC testmissing(void);
DIAG_ASL_RC missing_menu(int, int *);
DIAG_ASL_RC test_path(int, int *);
DIAG_ASL_RC test_ms1_opt(int);
DIAG_ASL_RC test_ms2_opt(int device);
void update_do_not_display(int, int, diag_dev_info_ptr_t *, int);
void update_delete(int, int, diag_dev_info_ptr_t *, int);
void update_parent(int, int, diag_dev_info_ptr_t *, char *);
void update_chgstatus(int, int, diag_dev_info_ptr_t *, int);
DIAG_ASL_RC disp_new(int, int *);
DIAG_ASL_RC reconfig(int, int);
void flag_selection(int, int, int, int, ASL_SCR_INFO *);
void check_frub(void);
int add_missing_fru(int, char *, int, int);
DIAG_ASL_RC check_new_devices(void);
DIAG_ASL_RC new_menu(int *);
void update_device_database(void);

/* EXTERNALLY DEFINED VARIABLES */
extern int 	num_Top;
extern int 	num_All;
extern int 	exenvflg;
extern int 	lmflg;
extern int 	diag_mode;
extern int	diag_ipl_source;
extern nl_catd	fdes;
extern int 	num_dev_to_test;	/* number of devices to test */
extern diag_dev_info_t  *Top;
extern diag_dev_info_ptr_t	*test_array;
extern diag_dev_info_ptr_t	*All;
extern struct Class CuDv_CLASS[];
extern char *diag_cat_gets();

/* CALLED FUNCTIONS */
int		 disp_mgoal();
char		 *malloc();
diag_dev_info_t  *pop_device();
diag_dev_info_ptr_t  *gen_ds_menu();
diag_dev_info_ptr_t  *generate_All();

/*   */
/*
 * NAME: testmissing()
 *
 * FUNCTION: 
 *
 * NOTES:
 *
 *	Obtain All devices from Customized
 *
 *	For each device in MISSING list 
 *		Present MISSING DEVICE MENU #2
 *		CASE "The resource has NOT been removed,moved,or turned off"
 *			For each device in test path for MISSING Device
 *				Execute DA
 *				If status is bad
 *					set defective-devfru for device
 *					For each device attached to defective
 *						set do-not-present-flag
 *			If DA for MISSING Device includes an EnclDaName 
 *				Execute EnclDaName using Missing device as dname
 *				If status is bad
 *					set defective-devfru for device
 *					For each device attached to defective
 *						set do-not-present-flag
 *			Else If DA for MISSING Device supports MS1
 *				Execute DA
 *				If Menu Goal
 *					Present Menu Goal
 *				If FRU Bucket presented
 *					set defective-devfru for device
 *					For each device attached to defective
 *						set do-not-present-flag
 *			else 
 *				set defective-flag for device
 *				For each device attached to defective
 *					set do-not-present-flag
 *		CASE "The resource has been removed and should be removed 
 *							from configuration"
 *			If DA for MISSING Device supports MS2
 *				Execute DA
 *			else 
 *				set delete-flag for device
 *		CASE "The device has been moved to another location"
 *			Display list of NEW devices that are of same type
 *			Set update database flag
 *		CASE "The resource has been turned off and should be removed 
 *							from configuration"
 *			set delete-flag for device
 *		CASE "The resource has been turned off and should remain in
 *							in configuration"
 *			do nothing
 *
 *	If any devices have defective-flag set
 *		if defective flag-fru set , use FRU bucket
 *		else generate predefined SRN
 *		display problem report menu
 *
 *	For each device flagged to be deleted
 *		Execute the devices Unconfigure method if previously configured
 *		Execute the devices Undefine method to delete from database
 *
 *	Display a list of all NEW devices
 *		CASE "List is correct"
 *			do nothing
 *		CASE "List is not correct"
 *			present predefined SRN
 *
 * RETURNS:
 *      0:  No Trouble Found
 *      1:  A problem was detected
 *     -1:  Cancel was entered
 *     -2:  Esc was entered
 */
/*   */
DIAG_ASL_RC testmissing(void)
{
	int			rc, device, selection, new_device, state;   
	int			next;

	lmflg = LOOPMODE_NOTLM;

	/* remember what was tested so that the FRUs cans be isolated */
	if ( missing_tested == NULL )
		missing_tested = (diag_dev_info_ptr_t *)
			calloc( num_All, sizeof( missing_tested[0] ));
	missing_tested[0] = NULL;

	/* 
	 * for each MISSING device, present a menu that asks the user if 
	 * the device was moved, removed, or turned off 
	 */
	for ( device = 0; device < num_All; device++ ) {

		diag_mode = DMODE_PD;

		/* find a missing device */
		for ( ; device < num_All; device++ )
			if CHGSTATUS( device, MISSING )
				break;

		/* has it already been processed */
		if ( (device < num_All)                                &&
		     (All[device]->flags.do_not_disp_miss==DIAG_FALSE) ) {

			/* display the missing device menu */
			rc = missing_menu( device, &selection ); 
			switch( rc ){
				case DIAG_ASL_EXIT :
				case DIAG_ASL_CANCEL :
					return( rc );
				default :
					break;
			}
			switch ( selection ) { 
				/* "Resource HAS not been removed, ... " */
				case 1:
					rc = test_path(device, &state); 
					if ( !((rc == DIAG_ASL_EXIT) ||
					       (rc == DIAG_ASL_CANCEL)) &&
					     ( state == STATE_GOOD ) )
						rc = test_ms1_opt( device ); 
					break;
				/* "Resource HAS been removed, ... " */
				case 2:
					rc = test_ms2_opt(device); 
					break;
				/* "Resource HAS been moved to another loc */
				case 3:
					rc = disp_new(device, &new_device);
					if (!( ( rc == DIAG_ASL_EXIT   )  ||
					       ( rc == DIAG_ASL_CANCEL )  ||
					       ( rc == FRU_REPORTED    ) ) )
					       	rc=reconfig(device,new_device);
					break;
				/* "Resource HAS been turned off ...removed.. */
				case 4:
					find_next_non_dependent( device, &next,
								 All, num_All );
					update_delete( device, next, 
							       All, DIAG_TRUE );
					break;
				/* "Resource HAS been turned off ...remain.. */
				default:
					break;
			}
		}
		
		if (( rc == DIAG_ASL_EXIT ) || (rc == DIAG_ASL_CANCEL))
			break;
	}

	/* report any problems found	*/
	check_frub();
	disp_mgoal();

	if (( rc != DIAG_ASL_EXIT ) && (rc != DIAG_ASL_CANCEL)){
		/* check for any devices which need to be updated */
		update_device_database();

		/* check for any newly added resources	*/
		if ( check_new_devices() == DIAG_ASL_OK )
			update_device_database();
	}

	return( rc );
}

/*   */
/*
 * NAME: missing_menu
 *
 * FUNCTION: Display the Missing Device Menu 
 *
 * NOTES: Display menu if devices' delete_flag is not set 
 *
 * RETURNS:
 *
 */

DIAG_ASL_RC missing_menu(int device,
		    int *selection)
{
	int 	rc;
	char	*buffer;
	char	devstring[1024];
	static struct  msglist missing[] = {
			{ SET_MISSING, MSG_MISSING, },
			{ SET_MISSING, MSG_MISSING_1, },
			{ SET_MISSING, MSG_MISSING_2, },
			{ SET_MISSING, MSG_MISSING_3, },
			{ SET_MISSING, MSG_MISSING_4, },
			{ SET_MISSING, MSG_MISSING_5, },
			{ SET_MISSING, MSG_MISSING_E, },
			(int)NULL
	};
	static ASL_SCR_INFO missinfo[ DIAG_NUM_ENTRIES(missing) ];
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	buffer = malloc(256);
	rc = diag_display(NULL, fdes, missing, DIAG_MSGONLY, NULL,
				  &menutype, missinfo);
	resource_desc(buffer,NTF_TYPE,All[device]); 
	sprintf(devstring, missinfo[6].text, buffer);
	free(missinfo[6].text);
	missinfo[6].text = devstring;
	rc = diag_display(MISS_RESOURCE, fdes, NULL, DIAG_IO,
				ASL_DIAG_LIST_CANCEL_EXIT_SC, 
				&menutype, missinfo);

	free(buffer);

	switch( rc ){
		case DIAG_ASL_EXIT :
		case DIAG_ASL_CANCEL :
			return( rc );
		case DIAG_ASL_COMMIT :
			*selection = DIAG_ITEM_SELECTED(menutype);
			return( DIAG_ASL_OK );
		default :
			return( -1 );
	}
}

/*   */
/* NAME: test_path
 *
 * FUNCTION: Test the path to the missing device. Start with the parent and
 *		move up the chain.
 * NOTES: 
 *
 * RETURNS:
 *	rc:
 *		DIAG_ASL_OK, normal return  
 *		DIAG_ASL_EXIT, when EXIT key entered 
 *		DIAG_ASL_CANCEL, when CANCEL key entered 
 *
 *	state:  STATE_GOOD, no defective devices were found
 *		STATE_BAD, a defective device was found
 */

DIAG_ASL_RC test_path(int device,
		      int *state)
{

	diag_dev_info_t		*dev_ptr;
	int 			i, next, rc, cdevice;
	
	/* find device's parent index value */
	for( i = device-1; i >= 0; i--)
		if (PARENT_CHILD( i, device ))
			break;

	/* if not found - assume path cannot be tested */
	*state = STATE_GOOD;
	if ( i < 0 )
		return(DIAG_ASL_OK);

	/* stack the devices to test */
        if (( num_dev_to_test = stack_devices(All, num_All, device)) < 0 ) {
		disp_dc_error( ERROR7, NULL);
		return(DIAG_ASL_FAIL);
	}

	/* save test array to simplify processing of FRUs */ 

	for ( i = 0; i < num_dev_to_test; i++) {
		missing_tested[missing_index++] = test_array[i];
	}
	missing_tested[missing_index++] = NULL;
	missing_tested[missing_index + 1] = NULL;

	/* test path */	
        for (cdevice = 1; cdevice < num_dev_to_test; cdevice++) {
		dev_ptr = pop_device(cdevice);
        	if ((rc = testdevice(cdevice, USE_DANAME)) == DIAG_ASL_FAIL)
 			return(DIAG_ASL_FAIL);
		if (da_exit_key)
			return( DIAG_ASL_EXIT );
		if (da_cancel_key)
			return( DIAG_ASL_CANCEL );
	 	if (dev_ptr->T_CDiagDev->State == STATE_BAD) {
			*state = STATE_BAD;
			dev_ptr->flags.defective_devfru = DIAG_TRUE;
			/* find defective device in All */
			for (i = 0; i < device; i++)
				if (dev_ptr == All[i])
					break;
			find_next_non_dependent( i, &next, All, num_All );
			update_do_not_display( i, next, All, DIAG_TRUE );
		}
		if (da_nocont) 
			break;
			
	}

	/* if path is good and device supports an enclosure da */
	cdevice = 0;
	if ( All[device]->T_PDiagDev && ( *state == STATE_GOOD ) && 
	     strlen(All[device]->T_PDiagDev->EnclDaName)) {
		dev_ptr = pop_device(cdevice);
        	if ((rc = testdevice(cdevice, USE_ENCLDANAME)) == DIAG_ASL_FAIL)
			return(DIAG_ASL_FAIL);
		if ( da_exit_key )
			return(DIAG_ASL_EXIT);
		if ( da_cancel_key )
			return( DIAG_ASL_CANCEL );
		if ( dev_ptr->T_CDiagDev->State == STATE_BAD ) {
			*state = STATE_BAD;
			dev_ptr->flags.defective_devfru = DIAG_TRUE;
			find_next_non_dependent( device, &next, All, num_All );
			update_do_not_display( device, next, All, DIAG_TRUE );
		}
	}
 
	return(DIAG_ASL_OK);
}

/*   */
/* NAME: test_ms1_opt
 *
 * FUNCTION: Execute DA with the MS1 option bit set
 *
 * NOTES: 
 *
 * RETURNS:
 *		DIAG_ASL_OK, normal return  
 *		DIAG_ASL_EXIT, when EXIT key entered 
 *		DIAG_ASL_CANCEL, when CANCEL key entered 
 */
DIAG_ASL_RC test_ms1_opt(int device)
{
#define FIRST 0
	int start, rc, next;
	int count;

	if ( All[device]->T_PDiagDev &&
	    (All[device]->T_PDiagDev->SupTests & SUPTESTS_MS1) ) {
		/* set up some defaults for the TMInput Class */
		diag_mode = DMODE_MS1;
        	count=stack_devices(All, num_All, device);
		if(num_dev_to_test<=0)
			num_dev_to_test = count;

		rc = testdevice(FIRST,USE_DANAME);
		if ( da_exit_key )
			return( DIAG_ASL_EXIT );
		if ( da_cancel_key )
			return( DIAG_ASL_CANCEL );

		/* display a menu goal if there is one */
		disp_mgoal();

		if( All[device]->T_CDiagDev->State == STATE_BAD ) {
			All[device]->flags.defective_devfru = DIAG_TRUE;
			find_next_non_dependent( device, &next, All, num_All );
			update_do_not_display( device, next, All, DIAG_TRUE );
		}
	}
	else {
		All[device]->flags.defective_device = DIAG_TRUE;
		add_missing_fru(device,"", DC_SOURCE_MISS, 
					   All[device]->T_Pdv->led);
		find_next_non_dependent( device, &next, All, num_All );
		update_do_not_display( device, next, All, DIAG_TRUE );
	}

	return( DIAG_ASL_OK );
}

/*   */
/* NAME: test_ms2_opt
 *
 * FUNCTION: Execute DA with the MS2 option bit set
 *
 * NOTES: 
 *
 * RETURNS:
 *		DIAG_ASL_OK, normal return  
 *		DIAG_ASL_EXIT, when EXIT key entered 
 *		DIAG_ASL_CANCEL, when CANCEL key entered 
 */

DIAG_ASL_RC test_ms2_opt(int device)
{
#define FIRST 0
	int rc, next;
	int count;

	if ( All[device]->T_PDiagDev &&
	    (All[device]->T_PDiagDev->SupTests & SUPTESTS_MS2) ) {
		/* set up some defaults for the TMInput Class */
		diag_mode = DMODE_MS2;
        	count=stack_devices(All, num_All, device);
		if(num_dev_to_test<=0)
			num_dev_to_test = count;

		rc = testdevice( FIRST, USE_DANAME );
		if ( da_exit_key )
			return( DIAG_ASL_EXIT );
		if ( da_cancel_key )
			return( DIAG_ASL_CANCEL );
	}
	else {
		find_next_non_dependent( device, &next, All, num_All );
		update_delete( device, next, All, DIAG_TRUE );
	}
	return( DIAG_ASL_OK );
}

/*   */
/* NAME: update_do_not_display
 * NAME: update_delete
 * NAME: update_parent
 * NAME: update_chgstatus
 *
 * FUNCTION: update a flag from the starting_device index in All
 *           until the next device.  
 *
 */

void update_do_not_display(int starting_device,
			   int next_device,
			   diag_dev_info_ptr_t *List,
			   int value)
{
	int i;

	for (i = starting_device; i < next_device; i++)
		List[i]->flags.do_not_disp_miss = value;

	return;
}
 
void update_delete(int starting_device,
		   int next_device,
		   diag_dev_info_ptr_t *List,
		   int value)
{
	int i;

	List[starting_device]->flags.delete_from_list = value;
	for (i = starting_device; i < next_device; i++) {
		List[i]->flags.do_not_disp_miss = DIAG_TRUE;
	}
	return;
}

void update_parent(int starting_device,
		   int next_device,
		   diag_dev_info_ptr_t *List,
		   char *value)
{
	int i;
	char	method_params[256], *outbuf, *errbuf;

	for (i = starting_device; i < next_device; i++) {
		/* odm_invoke( chdev )  */
		sprintf(method_params, "-l %s -p %s",
				List[i]->T_CuDv->name, value);
		invoke_method( CHDEV, method_params, 
				       &outbuf, &errbuf);
		strcpy(List[i]->T_CuDv->parent,value);
/*		List[i]->flags.update_database = TRUE;		*/
		List[i]->flags.do_not_disp_miss = DIAG_TRUE;
		free ( outbuf );
		free ( errbuf );
	} 
	return;
}

void update_chgstatus(int starting_device,
		      int next_device,
		      diag_dev_info_ptr_t *List,
		      int value)
{
	int i;

	for (i = starting_device; i < next_device; i++) {
		List[i]->T_CuDv->chgstatus = value;
		List[i]->flags.update_database = TRUE;
		List[i]->flags.do_not_disp_miss = DIAG_TRUE;
	} 

	return;
}

/*   */
/* NAME: disp_new
 *
 * FUNCTION: Display list of new devices that have the same device type 
 *           as the missing device.
 *
 * NOTES: 
 *
 * RETURNS:
 *		FRU_REPORTED, A Goal Was Reported  
 *		DIAG_ASL_OK, normal return  
 *		DIAG_ASL_EXIT, when EXIT key entered 
 *		DIAG_ASL_CANCEL, when CANCEL key entered 
 */

DIAG_ASL_RC disp_new(int device_index,
		     int *new_device)
{
	int 		rc, index, entry, selection=0, next;
	char		*tmp,*free_tmp;
	ASL_SCR_INFO 	*newinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	/* allocate some space to use  - 20 entries should be enough */
	/* since the # of matching devices cannot be larger than the */
	/* # of slots, i.e., entry <= 8 for most machines, OR 	     */
	/*		     entry <= 16 for Gomez		     */
	newinfo = (ASL_SCR_INFO *) calloc( 1, 20*sizeof(ASL_SCR_INFO) );
	free_tmp = tmp = malloc( 20*512 );

	for(index = 0,entry=2; index < num_All; index++)
		if ( ( index != device_index )  &&	
			(CHGSTATUS( index, NEW )) && 
			!strcmp( All[device_index]->T_PDiagDev->DType, 
			       	 All[index]->T_PDiagDev->DType)) {
			resource_desc(tmp,NTF_TYPE,All[index]); 
			newinfo[entry].text = tmp;
			tmp = tmp + strlen(tmp) + 1;
			entry++;
		}

	/* if new entry not found - display error frame */
	if ( entry == 2 ) {
		free(free_tmp);
		free(newinfo);
		All[device_index]->flags.defective_device = DIAG_TRUE;
		find_next_non_dependent(device_index, &next, All, num_All);
		update_do_not_display(device_index, next, All, DIAG_TRUE);
		return(add_missing_fru(device_index,"00-00", DC_SOURCE_NEW, 
				       			     DC_RCODE_801_D));
	}

	/* put in the title line */
	newinfo[0].text = diag_cat_gets(fdes, SET_SIMNEW, MSG_SIMNEW);
	/* put in the not listed line */
	newinfo[1].text = diag_cat_gets(fdes, SET_SIMNEW, MSG_SIMNEW_1);
	/* put in the last line */
	newinfo[entry].text = diag_cat_gets(fdes, SET_SIMNEW, MSG_SIMNEW_E);

	menutype.max_index =  entry;
	rc = diag_display(NEW_RESOURCE_2, fdes, NULL, DIAG_IO, 
				ASL_DIAG_LIST_CANCEL_EXIT_SC, 
				&menutype, newinfo);

	free(free_tmp);
	free(newinfo);

	switch( rc ){
		case DIAG_ASL_EXIT :
		case DIAG_ASL_CANCEL :
			return( rc );
		case DIAG_ASL_COMMIT :
			break;
		default :
			return(DIAG_ASL_FAIL);
	}

	/* find device index corresponding to user's selection */
	if ( (selection = DIAG_ITEM_SELECTED(menutype)) == 1 ) {
		All[device_index]->flags.defective_device = DIAG_TRUE;
		find_next_non_dependent(device_index, &next, All, num_All);
		update_do_not_display(device_index, next, All, DIAG_TRUE);
		return(add_missing_fru(device_index,"00-00", DC_SOURCE_NEW, 
				       			     DC_RCODE_801_D));
	}
	for(index = 0, entry = 1; index < num_All; index++)
		if ( ( index != device_index )  &&	
			(CHGSTATUS( index, NEW )) && 
		     ( !strcmp( All[device_index]->T_PDiagDev->DType, 
			       	All[index]->T_PDiagDev->DType) ) ) {
			if ( ++entry == selection ) 
				break;
		}

	*new_device = index;
	return( DIAG_ASL_OK );
}

/*   */
/* NAME: reconfig        
 *
 * FUNCTION: This function reconfigures the new device from the missing device.
 *
 * NOTES: Undefine new device
 *
 * RETURNS:
 *
 */

DIAG_ASL_RC reconfig(int missing_device,
		     int new_device)
{

#define FIRST_ENTRY 3

	int 		index, entry, rc;
	int		selection;
	char 		*tmp, *free_tmp; 
        char            crit[256];
        struct  listinfo c_info;
        struct  CuDv    *cudv;
	ASL_SCR_INFO 	*attached_info;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	/* allocate some space to use */
	attached_info = (ASL_SCR_INFO *) calloc( 1, 
			(num_All-missing_device+5)*sizeof(ASL_SCR_INFO) );
	free_tmp = tmp = malloc( (num_All-missing_device+5)*512 );

	for (index = missing_device+1, entry = FIRST_ENTRY; 
						index < num_All; index++)
		if (PARENT_CHILD( missing_device, index ) &&
		    DETECTABLE( index, FALSE )){	
			resource_desc(tmp,DIAG_SEL_TYPE,All[index]); 
			attached_info[entry].text = tmp;
			tmp = tmp + strlen(tmp) + 1;
			entry++;
		}

	/* put in the title line */
	attached_info[0].text = diag_cat_gets(fdes, SET_MISSREP, MSG_MISSREP_T);

	/* selection for none of the devices listed */
	attached_info[1].text = diag_cat_gets(fdes, SET_MISSREP, MSG_MISSREP_1);

	/* selection for all devices listed */
	attached_info[2].text = diag_cat_gets(fdes, SET_MISSREP, MSG_MISSREP_2);

	/* put in the last line */
	attached_info[entry].text = diag_cat_gets(fdes, SET_MISSREP,
			MSG_MISSREP_E);

	menutype.max_index =  entry;

	while ( entry != FIRST_ENTRY ) {
		rc = diag_display(MISS_REPLACED, fdes, NULL, DIAG_IO, 
				ASL_DIAG_LIST_COMMIT_SC, 
				&menutype, attached_info);

		if ( rc == DIAG_ASL_EXIT )
			return( rc );
		else if ( rc == DIAG_ASL_COMMIT )
			break;
		else if ( rc == DIAG_ASL_ENTER || rc == DIAG_ASL_CANCEL )
			flag_selection(rc, DIAG_ITEM_SELECTED(menutype),
			               FIRST_ENTRY, entry, attached_info);
		else
			return(DIAG_ASL_FAIL);
	}

	free(free_tmp);

	/* if selection "None should be selected" was not chosen */
	if (attached_info[1].item_flag != '*' && entry != FIRST_ENTRY ) {
		/* all of the devices listed should be reconfigured */
		for(index = missing_device, entry = FIRST_ENTRY; 
						index < num_All; index++){
			if (PARENT_CHILD( missing_device, index ) &&
			    DETECTABLE( index, FALSE )){
				if ( attached_info[entry++].item_flag == '*') {
	 				update_parent( index, index+1, All,
						All[new_device]->T_CuDv->name );
					if (CHGSTATUS( index, MISSING )) 
						update_chgstatus( index,
							index+1, All, NEW );
				}
			}
		}
	}

	free(attached_info);
	/* Delete all detectable children of the missing device. */ 
	/* Then delete the missing device.			*/
        for (index = missing_device+1; index < num_All; index++){
                if (PARENT_CHILD( missing_device, index ) &&
                    DETECTABLE( index, TRUE )) {
                        sprintf(crit,"parent = %s", All[index]->T_CuDv->name);
                        cudv = (struct CuDv *)diag_get_list(CuDv_CLASS, crit,
                                &c_info, 1,1);
                        /* if no more children, then delete the device */
                        if ( c_info.num == 0 )
                                update_delete(index, index+1, All, DIAG_TRUE);
                        else
                                 diag_free_list(cudv, &c_info);
                }
	}
	update_delete(missing_device, missing_device+1, All, DIAG_TRUE);

	return(DIAG_ASL_OK);

}

/*   */
/* NAME: flag_selection  
 *
 * FUNCTION: This function flags the user's selection with an asterisk.
 *
 * NOTES:    If first selection is made, "None of the devices..."
 *			Clear all previous selections
 *           or second selection is made, "All of the devices..."
 *			Then all device selections are set with an "*'
 *	     Else the device selected is flagged
 *
 * RETURNS: NONE
 *
 */

void flag_selection(int asl_ret,
		    int user_select,
		    int first_entry,
		    int num_entries,
		    ASL_SCR_INFO *select_info)
{
	int	i;

	switch ( user_select ) {
		case 1 :		/* none of the devices	*/
			select_info[2].item_flag = ' ';
			for ( i = first_entry; i < num_entries; i++ ) {
				select_info[i].item_flag = ' ';
			}
			break;

		case 2 :		/* all of the devices	*/
			select_info[1].item_flag = ' ';
			for ( i = first_entry; i < num_entries; i++ ) {
				select_info[i].item_flag = 
					(asl_ret == DIAG_ASL_CANCEL ) 
							? ' ' : '*';
			}
			break;
		default:		/* selected device	*/
			select_info[1].item_flag = ' ';
			break;
	}
	/* flag the selected entry */
	select_info[user_select].item_flag = 
				(asl_ret == DIAG_ASL_CANCEL ) ? ' ' : '*';
}

/*   */
/*
 * NAME: check_frub
 *
 * FUNCTION: 
 * 	Check for any defective devices - if so display Trouble Found 
 *
 * NOTES:
 *
 * RETURNS: 0
 *
 */

void check_frub(void)
{
	int			i;

	for ( i = 0; i < num_All; i++ )
		if ( (All[i]->flags.defective_devfru == DIAG_TRUE) ||
		     (All[i]->flags.defective_device == DIAG_TRUE) ) {
			disp_fru();
			break;
		}
	return;
}

/*   */
/*
 * NAME: add_missing_fru
 *
 * FUNCTION: 
 *
 * NOTES:
 *
 * RETURNS:
 *		FRU_REPORTED, A Goal Was Reported  
 *
 */

int add_missing_fru(int index,
		    char *location,		/* optional location field	*/
		    int sn,
		    int rcode)
{
	int rc;
	static struct fru_bucket miss_frub; 

	strcpy( miss_frub.dname, All[index]->T_CuDv->name );
	miss_frub.ftype = FRUB1;
	miss_frub.sn = sn;
	miss_frub.rcode = rcode;
	miss_frub.rmsg = DC_MIS_DEV;
	miss_frub.frus[0].conf = 100;
	miss_frub.frus[0].fru_flag = DA_NAME;
	miss_frub.frus[0].fmsg = 0;
	strcpy( miss_frub.frus[0].fname, All[index]->T_CuDv->name );
	if (strlen(location))
		strcpy( miss_frub.frus[0].floc, location );

	miss_frub.frus[1].conf = 0;

	addfrub(&miss_frub);

	return(FRU_REPORTED);

}

/*   */
/* NAME: check_new_devices
 *
 * FUNCTION: Display list of new resources
 *
 * NOTES: 
 *
 * RETURNS:
 *		FRU_REPORTED, A Goal Was Reported  
 *		DIAG_ASL_OK, normal return  
 *		DIAG_ASL_EXIT, when EXIT key entered 
 *		DIAG_ASL_CANCEL, when CANCEL key entered 
 */

DIAG_ASL_RC check_new_devices(void)
{
	int 	rc, selection;
	int	index;
	char	isa_sa_prog[255];
	char    *flag_ptr[3];
	char	*str;
	char	diagdir[255];
	int	status;
	struct  fru_bucket frub;

	/* regenerate the All list with a new stacking order */
	free (All);
	All = generate_All( num_Top, Top, &num_All );

	switch (rc = new_menu(&selection)) {
		case DIAG_ASL_EXIT :
		case DIAG_ASL_CANCEL :
			return( rc );
		case DIAG_ASL_COMMIT :
			break;
		case NO_NEW_DEVICES :
			return(DIAG_ASL_OK);
		default :
			return(DIAG_ASL_FAIL);
	}

	switch ( selection ) {
		case 2:			/* set all NEW devices to SAME */
			for(index = 0; index < num_All; index++)
				if ( CHGSTATUS( index, NEW )) {
					All[index]->flags.update_database =
							DIAG_TRUE; 
					SETSTATUS( index, SAME);
				}
			break;
		case 3:			/* not correct - display default*/
		       			/* SRN                          */
			frub.sn = DC_SOURCE_NEW;
			frub.rcode = DC_RCODE_801_M;
			frub.rmsg = DC_NEW_MSG;
			frub.frus[0].conf = 0;	/* tells DC no frus */
			disp_frub( &frub );
			return( FRU_REPORTED );
		case 4:			/* invoke ISA config service aid */
			/* get directory path to use for execution of SA */
			if ((str = (char *)getenv("DIAG_UTILDIR")) == (char *)NULL ) {
				if ((str=(char *)getenv("DIAGNOSTICS")) == (char *)NULL)
				       sprintf(diagdir, "%s/bin/", DIAGNOSTICS);
				else
				       sprintf(diagdir, "%s/bin/", str);
			}
			else
				sprintf(diagdir, "%s/", str);

			sprintf(isa_sa_prog, "%sudisacfg", diagdir);
			flag_ptr[0] = isa_sa_prog;
			flag_ptr[1] = NULL;
			flag_ptr[2] = NULL;
			rc=diag_asl_execute(isa_sa_prog, flag_ptr, &status);
			break;
	}	

	return(DIAG_ASL_OK);
}

/*   */
/*
 * NAME: new_menu
 *
 * FUNCTION: Display the New Device Menu 
 *
 * NOTES:
 *
 * RETURNS:
 *		NO_NEW_DEVICES, no new devices were found
 *		DIAG_ASL_EXIT, when EXIT key entered 
 *		DIAG_ASL_CANCEL, when CANCEL key entered 
 *		DIAG_ASL_COMMIT, when selection has been made
 *
 */

DIAG_ASL_RC new_menu(int *selection)
{
	int 	rc, count=0;
	int 	index;
	int	new_device_found = DIAG_FALSE;
	char	*tmp;
	char	*buffer;
	char	*opstring;
	static struct  msglist new[] = {
			{ SET_ALLNEW, MSG_ALLNEW, },
			{ SET_ALLNEW, MSG_ALLNEW_0, },
			{ SET_ALLNEW, MSG_NEWRESOURCES_OPT1, },
			{ SET_ALLNEW, MSG_NEWRESOURCES_OPT2, },
			{ SET_ALLNEW, MSG_ALLNEW_NOISA, },
			(int)NULL
	};
	static ASL_SCR_INFO newinfo[ DIAG_NUM_ENTRIES(new) ];
	static struct  msglist isa_new[] = {
			{ SET_ALLNEW, MSG_ALLNEW, },
			{ SET_ALLNEW, MSG_ALLNEW_0, },
			{ SET_ALLNEW, MSG_NEWRESOURCES_OPT1, },
			{ SET_ALLNEW, MSG_NEWRESOURCES_OPT2, },
			{ SET_ALLNEW, MSG_NEWRESOURCES_OPT3, },
			{ SET_ALLNEW, MSG_ALLNEW_ISA, },
			(int)NULL
	};
	static ASL_SCR_INFO isanewinfo[ DIAG_NUM_ENTRIES(isa_new) ];
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
        for(index = 0; index < num_All; index++)
		if ( CHGSTATUS( index, NEW ))
			count++;


	buffer = tmp = (char *)calloc(1,256*count);
	if(has_isa_capability())
		rc = diag_display(NULL, fdes, isa_new, DIAG_MSGONLY, NULL,
				  &menutype, isanewinfo);
	else
		rc = diag_display(NULL, fdes, new, DIAG_MSGONLY, NULL,
				  &menutype, newinfo);

	for(index = 0; index < num_All; index++)
		if ( CHGSTATUS( index, NEW )) {
			new_device_found = DIAG_TRUE;
			resource_desc(tmp,NTF_TYPE,All[index]); 
			tmp[strlen(tmp)] = '\n';
			tmp[strlen(tmp)] = '\0';
			tmp = tmp + strlen(tmp);
		}
	if ( !new_device_found ){
		free(buffer);
		return(NO_NEW_DEVICES);
	}
	opstring = malloc(strlen(buffer) + 256);
	if(has_isa_capability())
	{
		sprintf(opstring, isanewinfo[1].text, buffer);
		free(isanewinfo[1].text);
		isanewinfo[1].text = opstring;
		isanewinfo[1].non_select = ASL_NO;
	} else 
	{
		sprintf(opstring, newinfo[1].text, buffer);
		free(newinfo[1].text);
		newinfo[1].text = opstring;
		newinfo[1].non_select = ASL_NO;
	}
	rc = -1;
	while ( rc != DIAG_ASL_EXIT && rc != DIAG_ASL_CANCEL ) {
		if(has_isa_capability())
			rc = diag_display(NEW_RESOURCE_1, fdes, NULL, DIAG_IO, 
				  ASL_DIAG_LIST_CANCEL_EXIT_SC, 
				  &menutype, isanewinfo);
		else
			rc = diag_display(NEW_RESOURCE_1, fdes, NULL, DIAG_IO, 
				  ASL_DIAG_LIST_CANCEL_EXIT_SC, 
				  &menutype, newinfo);
		if ( rc == DIAG_ASL_COMMIT )
			*selection = DIAG_ITEM_SELECTED(menutype);
			if ( *selection == 1 )
				Perror(fdes, ESET, ERROR11, NULL);
			else
				break;
	}
	free(buffer);
	free(opstring);
	return(rc);
}

/*   */
/*
 * NAME: update_device_database
 *
 * FUNCTION: 
 * 	Update or Delete a device from the Customized Devices Database.
 *
 * NOTES:
 *
 * RETURNS:
 *
 */

void update_device_database(void)
{
	int i, j, next, rc;
	char method_params[256], *errbuf, *outbuf;

	for (i = 0; i < num_All; i++)  {
		if (All[i]->flags.update_database == DIAG_TRUE) {
			/* change T_CuDv */
			rc = diag_change_obj(CuDv_CLASS, All[i]->T_CuDv);
		}
		else if (All[i]->flags.delete_from_list == DIAG_TRUE){
			find_next_non_dependent(i, &next, All, num_All );
			for ( j = next-1; j >= i; j-- ) {
				/* odm_invoke( rm device )  */
				sprintf(method_params, "-l %s -d",
					All[j]->T_CuDv->name);
				invoke_method(
				      RMDEV, method_params, 
				       &outbuf, &errbuf);
				free ( outbuf );
				free ( errbuf );
			}
		}
	}	

	return;
}

