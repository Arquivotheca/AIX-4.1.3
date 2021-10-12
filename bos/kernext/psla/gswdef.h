/* @(#)08	1.10  10/12/93 10:32:04 */
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/* PURPOSE:     Defines for the device driver.                          */
/*                                                                      */
/*;                                                                     */
/*;bb 020290    Testing LOCK_SHORT .                                    */
/*;bb 020290    back to NDELAY lock.                                    */
/*;bb 030290    back to LOCK_SHORT.                                     */
/*;bb 032090    Added ParityError and IPFnotSet defines.                */
/*;bb 040990    Added PIOParityError, NO_UNLOCK, DO_UNLOCK.             */
/*;                                                                     */
/************************************************************************/

#define HYDRA_DEVS  16
#define SOLO_DEVS   3
/*----------------------------------------------------------------------*/
/* TO COMPILE FOR HYDRA, ONLY 'NumDevSupp' IS CHANGED.                  */
/*----------------------------------------------------------------------*/
#ifdef HYDRA
#define NumDevSupp   HYDRA_DEVS         /* HYDRA: 16 addresses          */
#else
#define NumDevSupp   SOLO_DEVS          /* SOLO: graf, porta,portb      */
#endif HYDRA

#define BIT0     0x00000080             /*                              */
#define BIT1     0x00000040             /*                              */
#define BIT2     0x00000020             /*                              */
#define BIT3     0x00000010             /*                              */
#define BIT4     0x00000008             /*                              */
#define BIT5     0x00000004             /*                              */
#define BIT6     0x00000002             /*                              */
#define BIT7     0x00000001             /*                              */
#define CCWWS    0x05                   /* write structured             */
#define CCWSRMA  0x2b                   /* SRMA                         */
#define CCWRMA   0x0a                   /* RMA                          */
#define CCWSETM  0xf7                   /* set mode                     */
#define CCWRMI   0x0e                   /* RMI                          */
#define CCWSWMA  0xfb                   /* SWMA                         */
#define CCWWMA   0x09                   /* WMA                          */
#define DLBAREA  0xfffe                 /*                              */
#define CCWLST           0x0100         /*a ccw list is present in ccb  */
#define SETUP_SENSE_AREA 0x0200         /*setup unsolicited sense area  */
#define LINK1            0x0001         /*means more ce's in ce block   */

#define WS_IO         1                 /* gswio called from ioctl      */
#define RW_IO         2                 /* gswio called from read/write */
#define LC_IO         3                 /* gswio called from lcw        */
#define FP_IO         4                 /* gswio called from ioctl fpgi */
#define G_ASYNC_IO    TRUE              /* type of io                   */
#define G_SYNC_IO     FALSE             /* type of io                   */
#define MAX_MULTI_IO  2                 /* concurrent outstanding io's  */
#define RMI_MAX       3                 /* rmi returns 3 bytes          */
#define QLEN          30                /* queue has 30 elements        */
#define MAX_INP       8                 /* number of input devices      */
#define ALL_MAX_INP   11                /* num of input devs plus 3 intr*/
#define MAX_INTR      10                /* num of slots in intr table   */
#define OPNX_INTRS    (KNO_5080 + KLINK_SW + KNOT_TO_READY)
#define PRIORTY       ((PZERO + 8) | PCATCH )
#define ASYNC_ERR     0xfc              /* byte0, bits0-5 of sense data */
#define RD_EXCESS     0xffff8000        /* read cannot exceed 32k-1     */
#define WT_EXCESS     0xffff0000        /* write cannot exceed 64k-1    */
#define ZERO_LOW2BITS 0xfffffffc        /* used to zero out low 2 bits  */

#define MAXCE        5                  /* maximum number of cmd element*/
#define MAXUINT      15                 /*15 unsolicited interrupts + 16*/
#define MAX_UCODE_SIZE 65536            /* ucode <= 64k                 */
#define SENSE_MODULO (MAXUINT + 20)     /* number of uns sense blocks   */
#define ALPHAMOD     0x00f0             /*                              */
#define GRAPHMOD     0x00fe             /*                              */
#define VPD_LEN      256                /* Vital Product Data length    */
#define MSLA_MAX_TIMEOUTS 6             /* number of timeouts           */

