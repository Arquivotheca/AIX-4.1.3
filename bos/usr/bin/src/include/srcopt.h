/* @(#)12	1.4  src/bos/usr/bin/src/include/srcopt.h, cmdsrc, bos411, 9428A410j 2/26/91 15:07:16 */
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifndef _H_SRCOPT
#define _H_SRCOPT

#define VIEWFIELD	1
#define NOVIEWFIELD	0
#define FLAGFIELD	'F'
#define FLAGLONG	'L'
#define FLAGSHORT	'S'

#define PRINTDEFAULTSUBSYSTEM	1	
#define PRINTSUBSYSTEM	2
#define PRINTSUBSERVER	3
#define PRINTNOTIFY	4

struct argview
{
	long size;	/* object element name of the input field */
	char *bufaddr;	/* address to place input data(not offset) */
	char type;	/* odm datatype of input element */
	char flag;	/* input flag for data element */
	char newval; 	/* indicator for entry of element
			** initialy set to zero value will be incremented
			** each time the field appears on the command line
			*/
	char view;	/* will this field be in the objview 
			** VIEWFIELD - will be part of update/add view
			** NOVIEWFIELD - will not be part of update/add view
			*/
	long min;	/* minimum value that an integer may have */
	long max;	/* maximum value that an integer may have or
			** the maximum length that a character string
			** can have
			**/
	int errno;	/* SRC error to be printed should the max or min test
			** fail for a field
			*/
};

#endif
