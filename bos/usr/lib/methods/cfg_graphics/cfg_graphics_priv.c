/* static char sccsid[] = "@(#)46	1.14  src/bos/usr/lib/methods/cfg_graphics/cfg_graphics_priv.c, dispcfg, bos411, 9435B411a 8/16/94 17:38:51"; */
/*
 *   COMPONENT_NAME: DISPCFG
 *
 *   FUNCTIONS: Check_Input_Parms
 *		Open_ODM_and_Get_Objects
 *		Prepare_Device
 *		Prepare_Feature_ROM_Files
 *		Prepare_MCAbus_Type_Device
 *		Prepare_SGAbus_Type_Device
 *		Process_Device_Customized_VPD
 *		Set_Up_CCM_DD_Case1830
 *		err_exit
 *		err_term_dd
 *		err_undo_dd
 *		err_undo_kmods
 *		err_unload_dd
 *		unlink_tmp_files
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* NOTE:  This file is #include-ed into other source files and only
   contains static subroutine definitions.  It will never have its
   own .o file and therefore cannot have an sccsid string or include
   files
*/
  
/*****************************************************************************
 ********************** MODULE HEADER TOP ************************************
 *****************************************************************************
 ---------------------------------------------------------------------------
  
 PURPOSE:	
		This routine is part of a problem-state process that
		is executed by the system during configuration and by
		the system when certain configuration commands are 
		invoked.  The functions it contains are the standard
		functions used by the AIX devices configuration architecture.
  
		Typically, device specific configuration methods are
		compiled into a ".o" which are linked with a standard
		"main.o".  This is because the configuration process is
		relatively standard.  However, for display devices, there
		are sufficient reasons to have a separate "main" that
		has been design just for graphics.
  
		This software therefore is called from routines in 
		the companion file "cfg_graphics.c".
  
		The interface between this problem-state process and the
		AIX device drivers it requires are generally found
		in the AIX system call "sysconfig( )".  There are some
		front end tools in "cfgraphics.c" and in "graphicstools.c"
		which assist in the management of the sysconfig( ) call.
  
		This routine provides the standard functions to build a
		define-device-structure and to query the VPD.  It also
		has device specific code in it, to decide which type of
		GTy adapter has been installed on the bus.  It updates
		the customized attributes accordingly.
  
		This routine also supports the common character mode
		VDD device driver in addition to the regular full function
		VDD.  See design 13588 for additional details.
  
 PROGRAMMING INTERFACE:
  
   cfgXXX -l <logical_name> [- < 1 | 2 > ] [ -D ]
  
 *****************************************************************************
 ********************** MODULE HEADER BOTTOM *********************************
 *****************************************************************************/




/*==========================================================================
|
|  These files need no include files, because the source code is included
|  directly into cfg_graphics.c to maintain static interfaces
|
|==========================================================================*/






/*
 * NAME: unlink_tmp_files 
 *
 * FUNCTION: 
 *
 * INPUT: 
 *
 * This routine is to be used only within this file.  The device specific
 * routines for the various device specific config methods must not call this
 * function. 
 *
 * NOTES: 
 *
 * 
 *
 * RETURNS: None 
 */

STATIC void 
unlink_tmp_files( )
{

if  ( 0 != strcmp( tmp_cfg_meth , "" ))
{
	unlink( tmp_cfg_meth );
}

if  ( 0 != strcmp( tmp_cdd , "" ))
{
	unlink( tmp_cdd );
}

if  ( 0 != strcmp( tmp_cdd_pin , "" ))
{
	unlink( tmp_cdd_pin );
}

if  ( 0 != strcmp( tmp_ucode , "" ))
{
	unlink( tmp_ucode );
}


	return;
}



/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *	back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is to be used only within this file.  The device
 *	specific routines for the various device specific config methods
 *	must not call this function.
 *
 * NOTES:
 *
 * void
 *	err_exit( exitcode )
 *	exitcode = The error exit code.
 *
 * RETURNS:
 *	None
 *
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/




STATIC void
err_exit (int             exitcode)
{
	/*
	 * Close any open object class 
	 */

	(void) odm_close_class (CuDv_CLASS);
	(void) odm_close_class (PdDv_CLASS);
	(void) odm_close_class (CuAt_CLASS);
	(void) odm_close_class (PdAt_CLASS);

	/*
	 * Unlock and Terminate the ODM 
	 */

	odm_unlock (lock_id);
	odm_terminate ();

	unlink_tmp_files( );

	exit (exitcode);
}






/*
 * NAME: err_unload_dd 
 *
 * FUNCTION: Unloads the device's device driver.  Used to back out on an error. 
 *
 * INPUT: pointer to device name OUTPUT: None EXECUTION ENVIRONMENT: 
 *
 * This routine is to be used only within this file.  The device specific
 * routines for the various device specific config methods must not call this
 * function. 
 *
 * NOTES: 
 *
 * void err_unload_dd( DvDr ) DvDr = pointer to device driver name. 
 *
 * RETURNS: None 
 */

STATIC void 
err_unload_dd (char *DvDr)
{
	/*
	 * unload driver 
	 */

	if (loadext (DvDr, FALSE, FALSE) == NULL)
	{
		DEBUG_0 ("cfg_graphics: error unloading driver\n")
	}
	return;
}



/*
 * NAME: err_term_dd 
 *
 * FUNCTION: Terminates the device.  Used to back out on an error. 
 *
 * INPUT: device number (devno) OUTPUT: None EXECUTION ENVIRONMENT: 
 *
 * This routine is to be used only within this file.  The device specific
 * routines for the various device specific config methods must not call this
 * function. 
 *
 * NOTES: 
 *
 * void err_term_dd( devno ) devno = The device's devno. 
 *
 * RETURNS: None 
 */

STATIC void
err_term_dd (dev_t devno)
{
	struct cfg_dd   cfg;	       /* sysconfig command structure */
	/*
	 * terminate device 
	 */

	cfg.devno = devno;
	cfg.kmid = (mid_t) 0;
	cfg.ddsptr = (caddr_t) NULL;
	cfg.ddslen = (int) 0;
	cfg.cmd = CFG_TERM;

	if (sysconfig (SYS_CFGDD, &cfg, sizeof (struct cfg_dd)) == FAIL)
	{
		DEBUG_0 ("cfg_graphics: error unconfiguring device\n")
	}
	return;
}
/*
 * NAME: err_undo_kmods 
 *
 * FUNCTION: 
 *
 * Terminates the device and unloads the driver, if the device has a driver.  If
 * the device does not have a driver, it simply returns. This routine is used
 * to back out on errors that occur while processing VPD. 
 *
 * INPUT: pointer to device name 
 *
 * EXECUTION ENVIRONMENT: 
 *
 * This routine is to be used only within this file.  The device specific
 * routines for the various device specific config methods must not call this
 * function. 
 *
 * NOTES: 
 *
 * void err_undo_kmods( DvDr ) DvDr  = pointer to device driver name.
 *
 * RETURNS: None 
 */