#define QS_STATE     0x04
#define RETURN_DATA  0x02
#define MORE_DATA    0x01
#define SENSE_PRESENT 0x00              /* sense present                */

/*----------------------------------------------------------------------*/
/* 'err_type's for gerr_log.                                            */
/*----------------------------------------------------------------------*/
/* software ones */
#define SW_ERRVALS_START  0x30          /* starting sw error values     */
#define SW_ERRVALS_END    0x4F          /* endinf sw error values       */
#define CMD_QOVERRUN      0x30          /* cmd queue overrun            */
#define UNATT_DEV         0x31          /* unattached async device intr */
#define BAD_UNSOL_DEV_REQ 0x32          /* bad unsolicited device reques*/
#define BAD_MODE          0x33
#define NO_SENSE_AREA     0x34          /* no sense  - area not defined */
#define SENSE_AREA_FULL   0x35          /* no sense  - area full        */
#define NO_SENSE_TIMEOUT  0x36          /* no sense  - timeout occurred */
#define NO_QEL            0x37          /*                              */
#define RSET_TIMEOUT      0x38          /* reset timeout occurred       */
#define STOPSTRT_TIMEOUT  0x39          /* stop-strt timeout occurred   */
#define PROGRPT_TIMEOUT   0x3A          /* prog prt timeout occurred    */
#define SCF_TIMEOUT       0x3B          /* scf flag timeout occurred    */
#define SCF_FLAG_HUNG     0x3C          /* serious error. adapter stuck.*/
#define BAD_ICC           0x3D          /* bad cap->icc code            */
#define BAD_SIO_SENSE     0x3E          /* sense returned from bad sio  */
#define BAD_ASY_SENSE     0x3F          /* sense returned from bad async*/

/* hardware ones */
#define HW_ERRVALS_START  0x50          /* starting hw error values     */
#define HW_ERRVALS_END    0x6F          /* endinf hw error values       */
#define NO_RMI_DATA       0x50          /* no data with the rmi         */
#define MSLA_NO_5080      0x51          /* msla- no 5080 present        */
#define MSLA_NO_RQ        0x52          /* msla- no response to req conf*/
#define FATAL_ERROR       0x53          /* fatal error occurred         */
#define FATAL_IO_ERR      0x54          /* fatal io error               */
#define BAD_UNDODMA       0x55          /* bad undo_dma call, fatal io  */
#define IO_TIMEOUT        0x58          /* io timeout occurred          */

/* link failure   */
#define MSLA_NO_HOST      0x70          /* msla- host link went down    */

/* non-error ones */
#define NW_ERRVALS_START  0x80          /* starting non-error values    */
#define NW_ERRVALS_END    0x9F          /* ending non-error values      */
#define MSLA_LM           0x80          /* msla entered Link Monitor mod*/
#define NOT_CONFIGURED    0x81          /* device not configured        */

/*----------------------------------------------------------------------*/
/* 'err_reason's for gerr_log.                                          */
/*----------------------------------------------------------------------*/
#define NoLdaDev        15              /* lda has no minor device      */
#define UnExpdAe_De     16              /* unexpected AE or DE          */
#define UnMtchLda       17              /* unmatching lda               */
#define NoCmdOwner      18              /* cmd active without an owner  */
#define NoDeFailCmd     19              /* no DE on failed command      */
#define AdaptErr        20              /* ipl diagnostic error         */
#define StatErr         21              /* stat counter error           */
#define UnitChekOn      22              /* Unit Check bit in comm area  */
					/* has come on                  */
#define ParityError     23              /* adptr Parity error occurred  */
#define IPFnotSet       24              /* IPF flag not set by ucode    */
#define PIOParityError  25              /* PIO Parity error occurred    */

/*----------------------------------*/
/* defines for device status byte   */
/*----------------------------------*/

