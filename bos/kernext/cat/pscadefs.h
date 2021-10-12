/* @(#)63	1.7  src/bos/kernext/cat/pscadefs.h, sysxcat, bos411, 9428A410j 2/24/92 09:41:53 */
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: pscadefs.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_PSCADEFS
#define _H_PSCADEFS

/********************************************************************/
/*                                                                  */
/* PSCADEFS - "C" common DEFINES and structure used in              */
/*             PSCA programs.                                       */
/*                                                                  */
/********************************************************************/
/* PSCA Version 1.0 Memory Layout (Offsets from shared RAM base)    */
/*                                                                  */
/*  3FE00-3FFFF  - Transmit Buffer FIFO (128 buffers for transmits) */
/*  3FC00-3FDFF  - Receive Buffer FIFO  (128 buffers for receives)  */
/*  3F800-3FBFF  - Reserved - Controls (FIFO, Interrupts, etc)      */
/*  3F400-3F7FF  - Garbage Transfer Area (ex. SMOD commands)        */
/*  3E000-3F3FF  - CLAW Map / VTAM Header data area                 */
/*  3D000-3DFFF  - Sense Adapter Data Area                          */
/*  3C000-3CFFF  - Sense ID Data Area                               */
/*  08000-3BFFF  - Data buffer area                                 */
/*  03000-07FFF  - Ctl/Rsp Buffer Area  ( 80 256 Byte buffers)      */
/*  02000-02FFF  - Adapter Control FIFO (256 entry command FIFO)    */
/*  01000-01FFF  - Adapter Response FIFO(256 entry status FIFO)     */
/*  00C00-00FFF  - Control Buffer FIFO  (256 buffers for commands)  */
/*  00800-00BFF  - Response Buffer FIFO (256 buffers for status)    */
/*  00400-007FF  - Trace Area for PSCA Adapter code                 */
/*  00000-003FF  - Status Information Area (described below)        */
/********************************************************************/

/*************************************************************************/
/*****     PSCA Memory Map Definitions                               *****/
/*************************************************************************/
#define XMTBFIFO 0x3FE00  /* Transmit Buffer fifo  (offset from base)   */
#define RCVBFIFO 0x3FC00  /* Receive Buffer fifo   (offset from base)   */
#define GARBXFER 0x3F400  /* Garbage Transfer Area (offset from base)   */
#define CLAWMAPS 0x3E000  /* CLAW Map Data Area    (offset from base)   */
#define SENSEAD  0x3D000  /* Sense Data Area       (offset from base)   */
#define SENSEID  0x3C000  /* Sense ID Data Area    (offset from base)   */
#define DATABUFS 0x08000  /* Data Buffer Area      (offset from base)   */
#define CNTLBUFS 0x03000  /* Control Buffer Area   (offset from base)   */
#define CNTLFIFO 0x02000  /* Adapter control fifo  (offset from base)   */
#define RESPFIFO 0x01000  /* Adapter response fifo (offset from base)   */
#define CTLBFIFO 0x00C00  /* Control Buffer fifo   (offset from base)   */
#define RSPBFIFO 0x00800  /* Response Buffer fifo  (offset from base)   */
#define TRACDATA 0x00400  /* Trace Data Area       (offset from base)   */
#define STATDATA 0x00000  /* Status Data Area      (offset from base)   */

#define SHARESZ  0x40000  /* Size of shared RAM                         */
#define REALSZ   0x3F800  /* Size of shared RAM                         */
#define DATASZ   0x34000  /* Size of Data Buffer Area                   */

/*************************************************************************/
/*****     PSCA String Constants                                     *****/
/*************************************************************************/
#define HARDNAME  "IBM PSCA"  /* Adapter name */
#define HARDVERS  "1120"      /* Hardware version number */
#define SOFTNAME  "PSCA3088"  /* Software name */
#define SOFTVERS  "1120"      /* Software version number */
#define PSCAID    0x92FE      /* channel adapter id */