STATIC void
err_undo_kmods ()
{
char * DvDr;
	
	if ( CCM_DDS_ccmdd_kmid (CFGGRAPH_ccm_dds) != 0 )
	{
		err_term_dd (CCM_DDS_device_num( CFGGRAPH_ccm_dds ));
	}

	if (CCM_DDS_cdd_kmid( CFGGRAPH_ccm_dds ) != 0)
	{
		DvDr = (CCM_PATHNAME_cdd_kmod_entry( CFGGRAPH_ccm_pathnames));

		if (loadext (DvDr, FALSE, FALSE) == NULL)
		{
			DEBUG_1 ("cfg_graphics: error unloading driver %s\n ",DvDr)
		}
	}
	if (CCM_DDS_cdd_pin_kmid( CFGGRAPH_ccm_dds ) != 0)
	{
		DvDr = (CCM_PATHNAME_cdd_kmod_interrupt( CFGGRAPH_ccm_pathnames));

		if (loadext (DvDr, FALSE, FALSE) == NULL)
		{
			DEBUG_1 ("cfg_graphics: error unloading driver %s\n ",DvDr)
		}
	}
	if (CCM_DDS_ccmdd_kmid( CFGGRAPH_ccm_dds ) != 0)
	{
		DvDr = (CCM_PATHNAME_ccm_dd( CFGGRAPH_ccm_pathnames));

		if (loadext (DvDr, FALSE, FALSE) == NULL)
		{
			DEBUG_1 ("cfg_graphics: error unloading driver %s\n ",DvDr)
		}
	}
	if (CCM_DDS_ccmdd_pin_kmid( CFGGRAPH_ccm_dds ) != 0)
	{
		DvDr = (CCM_PATHNAME_ccm_dd_pin( CFGGRAPH_ccm_pathnames));

		if (loadext (DvDr, FALSE, FALSE) == NULL)
		{
			DEBUG_1 ("cfg_graphics: error unloading driver %s\n ",DvDr)
		}
	}
}

/*
 * NAME: err_undo_dd 
 *
 * FUNCTION: 
 *
 * Terminates the device and unloads the driver, if the device has a driver.  If
 * the device does not have a driver, it simply returns. This routine is used
 * to back out on errors that occur while processing VPD. 
 *
 * INPUT: pointer to device name device number (devno) 
 *
 * EXECUTION ENVIRONMENT: 
 *
 * This routine is to be used only within this file.  The device specific
 * routines for the various device specific config methods must not call this
 * function. 
 *
 * NOTES: 
 *
 * void err_undo_dd( DvDr , devno ) DvDr  = pointer to device driver name. devno =
 * The device's devno. 
 *
 * RETURNS: None 
 */


STATIC void
err_undo_dd (char           *DvDr,
dev_t devno)
{
	/*
	 * make sure CuVPD is closed 
	 */

	odm_close_class (CuVPD_CLASS);

	if (strcmp (DvDr, "") != 0)
	{
		/*
		 * If device has a driver then terminate and unload it 
		 */

		err_term_dd (devno);
		err_unload_dd (DvDr);
	}
	return;
}


/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	
  
 TITLE: 	
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


STATIC void
Check_Input_Parms(	int	argc,
			char ** argv,
			char ** envp 	 )
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/


/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*------------------------------------------------------------------
	 | process the argc and argv for valid flags	 
	 |-----------------------------------------------------------------*/

	while ((c = getopt (argc, argv, "l:12D")) != EOF)
	{
		switch (c)
		{
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		case 'D':
			if (Dflag != FALSE)
				errflg++;
			Dflag = TRUE;
			break;
		case '1':
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE1;
			break;
		case '2':
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE2;
			break;
		default:
			errflg++;
		}		       /* end switch */
	}

	if (errflg != 0)
	{
		/* error parsing parameters */
		DEBUG_0 ("cfg_graphics: command line error\n")
		exit (E_ARGS);
	}

	/*--------------------------------------------------------------
	 | Validate Parameters: logical name must be specified 
	 |--------------------------------------------------------------*/

	if (logical_name == NULL)
	{
		DEBUG_0 ("cfg_graphics: logical name must be specified\n")
		exit (E_LNAME);
	}

	return;

}






/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	
  
 TITLE: 	
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


STATIC void
Open_ODM_and_Get_Objects( )
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/


/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*-----------------------------------------------------------------
	| Inputs are OK.  Now we start up the ODM 
	|-----------------------------------------------------------------*/

	DEBUG_1 ("cfg_graphics: Configuring device: %s\n", logical_name)


	if (odm_initialize () == FAIL)
	{
		/* initialization failed */
		DEBUG_0 ("cfg_graphics: odm_initialize() failed\n")
		exit (E_ODMINIT);
	}


	if ( (lock_id = odm_lock ("/etc/objrepos/config_lock", 0) ) == FAIL)
	{
		DEBUG_0 ("cfg_graphics: odm_lock() failed\n")
		err_exit (E_ODMLOCK);
	}

	DEBUG_0 ("cfg_graphics: ODM initialized and locked\n")


	/*-----------------------------------------------------------------
	| ODM is initialized and locked.
	| Now, open the customized object class and get the customized obj
	|-----------------------------------------------------------------*/

	if ((int) (p_Class_CuDv = odm_open_class (CuDv_CLASS)) == FAIL)
	{
		DEBUG_0 ("cfg_graphics: open class CuDv failed\n")
		err_exit (E_ODMOPEN);
	}


	DEBUG_1 ("cfg_graphics: calling Get_Custom_Object for device %s.\n",
		 logical_name )

	rc = Get_Custom_Object (p_Class_CuDv, 
				logical_name, 
				&s_CuDv_object);

	if (rc != PASS)
	{
		DEBUG_1 ("cfg_graphics: Get CuDv failed rc=%d.\n", rc )
		err_exit (rc);
	}

	/*----------------------------------------------------------------
	| we found the CuDv, so try to find the parent of the CuDv
	|-----------------------------------------------------------------*/

	DEBUG_0 ("cfg_graphics: calling Get_Custom_Object for parent device.\n")

	rc = Get_Custom_Object (p_Class_CuDv, 
				s_CuDv_object.parent, 
				&s_CuDv_parent);

	if (rc != PASS)
	{
		DEBUG_1 ("cfg_graphics: Get Parent device failed rc=%d.\n",
			  rc)
		err_exit (rc);
	}

	/*-----------------------------------------------------------------
	|  next, open the predefined device object class and get the 
	|  predefined device object for our device and parent
	|-----------------------------------------------------------------*/

	DEBUG_0 ("cfg_graphics: calling odm_open_class PdDv for parent .\n")

	if (((int) (p_Class_PdDv = odm_open_class (PdDv_CLASS))) == FAIL)
	{
		DEBUG_1 ("cfg_graphics: open class PdDv failed rc=%d.\n",
			  rc)
		err_exit (E_ODMOPEN);
	}

	DEBUG_1 ("cfg_graphics: searching PdDv for  %s\n", 
		  s_CuDv_object.PdDvLn_Lvalue )

	rc = Get_Predef_Object (p_Class_PdDv, 
				s_CuDv_object.PdDvLn_Lvalue,
				&s_PdDv_object);

	if (rc != PASS)
	{
		DEBUG_1 ("cfg_graphics: failed to find PdDv object rc=%d.\n",
			 rc	)
		err_exit (rc);
	}

	DEBUG_1 ("cfg_graphics: searching PdDv for  %s\n", 
		  s_CuDv_parent.PdDvLn_Lvalue )

	rc = Get_Predef_Object (p_Class_PdDv, 
				s_CuDv_parent.PdDvLn_Lvalue,
				&s_PdDv_parent);

	if (rc != PASS)
	{
		DEBUG_1 ("cfg_graphics: failed to find PdDv parent rc=%d.\n",
			 rc	)
		err_exit (rc);
	}


	/*-----------------------------------------------------------------
	|  next, open the predefined attribute object class 
	|  and the customized attribute object class 
	|-----------------------------------------------------------------*/

	if (((int) (p_Class_PdAt = odm_open_class (PdAt_CLASS))) == FAIL)
	{
		DEBUG_1 ("cfg_graphics: open class PdAt failed rc=%d.\n",
			  rc)
		err_exit (E_ODMOPEN);
	}

	if (((int) (p_Class_CuAt = odm_open_class (CuAt_CLASS))) == FAIL)
	{
		DEBUG_1 ("cfg_graphics: open class CuAt failed rc=%d.\n",
			  rc)
		err_exit (E_ODMOPEN);
	}
	/*--------------------------------------------------------------
	| Get the name of the device dependent config methods
	|---------------------------------------------------------------*/

	rc = getatt (CFGGRAPH_cfg_methods,
 		   (char) 's',			/* get attr as a "string" */
	 	   p_Class_CuAt,
		   p_Class_PdAt,
		   logical_name,
		   s_CuDv_object.PdDvLn_Lvalue,
		   "cfg_method_load",
		   NULL		) ;

	if ( rc != E_OK )
	{
		DEBUG_1 ("cfg_graphics: failed to get device dependent method name rc=%d.\n", rc)
		err_exit (rc);
	}

	return;

}


	




