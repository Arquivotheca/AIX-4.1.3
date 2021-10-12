/* @(#)23       1.13.2.6  src/bos/kernext/disp/gem/inc/gem_def.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:41:53 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define BUS_ID         0x82000020
#define GEMINI_ID      0x00008FFD
#define PosEnable      0x01           /* Enable Adapter via POS        */
#define PosDisable     0x00           /* Disble Adapter via POS        */

/************************************************************************/
/* Gemini request codes                                                 */
/************************************************************************/
#define RQ_STRT_FIFO   0x02               /* start FIFO                 */
#define RQ_STOP_FIFO   0x04               /* stop FIFO                  */
#define DRP_SOFTRSET   0x00000001         /* requests a DrP Soft Reset  */
#define RQ_GCP_SRSET   0x00000000         /* requests a GCP Soft Reset  */

#define X_RES          1280             /* Max number of pels on x axis */
#define Y_RES          1024             /* Max number of pels on y axis */

/************************************************************************/
/* Adapter Interrupt Event Codes                                        */
/************************************************************************/
#define THRESHOLD_EVENT     0x01        /* Threshold Event              */
#define FIFO_SYNC_CNT_EVENT 0x02        /* FIFO Sync Counter Event      */
#define FIFO_SYNC_EVENT     0x04        /* FIFO Syncronization Event    */
#define PICK_EVENT          0x08        /* Pick Event                   */
#define ERROR_EVENT         0x10        /* Adapter Error Event          */

/************************************************************************/
/* Event codes used for synchronous and asynchronous events             */
/************************************************************************/
#define NULL_CMD          0x00000000
#define SYNC_WAIT_CMD     0x00000001
#define SYNC_NOWAIT_CMD   0x00000002
#define ASYNC_CMD         0x00000004
#define PICK_CMD          0x00000008
#define GSYNC_CMD         0x00000010
#define gto_WAIT          0x00000001
#define DMA_IN_PROGRESS   0x00000020
#define BLOCKIMM          0x00000040
#define BLOCKTRAV         0x00000080

/************************************************************************/
/* Codes used by Enable Event to save current process id                */
/************************************************************************/
#define SAVE_IMM_PID      0x00000001
#define SAVE_TRAV_PID     0x00000002

/************************************************************************/
/* Gemini adapter interrupt reason codes                                */
/************************************************************************/
#define R_SYNC            0x09           /* syncroniztion complete      */
#define R_PICK_OCCR       0x07           /* pick occured                */
#define R_DRP_COMP1       0x00           /* DRP IMM FIFO Complete       */
#define R_DRP_COMP3       0x01           /* DRP TRAV FIFO COmplete      */
#define R_DRP_EXCP        0x02           /* DRP Execption Error         */

/************************************************************************/
/* Reasons for Error logs.                                              */
/************************************************************************/
#define R_UNKN_REA_COD    0xF0           /* unknown reason code         */
#define R_UNKN_VME_INT    0xF1           /* unknown CVME interrupt      */
#define R_UNKN_ADP_INT    0xF2           /* unknown Gemini interrupt    */
#define R_INVAL_OP        0xF3           /* invalid operation code      */

/************************************************************************/
/* Error Codes                                                          */
/************************************************************************/
#define ERROR             -1               /* Return unsuccessful       */
#define SUCCESS            0               /* Return successful         */

/************************************************************************/
/* Sync Counter Event Codes                                             */
/************************************************************************/
#define SYNC_CTR0         0x01             /* Fifo 0 Sync intr          */
#define SYNC_CTR1         0x02             /* Fifo 1 Sync intr          */
#define SYNC_CTR2         0x04             /* Fifo 2 Sync intr          */
#define SYNC_CTR3         0x08             /* Fifo 3 Sync intr          */

/************************************************************************/
/* Start of Adapter Global Memory                                       */
/************************************************************************/
#define GLOBAL_OFF     0x00002000

