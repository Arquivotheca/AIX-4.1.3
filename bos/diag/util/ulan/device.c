static char sccsid[] = "@(#)39	1.9  src/bos/diag/util/ulan/device.c, dsalan, bos41J, 9523B_all 6/6/95 16:16:15";
/*
 * COMPONENT_NAME: LAN service aid 
 *
 * FUNCTIONS: clean_up(), change_data_base(),  configuring_device()
 * 	      find_all_adapters(), create_list()
 *	      
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/devinfo.h>
#include <sys/ciouser.h>
#include <sys/comio.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/tokuser.h>
#include <sys/signal.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>
#include <diag/da.h>
#include <diag/diag_exit.h>
#include <diag/diag.h>
#include <diag/diago.h>
#include <diag/dcda_msg.h>
#include <diag/bit_def.h>
#include <toktst.h>
#include <locale.h>
#include "netdefs.h"
#include "saring.h"
#include "ulan_msg.h"
#include <odmi.h>

int	unconfigure_lpp=FALSE;
int	diag_device_configured=FALSE;
int	set_device_to_diagnose = FALSE;
char	diag_configure_method[128];
extern	int filedes;
extern  ASL_SCR_TYPE device_menutype;
int	what_kind_of_ethernet();
extern  invoke_method();

/*----------------------------------------------------------------------*/
/*	Function 	: clean_up					*/
/*	Description	: close all the devices that are opened		*/
/*			  by this application.				*/
/*			  close the catalog file, close asl      	*/ 
/*			  Restoring the device to the initial state	*/
/*----------------------------------------------------------------------*/

clean_up ()
{
	int	rc, odm_rc;
	char	criteria[256], define_args[512];
	char    odm_search_crit[256], msgstr[1024];
	struct  listinfo        obj_info;
        char    *cfg_outbuf, *eptr;


	display(LAN_STANDBY);
	if (filedes > 0)
		close (filedes);

        configure_lpp_device();

	if ((net_type == TOKEN) && (add_ring_speed_database))
	{
		/* deleting the entry in the database that was added	*/
		/* by DA						*/

		sprintf(odm_search_crit,
		  "odmdelete -o CuAt -q\"name=%s AND attribute=ring_speed\"",
			resource_name);
        	odm_rc =odm_run_method (odm_search_crit, 0, &cfg_outbuf, &eptr);
	}
	else if ((net_type == TOKEN) &&( ring_speed_database_changed))
	{
		sprintf(odm_search_crit,"name=%s AND attribute=ring_speed",
					resource_name);
		rc=(int)odm_get_obj(CuAt_CLASS,odm_search_crit,&cu_at,
			ODM_FIRST);
		if ( rc != 0)
		{
			if (ring_speed == 4)
			{
				strcpy (cu_at.value, "16");
				odm_change_obj(CuAt_CLASS,&cu_at);
			}
			else if (ring_speed == 16)
			{
				strcpy (cu_at.value, "4");
				odm_change_obj(CuAt_CLASS,&cu_at);
			}
		}
	}
	if ((net_type == TOKEN) && (beacon_mac_database_changed))
	{
		sprintf(odm_search_crit,"name=%s AND attribute=beacon_mac",
					resource_name);
		rc=(int)odm_get_first(CuAt_CLASS,odm_search_crit,&cu_at);
		if (rc != 0)
		{
			strcpy (cu_at.value, "no");
			odm_change_obj(CuAt_CLASS,&cu_at);
		}
	}
	else if ((net_type == TOKEN) && (add_beacon_mac_database))
	{
		/* deleting the entry for this Customized Database */
		sprintf(odm_search_crit,
		  "odmdelete -o CuAt -q\"name=%s AND attribute=beacon_mac\"",
			resource_name);
        	odm_rc =odm_run_method (odm_search_crit, 0, &cfg_outbuf, &eptr);
	}

	catclose(catd);
	diag_asl_quit(NULL);
	exit (0);


}
/*----------------------------------------------------------------------*/
/*	Function 	: change_data_base				*/ 
/*	Description	: this is for running on tokenring only		*/
/*			  if the default value of database is		*/
/*			  different from the values that the user	*/
/*			  chose. Then modify the PdAt database		*/
/*----------------------------------------------------------------------*/

