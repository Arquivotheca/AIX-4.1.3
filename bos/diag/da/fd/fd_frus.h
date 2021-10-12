/* @(#)76       1.4  src/bos/diag/da/fd/fd_frus.h, dafd, bos411, 9428A410j 12/17/92 10:58:07 */
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef  _H_FD_FRUS
#define  _H_FD_FRUS  


/************************************************************************/
/*									*/
/* fru_bucket structure holds related information to the FRU bucket.	*/
/* Variables in this structure are explained in RIOS Diagnostic		*/
/* Subsystem CAS under the function name addfrub().			*/
/*									*/
/************************************************************************/


struct fru_bucket frub[] = {
	{ "" , FRUB1, 0x0 , 0x0, 0 , 
	{
	{  0 , "" , "", 0 , DA_NAME      , EXEMPT },
	},
	},
	{ "" , FRUB1, 0x0 , 0x0, 0,  
	{
	{ 0 , ""  , "" , 0  ,DA_NAME      , EXEMPT  },
	{ 0 , ""  , "" , 0  ,PARENT_NAME  , EXEMPT  },
	},	
	},
	{ "" , FRUB1, 0x0 , 0x0, 0,	
	{
        { 0 , "" , "" , 0  ,DA_NAME        , EXEMPT    },
	{ 0 , "" , "" , 0  ,PARENT_NAME    , EXEMPT    },
	{ 0 , "" , "" , 0  ,0              , NONEXEMPT },
	},
	},
	};


#endif 



