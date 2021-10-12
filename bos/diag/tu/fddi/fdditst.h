/* @(#)41       1.1.3.1  src/bos/diag/tu/fddi/fdditst.h, tu_fddi, bos41J, 9512A_all 3/2/95 10:24:34 */
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Header HTX/Mfg. FDDI Definitions File

Header file contains basic definitions needed by three applications for
testing the FDDI Adapter:

        1)  Hardware exerciser invoked by HTX,
        2)  Manufacturing application, and
        3)  Diagnostic application.


*****************************************************************************/
/*
 * We include the HTX header file hxihtx.h because we have a need
 * to access this structure within test units in case the invoker
 * is an HTX Hardware Exerciser application (i.e., NOT the
 * manufacturing diagnostic application).  Since, the main driver
 * function, exectu(), has already been defined for use by both
 * types of applications, we will "sneak" in this structure
 * inside the TUTYPE one that we are allowed to define and pass in.
 */
#include <hxihtx.h>

/*
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 */
#define INVOKED_BY_HTX   99

/************************************************************************/
/* Error types for FDDI test units.                                     */
/************************************************************************/
#define SYS_ERR     0x00
#define FDDI_ERR    0x01
#define LOG_ERR     0x02

#define PASSED        0
#define FAILED       -1

/************************************************************************/
/*  DMA commands                                                        */
/************************************************************************/
#define LOCK_DMA        0x01
#define UNLOCK_DMA      0x02

#define ALLOCATE_DMA    0x01
#define WRITE_DMA       0x02
#define READ_DMA        0x03
#define FREE_DMA        0x04

/************************************************************************/
/* FDDI_ERR (Error codes returned from the adapter, itself)             */
/************************************************************************/
#define INIT_TST_ERR    0x0030  /* Initial Test error.                  */
#define MC_CRC_ERR      0x0031  /* Microcode not loaded or microcode
                                        CRC error.                      */
#define RAM_ERR         0x0032  /* RAM error.                           */
#define INSTR_TST_ERR   0x0033  /* Instruction test error.              */
#define XOP_INTR_ERR    0x0034  /* XOP test error, interrupt test error.*/
#define PH_HDW_ERR      0x0035  /* PH hardware error.                   */
#define SIF_REG_ERR     0x0036  /* SIF register error.                  */
#define PARMS1_ERR      0x0011  /* Parameters written by system
                                        incomplete/invalid.             */
#define PARMS2_ERR      0x0012  /* Parameters written by system
                                        incomplete/invalid.             */
#define PARMS3_ERR      0x0013  /* Parameters written by system
                                        incomplete/invalid.             */
#define PARMS4_ERR      0x0014  /* Parameters written by system
                                        incomplete/invalid.             */
#define PARMS5_ERR      0x0015  /* Parameters written by system
                                        incomplete/invalid.             */
#define PARMS6_ERR      0x0016  /* Parameters written by system
                                        incomplete/invalid.             */
#define PARMS7_ERR      0x0017  /* Parameters written by system
                                        incomplete/invalid.             */
#define MMIO_PAR_ERR    0x0018  /* MMIO parity error.                   */
#define DMA_TIM_ERR     0x0019  /* DMA timeout error.                   */
#define DMA_PAR_ERR     0x001A  /* DMA parity error.                    */
#define DMA_BUS_ERR     0x001B  /* DMA bus error.                       */
#define DMA_CMP_ERR     0x001C  /* DMA data compare error.              */
#define ADAP_CHK_ERR    0x001D  /* Adapter check.                       */

#define NONS_INIT_ERR   0x8820  /* Nonsensical initialization results.  */
#define INIT_CMP_ERR    0x8830  /* Initialization parms. in adpater
                                        do not compare.                 */
#define INIT_TIME_ERR   0x8832  /* Initialization times out - no status.*/
#define OPEN_TIME_ERR   0x8842  /* Time out during adapter open.        */
#define OPN_PARM_ERR    0x8C00  /* Open parameters invalid.             */
#define LM_WRAP_ERR     0x0211  /* Lobe media wrap test failed.         */
#define LM_WRAP_PAS     0x0212  /* Lobe media wrap test passed.         */

#define OPN_PINS_ERR    0x8D20  /* Open failed - physical insertion.    */
#define OPN_ADDR_ERR    0x8D30  /* Open failed - address verification.  */
#define OPN_RPOLL_ERR   0x8D40  /* Open failed - ring poll.             */
#define OPN_RPARM_ERR   0x8D50  /* Open failed - request parameters.    */

