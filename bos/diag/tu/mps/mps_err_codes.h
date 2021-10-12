/* @(#)37       1.1  src/bos/diag/tu/mps/mps_err_codes.h, tu_mps, bos411, 9437B411a 8/23/94 16:27:02 */
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: MPS Error Codes Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*************************************************
 * error types and error codes for tca test units.
 *************************************************/
#define SYS_ERR     0x00
#define LOG_ERR     0x02

/*********************************************************************
 * Error codes. Please reserve 0x8880 to 0x8889 for good return code
 * and 0x9xxx for errno .
 *********************************************************************/
#define READ_ERR        0x1     /* Generic read error code           */
#define WRITE_ERR       0x2     /* Generic write error code          */
#define COMPARE_ERR     0x4     /* Generic compare error code        */

#define ADAPTER_ERR     0xC01
#define ADAP_INIT_ERR   0xC02
#define ADAP_INT_ERR    0xC03
#define BAD_INTERRUPT   0xC04
#define CLEANUP_ERR     0xC05
#define CLOSE_ERR       0xC06
#define FILE_ERR        0xC07
#define FRAME_LEN_ERR   0xC08
#define OPEN_ERR        0xC09
#define REGISTER_ERR    0xC0A
#define STARTUP_ERR     0xC0B
#define SYSTEM_ERR      0xC0C
#define TIMEOUT_ERR     0xC0D
#define WRAP_ERR        0xC0E
#define SRB_CMD_ERR     0xC0F
#define SIGNAL_ERR      0xC10
#define HARD_ERR        0xC11
#define SOFT_ERR        0xC12
#define BEACON_ERR      0xC13
#define LOBE_ERR        0xC14
#define AUTO_ERR        0xC15
#define LAN_ERR         0xC16
#define REMOVE_ERR      0xC17
#define CABLE_ERR       0xC18

/************ POS Register Error Codes (tu001) **************************/
#define POS0_ERR     0x1000  /* Error in POS register 0 */
#define POS1_ERR     0x1100  /* Error in POS register 1 */
#define POS2_ERR     0x1200  /* Error in POS register 2 */
#define POS3_ERR     0x1300  /* Error in POS register 3 */
#define POS4_ERR     0x1400  /* Error in POS register 4 */
#define POS5_ERR     0x1500  /* Error in POS register 5 */
#define POS6_ERR     0x1600  /* Error in POS register 6 */

/************ I/O Register Error Codes (tu002) **************************/
#define IAA_ERR           0x2000
#define IAP_ERR           0x2100
#define BCtl_ERR          0x2200
#define RxCDPt_ERR        0x2300
#define RxStatPt_ERR      0x2400
#define Tx1CDPt_ERR       0x2500
#define Tx1StatPt_ERR     0x2600
#define Tx2CDPt_ERR       0x2700
#define Tx2StatPt_ERR     0x2800
#define RxPTIPtr_ERR      0x2900
#define Tx1PTIPtr_ERR     0x3000
#define Tx2PTIPtr_ERR     0x3100
#define LISR_ERR          0x3200
#define LISR_SUM_ERR      0x3300
#define LISR_RUM_ERR      0x3400
#define SISR_ERR          0x3500
#define SISR_SUM_ERR      0x3600
#define SISR_RUM_ERR      0x3700
#define SISRM_ERR         0x3800
#define SISRM_SUM_ERR     0x3900
#define SISRM_RUM_ERR     0x4000
#define MISR_ERR          0x4100
#define MISRM_SUM_ERR     0x4200
#define MISRM_RUM_ERR     0x4300
#define LAPA_ERR          0x4400
#define LAPE_ERR          0x4500
#define LAPD_ERR          0x4600
#define LAPDInc_ERR       0x4700
#define LAPWWO_ERR        0x4800
#define LAPWWC_ERR        0x4900
#define LAPCtl_ERR        0x5000
#define Timer_ERR         0x5100
#define SCBSig_ERR        0x5200
#define SCBStat_ERR       0x5300
#define BMCtl_SUM_ERR     0x5400
#define BMCtl_RUM_ERR     0x5500
#define RxLBDA_LO_ERR     0x5600
#define RxLBDA_HI_ERR     0x5700
#define RxBDA_LO_ERR      0x5800
#define RxBDA_HI_ERR      0x5900
#define RxStat_LO_ERR     0x6000
#define RxStat_HI_ERR     0x6100
#define RxDBA_LO_ERR      0x6200
#define RxDBA_HI_ERR      0x6300
#define Tx1LFDA_LO_ERR    0x6400
#define Tx1LFDA_HI_ERR    0x6500
#define Tx1FDA_LO_ERR     0x6600
#define Tx1FDA_HI_ERR     0x6700
#define Tx1Stat_LO_ERR    0x6800
#define Tx1Stat_HI_ERR    0x6900
#define Tx1DBA_LO_ERR     0x7000
#define Tx1DBA_HI_ERR     0x7100
#define Tx2LFDA_LO_ERR    0x7200
#define Tx2LFDA_HI_ERR    0x7300
#define Tx2FDA_LO_ERR     0x7400
#define Tx2FDA_HI_ERR     0x7500
#define Tx2Stat_LO_ERR    0x7600
#define Tx2Stat_HI_ERR    0x7700
#define Tx2DBA_LO_ERR     0x7800
#define Tx2DBA_HI_ERR     0x7900