/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	
  
 TITLE: 	
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


STATIC void
Prepare_MCAbus_Type_Device( )
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/
int			ret_code;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*--------------------------------------------------------------
	| Get the bus id from the parent object of our device
	|---------------------------------------------------------------*/

	rc = getatt (&bus_id,
 		   (char) 'l',			/* get attr as a "long" */
	 	   p_Class_CuAt,
		   p_Class_PdAt,
		   s_CuDv_parent.name,
		   s_CuDv_parent.PdDvLn_Lvalue,
		   "bus_id",
		   NULL		) ;

	if ( rc != E_OK )
	{
		DEBUG_1 ("cfg_graphics: failed to get busid rc=%d.\n", rc)
		err_exit (rc);
	}


	/*---------------------------------------------------------
	| Get the database to get the parameters needed
	| to setup a bus class device
	|---------------------------------------------------------*/
	sprintf (sstring, 
		"parent = '%s' AND connwhere = '%s' AND status = %d",
	        s_CuDv_object.parent, 
		s_CuDv_object.connwhere, 
		AVAILABLE);

	DEBUG_1 ("cfg_graphics: check other configured dev with %s\n",
		 sstring)

	ret_code = (int) odm_get_first (p_Class_CuDv, 
				  sstring, 
				  &s_CuDv_temp);

  	if (ret_code != 0)
   	{
		/*
	 	* Error: device config'd at this location 
	 	*/
		
		DEBUG_0 ("cfg_graphics: device already AVAILABLE here\n");
		err_exit (E_AVAILCONNECT);

   	}

 	else if (ret_code == FAIL)
   	{
		/*
       		 * ODM error of some kind occurred - return that error
       		 */
		DEBUG_0("cfg_graphics: ODM error while bus check\n");
		
      		err_exit (E_ODMGET);
   	}
		
	slot = atoi (s_CuDv_object.connwhere);
	

	/*-----------------------------------------------------------
	|  If the device is an adapter being configured at RUN TIME,
	|  then we must resolve any bus attribute conflicts before
	|  configuring device to the driver. If it is not RUNTIME,
	|  then we assume the resolution has already taken place. 
	------------------------------------------------------------*/

	if (ipl_phase == RUNTIME_CFG)
	{

		/*-----------------------------------------------------
		| ensure we are located in the slot ODM thinks we have
		|-----------------------------------------------------*/

		devid = (ushort) strtol (s_PdDv_object.devid, 
					 (char **) NULL, 
					 0);

		sprintf (sstring, 
			 "/dev/%s", 
			 s_CuDv_object.parent);

		DEBUG_3 ("cfg_graphics: chkslot(%s,%d,%d)\n",
			 sstring,
			 slot,
			 devid)

		ret_code = chkslot (sstring, slot, devid);

		if (ret_code != 0)
		{
			DEBUG_2 ("cfg_graphics: card %s not found in slot %d\n",
				 logical_name,
				 slot		);
			err_exit (ret_code);
		}

		/*-----------------------------------------------------
		|  adapter is OK, so get the ID of the bus that
		|  the adapter sits on	 
		|-----------------------------------------------------*/

		ret_code = Get_Parent_Bus(p_Class_CuDv,
					s_CuDv_object.parent,
					&s_CuDv_temp );

		if (ret_code != PASS)
		{
			DEBUG_0 ("cfg_graphics: Get Parent Bus failed .\n")
			err_exit (ret_code);
		}

		/*-----------------------------------------------------
		|  We have the bus via customized object s_CuDv_parent,
		|  so now we resolve the bus addressing of the device
		|  customized object	 
		|----------------------------------------------------*/

		conflist[0] = '\0';
		not_resolved[0] = '\0';

		DEBUG_0 ("cfg_graphics: calling busresolve\n")

		ret_code = busresolve (logical_name,
				  (int) 0,
				  conflist,
				  not_resolved,
				  s_CuDv_temp.name);

		if (ret_code != 0)
		{
			DEBUG_0 ("cfg_graphics: bus resource could not be resolved\n")

			DEBUG_3 ("conflist = '%s'\nnot_resolved='%s'\nret_code=%d\n",
				 conflist,
				 not_resolved,
				 ret_code)
			err_exit (ret_code);
		}

	}		      /* end of if (ipl_phase == RUNTIME_CFG) */

		/*----------------------------------------------------------
		| Put the bus type in the dds , however, the pointer to
		| the dds has not yet been initialized, so we use the global
		| pointer to the dds.
		------------------------------------------------------------*/

	CCM_DDS_bus_type( &(__GLOBAL_DATA_PTR -> ccm_dds_struc) ) = BUS_MICRO_CHANNEL;

	/*-------------------------------------------------------------
	| Fall through to here means:
	|    status == DEFINED
	|    and parent is BUS CLASS
	|
	| Set up the FRS addressing variables in case we need to do FRS
	| checking later
	|-------------------------------------------------------------*/

	CFGGRAPH_frs_buid_type	= CFG_GRAPH_BUID_2x ;

		/*---------------------------------------
		| frs_sub_0 holds the MCA bus seg reg
		|---------------------------------------*/

	CFGGRAPH_frs_sub_0	=   ( bus_id & CDD_BUID_MASK )	/* get just BUID */
				  | CDD_SEG_T_BIT 		/* T = 1 	*/
				  | CDD_BYPASS_TCW		/* no TCWs yet	*/
				  | CDD_ADDR_CHK 
				  | CDD_ADDR_INCR;
	
		/*---------------------------------------
		| frs_sub_1 holds the MCA bus IOCC seg reg
		|---------------------------------------*/
			  
	CFGGRAPH_frs_sub_1	= CFGGRAPH_frs_sub_0 | CDD_IOCC_SELECT | CDD_ADDR_RTMODE ;

		/*----------------------------------------
		| frs_sub_2 holds the MCA slot
		|-----------------------------------------*/

	CFGGRAPH_frs_sub_2	= slot;

	return;

}




/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:  Prepare_Device
  
 TITLE: 	Prepare device for config methods.	
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


