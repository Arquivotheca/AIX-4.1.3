/* @(#)36       1.4  src/bos/kernext/mps_tok/mps_dslo.h, sysxmps, bos41J, 9511A_all 3/7/95 15:28:04 */
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: GET_NETID
 *		MAC_FRAME
 *		MBUF_TO_MEM
 *		MPS_GETPOS
 *		MPS_PUTPOS
 *		PIO_GETCX
 *		PIO_GETLRX
 *		PIO_GETLX
 *		PIO_GETSRX
 *		PIO_GETSTRX
 *		PIO_GETSX
 *		PIO_PUTCX
 *		PIO_PUTLRX
 *		PIO_PUTLX
 *		PIO_PUTSRX
 *		PIO_PUTSTRX
 *		PIO_PUTSX
 *		toendianL
 *		toendianW
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include <sys/ioacc.h>
 
#ifndef _H_MPSDSLO
#define _H_MPSDSLO
 
/******************************************************************************/
/*  These BUS accessors are PIO-recovery versions of the original BUS         */
/*  accessor macros.  The essential difference is that retries are            */
/*  performed if pio errors occur; if the retry limit is exceeded, a -1       */
/*  is returned (hence all return an int value).  In the cases of             */
/*  PIO_GETL and PIO_GETLR, the -1 is indistinguishable from all FF's so      */
/*  some heuristic must be used to determine if it is an error (i.e., is      */
/*  all FF's a legitimate read value?).                                       */
/******************************************************************************/

#define PIO_RETRY_COUNT         3

#define MPS_GETPOS(addr, c) \
{ \
        uchar pos_c; \
        uint pos_i; \
                    \
        for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++) { \
                BUS_GETCX((char *)addr, (char *)(c)); \
                BUS_GETCX((char *)addr, &pos_c); \
                if (*(char *)(c) == pos_c) \
                        break; \
        } \
        if (pos_i >= PIO_RETRY_COUNT) \
                pio_rc = TRUE; \
}

#define MPS_PUTPOS(addr, c) \
{ \
        uchar pos_c; \
        uint pos_i; \
                    \
        for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++) { \
                BUS_PUTCX((char *)addr, (char)(c)); \
                BUS_GETCX((char *)addr, &pos_c); \
                if ((char)(c) == pos_c) \
                        break; \
        } \
        if (pos_i >= PIO_RETRY_COUNT) \
                pio_rc = TRUE; \
}
 
enum pio_func{
        GETCX, GETSX, GETSRX, GETLX, GETLRX, GETSTRX,
        PUTCX, PUTSX, PUTSRX, PUTLX, PUTLRX, PUTSTRX,
};
 
#define PIO_GETCX(addr, c) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_GETCX((char *)(addr), (char *)(c))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           GETCX, (int) addr, (long) c, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_GETSX(addr, s) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_GETSX((short *)(addr), (short *)(s))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           GETSX, (int) addr, (long) s, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_GETSRX(addr, sr) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_GETSRX((short *)(addr), (short *)(sr))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           GETSRX, (int) addr, (long) sr, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_GETLX(addr, lr) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_GETLX((long *)(addr), (long *)(lr))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           GETLX, (int) addr, (long) lr, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_GETLRX(addr, lr) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_GETLRX((long *)(addr), (long *)(lr))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           GETLRX, (int) addr, (long) lr, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_PUTCX(addr, c) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_PUTCX((char *)(addr), (char)(c))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           PUTCX, (int) addr, (long) c, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_PUTSX(addr, sr) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_PUTSX((short *)(addr), (short)(sr))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           PUTSX, (int) addr, (long) sr, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_PUTSRX(addr, sr) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_PUTSRX((short *)(addr), (short)(sr))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           PUTSRX, (int) addr, (long) sr, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_PUTLX(addr, lr) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_PUTLX((long *)(addr), (long)(lr))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           PUTLX, (int) addr, (long) lr, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_PUTLRX(addr, lr) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_PUTLRX((long *)(addr), (long)(lr))) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           PUTLRX, (int) addr, (long) lr, 0); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_PUTSTRX(addr, src, c) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_PUTSTRX((char *)(addr), (char *)(src), (int)c)) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           PUTSTRX, (int) addr, (long) src, \
                                           (int) c); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}

