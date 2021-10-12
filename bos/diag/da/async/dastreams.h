/* @(#)37       1.3  src/bos/diag/da/async/dastreams.h, daasync, bos411, 9428A410j 7/6/94 21:27:50 */
/*
 * COMPONENT_NAME: DAASYNC
 *
 * FUNCTIONS:   This file contains global defines, variables, and structures
 *              for the Async adapter diagnostic application.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/termio.h>
#include <stropts.h>

#define DEFAULT_STACK_DEPTH     3  /* ldterm, tioc, driver */
#define STACK_NOT_CHANGED       0  /* Stack was checked and not altered */
#define PROCESSED               1  /* Stack has been checked already */
#define NOT_PROCESSED           0  /* Stack needs checking before testing */

#ifndef NULL
#define NULL                    0
#endif

#define INVALID_CALL_PARAMETER  3 /* Call inputs invalid. Ether the stack
                                        depth was not possible or the file
                                        descriptor for the port was negative
                                        or pointed to STD-IN/OUT/ERR. */
#define MEMORY_ALLOC_FAILED     2 /* A call to either malloc or free failed */
#define STACK_CALL_FAILED       1 /* An ioctl call (ie:"streamio") failed and
                                        the state of the stream stack is
                                        unknown. */

typedef struct {
    struct str_list    CallArguments;
    struct str_mlist   NameSpace[99];
    } StackListStruct;

typedef struct {
    int                 StackDepth;
    int                 PortId;
    int                 ArraySize;
    int                 StackProcessed;
    struct termios      PortAttributes;
    StackListStruct    *StackList;
  } StreamStackStruct;

#ifndef DASTREAMS_SRC
extern int SetDAStack(StreamStackStruct *StreamStack);
extern int RestoreUserStack(StreamStackStruct *StreamStack);
#endif

/*
   In the AIX world streamio is just an ioctl. In other worlds other
   conditions obtain.  So here we have a #define to set streamio
   to the current default condition. If this changes, and it may, then
   change the def to and extern or what-ever.
*/

#ifndef streamio
#define streamio ioctl
#endif