STATIC void
Prepare_Device( )
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/
int			ret_code;
char			criteria[128];
struct PdAt		pdat;
struct CuAt		cuat;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*-------------------------------------------------------------
	| Since the adapter is DEFINED rather than AVAILABLE, we need
	| to get ready to load the device driver and to call of the
	| device specific entry points associated with the completion
	| of the config process.
	|
	| Each new graphics adapter may have a full function 
	| device specific VDD and associated ucode  or it may use
	| a limited function device independent common character mode
	| VDD and associated device specific CDD and special CCM ucode.
	|
	| First, then, we need to decide which way we are going to
	| configure the device.
	| 
	|--------------------------------------------------------------*/

	if (strcmp(s_PdDv_object.DvDr, "") == 0 )
	{
		/*----------------------------------------------------
		| error if object has NO device driver
		|----------------------------------------------------*/
		
		DEBUG_0("cfg_graphics: PdDv DvDr not defined!\n")
		err_exit( E_INVATTR );	
	}

		/*-------------------------------------------------
		| save the dev driver name in the global data
		|-------------------------------------------------*/

	(void) strcpy( CFGGRAPH_dev_dd_name,
			s_PdDv_object.DvDr );

		/*---------------------------------------------------
		| get the predefined attributes for the other paths
		|----------------------------------------------------*/

	ret_code = getatt( CFGGRAPH_dev_dd_pin,
			(char) 's',
			p_Class_CuAt,
			p_Class_PdAt,
			logical_name,
			s_CuDv_object.PdDvLn_Lvalue,
			"middd_intr",
			NULL				);

	if ( ret_code != E_OK )
	{
		DEBUG_0("cfg_graphics: getatt middd interrupt routine failed\n")	
	}


	ret_code = getatt( CFGGRAPH_dev_ucode,
			(char) 's',
			p_Class_CuAt,
			p_Class_PdAt,
			logical_name,
			s_CuDv_object.PdDvLn_Lvalue,
			"microcode",
			NULL				);

	if ( ret_code != E_OK )
	{
		DEBUG_0("cfg_graphics: getatt dev microcode failed\n")	
	}



	ret_code = getatt( CFGGRAPH_ccm_ucode,
			(char) 's',
			p_Class_CuAt,
			p_Class_PdAt,
			logical_name,
			s_CuDv_object.PdDvLn_Lvalue,
			"ccm_microcode",
			NULL				);

	if ( ret_code != E_OK )
	{
		DEBUG_0("cfg_graphics: getatt ccm microcode failed\n")	
	}

	
	(void) strcat( (strcpy( CCM_DDS_ucode_filename( &(__GLOBAL_DATA_PTR -> ccm_dds_struc) ),
			"/usr/lib/microcode/")), CFGGRAPH_ccm_ucode);

	ret_code = getatt( CFGGRAPH_ccm_dd_name,
			(char) 's',
			p_Class_CuAt,
			p_Class_PdAt,
			logical_name,
			s_CuDv_object.PdDvLn_Lvalue,
			"ccm_dd",
			NULL				);

	if ( ret_code != E_OK )
	{
		DEBUG_0("cfg_graphics: getatt ccm dd failed\n")	
	}


	ret_code = getatt(  CFGGRAPH_ccm_dd_pin,
			(char) 's',
			p_Class_CuAt,
			p_Class_PdAt,
			logical_name,
			s_CuDv_object.PdDvLn_Lvalue,
			"ccmdd_intr",
			NULL				);

	if ( ret_code != E_OK )
	{
		DEBUG_0("cfg_graphics: getatt ccmdd interrupt routine failed\n")	
	}


	ret_code = getatt( CFGGRAPH_cdd_name,
			(char) 's',
			p_Class_CuAt,
			p_Class_PdAt,
			logical_name,
			s_CuDv_object.PdDvLn_Lvalue,
			"cdd_kmod_entry",
			NULL				);

	if ( ret_code != E_OK )
	{
		DEBUG_0("cfg_graphics: getatt cdd kmod entry failed\n")	
	}


	ret_code = getatt( CFGGRAPH_cdd_pin,
			(char) 's',
			p_Class_CuAt,
			p_Class_PdAt,
			logical_name,
			s_CuDv_object.PdDvLn_Lvalue,
			"cdd_kmod_inter",
			NULL				);

	if ( ret_code != E_OK )
	{
		DEBUG_0("cfg_graphics: getatt cdd kmod interrupt failed\n")	
	}


	/*----------------------------------------------------------
	| we now do the comparisons of these various parameters,
	| to determine the logical state of the various components
	|------------------------------------------------------------*/

		/*-------------------------------------------
		|	Full Function Device Driver
		|	Full Function Config Method
		|-------------------------------------------*/

	if (    
		(0 == (strcmp( CFGGRAPH_dev_dd_name , "") ) )
	     || (DD_Not_Found(  CFGGRAPH_dev_dd_name  ) )
	     || (0 == (strcmp( CFGGRAPH_cfg_methods , "") ) )
	     || (Method_Not_Found(  CFGGRAPH_cfg_methods  ) )
	   )	
	{
		CFGGRAPH_use_dev_vdd	 = FALSE;
	}
	else
	{
		CFGGRAPH_use_dev_vdd	 = TRUE;
	}

			
		/*-------------------------------------------
		|	Full Function Microcode
		|-------------------------------------------*/

	if ( 0 == (strcmp( CFGGRAPH_dev_ucode , "") ) ) 
	      
	{
		CFGGRAPH_use_device_ucode = TRUE;
	}
	else
	       
	{
		CFGGRAPH_use_device_ucode = ( ! (Ucode_Not_Found( CFGGRAPH_dev_ucode ) ) ); 
	}


		/*-------------------------------------------
		|	CCM Device Driver
		|-------------------------------------------*/

	if (    ( 0 == (strcmp( CFGGRAPH_ccm_dd_name , "") ) )
	     || (DD_Not_Found(  CFGGRAPH_ccm_dd_name  ) )
	   )	
	{
		CFGGRAPH_use_ccm_vdd = FALSE;
	}
	else
	{
		CFGGRAPH_use_ccm_vdd = TRUE;
	}

		/*-------------------------------------------
		|	CCM Microcode
		|-------------------------------------------*/

	if ( 0 == (strcmp( CFGGRAPH_ccm_ucode , "" ) ) )
	{
		CFGGRAPH_use_ccm_ucode = TRUE;
	}
	else
	{
		CFGGRAPH_use_ccm_ucode = ( ! (Ucode_Not_Found( CFGGRAPH_ccm_ucode ) ) ); 
	}

	/*---------------------------------------------------------
	| Next we resolve the order of precedence of the flags 
	| set above.  In the end, one pair will be true and 
	| one pair will be false, or there is an error.
	|----------------------------------------------------------*/

	if (   (CFGGRAPH_use_device_ucode == TRUE )
	    && (CFGGRAPH_use_dev_vdd      == TRUE ) )
	{
		/*---------------------------------------------------
		| we need both dev pieces to use full function
		|----------------------------------------------------*/

		CFGGRAPH_use_ccm_vdd	= FALSE;
		CFGGRAPH_use_ccm_ucode	= FALSE;
	}
	else if (    (CFGGRAPH_use_ccm_ucode == TRUE)
		  && (CFGGRAPH_use_ccm_vdd   == TRUE)  )
	{
		/*--------------------------------------------------
		| we need both ccm pieces for ccm function
		|--------------------------------------------------*/

		CFGGRAPH_use_device_ucode = FALSE;
		CFGGRAPH_use_dev_vdd      = FALSE;

	}
	else
	{
		/*--------------------------------------------------
		| this is an error condition
		|--------------------------------------------------*/

		DEBUG_0("cfg_graphics: attr mismatch on ccm select\n")
		err_exit(  E_BADATTR );
	}

	
	/*--------------------------------------------------
	| If we are an MCA device and not an 0x8ee3 adapter,
	| then we need to do some special processing here.
	| The reason we don't care about the 0x8ee3 MCA
	| adapter here is because that adapter is CCM
	| capable but not FRS capable. It is a special
	| animal and no other adapter is quite like it
	| in this respect. BTW - See CMVC defect #155298
	| for details of what is being added in this
	| section for MCA FRS capable adapters...
	|--------------------------------------------------*/

	if ( (!(strcmp(s_PdDv_object.subclass, "mca"))) &&
	     (strcmp(s_PdDv_object.devid, "0xe38e")) ) {

		/*--------------------------------------------------
		| If the PdAt "frs" attribute does not exist for
		| this device instance, then we need to create
		| one. This will help out the ISV's since they
		| are not required to modify their .add file to
		| have the "frs" PdAt attribute, but they should.
		| If they don't, this will fix them, but only
		| after the second reboot since by the time we're
		| here, it is too late to go back and make cfgbus
		| allocate the MCA bus memory space needed.
		|--------------------------------------------------*/
		
		if ( !(getattr (s_CuDv_object.name, "frs",
				TRUE, &ret_code)) ){

			strcpy(pdat.uniquetype, s_CuDv_object.PdDvLn);
			strcpy(pdat.attribute, "frs");
			*pdat.deflt = '\0';
			*pdat.values = '\0';
			*pdat.width = '\0';
			strcpy(pdat.type, "R");
			*pdat.generic = '\0';
			strcpy(pdat.rep, "sl");
			pdat.nls_index = 0;

			if (odm_add_obj(PdAt_CLASS, &pdat) < 0) {
				DEBUG_0("cfg_graphics: Can't create FRS attribute in the PdAt.\n");
			}
		}

		if ( CFGGRAPH_use_dev_vdd == TRUE ){

			/*--------------------------------------------------
			| If we're running the real device driver, make sure
			| the CuAt "frs" attribute exists.
			|--------------------------------------------------*/

			strcpy(cuat.name, s_CuDv_object.name);
			strcpy(cuat.attribute, "frs");
			*cuat.value = '\1';
			strcpy(cuat.type, "R");
			strcpy(cuat.generic, "D");
			strcpy(cuat.rep, "sl");
			cuat.nls_index = 0;

			if (putattr (&cuat)) {
				DEBUG_0("cfg_graphics: Can't create FRS attribute in the CuAt.\n");
			}

		}else{	/* CFGGRAPH_use_dev_vdd == FALSE */

			/*--------------------------------------------------
			| If we're running the CCM device driver, make sure
			| the CuAt "frs" attribute does not exist.
			|--------------------------------------------------*/

			sprintf(criteria, "name = '%s' AND attribute = 'frs'",
				s_CuDv_object.name);

			odm_rm_obj (CuAt_CLASS, criteria);
		}
	}
	
	return;

}
			


