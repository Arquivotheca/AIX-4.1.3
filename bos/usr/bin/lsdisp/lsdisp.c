static char sccsid[] = "@(#)50	1.16.2.3  src/bos/usr/bin/lsdisp/lsdisp.c, cmdlft, bos411, 9428A410j 12/13/93 15:00:09";

/*
 * COMPONENT_NAME: (CMDLFT) Low Function Terminal Commands
 *
 * FUNCTIONS:  main, create_list, print_list
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------
  PURPOSE:
  Display a list of physical displays available on the lft.

  FUNCTIONS:
  create_list() - create a linked list of adapter information for each
	adapter available to the lft.

  print_list() - display the logical name, slot number, physical name, 
	and description for each adapter in the list
  ------------*/

#include <locale.h>
#include "lftcmds.h"
#include "lftcmds_msg.h"
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <odmi.h>

#define SUCCESS 0
#define FAIL -1

struct _displays {
	int order;
	char logical_name[NAMESIZE];
	char slot_nbr[LOCSIZE];
	char physical_name[NAMESIZE];
	char bus_type[CLASSIZE];
	char description[DEFAULTSIZE];
	struct _displays *next;
} *displays = NULL;

int nbr_of_displays;
char default_disp[NAMESIZE];
int lflag = FALSE;


main(argc, argv)
int argc;
char *argv[];
{

	int rc;

	setlocale(LC_ALL,"");

	/* Open the message facilities catalog.  Retrieve the program name. */
	catd=catopen("lftcmds.cat", NL_CAT_LOCALE);
	strcpy(program, MSGSTR(LSDISP, LSDISP_PGM, M_LSDISP_PGM));

	/* Verify correct options were specified */
	if (! (argc == 1 || (argc == 2 && strcmp(argv[1],"-l") == 0)) ) 
	{
		if ( argc == 2)
			fprintf(stderr,MSGSTR(COMMON, BADFLAG, M_BADFLAG),
					program,argv[1]);
		fprintf(stderr,MSGSTR(LSDISP, LSDISP_USAGE, M_LSDISP_USAGE));
		exit(FAIL);
	}
	if ( argc == 2 )
		lflag = TRUE;

	/* Create the display list */
	if ( create_list() != SUCCESS)
	{
		catclose(catd);
		exit(FAIL);
	}

	/* print them out */
	rc = print_list();

	/* Close the message facilities catalog */
	catclose(catd);

	exit(rc);
}



/************************************************************************
  The create_list function will create a linked list containing information 
  about each available lft display. It also sorts the list in ascending 
  order by the slot number for each display.
************************************************************************/

