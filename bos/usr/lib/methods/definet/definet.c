static char sccsid[] = "@(#)08        1.19  src/bos/usr/lib/methods/definet/definet.c, cmdnet, bos411, 9428A410j 11/12/93 13:43:03";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, definet, usage
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
  definet -- defines the inet instance in the config database.

  - if customized inet instance doesn't exist already, definet
    takes predefined inet object and creates a customized record.
  - for each predefined attribute, a customized attribute is defined, 
    if not already defined.
  - status flag for inet instance set to DEFINED.
 */

#include "tcpcfg.h"

char progname[128];		/* = argv[0] at run time */

#include "definet_msg.h"
#define MSG_SET MS_DEFINET
#include "msg_hdr.h"
nl_catd catd;

void usage(char *, char *, char *, char *);
/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(int argc, char **argv)
{
	char *cp;
	int c, phase = 0;

	setlocale(LC_ALL, "");
	catd = catopen(MF_DEFINET, NL_CAT_LOCALE);
	
	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);
	
	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case 'c':
			if (strcmp(optarg, "tcpip") != 0) {
				ERR(Msg(BADCLASS,
		"0821-039 definet: The class specified must be tcpip.\n"));
				exit(1);
			}
			break;
		case '2':
			phase = 2;
			break;
		case 'h':
		case '?':
			usage(0,0,0,0);
		}
	}
	
	/* set odm path, initialize and lock the config database. */
	cfg_init_db();

	definet();

	odm_terminate();
	return(0);
}


/*
 * NAME: definet
 *                                                                    
 * FUNCTION: defines the inet instance in the config database.
 *                                                                    
 * NOTES:
 *
 *
 * RETURNS:  nothing.
 */
definet()
{
	CLASS_SYMBOL cudv_hndl;
	struct PdDv *pddvp;
	struct CuDv *inetp, inet;
	struct PdAt *pdatp;
	struct listinfo cudv_info, pddv_info;

	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-044 definet: Cannot open CuDv.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	/*
	 * first check to see if customized inet instance already exists.
	 */

	inetp = get_CuDv_list(CuDv_CLASS, "name = 'inet0'", &cudv_info,
			      1, 1);
	if ((int) inetp == -1) {
		ERR(Msg(GETCUDV,
			"0821-041 definet: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	if (cudv_info.num > 0) {
		printf("%s\n", inetp->name);
		CFGEXIT(0);
	}
	
	/*
	 * create customized inet instance from predefined definition.
	 * we read the predefined inet record.  derive the inet instance
	 * name, and drop the customized record in CuDv.
	 */
	
	pddvp = get_PdDv_list(PdDv_CLASS, "type = 'inet'", &pddv_info, 1, 1);
	if ((int) pddvp == -1) {
		ERR(Msg(GETPDDV,
			"0821-042 definet: Cannot get records from PdDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	memset(&inet, 0, sizeof(inet));
	strcpy(inet.name, pddvp->prefix);
	strcat(inet.name, "0");
	inet.status = DEFINED;
	inet.chgstatus = 1;
	*(inet.ddins) = 0;
	*(inet.location) = 0;
	*(inet.parent) = 0;
	*(inet.connwhere) = 0;
	inet.PdDvLn = pddvp;
	inet.PdDvLn_info = &pddv_info;
	strcpy(inet.PdDvLn_Lvalue, pddvp->uniquetype);
	
	/*
	 * store the new record.
	 */
	
	DBGMSG((stderr, "adding inet record"));
	if (odm_add_obj(cudv_hndl, &inet) == -1) {
		ERR(Msg(BADADD,
		"0821-038 definet: Cannot add a customized inet record.\n"));
		CFGEXIT(E_ODMADD);
	}

	/*
	 * now we write the name of the defined device to stdout.
	 * so cfgmgr and hi-level commands work.
	 */
	printf("%s\n", inet.name);


	/*
	 * Close up shop.
	 */

	odm_close_class(cudv_hndl);
}


/*
 * NAME: usage
 *
 * FUNCTION: prints out the usage message when user makes a mistake on 
 *           the command line.
 *
 * RETURNS:  nothing, exits.
 */
void usage(char *a, char *b, char *c, char *d)
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr,
		Msg(USAGE1, "\nusage:\t%s [-c class]\"\n"),
		progname);
	fprintf(stderr,
		Msg(USAGE2, "\t%s -h\n"),
		progname);
	odm_terminate();
	CFGEXIT(E_ARGS);
}