change_data_base()
{
	char    odm_search_crit[256];
	struct  listinfo        obj_info;
	int	rc;
	int	diskette_mode;
	int	diskette_based;
	int	how_many;

	int  	odm_rc;
        char    *cfg_outbuf, *eptr;
	char	define_args[512];


	/* get the beacon_mac value of the tokenring		*/

	sprintf(odm_search_crit,"name = %s AND attribute = beacon_mac",
                                       resource_name); 
	rc=(int)odm_get_first(CuAt_CLASS,odm_search_crit,&cu_at);

	if (rc <=0 )	/* no entry for attribute beacon_mac	*/
			/* in Customized attribute database	*/
	{
		sprintf(odm_search_crit,"attribute = beacon_mac");
		rc=(int)odm_get_first(PdAt_CLASS,odm_search_crit,&pd_at);
		if (rc <=0)
		{
				display (UNIDENTIFIED_ERROR);
				clean_up();
		}
	        rc = strcmp (pd_at.deflt, "no");
		if (rc == 0)
		{
			cu_at_obj = getattr(resource_name,
				"beacon_mac",FALSE,&how_many);
			if (cu_at_obj != 0)
			{
				strcpy (cu_at_obj->value, "yes");
				if (putattr (cu_at_obj) <0)
				{
					display (UNIDENTIFIED_ERROR);
					clean_up();
				}
				add_beacon_mac_database = TRUE;
				database_changed = TRUE;
			}
			else	/* problem getting attribute	*/	
			{
				display (UNIDENTIFIED_ERROR);
				clean_up();
			}
		}
	}
	else 	/* there is entry for attribute `beacon_mac` in 	*/
		/* customized attribute database			*/ 
	{

		rc = strcmp (cu_at.value, "yes");
		if (rc != 0)
		{
			strcpy (cu_at.value, "yes");
			rc = odm_change_obj(CuAt_CLASS,&cu_at);
			if ( rc == -1 )
			{
				display (UNIDENTIFIED_ERROR);
				clean_up();
			}
			beacon_mac_database_changed = TRUE;
		}
	}



	/* get the ring speed value of the tokenring		*/

	sprintf(odm_search_crit,"name=%s AND attribute=ring_speed",
				resource_name);

	rc=(int)odm_get_first(CuAt_CLASS,odm_search_crit,&cu_at);
	if (rc == 0)	/* no CuAt available for this one, creating one  */
	{
		sprintf(odm_search_crit,"attribute=ring_speed");
		rc=(int)odm_get_first(PdAt_CLASS,odm_search_crit,&pd_at);
		if (rc <=0)
		{
				display (UNIDENTIFIED_ERROR);
				clean_up();
		}
	        rc = strcmp (pd_at.deflt, "4");
		if (rc == 0)
		{
			if (ring_speed == 16)
			{
				/* get attribute */
				cu_at_obj = getattr(resource_name,
					"ring_speed",FALSE,&how_many);
				if (cu_at_obj != 0)
				{
					diskette_mode=ipl_mode(&diskette_based);
                			if (diskette_based == DIAG_FALSE)
                			{
						display (WARNING_1);
					}
					add_ring_speed_database = TRUE;
					database_changed = TRUE;
					strcpy (cu_at_obj->value, "16");
					if (putattr (cu_at_obj) <0)
					{
						display (UNIDENTIFIED_ERROR);
						clean_up();
					}
				}
				else
				{
					display (UNIDENTIFIED_ERROR);
					clean_up();
				}
				return (GOOD);
			}

		}
	        rc = strcmp (pd_at.deflt, "16");
		if (rc == 0)
		{
			if (ring_speed == 4)
			{
				/* get attribute */
				cu_at_obj = getattr(resource_name,
					"ring_speed",FALSE,&how_many);
				if (cu_at_obj != 0)
				{
					diskette_mode=ipl_mode(&diskette_based);
                			if (diskette_based == DIAG_FALSE)
                			{
						display (WARNING_1);
					}
					add_ring_speed_database = TRUE;
					database_changed = TRUE;
					strcpy (cu_at_obj->value, "4");
					if (putattr (cu_at_obj) <0)
					{
						display (UNIDENTIFIED_ERROR);
						clean_up();
					}
				}
				else
				{
					display (UNIDENTIFIED_ERROR);
					clean_up();
				}
				return (GOOD);
			}
		}
	}

	/* There is an entry for this attribute in Customized attribute */
	rc = strcmp (cu_at.value, "4");
	/* if ring speed in predefined database is 4 Meg bits		*/
	if ( rc == 0)
	{
		/* if ring speed that user choose is 16, change database */
		if (ring_speed == 16)
		{
			/* display the warning message tell the user 	*/
			/* about the different between ring speed in 	*/
			/* database and the chosen one			*/

			diskette_mode = ipl_mode(&diskette_based);
                	if (diskette_based == DIAG_FALSE)
                	{
				display (WARNING_1);
			}

			strcpy (cu_at.value, "16");
			rc = odm_change_obj(CuAt_CLASS,&cu_at);
			if (rc == -1)
			{
				display (UNIDENTIFIED_ERROR);
				clean_up();

			}
			/* set this flag so in clean_up restoring the	*/
			/* CuAt database to the normal			*/
			ring_speed_database_changed = TRUE;
			database_changed = TRUE;
		}
		return (GOOD);
	}
	rc = strcmp (cu_at.value, "16");
	if ( rc == 0)		/* default ring speed in database is 16 Meg */
	{
		/* if the user chooses ring speed 4 Meg, change it	*/
		if (ring_speed == 4)
		{
			/* display the warning message tell the user 	*/
			/* about the different between ring speed in 	*/
			/* database and the chosen one			*/

			diskette_mode = ipl_mode(&diskette_based);
                	if (diskette_based == DIAG_FALSE)
                	{
				display (WARNING_1);
			}

			strcpy (cu_at.value, "4");
			odm_change_obj(CuAt_CLASS,&cu_at);
			if ( rc == -1)
			{
				display (UNIDENTIFIED_ERROR);
				clean_up();
			}

			/* set this flag so in clean_up restoring the	*/
			/* CuAt database to the normal			*/
			database_changed = TRUE;
			ring_speed_database_changed = TRUE;
		}
		return (GOOD);
	}

}