#define PIO_GETSTRX(addr, src, c) \
{ \
        int excpt_code; \
        if (excpt_code = BUS_GETSTRX((char *)(addr), (char *)(src), (int)c)) { \
                excpt_code = pio_retry(p_dev_ctl, excpt_code, \
                                           GETSTRX, (int) addr, (long) src, \
                                           (int) c); \
                WRK.pio_rc |= excpt_code; \
                mps_logerr(p_dev_ctl, ERRID_MPS_PIO_ERR, __LINE__,      \
                           __FILE__, NDD_PIO_FAIL, excpt_code, 0);      \
        } \
}


/******************************************************************************/
/*   toendnW, toendianL -                                                     */
/*   convert bewteen endian-ness for word (W) and long word (L).              */
/******************************************************************************/
#define toendianW(v)                                  \
  ((unsigned)                                         \
    (((v & 0xff00) >> 8)      |                       \
      ((v & 0x00ff) <<  8)))
 
#define toendianL(v)                                  \
  ((ulong)                                            \
    (((v & 0xff000000) >> 24) |                       \
      ((v & 0x00ff0000) >>  8)  |                     \
        ((v & 0x0000ff00) <<  8)  |                   \
          ((v & 0x000000ff) << 24)))
 
/******************************************************************************/
/*                                 MACROS                                     */
/******************************************************************************/
#define NETID_RECV_OFFSET  14
#define FC_OFFSET          1            /* byte offset to frame ctrl field */
 
#define MBUF_TO_MEM(mbufp, buf)                                 \
{                                                               \
        struct  mbuf    *mp = mbufp;                            \
        caddr_t         mem = buf;                              \
                                                                \
        while (mp) {                                            \
                bcopy(mtod(mp, caddr_t), mem, mp->m_len);       \
                mem += mp->m_len;                               \
                mp = mp->m_next;                                \
        }                                                       \
}
 
# define GET_NETID(frameptr) \
             *((uchar *)(frameptr + NETID_RECV_OFFSET))
 
# define MAC_FRAME(frameptr) \
   ((*((uchar *)((uchar *)frameptr + FC_OFFSET))) & FC_TYPE) == MAC_TYPE
 
/*----------------------------------------------------------------------------*/
/*                 Constant Defines                         */
/*----------------------------------------------------------------------------*/
#define POS0            0x00                /* POS Register 0 IOCC offset    */
#define POS1            0x01                /* POS Register 1 IOCC offset    */
#define POS2            0x02                /* POS Register 2 IOCC offset    */
#define POS3            0x03                /* POS Register 3 IOCC offset    */
#define POS4            0x04                /* POS Register 4 IOCC offset    */
#define POS5            0x05                /* POS Register 5 IOCC offset    */
#define POS6            0x06                /* POS Register 6 IOCC offset    */
#define POS7            0x07                /* POS Register 7 IOCC offset    */
#define PR0_VALUE       0xA2                /* low  byte of CARDID           */
#define PR1_VALUE       0x8F                /* high byte of CARDID           */
 
#define MAC_BUFF_LEN     160                /* Length of TRB data Buffer     */

/******************************************************************************/
/*              Adapter Error log type defination                             */
/******************************************************************************/

typedef struct {
  uchar         line_error;
  uchar         internal_error;
  uchar         burst_error;
  uchar         ari_fci_error;
  uchar         abort_delimiter;
  uchar         r1;
  uchar         lost_frame;
  uchar         receive_congestion;
  uchar         frame_copied_error;
  uchar         r2;
  uchar         token_error;
} tok_adapter_log;
 
/*----------------------------------------------------------------------------*/
/*                MPS POS Register bits                        */
/*----------------------------------------------------------------------------*/
 
/*----------------------------------------------------------------------------*/
/*                MPS Register offset                          */
/*----------------------------------------------------------------------------*/
/* POS register */
#define AID        0x78 	/* adapter ID                                 */
#define BConfig    0x7A    	/* configuration register                     */
#define BMConfig   0x7C    	/* MIC Bus master config register             */
 
/* General Control Register */
#define BCtl       0x60    	/* Basic control register                     */
#define BMCtl_sum  0x50    	/* Bus master control register                */
#define BMCtl_rum  0x52    	/* Bus master control register                */
#define Timer      0x4E 	/* Timer control Register                     */
 
