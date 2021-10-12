static char sccsid[] = "@(#)33	1.4  src/bos/usr/ccs/lib/libc/ypprot_err.c, libcyp, bos411, 9428A410j 6/16/90 02:44:14";
/* 
 * COMPONENT_NAME: (LIBCYP) Yellow Pages Library
 * 
 * FUNCTIONS: ypprot_err 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.8 87/08/12 
 * 
 */


/* #if defined(LIBC_SCCS) && !defined(lint)
 * static char sccsid[] = 	"(#)ypprot_err.c	1.2 88/07/27 4.0NFSSRC Copyr 1988 Sun Micro";
 * #endif
 */



#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

/*
 * Maps a yp protocol error code (as defined in
 * yp_prot.h) to a yp client interface error code (as defined in
 * ypclnt.h).
 */
int
ypprot_err(yp_protocol_error)
	unsigned int yp_protocol_error;
{
	int reason;

	switch (yp_protocol_error) {
	case YP_TRUE: 
		reason = 0;
		break;
 	case YP_NOMORE: 
		reason = YPERR_NOMORE;
		break;
 	case YP_NOMAP: 
		reason = YPERR_MAP;
		break;
 	case YP_NODOM: 
		reason = YPERR_DOMAIN;
		break;
 	case YP_NOKEY: 
		reason = YPERR_KEY;
		break;
 	case YP_BADARGS:
		reason = YPERR_BADARGS;
		break;
 	case YP_BADDB:
		reason = YPERR_BADDB;
		break;
 	case YP_VERS:
		reason = YPERR_VERS;
		break;
	default:
		reason = YPERR_YPERR;
		break;
	}
	
  	return(reason);
}