/*---------------------------------------------------------------------------
 *
 * NAME: find_all_adaps
 *
 * FUNCTION: finds out what adapters are in the system by looking in the cudv 
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: num_found = number of LAN adapters in system
 *			      	     -1 = error occurred
 ---------------------------------------------------------------------------*/
int find_all_adapters()
{
	char criteria[256];
	struct listinfo *obj_info;
	struct CuDv *cudv;
	int count, num_found;
	int	max_expect=8;
	int	depth = 1;

	num_devices_found = 0;
	cudv = (struct CuDv *) malloc (sizeof (struct CuDv));
	obj_info = (struct listinfo *) malloc (sizeof (struct listinfo));

	/* searching in ODM for the Ethernet device and build 		*/
	/* the test list 						*/

	sprintf(criteria, "name LIKE ent? AND chgstatus != 3 ");
	cudv = odm_get_list (CuDv_CLASS, criteria, obj_info, max_expect, depth);
	if (cudv != (struct CuDv *) -1)
	{
		num_found = obj_info->num;
		num_devices_found = num_found;

		for (count = 0; count < num_found; count ++)
		{
			device_found[count] = (char *) malloc (sizeof(64));
			location[count] = (char *) malloc (sizeof(64));
			strcpy (device_found[count], cudv[count].name);
			strcpy (location[count], cudv[count].location);
			device_type[count] = ETHERNET;
		}
	}

	/* searching in ODM for the Tokenring device and build 		*/
	/* the test list 						*/

	sprintf(criteria, "name LIKE tok? AND chgstatus != 3");
	cudv = odm_get_list (CuDv_CLASS, criteria, obj_info, max_expect, depth);
	if (cudv != (struct CuDv *) -1)
	{
		num_found = obj_info->num;

		for (count = 0; count < num_found; count ++)
		{
			device_found[count+num_devices_found] 
					= (char *) malloc (sizeof(64));
			location[count+num_devices_found] 
					= (char *) malloc (sizeof(64));
			strcpy (device_found[count+num_devices_found], 
					cudv[count].name);
			strcpy (location[count+num_devices_found], 
					cudv[count].location);
			device_type[count+num_devices_found] = TOKEN;
		}
		num_devices_found += num_found;
	}

	/* searching in ODM for the fddi device and build 		*/
	/* the test list 						*/

	sprintf(criteria, "name LIKE fddi? AND chgstatus != 3");
	cudv = odm_get_list (CuDv_CLASS, criteria, obj_info, max_expect, depth);
	if (cudv != (struct CuDv *) -1)
	{
		num_found = obj_info->num;

		for (count = 0; count < num_found; count ++)
		{
			device_found[count+num_devices_found] 
					= (char *) malloc (sizeof(64));
			location[count+num_devices_found] 
					= (char *) malloc (sizeof(64));
			strcpy (device_found[count+num_devices_found], 
					cudv[count].name);
			strcpy (location[count+num_devices_found], 
					cudv[count].location);
			device_type[count+num_devices_found] = FDDI;
		}
		num_devices_found += num_found;
	}

	return(num_devices_found);
}

