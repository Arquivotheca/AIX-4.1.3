static char sccsid[] = "@(#)78	1.1  src/bos/usr/lib/nls/loc/imk/khhc/Hhclstutil.c, libkr, bos411, 9428A410j 5/25/92 15:43:55";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		Hhclstutil.c
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
 *  Component:    Korean IM HHC  
 *
 *  Module:       Hhclstutil.c
 *
 *  Description:  The routines of the MRU list handling.
 *
 *  Functions:    alloc_node()
 *		  free_node()
 *		  list_init()
 *		  list_empty()
 *		  list_insert()
 *	  	  list_append()
 *		  list_delete()
 *
 *  History:      5/20/90  Initial Creation.     
 * 
 ******************************************************************/

/*------------------------------------------------------------------------------*/
/*                      Includes Header Files.                                  */
/*------------------------------------------------------------------------------*/

#include 	<sys/types.h>
#include	<stdio.h>
#include        <string.h>
#include        <im.h>
#include	"kedconst.h"	
#include	"Hhclist.h"
#include 	"Hhcim.h"

/*------------------------------------------------------------------------------*/
/*                      External reference                                      */
/*------------------------------------------------------------------------------*/

extern void     free();
extern caddr_t  malloc();

/*------------------------------------------------------------------------------*/
/*                      Begining of alloc_node()                                */
/* 	Purpose:	 Allocates a list node.     				*/
/*------------------------------------------------------------------------------*/
int	alloc_node(p_L, key, cand, status)
List	*p_L;
caddr_t	key;
caddr_t	cand;
ushort  status;
{
List	node;

	node = (List)malloc(sizeof(NODE));
	if (node == NULL) 
	{
		return (KP_ERR);
	}
	*p_L            = node;
	KEY(node)       = key;
	CAND(node)      = cand;
	M_STATUS(node)  = status;
	NEXT(node)      = NULL;
	return (KP_OK);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of free_node()                                 */
/* 	Purpose:	 Deallocates a list node.   				*/
/*------------------------------------------------------------------------------*/
int	free_node(p_L)
List	*p_L;
{
	free(KEY(*p_L));
	free(CAND(*p_L));
	free(*p_L);
	*p_L = NULL;
}	

/*------------------------------------------------------------------------------*/
/*                      Begining of list_init()                                 */
/* 	Purpose: 	 Initializes the list.      				*/
/*------------------------------------------------------------------------------*/
int	list_init(p_L)
List	*p_L;
{
	*p_L = NULL;
	return (KP_OK);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of list_empty()                                */
/* Purpose:		 Checks the list is empty.  				*/
/*------------------------------------------------------------------------------*/
int	list_empty(L)
List	L;
{
	return ((L == NULL) ? TRUE : FALSE);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of list_insert()                               */
/* 	Purpose:   Inserts a node at the front of the list. 			*/
/*------------------------------------------------------------------------------*/
int	list_insert(p_L, key, cand)
List	*p_L;
caddr_t	key;
caddr_t	cand;
{
List	node;
	
	if (alloc_node(&node, key, cand, UD_MRU_NEW) == KP_ERR)
	{
		return (KP_ERR);
	}
	NEXT(node) = *p_L;
	*p_L = node;
	return (KP_OK);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of list_delete()                               */
/* 	Purpose:	 Deletes a node of the list.       			*/
/*------------------------------------------------------------------------------*/
int	list_delete(p_L, node)
List	*p_L;
List	node;
{
	if (list_empty(*p_L) == TRUE) 
	{
		return (KP_ERR);
	}
	if (*p_L == node) 
	{
		*p_L = NEXT(*p_L);
	}
	else 
 	{
		register List 	L;
		for(L=*p_L; L != NULL && NEXT(L) != node; 
				L = NEXT(L))
				;
		if (L == NULL) 
		{
			return (KP_ERR);
		}
		else 
		{
			NEXT(L) = NEXT(node);
		}
	}
	free_node(&node);
	return (KP_OK);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of list_append()                               */
/* 	Purpose:	 Appends a node at the rear of the list.  		*/
/*------------------------------------------------------------------------------*/
int	list_append(p_L, key, cand)
List	*p_L;
caddr_t	key;
caddr_t	cand;
{
List	      node;
register List tmp;

	if (alloc_node(&node, key, cand, UD_MRU_NEW) == KP_ERR) 
	{
	  return (KP_ERR);
	}
	if (list_empty(*p_L) == TRUE)
	{
	  *p_L = node;
	}
	else 
	{
		for(tmp=*p_L; 
	 	  NEXT(tmp) != NULL; tmp=NEXT(tmp)) ;
		NEXT(tmp) = node;
	}
	return (KP_OK);
} 