/* Interrupt Status Register */
#define LISR       0x10 	/* Local Interrupt Status Register            */
#define LISR_SUM   0x12 	/* Local Interrupt Status Register            */
#define SISR       0x16 	/* System Interrupt Status Register           */
#define SISR_RUM   0x1A 	/* System Interrupt Status Register           */
#define SISRMask   0x54    	/* System Interrupt Status Mask Register      */
#define MISR       0x5A 	/* Bus Master Interrupt Status Register       */
#define MISRMask   0x5C    	/* Bus Master Interrupt Status Mask Register  */
#define MISRMask_RUM   0x5E    	/* Bus Master Interrupt Status Mask Register  */
 
/* Local Access Port (LAP) Register */
#define LAPA       0x62 	/* Local access port address register         */
#define LAPE       0x64    	/* Local access port Extended address register*/
#define LAPD       0x66 	/* Local access port data/DInc register       */
#define LAPD_I     0x68 	/* Local access port data/DInc register       */
#define LAPWWO     0x6A   	/* Local access port RAM write window open reg*/
#define LAPWWC     0x6C    	/* Local access port RAM write window close   */
#define LAPCtl     0x6E    	/* Local Access Port Access Control Register  */
 
/* System Control Block (SCB) register */
#define SCBSig     0x04 	/* SCB Signalling and control port register   */
#define SCBStat    0x06 	/* SCB status port register                   */
 
/* Micro Channel Bus Master Register - Receive Channel */
#define RxLBDA_L   0x90	 	/* Rx channel last buffer descriptor addr reg */
#define RxLBDA_H   0x92 	/* Rx channel last buffer descriptor addr reg */
#define RxBDA_L    0x94    	/* Rx channel buffer descriptor addr reg      */
#define RxBDA_H    0x96    	/* Rx channel buffer descriptor addr reg      */
#define RxStat_L   0x98    	/* Rx channel status reg                      */
#define RxStat_H   0x9A    	/* Rx channel status reg                      */
#define RxDBA_L    0x9C    	/* Rx channel data buffer address register    */
#define RxDBA_H    0x9E    	/* Rx channel data buffer address register    */
 
/* Micro Channel Bus Master Register - Transmit Channel 1 */
#define Tx1LFDA_L  0xA0 	/* Tx1 channel last frame descriptor addr reg */
#define Tx1LFDA_H  0xA2 	/* Tx1 channel last frame descriptor addr reg */
#define Tx1FDA_L   0xA4    	/* Tx1 channel frame descriptor addr reg      */
#define Tx1FDA_H   0xA6    	/* Tx1 channel frame descriptor addr reg      */
#define Tx1Stat_L  0xA8   	/* Tx1 channel status reg                     */
#define Tx1Stat_H  0xAA    	/* Tx1 channel status reg                     */
#define Tx1DBA_L   0xAC    	/* Tx1 channel data buffer address register   */
#define Tx1DBA_H   0xAE    	/* Tx1 channel data buffer address register   */
 
/* Micro Channel Bus Master Register - Transmit Channel 2 */
#define Tx2LFDA_L  0xB0 	/* Tx2 channel last frame descriptor addr reg */
#define Tx2LFDA_H  0xB2 	/* Tx2 channel last frame descriptor addr reg */
#define Tx2FDA_L   0xB4    	/* Tx2 channel frame descriptor addr reg      */
#define Tx2FDA_H   0xB6   	/* Tx2 channel frame descriptor addr reg      */
#define Tx2Stat_L  0xB8    	/* Tx2 channel status reg                     */
#define Tx2Stat_H  0xBA    	/* Tx2 channel status reg                     */
#define Tx2DBA_L   0xBC    	/* Tx2 channel data buffer address register   */
#define Tx2DBA_H   0xBE    	/* Tx2 channel data buffer address register   */
 
/*----------------------------------------------------------------------------*/
/*               Open adapter error return code set by adapter                */
/*----------------------------------------------------------------------------*/
#define lobe_media_test         0x10  /* Lobe media test                    */
#define physical_insertion      0x20  /* Physical insertion                 */
#define address_verification    0x30  /* Address verification               */
#define roll_call_poll          0x40  /* Roll call poll                     */
#define request_parameters      0x50  /* Request parameters                 */
 
