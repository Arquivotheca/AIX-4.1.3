/* @(#)04	1.1  src/bos/usr/include/lib_data.h, libcthrd, bos411, 9428A410j 10/20/93 14:13:56 */
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: LIB_DATA_FUNCTION
 *		lib_data_hdl
 *		lib_data_ref
 *
 *   ORIGINS: 27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

/* lib_data.h,v $ $Revision: 1.2 $ (OSF) */

/*
 * Library macros/ types for "thread" data access
 */

typedef struct lib_data_functions {
	int	(*data_hdl)(void **);
	void	*(*data_ref)(void *);
} lib_data_functions_t;

#define	LIB_DATA_FUNCTION(datastruct, operation, arg) \
	((datastruct).operation ? (*(datastruct).operation)(arg) : 0)

#define	lib_data_hdl(datastruct, hdl) \
	LIB_DATA_FUNCTION(datastruct, data_hdl, hdl)

#define	lib_data_ref(datastruct, hdl) \
	LIB_DATA_FUNCTION(datastruct, data_ref, hdl)

