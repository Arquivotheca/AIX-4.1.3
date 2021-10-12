static char sccsid[] = "@(#)34	1.13  src/bos/usr/ccs/lib/librts/errlog.c, cmderrlg, bos411, 9428A410j 10/21/93 08:35:05";

/*
 * COMPONENT_NAME: CMDERRLG/LIBRTS   system error logging facility
 *
 * FUNCTIONS: errlog   application-level error logging routine
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/err_rec.h>

/*
 * NAME:
 *  errlog()
 * FUNCTION:
 *	Attempt to log the incoming data to the system error device. Do not put suspect 
 *  data in /dev/error.
 *
 *	The resource_name must neither be empty nor contain nonprintable characters.
 *
 *	The length must be at least the ERR_REC_SIZE bytes and not greater than
 *	ERR_REC_MAX_SIZE bytes. ERR_REC_MAX_SIZE is the size of err_rec0 plus ERR_REC_MAX
 *	the maximum length of detail data.
 * INPUTS:
 *  A pointer to the ErrorStructure containing the error data.  The buffer must be 
 *  	of type err_rec or err_rec0.
 *  The length of the data in the ErrorStructure.
 *
 * RETURNS:
 *	Upon successful completion, the errlog subroutine returns a value of 0.
 *	If errlog fails, a value of -1 is returned and the errno global variable
 *	is set to indicate the error.
 *
 *	Error Codes
 *	The errlog subroutine fails if any of the following are true:
 *	EFAULT	The ErrorStructure parameter points outside of the process' address space.
 *	EINVAL	The Length parameter is less than the size of struct err_rec0, or
 *			is greater than the maximum size allowed, ERR_REC_MAX_SIZE.
 *	EINVAL	The provided resource_name is empty, or contains nonprintable characters,
 *			other than the string terminating null.
 *	EAGAIN	Error device temporarily unavailable.
 */


errlog(void *buf,int len)
/*  len is number of bytes (not chars!!) in buf.  
    It can contain ascii, ebcidec, embedded nulls, multi-byte data, etc... */
{
	int		fd = -1, rv = 0;

	/* Maximum size of buf allowed is 20 bytes of data + 230 bytes
	   of detail data - 1 */

	if(buf == NULL) {
		rv = -1;
		errno = EFAULT;
	}
	else if(len < ERR_REC_SIZE ||
			len > ERR_REC_MAX_SIZE ||
			not_printable((char*)buf+sizeof(unsigned)))  {
		rv = -1;
		errno = EINVAL;
	}
	else if((fd = open("/dev/error",O_WRONLY)) < 0)
		rv = -1;	/* errno set by open */
	else if((rv=write(fd,buf,len)) != len) {
		if(rv >= 0) {		/* partial write */
			rv = -1;
			errno = EAGAIN;
		}
	}
	
	if(fd != -1)	/* close dev/error if we opened it successfully */
		if((rv=close(fd)) < 0)	/* errno set by close */
			rv = -1;

	return(rv);
}

/*
 *	NAME:   not_printable()
 *	FUNCTION:
 *		Check that a given character array contains
 *		only printable ascii characters, and is not
 *		empty.
 *	RETURNS:
 *		0 - only good chars found.
 *		1 - bad chars found, or empty.
 */
static
int not_printable(char ary[])
{
	int i;
	int	rc;


	rc = 0;

	for (i=0; i< ERR_NAMESIZE; i++) {
		if(!isprint(ary[i])) {		/* not printable... */
			if(ary[i] == '\0') {
				if(i > 0) {		/* good char already encountered */
					break;
				}
				else {		/* 'empty' name */
					rc = 1;
					break;
				}
			}
			else {
				rc = 1;
				break;
			}
		}
	}
	return(rc);
}