#define C_ATTN     0x80                 /* attention                    */
#define C_ST_MDFR  0x40                 /* status modifier              */
#define C_CTLU_END 0x20                 /* control unit end             */
#define C_BUSY     0x10                 /* busy                         */
#define C_CH_END   0x08                 /* channel end                  */
#define C_DEV_END  0x04                 /* device end                   */
#define C_U_CHK    0x02                 /* unit check                   */
#define C_U_EXCPT  0x01                 /* unit exception               */

/*----------------------------------*/
/* defines for cu status byte       */
/*----------------------------------*/

#define C_PCI        0x80               /* pgm-controlled interruption  */
#define C_LENGTH     0x40               /* incorrect length             */
#define C_PGM_CHK    0x20               /* pgm check                    */
#define C_PROT_CHK   0x10               /* protection check             */
#define C_CHDATA_CHK 0x08               /* channel-data check           */
#define C_CHCTL_CHK  0x04               /* channel-control check        */
#define C_INTFC_CHK  0x02               /* interface-control check      */
#define C_CHN_CHK    0x01               /* chaining check               */

#define CCWSENSE 0x04                   /* sense                        */
#define CCWNOP   0x03                   /* nop                          */

/*----------------------------------*/
/* defines for g_flags              */
/*----------------------------------*/

#define GSW_OPEN   0x0001               /* device in use by a PM        */
#define GSW_ATTN   0x0002               /* device has asserted attention*/

/* use these defines to refer to the ccw flag byte or the separate bits */

#define CCWCD   0x80                    /* chain data                   */
#define CCWCC   0x40                    /* command chain                */
#define CCWSILI 0x20                    /* suppress incorrect length    */
#define CCWSKIP 0x10                    /* suppress data transfer       */
#define CCWPCI  0x08                    /* cause program control interru*/
#define CCWIDA  0x04                    /* indirect data address        */
#define CCWSUS  0x02                    /* suspend/resume, 3033 extensio*/
#define CCWZERO 0x01                    /* reserved, must be zero       */

#define START_TC   0x7127
#define STOP_TC    0x7107
#define LD_BLNK_TC 0x71f5
#define LD_LINE_TC 0x71f1

#ifndef INTRQ_SIZE
#define INTRQ_SIZE 8172                 /* ***** 8k -20              ****/
#endif INTRQ_SIZE

#ifndef MAX_DATA
#define MAX_DATA 512                    /* max size of intr read data   */
#endif MAX_DATA


#define G_DVR_CLASS     0x02            /* errlog class for 5080        */
#define G_DVR_SUBCLASS  0x04            /* errlog subclass              */


/*----------------------------------------------------------------------*/
/* Assorted bits.                                                       */
/*----------------------------------------------------------------------*/

#define SYS5080_MODE    0x0000
#define FPGI_MODE       0x0010
#define SYS_MODE        0x0020
#define UnknownDevmode   4              /* openx ext devmode value bad  */
#define FPGI_BUFINFO     1
#define AIX_CLOSE        2
#define SETMODE_5080    0x00fe          /* 2 byte value for set mode    */
#define SETMODE_FPGI    0x0100          /* 2 byte value for set mode    */

#define DCMask          0x80            /* data chaining mask (in LD)   */
#define CCMask          0x40            /* cmd  chaining mask (in LD)   */
#define SLIMask         0x20            /* sli           mask (in LD)   */
#define SKIPMask        0x10            /* skip          mask (in LD)   */
#define AEMask          0x80            /* adapter end   mask (in LD)   */
#define DEMask          0x40            /* device end    mask (in LD)   */
#define DQMask          0x20            /* deque pend    mask (in LD)   */
#define IPMask          0x01            /* intrpt pending mask          */

#define ZeroTopNib      0x0FFFFFFF      /* AND with an adr to zero top  */
#define IoDelayAdr      0x80E7          /* io delay address             */

#define NO_UNLOCK       0               /* dont do UN_LOCK in SET_PARITY*/
#define DO_UNLOCK       1               /* do UN_LOCK in SET_PARITY     */

/*----------------------------------------------------------------------*/
/* Bits for status bytes 1 and 2 in struct stat1, struct stat2 (commarea*/
/*----------------------------------------------------------------------*/

