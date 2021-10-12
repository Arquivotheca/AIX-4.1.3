/* @(#)19	1.4  src/bos/diag/da/msla/dsladef.h, damsla, bos411, 9428A410j 12/10/92 09:07:59 */
/*
 *   COMPONENT_NAME: DAMSLA
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef         CATD_ERR                /* catalog error return code      */
#define         CATD_ERR        -1      /* catalog error return code for  */
#endif                                  /* the old NLS code used by RIOS  */

#define MSLA_POS_TEST     1 
#define MSLA_MEM_TEST     2
#define MSLA_REG_TEST     3
#define MSLA_HW_TEST      4 
#define MSLA_VPD_TEST     5
#define MSLA_WRAP_TEST    6 
#define MSLA_INTR_TEST    7 
#define MSLA_DMA_TEST     8 

#define PTR_POS_TEST       0       /* pointers to tmode array structure */
#define PTR_MEM_TEST       1       
#define PTR_REG_TEST       2      
#define PTR_HW_TEST        3  
#define PTR_WRAP_TEST      4 
#define PTR_INTR_TEST      5  
#define PTR_DMA_TEST       6 
#define PTR_VPD_TEST       7 

#define ERR_FILE_OPEN        -1
