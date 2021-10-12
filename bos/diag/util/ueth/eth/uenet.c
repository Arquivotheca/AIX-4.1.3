static char sccsid[] = "@(#)36	1.6  src/bos/diag/util/ueth/eth/uenet.c, dsaueth, bos41J, 9523B_all 6/6/95 17:44:53";
/*
 *   COMPONENT_NAME: DSAUETH
 *
 *   FUNCTIONS: create_dev_name
 *		main
 *		process_rc
 *		serv_aid
 *		what_kind_of_ethernet
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

#define		NOT_CONFIG	-1
#define		NO_ERROR	0

#include <stdio.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include "diag/diago.h"
#include "ethtst.h"
#include "uenet_msg.h"
#include "uenet.h"
#include <locale.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

extern disp_menu();
extern  int exectu (int,TUTYPE *);
extern int errno;
extern char *dname[8];
int fdes = 0;
int reason = 0;
TUTYPE tucb;
nl_catd catd;
char *devname;			/* name of device to be tested */
int	adap_num;
char	*dnameloc[8];

char 	device_to_test[16];
char	device_location[16];	
char	msgstr[256];
char	msgstr1[256];
char	option[256];
char	*new_out, *new_err;
int	diag_device_configured=FALSE;
int	set_device_to_diagnose = FALSE;

/**************************************************************************/
/*
 * NAME: main
 *
 * FUNCTION: The main routine for the Ethernet Service Aid.
 *           It is invoked by the Diagnostic Controller.
 *	     It handles setting up asl and the odm and invokes
 *	     a routine to handle the rest.
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: none
 */

main()
{
	setlocale (LC_ALL, "");
	catd = diag_catopen(CAT_NAME, 0);
	diag_asl_init("NO_TYPE_AHEAD");
	init_dgodm();
	serv_aid();
	term_dgodm();
	diag_asl_quit(NULL);
	catclose(catd);
	exit (0);
}
/************************************************************************/
/*
 * NAME: serv_aid
 *
 * FUNCTION: handles user interface, opening and closing ethernet device
 * 	     driver and executing test units to check wrap facility.
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: none
 */

serv_aid()
{
	int 	rc; /* results of wrap test */
	int 	devnum; /* array index of device name */
	int 	con_type; /* type of connector used */
	char    *diagdir;       /* points to default diag path          */
        char    *com;           /* points to command to execute         */
        char    command[128];   /* holds full path of command           */


	rc = disp_menu(DESCRIP);
	devnum = disp_menu(ACTION) - 1;
	if (devnum == -1)
	{
		disp_menu(NO_ADAPS);
		return;
	}
	else if (devnum == -2)
	{
		disp_menu(SYS_ER);
		return;
	}
	rc = create_dev_name(devnum);
	if (rc == -1)
	{
		disp_menu(SYS_ER);
		return;
	}
	else
	{
     		/* get directory path to use for command execution of diag's */
        	if((diagdir = (char *)getenv("DIAGNOSTICS")) == NULL )
               		diagdir = "/usr/lpp/diagnostics";
        	strcpy(command, diagdir);
        	strcat(command, "/bin/");
        	com = command + strlen(command);

		rc = what_kind_of_ethernet (dname[devnum]);

		/* the Ethernet card is Stilwell Ethernet 	*/
		if (rc == 2) 
		{
		     term_dgodm();
		     diag_asl_quit (NULL);
		     catclose (catd);
	    	     strcpy(com, "stiluenet");
		     execlp (command, "stiluenet", dname[devnum], dnameloc[adap_num],0);
		}

		/* The Ethernet Card is Salmon Ethernet		*/
		if ( rc == 3)
		{
		     term_dgodm();
		     diag_asl_quit (NULL);
		     catclose (catd);
	    	     strcpy(com, "saluenet");
		     execlp (command, "saluenet", dname[devnum], dnameloc[adap_num],0);
		}

		if (rc != 1)	 /* old 3 com card	*/
		{
			disp_menu (SYS_ER);
			clean_up ();
		}

		strcpy (device_to_test, dname[devnum]);
		strcpy (device_location, dnameloc[adap_num]);

		/* Unconfigure lpp device and configuring diagnostics device */
 
		unconfigure_lpp_device ();

		fdes = open(devname, O_RDWR | O_NDELAY);
        	if (fdes == -1)     /* if device driver doesn't open */
		{
			reason = errno;
        		rc = fdes;
		}
		else
		{
			tucb.header.mfg = 0;
			tucb.header.loop = 1;
			con_type = disp_menu(WHICH_CON);
			disp_menu(STANDBY);
			switch(con_type)
			{
				case DIX:
					tucb.header.tu = 7;
					break;
				case BNC:
					tucb.header.tu = 8;
					break;
			}
			rc = exectu(fdes, &tucb); 
		}
		process_rc(rc);
		clean_up();
	}
	return;
}
/*************************************************************************/
/*
 * NAME: create_dev_name
 *
 * FUNCTION: creates the name of the device to be tested
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: none
 */
