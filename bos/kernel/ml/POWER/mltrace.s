# @(#)24        1.3  src/bos/kernel/ml/POWER/mltrace.s, sysml, bos411, 9428A410j 6/18/93 08:33:27
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: mltrace
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992,1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

                .file "mltrace.s"
                .machine "com"

#******************************************************************************
#
# NAME: mltrace
#
# CALL: trchookx(id, w1, w2, w3)
#
# FUNCTION:
#       Simple trace hook for debugging.  Data is
# traced to a V=R circular buffer.
#
# NOTES:
#       This function can be extended to steal the trace subsystem's
# trace hook routine when AIX trace is not loaded.
#
# ASSUMPTIONS:
#	This function must preserve GPRs 0-6
#
# RETURNS:
#       None
#
# PSEUDO CODE:
#
#       struct{
#               int trcbuf[BUFSIZE];
#               int *trcptr;
#       }tb;
#
#       void
#       mltrace(
#               int id,         /* hook id */
#               int w1,         /* data word 1 */
#               int w2,         /* data word 2 */
#               int w3)         /* data word 3 */
#       {
#               int oldmsr;
#               int *ptr;
#
#               /* disable all interrupts
#                */
#               oldmsr = mfmsr();
#               mtmsr(DISABLED_REAL_MSR);
#               isync();
#
#               /* get address of last trace entry, and increment
#                * to get address of next trace entry
#                */
#               ptr = tb.trcptr;
#               ptr = (int)ptr + TRCREC;
#
#               /* check for buffer wrap
#                */
#               if (ptr == &tb.trcptr)
#               {
#                       ptr = &tb.trcbuf[0];
#               }
#               
#               /* log trace data
#                */
#               *(ptr+0) = id;
#               *(ptr+1) = w1;
#               *(ptr+2) = w2;
#               *(ptr+3) = w3;
#
#               /* update trace pointer
#                */
#               tb.trcptr = ptr;
#
#               /* restore callers msr
#                */
#               mtmsr(oldmsr);
#               isync();
#       }
#
#******************************************************************************

ifdef(`MLDEBUG',`

        .csect  trace_sect[PR],2
ENTRY(mltrace):
        .globl  ENTRY(mltrace)

        lil     r10, DISABLED_REAL_MSR  # get msr value
        mfmsr   r9                      # Save old msr
        mtmsr   r10                     # disable all interrupts
        isync                           # sync on msr update

        LTOC(r7, trcptr, data)          # get address of trace buffer pointer
        l       r8, 0(r7)               # load trace pointer
        ai      r8, r8, TRCREC          # Bump to next record
        cmp     cr0, r7, r8             # check for overflow
        bne     cr0, no_wrap            # branch if have not wrapped
        LTOC(r8, trcbuf, data)          # get start of trace buffer
no_wrap:

        st      r3, 0(r8)               # store trace data
        st      r4, 4(r8)
        st      r5, 8(r8)
        st      r6, 12(8)
        st      r8, 0(r7)               # update trace pointer
        mtmsr   r9                      # restore msr
        isync                           # sync on msr update
        br                              # return to caller

        .align  2
        .set    BUFSIZE, 1024           # Number of trace entries
        .set    TRCREC, 4*4             # size of a trace record

trcbuf: .space  BUFSIZE*TRCREC          # trace buffer
trcptr: .long   trcbuf                  # current trace buffer
        .globl  trcbuf
        .globl  trcptr

        .toc
        TOCL(trcptr, data)
        TOCL(trcbuf, data)

include(machine.m4)

',`
        .csect dumy[DS]
        .long   0                       # for assembler problem
')