#define ATTN            0x80            /* attention                    */
#define STAT_MOD        0x40            /* status modifier              */
#define ADAPT_READY     0x20            /* adapter ready                */
#define BUSY            0x10            /* busy                         */
#define AE              0x08            /* adapter end                  */
#define DE              0x04            /* device end                   */
#define UC              0x02            /* unit check                   */
#define UE              0x01            /* unit exception               */



/*----------------------------------------------------------------------*/
/* scf and smf literals for r2-msla comm area                           */
/*----------------------------------------------------------------------*/

#define NormalScf       1               /* normal status clear flag     */
#define NormalSenseMode 0               /* dont get sense unless asked  */
#define AutoSenseMode   1               /* 'automatic get sense' mode   */

/*----------------------------------------------------------------------*/
/* Exception masks and timer handling.                                  */
/*----------------------------------------------------------------------*/

#define ResetTmrVal     100             /* value in msec                */
#define IoTmrVal        9000            /* value in msec                */
#define StopStrtTmrVal  30000           /* value in msec                */
#define ProgRptTmrVal   10000           /* value in msec                */
#define NrtrTmrVal      10000           /* value in msec                */
#define ScfTmrVal       3000            /* value in msec                */

#define ResetTmrMask    0x00000001      /* reset msla card timeout      */
#define IoTmrMask       0x00000002      /* io timeout mask              */
#define StopStrtTmrMask 0x00000004      /* stop/strt timeout mask       */
#define ProgRptTmrMask  0x00000008      /* timer for progress report    */
#define NrtrTmrMask     0x00000010      /* timer for NotReadyToReady    */
#define ScfTmrMask      0x00000020      /* timer for Scf flag checking  */
#define LoadEcb         0x80000000      /* msla code event control bit  */


/*----------------------------------------------------------------------*/
/* Values for area in communications area between AIX driver and the    */
/* MSLA ucode. See comm_area struct.                                    */
/*----------------------------------------------------------------------*/

/* icc values from R2 to MSLA comm area structure */
#define Cmd             0x00            /* a command was issued         */
#define LinkSwitch      0x01            /* link switch occurring        */
#define Init            0x04            /* initialization occurring     */
#define StatAccp        0x08            /* status accepted              */

/* icc values from MSLA to R2 comm area structure */
#define Cmd             0x00            /* a command was issued         */
#define LinkSwitch      0x01            /* link switch occurring        */
#define Init            0x04            /* initialization occurring     */
#define Diag            0x02            /* ccc defines a diagnose cmd   */
#define Debug           0x03            /* ccc defines a debug cmd      */
#define ProgCode        0x05            /* ccc defines a progress code  */
#define UnsolDevReq     0x06            /* unsolicited device request   */
#define AdapterErr      0x07            /* adapter error                */

/* ccc values for Cmd */
#define RdSense         0x04            /* read sense                   */
#define WrStruct        0x05            /* write structured             */
#define WrMemArea       0x09            /* write memory area            */
#define RdMemArea       0x0A            /* read memory area             */
#define Rmi             0x0E            /* read manual input            */
#define SelRdMemArea    0x2B            /* select read memory area      */
#define SetMode         0xF7            /* set mode                     */
#define SelWrMemArea    0xFB            /* select write memory area     */
#define UndefCmd        0xFF            /* undefined command            */

/* ccc values for LinkSwitch */
#define Connect         0x01            /* connect to the 5085          */

/* ccc values for Init */
#define RES1Top         0x30            /* top RES1 byte from Intr Init */
					/*     OR it with bits below    */
#define StartCmd        0x01            /* start command                */
#define StopCmd         0x02            /* stop command                 */
#define EnterMonMode    0x03            /* enter monitor mode           */
#define ConfigDataPres  0x04            /* configuration data present   */
#define DevNotPres      0x05            /* device not present           */
#define DevNotResp      0x06            /* device not responding        */
#define LinkDown        0x07            /* device not responding        */

/*----------------------------------------------------------------------*/
/*      Defines needed for GSW                                          */
/*----------------------------------------------------------------------*/
#define RWPERMS   S_IRUSR + S_IWUSR + S_IRGRP + S_IWGRP + S_IROTH + S_IWOTH
#define MslaLockFlags  LOCK_SHORT + LOCK_SIGRET
/***#define MslaLockFlags  LOCK_NDELAY + LOCK_SIGRET***/

