/* @(#)02	1.3.1.1  src/bos/kernext/audio/acpa/oldacpahw.h, sysxacpa, bos411, 9428A410j 7/22/92 21:50:28 */

/*
 * COMPONENT_NAME: SYSXACPA     Multimedia Audio Capture and Playback Adapter
 *
 * FUNCTIONS:
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

/* Source of magic numbers:  G571-0181-00, page 6-9 */

#define ACPA_POS_ID 0x6e6c	/* The POS id assigned to the ACPA card */
#define ACPA_POS_ID0 0x6c
#define ACPA_POS_ID1 0x6e
/*--------------------------------------------------------------------------*/
/* Definition of the ACPA card's I/O ports.  The card uses 8 addresses      */
/* starting at acpa_base; the #defines define which ports do what           */
/*--------------------------------------------------------------------------*/

#define DATA_LOW_REG (0)	/* Low order byte of data read or written   */
#define DATA_HI_REG (1)		/* Hi order byte of data read or written    */
				/* NOTE--> Using the HI reg increments the  */
				/* address field, so accesses must be       */
				/* LOW then HI in that order.               */
#define ADDR_LOW_REG (4)	/* Low order byte of addr to read/write     */
#define ADDR_HI_REG (5)		/* Hi order byte of addr to read/write      */
#define CMD_REG (6)		/* Command register = acpa_base + CMD_REG   */
#define STATUS_REG (6)		/* Status register                          */

/* Masks in the Status register */

#define TMS_INT_STAT (0x01)	/* 0 = int pending from host to TMS */
				/* 1 = int from host serviced by TMS */
#define HST_REQ (0x02)		/* 0 = request from TMS serviced by host */
				/* 1 = request pending from TMS to host */

/* Bits in the Command register */

#define TMS_RES (0x01)		/* 0 = TMS is in the reset state (latched) */
				/* 1 = TMS is in operating state (latched) */
#define HREQACK (0x02)		/* 0 = clears int from TMS (strobed)       */
				/* 1 = does not clear int from TMS         */
#define HINTENA (0x04)		/* 0 = TMS can't interrupt host (latched)  */
				/* 1 = TMS can interrupt host (latched)    */
#define TMS_INT_CMD (0x08)	/* 0 = interrupts the TMS (strobed)        */
				/* 1 = does not interrupt the TMS          */
#define SKPR_EN (0x10)		/* 0 = Disables the internal PS/2 speaker  */
				/* 1 = Enables the internal PS/2 speaker   */
/*----------------------------------------------------------------------
 * Functions which are defined by the DSP microcode in use.  Source:
 * "AVC DSP Programming Description & Technical Reference 8.02" by
 * R. J. Lisle, IBM Austin
 *----------------------------------------------------------------------*/

/* Fields defined in the "Master Control Block" in "Shared Memory"        */

#define DSP_INPUT_SELECT (0x0e10) /* Used to select input source..values: */
#define   INPUT_FROM_HIGAIN_MIKE (0) /* Input from high gain microphone   */
#define   INPUT_FROM_LOGAIN_MIKE (4) /* Input from low gain microphone    */
#define   INPUT_FROM_LINE_LEFT   (1) /* Input from line left              */
#define   INPUT_FROM_LINE_RIGHT  (2) /* Input from line right             */
#define   INPUT_FROM_LINE_BOTH   (3) /* Input from line left and right    */

#define DSP_MONITOR_SELECT (0x0e11) /* Determines monitor mode (recording)*/

#define DSP_TMS_INT_SOURCE (0x0e12) /* Which channel caused TMS interrupt */
#define   TMS_INT_FROM_T1        (0) /* Interrupt caused by track 1       */
#define   TMS_INT_FROM_T2        (1) /* Interrupt caused by track 2       */

#define DSP_TMS_INT_INHIBIT_FLAG (0x0e13) /* Interrupts allowed? */
#define   TMS_CAN_INTERRUPT      (0) /* DSP can interrupt PS/2 */
#define   TMS_CAN_NOT_INTERRUPT  (1) /* DSP can not interrupt PS/2 */

#define DSP_T1_LEVEL_INDICATOR (0x0e14)	/* Level of left channel */
#define DSP_T2_LEVEL_INDICATOR (0x0e15)	/* Level of right channel */
#define DSP_FREE_TIME_COUNT (0x0e16) /* cycles*10 per 344 ints/sec */

/* Definition of the Track 1 and Track 2 Control Blocks                   */

#define DSP_T1_CONTROL_BLOCK_BASE (0x0e00) /* Beginning of T1 control block */
#define DSP_T2_CONTROL_BLOCK_BASE (0x0e08) /* Beginning of T2 control block */

#define DSP_TX_MASTER_VOLUME  (0) /* Offset of master volume (0-7fff)   */
#define DSP_TX_TRACK_VOLUME   (1) /* Offset of track volume (0-7fff)    */
#define DSP_TX_TRACK_VOL_RATE (2) /* Offset of track volume rate        */
				  /* 0-ffff; n. msecs to change volume  */
#define DSP_TX_TRACK_BALANCE  (3) /* Offset of track balance (0-7fff)   */
#define DSP_TX_TRACK_BAL_RATE (4) /* Offset of track balance rate       */
				  /* 0-fff; n. misecs to change balance */