#ifdef NOTUSED
/*************************************************************************/
/*****     PSCA Base Memory Address Definitions                      *****/
/*************************************************************************/
static ulong basemem[15] = {0x00800000,0x00f40000,0x00f80000,0x00fc0000,
                            0x01000000,0x01100000,0x01200000,0x01300000,
                            0x01400000,0x01500000,0x01600000,0x01700000,
                            0xffc00000,0xffd00000,0xffe00000};

/*************************************************************************/
/*****     PSCA Interrupt Definitions                                *****/
/*************************************************************************/
static uchar intlvls[8] = {4,5,6,7,9,10,11,14};
#endif

/*************************************************************************/
/*                                                                       */
/*              PSCA FIFO Structures and Definitions                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*****     PSCA Control and Response FIFOs to/from 80386             *****/
/*************************************************************************/
typedef struct {
    uchar         command;    /* Control Command                          */
    uchar         cmdmod;     /* Command modifier                         */
    uchar         correl;     /* Command correlator (to correlate acks)   */
    uchar         subchan;    /* Subchannel to which command is directed  */
    uchar         chanstat;   /* Channel Status returned - resets/errors  */
    uchar         ccw;        /* Command Control Word                     */
    uchar         origcmd;    /* Command being ACKed or which caused error*/
    uchar         retcode;    /* Return code/Error code for command       */
    uchar         data[2];    /* Command specific data                    */
    ushort        length;     /* length of control buffer                 */
    ulong        buffer;     /* Offset of buffer from start of shared RAM*/
    } CTLFIFO;

/*************************************************************************/
/*****     PSCA "Free Buffer FIFOs"                                  *****/
/*************************************************************************/
typedef struct {
    ulong        buffer;     /* Offset to buffer from start of shared RAM*/
    } BUFFIFO;

/*************************************************************************/
/*****     Definitions for Transmit/Receive Buffer Controls (386)    *****/
/*************************************************************************/
#define    XBUFDATA  0x3FC00 /* Offset to read from Transmit Buffer FIFO */
#define    RBUFDATA  0x3FC00 /* Offset to write to Receive Buffer FIFO   */
#define    XBUFSTAT  0x3FD80 /* Offset to read Transmit Buffer status    */
#define    RBUFSTAT  0x3FD80 /* Offset to read Receive Buffer status     */
#define    XBUFRESV  0x3FD00 /* Offset to reserve a Transmit Buffer      */
#define    RBUFRESV  0x3FC80 /* Offset to reserve a Receive Buffer       */

#define    XBUFREMP  0x0001  /* Transmit Buffer Reserved Empty           */
#define    XBUFEMPT  0x0004  /* Transmit Buffer FIFO Empty               */
#define    RBUFRFUL  0x0010  /* Reveive Buffer Reserved Full             */
#define    RBUFFULL  0x0040  /* Receive Buffer FIFO Full                 */

#define    PARERRLO  0x0100  /* Adapter Parity Error Byte 0 (0 = error)      */
#define    PARERRHI  0x0400  /* Adapter Parity Error Byte 1 (0 = error)      */
#define    CHANACTV  0x1000  /* Channel drivers active  (1 = enabled)    */
#define    IRQACTIV  0x4000  /* Adapter generating interrupt (0 = active)    */

/*************************************************************************/
/*****     Definitions for Interrupt and DMA Controls (386)          *****/
/*************************************************************************/
#define    STARTDMA  0x3F800 /* Offset for starting DMA to/from PSCA     */
#define    UCIRQ186  0x3F806 /* Offset for uchannel to int 186 (uchar)    */
#define    RSTUCIRQ  0x3F804 /* Offset to reset 186 int of microch (uchar)*/
#define    INTRSTAT  0x3FF00 /* Offset to read Interrupt status          */

