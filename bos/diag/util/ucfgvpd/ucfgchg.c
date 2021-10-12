static char sccsid[] = "@(#)04	1.11  src/bos/diag/util/ucfgvpd/ucfgchg.c, dsauchgvpd, bos411, 9428A410j 5/26/93 16:26:29";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY
 *
 * FUNCTIONS: 	cfg_change
 *		add_drawer
 *		get_slot
 *		config_device
 *		delete_drawer
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
#include <signal.h>
#include <nl_types.h>
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/class_def.h"	/* object class data structures	*/ 
#include "ucfgvpd_msg.h"

#define UCHGCFG_MENU	0x802035
#define SLOT_MENU	0x802036
#define NODRAW_MENU	0x802037
#define DRAWER_MENU	0x802038
#define DCHILD_MENU	0x802039

/* GLOBAL VARIABLES	*/
int num_All;			/* number of devices in All              */
int num_Top;			/* number of devices in Top              */
diag_dev_info_t	    *Top;	/* pointer to all device data structures */
extern diag_dev_info_ptr_t *All;	/* array containing devices in All       */
diag_dev_info_ptr_t *DEV_disp;	/* array containing devices listed       */
char	config_device_name[NAMESIZE];

/* EXTERNAL VARIABLES	*/
extern nl_catd		fdes;		/* catalog file descriptor	*/
extern ASL_SCR_TYPE 	dm_menutype;

/* CALLED FUNCTIONS	*/
extern char *substrg();
extern char *diag_cat_gets();

diag_dev_info_t *init_diag_db();
diag_dev_info_ptr_t *generate_All();

/* FUNCTION PROTOTYPES */
int cfg_change(void);
int add_drawer(int);
int get_slot(int *);
int config_device(char *, int);
int display_drawers(int *);
int delete_drawer(void);
int resource_desc(char *, diag_dev_info_t *);
int disp_drawer_children(int);
int remove_device(char *);

/*  */ 
/*
 * NAME: cfg_change
 *                                                                    
 * FUNCTION: Add or delete resources to or from the configuration.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS:
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_CANCEL
 */  

int cfg_change(void)
{
	int 	rc = -1;
	int	selection;
	static struct msglist	menulist[] = {
		{ MSET6, M6TITLE },
		{ MSET6, M6OPTACP },	/* add cpu drawer	*/
		{ MSET6, M6OPTASD },	/* add SCSI device draw */
		{ MSET6, M6OPTASA },	/* add SCSI DASD drawer */
		{ MSET6, M6OPTAAD },	/* ADD async drawer     */
		{ MSET6, M6OPTDD },	/* delete drawer	*/
		{ MSET6, M6LASTLINE },
		{ (int )NULL, (int )NULL}
	};

	while( rc != DIAG_ASL_CANCEL && rc != DIAG_ASL_EXIT )  {

		rc = diag_display(UCHGCFG_MENU, fdes, menulist, 
					DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
					NULL, NULL);
		if( rc == ASL_COMMIT )
			switch (selection = DIAG_ITEM_SELECTED(dm_menutype))  {
				case 1 :
				case 2 :
				case 3 :
				case 4 :
					add_drawer(selection);
					break;
				case 5 :
					delete_drawer();
					break;
			}
	}
	return(rc);
}
/*  */ 
/*
 * NAME: add_drawer
 *                                                                    
 * FUNCTION: Add a drawer to the configuration
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * NOTES:
 *	Present a menu asking the slot number of the adapter the drawer 
 *		is attached to.
 *	Issue a mkdev command to define the new device.
 *		mkdev 	-c drawer 	(class)
 *			-s class	(subclass)
 *			-t model	(type)
 *			-d		(define device only)
 *			-w slot#	(connection_location)
 *			-p parent	(parent)
 *	Read device info from CuDv
 *	Present a menu to add vpd data for new drawer
 *	Update CuDv entry with new vpd data
 *
 * RETURNS:
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_CANCEL
 *	 0 		- no error
 *	-1 		- error
 */  