/************************************************************************/
/* FIFO Starting offsets in Global Memory                               */
/************************************************************************/
#define GCPImmedFifo   0x001E0000          /* immed. SE fifo offset     */
#define BltImmedFifo   0x001F0000          /* immed. DATA fifo offset   */
#define GCPTravFifo    0x00010000          /* trav. SE fifo offset      */
#define BltTravFifo    0x00020000          /* trav. DATA fifo offset    */

/************************************************************************/
/* ++++++++++ Offsets to Registers and Register masks  +++++++++++++++  */
/************************************************************************/

/************************************************************************/
/* MBC Registers                                                        */
/************************************************************************/
#define MASTER_STAT    0x00000200          /* Master Status Register     */
#define M_STAT_MASK    0xF0000000          /* Master Status Register Mask*/
#define MASTER_CTRL    0x00000204          /* Master Control Reg         */
#define M_CTRL_MASK    0xE8000000          /* Master Control Reg Mask    */
#define MEMORY_START   0x00000208          /* Memory Start Address Reg   */
#define MEMORY_END     0x0000020C          /* Memory End Address Reg     */
#define LOCAL_START    0x00000210          /* Local Bus Start Addr Reg   */
#define LOCAL_END      0x00000214          /* Local Bus End Address Reg  */
#define GLOBAL_START   0x00000218          /* Global Bus Start Address   */
#define GLOBAL_END     0x0000021C          /* Global Bus End Address Reg */

/************************************************************************/
/* Adapter Control Registers                                            */
/************************************************************************/
#define GEM_CONTROL    0x00000300          /* Gemini Control Reg         */
#define GCR_MASK       0xF7030007          /* Gemini Control Reg Mask    */

#define STATUS_REG     0x00000304          /* Status Reg                 */
#define STATUS_MASK    0x0000FFFD          /* Status Register Mask       */
#define STATUS_INT     0x00000009          /* Status Reg Interrupt Mask  */

#define INTR_PENDING   0x00000308          /* Interrupt Pending Reg      */
#define INTR_PEND_MASK 0x00007FFF          /* Interrupt Pending Reg Mask */

#define INC_DEC_CLEAR  0x0000030C          /* Incr/Dec/Clear Reg         */
#define INCDECLR_MASK  0x0000090F          /* Incr/Dec/Clear Reg Mask    */

#define GEO_ADDR       0x00000310          /* Geographic Address Reg     */
#define GEO_ADDR_MASK  0x0000000F          /* Geographic Address Reg Mask*/

#define DMA_DEST_ADDR  0x00000314          /* DMA Destination Address Reg*/
#define DMA_DEST_MASK  0x003FFFFF          /* DMA Destination Addr Mask  */

#define FIF0_SYNC_INT0 0x00000320          /* FIFO Sync Int Enable 0     */
#define FIF0_SYNC_INT1 0x00000324          /* FIFO Sync Int Enable 1     */
#define FIF0_SYNC_INT2 0x00000328          /* FIFO Sync Int Enable 2     */
#define FIF0_SYNC_INT3 0x0000032C          /* FIFO Sync Int Enable 3     */
#define FIF0_SYNC_MASK 0x00000001          /* FIFO Sync Int Enable Mask  */

#define ENA_DMA_COMP   0x00000330          /* Term Count DMA Int Enable  */
#define ENA_DMA_INTR   0x00000001          /* Enable DMA Complete Intr   */

#define SYS_CONTROL    0x00000400          /* System Control Reg         */
#define SYS_CTL_MASK   0xF7FF11FF          /* System Control Reg Mask    */

#define VME_INTR_VECT  0x00000480          /* cVME Bus Interrupt Vector  */
#define VME_INTR_MASK  0x000000FF          /* cVME Bus Interrupt Mask    */