/*************************************************************************/
/*****     Control Command Definitions                               *****/
/*************************************************************************/
#define PSCANOP		0x00	/* No operation */

#define PSCASETA	0x01	/* Set adapter parameters */
#define PSCARTA		0x02	/* Retrieve adapter parameters */
#define PSCASETS	0x03	/* Set subchannel parameters (recv size,prot) */
#define PSCARTS		0x04	/* Retrieve subchannel parameters */
#define PSCALDCU	0x05	/* Load Control Unit Table */
#define PSCARTCU	0x06	/* Retrieve Control Unit Table */

#define PSCASTRT	0x21	/* Start the subchannel specified */
#define PSCASTOP	0x22	/* Stop the subchannel specified */
#define PSCARSTA	0x23	/* Reset/Flush all subchannels */
#define PSCARSTS	0x24	/* Reset/Flush the subchannel specified */
#define PSCACTCRNR	0x30	/* CTC ready/not ready */
#define PSCASBAS	0x31	/* Set Basic Mode */
#define PSCASEXT	0x32	/* Set Extended Mode */
#define PSCASUSP	0x33	/* suspend/resume accepting 370 write cmds */
#define PSCAVTAM	0x34	/* VTAM header mode start/stop */

/* claw commands */

#define PSCACLREQ       0x38    /* Claw connection request */
#define PSCACLCON       0x39    /* Claw connection confirm */
#define PSCACLDISC      0x3A    /* Claw disconnect */
#define PSCASYSVAL       0x99   /* Claw system validate */

#define PSCAXBUF	0x41	/* Transfer data buffer */
#define PSCAXLST	0x42	/* Transfer list of data buffers */
#define PSCAXCCW	0x43	/* Transfer channel control word */
#define PSCAWEOF	0x44	/* CTC Write End Of File Command */
#define PSCAUNST	0x45	/* Give unsolicited status to channel */
#define PSCARBUF	0x50	/* Return Unused FIFO buffer(s) */
#define PSCAXDRP	0x51	/* Drop transfer in progress (xmit pending) */

#define PSCACDBA	0x61	/* Change adapter debug level */
#define PSCACTRA	0x62	/* Change adapter trace level */
#define PSCADIAG	0x65	/* Run on-board diagnostics */
#define PSCAWRPM	0x66	/* Write to program (186) memory */
#define PSCARTPM	0x67	/* Retrieve from program (186) memory */

/*
** Notifications received from PSCA
*/
#define PSCAACK		0x81	/* Acknowledgement - command completed */
#define PSCAERR		0x82	/* Error detected in command */
#define PSCAPRNT	0x83	/* Print information contained in buffer */
#define PSCAABRT	0x84	/* PSCA microcode aborted */
#define PSCAXFLU	0x85	/* Flush current transfer (CCW cancelled) */
#define PSCABUFAV	0x86	/* Buffer has been added to xmit buffer fifo */
#define PSCADMAC	0x87	/* DMA complete notification */

#define PSCACTCRNR	0x30	/* CTC ready/not ready transition notification */
#define PSCABASIC	0x31	/* Basic mode set notification */
#define PSCAEXT		0x32	/* Extended mode set notification */
#define PSCAVTAM	0x34	/* VTAM header mode start/stop notification */
#define PSCACMD		0x43	/* Command received from S/370 notification */

#define	PSCAONLN	0x90	/* Adapter online to S/370 notification */
#define PSCAOFFLN	0x91	/* Adapter offline to S/370 notification */


/* Claw notification */

#define PSCACLRESP       0x98   /* Adapter to MCA Claw response */
#define PSCASYSVAL       0x99   /* Claw system validate */
#define PSCACLREQ       0x38    /* Claw connection request */
#define PSCACLCON       0x39    /* Claw connection confirm */
#define PSCACLDISC      0x3A    /* Claw disconnect */

