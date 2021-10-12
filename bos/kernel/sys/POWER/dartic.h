/* @(#)18        1.1  src/bos/kernel/sys/POWER/dartic.h, dd_artic, bos411, 9428A410j 11/11/93 15:24:44
 *
 * COMPONENT_NAME: dd_artic -- ARTIC diagnostic device driver.
 *
 * FUNCTIONS: Include file
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * This include file is used by ARTIC diagnostic device driver and
 * its configuration method
 *
 */


/* Maximum number of adapters allowed   */

#define MAXADAPTERS     16

/*
 *      Maximum number of RCM tasks is MAXRCMTASKS.
 *      We round that up to 256 for OA_MAXTASKS.
  */

#define MAXRCMTASKS     254
#define OA_MAXTASKS     256

/*
 *      Adapter Base Type Definitions
 */


#define         NO_ADAPTER              -1
#define         MULTIPORT_2              0
#define         PORTMASTER               2
#define         SP5_ADAPTER              3

/*
 *      Adapter Type Definitions
 */

#define         X25                      0
#define         MP2_4P232                1
#define         MP2_6PSYNC               2
#define         MP2_8P232                3
#define         MP2_8P422                4
#define         MP2_4P4224P232           5
#define         MP2_UNRECOG_EIB          99

#define         MPQP                     100
#define         PM_6PV35                 101
#define         PM_6PX21                 102
#define         PM_8P232                 103
#define         PM_8P422                 104
#define         PM_UNRECOG_EIB           199

#define         SP5                      200
#define         SP5_UNRECOG_EIB          299


/* ----------------------------------------------------------------
*       Miscellaneous definitions                                     *
* ----------------------------------------------------------------*/

/*
        The following definitions give the displacement of the Interface
        Block (IB) fields as indices to an array of uchar's.
*/

#define PC_SELECT                       0x00
#define INTID                           0x01
#define SUPVID                          0x02
#define CARDID                          0x03
#define MAXTASK                         0x04
#define MAXPRI                          0x05
#define MAXQUEUE                        0x06
#define MAXTIME                         0x07
#define TCBTAB_PTR                      0x08
#define PRIL_PTR                        0x0c
#define STOR_PTR                        0x10
#define RESERVE0                        0x14
#define HCD_1                           0x16
#define HCD_2                           0x18
#define BCB_OFF                         0x1a
#define TASKTAB_PTR                     0x1c
#define QUEUETAB_PTR                    0x20
#define TIMETAB_PTR                     0x24
#define RESERVE1                        0x28
#define RESERVE2                        0x2c
#define RESERVE3                        0x30
#define RESERVE4                        0x34
#define SIB_PTR                         0x34
#define STORSIZE                        0x38
#define DB0ID                           0x3a
#define DB1ID                           0x3b
#define STATARRAY                       0x3c

/*
 *      RCM Interface Block access definitions
 *
 *      To use these, the argument to the macro must be a char pointer
 *      that contains the address of the beginning of an Interface Block (IB),
 *      except for the STATARRAY, which requires a second argument, the task
 *      number.
 *
 *      These are necessary due to the alignment properties of the compiler used
 *      to create RCM.  There is no aligning of shorts, longs, or pointers to
 *      even or mod 4 boundaries.  And the host compiler does do alignment.
 */

