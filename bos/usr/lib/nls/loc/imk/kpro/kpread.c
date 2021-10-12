static char sccsid[] = "@(#)88  1.2  src/bos/usr/lib/nls/loc/imk/kpro/kpread.c, libkr, bos411, 9428A410j 7/21/92 00:39:37";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		kpread.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM Profile
 *
 *  Module:       kpread.c
 *
 *  Description:  koreane Input Method profile retriever
 *
 *  Functions:    getprofile()
 *		  closeprofile() 
 *		  pushback()
 *		  nextchar()
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*       Include files
*-----------------------------------------------------------------------*/
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "kimpro.h"

/*-----------------------------------------------------------------------*
*     global variables 
*-----------------------------------------------------------------------*/
char *protbl;           /* beginning of profile data area          */
char *prooffset;        /* currect offset, at which nextchar reads */
int  datasize;          /* size of profile data                    */

/*-----------------------------------------------------------------------*
*     read profile data from file into memory 
*-----------------------------------------------------------------------*/
int getprofile(profile)
char *profile;
{
    int fd; /* file descriptor for KIM profile */
    struct stat sbuf; /* file status information */

    /* open specified KIM profile */

    if((fd = open(profile, O_RDONLY)) == -1) 
	return(NULL);

    /* read data from KIM profile by its size */
    if((fstat(fd, &sbuf) == -1) || 
	    ((protbl = (char *)malloc(sbuf.st_size)) == NULL))
	return(NULL);
    datasize = sbuf.st_size;
    read(fd, protbl, datasize);
    prooffset = protbl; /* set initial offset for subsequent read */

    /* close KIM profile */
    close(fd);

    /* parse and make word list */
    if(yyparse() == 1) {
	if(protbl)
	    free(protbl);
	destroylist();
	return(NULL);
    }
    else
	return(TRUE);
}

/*-----------------------------------------------------------------------*
*     free allocated memory so far
*-----------------------------------------------------------------------*/
void closeprofile()
{
    if(protbl)
	free(protbl);
    destroylist();
}

/*-----------------------------------------------------------------------*
*     give/push back  one character to/from lexical analyzer at a time
*     they are called only by yylex
*-----------------------------------------------------------------------*/
int nextchar()
{
    if((prooffset ) >= (protbl + datasize)) /* end of data ? */
	return(EOF);
    else
	return(*prooffset++);
}

void pushback()
{
    prooffset--;
}
 