/*************************************************************************/
/*****     Command Modifier Definitions                              *****/
/*************************************************************************/
#define    MODACK    0x01  /* Command should be acknowledged when done   */
#define    MODCHAIN  0x02  /* Command not complete, another piece follows*/
#define    MODATTN   0x04  /* Give ATTN Interrupt with this command      */
#define    MODPREP   0x08  /* PREPARE CCW preceded this command          */
#define    MODHDR    0x10  /* Header included in data                    */
#define    MODGRP    0x20  /* apply the command to the whole group       */

/*************************************************************************/
/*****     Channel Status Definitions                                *****/
/*************************************************************************/
#define    CSTATIFD   0x01  /* Interface disconnect                       */
#define    CSTATSYSR  0x02  /* System Reset                               */
#define    CSTATSELR  0x04  /* Selective Reset                            */
#define    CSTATUCTO  0x08  /* Unit Check - Tag Timeout                   */
#define    CSTATUCBP  0x10  /* Unit Check - Bad Parity                    */
#define    CSTATITMO  0x20  /* Interface Timeout                          */

/*************************************************************************/
/*****     Return/Error Codes from 186 operations                    *****/
/*************************************************************************/
#define    RETGOOD   0x00  /* No problems, all is well                   */

/* Error codes from Main/Set-up */
#define    RETMATSK  0x01  /* Unable to attach task                      */
#define    RETMHUNG  0x02  /* Working task appears to be hung            */
#define    RETMICMD  0x03  /* PSCAMAIN received an invalid command       */
#define    RETMVERS  0x07  /* Hardware version mismatch                  */
 
/* Return codes from Control commands (General) */
#define    RETCONLN  0x20  /* Unable to execute command - adp/sub online */
#define    RETCOFLN  0x21  /* Unable to execute command - adp/sub offline*/
#define    RETCUNAV  0x22  /* subchannel unavailable                     */
#define    RETCNOSB  0x23  /* Invalid number of subchannels specified    */
#define    RETCSBNO  0x24  /* Invalid subchannel number                  */
#define    RETCCUTB  0x25  /* Invalid control unit table reference       */
#define    RETCICMD  0x26  /* PSCACTL received an invalid command        */
#define    RETCUNSP  0x27  /* command not supported at present           */
#define    RETCFOFF  0x28  /* PSCAACK to PSCASTOP if offline was forced
                              to occur due to a timeout instead of a normal
                              channel sequence (sdm00003) */
#define    RETCOFLP  0x29  /* PSCAERR, command rejected due to offline pending
                              condition (sdm00003) */
 
/* Return codes from Set Adapter / Set Subchannel Parameters */
#define    RETCBDEF  0x40  /* data buffer redefinition not allowed       */
#define    RETMXSIZ  0x41  /* Invalid transmit buffer size               */
#define    RETMRSIZ  0x42  /* Invalid receive buffer size                */
#define    RETMBNO   0x43  /* Invalid number of data buffers             */
#define    RETCTMAT  0x44  /* Set Subchan Parms - Tables don't match mode*/
 
/* Return codes from Load/Retrieve Control Unit Table */
#define    RETCCUMX  0x60  /* All Control Unit table slots used          */
#define    RETCCUDP  0x61  /* Attempt to replace table, REPLACE not spec */
#define    RETCCUNL  0x62  /* Attempt to retrieve table, not loaded      */
#define    RETCCUBD  0x63  /* Control Unit headers invalid, rejected     */
#define    RETCCUCR  0x64  /* Control Unit had bad CRC                   */
 
/* Return codes from Non-Maskable Interrupts */
#define    RETCNMII  0x80  /* Owed initial status (command reject)       */
#define    RETCNMIS  0x81  /* Received an NMI interrupt - SCB Not Ready  */
#define    RETCNMIT  0x82  /* Received an NMI interrupt - Timeout        */
#define    RETCNMIP  0x83  /* Received an NMI interrupt - Parity         */
#define    RETCNMIC  0x84  /* Received an NMI interrupt - Command Out    */
#define    RETCNMIE  0x85  /* Owed ending status                         */
 