#define function_failure        0x01  /* Function failure                   */
#define signal_los              0x02  /* Signal loss                        */
#define wire_fault              0x03  /* Wire fault                         */
#define ring_speed_mismatch     0x04  /* Ring speed mismatch                */
#define time_out                0x05  /* Timeout                            */
#define ring_failure            0x06  /* Ring failure                       */
#define ring_beaconing          0x07  /* Ring beaconing                     */
#define duplicate_node_address  0x08  /* Duplicate node address             */
#define request_parameter       0x09  /* Request parameters                 */
#define remove_received         0x0A  /* Remove received                    */
#define no_monitor_RPL          0x0D  /* No monitor dected for RPL          */
#define monitor_failed_RPL      0x0E  /* Monitor contention failed for RPL  */
 
/*----------------------------------------------------------------------------*/
/*              MPS Adapter to Host Commands                     */
/*----------------------------------------------------------------------------*/
#define RECEIVE_DATA              0x81
#define LAN_STATUS_CHANGE         0x84
#define SEND_FLASH_DATA           0x98
 
/* Transmit Request Block(TRB) data structure */
struct {
  ushort        reserve;
  uchar       	retcode;
  uchar         reserved[4];
  uchar         transmit_error;
  ushort 	frame_length;
  uchar         buff[MAC_BUFF_LEN];
} trb_com;
 
/* Adapter Request Block(ARB) data structure */
struct {
  uchar         command;
  uchar         reserved[5];
  ushort 	data;
  uchar         tr_hdr_len;
  uchar         reserve;
  ushort        frame_length;
} arb_com;
 
/* Adapter Status Block(ASB) data structure */
struct {
  uchar       command;
  uchar       reserve;
  uchar       retcode;
  uchar       reserves[3];
  uchar       receive_buffer;
} asb_rep;
 
/*----------------------------------------------------------------------------*/
/*               MPS Host to Adapter Commands                    */
/*----------------------------------------------------------------------------*/
 
#define ret_code_size		  0x80
#define INITIALIZATION_COMPLETE   0x80
#define ACCESS_MPC_REGISTER       0x12
#define CLOSE_ADAPTER             0x04
#define CONFIGURE_BRIDGE_CHANNEL  0x0c
#define CONFIGURE_HP_CHANNEL      0x13
#define FLASH_UPDATE              0x19
#define MODIFY_BRIDGE_PARAMETERS  0x15
#define MODIFY_OPEN_OPTIONS       0x01
#define MODIFY_RECEIVE_OPTION     0x17
#define NO_OPERATION              0x00
#define OPEN_ADAPTER              0x03
#define READ_LOG                  0x08
#define READ_SR_BRIDGE_COUNTERS   0x16
#define RESET_GROUP_ADDRESS       0x02
#define RESET_TARGET_SEGMENT      0x14
#define SET_BRIDGE_PARAMETERS     0x09
#define SET_TARGET_SEGMENT        0x05
#define SET_FUNCTIONAL_ADDRESS    0x07
#define SET_GROUP_ADDRESS         0x06
#define TB_REJECT_ADDRESS         0x18
 
/*----------------------------------------------------------------------------*/
/*                 MPS Interrupt Code definitions                */
/*----------------------------------------------------------------------------*/
#define PASSWD_0         	  0x4A
#define PASSWD_1         	  0x41
#define PASSWD_2         	  0x4d
#define PASSWD_3         	  0x45
#define PASSWD_4         	  0x53
#define PASSWD_5         	  0x20
#define PASSWD_6         	  0x20
#define PASSWD_7         	  0x20
 
/*----------------------------------------------------------------------------*/
/*                 MPS Interrupt Code definitions                */
/*----------------------------------------------------------------------------*/
/* Mask interrupt register for LISR */
#define SRB_CMD         0x20     /* Command in SRB                            */
#define ASB_RSP         0x10     /* Response in ASB                           */
#define ASB_REQ         0x04     /* ASB free request                          */
#define ARB_FREE   	0x02     /* ARB free                                  */
#define TRB_FRAME  	0x01     /* Frame in TRB                              */
 
