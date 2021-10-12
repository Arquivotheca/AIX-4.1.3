/* @(#)43	1.2  src/bos/diag/tu/ped4/bim_defs.h, tu_ped4, bos411, 9428A410j 8/2/93 11:55:49 */
/*
 * COMPONENT_NAME: (tu_ped4) Pedernales Graphics Adapter Test Units
 *
 * FUNCTIONS: Declarations only
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * MODULE NAME: bim_defs.h
 *
 * STATUS: Release 1, EC 00, EVT Version 1
 *
 * DEPENDENCIES: None
 *
 * RESTRICTIONS: None
 *
 * EXTERNAL REFERENCES:
 *
 *      OTHER ROUTINES: None
 *
 *      DATA AREAS: None
 *
 *      TABLES: None
 *
 *      MACROS: None
 *
 * COMPILER/ASSEMBLER
 *
 *      TYPE, VERSION: AIX C Compiler
 *
 *      OPTIONS:
 *
 * NOTES: BIM register definitions for PED/PED4. These are compatible with Ped.
 *
 * CHANGE ACTIVITIES:
 *
 *    EC00, VERSION 00 (ORIGINAL), 1/08/91 !KM!
 *
 */


/* DEFINES for BIM register offsets from bim base address. */
/* Since word pointers are used, the offsets are converted from byte offsets. */

#define HOST_INTR_MASK  (0x00080>>2)  /* host_interrupt_mask_reg */
#define HOST_STATUS     (0x00084>>2)  /* host_status_reg         */
#define HOST_COMMO      (0x0008C>>2)  /* host_commo_reg          */
#define DSP_COMMO       (0x00088>>2)  /* dsp_commo_reg           */
#define PRIORITY_IN     (0x01000>>2)  /* priority_in_port        */
#define PIO_FREE        (0x02040>>2)  /* pio_free_reg            */
#define PIO_DATA_IN     (0x02000>>2)  /* pio_data_in_port        */
#define IND_CONTROL     (0x03070>>2)  /* ind_control_reg         */
#define DSP_CONTROL     (0x00068>>2)  /* dsp_control_reg !!NEW!! */
#define IND_ADDRESS     (0x03074>>2)  /* ind_address_reg         */
#define IND_DATA        (0x03078>>2)  /* ind_data_reg            */
#define IO_INTR_MASK    (0x0007C>>2)  /* io_interrupt_mask_reg   */
#define PRIORITY_STATUS (0x01020>>2)  /* priority_status_register !!NEW!! */

/* The following are BIM registers on the DSP side */
/* These are accessible from the HOST only through the special WRAP MODE */

#define BUS_IO_PORT_DSP            0x00
#define BUS_H_ADDR_DSP             0x01
#define HOST_WORKING_ADDR_DSP      0x21
#define BUS_LENGTH_DSP             0x02
#define BUS_STRIDE_DSP             0x03
#define BUS_WIDTH_DSP              0x04
#define BUS_WORKING_WIDTH_DSP      0x05
#define BUS_IO_ADDR_DSP            0x06
#define BUS_IO_LENGTH_DSP          0x07
#define BUS_CONTROL_DSP            0x08
#define BUS_STATUS_DSP             0x09
#define PIO_UPPER_BOUND_ADDR_DSP   0x0B
#define PIO_LOWER_BOUND_ADDR_DSP   0x0C
#define PIO_CURRENT_BOUND_ADDR_DSP 0x0D
#define PIO_FREE_DSP               0x0E
#define PIO_FREE_UPDATE_DSP        0x0F
#define PIO_SUB_PAGE_DSP           0x10
#define PIO_HIGH_WATER_DSP         0x11
#define PIO_STATUS_DSP             0x12
#define PRIORITY_OUT_PORT_DSP      0x13
#define PRIORITY_STATUS_DSP        0x14
#define IO_INTR_MASK_DSP           0x16
#define IO_STATUS_DSP              0x17
#define DSP_COMMO_DSP              0x18
#define HOST_COMMO_DSP             0x19
#define DSP_SOFT_INTR_DSP          0x20

/* BIM indirect access modes (OR'ed into the indirect control register) */
#define IND_WRAPMODE     0x00      /* Destination = BIM registers      */
#define IND_DSPMEM       0x08      /* Destination = DSP memory         */
#define IND_WRITE        0x00      /* Request write operation          */
#define IND_READ         0x10      /* Request read operation           */
#define IND_AUTOINC      0x20      /* Auto increment                   */

/* DSP control values (written to the DSP CONTROL REGISTER) */
#define RESET_DSP   0  /* holds DSP in reset */
#define RELEASE_DSP 1  /* releases DSP (puts DSP in running state) */

/* HOST INTERRUPT MASK/STATUS bit definitions */
#define HIGH_WATER_REACHED  0x001
#define WRITE_TO_DSP_COMMO  0x002
#define BAD_PARITY_RECEIVED 0x004
#define DSP_READ_HOST_COMMO 0x008
#define SOFT_INTERRUPT_0    0x010
#define SOFT_INTERRUPT_1    0x020
#define SOFT_INTERRUPT_2    0x040
#define SOFT_INTERRUPT_3    0x080
#define PRIORITY_FIFO_EMPTY 0x100
#define PRIORITY_FIFO_FULL  0x200

/* BIM POS register masks */
#define POS_5_ENABLE_VPD_0          0 /* Enable first VPD */
#define POS_5_ENABLE_VPD_1          2 /* Enable second VPD */
#define POS_5_ENABLE_VPD_2          4 /* Enable third VPD */
#define POS_5_DISABLE_CHANNEL_CHECK 8 /* Disable channel checks            */
#define POS_5_AUTO_INC_ENABLE       1 /* Enable POS address auto-increment */