/* Return codes from Other commands */
#define    RETCXOVF  0xA0  /* PSCAXLST contained too many buffers (SCBs) */
#define    RETCNTBL  0xA1  /* cu table not loaded, cannot start subchan  */
#define    RETCRSTP  0xA2  /* Reset pending, command discarded           */
#define    RETCNCLW  0xA3  /* CLAW command, subchan not defined for CLAW */
#define    RETCNLNK  0xA4  /* CLAW connect, no more links for subchan    */
#define    RETCBLID  0xA5  /* CLAW command, bad link id                  */
#define    RETCSYSN  0xA6  /* CLAW command, system name validation failed*/
#define    RETCNACK  0xA7  /* Data buffer requiring ACK discarded        */
#define    RETCVTER  0xA8  /* A request occured to run in VTAM hdr mode, but
                              the max subch allowed (VTAMSLOTS) is running (sdm00001) */
#define    RETCRDSP  0xA9  /* A 2nd PSCAVTRR cmd was sent before 1st ended */
#define    RETCRDXP  0xB0  /* A PSCAVTRR cmd was sent before Xmit ended */
#define    RETCVTBD  0xB1  /* Less than 8 bytes recieved while in VTAM
                              data transfer mode */
#define    RETCVTBC  0xB2  /* Count in header did not match the number of
                              bytes received (sdm00017) */
#define    RETCBVER  0xB3  /* Not compatable CLAW version number (sdm00025) */
#define    RETCBHMR  0xB4  /* Host Max Receive value in CLAW SYS VAL is less
                              than transmit buffer size (sdm00025) */
 
/* Return codes from Channel commands */
#define    RETHICMD  0xC0  /* PSCACHAN received an invalid/unexpected cmd*/
#define    RETHATOV  0xC1  /* ATTENTION list overflow, command rejected  */
#define    RETHOFLN  0xC2  /* WRITE occured when subchan offline (sdm00014)*/
#define    RETHRD4A  0xC3  /* A READ received 4A status (sdm00023) */
 
#define    RETHRSTO  0xD0  /* Reset Ack Timeout occurred                 */
#define    RETHWACT  0xDD  /* HW has not released the subchannel (sdm0000e)*/
#define    RETHWREL  0xDE  /* HW has "finally" released the subch (sdm0000e)*/
 
/* Return codes from Utility Routines */
#define    RETUNBUF  0xE0  /* PRINTF could not get a buffer              */
 