#define ibPC_SELECT(p)                  (*((uchar  *)(p + PC_SELECT)))
#define ibINTID(p)                      (*((uchar  *)(p + INTID)))
#define ibSUPVID(p)                     (*((uchar  *)(p + SUPVID)))
#define ibCARDID(p)                     (*((uchar  *)(p + CARDID)))
#define ibMAXTASK(p)                    (*((uchar  *)(p + MAXTASK)))
#define ibMAXPRI(p)                     (*((uchar  *)(p + MAXPRI)))
#define ibMAXQUEUE(p)                   (*((uchar  *)(p + MAXQUEUE)))
#define ibMAXTIME(p)                    (*((uchar  *)(p + MAXTIME)))
#define ibTCBTAB_PTR(p)                 (*((ulong  *)(p + TCBTAB_PTR)))
#define ibPRIL_PTR(p)                   (*((ulong  *)(p + PRIL_PTR)))
#define ibSTOR_PTR(p)                   (*((ulong  *)(p + STOR_PTR)))
#define ibRESERVE0(p)                   (*((ushort  *)(p + RESERVE0)))
#define ibHCD_1(p)                      (*((ushort  *)(p + HCD_1)))
#define ibHCD_2(p)                      (*((ushort  *)(p + HCD_2)))
#define ibBCB_OFF(p)                    (*((ushort  *)(p + BCB_OFF)))
#define ibTASKTAB_PTR(p)                (*((ulong  *)(p + TASKTAB_PTR)))
#define ibQUEUETAB_PTR(p)               (*((ulong  *)(p + QUEUETAB_PTR)))
#define ibTIMETAB_PTR(p)                (*((ulong  *)(p + TIMETAB_PTR)))
#define ibRESERVE1(p)                   (*((ulong  *)(p + RESERVE1)))
#define ibRESERVE2(p)                   (*((ulong  *)(p + RESERVE2)))
#define ibRESERVE3(p)                   (*((ulong  *)(p + RESERVE3)))
#define ibRESERVE4(p)                   (*((ulong  *)(p + RESERVE4)))
#define ibSIB_PTR(p)                    (*((ulong  *)(p + SIB_PTR)))
#define ibSTORSIZE(p)                   (*((ushort  *)(p + STORSIZE)))
#define ibDB0ID(p)                      (*((uchar  *)(p + DB0ID)))
#define ibDB1ID(p)                      (*((uchar  *)(p + DB1ID)))
#define ibSTATARRAY(p,task)             (*((uchar  *)(p + STATARRAY + task)))


/**********************************************************************
 *                                                                    *
 *       THE FOLLOWING DEFINES THE INTERFACE BLOCK (IB)               *
 *       ADDRESS THAT A TASK MIGHT NEED. THIS VALUE SHOULD BE USED    *
 *       WITH A SEGMENT EQUAL TO 0.                                   *
 *                                                                    *
 **********************************************************************/


#define IBADDR          0x440          /* INTERFACE BLOCK START ADDRESS */


/* Read/Write CoProc memory address formats */
#define ADDRFMT_PAGE    0xFF            /* segpage is a coproc page             */
#define ADDRFMT_SEGMENT 0x00            /* segpage is a coproc segment  */
#define ADDRFMT_32BIT   0x01            /* segpage and offset are               */
                                                                        /* 32-bit phys addr                             */

/*************************** RETURN CODES *******************************/



/*
 * darticconfig returns success or failure based on whether it successfully
 * programs and configures the device.  If successful, darticconfig returns 0.
 * Following are the error codes that darticconfig can return.
 */

#define     DARTIC_SELFTEST_FAILURE                  116
#define     DARTIC_I_CLEAR_FAILURE                   117
#define     DARTIC_I_INIT_FAILURE                    118
#define     DARTIC_UNKNOWN_ADAPTER                   119
#define     DARTIC_UNKNOWN_EXCEPTION                 120
#define     DARTIC_PINCODE_FAILURE                   121
#define     DARTIC_UIOMOVE_FAILURE                   122
#define     DARTIC_INVALID_ADAPTER                   123
#define     DARTIC_DEVSWADD_FAILURE                  124
#define     DARTIC_INVALID_WINDOW_SIZE               125
#define     DARTIC_DEVSWDEL_FAILURE                  126
#define     DARTIC_UNPINCODE_FAILURE                 127



/*
 * Device Driver and C Library Return Codes
 */

#define         NO_ERROR                                0x0000
#define         E_ICA_INVALID_COPROC                    0xFF05
#define         E_ICA_INVALID_TASK_STATUS               0xFF06
#define         E_ICA_INVALID_PAGE                      0xFF07
#define         E_ICA_INVALID_OFFSET                    0xFF08
#define         E_ICA_INVALID_FORMAT                    0xFF09
#define         E_ICA_TIMEOUT                           0xFF0B
#define         E_ICA_INVALID_CONTROL                   0xFF0D
#define         E_ICA_BAD_PCSELECT                      0xFF11
#define         E_ICA_CMD_REJECTED                      0xFF12
#define         E_ICA_NO_CMD_RESPONSE                   0xFF13
#define         E_ICA_OB_SIZE                           0xFF14
#define         E_ICA_ALREADY_REG                       0xFF15
#define         E_ICA_NOT_REG                           0xFF17
#define         E_ICA_XMALLOC_FAIL                      0xFF25
#define         E_ICA_ALREADY_OPEN                      0xFF26
#define         E_ICA_INTR                              0xFF28
#define         E_ICA_DMASETUP                          0xFF29
#define         E_ICA_DMAREL                            0xFF2A
#define         E_ICA_DMAPARMS                          0xFF2B
#define         E_ICA_NOMEM                             0xFF2C
#define         E_ICA_INVALID_POSREG                    0xFF2D