/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:  Prepare_Feature_ROM_Files
  
 TITLE: 	
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


STATIC void
Prepare_Feature_ROM_Files( )
{
/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/

RSCAN_VIDEO_HEAD *	mem_head ;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	memset( tmp_cfg_meth , '\0' , sizeof( tmp_cfg_meth ));
	memset( tmp_cdd      , '\0' , sizeof( tmp_cdd      ));
	memset( tmp_cdd_pin  , '\0' , sizeof( tmp_cdd_pin  ));
	memset( tmp_ucode    , '\0' , sizeof( tmp_ucode    ));

	/*-----------------------------------------------------------------
	 | Call FRS_Find_Video_Device() to check to see if we have a video ROS device 
	 |
	 | We are going to make the setup for the call differently depending
	 | on the bus type ( MICROCHANNEL, BUID40, 7F (601 bus) )
	 | 
	 | The Prepare_XXX_Class_Device routine should have already set
	 | up the frs global data for use by this routine
	 |
	 | Preset the CFGGRAPH_device_uses_VIDEO_ROS flag prior to testing
	 |----------------------------------------------------------------*/

	CFGGRAPH_device_uses_VIDEO_ROS = FALSE;

	rc = FRS_Find_Video_Device( 	  CFGGRAPH_frs_buid_type , 
			   		  &mem_head,
			   		  CFGGRAPH_frs_sub_0 ,
			   		  CFGGRAPH_frs_sub_1 ,
			   		  CFGGRAPH_frs_sub_2	);
	switch ( rc )
	{
	case E_OK:		/* found one */

		CFGGRAPH_device_uses_VIDEO_ROS = TRUE;

		p_video_head = (void *) mem_head;

		break;

	case E_NODETECT:	/* did not find one */
		return;

	default:		/* error return n*/
		DEBUG_1("cfg_graphics: FRS_Find_Video_Device return rc =%d\n", rc );
		err_exit ( rc );
	}

	/*-----------------------------------------------------------------
	| Fall through to here means we found a good FRS adapter
	|
	| We extract the following files from the ROM contents
	| There are two sets, required ones and optional ones
	| The required ones are:
	|	- cdd load module
	|
	| The optional ones are:
	|	- device dependent cfg_method load module
	|	- cdd_pin load module
	|	- microcode
	|
	| In all cases, if the ODM attribute is non-empty, then we assume
	| that the filesystem contains a valid file.  The only case where
	|
	|-----------------------------------------------------------------*/
		/*---------------------------------------
		| Process required file cdd
		|----------------------------------------*/

	rc = FRS_Make_Temp_File( mem_head,
				CFG_GRAPH_CDD_FILE,
				CFG_GRAPH_PERMISSIONS,
				&tmp_cdd );

        DEBUG_1("cfg_graphics: make CDD file, name =<%s>\n",tmp_cdd);

	if ( rc != E_OK )
	{
		DEBUG_1("cfg_graphics: FRS_Make_Temp_File failed rc = %x\n",rc);
		err_exit( rc );
	}
	
        /*
         * if we didn't find the cdd specified in .add in the file system, then
         * use the cdd extracted from ROM above.
         */

        if ( (0 == strcmp( CFGGRAPH_cdd_name , "" )) ||
              DD_Not_Found (CFGGRAPH_cdd_name)          )
        {
                DEBUG_1("cfg_graphics:Prepare_Feature_ROM_File: use ROM cdd file = %s\n",tmp_cdd);
                strcpy( CFGGRAPH_cdd_name , tmp_cdd );
        }

		/*---------------------------------------
		| Process optional file cfg_method
		|----------------------------------------*/

	if (    ( RSCAN_VIDEO_HEAD_kmod_len(    mem_head, VRS_KMOD_CFG_METHOD ) != 0 )
	     && ( RSCAN_VIDEO_HEAD_kmod_offset( mem_head, VRS_KMOD_CFG_METHOD ) != 0 ) )
	{
		rc = FRS_Make_Temp_File( mem_head,
					CFG_GRAPH_METHOD_FILE,
					CFG_GRAPH_PERMISSIONS,
					&tmp_cfg_meth );

                DEBUG_1("cfg_graphics: make method file, name = <%s>\n", tmp_cfg_meth );

		if ( rc != E_OK )
		{
			DEBUG_1("cfg_graphics: FRS_Make_Temp_File failed rc = %x\n",rc);
			err_exit( rc );
		}
		
		if ( 0 == strcmp( CFGGRAPH_cfg_methods , "" ))
		{
			strcpy( CFGGRAPH_cfg_methods , tmp_cfg_meth );
		}
	}

		/*---------------------------------------
		| Process optional file cdd pin
		|----------------------------------------*/

	if (    ( RSCAN_VIDEO_HEAD_kmod_len(    mem_head, VRS_KMOD_INTERRUPT ) != 0 )
	     && ( RSCAN_VIDEO_HEAD_kmod_offset( mem_head, VRS_KMOD_INTERRUPT ) != 0 ) )
	{
		rc = FRS_Make_Temp_File( mem_head,
					CFG_GRAPH_CDD_PIN_FILE,
					CFG_GRAPH_PERMISSIONS,
					&tmp_cdd_pin );

                DEBUG_1("make CDD PIN file, name = <%s>\n",tmp_cdd_pin );

		if ( rc != E_OK )
		{
			DEBUG_1("cfg_graphics: FRS_Make_Temp_File failed rc = %x\n",rc);
			err_exit( rc );
		}
		
		if ( 0 == strcmp( CFGGRAPH_cdd_pin , "" ))
		{
			strcpy( CFGGRAPH_cdd_pin , tmp_cdd_pin );
		}
	}
	
		/*---------------------------------------
		| Process optional file ucode
		|----------------------------------------*/

	if (    ( RSCAN_VIDEO_HEAD_ucode_len(    mem_head ) != 0 )
	     && ( RSCAN_VIDEO_HEAD_ucode_offset( mem_head ) != 0 ) )
	{
		rc = FRS_Make_Temp_File( mem_head,
					CFG_GRAPH_UCODE_FILE,
					CFG_GRAPH_PERMISSIONS,
					&tmp_ucode );

                DEBUG_1("make UCODE file, name =<%s>\n",tmp_ucode );

		if ( rc != E_OK )
		{
			DEBUG_1("cfg_graphics: FRS_Make_Temp_File failed rc = %x\n",rc);
			err_exit( rc );
		}
		
		if ( 0 == strcmp( CFGGRAPH_ccm_ucode , "" ))
		{
			strcpy( CFGGRAPH_ccm_ucode , tmp_ucode );
			strcpy( CCM_DDS_ucode_filename( &(__GLOBAL_DATA_PTR -> ccm_dds_struc) ),
					CFGGRAPH_ccm_ucode);
		}
	}

        DEBUG_0("Leave Prepare_Feature_ROM_Files, rc = 0\n");
	
	return;

}