/* Mask interrupt register for SISR */
#define MISR_INT        0x8000   /* MISR interrupt indicate                   */
#define SCB_STAT        0x4000   /* SCB status port interrupt                 */
#define SCB_CTL         0x2000   /* SCB control port interrupt                */
#define SCB_SIG         0x1000   /* SCB signalling port interrupt             */
#define TIMER_EXP       0x0800   /* Timer expired interrupt                   */
#define LAP_PRTY        0x0400   /* LAP data parity error interrupt bit       */
#define LAP_ACC         0x0200   /* LAP access violation error interrupt      */
#define MC_PRTY         0x0100   /* Micro channal parity error interrupt      */
#define ADAPT_CHK 	0x0040   /* Adapter Check interrupt                   */
#define SRB_RSP         0x0020   /* SRB response  interrupt                   */
#define ASB_FREE   	0x0010   /* ASB free interrupt                        */
#define ARB_CMD         0x0008   /* ARB command interrupt                     */
#define TRB_RSP         0x0004   /* TRB response interrupt                    */
 
#define SISR_MSK        0x87FF   /* System interrupt status mask register     */
#define MISR_MSK        0x7737   /* Master interrupt status mask register     */
#define SOFT_RST_MSK    0x8000   /* Soft reset mask bit                       */

/* Rx status */ 
#define RECEIVE_MSK     0x0037   /* Receive interrupt mask register           */
#define Rx_NBA          0x0020   /* Receive channel no buffers available      */
#define Rx_MSK          0x0007   /* Receive channel enf of frame              */
#define Rx_EOB          0x0010   /* Receive channel enf of buffer             */
#define Rx_NOSTA        0x0004   /* Receive Channel No Status Posted          */
#define Rx_HALT         0x0002   /* Receive channel halt                      */
#define Rx_EOF          0x0001   /* Receive channel enf of frame              */
#define RX_DISABLE      0x0040   /* receive disable bit in BMCTL              */

/* Tx Status */ 
#define XMIT_DONE_MSK_2 0x7000   /* Transmit done interrupt mask register     */
#define XMIT_DONE_MSK_1 0x0700   /* Transmit done interrupt mask register     */
#define Tx2_NOST        0x4000   /* Transmit channel 2 No status posted       */
#define Tx2_HALT        0x2000   /* Transmit channel 2 halt interrupt         */
#define Tx2_EOF         0x1000   /* Transmit channel 2 end of frame           */
#define Tx2_DISABLE     0x0400   /* Transmit channel 2 Disable bit in BMCTL   */
#define Tx2_DISABLE_R   0x0800   /* Transmit channel 2 request Disable bit    */
#define Tx1_NOST        0x0400   /* Transmit channel 1 No status posted       */
#define Tx1_HALT        0x0200   /* Transmit channel 1 halt interrupt         */
#define Tx1_EOF         0x0100   /* Transmit channel 1 end of frame           */
#define Tx1_DISABLE     0x4000   /* Transmit channel 1 Disable bit in BMCTL   */
#define Tx1_DISABLE_R   0x8000   /* Transmit channel 1 request Disable bit    */

/* Receive frame status */
#define RX_ERR		 0x7c010000
#define RECV_OVERRUN	 0x00020000
#define PROTOCOL_ERR1	 0x00027E00
#define PROTOCOL_ERR2	 0x00027C00
#define CHCK_ERR	 0x60000000
#define END_OF_FRAME     0x80000000
#define BUF_PROCESS      0x01000000
#define BUF_PROCES       0x00000001
#define VPD_ADDR         0x0001FF00
 
/* Transmit frame status */
#define ERR_STATUS	0x78037E00
#define TX_PROTOCOL_ERR	0x00007E00
#define TX_UNDERRUN	0x00020000
#define MC_ERR		0x78030000

/* ring speed option */
#define FOUR_MBS	0
#define SIXTEEN_MBS	1
#define AUTOSENSE	2

#define CHCK_BIT	0x4000
#define CHCK_DISABLE	0x2008
#define CONF_FLAG	0x80
#define CARD_ENABLE	0x01
#define SEG1            0x01
#define BEACON_MAC      0x80
#define ATTENTION       0x800
#define SPEED16         0x0B
#define SPEED_16        0x01
#define SPEED4          0x0A
#define CHNL_ENABLE     0xBBBF
#define RV_UNMASK       0x0037

#endif 
 