/*
**********************************************************************
*                                                                    *
*       THE FOLLOWING EQUATES DEFINE THE PRIMARY STATUS BITS.        *
*                                                                    *
**********************************************************************
*/

#define PSLOAD          0x01            /* LOADED */
#define PSINIT          0x02            /* INITIALIZED */
#define PSBIT2          0x04            /* USER DEFINED BIT */
#define PSBIT3          0x08            /* USER DEFINED BIT */
#define PSERR           0x10            /* ERROR */
#define PSSTAV          0x20            /* STATUS AVAILABLE */
#define PSOBBZ          0x40            /* OUTPUT BUFFER BUSY */
#define PSBUSY          0x80            /* BUSY */

/* The following defines a mask to clear busy and error bits from status */

#define PSNOTBZ         PSLOAD|PSINIT|PSBIT2|PSBIT3|PSSTAV

/*
**********************************************************************
*   THE FOLLOWING  DEFINE THE VALUES IN A BUFFER CONTROL                 *
*   BLOCK (BCB).                                                     *
*                                                                    *
**********************************************************************
*/

#define CMD                     0x00    /* COMMAND BYTE */
#define STATLNG         0x01    /* STATUS AND CONTROL LENGTH */
#define STATOFF         0x03    /* STATUS AND CONTROL OFFSET */
#define STATPAG         0x05    /* STATUS AND CONTROL PAGE */
#define INLNG           0x06    /* INPUT BUFFER LENGTH */
#define INLNGlow        0x06    /* INPUT BUFFER LENGTH */
#define INLNGhigh       0x07    /* INPUT BUFFER LENGTH */
#define INOFF           0x08    /* INPUT BUFFER OFFSET */
#define INOFFlow        0x08    /* INPUT BUFFER OFFSET */
#define INOFFhigh       0x09    /* INPUT BUFFER OFFSET */
#define INPAG           0x0a    /* INPUT BUFFER PAGE */
#define OUTLNG          0x0b    /* OUTPUT BUFFER LENGTH */
#define OUTOFF          0x0d    /* OUTPUT BUFFER OFFSET */
#define OUTPAG          0x0f    /* OUTPUT BUFFER PAGE */


#define BCBSIZE 0x10    /* SIZE OF BCB */

#define bcbCMD(p)               (*((uchar *)(p + CMD)))
#define bcbSTATLNG(p)           (*((ushort *)(p + STATLNG)))
#define bcbSTATOFF(p)           (*((ushort *)(p + STATOFF)))
#define bcbSTATPAG(p)           (*((uchar *)(p + STATPAG)))
#define bcbINLNG(p)             (*((ushort *)(p + INLNG)))
#define bcbINLNGlow(p)          (*((uchar *)(p + INLNGlow)))
#define bcbINLNGhigh(p)         (*((uchar *)(p + INLNGhigh)))
#define bcbINOFF(p)             (*((ushort *)(p + INOFF)))
#define bcbINOFFlow(p)          (*((uchar *)(p + INOFFlow)))
#define bcbINOFFhigh(p)         (*((uchar *)(p + INOFFhigh)))
#define bcbINPAG(p)             (*((uchar *)(p + INPAG)))
#define bcbOUTLNG(p)            (*((ushort *)(p + OUTLNG)))
#define bcbOUTOFF(p)            (*((ushort *)(p + OUTOFF)))
#define bcbOUTPAG(p)            (*((uchar *)(p + OUTPAG)))

/* same as bcbOUTOFF(p) except cast as a ulong...       */

#define bcbOUTPTR(p)            (*((ulong *)(p + OUTOFF)))