#define XMIT_PARM_ERR   0x9000  /* Transmit list parameter error.       */
#define XMIT_FSTR_ERR   0x9100  /* Error during transmit or frame strip.*/
#define XR_CMP_ERR      0x9400  /* Received and transmit data did not
                                        match.                          */
#define WRAP_TIM_ERR    0x9401  /* Wrap time-out.                       */
#define SCB_BUSY0_ERR   0x9402  /* System Command Block busy.           */
#define SCB_BUSY1_ERR   0x9403  /* System Command Block busy.           */
#define UNX_SFT_ERR     0x9C00  /* Unexpected software error.           */

#define RSTAT_OFF_ERR   0xA000  /* Ring status causing adapter to
                                        get off.                        */
#define UNX_HDW_ERR     0xA400  /* Unexpected hardware error.           */
#define SCB_BUSY2_ERR   0xA408  /* SCB was busy not allowing CLOSE.     */

#define AC_XDMA_R_ERR   0xFFDA  /* Adapter check, all except DMA
                                        abort - read.                   */
#define OPN_TIM_ERR     0xFFDB  /* OPEN timeout.                        */
#define AC_DMA_R_ERR    0xFFDC  /* Adapter check, DMA abort - read.     */
#define ADAP_NOPEN_ERR  0xFFF5  /* Adapter not OPENed.                  */
#define ADAP_OPN_ERR    0xFFF6  /* Adapter is already OPENed.           */

/************************************************************************/
/* Error codes (Logic Errors)                                           */
/************************************************************************/

#define POS_RD_ERR      0x0080  /* Error reading POS register */
#define POS0_RD_ERR     0x0080  /* Error reading POS register 0 */
#define POS1_RD_ERR     0x0081  /* Error reading POS register 1 */
#define POS2_RD_ERR     0x0082  /* Error reading POS register 2 */
#define POS3_RD_ERR     0x0083  /* Error reading POS register 3 */
#define POS4_RD_ERR     0x0084  /* Error reading POS register 4 */
#define POS5_RD_ERR     0x0085  /* Error reading POS register 5 */
#define POS6_RD_ERR     0x0086  /* Error reading POS register 6 */
#define POS7_RD_ERR     0x0087  /* Error reading POS register 7 */

#define POS_WR_ERR      0x0090  /* Error writing POS register */
#define POS0_WR_ERR     0x0090  /* Error writing POS register 0 */
#define POS1_WR_ERR     0x0091  /* Error writing POS register 1 */
#define POS2_WR_ERR     0x0092  /* Error writing POS register 2 */
#define POS3_WR_ERR     0x0093  /* Error writing POS register 3 */
#define POS4_WR_ERR     0x0094  /* Error writing POS register 4 */
#define POS5_WR_ERR     0x0095  /* Error writing POS register 5 */
#define POS6_WR_ERR     0x0096  /* Error writing POS register 6 */
#define POS7_WR_ERR     0x0097  /* Error writing POS register 7 */

#define POS_CMP_ERR     0x0100  /* POS register r/w didn't compare */
#define POS0_CMP_ERR    0x0100  /* POS 0 has invalid value */
#define POS1_CMP_ERR    0x0101  /* POS 1 has invalid value */
#define POS2_CMP_ERR    0x0102  /* POS 2 Write/Read did not compare */
#define POS3_CMP_ERR    0x0103  /* POS 3 Write/Read did not compare */
#define POS4_CMP_ERR    0x0104  /* POS 4 Write/Read did not compare */
#define POS5_CMP_ERR    0x0105  /* POS 5 Write/Read did not compare */
#define POS6_CMP_ERR    0x0106  /* POS 6 Write/Read did not compare */
#define POS7_CMP_ERR    0x0107  /* POS 7 Write/Read did not compare */