/************************************************************************/
/* Adapter FIFO Control Registers                                       */
/************************************************************************/
#define FIF0_0_INPUT   0x00000600          /* FIFO 0 Input Range         */
#define FIF0_1_INPUT   0x00000800          /* FIFO 1 Input Range         */
#define INTR_HIGH_0    0x00000A00          /* Interrupt Enable High 0    */
#define INTR_LOW_0     0x00000A04          /* Interrupt Enable Low 0     */
#define HIGH_THRESH_0  0x00000A08          /* High Threshold 0           */
#define LOW_THRESH_0   0x00000A0C          /* Low Threshold 0            */
#define GBL_MEM_PTR_0  0x00000A10          /* Global Memory Pointer 0    */
#define INDEX_PTR_0    0x00000A14          /* Index Pointer 0            */
#define FIFO_0_USE_CNT 0x00000A18          /* FIFO In Use Count 0        */
#define FIFO_0_CONTROL 0x00000A1C          /* FIFO 0 Control Reg         */
#define FIFO_0_ADD     0x00000A20          /* FIFO 0 Add Reg             */
#define FIFO_0_SUB     0x00000A24          /* FIFO 0 Subtract Reg        */

#define INTR_HIGH_1    0x00000B00          /* Interrupt Enable High 1    */
#define INTR_LOW_1     0x00000B04          /* Interrupt Enable Low 1     */
#define HIGH_THRESH_1  0x00000B08          /* High Threshold 1           */
#define LOW_THRESH_1   0x00000B0C          /* Low Threshold 1            */
#define GBL_MEM_PTR_1  0x00000B10          /* Global Memory Pointer 1    */
#define INDEX_PTR_1    0x00000B14          /* Index Pointer 1            */
#define FIFO_1_USE_CNT 0x00000B18          /* FIFO In Use Count 1        */
#define FIFO_1_CONTROL 0x00000B1C          /* FIFO 1 Control Reg         */
#define FIFO_1_ADD     0x00000B20          /* FIFO 1 Add Reg             */
#define FIFO_1_SUB     0x00000B24          /* FIFO 1 Subtract Reg        */

#define VPD_START      0x00001000          /* VPD Data                   */

#define FIF0_2_INPUT   0x00001600          /* FIFO 2 Input Range         */
#define FIF0_3_INPUT   0x00001800          /* FIFO 3 Input Range         */
#define INTR_HIGH_2    0x00001A00          /* Interrupt Enable High 2    */
#define INTR_LOW_2     0x00001A04          /* Interrupt Enable Low 2     */
#define HIGH_THRESH_2  0x00001A08          /* High Threshold 2           */
#define LOW_THRESH_2   0x00001A0C          /* Low Threshold 2            */
#define GBL_MEM_PTR_2  0x00001A10          /* Global Memory Pointer 2    */
#define INDEX_PTR_2    0x00001A14          /* Index Pointer 2            */
#define FIFO_2_USE_CNT 0x00001A18          /* FIFO In Use Count 2        */
#define FIFO_2_CONTROL 0x00001A1C          /* FIFO 2 Control Reg         */
#define FIFO_2_ADD     0x00001A20          /* FIFO 2 Add Reg             */
#define FIFO_2_SUB     0x00001A24          /* FIFO 2 Subtract Reg        */

#define INTR_HIGH_3    0x00001B00          /* Interrupt Enable High 3    */
#define INTR_LOW_3     0x00001B04          /* Interrupt Enable Low 3     */
#define HIGH_THRESH_3  0x00001B08          /* High Threshold 3           */
#define LOW_THRESH_3   0x00001B0C          /* Low Threshold 3            */
#define GBL_MEM_PTR_3  0x00001B10          /* Global Memory Pointer 3    */
#define INDEX_PTR_3    0x00001B14          /* Index Pointer 3            */
#define FIFO_3_USE_CNT 0x00001B18          /* FIFO In Use Count 3        */
#define FIFO_3_CONTROL 0x00001B1C          /* FIFO 3 Control Reg         */
#define FIFO_3_ADD     0x00001B20          /* FIFO 3 Add Reg             */
#define FIFO_3_SUB     0x00001B24          /* FIFO 3 Subtract Reg        */

