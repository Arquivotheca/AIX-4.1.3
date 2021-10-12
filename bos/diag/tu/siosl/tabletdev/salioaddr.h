/* @(#)59       1.2  src/bos/diag/tu/siosl/tabletdev/salioaddr.h, tu_siosl, bos411, 9428A410j 11/18/93 17:27:29 */
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* RSC/POWER I/O addresses
*/

/* Ethernet :			*/

#define	ETHER_ID0_REG 		0x4e00
#define	ETHER_ID1_REG 		0x4e01
#define	ETHER_CONTROL_REG 	0x4e02
#define	ETHER_PORT_REG 		0xffc0

/* Floppy :				*/

#define	FP_STATUS_A_REG 	0xff60
#define	FP_STATUS_B_REG 	0xff61	
#define	FP_DIG_OUT_REG 		0xff62	
#define	FP_MAIN_ST_REG 		0xff64
#define	FP_DATA_RATE_REG 	0xff64
#define	FP_DATA_FIFO_REG 	0xff65
#define	FP_DIG_IN_REG 		0xff67
#define	FP_CONF_CTRL_REG 	0xff67
#define	FP_FIFO_THR_REG 	0xff68	

/* Keyboard :   		*/
	
#define	KBD_DATA_REG 		0x0050
#define	KBD_STAT_CMD_REG 	0x0051

/* Bits mapped for key board command and stat reg   */
#define	KBD_CLOCK_LOW      0x01
#define	KBD_HARDWARE_RESET 0x02
#define	KBD_LOOP_MODE      0x04
#define	KBD_DEVICE_DATA    0x08
#define	KBD_DEVICE_CLOCK   0x10
#define	KBD_TX_BUF_EMPTY   0x20
#define	KBD_RX_BUF_FULL    0x40
#define	KBD_STAT_ERROR     0x80

/* Below are defines for RSC NVRAM and LED locations */

#define NVRAM_ADDR_RSC			0xa00000
#define	LED_REG_RSC 			0xa00300

/* Below are defines for POWER NVRAM and LED locations */
#define NVRAM_ADDR_POWER		0xFF600000
#define	LED_REG_POWER 			0xFF600300

/* Mouse :				*/

#define	MOUSE_DATA_TX_REG 		0x0048	
#define	MOUSE_CMD_REG    	        0x0049	
#define	MOUSE_STAT_REG   		0x004a	
#define	MOUSE_RX_STAT_REG 		0x004c	
#define	MOUSE_RX_1_REG 			0x004d	
#define	MOUSE_RX_2_REG 			0x004e	
#define	MOUSE_RX_3_REG 			0x004f

/* Values set for mouse command register  */
#define	MOUSE_CMD_TX_DATA    	        0x00	
#define	MOUSE_CMD_DIAG_WRAP  	        0x01	
#define	MOUSE_CMD_ENABLE_INTERRUPT      0x02	
#define	MOUSE_CMD_DISABLE_INTERRUPT     0x03	
#define	MOUSE_CMD_ENABLE_BLK_MODE       0x04	
#define	MOUSE_CMD_DISABLE_BLK_MODE      0x05	
 
/* Bits mapped for mouse status register  */
#define	MOUSE_STAT_INTERRUPT_ENABLED    0x01	
#define	MOUSE_STAT_BLK_MODE_ENABLED     0x02	
#define	MOUSE_STAT_BLK_MODE_DISABLED    0x04	
 
/* Bits mapped for mouse receive status register */
#define	MOUSE_STAT_RX_INTERRUPT         0x01	
#define	MOUSE_STAT_RX_DATA_ERROR        0x02	
#define	MOUSE_STAT_RX_INTF_ERROR        0x04	
#define	MOUSE_STAT_RX_REG1_FULL         0x08	
#define	MOUSE_STAT_RX_REG2_FULL         0x10	
#define	MOUSE_STAT_RX_REG3_FULL         0x20	

/* Parallel port :		     */

#define	PP_DATA_REG 	0xff78
#define	PP_STATUS_REG 	0xff79
#define	PP_CONTROL_REG 	0xff7a

/* SCSI :				*/

#define	SCSI_ID0_REG 		0x4d00	/* in LEON			*/
#define	SCSI_ID1_REG 		0x4d01
#define	SCSI_CONTROL_REG 	0x4d02

#define	SCSI_CONTROL0_REG 	0xff80	/* in SCSI chip		*/
#define	SCSI_CHIPID_REG 	0xff84	
#define	SCSI_RESETST_REG 	0xffbc	/* Reset/Status		*/	

/* Serial Port A :		*/
	