#define SIF_RD_ERR      0x0110  /* Error Reading a SIF Register */
#define RHSR_RD_ERR     0x0110  /* Error Reading HSR */
#define RHCR_RD_ERR     0x0111  /* Error Reading HCR */
#define RNS1_RD_ERR     0x0112  /* Error Reading NS1 Register */
#define RNS2_RD_ERR     0x0113  /* Error Reading NS2 Register */
#define RHSR_MK_RD_ERR  0x0114  /* Error Reading HSR Mask Register */
#define RNS1_MK_RD_ERR  0x0115  /* Error Reading NS1 Mask Register */
#define RNS2_MK_RD_ERR  0x0116  /* Error Reading NS2 Mask Register */
#define RALISA_RD_ERR   0x0117  /* Error Reading ALISA Register */
#define RDEF_RD_ERR     0x0118  /* Error Reading System Interface Registers */

#define SIF_WR_ERR      0x0120  /* Error Writing a SIF Register */
#define RHSR_WR_ERR     0x0120  /* Error Writing HSR */
#define RHCR_WR_ERR     0x0121  /* Error Writing HCR */
#define RNS1_WR_ERR     0x0122  /* Error Writing NS1 Register */
#define RNS2_WR_ERR     0x0123  /* Error Writing NS2 Register */
#define RHSR_MK_WR_ERR  0x0124  /* Error Writing HSR Mask Register */
#define RNS1_MK_WR_ERR  0x0125  /* Error Writing NS1 Mask Register */
#define RNS2_MK_WR_ERR  0x0126  /* Error Writing NS2 Mask Register */
#define RALISA_WR_ERR   0x0127  /* Error Writing ALISA Register */
#define RDEF_WR_ERR     0x0128  /* Error Writing System Interface Registers */

#define SIF_CMP_ERR     0x0130  /* Write/Read of a SIF Register miscompared */
#define RHSR_CMP_ERR    0x0130  /* HSR Write/Read did not compare */
#define RHCR_CMP_ERR    0x0131  /* HCR Write/Read did not compare */
#define RNS1_CMP_ERR    0x0132  /* NS1 Register Write/Read did not compare */
#define RNS2_CMP_ERR    0x0133  /* NS2 Register Write/Read did not compare */
#define RHSR_MK_CMP_ERR 0x0134  /* HSR Mask Register Write/Read did not
                                        compare */
#define RNS1_MK_CMP_ERR 0x0135  /* NS1 Mask Register Write/Read did not
                                        compare  */
#define RNS2_MK_CMP_ERR 0x0136  /* NS2 Mask Register Write/Read did not
                                        compare */
#define RALISA_CMP_ERR  0x0137  /* ALISA Register Write/Read did not compare */
#define RDEF_CMP_ERR    0x0138  /* System Interface Registers did not compare */

#define SIF_SAVE_REG    0x0140  /* Error saving SIF registers */
#define SIF_RESTORE_REG 0x0141  /* Error restoring SIF registers */

#define SH_RAM_RD_ERR   0x0300  /* Error reading shared memory */
#define SH_RAM_WR_ERR   0x0301  /* Error writing shared memory */
#define SH_RAM_CMP_ERR  0x0302  /* Shared Memory Write/Read did not compare */

#define FDDIRAM1_RD_ERR  0x0310 /* Error reading FDDI RAM Buffer (1 transfer) */
#define FDDIRAM1_WR_ERR  0x0311 /* Error writing FDDI RAM Buffer (1 transfer) */
#define FDDIRAM1_CMP_ERR 0x0312 /* FDDI RAM (1st Buffer) Write/Read did not
                                        compare */

#define FDDIRAM2_RD_ERR  0x0320 /* Error reading FDDI RAM Buffer (2 transfers)*/
#define FDDIRAM2_WR_ERR  0x0321 /* Error writing FDDI RAM Buffer (2 transfers)*/
#define FDDIRAM2_CMP_ERR 0x0322 /* FDDI RAM (2nd Buffer) Write/Read did not
                                        compare */

#define FDDIRAM3_RD_ERR  0x0330 /* Error reading FDDI RAM Buffer (3 transfers)*/
#define FDDIRAM3_WR_ERR  0x0331 /* Error writing FDDI RAM Buffer (3 transfers)*/
#define FDDIRAM3_CMP_ERR 0x0332 /* FDDI RAM (3rd Buffer) Write/Read did not
                                        compare */

#define DATASTOR_RD_ERR 0x0335  /* Error reading Data Store area. */
#define DATASTOR_WR_ERR 0x0336  /* Error writing Data Store area. */
#define RBC_CMP_ERR     0x0337  /* Compare error of RBC */
#define SWRESET_CMP_ERR 0x0338  /* Compare error of Software Reset. */