/*------------------------------------------------------------------------
 *
 * NAME: create_list
 *
 * FUNCTION: creates a tested list for LAN service aid.
 *
 * NOTES:
 *
 ------------------------------------------------------------------------*/

create_list ()
{
        int     index;
        int     line = 0;
        char    *string;
        char    *string2[8];

	/* if pointer is not allocated and number of device found	*/
	/* in database is greater than 0  				*/
        if ((!device_info) && (num_devices_found > 0))
	{
                device_info = (ASL_SCR_INFO *)
                    calloc(1, (num_devices_found+2)*sizeof(ASL_SCR_INFO));
                string = (char *)malloc(132);
                device_info[line++].text = (char *) diag_cat_gets(catd, 1,
                        ACTION);

                for(index = 0; index < num_devices_found; index++)
		{
			if (device_type[index] == ETHERNET)
			{
                		string = (char *)diag_cat_gets(catd, 1, 
					ETHERNET_ADAP);
			}
			else if (device_type[index] == TOKEN)
			{
                		string = (char *)diag_cat_gets(catd, 1, 
					TOKENRING_ADAP);
			}
			else if (device_type[index] == FDDI)
			{
                		string = (char *)diag_cat_gets(catd, 1, 
					FDDI_ADAP);
			}
			else
			{
				return (-1);
			}
                        string2[index] = (char *)malloc(132);
                        sprintf(string2[index], string, device_found[index],
                            location[index]);
                        device_info[line].text = string2[index];
                        device_info[line].non_select = ASL_NO;
                        line++;
		}
                device_info[line].text = (char *) diag_cat_gets(catd, 1,
                        SELECT);
		device_menutype.max_index = line;
	}
}
/*---------------------------------------------------------------------------
 * NAME:  unconfigure_lpp_device
 *
 * FUNCTION: unconfigure the device to be tested (DEFINE state)
 *           load the diagnostic driver (DIAGNOSE state)
 *	     load device driver
 *
 * NOTES:
 *
 ---------------------------------------------------------------------------*/
