/* @(#)92	1.14  src/bos/usr/include/IN/backend.h, libIN, bos411, 9428A410j 3/22/93 11:34:03 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_BACKEND
#define _H_BACKEND

/* values for status */
#define READY   1	/* virtual device is up */
#define RUNNING 2	/* virtual device is running a job */
#define WAITING 3	/* virtual device is could not open and is waiting on 
				a device */
#define OFF     4	/* virtual device is down */
#define OPRWAIT 5	/* virtual device is waiting on operator message 
				response */
#define INIT	6	/* virtual device is running a job which has set status
				to initialize */
#define SENDING	7	/* virtual device is remote and in the process of 
				sending data to the foriegn server */
#define GETHOST	8	/* virtual device is remote and in the process of 
				determining the foriegn server */
#define CONNECT	9	/* virtual device is remote and in the process of 
				connecting to the foriegn server */
#define BUSY	10	/* virtual device is busy printing another job for
				another queue */
#define STATMAX	10	/* for checking boundary conditions of statii */

/* values for header and trailer */
#define NEVER 0
#define ALWAYS 1
#define GROUP 2
#define USEDEF 3


/* sysnot constants for libqb.a */
#define	DOWRITE	1 	/* use write for sending messages and if it fails, use	
				mail */
#define	DOMAIL	2	/* use mail exclusively */

#endif /* _H_BACKEND */