#define BUSDAT1_RD_ERR  0x0340  /* Error reading NP Bus Data Store
                                        (1 transfer) */
#define BUSDAT1_WR_ERR  0x0341  /* Error writing NP Bus Data Store
                                        (1 transfer) */
#define BUSDAT1_CMP_ERR 0x0342  /* NP Bus Data Store (1st Buffer) Write/Read
                                        did not compare */

#define BUSDAT2_RD_ERR  0x0350  /* Error reading NP Bus Data Store
                                        (2 transfers) */
#define BUSDAT2_WR_ERR  0x0351  /* Error writing NP Bus Data Store
                                        (2 transfers) */
#define BUSDAT2_CMP_ERR 0x0352  /* NP Bus Data Store (2nd Buffer) Write/Read
                                        did not compare */

#define BUSDAT3_RD_ERR  0x0360  /* Error reading NP Bus Data Store
                                        (3 transfers) */
#define BUSDAT3_WR_ERR  0x0361  /* Error writing NP Bus Data Store
                                        (3 transfers) */
#define BUSDAT3_CMP_ERR 0x0362  /* NP Bus Data Store (3rd Buffer) Write/Read
                                        did not compare */

#define BUSPRG1_RD_ERR  0x0370  /* Error reading NP Bus Program Store
                                        (1 transfer) */
#define BUSPRG1_WR_ERR  0x0371  /* Error writing NP Bus Program Store
                                        (1 transfer) */
#define BUSPRG1_CMP_ERR 0x0372  /* NP Bus Program Store (1st Buffer)
                                        Write/Read did not compare*/

#define BUSPRG2_RD_ERR  0x0380  /* Error reading NP Bus Program Store
                                        (2 transfers) */
#define BUSPRG2_WR_ERR  0x0381  /* Error writing NP Bus Program Store
                                        (2 transfers) */
#define BUSPRG2_CMP_ERR 0x0382  /* NP Bus Program Store (2nd Buffer)
                                        Write/Read did not compare*/

#define BUSPRG3_RD_ERR  0x0390  /* Error reading NP Bus Program Store
                                        (3 transfers) */
#define BUSPRG3_WR_ERR  0x0391  /* Error writing NP Bus Program Store
                                        (3 transfers) */
#define BUSPRG3_CMP_ERR 0x0392  /* NP Bus Program Store (3rd Buffer)
                                        Write/Read did not compare*/

#define BASE_WRAP_ERR   0x0400  /* Base FDDI card wrap test failed. */

#define OPEN_MICRO_ERR  0x0500  /* Error opening Microcode file */
#define STAT_MICRO_ERR  0x0501  /* Error STAT system call failed */
#define RD_MICRO_ERR    0x0502  /* Error reading Microcode file */
#define DOWN_MICRO_ERR  0x0503  /* Error Downloading Microcode */
#define INVALID_SLOT    0x0504  /* Error determining port from slot number */

#define INTR_LVL_ERR    0x0700  /* Error in reseting adapter Interrupt Level */

#define OPEN_DIAG_ERR   0x0800  /* Error opening Diagnostic Microcode file */
#define STAT_DIAG_ERR   0x0801  /* Error STAT system call failed */
#define RD_DIAG_ERR     0x0802  /* Error reading Diagnostic Microcode file */
#define DOWN_DIAG_ERR   0x0803  /* Error Downloading Diagnostic Microcode */

#define HCR_CMD_WR_ERR  0x0901  /* Error writing HCR Cmd for Adapter Tests */

#define RD_VPD_ERR      0x0950  /* Error reading VPD Information */
#define VPD_NOT_RD      0x0951  /* VPD not read */
#define VPD_INVAL       0x0952  /* VPD invalid. */
#define VPD_UNDEFINE    0x0953  /* VPD undefined status return code. */
#define INVALID_HDR_VPD 0x0954  /* Invalid VPD header information */
#define VPD_CRC_ERR     0x0955  /* VPD CRC does not compare. */

#define DUAL_WRAP_ERR   0x0A00  /* Dual wrap test failed. */
#define NO_EXTENDER_ERR 0x0A01  /* No Extender Card found. */