/************ MPS Kernel Extension Error Codes  **************************/
#define D_INIT_ERR        0xE00
#define XMALLOC_ERR       0xE01
#define XMATTACH_ERR      0xE02
#define MBUF_ERR          0xE03

#define DMAMASK1          0x10000
#define DMAMASK2          0x20000
#define DMAMASK3          0x30000
#define DMAMASK4          0x40000

/************ General Error Codes  **************************/
#define ILLEGAL_TU_ERR    0x777  /* Illegal TU value - internal error */

/************************************
 *   Error Message Definitions
*************************************/
#define CONNECT_PREFIX      "CONNECTION TEST ERROR: "
#define INT_WRAP_PREFIX     "INTERNAL WRAP TEST ERROR: "
#define EXT_WRAP_PREFIX     "EXTERNAL WRAP TEST ERROR: "
#define NETWORK_PREFIX      "NETWORK TEST ERROR: "
#define POS_PREFIX          "POS REGISTER TEST ERROR: "
#define IO_PREFIX           "I/O REGISTER TEST ERROR: "
#define ONCARD_PREFIX       "ONCARD DIAGNOSTICS TEST ERROR: "
#define INIT_PREFIX         "INITIALIZATION ERROR: "
#define TERM_PREFIX         "TERMINATION ERROR: "
#define DEFAULT_PREFIX      "ERROR: "
#define FRAME_LEN_ERR_MSG   "ILLEGAL FRAME SIZE SPECIFIED\n"
#define FILE_ERR_MSG        "COULD NOT OPEN TEST PATTERN FILE\n"
#define READ_ERR_MSG        "READ FAILED\n"
#define WRITE_ERR_MSG       "WRITE FAILED\n"
#define COMPARE_ERR_MSG     "COMPARE FAILED\n"
#define ADAPTER_ERR_MSG     "ADAPTER TEST FAILED\n"
#define CLOSE_ERR_MSG       "CLOSE FAILED\n"
#define OPEN_ERR_MSG        "OPEN FAILED\n"
#define CLEANUP_ERR_MSG     "CLEANUP FAILED\n"
#define STARTUP_ERR_MSG     "STARTUP FAILED\n"
#define SYSTEM_ERR_MSG      "SYSTEM FAILED\n"
#define TIMEOUT_ERR_MSG     "SYSTEM TIMED OUT\n"
#define WRAP_ERR_MSG        "WRAP TEST FAILED\n"
#define LOBE_ERR_MSG        "LOBE WIRE FAULT\n"
#define AUTO_ERR_MSG        "AUTO REMOVAL ERROR\n"
#define REMOVE_ERR_MSG      "REMOVE RECEIVE ERROR\n"
#define COUNT_ERR_MSG       "COUNTER OVERFLOW\n"
#define RING_ERR_MSG        "RING RECOVERY FAILURE\n"
#define LAN_ERR_MSG         "LAN STATUS FAILURE\n"
#define HARD_ERR_MSG        "HARD FAILURE\n"
#define SOFT_ERR_MSG        "SOFT FAILURE\n"
#define BEACON_ERR_MSG      "TRANSMIT BEACON FAILURE\n"
#define CABLE_ERR_MSG       "CABLE NOT CONNECTED\n"
#define SIGNAL_ERR_MSG      "SIGNAL LOSS\n"
#define RETURN_CODE_MSG     "  RETURN CODE = %x\n"
#define SYS_RC_MSG          "  SYSTEM RETURN CODE = %x\n"
#define BAD_ADDRESS_MSG     "  FAILING ADDRESS = %x\n"
#define EXPECTED_VALUE_MSG  "  EXPECTED VALUE  = %x\n"
#define ACTUAL_VALUE_MSG    "  ACTUAL VALUE    = %x\n"
#define HOST_RC_MSG         "  HOST RETURN CODE          = %x\n"
#define MANUFAC_RC_MSG      "  MANUFACTURING RETURN CODE = %x\n"
#define ATTEMPTED_WRITE_MSG "  ATTEMPTED WRITE VALUE = %x\n"
#define BAD_INT_MSG         "  UNEXPECTED INTERRUPT = %x\n"
#define BAD_SRB_MSG         "  UNEXPECTED SRB RESPONSE = %x\n"
#define OPEN_CODE_MSG       "  OPEN ERROR RETURN CODE = %x\n"
#define WRAP_ERR_CODE       "  WRAP ERROR RETURN CODE = %x\n"
#define TX_LEN_MSG          "  Transmit length  = %d\n"
#define RX_LEN_MSG          "  Receive length = %d\n"