unconfigure_lpp_device()
{
	char 	criteria[256], define_args[512];
	char 	msgstr[1024];
	char	*cfg_outbuf, *eptr;
	int  	result;

        result = diagex_cfg_state ( resource_name );

        switch (result) {
                case 2:
			sprintf(msgstr, (char *) diag_cat_gets(catd, 1,
				RESOURCE_BUSY));
			diag_asl_msg ("%s\n", msgstr);
			clean_up();
                        break;
                case 3: sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE),
					resource_name, device_location);
			diag_asl_msg ("%s\n", msgstr);
                        clean_up();
                        break;
        }
	set_device_to_diagnose=TRUE;

	/* load diagnostics device driver into system */
	sprintf(define_args, "-l %s -f 0", resource_name);
	if (net_type == TOKEN)
	{
		strcpy (diag_configure_method, "/usr/lib/methods/cfgddtok");
	}
	else if (net_type == FDDI)
	{
		strcpy (diag_configure_method, "/usr/lib/methods/cfgddfddi");
	}
	else if (net_type == ETHERNET)
	{
		result = what_kind_of_ethernet (resource_name);
		switch (result)
		{
			case 1:	/* Old 3 com adapter 	*/
				strcpy (diag_configure_method, 
					"/usr/lib/methods/cfgddent");
				break;
			case 2:	/* Salmon and Rainbow 3 	*/
			case 3:
				strcpy (diag_configure_method, 
					"/usr/lib/methods/cfgddeth");
				break;
			default: 
				break;
		}
	}

	result = invoke_method(diag_configure_method, 
			define_args, &cfg_outbuf, &eptr);
	free(cfg_outbuf);
	free(eptr);
	if (result == GOOD)
	{
		diag_device_configured = TRUE;
	}
	else
	{
		sprintf(msgstr, (char*) diag_cat_gets(catd, 1,
			DIAG_DEVICE_CANNOT_CONFIGURED),
			resource_name, device_location);
		diag_asl_msg ("%s\n", msgstr);
		clean_up();
	}
		
}
/*------------------------------------------------------------------------
 * NAME: configure_lpp_device
 *
 * FUNCTION: unload device driver 
 *	     unload diagnostics device driver
 *           reconfigure the device to its original state
 *
 * NOTES: called from clean_up()
 *
 ------------------------------------------------------------------------*/
configure_lpp_device()
{
	
	char	criteria[256], define_args[512];
	char	msgstr[1024];
	char	*cfg_outbuf, *eptr;
	int 	result;

	/* unload device driver from system */
	if (diag_device_configured)
	{
		sprintf(define_args, "-l %s -f 1", resource_name);
		result = invoke_method(diag_configure_method, define_args, 
					&cfg_outbuf, &eptr);
		free(cfg_outbuf);
		free(eptr);
		if (result != 0)
		{
			sprintf(msgstr, (char*) diag_cat_gets(catd, 1,
				LPP_DEVICE_CANNOT_UNCONFIGURED),
				resource_name, device_location);
			diag_asl_msg ("%s\n", msgstr);
		}
	}

	/* setting the device state to DEFINE state */
	/* clean_up() will restore the state to original */
	/* DIAGNOSE state of the device is set to DEFINE state */
	/* even though before running diagnostics it is in DIAGNOSE state */
	if (set_device_to_diagnose)
	{
		result = diagex_initial_state (resource_name);
                if (result != 0)
                {
                        sprintf(msgstr, (char *)diag_cat_gets (
                                catd, 1,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
				resource_name, device_location);
			diag_asl_msg ("%s\n", msgstr);
                }
		else
			set_device_to_diagnose = FALSE;
	}
}


int	what_kind_of_ethernet(device_name)
char	*device_name;
{
        char    odm_search_crit[40];
        struct  CuDv            *cudv;
        struct  listinfo        obj_info;
	int	rc;

	sprintf(odm_search_crit,
       		"name = %s",device_name);
               cudv = get_CuDv_list( CuDv_CLASS,
 	odm_search_crit, &obj_info, 1, 2 );
	rc = strcmp ("adapter/mca/ethernet", cudv->PdDvLn_Lvalue);
	if 	/* old 3 Com card	*/
		(rc == 0) return (1);

	rc = strcmp ("adapter/sio/ient_1", cudv->PdDvLn_Lvalue);
	if 	/* Stilwell Integrated Ethernet */
		(rc == 0) return (2);

	rc = strcmp ("adapter/sio/ient_2", cudv->PdDvLn_Lvalue);
	if 	/* Salmon Integrated Ethernet */
		(rc == 0) 
	{
		return (3);
	}
	rc = strcmp ("adapter/sio/ient_6", cudv->PdDvLn_Lvalue);
	if 	/* Rainbow 3 Integrated Ethernet */
		(rc == 0) 
	{
		return (3);
	}
	rc = strcmp ("adapter/sio/ient", cudv->PdDvLn_Lvalue);
	if 	/* Integrated Ethernet */
		(rc == 0) 
	{
		return (3);
	}

	return (0);
}