/*************************************************************************/
/*****     PSCA "Status Information Area" of shared RAM              *****/
/*************************************************************************/
typedef struct {
    uchar         hardname[8];   /* Adapter name (ASCII)                  */
    uchar         hardvers[4];   /* Adapter hardware version number       */
    uchar         softname[8];   /* Name of Microcode Program on adapter  */
    uchar         softvers[4];   /* Adapter software version number       */
    uchar         operlvl;       /* Operational status (on startup)       */
    uchar         ready;         /* 186 is ready for work                 */
    uchar         intps2;        /* interrupt PS/2 (PS/2 wants interrupts)*/
    uchar         intbuf;        /* interrupt PS/2 to deliver xmit buffers*/
    uchar         tracind;       /* Current index into Trace data area    */
    uchar         bugfix;        /* Required due to IBM C Compiler bug    */
    uchar         cutblmap[8];   /* map showing tables loaded             */
    uchar         codeload;      /* Code has been loaded to Data RAM      */
    uchar         chanspd;       /* Channel speed adapter set to use      */
    uchar         reserveb[24];  /* Reserved for future use               */
    ushort        cutables;      /* number of Control Unit Tables loaded  */
    ushort        sbchact;       /* number of subchannels currently active*/
    ushort        xbuflen;       /* Length of transmit buffers            */
    ushort        rbuflen;       /* Length of receive buffers             */
    ushort        cbuflen;       /* Length of command/status buffers      */
    ushort        xbufno;        /* Number of transmit data buffers       */
    ushort        rbufno;        /* Number of receive data buffers        */
    ushort        cbufno;        /* Number of command buffers             */
    ushort        sbufno;        /* Number of response buffers            */
    ushort        xbuffisz;      /* no. of slots in Transmit Buffer fifo  */
    ushort        rbuffisz;      /* no. of slots in Receive Buffer fifo   */
    ushort        cntlfisz;      /* no. of slots in Control fifo          */
    ushort        respfisz;      /* no. of slots in Response fifo         */
    ushort        cbuffisz;      /* no. of slots in Control Buffer fifo   */
    ushort        sbuffisz;      /* no. of slots in Response Buffer fifo  */
    ushort        debug;         /* adapter debug level                   */
    ushort        trace;         /* adapter trace level                   */
    ushort        errdgblk[8];   /* error block for severe errors/diagnos */
    ushort        maxlstsz;      /* Maximum number buffers in PSCAXLST    */
    ushort        adapflgs;      /* flags field from set adap config cmd  */
    ushort        xbufadds;      /* Transmit buffers added to FIFO        */
    ushort        xbufsubs;      /* Transmit buffers deleted from FIFO    */
    ushort        xbufwait;      /* PS/2 waiting for buffers              */
    ushort        rbufadds;      /* Receive buffers added to FIFO         */
    ushort        rbufsubs;      /* Receive buffers deleted from FIFO     */
    ushort        reservew[32];  /* Reserved for future use               */
    ulong        reservel[4];   /* reserved for future use               */
    ulong        xmitbase;      /* offset/base of transmit buffer pool   */
    ulong        recvbase;      /* offset/base of receive buffer pool    */
    ulong        datasize;      /* size in bytes of data buffer area     */
    ulong        databufs;      /* offset of data buffer area            */
    ulong        cntlbufs;      /* offset of command/status buffer area  */
    ulong        xbuffifo;      /* offset of Transmit Buffer fifo        */
    ulong        rbuffifo;      /* offset of Receive Buffer fifo         */
    ulong        cntlfifo;      /* offset of Command fifo                */
    ulong        respfifo;      /* offset of Status fifo                 */
    ulong        cbuffifo;      /* offset of Command Buffer fifo         */
    ulong        sbuffifo;      /* offset of Status Buffer fifo          */
    ulong        tracdata;      /* offset of Trace Data Area             */
    uchar         status[256];   /* status of each subchannel             */
    uchar         opmode[256];   /* operating mode of subchannels         */
    uchar         cutype[256];   /* control unit types for subchannels    */
    } STATAREA;


/*************************************************************************/
/*****     Subchannel status bit definitions                         *****/
/*************************************************************************/
#define    SUBAVAIL  0x80  /* Subchannel Available for use (in config)   */
#define    SUBONLIN  0x40  /* Subchannel online                          */
#define    SUBMULTI  0x20  /* Subchannel in multiple subchannel group    */
#define    SUBVTAM   0x02  /* Subchannel is in VTAM header mode          */
#define    SUBSUSPD  0x01  /* Subchannel is WRITE suspended              */


/*************************************************************************/
/*****     PSCA "Trace Information Area" of shared RAM               *****/
/*************************************************************************/
typedef struct {
    ulong        tracdata[256]; /* Trace data area                       */
    } TRACAREA;


/*************************************************************************/
/*****   Set Adapter Parameters and Configuration (buffer structure) *****/
/*************************************************************************/
typedef struct {
    uchar         subchan;    /* Subchannel address                       */
    uchar         cutype[2];  /* Control unit table no. (basic, extended) */
    uchar         reserve;    /* Reserved for future use                  */
    } SUBCHID;

