/* @(#)55	1.6  src/bos/usr/include/cmdnim_ip.h, cmdnim, bos411, 9428A410j  9/16/93  16:06:08 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/include/cmdnim_ip.h
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CMDNIM_IP
#define _H_CMDNIM_IP
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_NIM_PKT	1024		/* Maximum NIM packet	 	   */
#define NIM_DIR	      	"/var/adm/nim"

#define NIMREG_FMT	"%s %s %s %s %s %s %s"
#define NIMREG_PARMS	7

#define IS_REGISTRATION  1
#define NOT_REGISTRATION 0

typedef struct {
	int	useRes; 	/* Sez if we should use a reserved port	    */
	int	fd; 		/* The socket descriptor	 	    */
	FILE	*FP;		/* A file pointer to the socket		    */
struct	sockaddr_in addr; 	/* Socket address information		    */
}NIM_SOCKINFO;

#endif /* _H_CMDNIM_IP */