#define	SERIAL_TX_BUFF_REG 		0xff30
#define	SERIAL_RX_BUFF_REG 		0xff30
#define	SERIAL_DIV_LSB_REG 		0xff30
#define	SERIAL_DIV_MSB_REG 		0xff31
#define	SERIAL_INT_ENBL_REG 	0xff31
#define	SERIAL_INT_ID_REG 		0xff32
#define	SERIAL_FIFO_CTRL_REG 	0xff32
#define	SERIAL_ALT_FCN_REG 		0xff32
#define	SERIAL_LINE_CTRL_REG 	0xff33
#define	SERIAL_MODEM_CTRL_REG 	0xff34
#define	SERIAL_LINE_ST_REG 		0xff35
#define	SERIAL_MODEM_ST_REG 	0xff36
#define	SERIAL_SCRATCH_REG 		0xff37

/* Serial Port B :		*/

#define	SERIAL_PORT_B_OFFSET	0x8	/* Add this offset to the previous addr. */
									/* to obtain PORT_B registers			 */

#define	SERIAL_INT_REG 	0xff40		/* these 2 registers are in LEON	*/
#define	SERIAL_DMA_REG 	0xff41

/* Standard I/O POS registers :		*/

#define	SIO_ID0_REG 		0x004f0000
#define	SIO_ID1_REG 		0x004f0001
#define	SIO_CONTROL_REG 	0x004f0002
#define POS3_REG                0x004f0003
#define POS6_REG                0x004f0006

/* Bit mask for standard I/O registers   */

#define	SIO_REG_ENABLE		0x01  
#define	SIO_REG_PARITY_ENABLE   0x02
#define	SIO_REG_RESET_KBD       0x04
#define	SIO_REG_RESET_DSKT      0x08
#define	SIO_REG_RESET_PORT      0x10
#define	SIO_REG_RESERVED        0x20
#define	SIO_REG_DIAG_READ       0x40
#define	SIO_REG_RESET_MOUSE     0x80
#define	SIO_REG_RESET_MOUSE_MSK 0x7F
 
/* Key board data input/output   */

#define	KBD_RESET_CMD           0xff
#define	KBD_BAT_CC              0xaa      /* BAT completion code         */
#define	KBD_ACK                 0xfa      /* Keyboard acknowlege         */
#define	REPEAT_COUNT            100       /* Number of time to retry     */

/* Slot0 			*/

#define	SLOT0_ID0_REG 	0x4000
#define	SLOT0_ID1_REG 	0x4001

/* Speaker 			*/

#define	SPK_CTRL0_REG 	0x0054		/* Speaker Control Byte 1 */
#define	SPK_CTRL1_REG 	0x0055		/* Speaker Control Byte 2 */

/* Masks for speaker control registers  */

#define	SPK_ENABLE       0x80  		/* Speaker enable command        */
#define	SPK_VOL_OFF      0x9f  		/* Speaker control off           */
#define	SPK_VOL_LOW      0x20  		/* Speaker control low volume    */
#define	SPK_VOL_MEDIUM   0x40  		/* Speaker control medium volume */
#define	SPK_VOL_HIGH     0x60  		/* Speaker control high volume   */

/* Tablet :				*/

#define	TABLET_TX_BUFF_REG 		0x0070
#define	TABLET_RX_BUFF_REG 		0x0070
#define	TABLET_DIV_LSB_REG 		0x0070
#define	TABLET_DIV_MSB_REG 		0x0071
#define	TABLET_INT_ENBL_REG 		0x0071
#define	TABLET_INT_ID_REG 		0x0072
#define	TABLET_FIFO_CTRL_REG 		0x0072
#define	TABLET_ALT_FCN_REG 		0x0072
#define	TABLET_LINE_CTRL_REG 		0x0073
#define	TABLET_MODEM_CTRL_REG 	 	0x0074
#define	TABLET_LINE_ST_REG 		0x0075
#define	TABLET_MODEM_ST_REG 		0x0076
#define	TABLET_SCRATCH_REG 		0x0077

/* Bit mask for tablet registers */

#define	TABLET_MODEM_FUSE_STAT 		0x10

/* System registers :		 */
/* Below are system register defines for RSC */

#define	COMP_RESET_REG_RSC 	0x0040002c	/* Component Reset Register */
#define	TOD_IDX_REG_RSC 	0x004000c0	/* Time of Day	*/
#define	TOD_DATA_REG_RSC 	0x004000c4
#define	POWER_STAT_REG_RSC 	0x004000e4	/* Power Status Register */

/* Below are system register defines for POWER */
 
#define	COMP_RESET_REG_POWER 	0x000100A0	/* Component Reset Register */
#define	TOD_IDX_REG_POWER 	0xFF0000C0	/* Time of Day	*/
#define	TOD_DATA_REG_POWER 	0xFF0000C4
#define	POWER_STAT_REG_POWER 	0xFF0000e4	/* Power Status Register */