#define RD_EVPD_ERR     0x0B00  /* Error reading Extender VPD Information */
#define EVPD_NOT_READ   0x0B01  /* Extender VPD not read. */
#define EVPD_INVALID    0x0B03  /* Extender VPD invalid. */
#define EVPD_UNDEFINE   0x0B04  /* Extender VPD undefined status return code. */
#define EVPD_CRC_ERR    0x0B05  /* Extender VPD CRC does not compare. */

/*
 * Start and Halt Error Codes.
 */
#define OPEN_DD_ERR     0x1000  /* Open of FDDI device driver failed. */
#define RULE_FD_ERR     0x1001  /* Rule File error. */
#define START_FAIL      0x1010  /* START of FDDI failed - no status. */
#define START_BAD_NETID 0x1011  /* START failed - bad NETID. */
#define GET_STATUS_ERR  0x1012  /* GET_STATUS for START_DONE failed. */
#define START_DONE_ERR  0x1013  /* START_DONE never received. */
#define START_BAD_NETAD 0x1014  /* START of FDDI failed - bad net address. */
#define START_TIME_ERR  0x1015  /* START status timed out - no completion. */

#define HALT_ERR        0x1100  /* Halt failed - no status. */

/*
 * DMA Error Codes.
 */
#define DMA_NOTA_ERR    0x2000  /* DMA Buffer not yet allocated */
#define DMA_RD_ERR      0x2001  /* DMA Buffer read error */
#define DMA_WR_ERR      0x2002  /* DMA Buffer write error */
#define DMA_UNK_ERR     0x2003  /* Unknown Illegal DMA operation error */

/*
 * ODM Error Codes
 */

#define ODMOPEN_ERR     0x4000  /* Error on ODM open. */
#define NOCuDv_ERR      0x4001  /* Error on ODM get object. */
#define ODMGET_ERR      0x4002  /* Error on ODM get object. */
#define ODMCLOSE_ERR    0x4003  /* Error on ODM close. */
#define GETATTR_ERR     0x4004  /* Error on get attatch Microcode file. */
#define ODMINIT_ERR     0x4005  /* Error on initializing ODM. */
#define NOATTR_ERR      0x4006  /* No attribute found. */
#define BADATTR_ERR     0x4007  /* Bad attribute found. */

/*
 * FINDMCODE function call errors
 */

 #define FINDMCODE_ERR  0x4050  /* Bad sts returned from findmcode function */

/*
 * STATX function call errors
 */

 #define STATX_ERR  0x4051  /* Bad sts returned from statx function */

/*
 * Specific Test Unit Error Codes.
 */
#define ADDR_SHORT_ERR  0x5000  /* Failed to write Set Address Short cmd. */
#define ADDSHRT_INT_ERR 0x5001  /* No interrupt completion for Set Address
                                        Short command. */

#define ADDR_LONG_ERR   0x5100  /* Failed to write Set Address Long cmd. */
#define ADDLONG_INT_ERR 0x5101  /* No interrupt completion for Set Address
                                        Long command. */

#define ADDR_CLR_ERR    0x5200  /* Failed to write Clear Address cmd. */
#define ADDCLR_INT_ERR  0x5201  /* No interrupt completion for Clear Address
                                        command. */

#define UP_LNK_STAT_ERR 0x5300  /* Failed to write Update Link Status cmd. */
#define UP_LNK_INT_ERR  0x5310  /* No interrupt completion for Update Link
                                        Status command. */

#define DIAG_MODE_ERR   0x5400  /* Failed to write Enter Diag Mode cmd. */
#define DIAG_INT_ERR    0x5401  /* No interrupt completion for Enter Diag Mode
                                        command. */

#define CONNECT_ERR     0x5500  /* Failed to write Connect cmd. */
#define CONNECT_INT_ERR 0x5501  /* No interrupt completion for Connect cmd. */

#define DISCONNECT_ERR  0x5600  /* Failed to write Disconnect cmd. */
#define DISCON_INT_ERR  0x5601  /* No interrupt completion for Disconnect
                                        command. */

#define RELEASE_DAG_ERR 0x5700  /* Failed to write Release Diag cmd. */
#define RELEASE_INT_ERR 0x5701  /* No interrupt completion for Release Diag
                                        command. */

#define CLASS_B_LOOP_ER 0x5800  /* Failed to write Class B Loop test cmd. */
#define CLASS_B_INT_ER  0x5801  /* No interrupt completion for Class B Loop test
                                        command. */

