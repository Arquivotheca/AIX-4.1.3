static char sccsid[] = "@(#)78	1.6  src/bos/usr/bin/uucp/altconn.c, cmduucp, bos411, 9428A410j 6/17/93 13:52:12";
/* 
 * COMPONENT_NAME: CMDUUCP altconn.c
 * 
 * FUNCTIONS: altconn 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* static char sccsid[] = "altconn.c	5.1 -  - "; */
/*	altconn.c	1.8
*/
#include "uucp.h"
extern nl_catd catd;


static struct call {		/* cu calling info-also in cu.c; */
				/* make changes in both places!*/
	char *speed;		/* transmission baud rate */
	char *line;		/* device name for outgoing line */
	char *telno;		/* ptr to tel-no digit string */
	char *class;		/* class of call */
};

/*

 * altconn - place a telephone call to system 
 * from cu when telephone number or direct line used
 *
 * return codes:
 *	FAIL - connection failed
 *	>0  - file no.  -  connect ok
 * When a failure occurs, Uerror is set.
 */

altconn(call)
struct call *call;
{
	int nf, fn = FAIL, i;
	char *alt[7];
	extern char *Myline;

	alt[F_NAME] = "dummy";	/* to replace the Systems file fields  */
	alt[F_TIME] = "Any";	/* needed for getto(); [F_TYPE] and    */
	alt[F_TYPE] = "";	/* [F_PHONE] assignment below          */
	alt[F_CLASS] = call->speed;
	alt[F_PHONE] = "";
	alt[F_LOGIN] = "";
	alt[6] = NULL;

	CDEBUG(4,MSGSTR(MSG_ALTC_CD1,"altconn called\r\n"),"");

        /* cu -l dev ...                                        */
        /* if is "/dev/device", strip off "/dev/" because must  */
        /* exactly match entries in Devices file, which usually */
        /* omit the "/dev/".  if doesn't begin with "/dev/",    */
        /* either they've omitted the "/dev/" or it's a non-    */
        /* standard path name.  in either case, leave it as is  */

        if(call->line != NULL ) {
                if ( strncmp(call->line, "/dev/", 5) == 0 ) {
                        Myline = (call->line + 5);
                } else {
                        Myline = call->line;
                }
        }

	/* cu  ... telno */
	if(call->telno != NULL) {
		alt[F_PHONE] = call->telno;
		alt[F_TYPE] = "ACU";
	}
	/* cu direct line */
	else {
		alt[F_TYPE] = "Direct";
	}

#ifdef forfutureuse
	if (call->class != NULL)
		alt[F_TYPE] = call->class;
#endif


	fn = getto(alt);
	CDEBUG(4, MSGSTR(MSG_ALTC_CD2,"getto ret %d\n"), fn);

	return(fn);

}

