static char sccsid[] = "@(#)32	1.9  src/bos/usr/ccs/lib/libc/yperr_str.c, libcyp, bos411, 9428A410j 1/19/94 10:15:18";
/* 
 * COMPONENT_NAME: (LIBCYP) NIS Library
 * 
 * FUNCTIONS: yperr_string 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.8 87/08/12
 */

#include <rpcsvc/ypclnt.h>
#include <locale.h>
#include <limits.h>
#include <nl_types.h>
#include "libc_msg.h"


/*
 * This returns a pointer to an error message string appropriate to an input
 * yp error code.  An input value of zero will return a success message.
 * In all cases, the message string will start with a lower case chararacter,
 * and will be terminated neither by a period (".") nor a newline.
 */
char *
yperr_string(code)
	int code;
{
	char *pmesg;
	nl_catd   catd;
	static char buffer[NL_TEXTMAX];
	
	catd = catopen(MF_LIBC,NL_CAT_LOCALE);

	switch (code) {

	case 0:  {
		pmesg = catgets(catd,LIBCYP,CYP2,"yp operation succeeded");
		break;
	}
		
	case YPERR_BADARGS:  {
		pmesg = catgets(catd,LIBCYP,CYP3,"args to yp function are bad");
		break;
	}
	
	case YPERR_RPC:  {
		pmesg = catgets(catd,LIBCYP,CYP4,"RPC failure on yp operation");
		break;
	}
	
	case YPERR_DOMAIN:  {
		pmesg = catgets(catd,LIBCYP,CYP5,
				"can't bind to a server which serves domain");
		break;
	}
	
	case YPERR_MAP:  {
		pmesg = catgets(catd,LIBCYP,CYP6,
				 	"no such map in server's domain");
		break;
	}
		
	case YPERR_KEY:  {
		pmesg = catgets(catd,LIBCYP,CYP7,"no such key in map");
		break;
	}
	
	case YPERR_YPERR:  {
		pmesg = catgets(catd,LIBCYP,CYP8,
				"internal yp server or client error");
		break;
	}
	
	case YPERR_RESRC:  {
		pmesg = catgets(catd,LIBCYP,CYP9,
				"local resource allocation failure");
		break;
	}
	
	case YPERR_NOMORE:  {
		pmesg = catgets(catd,LIBCYP,CYP10,
				"no more records in map database");
		break;
	}
	
	case YPERR_PMAP:  {
		pmesg = catgets(catd,LIBCYP,CYP11,
				"can't communicate with portmapper");
		break;
		}
		
	case YPERR_YPBIND:  {
		pmesg = catgets(catd,LIBCYP,CYP12,
				"can't communicate with ypbind");
		break;
		}
		
	case YPERR_YPSERV:  {
		pmesg = catgets(catd,LIBCYP,CYP13,
				"can't communicate with ypserv");
		break;
		}
		
	case YPERR_NODOM:  {
		pmesg = catgets(catd,LIBCYP,CYP14,
				"local domain name not set");
		break;
	}

	case YPERR_BADDB:  {
		pmesg = catgets(catd,LIBCYP,CYP15,
				"yp map data base is bad");
		break;
	}

	case YPERR_VERS:  {
		pmesg = catgets(catd,LIBCYP,CYP16,
				"yp client/server version mismatch");
		break;
	}

	case YPERR_ACCESS: {
		pmesg = catgets(catd,LIBCYP,CYP17,"permission denied");
		break;
	}

	case YPERR_BUSY: {
		pmesg = catgets(catd,LIBCYP,CYP18,"database is busy");
		break;
	}
		
	default:  {
		pmesg = catgets(catd,LIBCYP,CYP19,
				"unknown yp client error code");
		break;
	}
	
	}
	strcpy(buffer,pmesg);
        catclose(catd);
	return(buffer);
}