/*************************************************************************/
/* Adapter FIFO Control Registers Masks                                  */
/*************************************************************************/
#define IN_USE_MASK     0x0001FFFF         /* FIFO In Use Count Reg Mask */
#define LO_THRESH_MASK  0x0000FFFF         /* Low Threshold Mask         */
#define HI_THRESH_MASK  0x0000FFFF         /* High Threshold Mask        */
#define ENABLE_THRESH   0x00000001         /* Intr Enable Threshold Intr */
#define DISABLE_THRESH  0x00000000         /* Intr Disable Threshold Intr*/
#define ENABLE_DMA_SUSP 0x00000010;        /* Enable DMA Suspend Logic   */

/*************************************************************************/
/* Default DMA Threshold values                                          */
/*************************************************************************/
#define HIGH_DMA_THRESH   0xFF80
#define LOW_DMA_THRESH    0xFE80

/*************************************************************************/
/* PID Flags indicating current state of process                         */
/*************************************************************************/
#define AWAKE          0x00000000          /* Initialize PID flags       */
#define SLEEPING       0x00000001          /* Process is blocked         */
#define DRP_INTR_COMP  0x00000002          /* DRP Interrupt Complete Flag*/

/*************************************************************************/
/* Gemini structure element codes                                        */
/*************************************************************************/
#define CE_SDFBLN   0x000801C8             /* Select Drawing Frame Buffer*/
#define CE_FCTLLN   0x000801C9             /* frame buffer control       */
#define CE_ACTCLN   0x000801C0             /* activate context           */
#define CE_LBPCLN   0x000C01CA             /* load base plane color table*/
#define CE_SDDSLN   0x000401C3             /* Set Default Drawing State  */
#define CE_LOGOLN   0x0008018B             /* Set Logical Operation      */
#define CE_COPYLN   0x00240182             /* VPM to VPM Copy            */
#define CE_WRITLN   0x00240180             /* Write Pixel                */
#define SE_ISTYLN   0x00080018             /* Set Interior Style         */
#define SE_INCILN   0x00080186             /* Set Interior Color         */
#define CE_IPLGLN   0x00200189             /* Integer Polygon            */
#define SE_SWATLN   0x001001CF             /* Set Window Attributes      */
#define SE_PG2      0x0122                 /* FPGI floating polygon 2    */
#define CE_WRIT     0x0180                 /* write pixel                */
#define CE_COPY     0x0182                 /* vpm to vpm copy            */
#define SE_INCI     0x0186                 /* Set Interior Color Index   */
#define CE_LOGO     0x018B                 /* set logical operation      */
#define CE_LOPC     0x01CB                 /* load overlay plane colr tab*/
#define CE_GMEM     0x01D7                 /* set global memory          */

#define SOLID       0x00000002             /* Interior Polygon - Solid   */

/*************************************************************************/
/* defines used in Load_ucode routine                                    */
/*************************************************************************/
#define PSB_TIMOUT      0xE0000000        /* PSB timeout, Mast stat reg  */
#define GCP_ID          0x00F20000        /* actual GCP card ID No.      */
#define SHP_ID          0x00F10000        /* SHP card ID                 */
#define DRP_ID          0x00F60000
#define IMP_ID          0x00F30000
#define GBL_LO_LIMIT    0x03000000        /* MBC recgnzd lower limit add */
#define GBL_HI_LIMIT_2M 0x031FFFFF        /* MBC recgnzd upper limit 2m  */
#define GBL_HI_LIMIT_4M 0x033FFFFF        /* MBC recgnzd upper limit 4m  */
#define MLOAD_ADDR      0x003CEE00        /* Mode Load write address for
					     controller initialization   */