/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	
  
 TITLE: 	
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


STATIC void
Prepare_SGAbus_Type_Device( )
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/
int			ret_code;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*--------------------------------------------------------------
	| Get the bus id from our devices parent
	|---------------------------------------------------------------*/

	rc = getatt (&bus_id,
 		   (char) 'l',			/* get attr as a "long" */
	 	   p_Class_CuAt,
		   p_Class_PdAt,
		   s_CuDv_parent.name,
		   s_CuDv_parent.PdDvLn_Lvalue,
		   "bus_id",
		   NULL		) ;

	if ( rc != E_OK )
	{
		DEBUG_1 ("cfg_graphics: failed to get busid rc=%d.\n", rc)
		err_exit (rc);
	}


	/*----------------------------------------------------------
	| Put the bus type in the dds , however, the pointer to
	| the dds has not yet been initialized, so we use the global
	| pointer to the dds.
	------------------------------------------------------------*/

	CCM_DDS_bus_type( &(__GLOBAL_DATA_PTR -> ccm_dds_struc) ) = BUS_NONE;


	/*-------------------------------------------------------------
	| Fall through to here means:
	|    status == DEFINED
	|    and parent is SGABUS CLASS
	|
	| Set up the FRS addressing variables in case we need to do FRS
	| checking later
	|-------------------------------------------------------------*/

	CFGGRAPH_frs_buid_type	= CFG_GRAPH_BUID_40 ;

		/*---------------------------------------
		| frs_sub_0 , 1, 2 hold 0
		|---------------------------------------*/

	CFGGRAPH_frs_sub_0	=  0;
			  
	CFGGRAPH_frs_sub_1	=  0;
			  
	CFGGRAPH_frs_sub_2	=  0;

	return;

}





/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	
  
 TITLE: 	
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


STATIC void
Process_Device_Customized_VPD( )
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/


/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/



	memset( odm_vpd, 0, sizeof(odm_vpd) );

	DEBUG_0("cfg_graphics: Calling query_vpd()\n")
	
	/*-------------------------------------------------
	| call the device specific VPD routine first.
	| this routine will create device specific VPD
	| and will copy it into the private VPD
	|
	| the device specific device drivers actually ar implemented
	| with query_vpd functions that accept the first four of these
	| parameters.  The fifth is an extension to the arch.  The
	| device specific query vpd semantics is to copy all the
	| the VPD found into the odm_vpd string.  This must
	| be preserved by the CCM query vpd routine.
	|
	| However, the ccm vpd routine must also fill out the CCM_VPD_T
	| structure before returning.  It should then strcat ALL
	| of the VPD from all of the units of the ccm_vpd_t struc
	| into the odm_vpd string.
	|
	| It is expected that all new device dependent query vpd 
	| routines will also use the ccm_vpd_t structure in their
	| internals.
	|
	|-------------------------------------------------*/

	rc = CFGGRAPH_query_vpd( &s_CuDv_object, 
				 cfg_dd.kmid, 
				 cfg_dd.devno, 
				 odm_vpd,
				 &CFGGRAPH_vpd		);

	if ( rc  ) 
	{
		DEBUG_1("cfg_graphics: error getting VPD, rc=%d\n",rc)
		err_undo_dd( CFGGRAPH_ddname , cfg_dd.devno );

		if (    ( rc < 0 )
		     || ( rc > 255 ) )
		{	err_exit( E_VPD );	}
		else
		{	err_exit( rc );		}
	}

	DEBUG_0("cfg_graphics: Returned from query_vpd()\n")

	/*---------------------------------------------------
	| Now process the returned vpd into a useable form
	| Open the ODM class
	| Try to get a customized VPD object
	| If there was not one, create one and store it.
	| If there was one, check to see if the vpd changed.
	| If the vpd changed, update the existing object.
	| Close the class.
	|---------------------------------------------------*/

	p_Class_vpd = odm_open_class(CuVPD_CLASS);

	if ( ( (int) p_Class_vpd ) == FAIL ) 
	{
		DEBUG_0("cfg_graphics: open class CuVPD failed");
		err_undo_dd( CFGGRAPH_ddname , cfg_dd.devno );
		err_exit(E_ODMOPEN);
	}

	/* search for customized vpd object with this logical name */
	sprintf(sstring, "name = '%s' and vpd_type = '%d'",
		logical_name,HW_VPD);

	rc = (int)odm_get_first( p_Class_vpd,
				sstring,
				&s_CuVPD_object );

	if ( rc == FAIL ) 
	{
		DEBUG_0("cfgdevice: ODM failure getting CuVPD object");
		err_undo_dd( CFGGRAPH_ddname , cfg_dd.devno );
		err_exit(E_ODMGET);
	}

	if ( rc == 0 ) 
	{
		/*--------------------------------------------
		|  No customized VPD object was found, so
		|  add one now
		|---------------------------------------------*/

		DEBUG_0 ("cfg_graphics: Adding new VPD object\n")

		(void )strcpy( s_CuVPD_object.name , logical_name);

		s_CuVPD_object.vpd_type = HW_VPD;

		memcpy(s_CuVPD_object.vpd , odm_vpd, sizeof(odm_vpd));

		rc = odm_add_obj( p_Class_vpd,
				  &s_CuVPD_object	); 

		if ( rc == -1) 
		{
			DEBUG_0("cfggraohics: ODM failure adding CuVPD object")
			err_undo_dd( CFGGRAPH_ddname , cfg_dd.devno );
			err_exit(E_ODMADD);
		}
		DEBUG_0("Successfully added new VPD object\n");

	} 
	else 
	{
		/*----------------------------------------------
		| An object was found.  See if it needs to be
		| updated with new vpd
		|----------------------------------------------*/

		rc = memcmp( s_CuVPD_object.vpd,
			     odm_vpd,
			     sizeof(odm_vpd) 	);

		if ( rc != E_OK )
		{
			DEBUG_0("cfg_graphics: Updating VPD object\n");

			memcpy(s_CuVPD_object.vpd,
				odm_vpd,
				sizeof(odm_vpd)	);

			rc =  odm_change_obj( p_Class_vpd,
					    &s_CuVPD_object 	);
			if ( rc  == FAIL ) 
			{
				DEBUG_0("cfgdevice: ODM failure updating CuVPD object")
				err_undo_dd( CFGGRAPH_ddname , cfg_dd.devno );
				err_exit(E_ODMUPDATE);
			}
			DEBUG_0("Successfully updated VPD object\n");
		}
	}

	/* close customized vpd object class */

	rc =  odm_close_class(CuVPD_CLASS);

	if ( rc == FAIL ) 
	{
		DEBUG_0("cfgdevice: error closing CuVPD object class\n");
		err_undo_dd( CFGGRAPH_ddname , cfg_dd.devno );
		err_exit(E_ODMCLOSE);
	}

	return;

}




