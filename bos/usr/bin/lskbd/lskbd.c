static char sccsid[] = "@(#)52	1.17  src/bos/usr/bin/lskbd/lskbd.c, cmdlft, bos411, 9428A410j 4/16/94 13:55:39";

/*
 * COMPONENT_NAME: (CMDLFT) Low Function Terminal Commands
 *
 * FUNCTIONS:  main
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
  Lists the current software keyboard map available to the system

  FUNCTIONS:
  list_swkbs() - display the currently available software keyboards
  parse(argv,argc) - parse the users flags and parameters
  ------------*/

#include <locale.h>
#include <fcntl.h>
#include "lftcmds.h"
#include "lftcmds_msg.h"
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>


#define SUCCESS 0
#define FAIL -1

/*------------
  This command lists the software keyboard map currently loaded into the kernel
  ------------*/
main(argc, argv)
int argc;
char *argv[];
{
	int x;			/* temporary variable */
	struct CuAt *cuat;	/* storage to get customized attribute */

	/* set the locale */
	setlocale(LC_ALL,"");

	/* Open the message facilities catalog.  Retrieve the program name. */
	catd=catopen("lftcmds.cat", NL_CAT_LOCALE);
	strcpy(program, MSGSTR(LSKBD, LSKBD_PGM, M_LSKBD_PGM));

	/* verify there are no arguments */
	if (argc != 1)
	{
		fprintf(stderr,MSGSTR(COMMON, BADFLAG, M_BADFLAG),
				program, argv[1]);
		fprintf(stderr,MSGSTR(LSKBD, LSKBD_USAGE, M_LSKBD_USAGE));
		exit(FAIL);
	}

	/* Initialize the ODM database for access */
	if( odm_initialize() < 0 )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_INIT, M_ODM_INIT), program);
		exit(FAIL);
	}

	/* query the ODM for the software keyboard map */
	if( (cuat = getattr("lft0","swkb_path",FALSE, &x)) == NULL)
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ, M_ODM_NOOBJ),
				program, "getattr()", "PdAt", "swkb_path");
		odm_terminate();
		exit(FAIL);
	}

	if( cuat == -1 )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_FAIL, M_ODM_FAIL),
				program, "getattr()", "PdAt", "swkb_path");
		odm_terminate();
		exit(FAIL);
	}

	/* terminate the ODM */
	odm_terminate(); 

	/* list the software keyboard pathname */
	printf(MSGSTR(LSKBD, LSKBD_LIST, M_LSKBD_LIST), cuat->value);

	/* Close the message facilities catalog */
	catclose(catd);

	exit(SUCCESS);

}	/* end main */