/*----------------------------------------------------------------------*/
/*      Defines needed for POS regs                                     */
/*----------------------------------------------------------------------*/
#define Ena_Adapter    0x01                     /*             POS2     */
#define BusMasterDelay 0x0C                     /*  0 ns delay POS2     */
#define Ena_Parity     0x10                     /*             POS5     */

/*----------------------------------------------------------------------*/
/* FPGI defines.                                                        */
/*----------------------------------------------------------------------*/
#define FSMAMEMID       0xffee
#define CONN_CMD        0x717f
#define FPGI_NOINBUF    0x1001
#define FPGI_RDCOMPLETE 0x1002
#define FPGI_MODE       0x0010          /* Fpgi mode flag               */
#define SYSTEM_MODE     0x0020          /* system mode flag             */
#define CCWRMI          0x0e            /* command rmi                  */
#define CCWSRMA         0x2b            /* Select read mem area         */
#define CCWRMA          0x0a            /* Read memory area             */
#define CCWCC           0x40            /* Command chaining             */
#define CCWSSLI         0x20
#define LINK1           0x0001          /* chained ccws                 */
#define YesFpgiBuf      0x01
#define NoFpgiBuf       0x02
#define Conn_Compl      0x03
#define SEN_MAX         24              /* 24 bytes of sense            */
#define FpgiRdComplete  0x14
#define FpgiRdFailure   0x61
#define FpgiNoInbuf     0x62
#define FpgiWtComplete  0x70            /* write operation complete     */
#define FpgiWtMax       0x71            /* reached maximum writes       */

/*----------------------------------------------------------------------*/
/* Defines for use in OPEN routine to determine which ipl flag is set.  */
/*----------------------------------------------------------------------*/
#define GSW_OK              0           /* no bad iflags set            */
#define IO_NOT_ALLOWED      1           /* iflag.io_allowed flag not set*/
#define GSW_NOT_CONFIGURED  2           /* iflag.not_configured flag set*/
#define SWITCHED_TO_HOST    3           /* iflag.gsw_switched flag set  */
#define NOT_YET_READY       4           /* iflag.not_to_ready not set   */

/*----------------------------------------------------------------------*/
/* Defines for TRACE.                                                   */
/*----------------------------------------------------------------------*/
#define hkwd_CONF_R        HKWD_PSLA_CONF  | 1
#define hkwd_CONF_R1       HKWD_PSLA_CONF  | 2
#define hkwd_CONF_R2       HKWD_PSLA_CONF  | 3
#define hkwd_CONF_R3       HKWD_PSLA_CONF  | 4
#define hkwd_CONF_R4       HKWD_PSLA_CONF  | 5
#define hkwd_CONF_R5       HKWD_PSLA_CONF  | 6
#define hkwd_CONF_R6       HKWD_PSLA_CONF  | 7
#define hkwd_CONF_R7       HKWD_PSLA_CONF  | 8
#define hkwd_CONF_R8       HKWD_PSLA_CONF  | 9
#define hkwd_CONF_R9       HKWD_PSLA_CONF  | 10

#define hkwd_OPEN_R        HKWD_PSLA_OPEN  | 1
#define hkwd_OPEN_R1       HKWD_PSLA_OPEN  | 2
#define hkwd_OPEN_R2       HKWD_PSLA_OPEN  | 3
#define hkwd_OPEN_R3       HKWD_PSLA_OPEN  | 4

#define hkwd_CLOSE_R       HKWD_PSLA_CLOSE | 1
#define hkwd_CLOSE_R1      HKWD_PSLA_CLOSE | 2

#define hkwd_READ_R        HKWD_PSLA_READ  | 1
#define hkwd_READ_R1       HKWD_PSLA_READ  | 2
#define hkwd_READ_R2       HKWD_PSLA_READ  | 3
#define hkwd_READ_R3       HKWD_PSLA_READ  | 4