/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	
  
 TITLE: 	
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


STATIC void
Set_Up_CCM_DD_Case( )
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/


/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/


	/*-------------------------------------------
	| set up the CDD stuctures in the global data
	|--------------------------------------------*/

		/*---- init the cdd header pointers ----*/
	CDD_command(	CFGGRAPH_cdd )	= &(__GLOBAL_DATA_PTR -> cdd_attrs);
	CDD_device(	CFGGRAPH_cdd )	= &(__GLOBAL_DATA_PTR -> cdd_device);
	CDD_procs(	CFGGRAPH_cdd )	= &(__GLOBAL_DATA_PTR -> cdd_procs);
	CDD_svcs(	CFGGRAPH_cdd )	= &(__GLOBAL_DATA_PTR -> cdd_svcs);
	CDD_ext(	CFGGRAPH_cdd ) 	= NULL;
	CDD_version( 	CFGGRAPH_cdd )	= CDD_CURRENT_SW_VERSION;

		/*---- init some device fields ----*/

        if ((( bus_id & CFG_GRAPH_BUID_MASK ) == CFG_GRAPH_BUID_2x_MASK ) ||
            (( bus_id & CFG_GRAPH_BUID_MASK ) == CFG_GRAPH_BUID_40_MASK )
           )
        {
		CDD_busmem_seg(CFGGRAPH_cdd) = 	bus_id 		|
						CDD_SEG_T_BIT	|
       						CDD_BYPASS_TCW 	|
        					CDD_ADDR_CHK  	|
        					CDD_ADDR_INCR	;


		CDD_iocc_seg(CFGGRAPH_cdd) = 	bus_id 		|
        					CDD_ADDR_CHK  	|
        					CDD_ADDR_INCR	|
     						CDD_IOCC_SELECT	|
       						CDD_BYPASS_TCW 	|
        					CDD_ADDR_RTMODE	;

		CDD_iocc_base(CFGGRAPH_cdd) =	CDD_IOCC_BASE;

		CCM_DDS_bus_id( CFGGRAPH_ccm_dds ) = bus_id;
	}
        else if( ( bus_id & CFG_GRAPH_60x_BUID_MASK ) == CFG_GRAPH_BUID_7F_MASK )
        {
                CDD_60x_segment(CFGGRAPH_cdd) = CDD_60x_32BIT_SET_T_BIT |
                                                CDD_60x_32BIT_PROB_ST_KEY |
                                                bus_id;


		/* 
		   The cdd version number is being overridden here. 
		   We have to do this because an earlier version 
		   of the the bbl cdd thinks it is at level 0x110. 
                   Because of other problems with using a version 
		   number, the check for version number in the cdd is 
		   being removed at the same time this work-around is 
		   implemented.  So, later versions of the bbl cdd don't 
		   care what version they are. Thus it is OK to have this 
		   hard-coded value here, since the only bbl cdd code 
		   that will check it is the version with this incorrect 
		   number. What a hack...
		*/

		CDD_version( 	CFGGRAPH_cdd )	= 0x110;

                CDD_busmem_seg(CFGGRAPH_cdd)    = NULL;
                CDD_iocc_seg(CFGGRAPH_cdd)      = NULL;
                CDD_iocc_base(CFGGRAPH_cdd)     = NULL;

                CCM_DDS_60x_segment( CFGGRAPH_ccm_dds ) = bus_id;
        }
        else
        {
                DEBUG_0("cfg_graphics: bus id not recognized\n")
                err_exit(  E_BADATTR );
        }

	p1 = (void *) malloc( CDD_DDF_STANDARD_SIZE );
	if ( p1 == NULL)
	{	err_exit( E_MALLOC );	}

	CDD_ddf(	CFGGRAPH_cdd )	= p1;
	CDD_ddf_len(	CFGGRAPH_cdd )	= CDD_DDF_STANDARD_SIZE;

		/*----	no reason to init command fields ----*/
		/*----  no reason to init procs fields ----*/

		/*----	init the svcs fields ----*/
	CDD_svcs( CFGGRAPH_cdd ) -> i_enable	= (void (*)()) cfg_svcs_error;
	CDD_svcs( CFGGRAPH_cdd ) -> i_disable	= cfg_svcs_error;
			
	CDD_svcs( CFGGRAPH_cdd ) -> lockl	= cfg_svcs_error;
	CDD_svcs( CFGGRAPH_cdd ) -> unlockl	= (void (*)()) cfg_svcs_error;
			
	CDD_svcs( CFGGRAPH_cdd ) -> strncmp	= strncmp;

	CDD_svcs( CFGGRAPH_cdd ) -> bus_get_l	= cfg_svcs_bus_get_l ;
	CDD_svcs( CFGGRAPH_cdd ) -> bus_get_s	= cfg_svcs_bus_get_s ;
	CDD_svcs( CFGGRAPH_cdd ) -> bus_get_c	= cfg_svcs_bus_get_c ;
	CDD_svcs( CFGGRAPH_cdd ) -> bus_put_l	= cfg_svcs_bus_put_l ;
	CDD_svcs( CFGGRAPH_cdd ) -> bus_put_s	= cfg_svcs_bus_put_s ;
	CDD_svcs( CFGGRAPH_cdd ) -> bus_put_c	= cfg_svcs_bus_put_c ;
	CDD_svcs( CFGGRAPH_cdd ) -> bus_cpy	= cfg_svcs_bus_cpy ;

#ifdef DEBUG
	CDD_svcs( CFGGRAPH_cdd ) -> printf	= printf;             /* used by cdd for debug */
#endif
			
	if ( ( bus_id & CFG_GRAPH_BUID_MASK ) == CFG_GRAPH_BUID_40_MASK )
	{
		CFGGRAPH_write_opcode = MSLAPUT;
		CFGGRAPH_read_opcode = MSLAGET ;
	}
	else if( ( bus_id & CFG_GRAPH_BUID_MASK ) == CFG_GRAPH_BUID_2x_MASK )
	{
		CFGGRAPH_write_opcode = MIOBUSPUT; 
		CFGGRAPH_read_opcode = MIOBUSGET;
	}
 	else if( ( bus_id & CFG_GRAPH_60x_BUID_MASK ) == CFG_GRAPH_BUID_7F_MASK )
        {

                CFGGRAPH_write_opcode = 0;          /* Not used */
                CFGGRAPH_read_opcode =  0;

        }

	/*------------------------------------------------
        | fall through to here means we have validated the
        | system flags.  We need to load the cdd load module
	| as a part of the config method, not as a device
	| driver.  The code for he cdd's will use the services
	| allocated just above to access the VPD from user
	| space.  This will allow us to get the VPD prior
	| to loading a device driver.  
        | if one is in the filesystem. If we don't find one 
	| there we look on the adapter for the Video ROM.
        |
        | If the VIDEO_ROM feature is enabled, we need to
        | obtain the two routines from the adapter and write
        | them into a file.
        |
        | In either case, we need to use the "load" command
        | to bring in the cdd and link the code
        | with this config method.
        |-------------------------------------------------*/

	DEBUG_1 ("cfg_graphics: cdd name = %s\n ",CFGGRAPH_cdd_name)

	if ( 
	       ( ( strcmp ( CFGGRAPH_cdd_name , "" ) ) != 0 ) 	/* if there is a name 	*/
	    && ( ! (DD_Not_Found ( CFGGRAPH_cdd_name ) ) ) 	/* and we found the file */
	   )
	{
		/*----------------------------------------------
		| convert to full path if not already 
		|-----------------------------------------------*/

		if (      ( strncmp( "./",  CFGGRAPH_cdd_name, 2 ))
		      &&  ( strncmp( "../", CFGGRAPH_cdd_name, 3 ))
		      &&  ( CFGGRAPH_cdd_name[ 0 ] != '/' )	 	)
		{
	 		(void) strcat( ( strcpy( path1, "/usr/lib/drivers/") ),
			    		 CFGGRAPH_cdd_name );
		}
		else
		{
			strcpy( path1, CFGGRAPH_cdd_name );
		}

		/*------------------------------------------------------
		| Will normally expect the load modules to already exist in the
		| same directory with the device driver.  Thus, we just
		| load from the driver
		|-------------------------------------------------------*/

		CDD_procs( CFGGRAPH_cdd ) -> entry_point = load( path1, 0, NULL );
							
		if ( CDD_procs( CFGGRAPH_cdd)->entry_point == NULL )
		{
			DEBUG_0("cfg_graphics: cdd load failed on entry_point\n")
			err_exit( E_LOADEXT );
		}

	}
	else
	{
		DEBUG_0("cfg_graphics: did not find CDD file\n")
		err_exit(  E_BADATTR );
	} /*end of CDD  load */


	/*--------------------------------------------------------------
	| fall through to here means we have set up the CDD and have
	| loaded the kmod successfully into this user space process.
	|--------------------------------------------------------------*/
							
	return;

}


