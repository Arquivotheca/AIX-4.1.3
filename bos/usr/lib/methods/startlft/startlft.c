static char sccsid[] = "@(#)00	1.1  src/bos/usr/lib/methods/startlft/startlft.c, lftdd, bos411, 9428A410j 11/2/93 08:34:24";
/*
 * COMPONENT_NAME: (LFTDD)	LFT startup program
 *
 * FUNCTIONS: 
 *
 *	main
 *
 * ORIGIN: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*
 * Include files needed for this module follow
 */
#include <lft.h>

/*
 * odm interface include files
 */
#include <stdio.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"


/*
 * NAME: main
 *
 * FUNCTION: 
 *
 *	The purpose of this program is to start up the LFT. It will
 *	first check to see if the lft already has a customized object,
 *	and if so, it will configure the LFT if needed. If there is no
 *	customized device object, we will check to see if any displays
 *	are present for the LFT, and if so, we will define the LFT and
 *	then echo out to standard out the logical name of the LFT.
 */

main(argc, argv)
int	argc;
char	*argv[];
{
struct PdDv		pddv;		/* predefined dependency object */
struct CuDv		cudv;		/* predefined dependency object */
struct CuDv		*cudv_ptr;	/* customized dependency object ptr */
struct objlistinfo	cudv_info;	/* info about cudv object list */
struct PdAt		*pdat_ptr;	/* predefined dependency object ptr */
struct PdAt		*tmp_ptr;	/* tmp predefined dependency obj ptr */
struct objlistinfo	pdat_info;	/* info about pdat object list */
int			rc;		/* return status */
char			crit[256];	/* search criteria string */
char			*outp,		/* return buffer pointers for	*/
			*errp;		/* invoking define & config methods */
int			i;		/* loop variable */

	/*
	 * Start up ODM for further use.
	 */
	if( (rc = odm_initialize()) < 0 )
	{
		cfg_lfterr(NULL,"STARTLFT","startlft","odm_initialize",-1,
				STARTLFT_ODM_INIT, UNIQUE_1);

		DEBUG_0("Can't initialize ODM\n");
		exit(E_ODMINIT);
	}

	/*
	 * Obtain the predefined device object for the LFT type, this
	 * will give us the define and config methods to use for the lft.
	 */
	if( (rc = (int)odm_get_first(PdDv_CLASS, "type='lft'", &pddv)) == 0 )
	{
		cfg_lfterr(NULL,"STARTLFT","startlft","odm_get_first",0,
				STARTLFT_ODM_PDDV, UNIQUE_2);
		DEBUG_0("startlft: no PdDv object for lft\n");
		err_exit(E_NOPdDv);
	}
	else if( rc == -1 )
	{
		cfg_lfterr(NULL,"STARTLFT","startlft","odm_get_first",-1,
				STARTLFT_ODM_PDDV, UNIQUE_3);
		DEBUG_0("startlft: fatal ODM error\n");
		err_exit(E_ODMGET);
	}

	/*
	 * Define/configure the LFT pseudo device (currently only one).
	 * First determine if a customized object was previously created
	 * for the lft. If not, then a check must be made to determine if any
	 * adapters are available before defining and configuring the lft.
	 */

	sprintf(crit, "PdDvLn = '%s'", pddv.uniquetype);
	if( (rc = (int)odm_get_first(CuDv_CLASS, crit, &cudv)) > 0 )
	{
		/*
		 * There is a customized device, see if the LFT
		 * is in the defined state, and if so, configure
		 * the LFT.
		 */

		if( cudv.status == DEFINED )
		{
			/* echo name so cfgmgr will execute it's cfg method */
			fprintf(stdout,"%s\n",cudv.name);
		}
		else 
		{
			/* LFT already available so there is nothing to do. */
			DEBUG_0("lft already available");
		}
	}
	else
	{
		/*
		 * There is no customized device, so check if any
		 * displays are configured for this LFT. If no 
		 * displays are configured, the LFT can't come up.
		 *
		 * First get list of adapter types belonging to the lft 
		 * These types are acquired from the 'uniquetype' field
		 * of the PdAt. An example of the Gt3 adapter type would
		 * be 'adapter/mca/ppr'.
		 */
		pdat_ptr = tmp_ptr = (struct PdAt *)odm_get_list(PdAt_CLASS,
				"deflt='graphics' AND attribute='belongs_to'",
				 &pdat_info,1,1);
		if ( pdat_ptr == (struct PdAt *) -1 ) 
		{
			cfg_lfterr(NULL,"STARTLFT","startlft","odm_get_list",-1,
					STARTLFT_ODM_PDAT, UNIQUE_4);
			DEBUG_0("Error accessing lft data from PdAt\n");
			err_exit(E_ODMGET);
		}

		/*
		 * Loop thru each adapter type looking for the first available 
		 * adapter. Adapters can be determined 'available' from the
		 * CuDv class by having their 'status' field set to 1.
		 */
		for ( i = 0; i < pdat_info.num; i++, pdat_ptr++ ) 
		{
			sprintf(crit, "PdDvLn = '%s' AND status = '1'",
					pdat_ptr->uniquetype);
			cudv_ptr = (struct CuDv *)odm_get_list(CuDv_CLASS,
					crit, &cudv_info,1,1);
			if ( cudv_ptr == (struct CuDv *) -1 ) 
			{
				cfg_lfterr(NULL,"STARTLFT","startlft",
						"odm_get_list",-1,
						STARTLFT_ODM_CUDV, UNIQUE_5);
				DEBUG_0("Error accessing lft data from CuDv\n");
				err_exit(E_ODMGET);
			}

			odm_free_list(cudv_ptr, &cudv_info);

			/* if adapter found get out of loop */
			if ( cudv_info.num > 0 )
				break;
		}
		odm_free_list(tmp_ptr, &pdat_info);

		/* if no available adapters found then exit */
		if ( cudv_info.num <= 0 ) 
		{
			cfg_lfterr(NULL,"STARTLFT","startlft",NULL,0,
					STARTLFT_NO_DISPS, UNIQUE_6);
			DEBUG_0("no lft/display dependency objects\n");
			err_exit(E_OK); 
		}

		/*
		 * The LFT has displays and is ready to come up.
		 * Invoke the associated define
		 * and configure methods for the LFT.
		 */

		rc = odm_run_method(pddv.Define,"-c lft -s node -t lft",
				&outp,&errp);
		if (rc != E_OK) 
		{
			cfg_lfterr(NULL,"STARTLFT","startlft","odm_run_method",
					rc, STARTLFT_DEFINE, UNIQUE_7);
			DEBUG_0(errp);
			err_exit(rc < 0 ? E_ODMRUNMETHOD : rc);
		}

		/* echo name of lft so cfgmgr will execute it's cfg method */
		fprintf(stdout,"%s\n",outp);

	} /* no customized device */

	/*
	 * Terminate the ODM connection and exit with success.
	 */

	odm_terminate();
	exit(E_OK);
}


/* This err_exit routine closes the ODM prior to exiting */

err_exit(exitcode)
char exitcode;
{
	odm_terminate();
	exit(exitcode);
}