int create_list()
{

	int rc, x, y, order;		/* temporary variables */
	char crit[60];
	char *str_ptr;
	struct PdAt *pdat,*tmp_pdat;	/* predefined attribute obj pointer */
	struct PdDv pddv;		/* predefined device structure */
	struct CuDv *cudv,*tmp_cudv;	/* customized device object storage */
	struct CuAt *cuat;		/* customized attribute obj storage */
	struct objlistinfo pdat_info;	/* result of search stats */
	struct objlistinfo cudv_info;	/* result of search stats */
	struct _displays *disp_ptr;	/* pointer to display structure */
	struct _displays *tmp_ptr;	/* pointer to display structure */
	nl_catd disp_catd;		/* msg file descriptor for displays */


	/* Initialize the ODM database for access */
	if( (rc = odm_initialize()) < 0 ) 
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_INIT, M_ODM_INIT), program);
		return(FAIL);
	}

	/* Get a list of all adapter types belonging to the lft from the PdAt.
	   The adapter type is stored in the 'uniquetype' field of the PdAt.
	   As an example, the Gt4 type would be 'adapter/mca/ppr'.
	 */

	pdat = tmp_pdat = (struct PdAt *) odm_get_list(PdAt_CLASS,
			"deflt = 'graphics' AND attribute = 'belongs_to'",
			&pdat_info,1,1);
	if ( pdat == (struct PdAt *) -1 ) 
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_FAIL, M_ODM_FAIL), program,
				"odm_get_list()", "PdAt",
				"deflt='graphics' AND attribute='belongs_to'");
		odm_terminate();
		return(FAIL);
	}

	/* loop thru each adapter type for a list of available adapters */
	for ( x = 1; x <= pdat_info.num; x++, pdat++ ) 
	{

		/* get a list of available adapter names from each adapter 
		 * type. The names come from the 'name' field of the CuDv 
		 * and will be like 'ppr0'. The 'status' field having a value
		 * of 1 means the * given adapter is available.
		 */
		sprintf(crit, "PdDvLn = '%s' AND status = '%d'",
				pdat->uniquetype, AVAILABLE);
		cudv = tmp_cudv = (struct CuDv *)odm_get_list(CuDv_CLASS, crit,
					&cudv_info,1,1);
		if ( cudv == (struct CuDv *) -1 ) 
		{
			odm_free_list(tmp_pdat, &pdat_info);
			fprintf(stderr,MSGSTR(COMMON, ODM_FAIL, M_ODM_FAIL),
					program,"odm_get_list()","CuDv", crit);
			odm_terminate();
			return(FAIL);
		}

		/* loop thru list of available adapters, creating a linked 
		 * list of adapter information. After list is complete, sort
		 * the list by each adapter's slot number.
		 */
		for (y = 1; y <= cudv_info.num; y++, cudv++ ) 
		{
			/* malloc new structure for linked list */
			tmp_ptr = (struct _displays*) 
					malloc( sizeof(struct _displays) );
			if ( tmp_ptr == NULL )
			{
				odm_free_list(tmp_pdat, &pdat_info);
				odm_free_list(tmp_cudv, &cudv_info);
				fprintf(stderr,MSGSTR(COMMON,NO_SYS_MEM,
						M_NO_SYS_MEM), program);
				odm_terminate();
				return(FAIL);
			}		
			if ( ! displays )
			{
				displays = disp_ptr = tmp_ptr;
			}
			else
			{
				disp_ptr->next = tmp_ptr;
				disp_ptr = disp_ptr->next;
			}
			disp_ptr->next = NULL;

			/* store all adapter info into linked list structure */
			strcpy( disp_ptr->logical_name, cudv->name );

			/* skip initial digits to get the slot number */
			str_ptr = cudv->location;
			while(*str_ptr != '-') str_ptr++;
			strcpy( disp_ptr->slot_nbr, ++str_ptr ); 

			/* get the adapter name from either CuAt or PdAt */
			cuat = getattr(cudv->name, "dsp_name", FALSE, &rc);
			if ( cuat == NULL )
			{ 
				odm_free_list(tmp_pdat, &pdat_info);
				odm_free_list(tmp_cudv, &cudv_info);
				fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ,
						M_ODM_NOOBJ), program,
						"getattr()","PdAt","dsp_name");
				odm_terminate();
				return(FAIL);
			}
			strcpy( disp_ptr->physical_name, cuat->value );

			/* Get the adapter description from the message 
			 * catalog. Check if the adapter has a _subtype,
			 * if so then use the _subtype msg number from the
			 * CuAt otherwise use the message number from the PdDv.
			 */
			sprintf(crit, "uniquetype = %s", pdat->uniquetype);
			if( odm_get_obj(PdDv_CLASS, crit, &pddv, TRUE) < 1)
			{
				odm_free_list(tmp_pdat, &pdat_info);
				odm_free_list(tmp_cudv, &cudv_info);
				fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ,
						M_ODM_NOOBJ), program);
				odm_terminate();
				return(FAIL);
			}

			 /* open the catalog file */
			disp_catd = catopen(pddv.catalog, NL_CAT_LOCALE);
			
			if((cuat=getattr(cudv->name,"_subtype",FALSE,&rc)) > 0)
			{ 
				strcpy( disp_ptr->description,
						catgets(disp_catd,pddv.setno,
						atoi(cuat->value),
						"Graphics Adapter"));
			}
			else
			{ 
				strcpy( disp_ptr->description,
						catgets(disp_catd,pddv.setno,
						pddv.msgno,
						"Graphics Adapter"));
			}

			catclose(disp_catd);

			/* get the adapter bus type from the PdDv */
			strcpy( disp_ptr->bus_type, pddv.subclass);

			/* increment the available display count */
			nbr_of_displays++;  

		} /* end of each available adapter loop */

		/* free up the list of available adapters */
		odm_free_list(tmp_cudv, &cudv_info);

	} /* end of each adapter type loop */

	/* free up the list of all adapter types */
	odm_free_list(tmp_pdat, &pdat_info);

	/* get the default display name from either CuAt or PdAt */
	if ((cuat = getattr("lft0", "default_disp", FALSE, &rc)) == NULL) 
	{
		fprintf(stderr,MSGSTR(COMMON,NO_DFLT,M_NO_DFLT), program);
	}
	else if ( cuat->value[0] == '\0' )
		strcpy( default_disp, "unknown");
	else
		strcpy( default_disp, cuat->value );

	/* close down the ODM */
	odm_terminate();

	/* now sort the list by slot numbers. Slot numbers are in the 
	 * form '00-03' so that only the last 2 digits representing the 
	 * slot are sorted.
	 */
	for (disp_ptr=displays; disp_ptr != NULL; disp_ptr=disp_ptr->next)
	{
		x = atoi(disp_ptr->slot_nbr);
		for(tmp_ptr=displays, order=0; tmp_ptr != NULL;
				tmp_ptr=tmp_ptr->next) 
		{
			if( x > atoi(tmp_ptr->slot_nbr) )
				order++;
		}
		disp_ptr->order = order;
	}

	/* print error if no displays otherwise just return */
	if ( ! nbr_of_displays )
	{
		fprintf(stderr,MSGSTR(LSDISP, LSDISP_NODSP, M_LSDISP_NODSP));
		return(FAIL);
	}
	else
		return(SUCCESS);
}


