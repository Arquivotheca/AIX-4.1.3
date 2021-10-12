static char sccsid[] = "@(#)48	1.36.2.8  src/bos/usr/lib/methods/cfgsys/cfgsys.c, cfgmethods, bos411, 9428A410j 6/3/94 17:33:45";
/*
 *   COMPONENT_NAME: (CFGMETHODS) System Configuration Method
 *
 *   FUNCTIONS: close_odm_and_terminate
 *		copy_readable
 *		detect_device
 *		err_exit
 *		get_dev_objs
 *		get_key
 *		get_sysplanar_vpd
 *		main
 *		mdd_get
 *		open_odm_and_init
 *		parse_params
 *		process_buc_no_table
 *		process_buc_table
 *		process_non_buc_all_phase
 *		process_non_phase_1
 *		put_nvram
 *		set_attrs
 *		setattr
 *		update_cudv
 *		update_vpd
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* interface:
   cfgsys -l <logical_name> [-<1|2>]
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <fcntl.h>
#include <sys/systemcfg.h>

/* Required for IPL-control block: */
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>

/* NVRAM addresses */
#include <sys/nvdd.h>

/* Local header files */
#include "cfgdebug.h"

/* external functions */
extern	void	*malloc();
struct	CuAt	*getattr();

/* global variables */
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

#define BUID0			"20"	/* Bus Id for bus0 */
#define BUID1			"21"	/* Bus Id for bus1 */
#define SYS0_NAME		"sys0"
#define SYS0_UTYPE		"sys/node/sys"
#define SYS0_UTYPE1		"sys/node/sys1"
#define SYSPLANAR_NAME		"sysplanar0"
#define SYSPLANAR_UTYPE1	"planar/sys/sysplanar1"
#define SYSPLANAR_UTYPE2	"planar/sys/sysplanar2"
#define SYSPLANAR_UTYPE3	"planar/sys/sysplanar3"
#define SYSUNIT_NAME		"sysunit0"
#define SYSUNIT_UTYPE		"sysunit/sys/sysunit"
#define CPUCARD_NAME		"cpucard0"
#define CPUCARD_UTYPE		"cpucard/sys/cpucard1"
#define PROC_NAME		"proc0"
#define PROC_UTYPE		"processor/sys/proc1"
#define L2CACHE_UTYPE		"memory/sys/L2cache"
#define L2CACHE_NAME		"L2cache0"
#define IOPLANAR0_NAME		"ioplanar0"
#define IOPLANAR1_NAME		"ioplanar1"
#define IOPLANAR_UTYPE		"ioplanar/sys/ioplanar"
#define IOPLANAR1_UTYPE		"ioplanar/sys/ioplanar_1"
#define IOPLANAR2_UTYPE		"ioplanar/sys/ioplanar_2"
#define BUS0_NAME		"bus0"
#define BUS1_NAME		"bus1"
#define BUS_UTYPE		"bus/sys/mca"
#define SGABUS_UTYPE		"bus/sys/sgabus"
#define SGABUS_CONNECTION	"00-0J"
#define SGA_BUID		"40"
#define SOL_SLC_UTYPE		"adapter/sys/slc"
#define SGA_UTYPE		"adapter/sys/sga"
#define SOL_STATUS2		0x0074	/* Status register for SOL card */
#define	DEVPKG_PREFIX	"devices"		/* device package prefix */

/* Note:  These must match defines in cfgmem.c */
#define RS1_MODEL		0x00000001
#define RS1_XIO_MODEL		0x01000000
#define RSC_MODEL		0x02000000
#define RS2_MODEL		0x04000000
#define PowerPC_MODEL		0x08000000 

#define SYS0_LED		0x811
#define IOCC_TYPE		0x00300000
#define IO_TYPE			3
#define DISPLAY_TYPE		0x00004000
#define PowerPC_IOCC		0x00300000

struct  Class 	*cusdev;	/* customized devices class ptr */
struct  Class 	*cusatt;	/* customized attributes class ptr */
struct	Class	*preatt;	/* predefined attributes class ptr */
struct	Class	*predev;	/* predefined devices class ptr */
struct	Class	*cusvpd;	/* customized vpd class ptr */

int	allpkg = 0;		/* packaging flag 		*/

IPL_DIRECTORY	iplcb_dir;	/* IPL control block directory */
IPL_INFO	iplcb_info;	/* IPL control block info section */

/* Input parameters */
char	*logical_name;		/* logical name to configure	*/
int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2	*/


/* Provide prototypes for functions that return no value */
void process_non_buc_all_phase();
void process_non_phase_1();
int process_buc_table();
void process_buc_no_table();
void open_odm_and_init();
void close_odm_and_terminate();
void parse_params();
 
/*
 * NAME: main
 *
 * FUNCTION: Configure system object, define & configure other objects
 *           down to, but not including the mca bus.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This function (program) is invoked by the configuration manager.
 *	Entries are placed in the customized devices object class when they
 *	do not already exist, with VPD as read from NVRAM and the IPL control
 *	block.
 *
 * 	The flow of the program is as follows:
 *		1.  Non buc devices are processed first.  This 
 *		processing is separated by phase.  
 *		2.  Buc devices are processed next.  This processing
 *		is separated by whether or not a buc table exists.
 *		There are some checks for phase within this processing.
 *
 *	The names of bus devices and other io devices (such as graphics 
 *	cards) are printed to stdout for the configuration manager to invoke
 *	the configuration method for these devices.
 */

/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{

   /* model/ioplanar type variable */
   char	ioplanar_ut[UNIQUESIZE];	/* ioplanar uniquetype 	*/
   char	sysplanar_ut[UNIQUESIZE];	/* ioplanar uniquetype 	*/
   uint model;				/* model type		*/
   int	rc;				/* return code for buc  */
   
   void err_exit();		/* exit routine for errors */
   
   /* parse the parameters */
   parse_params(argc, argv);

   /* set the LED values to sys0's values */
   if (ipl_phase != RUNTIME_CFG)
       setleds(SYS0_LED);
   
   /* initialize odm and open the databases */
   open_odm_and_init();


   /* Phases and devices:
    * The following devices are to be configured
    * in the following phases:
    *	ALL PHASES		PHASE 2 AND RUNTIME
    *	sys0			sysunit0
    *	sysplanar0		fpa0
    *	ioplanar		slc
    *	bus			memory
    *				sga
    *				other BUC devices
    *				cpucard
    *				proc0
    */
   
   /* Process non-buc, all phase devices	*/
   /* This includes:				*/
   /*		sys0				*/
   /*		sysplanar0			*/
   /* Determine model based on iplcb information */
   process_non_buc_all_phase(&model, sysplanar_ut);

   /* process non-phase 1 devices that are similar for PowerPC and other models */
   if (ipl_phase != PHASE1) 
   {
	/* This includes:				*/
	/*		sysunit0			*/
	/*		fpa0				*/
	/*		cpucard	(if applicable)		*/
	/*		proc0				*/
	process_non_phase_1(model, sysplanar_ut);
	
	/*** PROCESS MEMORY CARDS ***/
   	/* Configure memory cards as children of sysplanar0 */
   	cfgmem(SYSPLANAR_NAME, &iplcb_dir, &iplcb_info, model);
   }
   	
   /* All that is left to process are the BUC devices 		*/
   /* First, determine I/O planar type based on model type 	*/

   if (model & RS1_MODEL)
   {
	/* RS1 Model, no XIO ==> with ioplanar	*/
   	strcpy(ioplanar_ut, IOPLANAR_UTYPE);
   }	
   else if (model & (RSC_MODEL | PowerPC_MODEL))
   {
   	/* RSC and PowerPC models */
   	strcpy(ioplanar_ut, IOPLANAR2_UTYPE);
   } 
   else 
   {
	/* RS1 with XIO, RS2, or anything else 	*/
	/* check to see if desktop or not	*/
	if ((iplcb_info.IO_planar_level_reg & 0x80000000) == 0)
	   	strcpy(ioplanar_ut, IOPLANAR_UTYPE);
	else
		/* integrated I/O planar 	*/
	   	strcpy(ioplanar_ut, IOPLANAR1_UTYPE);
   }

   
   rc = process_buc_table(ioplanar_ut);

   if (rc != 0)
	process_buc_no_table(ioplanar_ut, model);

   close_odm_and_terminate();

   exit(0);
}


/* process_non_buc_all_phase
 *	model 	output variable for model determination
 *
 *	This routine handles all non-buc devices that should
 *	be processed in any phase.  This includes
 *		sys0
 *		sysplanar0
 */
void
process_non_buc_all_phase(model, sysplanar_ut)
uint	*model;
char	*sysplanar_ut;		/* sysplanar uniquetype 	*/