#define BIF_RUNFLG_A    0x01FC            /* BIF Run Flag offset         */
#define RUN_FLAG        0x00010000        /* BIF Run Flag                */
#define MOV_TO_MEM      0x00010000        /* move to memory code in GCP  */
#define MOV_BR_EXE      0x00020000        /* move to mem and branch and
					     execute code in GCP         */
#define LOD_MCOD        0x00040000        /* load micro code in GCP      */
#define CVME_ADDR       0x00000800        /* CVME address                 */
#define SET_GEM_VME     0x0800000C        /* Set GEM and cVME reset       */
#define RESET_GEMINI    0x08000008        /* Reset GEMINI                 */
#define RESET_CVME      0x00000004        /* Reset CVME                   */
#define CLR_RESET_CVME  0xFFFFFFFB        /* Clear CVME Reset bit         */
#define ENABLE_CVME_INT 0x02000000        /* Enable CVME Interrupts       */
#define ENABLE_BERR_INT 0x04000000        /* Enable CVME Buss Error int   */
#define CLR_GEM_VME     0xF3FFFFF3        /* clear GEM and cVME reset     */
#define CLEAR_REG       0x00000000
#define SET_MODELOAD    0x00000002        /* sets mode load and byte order*/
#define RSET_MODELOAD   0xFFFFFFFD        /* resets mode load and sets byte
					     order                        */
#define ENABLE_INTR     0x06000000        /* enable cVME bus and Bus Error
					     bits in Gem Ctrl Reg         */
#define DABLE_INTRPTS   0xF5FFF00F        /* disable all interrupts in
					     Gem Ctrl reg                 */
#define DABLE_SYNCS     0xFFFFFF0F        /* disable sync intr bits in GCR*/
#define DABLE_TRSHDS    0xFFFFF0FF        /* disable threshd intr bits,GCR*/
#define CLR_G_ADR       0xFFFF0FFF        /* clear geogrphical addresses  */
#define DELAY_ADDR      0x80E7
#define DELAY_TIME      9000000
#define DELAY_MODE_LOAD 500000
#define BITS_IN_BYTE    8

/**************************************************************************/
/* Annotation Text Initialization Constants                               */
/**************************************************************************/
#define MAX_FONTS         8
#define BIF_TEXT_OFF      0x00000074
#define TEXT_TBL_LEN      0x00000400
#define LOOKUP_TBL_LEN    0x00000400
#define LOC_TBL_RCD_LEN   0x00000150
#define LAST_LOC_ENTRY    0x40000000

/**************************************************************************/
/* Pixels per Meter                                                       */
/**************************************************************************/
#define PIX_METER_16      0x458797DD
#define PIX_METER_19      0x4560F8CD
#define PIX_METER_23L     0x453C3C3C
#define PIX_METER_23H     0x458D2D2D

/**************************************************************************/
/* Gemini Misc.                                                           */
/**************************************************************************/
#define MGC_G_ADR  ld->gcardslots->magic
#define SHP_G_ADR  ld->gcardslots->shp
					  /* SHP geog adr                 */
#define GCP_G_ADR  ld->gcardslots->gcp
					  /* GCP geog adr                 */
#define DRP_G_ADR  ld->gcardslots->drp
					  /*DRP geog adr                  */
#define MBC_INTR        0x40              /* cVME Bus error interrupt     */
#define GCP_INTR        0x41              /* GCP Interrupt Vector         */
#define DRP_INTR        0x42              /* Drp Interrupt Vector         */
#define SHP_INTR        0x43              /* ShP Interrupt Vector         */
#define IMP_INTR        0x44              /* Image Processor Int Vector   */
#define REL_ADAP        0x00000100        /* sets reset bits in sys ctrl r*/
#define RELS_ADAP       0x01000100        /* sets reset bits in sys ctrl r*/
#define HOLD_ADAP       0xFEFFFEFF        /* resets reset bits in sys ctrl*/
#define DRP_RES_MSK     0x0000FFFF        /* reason field mask in Drp Dblk*/
#define DRP_BIF_SRSET_A 0x0110            /* DrP BIF offset for Soft Rset
					     request                      */