/************************************************************************
  The print_list function prints the lsdisp header and then traverses 
  the display linked list printing the information in their selected
  order. The default display is printed at the end of the list.
************************************************************************/

int print_list()
{
	int x;				/* Temporary variables */
	struct _displays *tmp_ptr;	/* pointer to the display structure */

	if ( lflag == FALSE )
	{
		/* print header info from message catalog */
		printf("\n\n%-10s%-6s%-5s%-11s%-47s\n",
			MSGSTR(LSDISP, LSDISP_DEV_NAME, M_LSDISP_DEV_NAME),
			MSGSTR(LSDISP, LSDISP_SLOT, M_LSDISP_SLOT),
			MSGSTR(LSDISP, LSDISP_BUS, M_LSDISP_BUS),
			MSGSTR(LSDISP, LSDISP_ADPT_NAME, M_LSDISP_ADPT_NAME),
			MSGSTR(LSDISP, LSDISP_DESC, M_LSDISP_DESC) );

		/* print delimiter info */
		printf("%-10s%-6s%-5s%-11s%-47s\n", "========","====","===",
			"=========","===========");
	}

	/* loop through each display in the list by its order value */
	for ( x = 0; x < nbr_of_displays; x++ )
	{
		for ( tmp_ptr=displays; tmp_ptr != NULL; tmp_ptr=tmp_ptr->next) 
			/* if order matches count, then break */
			if ( tmp_ptr->order == x ) 
				break;
		
		/* print adapter info */
		printf("%-10s%-6s%-5s%-11s%-47s\n", tmp_ptr->logical_name,
				tmp_ptr->slot_nbr, tmp_ptr->bus_type,
				tmp_ptr->physical_name, tmp_ptr->description);
	}

	if ( lflag == FALSE )
	{
		/* Print out the default display at end of list */
		printf("\n   Default display = %s\n\n\n",default_disp);
	}

	return(SUCCESS);
}