#define DSP_TX_AUDIO_TYPE     (5) /* Type of audio in use               */
#define   TYPE_MUSIC            (0)  /* Music (mono 11Khz samples)      */
#define   TYPE_VOICE            (1)  /* Voice (mono 5.5Khz samples)     */
#define   TYPE_STEREO_MUSIC     (2)  /* Stereo music (11Khz samples x 2)*/

/* Definition of the Track 1 and Track 2 mailboxes                      */

#define DSP_T1_MAILBOX_BASE (0x0e18) /* Beginning of T1 mailbox         */
#define DSP_T2_MAILBOX_BASE (0x0e20) /* Beginning of T2 mailbox         */

#define DSP_TX_CMD_FROM_HOST (0) /* Command from host */
#define DSP_TX_DSP_BUF_PTR   (1) /* DSP's buffer pointer (0-3) */
#define DSP_TX_CMD_FROM_DSP  (2) /* Command from DSP to host */
#define DSP_TX_HOST_BUF_PTR  (3) /* Host buffer pointer (0-3) */
#define DSP_TX_BUFFER_CNT    (4) /* Buffer count */
#define DSP_TX_TMS_STATUS    (5) /* Track status */
#define DSP_TX_LEVEL         (6) /* Playback avg level or record peak level */

#define PCM_T1_CONTROL_PARMS  ( 0x0e30 )  /* Beginning of PCM's first track */
					  /* control parameters */
#define PCM_T2_CONTROL_PARMS  ( 0x0e50 )  /* Beginning of PCM's second track */
					  /* control parameters */
#define PCM_INTERRUPT_COUNTER ( 0 )       /* the interrupt counter */
#define PCM_TYPE              ( 1 )       /* normal, mu-law, etc. types */
#define PCM_SAMPLE_RATE       ( 2 )       /* selected sampling rate */
#define PCM_DITHER_LEVEL      ( 3 )       /* desired dither level in % */
#define PCM_SAMPLE_WIDTH      ( 4 )       /* sample width in bits */
#define PCM_BYTE_SWAPPING     ( 5 )       /* whether to enable byte swapping */
#define PCM_NUMBER_FORMAT     ( 6 )       /* where is most significant bit? */
#define PCM_NUMBER_CHANNELS   ( 8 )       /* one or two channels? */
#define PCM_SOURCE_MIX        ( 11 )      /* mix source input into output */

/* Definition of the various buffers for host<-->DSP data                */

#define VOICE_BUFFER_SIZE  (288) /* Size of a voice buffer (words) */
#define MUSIC_BUFFER_SIZE  (564) /* Size of a music buffer (words) */
#define CD_BUFFER_SIZE     (564) /* Size of a CD quality buffer (words) */
#define STEREO_BUFFER_SIZE (1128) /* Size of a stereo music buffer (words) */

/* Masks defining the minor device number.  The minor number encodes 
 * information about the source/sink, speed, and channel of the request.
 */

#define MASK_SPEED (0300)	/* Mask to determine speed */
#define   SPEED_VOICE (0)	/* Voice speed (5.5Khz ADPCM encoded)  */
#define   SPEED_MUSIC (0100)	/* Music speed (11Khz ADPCM encoded)   */
#define   SPEED_CD    (0200)    /* CD speed (44.1KHz PCM encoded)      */

#define MASK_SOURCE (0070)	/* Mask to determine source or sink    */
#define   SOURCE_LINE (0010)	/* Line input/output                   */
#define   SOURCE_SPKR (0020)	/* Speaker/headphone jack output       */
#define   SOURCE_PS2  (0030)	/* PS/2 internal speaker output        */
#define   SOURCE_HIMIKE (0040)	/* Hi gain microphone input            */
#define   SOURCE_LOMIKE (0050)	/* Lo gain microphone input            */
#define   SOURCE_RESERVED1 (0060) /* Not supported */
#define   SOURCE_RESERVED2 (0070) /* Not supported */

#define MASK_CHANNEL (0003)	/* Mask to determine channel(s)        */
#define   CHANNEL_LEFT (0001)	/* Left channel to be used             */
#define   CHANNEL_RIGHT (0002)	/* Right channel to be used            */
#define   CHANNEL_BOTH (0003)	/* Both channels (stereo)              */

#define MASK_CARD (03000)	/* Which of the 4 possible ACPA cards */
				/* are we using? */
#define CARD_SHIFT (9)		/* Shift right to get card 0-3 */

/* There are three special minor numbers for special purposes */

#define MINOR_SHARED_MEMORY (0)	/* Minor device number for reading */
				/* shared memory */
#define MINOR_RECORD_UCODE  (1)	/* Write recording DSP code to the */
				/* kernel from /etc/rc */
#define MINOR_PLAY_UCODE    (2)	/* Write playback DSP code to the */
				/* kernel from /etc/rc */
#define MINOR_RECORD_44K    (3) /* Write 44KHz recording DSP code to */
				/* the kernel from /etc/rc */
#define MINOR_PLAY_44K      (4) /* Write 44KHz playback DSP code to */
				/* the kernel from /etc/rc */