create_dev_name(devnum)
int devnum;
{
	char dir[] = "/dev/";	/* first part of device name */
	char openflag[] = "/D";	/* flag for diagnostic open */

	/* create device name */
	devname = (char * ) malloc(DEVNAMESIZE + (strlen(dir)+1));
	if (devname == NULL)
		return(-1);
	strcpy(devname, dir);
	strcat(devname, dname[devnum]);
	strcat(devname, openflag);
	return(0);
}
/************************************************************************/
/*
 * NAME: process_rc
 *
 * FUNCTION: invokes results menu according to the return code
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: none
 */
process_rc(rc)
int rc;
{
	switch(rc)
		{
		case 0:
			disp_menu(NO_ERR);
			break;
		case -1:
			switch(reason)
				{
				case EBUSY:
				case ENOTREADY:
					disp_menu(ADAP_BUSY);
					break;
				default:
					disp_menu(SYS_ER);
					break;
				}
			break;
		case CONFIG_ERR:
		case DMA_NOTA_ERR:
		case DMA_ALOC_ERR:
		case DMA_MAL_ERR:
		case DMA_RD_ERR:
		case DMA_WR_ERR:
		case DMA_FREE_ERR:
		case DMA_UNK_ERR:
		case TRBUF_MAL_ERR:
		case ILLEGAL_TU_ERR:
		case BAD_DMA_S_ERR:
		case BAD_PCK_S_ERR:
			disp_menu(SYS_ER);
			break;
		case TXBD_ST_RD_ERR:
		case RXBD_ST_RD_ERR:
			disp_menu(ADAP_ERR);
			break;
		case TXBD_TIME_ERR:
			disp_menu(TX_TIME);
			break;
		case TXBD_NOK_ERR:
			disp_menu(TX_ERR);
			break;
		case RXBD_TIME_ERR:
			disp_menu(RX_TIME);
			break;
		case RXBD_NOK_ERR:
			disp_menu(RX_ERR);
			break;
		case WRAP_CMP_ERR:
			disp_menu(WRAP_ERR);
			break;
		default:
			disp_menu(UNID_ERR);
			break;
		}
}
/************************************************************************/


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

	return (0);
}

/*---------------------------------------------------------------------------
 * NAME: configure_lpp_device () 
 *
 * FUNCTION: configure_lpp_device ()
 *	     Unload the diagnostic device driver 
 *	     setting the device to the DEFINE state (No matter what state
 *	     it is in before running the test. We do not want left it in
 *	     DIAGNOSE state after running diagnostics 
 *	     clean_up will restore the AVAILABLE state if it is so before
 *	     running the test
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 *--------------------------------------------------------------------------*/
int	configure_lpp_device()
{
        char    criteria[128];
	int	result;

	/* Unconfigure diagnostics device. Unload the device from system */
	if(diag_device_configured)
	{
		/* UCFG is defined as 1			*/
		sprintf (option," -l %s -f 1", device_to_test);

		strcpy (criteria,"/usr/lib/methods/cfgddent");

		result = invoke_method(criteria,option, &new_out,&new_err);
		free(new_out);
		free(new_err);
		if (result !=  NO_ERROR)
		{
			sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                DIAG_DEVICE_CANNOT_UNCONFIGURED),
                                device_to_test, device_location);

			diag_asl_msg ("%s\n", msgstr);
			return (-1);
		}
	}

	/* Setting the device state to original state */
	if (set_device_to_diagnose)
	{
		result = diagex_initial_state( device_to_test ); 
		if (result == NO_ERROR)
		{
			set_device_to_diagnose= FALSE;
		}
		else 
		{
                       sprintf(msgstr, (char *)diag_cat_gets (
                          catd, 1,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
                                 device_to_test, device_location);
			diag_asl_msg ("%s\n", msgstr);
			return (-1);
		}
	}
}
/*--------------------------------------------------------------*/
/*      NAME: unconfigure_lpp_device 				*/
/*      Description:    					*/
/*	Return		: 0 Good 				*/
/*			  -1 BAD				*/
/*--------------------------------------------------------------*/
int	unconfigure_lpp_device()
{
        char    criteria[128];
	int	result;


        result = diagex_cfg_state ( device_to_test );

        switch (result) {
                case 2:
			sprintf(msgstr, (char *)diag_cat_gets (
                                        catd, 1,LPP_DEVICE_CANNOT_UNCONFIGURED),
    					device_to_test, device_location);
                        diag_asl_msg ("%s\n", msgstr);
                        clean_up();
                        break;
                case 3: sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE),
    				device_to_test, device_location);
                        diag_asl_msg ("%s\n", msgstr);
                        clean_up();
                        break;
        }
	set_device_to_diagnose= TRUE;

	/* The diagnostics device driver needs to be loaded into the system */

	/* CFG is defined as 0	*/
	sprintf (option," -l %s -f 0", device_to_test);

	strcpy (criteria,"/usr/lib/methods/cfgddent");
	result = invoke_method(criteria,option, &new_out,&new_err);
	free(new_out);
	free(new_err);
	if (result == NO_ERROR)
	{
		diag_device_configured=TRUE;
	}
	else
	{
		sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
			DIAG_DEVICE_CANNOT_CONFIGURED),
    				device_to_test, device_location);
                diag_asl_msg ("%s\n", msgstr);
		clean_up();
	}

	return (0);
}