int add_drawer(int drawer_id)	/* 1 - CPU drawer
				 * 2 - SCSI device drawer
				 * 3 - SCSI DASD drawer
				 * 4 - Async Exp drawer */
{
	int 	rc;
	int	adapter_slot;
	char	*type;

	/* prompt for slot position of adapter card */
	adapter_slot = 0; 	/* CPU drawer is always 0 */
	if (drawer_id != 1) { 
		rc = get_slot(&adapter_slot);
		if ( rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL )
			genexit(0);
	}

	/* issue a mkdev command to configure the drawer */
	if (drawer_id == 1) type = CPU_DRAWER;
	else if (drawer_id == 2) type = SCSI_DRAWER;
	else if (drawer_id == 3) type = MEDIA_DRAWER;
	else type = ASYNC_DRAWER;
	if ( (rc = config_device(type,adapter_slot)) != 0 )
		return(1);

	/* get device entry from CuDv and update with new vpd data */
	rc = dsp_alt_vpd(config_device_name); 
	return(rc);

}
/*  */
/*
 * NAME: get_slot
 *                                                                    
 * FUNCTION: Present dialog box and prompt for adapter slot and bus
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */  

int get_slot(int *slot_id)
{

	int 	rc;
	int 	line = 0;
	int	bus_id = 0;
	char	entered_slot[2];
	char	entered_bus[2];
	struct  CuDv  	*cudv;
	struct	listinfo cinfo;
	static char	available_slots[] = { "1,2,3,4,5,6,7,8" };
	static char	valid_bus_ids[] = { "0,1" };
	ASL_SCR_INFO    *dialog_info;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        /* allocate space for at least 5 entries */
        dialog_info = (ASL_SCR_INFO *) calloc(5,sizeof(ASL_SCR_INFO) );
        if ( dialog_info == (ASL_SCR_INFO *) NULL)
                return(-1);

        /* set the title line in the array   */
        dialog_info[line++].text = diag_cat_gets(fdes, MSET6, M61BTITLE);

	/* slot prompt */
        dialog_info[line].text = diag_cat_gets(fdes, MSET6, M61CPROMPT);
	entered_slot[0] = '1';
	entered_slot[1] = '\0';
	dialog_info[line].op_type = ASL_RING_ENTRY;
	dialog_info[line].entry_type = ASL_NUM_ENTRY;
	dialog_info[line].required = ASL_YES;
	dialog_info[line].disp_values = available_slots;
	dialog_info[line].data_value = entered_slot;
	dialog_info[line].entry_size = 1;
	line++;
	
	/* might be a XIO bus */
	cudv = get_CuDv_list( CuDv_CLASS, "name like bus*", &cinfo,1,1 );
	if ( cudv == (struct CuDv *) -1 )
		return (-1);
	if ( cinfo.num > 1 )  {
		entered_bus[0] = '0';
		entered_bus[1] = '\0';
		dialog_info[line].op_type = ASL_RING_ENTRY;
		dialog_info[line].entry_type = ASL_NUM_ENTRY;
		dialog_info[line].required = ASL_YES;
		dialog_info[line].disp_values = valid_bus_ids;
		dialog_info[line].data_value = entered_bus;
		dialog_info[line].entry_size = 1;
        	dialog_info[line++].text = diag_cat_gets(fdes, 
						MSET6, M61CBUS);
	}
        dialog_info[line].text = diag_cat_gets(fdes, MSET6, M61DEFAULT);
	menutype.max_index = line ;

	while ( 1 ) {
		rc = diag_display( SLOT_MENU, fdes, NULL, DIAG_IO,
			ASL_DIAG_DIALOGUE_SC, &menutype, dialog_info );

		if ( rc == DIAG_ASL_COMMIT ) {
			*slot_id = (int)(dialog_info[1].data_value[0]-'0');
			if ( cinfo.num > 1 )
				bus_id=(int)(dialog_info[2].data_value[0]-'0');
			if ( ( *slot_id >= 1 && *slot_id <= 8 )  &&
					( bus_id == 0 || bus_id == 1 ) )
				break;
		}
	        else if ( rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL )
			return(rc);
	}
	if (cinfo.num > 1 && bus_id == 1 )
		*slot_id += 10;
	return (rc);
}
/*  */ 
/* NAME: config_device
 *
 * FUNCTION: Issues a mkdev command to configure a device 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	0 - no error
 *     -1 - error
 *
 */