#define CLASS_A_LOOP_ER 0x5900  /* Failed to write Class A Loop test cmd. */
#define CLASS_A_INT_ER  0x5901  /* No interrupt completion for Class A Loop test
                                        command. */

/*
 * SIG defined variables
 */
#define SIG_SAV_ERR     0x6001  /* Error saving software signals. */
#define SIG_RES_ERR     0x6002  /* Error restoring software signals. */
#define SIG_OP_ERR      0x6003  /* Unknown signal op err - internal. */
#define SIG_REC_ERR     0x6010  /* TU received software signal - terminating. */

/*
 * Parity Error Codes.
 */
#define PAR_RD_SHRAM    0x6300  /* Parity error reading from Shared RAM. */
#define PAR_WR_SHRAM    0x6301  /* Parity error writing to Shared RAM. */

#define PAR_RD_FDDIRAM  0x6310  /* Parity error reading from FDDI RAM. */
#define PAR_WR_FDDIRAM  0x6311  /* Parity error writing to FDDI RAM. */

#define PAR_RD_NPDATA   0x6340  /* Parity error reading NP Bus Data Store */
#define PAR_WR_NPDATA   0x6341  /* Parity error writing NP Bus Data Store */

#define PAR_RD_NPPRG    0x6370  /* Parity error reading NP Bus Program Store */
#define PAR_WR_NPPRG    0x6371  /* Parity error writing NP Bus Program Store */

/*
 * General Error Codes
 */
#define MALLOC_ERR      0x9000  /* Malloc of memory failed - system error. */
#define CALC_ADDR_ERR   0x9500  /* Can not calculate address of register */
#define OPEN_MDD_ERR    0x9550  /* Open machine device driver failed */
#define MDD_IOCTL_ERR   0x9560  /* Call to machine device driver failed */
#define LED_OFF         0x9570  /* Connect LED is not on when it should be*/
#define LED_ON          0x9580  /* Connect LED is not off when it should be */
#define INVALID_ENTRY   0x9590  /* Invalid entry */
#define ILLEGAL_TU_ERR  0x9999  /* Illegal TU value - internal error */

#define SOFT_ERR        0xDEAD  /* Unexpected, internal software error. */

/*
 * Self test errors - Primary card FRU
 */

#define  TST_0_P_FRU  0x000A  /* Self test 0 failed; FRU = primary card */
#define  TST_1_P_FRU  0x100A  /* Self test 1 failed; FRU = primary card */
#define  TST_2_P_FRU  0x200A  /* Self test 2 failed; FRU = primary card */
#define  TST_3_P_FRU  0x300A  /* Self test 3 failed; FRU = primary card */
#define  TST_4_P_FRU  0x400A  /* Self test 4 failed; FRU = primary card */
#define  TST_5_P_FRU  0x500A  /* Self test 5 failed; FRU = primary card */
#define  TST_6_P_FRU  0x600A  /* Self test 6 failed; FRU = primary card */
#define  TST_7_P_FRU  0x700A  /* Self test 7 failed; FRU = primary card */
#define  TST_8_P_FRU  0x800A  /* Self test 8 failed; FRU = primary card */
#define  TST_9_P_FRU  0x900A  /* Self test 9 failed; FRU = primary card */

/*
 * Self test errors - Sec/prim card FRU
 */

#define  TST_5_SP_FRU  0x50BC  /* Self test 5 failed; FRU = secondary or primary                                                                  card */
#define  TST_8_SP_FRU  0x80BC  /* Self test 8 failed; FRU = secondary or primary                                                                  card */
#define  TST_A_SP_FRU  0xA0BC  /* Self test A failed; FRU = secondary or primary                                                                  card */

/*
 * Self test masks for FRU determination
 */

#define TST_5_MSK_LW 0x5011    /* Lower limit for possible secondary card FRU */
#define TST_5_MSK_UP 0x5018    /* Upper limit for possible secondary card FRU */

#define TST_8_MSK_0 0xFFFF     /* Possible secondary card FRU */
#define TST_8_MSK_1 0x8001     /* Possible secondary card FRU 1st group  */
#define TST_8_MSK_2L 0x8005    /* Lower limit for possible secondary card FRU
                                                                  2nd group  */
#define TST_8_MSK_2U 0x8008    /* Upper limit for possible secondary card FRU
                                                                  2nd group  */
#define TST_8_MSK_3L 0x800B    /* Lower limit for possible secondary card FRU
                                                                  3rd group  */