#define hkwd_WRITE_R       HKWD_PSLA_WRITE | 1
#define hkwd_WRITE_R1      HKWD_PSLA_WRITE | 2
#define hkwd_WRITE_R2      HKWD_PSLA_WRITE | 3
#define hkwd_WRITE_R3      HKWD_PSLA_WRITE | 4

#define hkwd_IOCTL_R       HKWD_PSLA_IOCTL | 1
#define hkwd_IOCTL_R1      HKWD_PSLA_IOCTL | 2
#define hkwd_IOCTL_R2      HKWD_PSLA_IOCTL | 3
#define hkwd_IOCTL_R3      HKWD_PSLA_IOCTL | 4
#define hkwd_IOCTL_R4      HKWD_PSLA_IOCTL | 5
#define hkwd_IOCTL_R5      HKWD_PSLA_IOCTL | 6
#define hkwd_IOCTL_R6      HKWD_PSLA_IOCTL | 7
#define hkwd_IOCTL_R7      HKWD_PSLA_IOCTL | 8
#define hkwd_IOCTL_R8      HKWD_PSLA_IOCTL | 9
#define hkwd_IOCTL_R9      HKWD_PSLA_IOCTL | 10
#define hkwd_IOCTL_R10     HKWD_PSLA_IOCTL | 11

#define hkwd_INTR_DATA     HKWD_PSLA_INTR  | 1
#define hkwd_INTR_R1       HKWD_PSLA_INTR  | 2
#define hkwd_INTR_R2       HKWD_PSLA_INTR  | 3
#define hkwd_INTR_R3       HKWD_PSLA_INTR  | 4
#define hkwd_INTR_R4       HKWD_PSLA_INTR  | 5
#define hkwd_INTR_R5       HKWD_PSLA_INTR  | 6
#define hkwd_INTR_R6       HKWD_PSLA_INTR  | 7
#define hkwd_INTR_R7       HKWD_PSLA_INTR  | 8
#define hkwd_INTR_R8       HKWD_PSLA_INTR  | 9
#define hkwd_INTR_R9       HKWD_PSLA_INTR  | 10
#define hkwd_INTR_R10      HKWD_PSLA_INTR  | 11

/*----------------------------------------------------------------------*/
/* wsf_type - used in the IOCTL code as an index into an array of       */
/*            structured field codes and lengths. This allows the       */
/*            driver to set a type field and make a single assigmnent   */
/*            at the end of the ioctl routine.                          */
/*----------------------------------------------------------------------*/

enum wsf_type { unused, galrm, gstop, gstrt, gsbf, gsbfst, gscur,
	       grcur, gsinds, gsion, gsioff, gdfmem, gdlmem, grnmem };


  /*--------------------------------------------------------------------*/
  /* Macros:                                                            */
  /*                                                                    */
  /*    RST_MSLA        reset MSLA                                      */
  /*    DIS_INTR_MSLA   disable MSLA  interrupts                        */
  /*    ENA_INTR_MSLA   enable MSLA  interrupts                         */
  /*    INT_MSLA        interrupt MSLA                                  */
  /*    HALT_MSLA       halt MSLA                                       */
  /*    STRT_MSLA       start MSLA                                      */
  /*    RST_INTR_MSLA   reset interrupt MSLA                            */
  /*                                                                    */
  /*    DIS_PARITY_POS  disable parity checking via POS reg 5, bit4=0   */
  /*    ENA_PARITY_POS  enable  parity checking via POS reg 5, bit4=1   */
  /*    ENA_MSLA_POS    enable  the adapter     via POS reg 2, bit0=1   */
  /*                                                                    */
  /*--------------------------------------------------------------------*/
#define RST_MSLA                                                        \
	{                                                               \
	rst_rgp = (char *)BUSMEM_ATT(BUS_ID,start_busio+ResetIOAdr);    \
	PRINT(("RST_MSLA: rst_rgp = 0x%x\n",rst_rgp));                  \
	*rst_rgp  = 0;                                                  \
	BUSMEM_DET(rst_rgp);                                            \
	}

#define DIS_INTR_MSLA                                                   \
	{                                                               \
	disi_rgp = (char *)BUSMEM_ATT(BUS_ID,start_busio+DisIntrIOAdr); \
	PRINT(("DIS_INTR_MSLA: disi_rgp = 0x%x\n",disi_rgp));           \
	*disi_rgp  = 0;                                                 \
	BUSMEM_DET(disi_rgp);                                           \
	}

