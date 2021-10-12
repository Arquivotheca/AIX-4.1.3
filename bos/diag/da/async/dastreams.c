static char sccsid[] = "@(#)36  1.3  src/bos/diag/da/async/dastreams.c, daasync, bos411, 9428A410j 7/6/94 21:26:28";
/*
 * COMPONENT_NAME: DAASYNC
 *
 * FUNCTIONS:   SetDAStack()
 *              RestoreUserStack()
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

#define DASTREAMS_SRC

/* #include <sys/stropts.h> is done in the dastreams.h */

#include "dastreams.h"
#include <sys/errno.h>

/*
 * NAME: SetDAStack
 *
 * FUNCTION:  SetDAStack() changes the streams stack for the port to the
 *              default streams stack used by the DA. The old stack is
 *              saved in the input structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  zero (0) if successfull or negative one (-1) if failed
 */
int SetDAStack(StreamStackStruct *StreamStack)

{

    int                 temp;
    struct termios      DiagAttributes;

    if((StreamStack->StackDepth < 1) || (StreamStack->PortId < 3)) {
        err(0x131,StreamStack->StackDepth,StreamStack->PortId);
        errno = EINVAL;
        return (INVALID_CALL_PARAMETER);
    }
    else {
        StreamStack->ArraySize = (StreamStack->StackDepth * (FMNAMESZ + 1) +
                                sizeof(struct str_list));

        if ((StreamStack->StackList = (StackListStruct *) malloc(StreamStack->ArraySize)) == NULL) {
            err(0x132,0,0);
            return (MEMORY_ALLOC_FAILED);
        }
/*
    The values of the port's termio attributes can be lost when pushing or
    popping modules. Therefore we save the current values prior to changing
    the stack.
*/
        if ( temp = ioctl( StreamStack->PortId,
                           TCGETA,
                           &(StreamStack->PortAttributes)) != 0) {
            err(0x133,temp,0);
            return (STACK_CALL_FAILED);
        }
        StreamStack->StackList->CallArguments.sl_nmods
                                                = StreamStack->StackDepth;
        StreamStack->StackList->CallArguments.sl_modlist =
                        (struct str_mlist *)((int) StreamStack->StackList +
                         sizeof(struct str_list));

        if (((temp = streamio(StreamStack->PortId,
                              I_LIST,
                             StreamStack->StackList)) == -1) ||
                (temp != StreamStack->StackDepth)) {
            err(0x134,temp,0);
            return (STACK_CALL_FAILED);
        }
    /*
        The StackDepth includes the driver which must not be removed. So
        the count must be one (1) less than the StackDepth if the while
        is to end at the right place.
    */
        temp = StreamStack->StackDepth -1;
        while (temp) {
            if(streamio(StreamStack->PortId, I_POP, 0) == -1) {
                err(0x240,0,errno);
                return (STACK_CALL_FAILED);
            }
            temp--;
        }
        if(streamio(StreamStack->PortId, I_PUSH, "ldterm") == -1) {
            err(0x135,0,errno);
            return (STACK_CALL_FAILED);
        }
        if(streamio(StreamStack->PortId, I_PUSH, "tioc") == -1) {
            err(0x136,0,errno);
            return (STACK_CALL_FAILED);
        }
/*
    Now that we have our stack in place setup the diag default port
    attributes.
*/
        /* Get the current set - they may or may not mean anything */

        if ((ioctl (StreamStack->PortId, TCGETA, &DiagAttributes)) != 0) {
            err(0x137,0,errno);
            return (STACK_CALL_FAILED);
        }

        /* Turn off all the input and output processing  */

        DiagAttributes.c_lflag = 0;
        DiagAttributes.c_cc[VMIN] = 0;
        DiagAttributes.c_cc[VTIME] = 0;
        DiagAttributes.c_iflag = 0;
        DiagAttributes.c_oflag = 0;

        /* Set values for hardware characteristics  */

        DiagAttributes.c_cflag = CREAD | CLOCAL;
        /* Now set the new values into place */

        if ((ioctl (StreamStack->PortId, TCGETA, &DiagAttributes)) != 0) {
            err(0x138,0,errno);
            return (STACK_CALL_FAILED);
        }

        return (0);
    }
}
/*
 * NAME: RestoreUserStack
 *
 * FUNCTION:  RestoreUserStack() changes the streams stack for the port
 *              form the existing stack to the stack passed in with the
 *              StreamStack parameter. It is assumed that this is the
 *              user stack saved by SetDAStack().
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  zero (0) if successfull or negative one (-1) if failed on an
 *           ioctl() call or ngeative two (-2) if the free() call fails.
 */
int RestoreUserStack(StreamStackStruct *StreamStack)

{

    int                temp;
    struct str_mlist  UserModule;

    if((StreamStack->StackDepth == 0) ||
       (StreamStack->StackDepth !=
                StreamStack->StackList->CallArguments.sl_nmods) ||
       (StreamStack->PortId < 3)) {
          err(0x139,StreamStack->StackDepth,StreamStack->PortId);
          errno=EINVAL;
          return (INVALID_CALL_PARAMETER);
    }
    else {
        if((temp = streamio(StreamStack->PortId, I_LIST, NULL)) == -1) {
            err(0x140,temp,0);
            return(STACK_CALL_FAILED);
        }
    /*
      The value returned by streamio() for this ioctl is the number of
      modules on the stack including the driver. The driver must not be
      removed (can not be removed) so temp must be decremented to make
      the while end at the right point.
    */
        temp--;
        while(temp) {
            if (streamio(StreamStack->PortId, I_POP, NULL) == -1) {
                err(0x141,StreamStack->PortId,errno);
                return (STACK_CALL_FAILED);
            }
            temp--;
        }
    /*
      The value in StackDepth was returned by streamio() with this ioctl.
      Thus it is the number of modules on the stack including the driver.
      The driver must not be removed (can not be removed) so temp must be
      decremented to make the while end at the right point.
    */
        temp = StreamStack->StackDepth - 1;
        while(temp) {
            UserModule = StreamStack->StackList->NameSpace[temp-1];
            if (streamio(StreamStack->PortId, I_PUSH, &UserModule) == -1) {
                err(0x142,StreamStack->PortId,errno);
                return (STACK_CALL_FAILED);
            }
            temp--;
        }
/*
    Pushing and Popping the streams stack can cause the values of the port's
    attributes to become undefined. The attributes were saved in the
    SetDAStreams call and are here restored.
*/
        if (ioctl( StreamStack->PortId,
                   TCGETA,
                   &(StreamStack->PortAttributes)) != 0) {
            err(0x143,0,errno);
            return (STACK_CALL_FAILED);
        }
        if(free(StreamStack->StackList) == -1) {
            err(0x144,0,0);
            return (MEMORY_ALLOC_FAILED);
        }
        StreamStack->StackProcessed = 0;
        StreamStack->StackDepth = 0;
        StreamStack->ArraySize = 0;
        StreamStack->StackList = NULL;
        return(0);
    }
}

#undef DASTREAMS_SRC