#define TST_8_MSK_3U 0x8011    /* Upper limit for possible secondary card FRU
                                                                  3rd group  */
#define TST_8_MSK_4L 0x8015    /* Lower limit for possible secondary card FRU
                                                                  4th group  */
#define TST_8_MSK_4U 0x8018    /* Upper limit for possible secondary card FRU
                                                                  4th group  */
#define TST_8_MSK_5L 0x801B    /* Lower limit for possible secondary card FRU
                                                                  5th group  */
#define TST_8_MSK_5U 0x801C    /* Upper limit for possible secondary card FRU
                                                                  5th group  */


/************************************************************************/
/* Adapter specific definitions for FDDI test units.                    */
/************************************************************************/
#define RULE_LEN      8
#define PREG_LEN      4
#define RETRIES_LEN   3
#define PATTERN_LEN  80
#define YES_NO_LEN    3
#define NETADD_LEN    6
#define FAIR_LEN      3
#define STREAM_LEN    3
#define PRE_LEN       5
#define NET_OFFSET 0x0B         /* offset into VPD for network address */
#define TU_NET_ID  0xAA
#define ADAPTER_SLEEP   5      /* for adapter to reset itself and run all
                                   internal diagnostics */

#define FDDI_NETID      0xAA    /* Netid for FDDI testing */
#define READ_ERROR      0x0100  /* Return code if read error occurs */
#define RD_ER_DATASTORE 0x0150  /* Return code if datastore read error occurs */
#define WRITE_ERROR     0x0200  /* Return code if write error occurs */
#define WR_ER_DATASTORE 0x0250  /* Return code if datastore write error
                                        occurs */
#define COMPARE12_ERROR 0x3012  /* Return code if compare error occurs */
#define COMPARE35_ERROR 0x3035  /* Return code if compare error occurs */
#define COMPARE46_ERROR 0x3046  /* Return code if compare error occurs */
#define COMPARE79_ERROR 0x3079  /* Return code if compare error occurs */
#define COMPARE80_ERROR 0x3810  /* Return code if compare error occurs */
#define COMPARE_RBC     0x3900  /* Return code if RBC compare error occurs */
#define COMPARE_SRESET  0x3950  /* Return code if Soft Reset compare error
                                        occurs */

#define BUFF64K_SIZE    0xFFF0  /* Two less than 64K elements in data buffer */
#define BUFF32K_SIZE    0x7FF0  /* 32K elements in data buffer */
#define BUFF16K_SIZE    0x3FF0  /* 16K elements in data buffer */
#define BUFF64_SIZE     64      /* 6K bytes in data buffer */
#define TOT_BYTES       512     /* Total Number of Bytes to Test */
#define SHOW_BYTES      24      /* Number of Bytes to Display while Testing */
#define DIAG_BYTES      22      /* Number of Bytes for Diagnostic results */
#define NP_BUS_DATA_OFF 0x1000  /* Offset for NP Bus Data Store area */

/************************************************************************/
/* Definitions for Initializing NP Bus Data Store locations for         */
/* FDDI RAM Buffer Tests.                                               */
/************************************************************************/

#define RBC_BYTES       4       /* Number of initialization bytes */
#define NUM_INSTRUCT    7       /* Number of Instructions needed in Data Store*/

#define SOFT_RESET_RBC  0x0000  /* Software reset of RBC */
#define RPR             0x0000  /* Read Pointer for receive frames */
#define WPX             0x0000  /* Write Pointer for transmit frames */
#define WPR             0xFFFF  /* Write Pointer for receive frames */
#define SAR             0x0000  /* Start Address for receive FIFO */
#define EAR             0xFFFF  /* End Address for receive FIFO */
#define RBC_MODE        0x3600  /* RBC Mode Register */

#define SOFT_RESET_OFF  0x0700  /* Offset for Software Reset of RBC */
#define RPR_OFFSET      0x0703  /* Offset for Read Pointer Receive location */
#define WPX_OFFSET      0x0705  /* Offset for Write Pointer Transmit location */
#define WPR_OFFSET      0x0706  /* Offset for Write Pointer Receive location */
#define SAR_OFFSET      0x0708  /* Offset for Start Address receive FIFO loc */
#define EAR_OFFSET      0x070A  /* Offset for End Address receive FIFO loc */
#define RBC_OFFSET      0x070C  /* Offset for RBC Mode Register */

