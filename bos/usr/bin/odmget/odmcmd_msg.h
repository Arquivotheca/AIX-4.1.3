/* @(#)06       1.5  src/bos/usr/bin/odmget/odmcmd_msg.h, cmdodm, bos411, 9428A410j 2/19/93 12:25:04 */
/*
 * COMPONENT_NAME: CMDODM Object Data Manager commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. date 1, date 2
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <limits.h>
#include <nl_types.h>
#define MF_CMD "odmcmd.cat"



/* The following was generated from cmd.msg. */


/* definitions for set MS_odmadd */
#define MS_odmadd 5

#define ADD_MSG_0 1
#define ADD_MSG_1 2
#define ADD_MSG_2 3
#define ADD_MSG_3 4
#define ADD_MSG_4 5
#define ADD_MSG_5 6
#define ADD_MSG_6 7
#define ADD_MSG_7 8
#define ADD_MSG_8 9
#define ADD_MSG_9 10
#define ADD_MSG_10 11
#define ADD_MSG_11 12
#define ADD_MSG_12 13
#define ADD_MSG_13 14
#define ADD_MSG_14 15
#define ADD_MSG_15 16
#define ADD_MSG_16 17

/* definitions for set MS_odmget */
#define MS_odmget 6

#define GET_MSG_0 1
#define GET_MSG_1 2
#define GET_MSG_2 3
#define GET_MSG_3 4
#define GET_MSG_4 5
#define GET_MSG_5 6
#define GET_MSG_6 7
#define GET_MSG_7 8
#define GET_MSG_8 9

/* definitions for set MS_odmshow */
#define MS_odmshow 7

#define SHOW_MSG_0 1
#define SHOW_MSG_1 2
#define SHOW_MSG_2 3
#define SHOW_MSG_3 4
#define SHOW_MSG_4 5
#define SHOW_MSG_5 6
#define SHOW_MSG_6 7
#define SHOW_MSG_7 8
#define SHOW_MSG_8 9

#ifndef R5A
#include <locale.h>
#else
#define NLcatgets(a,b,c,d) d
#endif