#define ENA_INTR_MSLA                                                   \
	{                                                               \
	enai_rgp = (char *)BUSMEM_ATT(BUS_ID,start_busio+EnaIntrIOAdr); \
	PRINT(("ENA_INTR_MSLA: enai_rgp = 0x%x\n",enai_rgp));           \
	*enai_rgp  = 0;                                                 \
	BUSMEM_DET(enai_rgp);                                           \
	}

#define INT_MSLA                                                        \
	{                                                               \
	int_rgp = (char *)BUSMEM_ATT(BUS_ID,start_busio+IntIOAdr);      \
	PRINT(("INT_MSLA: int_rgp = 0x%x\n",int_rgp));                  \
	*int_rgp  = 0;                                                  \
	BUSMEM_DET(int_rgp);                                            \
	}

#define HALT_MSLA                                                       \
	{                                                               \
	stop_rgp = (char *)BUSMEM_ATT(BUS_ID,start_busio+StopIOAdr);    \
	PRINT(("HALT_MSLA: stop_rgp = 0x%x\n",stop_rgp));               \
	*stop_rgp  = 0;                                                 \
	BUSMEM_DET(stop_rgp);                                           \
	}

#define STRT_MSLA                                                       \
	{                                                               \
	strt_rgp = (char *)BUSMEM_ATT(BUS_ID,start_busio+StartIOAdr);   \
	PRINT(("STRT_MSLA: strt_rgp = 0x%x\n",strt_rgp));               \
	*strt_rgp  = 0;                                                 \
	BUSMEM_DET(strt_rgp);                                           \
	}

#define RST_INTR_MSLA                                                   \
	{                                                               \
	rsti_rgp = (char *)BUSMEM_ATT(BUS_ID,start_busio+ResetIntIOAdr);\
	PRINT(("RST_INTR_MSLA: rsti_rgp = 0x%x\n",rsti_rgp));           \
	*rsti_rgp  = 0;                                                 \
	BUSMEM_DET(rsti_rgp);                                           \
	}

#define DIS_PARITY_POS                                                  \
  {                                                                     \
   volatile char *pptr;                                                 \
   pptr  = (char *)IOCC_ATT(BUS_ID,POSREG(5,adapter_slot) + 0x400000);  \
   PRINT(("DIS_PARITY_POS: pptr = 0x%x\n",pptr));                       \
   *pptr &= ~Ena_Parity;              /* AND out neg parity bit POS5*/  \
   IOCC_DET(pptr);                                                      \
  }

#define ENA_PARITY_POS                                                  \
  {                                                                     \
   volatile char *pptr;                                                 \
   pptr  = (char *)IOCC_ATT(BUS_ID,POSREG(5,adapter_slot) + 0x400000);  \
   PRINT(("ENA_PARITY_POS: pptr = 0x%x\n",pptr));                       \
   *pptr |= Ena_Parity;               /* OR in   parity bit   POS5 */   \
   IOCC_DET(pptr);                                                      \
  }

#define ENA_MSLA_POS                                                    \
  {                                                                     \
   volatile char *pptr;                                                 \
   pptr  = (char *)IOCC_ATT(BUS_ID,POSREG(2,adapter_slot) + 0x400000);  \
   PRINT(("ENA_MSLA_POS: pptr = 0x%x\n",pptr));                         \
   *pptr  |= Ena_Adapter;               /* Enable the adapter   POS2 */ \
   IOCC_DET(pptr);                                                      \
  }

#define DIS_MSLA_POS                                                    \
  {                                                                     \
   volatile char *pptr;                                                 \
   pptr  = (char *)IOCC_ATT(BUS_ID,POSREG(2,adapter_slot) + 0x400000);  \
   PRINT(("DIS_MSLA_POS: pptr = 0x%x\n",pptr));                         \
   *pptr  &= ~Ena_Adapter;               /* Disable the adapter POS2 */ \
   IOCC_DET(pptr);                                                      \
  }