int config_device(
char *dtype,		/* type of device to configure	*/
int slot)		/* slot of adapter for drawer	*/
{
	int 		rc;
	char		buffer[128];
	char		mkdev_args[512];
	char		*outbuf, *errbuf;
	struct listinfo	p_info;
	struct PdDv	*T_PdDv;

	/*
		query predefined for class, subclass corresponding
		to attdtype. 
		mkdev -c class -s subclass -t attdtype -p parent 
		               -w connection -d
	*/
	sprintf(buffer, "type = %s", dtype );
	T_PdDv = get_PdDv_list(PdDv_CLASS, buffer,&p_info,1,1);
	if ( T_PdDv == (struct PdDv *) -1 )
		return(-1);
	if ( slot <= 8 ) 
		sprintf(mkdev_args,"-c %s -s %s -t %s -p %s -w %02d -d",
			T_PdDv->class,		/* class   	*/
			T_PdDv->subclass,	/* subclass 	*/
			T_PdDv->type,		/* type 	*/
			SYSUNIT,            	/* parent	*/
			slot);      		/* port  	*/
	else
		sprintf(mkdev_args,"-c %s -s %s -t %s -p %s -w %d -d",
			T_PdDv->class,		/* class   	*/
			T_PdDv->subclass,	/* subclass 	*/
			T_PdDv->type,		/* type 	*/
			SYSUNIT,            	/* parent	*/
			slot);      		/* port  	*/
	rc = odm_run_method(MKDEV, mkdev_args, &outbuf, &errbuf);
	if ( rc == 0 )
		sscanf(outbuf, "%s", config_device_name);
	else
		diag_hmsg(fdes, ESET, ERR_MKDEV, dtype);
	free ( outbuf );
	free ( errbuf );
	odm_free_list(T_PdDv, &p_info);
	return(rc);
}
/*  */ 
/*
 * NAME: delete_drawer
 *                                                                    
 * FUNCTION: Delete a drawer from the configuration
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES:
 *	Get list of all drawers
 *	If no drawers in database, present message that there are no resources
 *		to delete.
 *	Present menu with list of resources to delete
 *	Issue rmdev command to remove resource from configuration.
 *		rmdev -d -l dname
 *	If rmdev failed because of attached children
 *		Present menu of attached children and ask if want to delete
 *		If yes, issue rmdev for each attached device
 *		Resissue rmdev to remove parent resource
 *
 * RETURNS:
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_CANCEL
 *	-1, error
 */  

int delete_drawer(void)
{
	int 	rc;
	int	drawer_index;

	/* initialize the data structures for the device */
	if ( Top != NULL ) free ( Top );
	if ( All != NULL ) free ( All );
	if ( DEV_disp != NULL ) free ( DEV_disp );

	if ((Top = init_diag_db( &num_Top )) == (diag_dev_info_t *) -1 )
		return(-1);

	/* get list of all devices */
	All = generate_All( num_Top, Top , &num_All);

	DEV_disp = (diag_dev_info_ptr_t *)
		   	calloc(num_All,sizeof(DEV_disp[0]));


	/* prompt for the drawer */
	rc = display_drawers(&drawer_index);
	if ( rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL )
		return(0);

	if ( remove_device( DEV_disp[drawer_index]->T_CuDv->name ) != 0 ) {
		/* check for any children */
		rc = disp_drawer_children(drawer_index);
		if ( rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL )
			return(0);
	}

}
/*  */
/* NAME: display_drawers
 *
 * FUNCTION: Display all the drawers menu.
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *
 * RETURNS:
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */

int display_drawers(int *selection)
{

	int		i;
        int             index;
	int		num_disp_devices;
        int             rc = DIAG_ASL_OK;
	int		line = 0;
        char            *string;
        char            *free_string;
	ASL_SCR_INFO    *resinfo;
	static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for at least 16 drawers */
        resinfo = (ASL_SCR_INFO *) calloc(16,sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) 0)
                return(-1);

        free_string = string = (char *)malloc(num_All*132);
        if ( string == (char *) NULL)
                return(-1);

        /* set the title line in the array   */
        resinfo[line++].text = diag_cat_gets(fdes, MSET6, M63TITLE);

        /* read the entries from All, get the DName, location, and
           description and put them in the array        */
        for(index=0, num_disp_devices=0; index < num_All; index++)  {
		if (!strcmp(substrg(PCLASS,All[index]->T_CuDv->PdDvLn_Lvalue), 
					CLASS_DRAWER))
			resource_desc(string,All[index]);
		else
			continue;
                resinfo[line].text = string;
        	resinfo[line].non_select = ASL_NO;
		string = string + strlen(string)+1;
		line++;
		DEV_disp[num_disp_devices++] = All[index];
        }

        /* finally add the last line */
        resinfo[line].text = diag_cat_gets(fdes, MSET6, M63LASTLINE);

        restype.max_index = line;
        restype.cur_index = 1;

	if ( num_disp_devices == 0 ) 
		rc = DIAG_ASL_CANCEL;
	else
        	/* now display menu */
		rc = diag_display(DRAWER_MENU, fdes, NULL, DIAG_IO,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					&restype, resinfo);

	if ( rc == DIAG_ASL_COMMIT )
		*selection = DIAG_ITEM_SELECTED(restype) - 1;
	else
		*selection = 0;

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
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * NOTES:
 *	This function takes as input a line type which indicates the type
 *	of display line to build.  
 *
 * RETURNS: 0 
 */  

