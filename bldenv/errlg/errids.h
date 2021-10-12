/* @(#)30       1.3  src/bldenv/errlg/errids.h, cmderrlg, bos412, GOLDA411a 9/20/94 14:37:43 */


/*
 * COMPONENT_NAME: CMDERRLG  system error logging facility
 *
 * FUNCTIONS:  header file for error ids
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ERRIDS
#define _H_ERRIDS

#include "sys/err_rec.h"


#define ERRID_OPMSG          0xaa8ab241 /* alertable operator message */
#define ERRID_SYSLOG         0xc6aca566 /* syslog msg routed to error log */
#define ERRID_CHECKSTOP      0x8ea094ff /* checkstop has occurred */
#define ERRID_DISKETTE_ERR1  0xa92ae715 /* diskette device overrun error */
#define ERRID_DISKETTE_ERR2  0x3a9c2352 /* diskette timeout error */
#define ERRID_DISKETTE_ERR3  0x0734da1d /* diskette media error */
#define ERRID_DISKETTE_ERR4  0xb73bc3cd /* diskette unknown error */
#define ERRID_DISKETTE_ERR5  0xefec314d /* temporary diskette PIO error */
#define ERRID_DISKETTE_ERR6  0x8fef9795 /* permanent diskette PIO error */
#define ERRID_MSLA_CLOSE     0xfed1497c /* Msla close error */
#define ERRID_MSLA_PROTOCOL  0x5ae97eaa /* Msla protocol error */
#define ERRID_MSLA_START     0x69221791 /* Msla start error */
#define ERRID_MSLA_IOCTL     0xfbd2b2b5 /* Msla ioctl error */
#define ERRID_MSLA_INTR      0xcbe25456 /* Msla intr error */
#define ERRID_MSLA_WRITE     0x8d2cc3aa /* Msla write error */
#define ERRID_MSLA_ADAPTER   0xa28b68bd /* Msla hardware error */
#define ERRID_SCSI_ERR1      0x0502f666 /* permanent SCSI adapter hardware error */
#define ERRID_SCSI_ERR2      0xc14c511c /* temporary SCSI adapter hardware error */
#define ERRID_SCSI_ERR3      0x5cc986a0 /* permanent unknown SCSI adapter sfw error */
#define ERRID_SCSI_ERR4      0x13881423 /* temporary unknown SCSI adapter sfw error */
#define ERRID_SCSI_ERR5      0x4edef5a1 /* permanent unknown driver error */
#define ERRID_SCSI_ERR6      0x52db7218 /* temporary unknown driver error */
#define ERRID_SCSI_ERR7      0x038f2580 /* permanent unknown system error */
#define ERRID_SCSI_ERR8      0x038f2580 /* temporary unknown system error */
#define ERRID_SCSI_ERR9      0x54e423ed /* potential data loss condition */
#define ERRID_SCSI_ERR10     0x0ba49c99 /* temporary SCSI bus error */
#define ERRID_DISK_ERR1      0x21f54b38 /* CD-ROM, disk, or read/write optical medi */
#define ERRID_DISK_ERR2      0xa668f553 /* CD-ROM, disk, or read/write optical hard */
#define ERRID_DISK_ERR3      0x35bfc499 /* adapter-detected CD-ROM, disk, or read/w */
#define ERRID_DISK_ERR4      0x1581762b /* CD-ROM, disk, or read/write optical reco */
#define ERRID_DISK_ERR5      0x3cff4028 /* unknown CD-ROM, disk, or read/write opti */
#define ERRID_BADISK_ERR1    0x384e0485 /* badisk recovered soft read error */
#define ERRID_BADISK_ERR2    0x770f9606 /* badisk hard read error */
#define ERRID_BADISK_ERR3    0x627a4f55 /* badisk soft equipment check */
#define ERRID_BADISK_ERR4    0xcf4781d3 /* badisk hard equipment check */
#define ERRID_BADISK_ERR5    0xd2b9b5a9 /* badisk attachment error */
#define ERRID_BADISK_ERR8    0x2d3bddd6 /* badisk seek fault */
#define ERRID_CDROM_ERR1     0x30f182a4 /* cdrom permanent media error */
#define ERRID_CDROM_ERR2     0x34fc3203 /* cdrom temporary media error */
#define ERRID_CDROM_ERR3     0x1588ddd9 /* cdrom permanent hardware error */
#define ERRID_CDROM_ERR4     0xa38e8cf2 /* cdrom temporary hardware error */
#define ERRID_CDROM_ERR5     0x91d6c4f8 /* adapter-detected permanent cdrom error */
#define ERRID_CDROM_ERR6     0x8dd34341 /* adapter-detected temporary cdrom error */
#define ERRID_CDROM_ERR7     0x173d5818 /* cdrom device recovered error */
#define ERRID_CDROM_ERR8     0xad682624 /* cdrom unknown error */
#define ERRID_MEMORY         0x0e017ed1 /* memory parity error */
#define ERRID_TAPE_ERR1      0x4865fa9b /* tape media error */
#define ERRID_TAPE_ERR2      0x476b351d /* tape hardware/aborted command error */
#define ERRID_TAPE_ERR3      0x22006523 /* recovered error threshold exceeded */
#define ERRID_TAPE_ERR4      0x5537ac5f /* tape SCSI adapter detected error */
#define ERRID_TAPE_ERR5      0xffe2f73a /* tape unknown error */
#define ERRID_TAPE_ERR6      0xb617e928 /* tape drive needs cleaning */
#define ERRID_DMA_ERR        0x00530ea6 /* DMA unknown error, see device's error lo */
#define ERRID_INTR_ERR       0xd1e101cd /* INTERRUPT unknown error */
#define ERRID_INTRPPC_ERR    0xdadf69e4 /* INTERRUPT unknown error */
#define ERRID_MISC_ERR       0x7ab881d9 /* INTERRUPT:IO bus timeout or Channel Chec */
#define ERRID_EPOW_SUS       0x74533d1a /* EPOW: Loss of primary power. */
#define ERRID_EPOW_RES       0x0ec7e7e5 /* EPOW: Resumption of primary power. */
#define ERRID_SRC            0xe18e984f /* System Resource Controler */
#define ERRID_RCMERR         0xeb5f98b2 /* Rendering Context Mgr error */
#define ERRID_LFTERR         0xe85c5c4c /* Low Function Terminal error */
#define ERRID_CONSOLE        0x89b52aa5 /* Console device driver error */
#define ERRID_DUMP           0x6297ca97 /* error during init of primary dump device */
#define ERRID_ERRLOG_OFF     0x192ac071 /* errdemon turned off */
#define ERRID_ERRLOG_ON      0x9dbcfdee /* errdemon turned on */
#define ERRID_PPRINTER_ERR1  0xdfc508f5 /* pprinter unknown error */
#define ERRID_MPQP_QUE       0x84ee0148 /* MPQP Queue Unavailable */
#define ERRID_MPQP_XFTO      0x7a9e20bb /* Transmit Failsafe Timeout  */
#define ERRID_MPQP_X21TO     0xe61501a6 /* X.21 Timeout */
#define ERRID_MPQP_DSRTO     0x44cb9ece /* DRS Going On Timeout */
#define ERRID_MPQP_DSROFFTO  0xa741ad52 /*  DSR already on for switched line  */
#define ERRID_MPQP_CTSTO     0x56816728 /*  CTS going On Timeout  */
#define ERRID_MPQP_DSRDRP    0x038f3117 /*  DSR Dropped During Transmit  */
#define ERRID_MPQP_CTSDRP    0x038f3117 /*  CTS Dropped During Transmit  */
#define ERRID_MPQP_RCVERR    0x10c6ced6 /*  MPQP Error On Receive Data */
#define ERRID_MPQP_ASWCHK    0xad917fba /*  Adapter Software Checksum Error  */
#define ERRID_MPQP_ADPERR    0xb312955a /*  Adapter Not Present or not functioning  */
#define ERRID_MPQP_IPLTO     0xe4f5f86e /* Adapter IPL Timeout  */
#define ERRID_MPQP_X21CECLR  0x8c0353cb /*  X.21 Unexpected Clear During Call Estab */
#define ERRID_MPQP_X21DTCLR  0xe252fe92 /*  X.21 Unexpected Clear During Data Trans */
#define ERRID_MPQP_X21CPS    0xc88d3dd8 /*  X.21 Call Progress Signal  */
#define ERRID_MPQP_BFR       0x2b76062d /*  MPQP -  Memory Resources Unavailable  */
#define ERRID_MPQP_XMTUND    0x1b1647df /*  Transmit Underrun  */
#define ERRID_MPQP_RCVOVR    0xdb451f82 /*  Receive Overrun    */
#define ERRID_CMDLVM         0x4523caa9 /* Logical Volume Manager  */
#define ERRID_COM_PERM_PIO   0xe2109f7a /* permanent PIO error detected by driver */
#define ERRID_COM_TEMP_PIO   0x5416ce51 /* temporary PIO error detected by driver */
#define ERRID_TTY_BADINPUT   0xc0073bb4 /* ttyinput routine returned with error cod */
#define ERRID_TTY_PARERR     0x43d4adce /* Parity/Framing error on input */
#define ERRID_TTY_OVERRUN    0x9d30b78e /* Receiver over-run on input */
#define ERRID_TTY_TTYHOG     0x0873cf9f /* ttyhog over-run */
#define ERRID_RS_PROG_SLIH   0x0cfad921 /* Unconfiguration process could not find s */
#define ERRID_TTY_PROG_PTR   0xe2b9e02b /* Pointer to private structure is null */
#define ERRID_RS_PROG_IOCC   0x0cacec26 /* IOCC is not configured at unconfig time. */
#define ERRID_COM_CFG_ADPT   0xf5458763 /* adapter already configured */
#define ERRID_COM_CFG_NADP   0x3ec3c657 /* adapter not present */
#define ERRID_COM_CFG_BUSI   0xfde6a5a1 /* bad bus id */
#define ERRID_COM_CFG_BUST   0x06abb2eb /* bad bus type */
#define ERRID_COM_CFG_IFLG   0x5114c792 /* bad interrupt flags */
#define ERRID_COM_CFG_ILVL   0x92a72c14 /* bad interrupt level */
#define ERRID_COM_CFG_INTR   0xfe6a2d60 /* bad interrupt priority */
#define ERRID_COM_CFG_MNR    0x2a7392a2 /* bad minor number */
#define ERRID_COM_CFG_BUSID  0x4287a984 /* bus id out of range */
#define ERRID_COM_CFG_DMA    0xbe7f0c5d /* dma level conflict */
#define ERRID_COM_CFG_DEVA   0xbc8f0bbb /* devswadd failed */
#define ERRID_COM_CFG_PIN    0xda244dca /* pincode failed */
#define ERRID_COM_CFG_DEVD   0x29975223 /* devswdel failed */
#define ERRID_COM_CFG_UNPIN  0x7f0052c6 /* unpincode failed */
#define ERRID_COM_CFG_SLIH   0x544ff289 /* i_init of slih failed */
#define ERRID_COM_CFG_PORT   0xb216db3e /* port already configured */
#define ERRID_COM_CFG_RESID  0x804c1878 /* resid not correct */
#define ERRID_COM_CFG_UIO    0x4cebe931 /* uiomove failed */
#define ERRID_COM_CFG_UNK    0x7993098b /* Unknown adapater type */
#define ERRID_RS_MEM_EDGE    0x66c3412b /* can't allocate edge structure */
#define ERRID_RS_MEM_IOCC    0x22f7b47b /* can't allocate iocc structure */
#define ERRID_RS_MEM_EDGEV   0xe2a4ec26 /* can't allocate edge vector */
#define ERRID_COM_MEM_SLIH   0x29202ca2 /* can't allocate slih structure */
#define ERRID_RS_MEM_PVT     0xe97374ff /* can't allocate private structure */
#define ERRID_RS_PIN_EDGE    0xe7d0fe3f /* can't pin edge structure */
#define ERRID_RS_PIN_EDGEV   0xd41b92e8 /* can't pin edge vector */
#define ERRID_RS_PIN_IOCC    0x3a58abe2 /* can't pin iocc structure */
#define ERRID_COM_PIN_SLIH   0x60d5349f /* can't pin slih structure */
#define ERRID_RS_8_16_ARB    0x1e629bb1 /* 8/16 arbitration register failure */
#define ERRID_RS_BAD_INTER   0xb63e9c5e /* interrupt from non-existant port */
#define ERRID_EU_CFG_HERE    0xdbf56911 /* adapter already configured */
#define ERRID_EU_CFG_GONE    0xd9ee4ac1 /* adapter already unconfigured */
#define ERRID_EU_CFG_BUSY    0xb5982183 /* in use, can't uninitialized */
#define ERRID_EU_CFG_NADP    0xc9f4ee17 /* adapter not present */
#define ERRID_EU_CFG_NPLN    0x861365e7 /* can't access planar */
#define ERRID_EU_DIAG_ACC    0x25d74748 /* in use, no destructive diagnostics */
#define ERRID_EU_DIAG_MEM    0xa853f9ce /* can't allocate diagnostic buffer */
#define ERRID_LION_BOX_DIED  0x3f86401a /* 64port concentrator died */
#define ERRID_LION_HRDWRE    0x1251b5b7 /* Failure in adapter hardware */
#define ERRID_LION_MEM_LIST  0xd84b1c5b /* can't allocate ttyp_t list */
#define ERRID_LION_MEM_ADAP  0x9c7fe90b /* can't allocate adap structure */
#define ERRID_LION_PIN_ADAP  0xbe7e5290 /* can't pin ADAP structure */
#define ERRID_LION_BUFFERO   0x50ca5315 /* Buffer over-run on input */
#define ERRID_LION_UNKCHUNK  0xbf6d9219 /* Unknown error code in chunk */
#define ERRID_LION_CHUNKNUMC 0xb76a0a99 /* Bad value in chunk numc */
#define ERRID_C327_START     0x1a660730 /*  C327 Start Error  */
#define ERRID_C327_INTR      0xc89de914 /*  C327 Interrupt Error  */
#define ERRID_TOK_ADAP_ERR   0xcfff77bd /* Potential data loss condition */
#define ERRID_X25_ALERT5     0x30911e21 /* Modem Failure detected, DCD and/or DSR e */
#define ERRID_X25_ALERT7     0x2f65d788 /* Local modem auto call unit is not respon */
#define ERRID_X25_ALERT8     0xd3b0ecbf /* X.21 failure detected in the X.25 subsys */
#define ERRID_X25_ALERT9     0x0375dfc2 /* Frame type w received */
#define ERRID_X25_ALERT10    0x813e4b9a /* Frame type x received */
#define ERRID_X25_ALERT11    0x6f7d7290 /* Frame type y received */
#define ERRID_X25_ALERT12    0x59792439 /* Frame type z received */
#define ERRID_X25_ALERT13    0xb73a1d33 /* Frame type w sent */
#define ERRID_X25_ALERT14    0x81922194 /* Frame type x sent */
#define ERRID_X25_ALERT15    0x6fd1189e /* Frame type y sent */
#define ERRID_X25_ALERT16    0x59d54e37 /* Frame type z sent */
#define ERRID_X25_ALERT17    0xc70e1e46 /* Frame retry limit reached */
#define ERRID_X25_ALERT18    0x7a9c71e6 /* Unexpected disconnect frame received */
#define ERRID_X25_ALERT19    0x974cc901 /* Disc mode frame received during link act */
#define ERRID_X25_ALERT31    0x7873ce72 /* A RESET_INDICATION packet was received b */
#define ERRID_X25_ALERT21    0x5529e45b /* A CLEAR_INDICATION packet was received b */
#define ERRID_X25_ALERT22    0xea388e60 /* A RESTART_INDICATION packet was received */
#define ERRID_X25_ALERT23    0xe6599c95 /* A RESET_REQUEST packet was sent to the n */
#define ERRID_X25_ALERT24    0x618db24a /* A CLEAR_REQUEST packet was sent to the n */
#define ERRID_X25_ALERT25    0x01f2d769 /* A START_REQUEST packet was sent to the n */
#define ERRID_X25_ALERT26    0x1642b5a7 /* T20 expired prior to receipt of RESTART_ */
#define ERRID_X25_ALERT27    0x86922ccd /* T20 expired prior to receipt of RESET_CO */
#define ERRID_X25_ALERT28    0xdd2201a9 /* T21 expired prior to CALL_CONNECT or CLE */
#define ERRID_X25_ALERT29    0xe6784bc4 /* T23 expired prior to receipt of CLEAR_CO */
#define ERRID_X25_ALERT30    0x3503bdba /* DIAGNOSTIC packet received-DTE,protocol  */
#define ERRID_X25_ALERT32    0x1fd6c71a /* CLEAR_INDICATE packet was sent by DCE */
#define ERRID_X25_ALERT33    0x0d5c1698 /* RESET_INDICATION packet was sent by DCE */
#define ERRID_X25_ALERT34    0x103f1912 /* RESTART_INDICATION packet was sent by DC */
#define ERRID_X25_ALERT35    0xc0514a3f /* RESTART_REQUEST was received by the DCE */
#define ERRID_X25_ALERT36    0xc2b80bfb /* T10 expired (DCE) prior to receipt RESTA */
#define ERRID_X25_ALERT37    0x794a4421 /* T12 expired (DTE) prior to receipt RESET */
#define ERRID_X25_ALERT38    0xac47fa8a /* T11 expired (DTE) prior to receipt CALL_ */
#define ERRID_X25_ALERT39    0x150acba4 /* T13 expired (DCE) prior to receipt CLEAR */
#define ERRID_X25_ADAPT      0x57797644 /* X.25 adapter card is not present  */
#define ERRID_X25_IPL        0x74e0cea8 /* X.25 adapter time-out during IPL */
#define ERRID_X25_CONFIG     0x836a2443 /* X.25 configuration error */
#define ERRID_X25_UCODE      0xc9a0c741 /* X.25 microcode error */
#define ERRID_WHP0001        0x0a667c32 /* Memory Allocation error */
#define ERRID_WHP0002        0xe4ef0a90 /* Message Queue Recieve Error */
#define ERRID_WHP0003        0x7ff45ec0 /* IPC Message Queue Send Error */
#define ERRID_WHP0004        0x5cfbfa4a /* Hcon IPC Message Queue Creation Error */
#define ERRID_WHP0005        0x4f515df0 /* Hcon IPC Message Queue Creation Error */
#define ERRID_WHP0006        0x24247fb2 /* Hcon IPC Message Queue Stat Error */
#define ERRID_WHP0007        0xb7164fa8 /* Hcon IPC Message Queue Set Error */
#define ERRID_WHP0008        0x4224ba8c /* Hcon IPC Message Queue Remove Error */
#define ERRID_WHP0009        0xc580ded6 /* Hcon IPC Shared Segment Attachment Error */
#define ERRID_WHP0010        0xc1423e5b /* Hcon IPC Shared Segment Detachment Error */
#define ERRID_WHP0011        0xa5417864 /* Hcon IPC Shared Segment Allocation Error */
#define ERRID_WHP0012        0x2b60dd24 /* Hcon IPC Shared Segment Stat Error */
#define ERRID_WHP0013        0x1d5588be /* Hcon IPC Shared Segment Set Error */
#define ERRID_WHP0014        0xa80659f3 /* Hcon IPC Shared Segment Remove Error */
#define ERRID_ISI_PROC       0x71248bf5 /* instuction storage interrupt : processor */
#define ERRID_DSI_PROC       0x20faed7f /* data storage interrupt : processor */
#define ERRID_DSI_SCU        0xb8892a14 /* data storage interrupt : storage control */
#define ERRID_DSI_IOCC       0x27c1efff /* data storage interrupt : IOCC */
#define ERRID_DSI_SLA        0x76c9d063 /* data storage interrupt : SLA */
#define ERRID_MACHINECHECK   0x4a29d32a /* machine check */
#define ERRID_KERNEL_PANIC   0x225e3b63 /* Kernel Panic! */
#define ERRID_DOUBLE_PANIC   0x358d0a3e /* Double Panic! */
#define ERRID_EXCHECK_DMA    0xa9844fee /* external check - dma error */
#define ERRID_EXCHECK_SCRUB  0xf7e70b81 /* external check - memory scrub error */
#define ERRID_NB1            0xd3f26ec3 /* RUN OUT OF PSB IN INTERRUPT HANDLER */
#define ERRID_NB2            0x3c19f251 /* UNKNOWN ROUTER COMMAND OPTION */
#define ERRID_NB3            0x4c2bda1e /* OUT OF IOB */
#define ERRID_NB4            0x70559cae /* DEVIOCTL - DISABLE SAP FAILED */
#define ERRID_NB5            0x9e45396d /* SAP ALREADY CLOSED */
#define ERRID_NB6            0x484f5514 /* BUFFER OVERFLOW */
#define ERRID_NB7            0x773d6c8e /* INACTIVITY WITHOUT TERMINATION */
#define ERRID_NB8            0xbab1383b /* INACTIVITY ENDED */
#define ERRID_NB9            0x0a940597 /* UNDEFINED RESULT INDICATOR */
#define ERRID_NB10           0xb7f0ec53 /* OUT OF IOB IN OPEN LINK STATION */
#define ERRID_NB11           0xc92f456f /* OUT OF IOB IN OPEN LINK STATION COMPLETI */
#define ERRID_NB12           0xda80b2d4 /* DEVIOCTL (OPEN LINK STATION) FAILED */
#define ERRID_NB13           0x289590ae /* OUT OF IOB IN CONNECT LINK STATION */
#define ERRID_NB14           0x273fe0ac /* DEVIOCTL-CONTACT FAILED */
#define ERRID_NB15           0x804055eb /* CONNECT LINK STATION OUT OF RETRIES */
#define ERRID_NB16           0x4a4fbe2b /* CONNECT LINK STATION FAILED, RETRY */
#define ERRID_NB17           0x51f9313a /* DEVIOCTL (HALT LINK STATION) FAILED */
#define ERRID_NB18           0x504b04d3 /* OUT OF IOB IN ENABLE SAP */
#define ERRID_NB19           0xf734b194 /* DEVIOCTL (ENABLE SAP) FAILED */
#define ERRID_NB20           0x03acd152 /* DEVIOCTL-WRITE FAILED */
#define ERRID_NB21           0x1ccd189f /* ERROR OPERATION RESULT FROM LLC */
#define ERRID_NB22           0x13c8a0aa /* UNKNOWN USER-SAP-CORRELATOR RECEIVED IN  */
#define ERRID_NB23           0x419d40c2 /* UNKNOWN USER-SAP-CORRELATOR IN RECEIVE-F */
#define ERRID_NB24           0x24dcdba8 /* INVALID CORRELATOR FROM OPEN SAP/LS RESP */
#define ERRID_NB25           0xf5345aab /* DEVSWADD SVC FAILED */
#define ERRID_NB26           0x233e36d2 /* DEVSWDEL SVC FAILED */
#define ERRID_NB27           0x9844042c /* OPEN PENDING ON TERMINATION */
#define ERRID_NB28           0x21d5b396 /* UNKNOWN CONFIG OPTIONS */
#define ERRID_NB29           0x79fed1ed /* MALLOC SVC FAILED */
#define ERRID_NB30           0xaff4bd94 /* PALLOC SVC FAILED */
#define ERRID_ATE_ERR1       0x36c3328b /* no pacing character received */
#define ERRID_ATE_ERR2       0x36c3328b /* too many transmission errors */
#define ERRID_ATE_ERR3       0x36c3328b /* no acknowledgement from receiving site */
#define ERRID_ATE_ERR4       0x36c3328b /* receiving site is not ready */
#define ERRID_ATE_ERR5       0x36c3328b /* sending site is not sending */
#define ERRID_ATE_ERR6       0x3a67afe0 /* no carrier signal */
#define ERRID_ATE_ERR7       0xf6e3c547 /* checksum error */
#define ERRID_ATE_ERR8       0xf6e3c547 /* sector received twice */
#define ERRID_ATE_ERR9       0xf6e3c547 /* incorrect sector received */
#define ERRID_ATE_ERR10      0xf6e3c547 /* sector could not be verified */
#define ERRID_PROGRAM_INT    0xdd11b4af /* program interrupt */
#define ERRID_FLPT_UNAVAIL   0xc8f22e8e /* floating point unavailable */
#define ERRID_PSLA001        0xb598ecb3 /* PSLA graphic unit or cable error  */
#define ERRID_PSLA002        0x087468d0 /* PSLA device driver or microcode error  */
#define ERRID_PSLA003        0xbf3f8438 /* PSLA host link error  */
#define ERRID_CORE_DUMP      0xde0a8dc4 /* Core dump caused by signal */
#define ERRID_ACCT_OFF       0x2e7c3756 /* Accounting off */
#define ERRID_EU_BAD_ADPT    0x2c7ce30e /* Illegal adapter in expansion unit */
#define ERRID_LVM_SWREL      0x83e4c0b2 /* SW relocation successful */
#define ERRID_LVM_HWREL      0x5dfeadcb /* HW relocation successful */
#define ERRID_LVM_HWFAIL     0xc67e7d0f /* HW relocation failed */
#define ERRID_LVM_BBFAIL     0xdbf832ff /* Error during BB relocation */
#define ERRID_LVM_BBRELMAX   0x84917289 /* Max retries during BB relocation */
#define ERRID_LVM_BBEPOOL    0x0e37fe58 /* BB relocation pool is empty */
#define ERRID_LVM_BBDIRERR   0x18a546cd /* Error during BB dir IO op */
#define ERRID_LVM_BBDIRBAD   0xd62aafd8 /* BB directory corrupted */
#define ERRID_LVM_BBDIRFUL   0x54b73180 /* BB directory is full */
#define ERRID_LVM_BBDIR90    0x684b0e5c /* BB directory is over 90% full */
#define ERRID_CFGMGR_OPTION  0x91fda5e4 /* Configuration manager invalid option */
#define ERRID_CFGMGR_FATAL_DB 0xf9171b5c /* Configuration manager fatal database pro */
#define ERRID_CFGMGR_NONFATAL_DB 0x7ef0a4ff /* Configuration manager nonfatal database  */
#define ERRID_CFGMGR_MEMORY  0x2cf9ab6c /* Configuration manager not enough memory */
#define ERRID_CFGMGR_PROGRAM_NF 0xe6cdbcfc /* Configuration manager program not found */
#define ERRID_CFGMGR_LOCK    0x6b0b47fa /* Configuration manager could not acquire  */
#define ERRID_CFGMGR_CHILD   0xf81946d8 /* Configuration manager child process erro */
#define ERRID_DUMP_STATS     0x5d66bbc4 /* System dump statistics */
#define ERRID_SYSDUMP_SYMP   0x3573a829 /* Symptom data for a system dump */
#define ERRID_MEM1           0x3d858a1b /* Memory card pair failure */
#define ERRID_MEM2           0x069db93b /* Memory SIMM failure */
#define ERRID_MEM3           0x77e0148a /* Memory card failure */
#define ERRID_PGSP_KILL      0xc5c09ffa /* Process killed due to low paging space */
#define ERRID_NLS_MAP        0x28935927 /* NLS mapping problem */
#define ERRID_NLS_BADMAP     0xe7e2e3e9 /* bad NLS map removed */
#define ERRID_LVM_MISSPVADDED 0x1a2e7186 /* A physical volume has been defined as mi */
#define ERRID_LVM_MISSPVRET  0x9359f226 /* Physical volume is now active. */
#define ERRID_LVM_SA_WRTERR  0x0c1ec9fa /* Failed to write Volume Group Status area */
#define ERRID_LVM_SA_PVMISS  0xcbe1d1a5 /* A physical volume has been defined as mi */
#define ERRID_LVM_SA_STALEPP 0xb188909a /* Physical partition marked stale. */
#define ERRID_LVM_SA_FRESHPP 0x1ac82784 /* Physical partition marked active. */
#define ERRID_LVM_SA_QUORCLOSE 0x91f9700d /* Quorum lost, volume group closing. */
#define ERRID_LVM_MWCWFAIL   0x1a9465a3 /* Mirror write consistency cache write fai */
#define ERRID_SDM_ERR1       0xbaecc981 /* Software or microcode errors */
#define ERRID_SDC_ERR1       0xa9ed5bb6 /* Controller/DASD Link Error */
#define ERRID_SDC_ERR2       0xf438e969 /* Controller Hardware Error */
#define ERRID_SDC_ERR3       0xb1462f15 /* Recovered Controller Hardware Error */
#define ERRID_SDA_ERR1       0xb135ae8b /* Adapter Hardware Error Condition */
#define ERRID_SDA_ERR2       0x0733ffa0 /* Recovered Adapter Hardware Error */
#define ERRID_SDA_ERR3       0xfec31570 /* permanent unknown system error */
#define ERRID_SDA_ERR4       0xb18287f3 /* temporary unknown system error */
#define ERRID_SYS_RESET      0x1104aa28 /* system reset button pushed */
#define ERRID_REBOOT_ID      0x2bfa76f6 /* system shutdown by user */
#define ERRID_TMSCSI_READ_ERR 0x868921f2 /* non-retriable hdw err receiving data */
#define ERRID_TMSCSI_CMD_ERR 0xee18df01 /* non-retriable hdw err sending cmd */
#define ERRID_TMSCSI_UNRECVRD_ERR 0xfbf0bfc1 /* unrecovered error sending command */
#define ERRID_TMSCSI_RECVRD_ERR 0x98f39a90 /* recovered error sending command */
#define ERRID_TMSCSI_UNKN_SFW_ERR 0x72cbc436 /* device driver detected sfw err */
#define ERRID_CORRECTED_SCRUB 0xa6bad8e6 /* Corrected memory via scrubbing */
#define ERRID_ACPA_INITZ     0x17a1f1e4 /* ACPA Host Independent Initialization fai */
#define ERRID_ACPA_LOAD      0x1a1d42f9 /* Failed loading microcode */
#define ERRID_ACPA_MEM       0xbb5c513f /* Failed pinning memory */
#define ERRID_ACPA_INTR1     0x835c5977 /* Interrupt handler registration failed */
#define ERRID_ACPA_INTR2     0x18b25e18 /* Unexpected interrupt */
#define ERRID_ACPA_INTR3     0xe79a3c09 /* Invalid interrupt */
#define ERRID_ACPA_INTR4     0x89c695bb /* Interrupt timed out */
#define ERRID_ACPA_IOCTL1    0x53920b1f /* Unsupported audio control request */
#define ERRID_ACPA_IOCTL2    0x506e5213 /* Invalid ACPA ioctl request */
#define ERRID_ACPA_UCODE     0xbba1d78b /* ACPA failed loading microcode */
#define ERRID_EXCHECK_RSC    0x9a335282 /* external check - dma error */
#define ERRID_CAT_ERR1       0xd7dddc46 /* microcode abnormally terminated */
#define ERRID_CAT_ERR2       0x4ab56573 /* microcode load failed */
#define ERRID_CAT_ERR3       0x9060a2f8 /* no mbuf available */
#define ERRID_CAT_ERR4       0x80f672ff /* no pinned memory available */
#define ERRID_CAT_ERR5       0xd080e08d /* permanent channel adapter hardware error */
#define ERRID_CAT_ERR6       0x9060a2f8 /* 370 Parallel Channel Adapter system reso */
#define ERRID_CAT_ERR7       0xbe910c7f /* 370 Parallel Channel Adapter driver reso */
#define ERRID_CAT_ERR8       0x5d1f16fa /* 370 Parallel Channel Adapter detected a  */
#define ERRID_REPLACED_FRU   0xbe42630e /* FRU replacement in the field */
#define ERRID_VCA_INITZ      0x04b1c8c0 /* VCA Host independent initialization fail */
#define ERRID_VCA_MEM        0xa84c681b /* failed pinning memory */
#define ERRID_VCA_INTR1      0x904c6053 /* interrupt handler registration failed */
#define ERRID_VCA_INTR2      0x270cb959 /* Unexpected interrupt */
#define ERRID_VCA_INTR3      0xd824db48 /* Invalid interrupt */
#define ERRID_VCA_INTR4      0x9ad6ac9f /* interrupt timed out */
#define ERRID_VCA_IOCTL1     0xe70473e7 /* Unsupported video control request */
#define ERRID_VCA_IOCTL2     0xddbca0ee /* Invalid VCA ioctl request */
#define ERRID_TTY_INTR_HOG   0x345707f5 /* temporary interrupt hog detected by driv */
#define ERRID_PSIGDELIVERY   0xa2a97a5f /* Program Interrupt During Signal Delivery */
#define ERRID_HARDWARE_SYMPTOM 0xcb36dff0 /* Symptom data reported by hardware */
#define ERRID_SOFTWARE_SYMPTOM 0x20746462 /* Symptom data reported by software */
#define ERRID_INIT_RAPID     0x3a30359f /* init command respawning too rapidly  */
#define ERRID_INIT_UTMP      0xe47e212e /* init failed write of utmp entry */
#define ERRID_INIT_CREATE    0x80a357f9 /* init cannot create utmp */
#define ERRID_INIT_OPEN      0x5ce03b80 /* init cannot open */
#define ERRID_INIT_UNKNOWN   0x4f3e9630 /* init unknown err logged -- check detail  */
#define ERRID_LOG_PRE_WRAP   0xf2936fc5 /* Error log file premature wrap */
#define ERRID_LOG_REG_WRAP   0xd1e21ba3 /* Error log file regular wrap */
#define ERRID_LOG_NVRAM      0x8c7d9b28 /* Error demon nvram processing failures */
#define ERRID_BAD_BUF_ENTRY  0x18592686 /* Error demon annunciate bad buffer entry */
#define ERRID_ODM_CHANGE_SWSERVAT 0x27ea672a /* Errdemon cannont change SWservAT in ODM */
#define ERRID_GRAPHICS       0xe85c5c4c /* Graphics subsystem errors */
#define ERRID_LOST_EVENTS    0xa39f8a49 /* error logging buffer lost events error */
#define ERRID_LOW_MBUFS      0xe931ba9f /* Low Mbufs */
#define ERRID_MID_UCODE      0x9198533d /* Microcode CRC test failed */
#define ERRID_MID_PIPE1      0x88bc536d /* PIPE processor 1 not responding */
#define ERRID_MID_PIPE2      0x5f0bc7c4 /* PIPE processor 2 not responding */
#define ERRID_MID_PIPE3      0x984f565b /* PIPE processor 3 not responding */
#define ERRID_MID_PIPE4      0xcd5f7617 /* PIPE processor 4 not responding */
#define ERRID_MID_BLAST      0xf07d3c3d /* BLAST processor not responding */
#define ERRID_MID_BLASTB     0x662ab9a3 /* BLAST (backend) processor not responding */
#define ERRID_MID_SW         0x2adff53f /* Datastream (software) error */
#define ERRID_BUMP_ERROR_TABLES 0x3fa9ce30 /* bump errors detected */
#define ERRID_BUMP_INT       0xc960be94 /* Recoverable memory parity error */

#endif /* _H_ERRIDS */