{
   int	rc;
   struct CuDv cudv;		/* customized device object storage */
   struct PdAt pdat;		/* predefined attribute object storage */
   char	sstring[256];		/* search criteria pointer */
   char	sys_ut[UNIQUESIZE];	/* sys0  uniquetype 	*/


   /* Read in the IPL Control Block directory */
   rc = mdd_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB);
   if (rc) 
   {
   	err_exit(rc);
   }

   /* Read in the IPL info section of the Control Block */
   rc = mdd_get(&iplcb_info,iplcb_dir.ipl_info_offset,
   				sizeof(iplcb_info),MIOIPLCB);
   if (rc) 
   {
   	err_exit(rc);
   }

   /* Determine which model we are working with */
   *model = iplcb_info.model & 0xFF000000;

   /* If RS1 model, set a bit so we can use it later */
   if (*model == 0)
	*model = RS1_MODEL;


   /* Determine the uniquetype of this model */
   if (*model == PowerPC_MODEL || *model == RSC_MODEL)
	strcpy(sys_ut, SYS0_UTYPE1);
   else
	strcpy(sys_ut, SYS0_UTYPE);


   /*** PROCESS SYS0 DEVICE ***/
   /* Get sys0 device's PdDv and CuDv objects */
   rc = get_dev_objs_by_name(SYS0_NAME, sys_ut, "", "0",&cudv);
   if (rc) 
   {
   	err_exit(rc);
   }

   /* if this model does not have memscrubbing, delete any  */
   /* memscrubbing CuAt that exists */
	
   sprintf(sstring, "uniquetype = '%s' AND attribute=memscrub", 
			cudv.PdDvLn_Lvalue);
   rc = (int)odm_get_first(preatt, sstring, &pdat);
   if (rc==-1) {
   	/* ODM failure */
   	DEBUG_1("cfgsys: ODM failure getting PdAt for %s\n",sstring)
   	err_exit(E_ODMGET);
   }
   else if (rc==0)
   {
	sprintf(sstring, "name = '%s' AND attribute=memscrub", cudv.name);
	odm_rm_obj(cusatt, sstring);
   }

   if (ipl_phase == PHASE1) 
   {
   	/* In phase 1 set up sys0's system attributes in database */
   	rc = set_attrs(logical_name,&iplcb_info,&iplcb_dir,*model);
   } 
   else 
   {
   	/* This is phase 2 or run time */
   	/* Set kernel parameters to their customized values */
   	DEBUG_0("Calling setvar\n")
   	rc = setvar(&cudv, ipl_phase);
   	DEBUG_0("setvar returned\n")
   }
   if (rc) 
   {
   	err_exit(rc);
   }

   /* sys0 is now configured, update fields and make sure it is AVAILABLE */
   rc = update_cudv(&cudv, "00-00", "", sys_ut, "", TRUE);
   if (rc) 
   {
   	err_exit(rc);
   }


   /*** PROCESS SYSPLANAR0 DEVICE ***/
   /* Configure sysplanar0 as a child of sys0, MUST be done in phase 1 */
   /* Define sysplanar0 if it doesn't already exist */
   
   /* Determine which type of sysplanar we want to configure based on model type */
   /* Set up SYSPLANAR_UTYPE based on model */
   /* RS1 and RS2 Desktop machines are sysplanar2 type */

   if ((*model == RS1_MODEL || *model == RS2_MODEL || *model == RS1_XIO_MODEL) && 
       ((iplcb_info.IO_planar_level_reg & 0x80000000) != 0))
       		strcpy(sysplanar_ut, SYSPLANAR_UTYPE2);

   else if (*model == RSC_MODEL || *model == PowerPC_MODEL)
   		strcpy(sysplanar_ut, SYSPLANAR_UTYPE3);

   else
       		strcpy(sysplanar_ut, SYSPLANAR_UTYPE1);

   rc = get_dev_objs_by_name(SYSPLANAR_NAME, sysplanar_ut, logical_name,"0",&cudv);
   if (rc) 
   {
   	err_exit(rc);
   }


   if (cudv.status != AVAILABLE) 
   {

   	/* Save sysplanar0's VPD in database */
   	rc = get_sysplanar_vpd(SYSPLANAR_NAME,&iplcb_info);

   	/* sysplanar0 is now configured, make sure it is AVAILABLE */
   	rc = update_cudv(&cudv,"00-00",logical_name,sysplanar_ut,"0", TRUE);
   	if (rc) 
   	{
   		err_exit(rc);
   	}
   }

   return;
}


/* process_non_phase_1
 *	model 	input variable 
 *
 *	This routine handles all non-buc devices that should
 *	be processed phase 2 or at run-time.  This includes
 *		sysunit0
 *		fpa0
 *		cpucard (where applicable)
 *		proc0
 */