#define GCP_REQ_RES_MSK 0x000000FF        /* request/reason fields mask
					     in GCP data blocks           */
#define CLR_BFLG        0x00000000        /* GCP/RIOS data block busy flg */
#define IN_PROCESS      0x00000200        /* GCP_RIOS data block in_proces*/
#define COUNT_1MS       100               /* count to wait 1 micro sec    */
#define COUNT_12MS      1000              /* count to wati 12 micro sec   */
#define VME_BUS_MEM     0x03000000        /* cVME bus global memory addres*/
#define SixtyFourK      0x10000
#define TWO_MEG         0x200000
#define GCP_GVP_LEN     0x2000

#define WATCH_RESTART_TIME   0x00000014

#define   ImmFIFO        0
#define   ImmBLTFIFO     1
#define   TravFIFO       2
#define   TravBLTFIFO    3

/************************************************************************/
/* Others                                                               */
/************************************************************************/
#define  ACTIVE    1
#define  INACTIVE  0
#define  MONITOR   0
#define  VISIBLE   1
#define  NON_BLANK 0
#define  BLANK     1

/***********************************************************************/
/* Status flags                                                        */
/***********************************************************************/
#define ON_ADAPT        0x10
#define IN_KERNEL       0x20
#define NEW_CXT         0x40
#define PINNED          0x80

/************************************************************************/
/* Domain Constants                                                   @9*/
/************************************************************************/
#define MAX_SLOTS       64

#define GM_AUTH_KEY     3               /* hak PM was 2                 */

/************************************************************************/
/* Hardware ID constants                                              @A*/
/************************************************************************/
/* RPD VWID Init:  Define some new constants to help describe the way that
 *   virtual window IDs are handled through X- that is, that some of them
 *   are reserved for X's use.
 */
#define NUM_HWID_PLANES         6
#define NUM_HWIDS               (1 << NUM_HWID_PLANES)
#define PROTECT_HWID		(NUM_HWIDS-1)
#define NUM_VLTS                5 

/* We have to reserve holding id's for when we don't have enough to go
 * around.  Save one holding id for each possible combination of color map,
 * base frame buffer display (a or b) and overlay frame buffer display (a
 * or b).  Then save an extra for the RCM's private HWID and one for the
 * special-reserved-hwid-that-I-don't-know-what-it's-for.
 */
#define SYS_HWID_RESRVD         (2*2*NUM_VLTS+1+1)

/*
 * NUM_DWA_HWIDS is defined in inc/sys/gmcomm.h.  It's 8 right now, and
 * I don't expect that it will ever need to change.
 */
#define DWA_HWID_RESRVD        NUM_DWA_HWIDS 

/* Give all of the extra HWIDS to X */
#define X_NUM_HWIDS             (NUM_HWIDS-SYS_HWID_RESRVD-DWA_HWID_RESRVD)

/* Define a range which indicates the first and last HWID available for the
 * RCM to assign to DWA windows.  FIRST_DWA_HWID and LAST_DWA_HWID are
 * defined in inc/sys/gmcomm.h.
 */
#define DWA_FIRST_HWID          FIRST_DWA_HWID
#define DWA_LAST_HWID           LAST_DWA_HWID

/* This private hwid is defined to be the lowest numbered system hwid */
#define RCM_PRIVATE_HWID        (NUM_HWIDS-SYS_HWID_RESRVD+1)

/************************************************************************/
/* ISO/ANTI-ALIAS Support                                               */
/************************************************************************/
#define SIXTY_HZ                    2
#define SEVENTY7_HZ                 3
#define ISO                      0x01
#define ANTI_ALIAS               0x02
#define ANTI_ALIAS_INSTALLED     0x08
#define ISO_SUPPORTED            0x10
#define SEVENTY7_HZ_REQUEST      0x20
#define FOUR_MEG_INSTALLED       0x02

