static char sccsid[] = "@(#)26        1.15  src/bos/usr/lib/methods/udefinet/udefinet.c, cmdnet, bos411, 9428A410j 11/12/93 13:43:34";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, udefinet, usage
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
  udefinet -- undefines inet instance in config database.

  - if inet instance is not in stopped or available state, exits with error.
  - removes objects associated with the inet instance, including
  the corresponding connection and attribute objects.
 */

#include "tcpcfg.h"

char progname[128];		/* = argv[0] at run time */

#include "udefinet_msg.h"
#define MSG_SET MS_UDEFINET
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
	int i, phase = 0, c;
	char *cp;

	setlocale(LC_ALL, "");
	catd = catopen(MF_UDEFINET, NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case '2':
			phase = 2;
			break;
		case 'h':
		case '?':
			usage(0,0,0,0);
		}
	}

	/* set odm path, initialize and lock the config database. */
	cfg_init_db(phase);

	udefinet();

	odm_terminate();
	return(0);
}


/*
 * NAME: udefinet
 *                                                                    
 * FUNCTION: 
 * removes the inet the records customized database.
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  nothing.
 */
udefinet()
{
	CLASS_SYMBOL cudv_hndl, cuat_hndl, cudp_hndl;
	struct CuDv *cudvp;
	struct listinfo cudv_info;

	/*
	 * make sure inet instance is stopped and unconfigured.
	 */

	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-142 udefinet: Cannot open CuDv.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	cudvp = get_CuDv_list(CuDv_CLASS, "name = 'inet0'", &cudv_info,
			      1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-137 udefinet: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	/* check to see if out job is already done. */
	if (cudv_info.num != 0) {
		
		if (cudv_info.num > 1) {
			ERR(Msg(GETINET,
	"0821-138 udefinet: Expected one inet0 instance, retrieved %d.\n"),
			    cudv_info.num);
			CFGEXIT(E_ODMGET);
		}
		
		if (cudvp->status == STOPPED || cudvp->status == AVAILABLE) {
			ERR(Msg(INETSTAT,
				"0821-139 udefinet: The inet0 must not be in the STOPPED or AVAILABLE state.\n"));
			CFGEXIT(E_DEVSTATE);
		}
	}

	/*
	 * try to delete all of the customized attributes.
	 */

	cuat_hndl = odm_open_class(CuAt_CLASS);
	if ((int) cuat_hndl == -1) {
		ERR(Msg(OPENCUAT, "0821-141 udefinet: Cannot open CuAt.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	if (odm_rm_obj(cuat_hndl, "name = 'inet0'") == -1) {
		ERR(Msg(CUATDEL,
			"0821-133 udefinet: Cannot delete from CuAt.\n"));
		CFGEXIT(E_ODMDELETE);
	}

	odm_close_class(cuat_hndl);

	/*
	 * now try to delete all of the customized connection info.
	 */

	cudp_hndl = odm_open_class(CuDep_CLASS);
	if ((int) cudp_hndl == -1) {
		ERR(Msg(OPENCUDP, "0821-140 udefinet: Cannot open CuDep.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	if (odm_rm_obj(cudp_hndl, "name = 'inet0'") == -1) {
		ERR(Msg(CUDEPDEL,
			"0821-134 udefinet: Cannot delete from CuDep.\n"));
		CFGEXIT(E_ODMDELETE);
	}

	odm_close_class(cudp_hndl);

	/*
	 * now try to remove the customized inet record.
	 */
	
	if (cudv_info.num > 0) {
		if (odm_rm_obj(cudv_hndl, "name = 'inet0'") == -1) {
			ERR(Msg(CUDVDEL,
		"0821-135 udefinet: Cannot delete inet0 from CuDv.\n"));
			CFGEXIT(E_ODMDELETE);
		}
	}

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
void 
usage(char *a, char *b, char *c, char *d)
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr, Msg(USAGE1, "\nusage:\t%s \n"), progname);
	CFGEXIT(E_ARGS);
}