void
process_non_phase_1(model, sysplanar_ut)
uint	model;
char 	*sysplanar_ut;
{
   int	rc;
   struct CuDv cudv;		/* customized device object storage */
   char   parent_of_proc[16];	/* parent of proc; either cpu or sysplanar */
   char   proc_connwhere[16];	/* connwhere; either "P" or "0" */
   char	  sstring[256];
   int	  installed_size;	/* local variable for installed size of L2 cache */
   L2_DATA L2_data;	/* L2 data structure */

	/*** PROCESS SYSUNIT0 DEVICE ***/
	/* Configure sysunit0 as a child of sys0 */
  	rc = get_dev_objs_by_name(SYSUNIT_NAME, SYSUNIT_UTYPE,
   			logical_name,"1",&cudv);
   	if (rc==0) 
   	{
   		/* sysunit0 is configured, make sure it is AVAILABLE */
   		rc = update_cudv(&cudv,"00-00",logical_name,SYSUNIT_UTYPE,"1",TRUE);
   		if (rc) 
   		{
   			err_exit(rc);
   		}
   	}


   	strcpy(parent_of_proc, SYSPLANAR_NAME);
	strcpy(proc_connwhere, "P");

	/* Configure the cpucard0 (if exists) and proc0 devices */
	if (strcmp(sysplanar_ut, SYSPLANAR_UTYPE2)==0)
	{
		/* create cpucard CuDv */
	   	rc = get_dev_objs_by_name(CPUCARD_NAME, CPUCARD_UTYPE, SYSPLANAR_NAME,
   			"P", &cudv);
	   	if (rc==0) 
   		{
   			/* cpucard0 is configured, make sure it is AVAILABLE */
	   		rc = update_cudv(&cudv,"00-00",SYSPLANAR_NAME,CPUCARD_UTYPE,"P",TRUE);
			if (rc) 
   			{
   				err_exit(rc);
   			}
   		}

	   	strcpy(parent_of_proc, CPUCARD_NAME);
		strcpy(proc_connwhere, "0");

	}
	else
	{	/* There is no cpu card - delete cpucards and attrs */
		sprintf(sstring, "name = '%s'", CPUCARD_NAME);
		odm_rm_obj(cusdev, sstring);
		odm_rm_obj(cusatt, sstring);
	}


	/* configure the processor device */
	rc = get_dev_objs_by_name(PROC_NAME, PROC_UTYPE, parent_of_proc,
   			proc_connwhere,&cudv);
   	if (rc==0) 
   	{
   		/* proc0 is configured, make sure it is AVAILABLE */
   		rc = update_cudv(&cudv,"00-00",parent_of_proc,PROC_UTYPE,proc_connwhere,TRUE);
   		if (rc) 
   		{
   			err_exit(rc);
   		}

		/* set up type and state attributes */
		set_proc_attrs(PROC_NAME, model);

   	}


	/*** PROCESS L2CACHE DEVICE ***/
	/* Set variable to 0 in case no L2 info is found */
	DEBUG_1("L2cache set to =%d\n",0)
	installed_size = 0;

	/* get L2 data structure 					*/
	/* Check 1.  If the address of the L2 data structure conflicts 	*/
	/*   with other info, there is no L2 structure.  Set L2 to 0. 	*/
	/* Check 2.  If the offset or the size is 0, set L2 to 0.	*/

	if ((&(iplcb_dir.l2_data_offset) < ((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128))
		&& (iplcb_dir.l2_data_offset != 0) && (iplcb_dir.l2_data_size != 0))
	{
		/* get the L2 cache information */
		mdd_get(&L2_data, iplcb_dir.l2_data_offset, sizeof(L2_data), MIOIPLCB);

		DEBUG_1("L2cache=%d\n",L2_data.installed_size);
		installed_size = L2_data.installed_size;
	}


	/* get any old L2 device object that is in database */
	rc = odm_get_first(cusdev, "name=L2cache0", &cudv);

	if (rc == -1)
		err_exit(E_ODMGET);
	
	/* if there was no previous L2 cache in database, create it */
	else if (rc == 0)
	{
		DEBUG_0("No L2 cache found in database - create new ones.\n");
		if (installed_size !=0)
			/* create cudv and cuat as needed */
			setup_L2cache(&parent_of_proc, installed_size);
	}

	else	/* there was L2 cache previously */
	{
		DEBUG_0("L2 cache WAS found in database.\n");
		DEBUG_1("size = %d\n", installed_size);
		DEBUG_1("new parent = %s\n", parent_of_proc);
		DEBUG_1("old parent = %s\n", cudv.parent);

		if (installed_size != 0)
		{
			DEBUG_0("size != 0, so calling setup_L2cache\n");
			setup_L2cache(&parent_of_proc, installed_size);
		}
		else if (strcmp(cudv.parent, parent_of_proc) != 0)	
		{
			DEBUG_0("L2 cache WAS found in database and new size == 0 so, deleteing old stuff.\n");
			/* delete L2 cache CuDv and CuAt */
			odm_rm_obj(cusatt, "name=L2cache0");
			odm_rm_obj(cusdev, "name=L2cache0");
		}	
		
	}

	return;
}


/* process_buc_table
 *	ioplanarut	uniquetype to use for ioplanar
 *
 *	This routine handles the processing of a buc table for
 *	any model that has a buc table.  The uniquetype of the
 *	ioplanar is determined before this routine is called
 *	and is passed in.
 */
int
process_buc_table(ioplanar_ut)
char	*ioplanar_ut;
{
	BUC_DATA	buc;		/* used in processing buc table */
	struct CuDv cudv;		/* customized device object storage */
	struct PdDv pddv;		/* pre-defined device object storage */
	int  buc_offset;		/* used in processing buc table */
	int  num_of_bucs;		/* used in processing buc table */    
	char location[6];		/* used in processing buc table */
	char slot_num[4];		/* used in processing buc table */ 
	char bus_num_str[2];		/* used in processing buc table */
	int  i, j;  	         	/* used in buc processing loop control */
	int  rc;			/* return code */
	char ioplanar_name[NAMESIZE] = "ioplanar";	/* ioplanar device name */
	char	bus_name[NAMESIZE] = "bus";	/* bus name */
	char	sstring[256];		/* search criteria pointer */
	char	sstr[256];		/* string for formatting package name */

	/*
	 * Determine if there is a buc table.  This involves three checks.
	 *
	 * 1.  Address check.
	 * The ipl_directory varies in size, so the following calculation
	 * is intended to determine whether the iplcb_dir.buc_info_offset
	 * field exists or not.  Do this by comparing the address of the
	 * buc_info_offset against the address of the ipl_info struct, which
	 * follows the ipl_directory in memory.  The address of the ipl_info
	 * struct is calculated by adding the address of the ipl_directory
	 * (cast to char * to prevent incrementing by size of the struct)
	 * to the ipl_info_offset (subtract 128 for 32 4 byte GP registers).
	 * If the address of buc_info_offset is less than the address of the
	 * ipl_info struct, assume existence and validity of the buc_info_offset.
	 *
	 * 2.  Do the buc_info_offset and buc_info_size both equal 0?  If so,
	 * the buc table is not valid.
	 *
	 * 3.  Check to see if the number of bucs is 0.  If so, the buc
 	 * table is not valid. 
 	 *
	 */

	/* Check 1.  If the address of the buc table conflicts with other info, return -1 */
	if (&iplcb_dir.buc_info_offset >=
   		((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128))
		return (-1);

	/* It looks like there is a table.  		 	*/
	/* Check 2.  If the offset or the size is 0, return -1	*/
	if ((iplcb_dir.buc_info_offset == 0) || (iplcb_dir.buc_info_size == 0))
		return(-1);

	/* read in the first BUC  */
	buc_offset = iplcb_dir.buc_info_offset;
	mdd_get(&buc, buc_offset, sizeof(buc), MIOIPLCB);

	num_of_bucs = buc.num_of_structs;

	/* Check 3.  If the number of bucs inside the table says "0", return -1 */
	if (num_of_bucs == 0)
		return(-1);

	/* NOTE!  If we reach this point, then we know there is a buc */
	/* table, we have read the first buc and set up num_of_bucs.  */

	/* loop through all BUCS */
	for (i=0; i<num_of_bucs; i++)
	{
		/* get BUC.  The first buc will be retrieved twice.  */
		/* bucs are retrieved at the top of loop to avoid    */
		/* searching beyond the buc table.		     */
		mdd_get(&buc, buc_offset, sizeof(buc), MIOIPLCB);

		

		/* Check for device_id_reg of 0xFFFFFFFF or 0			*/
		/* These are invalid device ids and should not be processed. 	*/
		/* Also check for device type of 0xF "device not present" and 	*/
		/* 0x0 "device present but not ready".  Don't process these, either. */
		if ((buc.device_id_reg == 0xFFFFFFFF) || (buc.device_id_reg == 0)
			|| ((buc.device_type & 0xF) == 0) || ((buc.device_type & 0xF) == 0xF))
		{
			/* update offset and continue loop */
			buc_offset = buc_offset + buc.struct_size;
			continue;
		}

		/* set up variables from values in the buc table 	*/
		/* get the slot number into a string			*/
		/* slot = (buc.bscr_value &0x1FF);			*/
		sprintf(slot_num, "%d", (buc.bscr_value & 0x1FF));

		/* Check to see if the buc entry has a location field.  */
		if (((uint)&buc.location - (uint)&buc) < buc.struct_size)
		{
			/* set up location variable based on location data in buc table */
			sprintf(location, "%c%c-%c%c", 
				buc.location[0],
				buc.location[1],
				buc.location[2],
				buc.location[3]);
		}
		else
		{
			strcpy(location,"00-00");
		}

		/* Early Rainbow IOCCs will not be have a PdDv in the database to
		   match their device_id_reg.  Therefore, we get the generic
		   iocc PdDv from the database and fix it up to match the
		   hardware
		*/


		if ( buc.device_id_reg == PowerPC_IOCC )
		{
			/* create ioplanar name by appending 3rd digit (4th char) of 	*/
			/* location to the char string "ioplanar"			*/
			ioplanar_name[8] = location[3];
			ioplanar_name[9] = '\0';
			
			rc = get_dev_objs_by_name(
					ioplanar_name,
					ioplanar_ut,  
					SYSPLANAR_NAME,
					slot_num,
					&cudv);

			/* rc is one of the following:
				E_NOPdDv
				E_NOCuDv
				E_ODMGET
				E_ODMRUNMETHOD	
			*/

			if (rc == E_NOPdDv || allpkg)
			    fprintf(stdout, ":%s.sys.mca ", DEVPKG_PREFIX);

			else if (rc) 
			{
				/* rc could be the following:
					E_NOCuDv
					E_ODMGET
					E_ODMRUNMETHOD	
				*/
				err_exit(rc);
			}
		
			if (rc == 0)	/* the PdDv was found */
			{
				/* get_ioplanar_vpd(ioplanar_name, iplcb_info.IO_planar_level_reg); */
	
				/* ioplanar0 is now configured, make sure it is AVAILABLE */
				rc = update_cudv(&cudv,location,SYSPLANAR_NAME,ioplanar_ut,slot_num,TRUE);
				if (rc) 
				{
					/* returns E_ODMUPDATE or 0 */
					err_exit(rc);
				}
	
				/* Define bus, if necessary */
				/* create the bus name based on location */ 
				bus_name[3] = location[3]; 	
				bus_name[4] = '\0';
	
				/* Create a string containing ONLY the bus number */
				sprintf(bus_num_str, "%c", location[3]);

				rc = get_dev_objs_by_name(bus_name, BUS_UTYPE, ioplanar_name,
						bus_num_str, &cudv);
				if (rc) 
				{
					/* rc could be the following:
						E_NOPdDv
						E_NOCuDv
						E_ODMGET
						E_ODMRUNMETHOD	
					*/
					err_exit(rc);
				}
	
				/* update the cudv - do not make it available */
				rc = update_cudv(&cudv, location, ioplanar_name, BUS_UTYPE, bus_num_str, FALSE);
	
				/* Write name of bus device to stdout */
				fprintf(stdout,"%s ", cudv.name);
			}

			/* Update the offset to get next buc */
			buc_offset = buc_offset + buc.struct_size;
			
 		}
		else	/* not an early rainbow */
		{
			/* set up packaging string in case needed 	*/
			/* to be :devices.buc.device_id_reg 		*/
			sprintf(sstr, ":%s.buc.%08x ", DEVPKG_PREFIX, buc.device_id_reg);

			/* set up search string for predefined device object for this device */
			sprintf(sstring, "subclass='buc' AND devid='%x'",
				buc.device_id_reg);
	 
			/* Each IO buc gets two chances to find a pddv */
			for (j=0; j<=1; j++)
			{
				rc = (int)odm_get_first(predev, sstring, &pddv);		
			 	if (rc==-1) 
				{
					/* ODM failure  - end the cfgsys attempt */
					DEBUG_1("cfgsys: ODM failure getting PdDv for %s\n", sstring)
					err_exit(E_ODMGET);
				}
			
				/* If we didn't find a PdDv, see if we need to create one */
				if (rc == 0) 
				{
					/* if this is the 2nd time on this buc, break */
					if (j == 1)
						break;
	
					/* if this is not an IO buc, go to next buc */
					if ((buc.device_type & 0xF) != IO_TYPE)
						break;

					if ( (buc.device_id_reg & 0x00FFFF00) == DISPLAY_TYPE ) 
					{
					    if (ipl_phase != PHASE1) 
					    {

						/* Print out the package name for this device   */
						
						fprintf(stdout, sstr);

						/* Try to load feature ROM 		*/
						rc = FRS_Add_PPC_Video_Device(SYSPLANAR_NAME, (buc.bscr_value & 0x1FF));
			
						/* If not successful, break out of twice-per-buc loop (j) 	*/
						/* and go on to next buc 					*/
						if (rc != 0)
						{
							break;
						}
					   }
					}				
					
					else 	/* other IO bucs */
					{
						/* write out package name with dev_id_register */
						fprintf(stdout, sstr);
		
						/* break out of the twice-per-buc lopp (j)		*/
						break;
					}
				}  /* end of processing when pddv not found */
		
				else  /* the pddv was found */
				{
					if (allpkg)
						fprintf(stdout, sstr);

					/* PdDv was found! 			*/
					rc = get_dev_objs_by_type( 
						   NULL,		/* we have pddv 	*/
						   &pddv,		/* the pddv 		*/
						   SYSPLANAR_NAME,	/* parent name 		*/
						   slot_num,		/* adapter slot number  */
						   TRUE,		/* define the device	*/
						   &cudv);		/* ptr to CuDv struc 	*/
								
					if (rc) 
					{	
						/* rc could be the following:
							E_NOPdDv
							E_NOCuDv
							E_ODMGET
							E_ODMRUNMETHOD	
						*/
						err_exit(rc);
					}
		
					/* found device, now make sure location is correct */
					rc = update_cudv(&cudv, location, SYSPLANAR_NAME,
						pddv.uniquetype, slot_num, FALSE);
					if (rc) 
					{
						/* returns E_ODMUPDATE or 0 */
						err_exit(rc);
					}
			
					/* write name of CuDv to stdout */
					fprintf(stdout,"%s ", cudv.name);
	
					/* break out of the "twice per buc" for loop */
					break;		
				}
	
			} /* end of "twice per buc" for loop */
	
			/* update offset and continue loop */
			buc_offset = buc_offset + buc.struct_size;
	
		}  /* end of else "not an early rainbow" */
	
	} /* end of table processing */
	
	return(0);
}

/* process_buc_no_table
 *	ioplanarut	uniquetype to use for ioplanar
 *	model		the model type
 *
 *	This routine handles the processing of buc devices when
 *	there is no buc table.  This will typically mean an 
 *	older model such as RS1, RSC.  These devices include
 *		ioplanar0 (and ioplanar1)
 *		bus0 (and bus1)
 *		slc devices
 *		sgabus
 *	The uniquetype of the
 *	ioplanar is determined before this routine is called
 *	and is passed in.
 */
void
process_buc_no_table(ioplanar_ut, model)
char	*ioplanar_ut;
int	model;
{	
	IOCC_POST_RESULTS	iplcb_post;	/* IOCC Post results section */
	int	rc;
	struct CuDv cudv;		/* customized device object storage */
	int	buid;			/* Serial Optical Link bus Id 	    */
	char	buidstr[4];		/* SOL bus Id in string form 	    */

	/*** PROCESS IOPLANAR0 DEVICE ***/
	/* Configure ioplanar0 as a child of sysplanar0 */
	/* Define ioplanar0 if it doesn't already exist */

	 rc = get_dev_objs_by_name(IOPLANAR0_NAME,ioplanar_ut,
		SYSPLANAR_NAME,BUID0,&cudv);

	DEBUG_1("cfgsys: getbyname for ioplanar0 returned %d\n",rc)
    
	if ( rc == E_NOPdDv || allpkg )
	{
	    fprintf(stdout, ":%s.sys.mca ", DEVPKG_PREFIX);
	}
	else if (rc) 
	{
		err_exit(rc);
	}

	if ( rc == 0 ) 	/* the pddv was found! */
	{

		if (cudv.status != AVAILABLE) 
		{
	
			get_ioplanar_vpd(IOPLANAR0_NAME, iplcb_info.IO_planar_level_reg);
			/* ioplanar0 is now configured, make sure it is AVAILABLE */

			rc = update_cudv(&cudv,"00-00",SYSPLANAR_NAME,ioplanar_ut,BUID0, TRUE);
			if (rc) 
			{
				err_exit(rc);
			}
		}
	
		/* Define bus0, if necessary */
		rc = get_dev_objs_by_name(BUS0_NAME,BUS_UTYPE,IOPLANAR0_NAME,
				"0",&cudv);

		if (rc)
		{
			err_exit(rc);
		}

		/* update CuDv but don't make the device available */
		rc = update_cudv(&cudv, "00-00", IOPLANAR0_NAME, BUS_UTYPE, "0", FALSE);

		/* Write name of bus0 device to stdout */
		fprintf(stdout,"%s ", cudv.name);
	
	
		/*** PROCESS IOPLANAR1 DEVICE ***/
		/* Determine if optional I/O planar exists, */
		/* If so, then configure ioplanar1 as a child of sysplanar0 */
		/* Define ioplanar1 if necessary */
	
		/* Read in the IOCC Post Results section of IPL Control Block */
		rc = mdd_get(&iplcb_post,iplcb_dir.iocc_post_results_offset,
							sizeof(iplcb_post),MIOIPLCB);
		if (rc) 
		{
			err_exit(rc);
		}
	
		if (iplcb_post.length == 2) 
		{
			/* The optional I/O planar is present */
			DEBUG_0("Optional I/O bus is present\n")
			rc = get_dev_objs_by_name(IOPLANAR1_NAME,ioplanar_ut,
					SYSPLANAR_NAME,BUID1,&cudv);
			if (rc) 
			{
				err_exit(rc);
			}
	
			if (cudv.status != AVAILABLE) 
			{
	
				get_ioplanar_vpd(IOPLANAR1_NAME, iplcb_info.IO_planar_level_reg_21);
		
				/* ioplanar1 is now configured, set it to AVAILABLE */
				rc = update_cudv(&cudv,"00-10",SYSPLANAR_NAME,ioplanar_ut,BUID1,TRUE);
				if (rc) 
				{
					err_exit(rc);
				}
			}
	
			/* Define bus1, if necessary */
			rc = get_dev_objs_by_name(BUS1_NAME,BUS_UTYPE,
					IOPLANAR1_NAME,"1",&cudv);
			if (rc) 
			{
				err_exit(rc);
			}
			
			/* update CuDv but don't make the device available */
			rc = update_cudv(&cudv, "00-10", IOPLANAR1_NAME, BUS_UTYPE, "1", FALSE);
	
			/* Write name of bus1 device to stdout */
			fprintf(stdout,"%s ", cudv.name);
		} 
		else 
		{
			DEBUG_0("No optional I/O bus\n")
		}
	
	}	
		

	if (ipl_phase != PHASE1) 
	{
		/*** PROCESS SLC DEVICES ***/
		/* Configure slc0,etc., as children of sysplanar0 */
		/* Note: Some desktop models have SLC devices which are */
		/* detectable but since they can not be used, SLC device */
		/* detection is skipped for desktop models. */

		if ((iplcb_info.IO_planar_level_reg & 0x80000000)==0) 
		{
		    for (buid = MSLA0; buid <= MSLA7; buid+=2) 
		    {
			sprintf(buidstr,"%02x",buid);

			/* we do NOT want to attempt to define this device yet 	*/
			/* if it is not already defined.			*/
			rc = get_dev_objs_by_type( 
					   SOL_SLC_UTYPE,	/* unique type 		*/
					   NULL,		/* we know utype 	*/
					   SYSPLANAR_NAME,	/* parent name 		*/
					   buidstr,		/* connection on parent */
					   FALSE,		/* don't define device  */
					   &cudv);		/* ptr to CuDv struc 	*/

			DEBUG_1("cfgsys: getbyname for slc returned %d\n",rc)

			if ( rc == E_NOPdDv || allpkg )
				fprintf(stdout, ":%s.sys.slc ", DEVPKG_PREFIX);

			if ( rc == E_NOPdDv )
				/* if there is no pddv, print out package message
					and get out of the loop - the pddv
					won't be there next time, either!
				*/
				break;
			
			/* at this point we have found a PdDv & created/found a CuDv */
			if (rc == 0 && cudv.status == AVAILABLE)
				/* Write name of SLC device to stdout */
				fprintf(stdout,"%s ", cudv.name);

			else if (rc == E_NOCuDv || 
				((cudv.status != AVAILABLE) && rc == 0) )

			/* if we did not find a cudv OR the device is not available,	*/
			/* we can run detect_device.					*/
			{
				rc = detect_device(buid, SOL_STATUS2);
				
				if (rc == 0) 
				{
					/* device is present so configure */
					rc = get_dev_objs_by_type( 
					   SOL_SLC_UTYPE,	/* unique type */
					   NULL,		/* we know utype */
					   SYSPLANAR_NAME,	/* parent name */
					   buidstr,		/* connection on parent */
					   TRUE,		/* define the device	 */
					   &cudv);		/* ptr to CuDv struc 	*/

					if (rc == 0) 
					{
						/* NOTE: if device is not AVAILABLE, */
						/* then make sure location code is */
						/* set to 00-00 and chgstatus is set */
						/* to SAME.  The device's config */
						/* method is responsible for making */
						/* it AVAILABLE */
						rc = update_cudv(&cudv,"00-00",SYSPLANAR_NAME,
							SOL_SLC_UTYPE,buidstr,FALSE);
	
					}
					/* Write name of SLC device to stdout */
					fprintf(stdout,"%s ", cudv.name);
				}
			}
		    }  /* end of for loop */
		}  /* end of if slc */
	

		if ( model & RSC_MODEL )	
		{
			    DEBUG_0( "Configuring sgabus.\n" )
				
				rc = get_dev_objs_by_type( 
					   SGABUS_UTYPE,	/* unique type */
					   NULL,		/* we know utype */
					   SYSPLANAR_NAME,	/* parent name */
					   SGA_BUID,		/* connection on parent */
					   TRUE,		/* define the device	*/
					   &cudv);		/* ptr to CuDv struc 	*/
	
				if ( rc == E_NOPdDv || allpkg )
					fprintf(stdout, ":%s.sys.sgabus ", DEVPKG_PREFIX);
			    
			    	/* 10/2 taking the else off */
				if (rc == 0)
				{	
					rc = update_cudv(&cudv,SGABUS_CONNECTION,SYSPLANAR_NAME,
							SGABUS_UTYPE,SGA_BUID,FALSE);
	
					fprintf( stdout, "%s ", cudv.name );
				}
		}
	} /* end of if !phase1 */

	return;

} /* end of non-buc processing */



/*
 * NAME: setup_L2cache
 *
 * FUNCTION: Creates/verifies CuDv and CuAt for L2 cache
 *
 * RETURNS:  error code.  0 means no error.
 */
int
setup_L2cache(device_name, installed_size)
char	*device_name;
int	installed_size;
{
	int	rc;		
	struct CuDv	cudv;
	char	  L2cache[16];	/* for setting the L2 cache attribute */

	sprintf( L2cache, "%d", installed_size);

	DEBUG_0("entering setup_L2cache.\n");
	/* configure the L2 cache device */
	rc = get_dev_objs_by_name(L2CACHE_NAME, L2CACHE_UTYPE, device_name, "L", &cudv);
	DEBUG_1("Returned from get by name with rc = %d\n", rc);

	if (rc) 
	{	
		/* rc could be the following:
			E_NOPdDv
			E_NOCuDv
			E_ODMGET
			E_ODMRUNMETHOD	
		*/
		err_exit(rc);
	}
			
	/* found device, now make sure location is correct */
	rc = update_cudv(&cudv, "00-00", device_name,
		L2CACHE_UTYPE, "L", TRUE);
		
	DEBUG_1("Returned from update_cudv with rc = %d\n", rc);
	if (rc) 
	{
		/* returns E_ODMUPDATE or 0 */
		err_exit(rc);
	}
	setattr(L2CACHE_NAME, "size", L2cache);
}


/* get_dev_objs_by_name
 *	desired_name for device
 *	utype
 *	pname
 *	connection
 *	cudv
 *
 *	This will get a CuDv based on the name field.  If a CuDv
 *	with the appropriate name exists, this routine will change
 *	its other attributes to that of the parameters passed in.
 *	i.e., if ioplanar0 exists in the CuDv database, we will
 *	update it to have the passed in parent_name, uniquetype
 *	and connwhere.   
 *
 *	If a CuDv for this name does not exist (e.g., ioplanar1),
 *	we will create one.
 *
 *	Currently, this routine is used for the following devices:
 *		fpa0
 *		ioplanar0 - 3
 *		sys0
 *		sysunit0
 *		sysplanar0
 *		bus0 - 3
 */

int
get_dev_objs_by_name(desired_name, utype, pname, connection, cudv)
char	*desired_name;		/* Device name					*/
char	*utype;			/* Device predefined uniquetype			*/
char	*pname;			/* Parent device logical name 			*/
char	*connection;		/* Connection location on parent device 	*/
struct CuDv	*cudv;		/* Pointer to CuDv object 			*/

{
	int	i;		/* loop control variable */
	int	rc;		/* ODM return code */
	char	sstring[256];	/* the ODM search string */
	struct PdDv pddv;	/* PdDv structure for processing */
	char	*out1;		/* string for odm_run_method */
	char 	*out2;		

	/* get PdDv based on uniquetype */

	/* This needs to be done first because if there is
		no PdDv, we need to return that info back
		in case we are spitting out package names.
	*/
	
	sprintf(sstring, "uniquetype = '%s'", utype);
	rc = (int)odm_get_first(predev, sstring, &pddv);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_1("cfgsys: failed to find PdDv object for %s\n",utype)
		return(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_1("cfgsys: ODM failure getting PdDv for %s\n",utype)
		return(E_ODMGET);
	}
	

	for (i=0; i<=1; i++)
	{ 
		/* get CuDv based on desired_name 	*/
		sprintf(sstring, "name = '%s'", desired_name);
		rc = (int)odm_get_first(cusdev,sstring,cudv);
	
		if (rc == -1) {
			/* ODM failure */
			DEBUG_0("cfgsys: ODM failure getting CuDv object\n")
			return(E_ODMGET);
		}
		
		if (rc == 0)
		{
			if (i==1)
				/* We've already tried this once - give up */
				return(E_NOCuDv);
			
			/* No CuDv object found so define one by running define method  */
	
			sprintf( sstring, "-c %s -s %s -t %s -p %s -w %s -l %s",
					pddv.class, pddv.subclass, pddv.type,
					pname, connection, desired_name);
	
			DEBUG_2( "Invoking %s method with params:\n%s\n",
								pddv.Define, sstring )
	
			rc = odm_run_method(pddv.Define,sstring,&out1,&out2);
			if (rc) {
				return(E_ODMRUNMETHOD);
			}

			/* now loop back through - we should
			 *  find this cudv just created next time 
			 */
		}
		else
			/* we found the CuDv! */
			break;
	}
	return(0);	
}

/* get_dev_objs_by_type
 *	utype		Input	may be null; utype char if we know it
 *	pddv		Input	may be null; ptr if PdDv has been found
 *	pname		Input	parent name
 *	connection	Input   connection location
 *	define_flag	Input   whether or not to define this device
 *	cudv		Output	The CuDv
 *	
 ****** NOTE:  Either utype OR pddv can be null, but not both.  One
 ****** of them MUST be passed in.
 */

int
get_dev_objs_by_type(utype, pddv, pname, connection, define_flag, cudv)
char	*utype;			/* Device predefined uniquetype; may be null	*/
struct PdDv *pddv;		/* PdDv for this device or null 		*/
char	*pname;			/* Parent device logical name 			*/
char	*connection;		/* Connection location on parent device 	*/
int	define_flag;		/* Should we define or just check 		*/
struct CuDv	*cudv;		/* Pointer to CuDv object 			*/

{
	int	rc;			/* ODM return code */
	int	rom_rc;			/* return code from ROM load */
	char	sstring[256];		/* the ODM search string */
	char	cudv_sstring[256];	/* another ODM search string */
	int 	i;			/* loop control variable */
	struct PdDv local_pddv;		/* local pddv in case pddv is null */
	char 	*out1;			/* output string for run method */
	char 	*out2;			

	/* Either the uniquetype or the pddv MUST be passed in. */
	/* If we have the utype (i.e., it is NOT null) then we  */
	/* don't have the pddv - go get it.			*/

	if (pddv == NULL)
	{
		pddv = &local_pddv;

		/* get predefined device object for this device */
				
		sprintf(sstring, "uniquetype = '%s'", utype);
		rc = (int)odm_get_first(predev, sstring, pddv);
	
		if (rc==0) {
			/* No PdDv object for this device */
			return (E_NOPdDv);
	
		}
		else if (rc==-1) {
			/* ODM failure */
			DEBUG_1("cfgsys: ODM failure getting PdDv for %s\n",utype)
			return(E_ODMGET);
		}
	}


	/* Set up the parameters for the CuDv search 		*/
	/* if utype is null, then get uniquetype from the pddv 	*/
	if (utype == NULL) {
		sprintf(cudv_sstring, "PdDvLn=%s AND parent='%s' AND connwhere='%s'",
				pddv->uniquetype, pname, connection);

	}
	else {
		sprintf(cudv_sstring, "PdDvLn=%s AND parent='%s' AND connwhere='%s'",
						utype,pname,connection);
	}


	for ( i=0; i<=1; i++ )
	{
		/* Get CuDv based on string containing uniquetype, pname, connection */
		rc = (int)odm_get_first(cusdev, cudv_sstring, cudv);
		if (rc==-1) {
			/* ODM failure */
			DEBUG_0("cfgsys: ODM failure getting CuDv object\n")
			return(E_ODMGET);
		}
		/* No CuDv object found so define one by running method */
		if (rc==0) {

			/* If not defining the device, then just return with error */
			if (!define_flag)
				return(E_NOCuDv);
		
			if (i==1)
				/* Already tried once, there is no cudv - give up! */
				return(E_NOCuDv);

			if (*pname == NULL) {
				sprintf( sstring, "-c %s -s %s -t %s",pddv->class,
						pddv->subclass, pddv->type);
			} else {
	
			sprintf( sstring, "-c %s -s %s -t %s -p %s -w %s",
				pddv->class, pddv->subclass, pddv->type,
						pname, connection);
			}	

			DEBUG_2( "Invoking %s method with params:\n%s\n",
							pddv->Define, sstring )
	
			rc = odm_run_method(pddv->Define,sstring,&out1,&out2);
			if (rc) {
				DEBUG_1("odm_run_method returned %d\n", rc);
				return(E_ODMRUNMETHOD);
			}
		}
	
		/* Got a cudv on the first attempt! Exit the loop and return */
		else break;
	}

	return (0);
}

int
get_ioplanar_vpd(name, io_planar_reg)
char 	*name;		/* ioplanar name 	*/
int	io_planar_reg;	/* register to read 	*/

{

	char    vpd[VPDSIZE];	        /* vpd data */
	char	tmpstr1[100];		/* temporary string variable */
	int	rc;			/* return code variable */

	/* Save ioplanar0's VPD in database */
	memset(vpd, 0, VPDSIZE);
	*vpd = '\0';
	sprintf(tmpstr1, "%02X", (io_planar_reg)>>24);
	add_descriptor(vpd, "EC", tmpstr1);
	rc = update_vpd(name,vpd);
	if (rc) 
	{
		return(rc);
	}

	return(0);
}
	

/*
 * NAME: set_proc_attrs
 *
 * FUNCTION: Sets attributes for processor in the database
 *
 * RETURNS:  error code.  0 means no error.
 */
int
set_proc_attrs(lname, model)
char		*lname;		/* pointer to name of sys object */
uint		model;		/* model type being configured 	*/
{
	uint	modcode;	/* the model code from IPL control block */
	int	aix_modcode;	/* the model code to be saved as attribute */
	char	state[10];	/* for setting state attribute */
	char	type[10] = "";	/* for setting type attribute */
	PROCESSOR_DATA proc_data;

	/* set the processor attributes */
	DEBUG_0("Setting processor attributes\n")

	/* First, look at the new field in the iplcb, the proc_descriptor field */
	/* look at iplcb implementation data field */
    	/* Check 1.  If the address of the processor data structure conflicts
    	   with other info, there is no processor structure.  */
    	/* Check 2.  If the offset or the size is 0, use model */
    	if ((&(iplcb_dir.processor_info_offset) < ((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128))
   		&& (iplcb_dir.processor_info_offset != 0) && (iplcb_dir.processor_info_size != 0))
    	{
    		/* get the processor information */
    		mdd_get(&proc_data, iplcb_dir.processor_info_offset, sizeof(proc_data), MIOIPLCB);

		/* BUT!  Is the new field in this structure actually there? */
		if (((uint)&proc_data.proc_descriptor - (uint)&proc_data) < proc_data.struct_size)
		{
			/* Yes, it's there.  Go ahead and look at it. */
			if (proc_data.proc_descriptor[0] != '\0')
				strcpy(type, proc_data.proc_descriptor);

			else /* all that work, and it's null! */
			{
				/* use the implementation field */
				if (proc_data.implementation == POWER_RS1 
					|| proc_data.implementation == POWER_RSC)
					strcpy(type, "POWER");
				else if (proc_data.implementation == POWER_RS2)
					strcpy(type, "POWER2");
				else if (proc_data.implementation == POWER_601)
					strcpy(type, "PowerPC_601");
				else if (proc_data.implementation == POWER_603)
					strcpy(type, "PowerPC_603");
				else if (proc_data.implementation == POWER_604)
					strcpy(type, "PowerPC_604");
				else if (proc_data.implementation == POWER_620)
					strcpy(type, "PowerPC_620");
			}
    		}
	}

	/* If we didn't find a type above, look at model */
	if (type[0] == '\0')
	{
		if (model == RS2_MODEL)
			sprintf(type, "%s", "POWER2");
		else if (model == PowerPC_MODEL)
			sprintf(type, "%s", "PowerPC_601");
		else 
			sprintf(type, "%s", "POWER");
	}

	setattr( lname, "type", type );

	sprintf( state, "%s", "enable");
	setattr( lname, "state", state );
}


    
/*
 * NAME: set_attrs
 *
 * FUNCTION: Sets sys0's attributes in the database
 *
 * RETURNS:  error code.  0 means no error.
 */
int
set_attrs(lname,iplcb_ptr,iplcb_dir,model)
char		*lname;		/* pointer to name of sys object */
IPL_INFO	*iplcb_ptr;	/* IPL control block info section */
IPL_DIRECTORY	*iplcb_dir;	/* IPL Directory 		*/
uint		model;		/* model type being configured 	*/
{
	char	modelcode[10];	/* for setting model code attribute */
	char	keylock[10];	/* for setting keylock attribute */
	char	rostime[12];	/* for setting rostime attribute */
	char	totalmem[10];	/* for setting realmem attribute */
	uint	modcode;	/* the model code from IPL control block */
	int	aix_modcode;	/* the model code to be saved as attribute */

	uint	memsize;	/* memory size */
	int	simm;		/* SIMM number */
	int	key;		/* key setting */
	int	rc;		/* return code from subroutine calls */
        int     get_key(int *);/* forward declaration */
	RAM_DATA  iplcb_simms;	/* ipl simms info for RSC model */

	/* set the system attributes */
	DEBUG_0("Setting system attributes\n")

	/* get model code from IPL control block */
	modcode = iplcb_ptr->model;
	DEBUG_1("modelcode=0x%08x\n",modcode)

	if (modcode & 0xff000000) {
		/* 3.2 model code definition */
		/* AIX model code is upper two bytes of model code */
		aix_modcode = modcode >> 16;

	} else {
		/* 3.1 model code definition */
		/* Convert obsolete model codes of 0x20 and 0x2E */
		if (modcode == 0x20)
			aix_modcode = 0x02;
		else if (modcode == 0x2E)
			aix_modcode = 0x0E;
		else
			aix_modcode = modcode;

	}

	/* Save model code attribute */
	DEBUG_1("modelcode=0x%04x\n",aix_modcode)
	sprintf( modelcode, "0x%04x", aix_modcode );
	setattr( lname, "modelcode", modelcode );


	DEBUG_0("Getting keylock state\n")
        if (rc = get_key(&key)) return(rc);
	switch(key)
	{
	case 0:	/* Undefined */
	case 1:
		strcpy( keylock, "secure" );
		break;
	case 2:
		strcpy( keylock, "service" );
		break;
	case 3:
		strcpy( keylock, "normal" );
		break;
	default:
		DEBUG_0("IMPOSSIBLE VALUE IN SWITCH STATEMENT???\n")
		;
	}
	DEBUG_1("keylock=%s\n",keylock)
	setattr( lname, "keylock", keylock );

	strncpy( rostime, iplcb_ptr->ipl_ros_timestamp, 10 );
	rostime[10] = '\0';
	DEBUG_1("rostime=%s\n",rostime)
	setattr( lname, "rostime", rostime );

	memsize = 0;

	/* Determine the total memory size and save as sys0 attribute */
	/* First, try to process a memdata table	*/
	rc = total_mem_from_table();
	if (rc != -1)
		memsize = rc;
	else if (model & RSC_MODEL)
	{
		/* Get the simm information */
		rc = mdd_get(&iplcb_simms, iplcb_dir->ram_post_results_offset, 
				sizeof(iplcb_simms), MIOIPLCB);
		
		/* On RSC model need to add up SIMM sizes */
		for (simm=0; simm<8; simm++) {
			memsize = memsize + iplcb_simms.simm_size[simm];
		}
		/* memsize is in Mbytes, so convert to Kbytes */
		DEBUG_1("memsize in MB = %d\n", memsize) ;
		memsize = 1024 * memsize;
		DEBUG_1("memsize in KB = %d\n", memsize) ;
	} 
	else if (model & (RS1_MODEL | RS1_XIO_MODEL | RS2_MODEL))
	{
		/* Just use value from ram_size field */
		/* Value is in bytes so convert to Kbytes */
		memsize = iplcb_ptr->ram_size / 1024;
	}

	/* if none of the models above matched, memsize will = 0!! */

	sprintf(totalmem,"%d", memsize);
	DEBUG_1("realmem=%s\n",totalmem)
	setattr( lname, "realmem", totalmem );


	/* Set previous IPL Device for IPL routines */
	/* Only copy if in normal mode, and not diskette */
	if ((key==3) && (strncmp(iplcb_ptr->previpl_device,"\002N",2)!=0)) {
		/* Copy the previous ipl device information from the */
		/* ipl-control block to nvram */
		rc = put_nvram( iplcb_ptr->previpl_device, 
                                IPLIST_PREVBOOT,
			        sizeof( iplcb_ptr->previpl_device ) );
		if (rc)
			return(rc);
	}

	return(E_OK);
}

/*
 * NAME: total_mem_from_table
 *
 * FUNCTION: To determine if there is a memtable and total
 *		the memory from it if it exists.
 *
 * RETURNS:  -1 if there is no table
 *	      memory size if there is a table
 */
int
total_mem_from_table()
{
	MEM_DATA memdata;	/* new format memory data structure */
	uint	mem_offset;	/* offset into the memdata ipl info */
	int	num_of_memdata;	/* loop counter */
	int 	size;		/* return value - total amount of memory */
	int	i;		/* loop control variable */
	int	rc;		/* return code for function calls */

	/*
	 * Determine if there is a memdata table.  This involves three checks.
	 *
	 * 1.  Address check.
	 * The ipl_directory varies in size, so the following calculation
	 * is intended to determine whether the iplcb_dir.mem_data_offset
	 * field exists or not.  Do this by comparing the address of the
	 * mem_data_offset against the address of the ipl_info struct, which
	 * follows the ipl_directory in memory.  The address of the ipl_info
	 * struct is calculated by adding the address of the ipl_directory
	 * (cast to char * to prevent incrementing by size of the struct)
	 * to the ipl_info_offset (subtract 128 for 32 4 byte GP registers).
	 * If the address of mem_data_offset is less than the address of the
	 * ipl_info struct, assume existence and validity of the mem_data_offset.
	 *
	 * 2.  Do the memdata_info_offset and memdata_info_size both equal 0?  If so,
	 * the memdata table is not valid.
	 *
	 * 3.  Check to see if the number of structures is 0.  If so, the memdata
 	 * table is not valid. 
 	 *
	 */


	/* Determine the total memory size and save as sys0 attribute */
	/* Check 1.  If the address of the table conflicts with other info, return -1 */
	if (&iplcb_dir.mem_data_offset >=
		((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128)) 
		return(-1);

	/* It looks like there is a table.  		 	*/
	/* Check 2.  If the offset OR the size is 0, return -1	*/
	if ((iplcb_dir.mem_data_offset == 0) || (iplcb_dir.mem_data_size == 0))
		return(-1);
	
	/* get first memdata section 		*/
	/* set up memdata offset from IPLCB 	*/
	mem_offset = iplcb_dir.mem_data_offset;

	/* read in the first memdata structure  */
	rc = mdd_get(&memdata, mem_offset, sizeof(memdata), MIOIPLCB);
	if (rc)
		err_exit(rc);

	num_of_memdata = memdata.num_of_structs;
	
	/* Check 3.  If the number of mems inside the table says "0", return -1 */
	if (num_of_memdata == 0)
		return(-1);

	/* initialize the size value 					*/
	size = 0;

	/* If we've reached this point, then we can assume that there 	*/
	/* is a memdata table and it is valid. 				*/

	for (i=0;i<num_of_memdata;i++)
	{
		/* get data structure */
	    	rc = mdd_get(&memdata, mem_offset, sizeof(memdata), MIOIPLCB);
		if (rc)
			err_exit(rc);

		/* update memsize */
		size = size + memdata.card_or_SIMM_size;

		/* update offset and continue loop */
		mem_offset = mem_offset + memdata.struct_size;
	}

	/* In early versions of the ROS for the RS2 model, the size is 	*/
	/* given in megabytes.  To allow these machines to come up, a	*/
	/* check is done here.  If the amount of memory we've added up 	*/
	/* is huge, we must have the old ROS.  Divide by 1024.		*/
	/* In a future release, we can delete this check and have only	*/
	/* the 'else' clause of this statement.				*/
	if ((size & 0xFF000000) != 0)
		size = size / 1024;
	else 
		/* we have memory size in megabytes 			*/
		size = 1024 * size;

	return (size);
}


/*
 * NAME: setattr
 *
 * FUNCTION: To set an attribute for a device
 *
 * NOTES: This function uses getattr, and putattr, so default values wont
 *	be placed in CuAt.
 *	The value passed in should be in the same base ( e.g. "0x0" ) as
 *	the default, for putattr to recognize it correctly.
 *
 * RETURNS:  error code.  0 means no error.
 */

int
setattr( lname, aname, avalue )
char	*lname;			/* name of device attribute belongs to */
char	*aname;			/* attribute name */
char	*avalue;		/* attribute value */

{
	struct	CuAt	*cuat;	/* customized attribute object */
	int	how_many;	/* variable needed for getattr call */

	/* routines called */
	struct	CuAt	*getattr();
	int		putattr();

	DEBUG_3("setattr(%s,%s,%s)\n",lname,aname,avalue)

	/* get current attribute setting */
	cuat = getattr(lname, aname, 0, &how_many);
	if (cuat == (struct CuAt *)NULL ) {
		DEBUG_0("ERROR: getattr() failed\n")
		return(E_NOATTR);
	}

	DEBUG_1("Value received by getattr: %s\n", cuat->value )

	/* Only update CuAt object if attr value really changed */
	if (strcmp(cuat->value,avalue)) {
		/* copy new attribute value into cuat structure */
		strcpy( cuat->value, avalue );

		if (putattr( cuat )) {
			DEBUG_0("Error: putattr() failed\n")
			return(E_ODMUPDATE);
		}
	}

	return(0);
}





/*
 * NAME: update_cudv

 *
 * FUNCTION: Update fields of the cudv and write out to database.  Only
 *	     set status to available if flag indicates.
 *
 * RETURNS:  error code.  0 means no error.
 */

int
update_cudv(cudv,location,pname,utype,connection, make_avail)
struct CuDv	*cudv;		/* Pointer to CuDv object */
char		*location;	/* location code for device */
char		*pname;		/* parent name for device */
char		*utype;		/* unique type of device */
char		*connection;	/* connwhere value  for device */
int		make_avail;	/* flag: make available or not? */
{
	int	rc;		/* ODM return code */

	/* Only update CuDv if not already AVAILABLE */
	if (cudv->status != AVAILABLE) {
		DEBUG_1("Updating fields for %s \n", cudv->name)

		/* Set location  code */
		strcpy(cudv->location,location);
		
		/* Set up parent name */
		strcpy(cudv->parent, pname);
		
		/* Set up uniquetype */
		strcpy(cudv->PdDvLn_Lvalue, utype);

		/* Set up connwhere value */
		strcpy(cudv->connwhere, connection);

		/* Set to same if currently missing */
		if (cudv->chgstatus == MISSING) {
			cudv->chgstatus = SAME;
		}

		/* Set status to available, if requested */
		if (make_avail)
			cudv->status = AVAILABLE;

		/* Update CuDv object */
		rc = (int)odm_change_obj( cusdev, cudv );
		if (rc == -1) {
			DEBUG_1("Error changing CuDv entry for %s\n",cudv->name)
			return(E_ODMUPDATE);
		}
	}
	return(E_OK);
}




/*
 * NAME: update_vpd
 *
 * FUNCTION: Updates a devices CuVPD object.  It does not write to CuVPD if
 *	     there has been no change in the device's VPD.
 *
 * RETURNS:  error code.  0 means no error.
 */

int
update_vpd(lname,vpd)
char	*lname;		/* Device's logical name */
char	*vpd;		/* Pointer to VPD buffer */

{
	struct CuVPD cuvpd_obj;		/* customized vpd object */
	char	sstring[256];		/* ODM search string */
	int	rc;			/* ODM return code */

	DEBUG_0("Storing CuVPD\n")

	/* Get device's current CuVPD object */
	sprintf( sstring, "name = %s AND vpd_type = %d",lname, HW_VPD);
	rc = (int)odm_get_first(cusvpd, sstring,&cuvpd_obj);
	if (rc == -1) {
		DEBUG_1("Error reading CuVPD where %s\n",sstring)
		return(E_ODMGET);
	} else if (rc==0) {
		/* There is no VPD in the database so far */
		if (vpd[0] != '\0') {
			/* Add a new object */
			DEBUG_0("Adding VPD\n")
			strcpy( cuvpd_obj.name, lname );
			cuvpd_obj.vpd_type = HW_VPD;
			memcpy(cuvpd_obj.vpd, vpd,VPDSIZE);
			odm_add_obj( cusvpd, &cuvpd_obj );
		}
	} else {
		/* There IS VPD in the database already */
		if (vpd[0] != '\0') {
			rc = memcmp(vpd,cuvpd_obj.vpd,VPDSIZE);
			if (rc != 0) {
				/* The VPD has changed */
				DEBUG_0("Changing VPD\n")
				memcpy(cuvpd_obj.vpd,vpd,VPDSIZE);
				odm_change_obj( cusvpd, &cuvpd_obj);
			}
		} else {
			/* The VPD in the database is no longer valid */
			odm_rm_obj( cusvpd, sstring );
		}
	}
	return(0);
}




/*
 * NAME: get_sysplanar_vpd
 *                                                                    
 * FUNCTION: Obtains the VPD for the sysplanar device, and formats it
 *                                                                    
 * RETURNS:  error code.  0 means no error.
 */  

int
get_sysplanar_vpd(lname, ii)
char	*lname;		/* Device logical name */
IPL_INFO *ii;		/* pointer to IPL info section of Ctrl block */
{
	char	*copy_readable();	/* forward reference to subroutine */
	int	mdd_get();		/* forward reference to subroutine */

	char	vpd[VPDSIZE];		/* storage for holding VPD */
	char	tmpstr1[100],		/* temporary storage */
		tmpstr2[100],
		tmpstr3[100];

	int	sig_no;
	struct	CHIP_SIGNATURE	*sig_ptr;
	int	rc;


	DEBUG_0("get_sysplanar_vpd\n")

	memset(vpd, 0, VPDSIZE);
	*vpd = '\0';

	add_descriptor( vpd, "PN",
			copy_readable( tmpstr1,ii->vpd_planar_part_number,8));

	add_descriptor( vpd, "EC",
			copy_readable( tmpstr1, ii->vpd_planar_ec_number,8));

	add_descriptor( vpd, "PI",
			copy_readable( tmpstr1,
				ii->vpd_processor_serial_number, 8 ) );

	sprintf( tmpstr1, "IPL%s,%s", 
			copy_readable( tmpstr2,ii->vpd_ipl_ros_ver_lev_id,14),
			copy_readable( tmpstr3,ii->vpd_ipl_ros_part_number,8));
	add_descriptor( vpd, "RL", tmpstr1 );

	rc = mdd_get(tmpstr2, OCS_INFO_AREA + 4, 4, MIONVGET);  
	sprintf( tmpstr1, "%08X%08X", ii->model, *(int *)tmpstr2);
	add_descriptor( vpd, "PC", tmpstr1 );

	sig_ptr = &(ii->floating_point.CHIP_SIGNATURE);

	for( sig_no=0; sig_no < 10; sig_no++ )
	{
		sprintf( tmpstr1, "Z%d", sig_no );

		sprintf( tmpstr2, "%02X%02X%02X", 
			(int)(sig_ptr[sig_no].cop_bus_address),
			(int)(sig_ptr[sig_no].dd_number),
			(int)(sig_ptr[sig_no].part_number) );

		add_descriptor( vpd, tmpstr1, tmpstr2 );
	}

	rc = mdd_get(tmpstr2, OCS_EC_LEVEL, 4, MIONVGET);
	sprintf( tmpstr1, "OCS(%08X)", *(int *)tmpstr2);
	add_descriptor( vpd, "RL", tmpstr1 );

	rc = mdd_get(tmpstr2, OCS_EPROM_EC_LEVEL, 4, MIONVGET);
	sprintf( tmpstr1, "SEEDS(%08X)", *(int *)tmpstr2);
	add_descriptor( vpd, "RL", tmpstr1 );

	update_vpd(lname,vpd);

	return(0);
}



/*
 * NAME: copy_readable
 *                                                                    
 * FUNCTION: Copies a maximum of len bytes from dest to source, and null-
 *	terminates the destination. The copy is also halted when a non-
 *	printable character is encountered
 *                                                                    
 * RETURNS: dest
 */  

char *copy_readable( dest, source, len )
char	*dest;
char	*source;
int	len;
{
	int		byte_no = 0;
	char	*dest_ptr = dest;

	while( byte_no < len ) {
		if( *source < ' ' || *source > '\176' )
			break;

		*dest_ptr++ = *source++;
		byte_no++;
	}

	if( byte_no & 1 )
		*dest_ptr++ = ' ';

	*dest_ptr = '\0';
	return dest;
}

/*
 * NAME: get_key
 *
 * FUNCTION: Reads keylock state via ioctl and stores it at "dest".
 *
 * RETURNS:  error code.  0 means no error.
 */
int 
get_key(dest)
int *dest;
{
     int        fd;
     MACH_DD_IO mdd;
     ulong      key = 0;

     if ((fd = open("/dev/nvram", O_RDONLY)) == -1) {
             DEBUG_0("Unable to open /dev/nvram")
             return(E_DEVACCESS);
     }

     DEBUG_0("Getting keylock state\n")

     mdd.md_incr = MV_WORD;
     mdd.md_size = 1;
     mdd.md_data = (uchar *)&key;
     if (ioctl(fd, MIOGETKEY, &mdd) == -1) {
             DEBUG_0("Error getting keylock state via MIOGETKEY ioctl")
             close(fd);
             return(E_DEVACCESS);
     }

     close(fd);
     *dest = key & 0x00000003;
     return(0);
}



/*
 * NAME: mdd_get
 *
 * FUNCTION: Reads "num_bytes" bytes from nvram or IPL control block.
 *	     Bytes are read from the offset into nvram or the IPL 
 *           control block and stored at address "dest". 
 *                                                                    
 * RETURNS:  error code.  0 means no error.
 */  

int
mdd_get(dest, offset, num_bytes, ioctl_type)
char	*dest;
int	offset;
int	num_bytes;
int	ioctl_type;
{
	int		fd;		/* file descriptor */
	MACH_DD_IO	mdd;


	if ((fd = open("/dev/nvram", O_RDONLY)) == -1) { 
                DEBUG_0("Unable to open /dev/nvram")
	        return(E_DEVACCESS);
        }

        DEBUG_2("mdd_get: offset=%08x, size=%08x\n",offset,num_bytes)
        DEBUG_1("mdd_get: ioctl type = %d\n",ioctl_type)

        mdd.md_addr = offset;
	mdd.md_data = dest;
	mdd.md_size = num_bytes;
	mdd.md_incr = MV_BYTE;

	DEBUG_0("Calling mdd ioctl\n")

	if (ioctl(fd, ioctl_type, &mdd) == -1) { 
                DEBUG_0("Error reading IPL-Ctrl block")
                close(fd);
                return(E_DEVACCESS);
        }


	close(fd);
	return(0);
}



/*
 * NAME: detect_device
 *                                                                    
 * FUNCTION: Used to determine the presence of the optical serial link
 *	     chips and the SGA adapter.  It does this by attempting to
 *	     access a register in the device's address range.
 *                                                                    
 * RETURNS:  0 means device is present.
 *	     E_NODETECT means device not present.
 */  

int
detect_device(buid, address)
int	buid;			/* Device's bus Id */
int	address;		/* Address of register to be accessed */

{
	MACH_DD_IO	mdd;		/* Structure for machine dd ioctls */
	int	md_fd;			/* mach dd file descriptor */
	int	data_word;		/* for putting data returned by mdd */
	int	rc;

	/* Open machine dd */
	if ((md_fd = open("/dev/bus0", O_RDONLY)) == -1) {
		DEBUG_0("detect_device: bus0 open failed\n")
		err_exit(E_DEVACCESS);
	}

	mdd.md_incr = MV_WORD;
	mdd.md_size = 1;        /* 1 word (4 bytes) output */
	mdd.md_data = (uchar *)&data_word;
	mdd.md_addr = address;
	mdd.md_buid = buid;
	rc = ioctl(md_fd,MSLAGET,(caddr_t)&mdd);

	/* Close machine device driver */
	close(md_fd);

	if (rc != 0) {
		/* Device not present */
		return(E_NODETECT);
	}

	/* Device is present */
	return(0);
}




/*
 * NAME: put_nvram
 *                                                                    
 * FUNCTION: Reads "num_bytes" bytes from the area starting at "address",
 *	     and writes them to nvram starting at the address "address".
 *                                                                    
 * RETURNS:  error code.  0 means no error.
 */  

int
put_nvram( source, address, num_bytes )
char	*source;
int	address;
int	num_bytes;
{
	int	fd;
	MACH_DD_IO	mdd;

	if ((fd = open("/dev/nvram", O_RDONLY)) == -1)  {
		DEBUG_0("Unable to open /dev/nvram")
		return(E_DEVACCESS);
	}

	mdd.md_addr = address;
	mdd.md_data = source;
	mdd.md_size = num_bytes;
	mdd.md_incr = MV_BYTE;

	if (ioctl(fd, MIONVPUT, &mdd)) {
		DEBUG_0("Error writing NVRAM")
		return(E_DEVACCESS);
	}

	close(fd);
	return(0);
}
/*
 * NAME: parse_params
 *                                                                    
 * FUNCTION: parses the parameters and puts them in global variables
 *                                                                    
 */  
void
parse_params(argc, argv)
int argc;			/* num args in argv */
char *argv[];			/* user's input */
{
   int  errflg,c;  	        /* used in parsing parameters   */


   /* Get the packaging environment variable */
   if (!strcmp(getenv("DEV_PKGNAME"),"ALL"))
	allpkg = 1;

   /*****                                                          */
   /***** Parse Parameters                                         */
   /*****                                                          */

   ipl_phase = RUNTIME_CFG;
   errflg = 0;
   logical_name = NULL;
   
   while ((c = getopt(argc,argv,"l:12")) != EOF) 
   {
   	switch (c) 
   	{
   	case 'l':
   		if (logical_name != NULL)
   			errflg++;
   		logical_name = optarg;
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
   	}
   }

   if (errflg) 
   {
   	/* error parsing parameters */
   	DEBUG_0("cfgsys: command line error\n");
   	exit(E_ARGS);
   }

   /*****                                                          */
   /***** Validate Parameters                                      */
   /*****                                                          */
   /* logical name must be specified */
   if (logical_name == NULL) 
   {
   	DEBUG_0("cfgsys: logical name must be specified\n");
   	exit(E_LNAME);
   }
   
   DEBUG_1("Configuring device: %s\n",logical_name)

   return;

}


/*
 * NAME: open_odm_and_init
 *                                                                    
 * FUNCTION: open the odm object classes
 *		and initialize odm
 *                                                                    
 */  
void
open_odm_and_init()
{
   /* start up odm */
   if (odm_initialize() == -1) 
   {
   	/* initialization failed */
   	DEBUG_0("cfgsys: odm_initialize() failed\n")
   	exit(E_ODMINIT);
   }
   DEBUG_0 ("ODM initialized\n")

   /* open customized devices object class */
   if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) 
   {
   	DEBUG_0("cfgsys: open class CuDv failed\n");
   	err_exit(E_ODMOPEN);
   }

   /* open predefined devices object class */
   if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) 
   {
   	DEBUG_0("cfgsys: open class PdDv failed\n");
   	err_exit(E_ODMOPEN);
   }
   
   /* open customized attribute object class */
   if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1) 
   {
   	DEBUG_0("cfgsys: open class CuAt failed\n");
   	err_exit(E_ODMOPEN);
   }

   /* open predefined attribute object class */
   if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1) 
   {
   	DEBUG_0("cfgsys: open class PdAt failed\n");
   	err_exit(E_ODMOPEN);
   }
     
   /* open CuVPD object class */
   if ((int)(cusvpd = odm_open_class(CuVPD_CLASS)) == -1) 
   {    
   	DEBUG_0("cfgsys: open class CuVPD failed\n");
   	err_exit(E_ODMOPEN);
   }
   DEBUG_0 ("Object classes opened\n")

   return;

}

