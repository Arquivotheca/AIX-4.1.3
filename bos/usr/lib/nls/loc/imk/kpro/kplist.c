static char sccsid[] = "@(#)87	1.1  src/bos/usr/lib/nls/loc/imk/kpro/kplist.c, libkr, bos411, 9428A410j 5/25/92 15:45:33";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kplist.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992
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
 *  Module:       kplist.c
 *
 *  Description:  korean Input Method profile information keeper
 *
 *  Functions:    makeword()
 *		  destroylist()
 *		  getoptvalue()
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/
#include <stdio.h>
#include "kimpro.h"

/*-----------------------------------------------------------------------*
*	variable declaration
*-----------------------------------------------------------------------*/
typedef struct element {
    struct element *next;
    char *leftword;
    char *rightword;
    } list;

static list *firstlist = NULL;
static list *endlist = NULL;

/*-----------------------------------------------------------------------*
*       store pair of words into list
*       called by yyparse on correct syntax
*-----------------------------------------------------------------------*/
int makeword(s1, s2)
char *s1;
char *s2;
{
    list *temp;

    if((temp = (list *)malloc(sizeof(list))) == NULL)
	return(KP_MAKEERR);

    if(firstlist == NULL) {
	firstlist = temp;
	endlist = temp;
    }
    else {
	endlist->next = temp;
	endlist = endlist->next;
    }
    endlist->leftword = s1;
    endlist->rightword = s2;
    endlist->next = NULL;
    return(KP_OK);
}
	
/*-----------------------------------------------------------------------*
*       destroy list and words stored
*-----------------------------------------------------------------------*/
void destroylist()
{
    list *nextptr, *listptr;

    if((nextptr = firstlist) == NULL)
	return;
    do {
	listptr = nextptr;
	free(listptr->leftword);
	free(listptr->rightword);
	nextptr = listptr->next;
	free(listptr);
    } while(nextptr);
    firstlist = NULL;
    endlist = NULL;
}

/*-----------------------------------------------------------------------*
*       search word tree for specified target string
*       this does not make semantics checking, which is
*       up to the caller
*-----------------------------------------------------------------------*/
char *getoptvalue(target)
char *target; /* target object, must be null terminated */
{
    list *listptr = firstlist;

    while(listptr != NULL) { /* simple linear search */
	if(strcmp(listptr->leftword, target)) {
	    listptr = listptr->next;
	    continue;
	}
	else /* target is found */
	    return(listptr->rightword);
    }
    return(NULL); /* target is not found */

} /* end of findword */

#ifdef DEBUG
void printlist()
{
    list *listptr = firstlist;

    while(listptr != NULL) {
	fprintf(stderr, "leftword = %s\n", listptr->leftword);
	fprintf(stderr, "rightword = %s\n", listptr->rightword);
	listptr = listptr->next;
    }
}
#endif DEBUG