STATIC void
Prepare_601bus_Type_Device( )            /* 601_CHANGE */
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine uses the data that are defined in the
	| "main" procedure in cfg_graphics.c.  It is made into
	| a function call only to help break up a long sequence
	| of code.
	|-----------------------------------------------------*/
int			ret_code;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*--------------------------------------------------------------
	|
	|    Get the bus id of the 601 bus in the ODM data base (bbl.add) 
	|    In plan 2 versions of Baby Blue, 1M and 3M.  The 1M version 
        |    has 1 device id (4001) and the 3M has another device id (4005)
	|    Both run on the Rain Bow.  These two are repackaging to run
        |    on RS2G also.  However each will have unique device id.
 	| 
	|    Since each one will have unique id, it is possible to hav PdAt
	|    describes the type of buses they are attached to.  This information
	|    in turn is used to determine the platform (H/W) we run on.
	|    We (configuration) need this information to read the Feature ROM.
	|    Mainly for coding the mvran_read/write in bytes/words 
	|    (see cfg_graphics_frs_tools.c)
	|
	|    We could easilly get the platform informatin from the processor
	|    structure in the iplcb or using the macros __power_xx in 
	|    R2/inc/sys/sytemcfg.h (assuming it is possible to use thes macros 
	|    in application space) 
	|
	|    For now just add a PdAt for "bus_id" in each .add file for each version
	|    On Rain Bow it is 0x87f0 0000; on RS2G it is 0x8100 0000 ??
	|    unless we choose other means
	|
	|---------------------------------------------------------------*/

	rc = getatt (&bus_id,
 		   (char) 'l',			/* get attr as a "long" */
	 	   p_Class_CuAt,
		   p_Class_PdAt,
		   logical_name,
		   s_CuDv_object.PdDvLn_Lvalue,
		   "bus_id",
		   NULL		) ;


	if ( rc != E_OK )
	{
		DEBUG_1 ("cfg_graphics:Prepare_601bus_Type_Device: failed to get bus id rc=%d.\n", rc)
		err_exit (rc);
	}

	DEBUG_1 ("cfg_graphics:Prepare_601bus_Type_Device:  bus id =%x\n", bus_id);

	/*----------------------------------------------------------
	| Put the bus type in the dds , however, the pointer to
	| the dds has not yet been initialized, so we use the global
	| pointer to the dds.
	------------------------------------------------------------*/

	CCM_DDS_bus_type( &(__GLOBAL_DATA_PTR -> ccm_dds_struc) ) = CCM_BUS_TYPE_60x;


	/*-------------------------------------------------------------
	| Fall through to here means:
	|    status == DEFINED
	|    and parent is 601 BUS CLASS
	|
	| Set up the FRS addressing variables in case we need to do FRS
	| checking later
	|-------------------------------------------------------------*/


	CFGGRAPH_frs_buid_type	= CFG_GRAPH_BUID_7F ;



	/* 

   	   601 device system specific registers locate at 0xFF00 0FFF and  
   	   Feature/VPD ROMs locate anywhere from 0xFFA0 0000 0xFFBF FFFF.  

   	   Since system addressing is segment based, four bits are used for 
           segment number and the other 28 bits are used as offset.  To 
           compensate for the 4 bits used as segment nunber, one of the field 
           in the segment content, bits 28-31, can be used to store bits 0-3 
           of the real address above.  The system will concatenate the 
           overflow bits (29-31 from segment content) with the 28 bits of 
           effective address to generate 32 bit real address on the bus 

             0   1    2              12        16           24        28      31
           ---------------------------------------------------------------------
           | T | Ks | Kp |   BUID   | 0 |0|0|0|0|   auth.  | I |0|0|0| addr    |
           |   |    |    | (9 bits) |   | | | | | (7 bits) |   | | | | (4 bits)|
           ---------------------------------------------------------------------

        */
                                              
        /* 
         *  Note I assume that bus_id has this value 0x87f00000
         *  We don't use this value to do actual io, but to tell us 
         *  which type of machine Baby Blue is runing on.  It is mainly
         *  used by the nvram_read_words an nvram_read_bytes (see cfg_graphics_frs_tools.c).
         *
         *  The actual io is done by the machine driver with the new ioctl commands, MIOVPDGET,
         *  MIOCFGGET, and MIOCFGPUT (see design for Feature 62524).  Note these ioctl commands
         *  only works on  601 power PC (specicially bus 7F)
         */

	CFGGRAPH_frs_sub_0 = bus_id;

	/* 
         * Baby Blue is on processor bus so IOCC setup here
         */
	CFGGRAPH_frs_sub_1 =  0;     
			  

	/* 
   	 * Bus Slot Config Register Definition (0xff00000c):
   	 * ------------------------------------------------
	 *
         *	0    : configuration enable

         *	          0 - inactivate all CONFIG signals
         *	          1 - activate appropriate CONFIG as descrived in bits 29 - 31
         *
   	 *	1-28 : reserved 
         *
         *	29-31: configuration field

         *	          000 - not implemented
         *	          001 - not implemented
         *	          010 - CONFIG_(2) signal driven active to Memory Controller
         *	          011 - CONFIG_(3) signal driven active to AEGEAN IOCC 
         *	          100 - CONFIG_(4) signal driven active to graphics slot 
         *	          101 - CONFIG_(5) signal driven active (reserved) 
         *	          11x - reserved
         *
         *      References:  Power PC 23 Bit Arch and Rain Bow 3 Enginerring Work Book
         *
         *      Note the value which goes to bits 29 to 31 of Bus Slot Select Config Register
         *      is called the slot number.  The machine driver sets up this register when
         *      we issue MIOCFGGET, MIOCFGPUT, and MIOVFGET.  All we have to pass is the slot #. 
	 */

        slot = atoi(s_CuDv_object.connwhere);    /* Get slot number from CuDv (cfgsys created it) */

	if ( slot != 4 )
	{
		DEBUG_1 ("cfg_graphics:Prepare_601bus_Type_Device:incorrect slot for graphics, slot=%d.\n", slot);
		err_exit (-1);
	}

	DEBUG_1 ("cfg_graphics:Prepare_601bus_Type_Device: graphic slot = %d.\n", slot);

	CFGGRAPH_frs_sub_2 =  slot;  

	return;

}