int resource_desc(
char *string,
diag_dev_info_t *dev_ptr)
{
	char	*tmp;

	sprintf(string, "  %-16s %-16.16s ", 
			dev_ptr->T_CuDv->name,
			dev_ptr->T_CuDv->location);

	tmp = string + strlen(string);
	mergetext( strlen(string), tmp, dev_ptr->Text);
	return(0);
}
/*  */
/* NAME: disp_drawer_children
 *
 * FUNCTION: Display all the children of a drawer
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS:
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 *	-1, error
 */

int disp_drawer_children(int parent_index)
{

	int		i;
        int             index = parent_index;
	int		num_disp_devices;
        int             rc;
	int		length;
        char            *string;
        char            *free_string;
	char		*children[16];
	char		dname[NAMESIZE];
	char		buffer[1024];
	ASL_SCR_INFO    *resinfo;
	static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for at least 4 */
        resinfo = (ASL_SCR_INFO *) calloc(4,sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) 0)
                return(-1);

        free_string = string = (char *)malloc(num_All*132);
        if ( string == (char *) NULL)
                return(-1);

        /* set the title line in the array   */
        resinfo[0].text = diag_cat_gets(fdes, MSET6, DCHILDTITLE);

        /* read the entries from All, get the DName, location, and
           description and put them in the array        */
        for(index=0, num_disp_devices=0; index < num_All; index++)  {
		if (!strcmp(All[index]->T_CuDv->parent,
		             DEV_disp[parent_index]->T_CuDv->name))
			resource_desc(string,All[index]);
		else
			continue;
                children[num_disp_devices++] = string;
		length = strlen(string);
		string[length++] = '\n';
		string[length++] = '\0';
		string = string + strlen(string);
        }
	if ( num_disp_devices ) {
		sprintf(buffer, resinfo[0].text, free_string);
		resinfo[0].text = buffer;
        	resinfo[1].text = diag_cat_gets(fdes, MSET6, MCHILDYES);
       		resinfo[2].text = diag_cat_gets(fdes, MSET6, MCHILDNO);
       		/* finally add the last line */
        	resinfo[3].text = diag_cat_gets(fdes, MSET6, MCHILDLAST);

        	/* now display menu */
        	restype.max_index = 3;
        	restype.cur_index = 1;
		rc = diag_display(DCHILD_MENU, fdes, NULL, DIAG_IO,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					&restype, resinfo);
	}
	else 
		rc = -1;
	if ( rc == DIAG_ASL_COMMIT )
		switch (DIAG_ITEM_SELECTED(restype)) {
			case 1:		/* yes */
				for ( i=0; i < num_disp_devices; i++ ) {
					sscanf(children[i], "%s", dname);
					remove_device( dname );
				}
				break;
			case 2:		/* no  */
				break;
		}
        free ( free_string );
        free ( resinfo );

	return (rc);
}
/*  */
/* NAME: remove_device
 *
 * FUNCTION: Issues a rmdev command to remove a device 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	0 - no error
 *     -1 - error
 *
 */

int remove_device(char *dname)		/* device name */
{
	int 		rc;
	char		rmdev_args[512];
	char		*outbuf, *errbuf;

	/*
		query predefined for class, subclass corresponding
		to attdtype. 
		rmdev -d -l dname
	*/
	sprintf(rmdev_args,"-d -l %s", dname);
	rc = odm_run_method(RMDEV, rmdev_args, &outbuf, &errbuf);
	return(rc);
}