/************************************************************************/
/* Definitions for System Interface Registers                           */
/************************************************************************/

#define NUM_SIF_REG     8       /* Number of System Interface Registers */

#define HSR_REG         0x0     /* Host Status Register */
#define HCR_REG         0x2     /* Host Command Register */
#define NS1_REG         0x4     /* Node Processor Status Register 1 */
#define NS2_REG         0x6     /* Node Processor Status Register 2 */
#define HSR_MK_REG      0x8     /* Host Status Register Mask */
#define NS1_MK_REG      0xA     /* Node Processor Status Register 1 Mask */
#define NS2_MK_REG      0xC     /* Node Processor Status Register 2 Mask */
#define ALISA_REG       0xE     /* ALISA Control Register */

/************************************************************************/
/* Definitions for commands used by FDDI                                */
/************************************************************************/

#define ON_LINE_DIAG_MODE       0x0D00  /* HCR Command for On Line Diag mode */
#define REL_ON_LN_DIAG          0x00FF  /* HCR Command Release On Line Diag  */
#define CLASS_B_LOOP_TEST       0x0000  /* HCR Command for Class B loop test */
#define CLASS_A_LOOP_TEST       0x0001  /* HCR Command for Class A loop test */
#define CCI_BIT                     0x0080      /* HSR Cmd Completion Interrupt bit */

#define EXTENDER_ABSENT         0x80EA  /* Extender card not installed */
#define NO_EXTENDER                 0x10EA      /* Extender card not installed */
#define EXTEND_VPD_ABSENT       0xA0EA  /* Extender card not installed */
#define TEST_NOT_INVOKED    0x9001  /* Host has not invoked test */

#define MASK_HSR_SW_BITS        0x80E6  /* HSR Mask to test SW bits */
#define MASK_ALISA_LDM_BIT      0x8000  /* ALISA Mask to test LDM bit */
#define MASK_NS1_HW_BITS        0x1FFF  /* NS1 Mask to clear Hw bits of reg */
#define MASK_NS2_HW_BITS        0x07FF  /* NS2 Mask to clear HW bits of reg */
#define MASK_POS2_CLEAR_RESET   0xFD    /* POS2 Mask to clear RESET bit */
#define MASK_POS2_RESET         0x02    /* POS2 Mask to set RESET bit */
#define MASK_POS2_INTR_RESET    0xF0    /* POS2 Mask to check for reset of
                                           Interrupt Level */
#define MASK_POS2_SET_D_D       0x08    /* POS2 Mask to set DOWNLOAD/DIAG */
#define MASK_POS2_CLEAR_D_D     0xF7    /* POS2 Mask to clear DOWNLOAD/DIAG */

/************************************************************************/
/* attribute structure used by microcode diagnostic test units.         */
/************************************************************************/
struct attr {
        char *attribute;
        char *value;
};

/************************************************************************/
/* standard structure used by manufacturing diagnostics.                */
/************************************************************************/
struct tucb_t
   {
        long tu,        /* test unit number   */
             loop,      /* loop test of tu    */
             mfg;       /* flag = 1 if running mfg. diagnostics, else 0  */

        long r1,        /* reserved */
             r2;        /* reserved */
   };

struct _fddi_htx
   {
        char rule_id[RULE_LEN+1];
        char retries[RETRIES_LEN+1];
        unsigned char network_address[NETADD_LEN];
        char fairness[FAIR_LEN+1];
        char streaming[STREAM_LEN+1];
        int pcard_flg;                  /* flag 0/1 use pcard */
        unsigned char pcard_enable[PREG_LEN+1]; /* enable reg 0xF value */
        unsigned char pcard_strobe[PREG_LEN+1]; /* strobe reg 0xE value */
        unsigned char pcard_ctrl[PREG_LEN+1];   /* ctrl reg 0xC value */
        struct htx_data *htx_sp;
   };

/************************************************************************/
/* definition of structure passed by BOTH hardware exerciser and        */
/* manufacturing diagnostics to "exectu()" function for invoking        */
/* test units.                                                          */
/************************************************************************/
#define TUTYPE struct _fddi_tu

struct _fddi_tu
   {
        struct tucb_t header;
        int mdd_fd;
        char slot;
        char bus_no;
        struct _fddi_htx fddi_s;
   };