/*
 * NAME: close_odm_and_terminate
 *                                                                    
 * FUNCTION: close the odm object classes
 *		and terminate odm
 *                                                                    
 */  
void
close_odm_and_terminate()
{
   /* close classes and terminate ODM */
   DEBUG_0( "Closing classes, and terminating odm\n")
   if (odm_close_class(predev) == -1) 
   {
   	DEBUG_0("cfgsys: close object class PdDv failed");
	err_exit( E_ODMCLOSE );
   }

   if (odm_close_class(preatt) == -1) 
   {
	DEBUG_0("cfgsys: close object class PdAt failed");
	err_exit( E_ODMCLOSE );
   }

   if (odm_close_class(cusatt) == -1) 
   {
	DEBUG_0("cfgsys: close object class CuAt failed");
	err_exit( E_ODMCLOSE );
   }

   if (odm_close_class(cusdev) == -1) 
   {
	DEBUG_0("cfgsys: close object class CuDv failed");
	err_exit( E_ODMCLOSE );
   }

   if (odm_close_class(cusvpd) == -1) 
   {
	DEBUG_0("cfgsys: close object class CuVPD failed");
	err_exit(E_ODMCLOSE);
   }

   odm_terminate();

   return;

}


/*
 * NAME: err_exit
 *                                                                    
 * FUNCTION: Closes all object classes and terminates ODM on fatal errors.
 *                                                                    
 * RETURNS: NONE
 */  

void
err_exit(error_code)
int	error_code;
{
	odm_close_class(PdDv_CLASS);
	odm_close_class(PdAt_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_close_class(CuDv_CLASS);
	odm_close_class(CuVPD_CLASS);
	odm_terminate();
	exit(error_code);
}

