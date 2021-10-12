/* @(#)68	1.3.1.4  src/bos/diag/da/iop/iop.h, daiop, bos411, 9428A410j 4/14/94 10:13:17 */
/*
 *   COMPONENT_NAME: DAIOP
 *
 *   FUNCTIONS:  header file 
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */
#ifndef _H_IOP 
#define _H_IOP

#ifndef NO_ERROR 
#define NO_ERROR                   0
#endif 

#ifndef MIONVCHCK  
#define MIONVCHCK                  15
#endif 
 
#ifndef TRUE
#define TRUE                       1
#define FALSE                      0
#endif

#ifndef AIX_ERROR  
#define AIX_ERROR                 -1
#endif 

#define ANSWERED_YES               1

/* TU error codes moved in ioptu.h in tu_iop component */
#include "ioptu.h"

#define TU_TEST_ORDER_ERROR       20

#define LOCKED_KEY_POSITION        0x05
#define SERVICE_KEY_POSITION       0x06
#define NORMAL_KEY_POSITION        0x07

#define KEY_MASK                   0x00000007 /* mask for the 28th bit     */
                                              /* required on some machines */
#define INVALID_TU_CALL          256 
#define INTERACTIVE_TEST_MODE      1
#define NO_VIDEO_TEST_MODE         2
#define TITLES_ONLY_TEST_MODE      3
#define INVALID_TM_INPUT          -1
#define A_LINE                    (80 * sizeof(" ") )

#endif