typedef struct {
    uchar         speed;      /* Channel speed to use                     */
    uchar         reserve[3]; /* Reserved for future use                  */
    ushort        xmitsz;     /* Transmit buffer size                     */
    ushort        recvsz;     /* Receive buffer size                      */
    ushort        xmitno;     /* Number of transmit buffers               */
    ushort        recvno;     /* Number of receive buffers                */
    ushort        flags;      /* Flags                                    */
    ushort        nosubs;     /* Number of subchannels                    */
    SUBCHID      subid[256]; /* Subchannel identification information    */
    } ADAPCFG;

/* channel speed settings */
#define    DCI       0x00    /* DC Interlock                             */
#define    STRM19    0x01    /* Streaming 1.9 MBytes/second              */
#define    STRM27    0x02    /* Streaming 2.7 MBytes/second              */
#define    STRM34    0x03    /* Streaming 3.4 MBytes/second              */
#define    STRM45    0x04    /* Streaming 4.5 MBytes/second              */

/* flags defined for adapcfg */
#define    SUDSPOOL  0x0001  /* SUDS buffer pool arrangement requested   */


/*************************************************************************/
/*****     Set Subchannel Parameters Command Buffer Structure        *****/
/*************************************************************************/
typedef struct {
    ushort        subset;     /* Number of associated subchannels         */
    uchar         specmode;   /* Special mode type - flags                */
    uchar         shrtbusy;   /* Short Busy Status                        */
    uchar         startde;    /* Give Unsolicited Device End at start-up  */
    uchar         flags;      /* Special purpose flags                    */
    } SUBPARMS;

/* flags defined for subparms flags */
#define    SUBSDEF   0x01    /* new definition included                  */
#define    SUBSRSET  0x02    /* reset subchannels in group to default    */


/*************************************************************************/
/*****     Set Subchannel Parameter Command Definitions              *****/
/*************************************************************************/
#define    SIMPLEX   0x01  /* Single subchannel operating independently  */
#define    DUPLEX    0x02  /* Two subchannels operating together         */
#define    BSYONLY   0x00  /* Short Busy alone                           */
#define    BSYSM     0x01  /* Short Busy + Status modifier               */
#define    BSYSMCUE  0x03  /* Short Busy + Status modifier + CU end      */
#define    STARTDE   0x01  /* Start subchannel with unsolicited DE       */


/*************************************************************************/
/*****     Operating Modes of Subchannel(s)                          *****/
/*************************************************************************/
#define    IBM8232   0x00  /* Default IBM 8232-like operations           */
#define    CLAW      0x01  /* CLAW mode                                  */
#define    SFCTC     0x02  /* Store and Forward Channel to Channel       */
#define    PK3215    0x04  /* Printer/Keyboard 3215                      */
#define    DATAATTN  0x40  /* Give ATTN with all data going to 370       */
#define    RSETFLSH  0x80  /* On resets, flush Transmit side, await ACK  */


/*************************************************************************/
/*****   Control Unit Table Load (buffer structure)                  *****/
/*************************************************************************/
typedef struct {
    uchar    cusense[8];      /* Control Unit Sense ID                    */
    uchar    cmdtbl[2048];    /* Actual Control Unit table                */
    ushort   crc;             /* Checksum for this information            */
    } CUTBFILE;

#define    REPLACE   0x01  /* Replace existing table (data[1] in command)*/
#define    KEEPSPD   0x10  /* Do not set speed in table, keep as it is   */

/*************************************************************************/
/*****   Transmit List of Data Buffers (buffer structure)            *****/
/*************************************************************************/
typedef struct {
    ushort        length;     /* Length of data in this buffer            */
    ushort        reserve;    /* Reserved - alignment                     */
    ulong        buffer;     /* Buffer address (offset)                  */
    } XMTINFO;

typedef struct {
    XMTINFO      buf[32];    /* Transmit list                            */
    } XDATLIST;

#endif _H_PSCADEFS
