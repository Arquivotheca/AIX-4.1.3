/* @(#)39	1.1  src/bos/usr/lib/nls/loc/imk/include/Hhclist.h, libkr, bos411, 9428A410j 5/25/92 15:36:08 */
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		Hhclist.h
 *
 * ORIGINS :		27
 *
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
 *  Module:       Hhclist.h
 *
 *  Description:  Korean Im Hhc Mru List Header.
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/

#ifndef	_HHCLIST_H_
#define	_HHCLIST_H_
/***********************/
/* file: _Hhclist.h    */
/***********************/
#include <sys/types.h>

typedef	struct node NODE, *List;
struct node {
	ushort  status;
	ushort  cand_src;
	caddr_t	keyptr;
	caddr_t	candptr;
	List	next;
};

/**********************************************/
/* A key and candidate is terminated by NULL. */
/**********************************************/
#define	KEY(L)		((L)->keyptr)
#define	CAND(L)		((L)->candptr)
#define NEXT(L)		((L)->next)
#define M_STATUS(L)	((L)->status)
#define M_SOURCE(L)	((L)->cand_src)

/***********************/
/* file: _Hhclist.h    */
/***********************/
#endif
