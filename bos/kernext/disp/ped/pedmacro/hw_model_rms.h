/* @(#)18	1.3  src/bos/kernext/disp/ped/pedmacro/hw_model_rms.h, pedmacro, bos411, 9428A410j 3/17/93 19:44:45 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/*                                                                      */
/*      PEDERNALES HARDWARE PROGRAMMING INTERFACE                       */
/*                                                                      */
/************************************************************************/

#ifndef _H_MID_HW_MODEL
#define _H_MID_HW_MODEL


/****************************************************************

THE FOLLOWING DEFINE TERM, FOR "MID_DATA_PTR", IS ONE OF ONLY
TWO THINGS THAT ACTUALLY HAS TO BE EDITED BY THE GAI MODEL WRITER

*****************************************************************/


/*
 * Defined within each module that needs to use it.
 */
/* #define MID_DATA_PTR    /* my_models_malloc_ptr_for_MID_model_data */ /**/


/****************************************************************

THE FOLLOWING DEFINE TERM, FOR "MID_BASE_PTR", IS ONE OF ONLY
TWO THINGS THAT ACTUALLY HAS TO BE EDITED BY THE GAI MODEL WRITER

*****************************************************************/


/*
 * Defined within each module that needs to use it.
 */
/* #define MID_BASE_PTR    /*((C statement to trace path to adapter address ))*/ /**/

/********************************************************************
IF THE TWO DEFINES ABOVE ARE CORRECT, EVERYTHING BELOW HERE WILL
GENERALLY WORK.
    1.  THEY ARE INCLUDED IN CASE THE MODEL WISHES TO ADD TO THEM
    2.  THEY ARE INCLUDED TO ALLOW THE MODEL TO RE-DEFINE MALLOC
*********************************************************************/


#define MID_PRODUCTION_HW       TRUE    /* this code built for production
                                           hardware, not prototype */


#define	MID_DEFER_CAPACITY	512


typedef struct  _MID_model_data
{
	ulong		errno;		/* error number of last error */
                /*-----------------------------
                  things defining the defer buf
                  -----------------------------*/
        ulong *         defer_buf;      /* pointer to the deferral buffer */
        ulong *         next_defer_wd;  /* used while adding to the buffer */
        ulong           defer_buf_len;  /* how much is in it?           */
}  MID_model_data,
   *pMID_model_data;


#define	MID_ERRNO		(MID_DATA_PTR->errno)

#define MID_DEFERBUF_HD         (MID_DATA_PTR->defer_buf)

#define MID_DEFERBUF            (MID_DATA_PTR->next_defer_wd)

#define MID_DEFERBUF_LEN        (MID_DATA_PTR->defer_buf_len)


#define	MID_SET_ERRNO( errval )						\
	{								\
	MID_ERRNO = ( ulong ) errval;					\
	}


#define MID_INIT                                                        \
        {                                                               \
        MID_DATA_PTR_FOR_MALLOC =					\
		(pMID_model_data)malloc(sizeof( MID_model_data ));	\
	if ( MID_DATA_PTR == NULL )					\
		{							\
		MID_SET_ERRNO ( MID_ERR_BAD_MALLOC );			\
		}							\
        MID_DEFERBUF_HD = (ulong*)malloc( MID_DEFER_CAPACITY );		\
	if ( MID_DEFERBUF_HD == NULL )					\
		{							\
		MID_SET_ERRNO ( MID_ERR_BAD_MALLOC );			\
		}							\
        MID_DEFERBUF = MID_DEFERBUF_HD;                                 \
        MID_DEFERBUF_LEN = 0;                                           \
        }


#define MID_DESTROY                                                     \
        {                                                               \
        free( MID_DEFERBUF_HD );                                        \
        free( MID_DATA_PTR );                                           \
        }


#endif /* _H_MID_HW_MODEL */
