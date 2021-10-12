static char sccsid[] = "@(#)24        1.16  src/bos/usr/lib/methods/udefif/udefif.c, cmdnet, bos411, 9428A410j 11/12/93 13:43:30";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, udefif, udefif_all, usage
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
  udefif -- removes an if instance from the config database.

  - removes if instance, and its associated attribute and connection
  information.
 */

#include "tcpcfg.h"

char progname[128];		/* = argv[0] at run time */

#include "udefif_msg.h"
#define MSG_SET MS_UDEFIF
#include "msg_hdr.h"
nl_catd catd;
void udefif(char *);
void usage(char *, char *, char *, char *);

/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(int argc, char **argv)
{
	char *cp, *ifname[69];
	int c, i, lflag = 0, phase = 0;

	setlocale(LC_ALL, "");
	catd = catopen(MF_UDEFIF, NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case 'l':
			ifname[lflag++] = optarg;
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

	if (lflag)
		for (i=0; i<lflag; i++)
			udefif(ifname[i]);
	else
		udefif_all();

	odm_terminate();
	return(0);
}


/*
 * NAME: udefif
 *                                                                    
 * FUNCTION: 
 * removes an interface and all its attributes from the
 * customized database.
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  nothing.
 */
void 
udefif(char *ifname)
{
	CLASS_SYMBOL cudv_hndl, cuat_hndl, cudep_hndl;
	struct CuDv *cudvp;
	struct CuDep *cudep;
	struct listinfo cudv_info, cudep_info;
	char crit[128];


	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-126 udefif: Cannot open CuDv.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	cuat_hndl = odm_open_class(CuAt_CLASS);
	if ((int) cuat_hndl == -1) {
		ERR(Msg(OPENCUAT, "0821-124 udefif: Cannot open CuAt.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	cudep_hndl = odm_open_class(CuDep_CLASS);
	if ((int) cudep_hndl == -1) {
		ERR(Msg(OPENCUDEP, "0821-125 udefif: Cannot open CuDep.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	/*
	 * first make sure that interface is not in stopped or available state
	 */

	strcpy(crit, "name = '");
	strcat(crit, ifname);
	strcat(crit, "'");

	DBGMSG((stderr, "undefining interface: %s\n", crit));
	cudvp = get_CuDv_list(CuDv_CLASS, crit, &cudv_info, 1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-121 udefif: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	if (cudv_info.num != 1) {
		ERR(Msg(GETIF,
	"0821-122 udefif: Expected one interface instance, retrieved %d.\n"),
		    cudv_info.num);
		CFGEXIT(E_ODMGET);
	}

	if (cudvp->status == STOPPED || cudvp->status == AVAILABLE) {
		ERR(Msg(INETSTAT,
			"0821-123 udefif: %s must not be in either the STOPPED or the AVAILABLE state.\n"),
		    ifname);
		CFGEXIT(E_DEVSTATE);
	}

	/*
	 * try to delete all of the customized attributes.
	 */

	DBGMSG((stderr, "Removing customized attributed for: %s", crit));
	if (odm_rm_obj(cuat_hndl, crit) == -1) {
		ERR(Msg(CUATDEL,
			"0821-118 udefif: Cannot delete from CuAt.\n"));
		CFGEXIT(E_ODMDELETE);
	}
	
	/*
	 * now remove the customized dependency record.
	 */

	DBGMSG((stderr, "Removing customized dependency for: %s", crit));
	if (odm_rm_obj(cudep_hndl, crit) == -1) {
		ERR(Msg(CUDEPDEL,
			"0821-119 udefif: Cannot delete from CuDep.\n"));
		CFGEXIT(E_ODMDELETE);
	}

	/*
	 * now try to remove the customized if record.
	 */

	DBGMSG((stderr, "Removing customized device record for: %s", crit));
	if (odm_rm_obj(cudv_hndl, crit) == -1) {
		ERR(Msg(CUDVDEL,
			"0821-120 udefif: Cannot delete from CuDv.\n"));
		CFGEXIT(E_ODMDELETE);
	}

	odm_close_class(cudv_hndl);
	odm_close_class(cudep_hndl);
	odm_close_class(cuat_hndl);
}


/*
 * NAME: udefif_all
 *                                                                    
 * FUNCTION: 
 *     undefines all the interfaces associated with current inet instance.
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  nothing.
 */
udefif_all()
{
	int i;
	struct CuDv *cudvp, *c;
	struct listinfo cudv_info;

	/*
	 * first we get a list of all interface types.
	 */
	 
	cudvp = get_CuDv_list(CuDv_CLASS, "parent = 'inet0'", &cudv_info,
			      1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-121 udefif: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}

	/*
	 * for each customized if
	 */
	for (i=0, c=cudvp; i<cudv_info.num; i++, c++) {
		/*
		 * we remove each customized interface.
		 */
		udefif(c->name);
	}
}


/*
 * NAME: usage
 *
 * FUNCTION: prints out the usage message when user makes a mistake on 
 *           the command line.
 *
 * RETURNS:  nothing, exits.
 */
void 
usage(char *a, char *b, char *c, char *d)
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr,
		Msg(USAGE1, "\nusage:\t%s  [-l interface ] ...\n"), progname);
	CFGEXIT(E_ARGS);
}
