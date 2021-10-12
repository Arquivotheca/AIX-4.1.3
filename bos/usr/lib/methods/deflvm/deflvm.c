static char sccsid[] = "@(#)69  1.2  src/bos/usr/lib/methods/deflvm/deflvm.c, cfgmethods, bos411, 9428A410j 3/30/94 09:28:21";
/*
 * COMPONENT_NAME: (CFGMETH) Define methods for LVM device drivers
 *
 * FUNCTIONS: main()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/cfgdb.h>
#include <stdio.h>
#include <sys/cfgodm.h>
#include <cf.h>


#define LVDD_UTYPE      "lvm/lvm/lvdd"
#define LVDD_TYPE       "lvdd"


/*****									*/
/***** Main Function							*/
/*****									*/

main (argc,argv,envp)
int argc;
char *argv[];
char *envp[];

{
	char sstr[256];		/* search string */
	struct PdDv PdDv;
	struct CuDv CuDv;

	int rc;



	/* start up odm */
	if(odm_initialize() == -1)
		exit(E_ODMINIT);

	/* See if CuDv object exists for LVM device driver */
	sprintf(sstr, "PdDvLn=%s",LVDD_UTYPE);
	rc = (int)odm_get_first(CuDv_CLASS,sstr,&CuDv);
	if (rc == -1)
	{
		/* ODM failure */
		(void)odm_terminate();
		exit(E_ODMGET);
	}
	else if (rc == 0)
	{
		/* No CuDv object found so need to define one */
		/* First need to get PdDv object for LVM device driver */
		sprintf(sstr, "uniquetype = %s", LVDD_UTYPE);
		rc = (int)odm_get_first(PdDv_CLASS, sstr, &PdDv);
		if (rc == 0)
		{
			/* No PdDv object for this device */
			(void)odm_terminate();
			exit(E_NOPdDv);
		}
		else if (rc == -1)
		{
			/* ODM failure */
			(void)odm_terminate();
			exit(E_ODMGET);
		}

		/* Fill in CuDv object */
		strcpy (CuDv.name, LVDD_TYPE);
		CuDv.status = DEFINED;
		CuDv.chgstatus = PdDv.chgstatus;
		strcpy (CuDv.location, "");
		strcpy (CuDv.parent, "");
		strcpy (CuDv.connwhere, "");
		strcpy (CuDv.ddins, PdDv.DvDr);
		strcpy (CuDv.PdDvLn_Lvalue, PdDv.uniquetype);

		/* add customized object to CuDv object class */
		if( odm_add_obj(CuDv_CLASS,&CuDv) == -1 )
		{
			/* ODM error */
			(void)odm_terminate();
			exit(E_ODMADD);
		}

		/* Output the name so cfgmgr will run the config method */
		fprintf(stdout, "%s\n", CuDv.name);

	}
	else
	{
		/* The CuDv already exists for the LVM device driver */
		/* Only output name if cfgmgr needs to run the config method */
		if( CuDv.status != AVAILABLE )
		{
		   /* Output the name so cfgmgr will run the config method */
			fprintf(stdout, "%s\n", CuDv.name);
		}
	}

	(void)odm_terminate();
	exit(0);
}

