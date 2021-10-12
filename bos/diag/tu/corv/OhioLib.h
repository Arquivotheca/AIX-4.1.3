/* @(#)31       1.1  R2/cmd/diag/tu/corv/OhioLib.h, tu_corv, bos325 7/22/93 18:57:16 */
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _OHIO
#define _OHIO


typedef int OHIO_STRUCT;


#define _RESERVED 0
#define _CEN 1
#define _POS_3 0x0000
#define _POS_3A 0x0100
#define _POS_3B 0x0101

/* Defined ranges for Attention Register */
#define _LOCATE_SCB 0x3
#define _IMMEDIATE_COMMAND 0X1

/* Defined ranges for Basic Control Register */
#define _ENABLE_SYSTEM_INTERRUPT 1
#define _DISABLE_SYSTEM_INTERRUPT 0
#define _ENABLE_BUS_MASTER_DMA 1
#define _DISABLE_BUS_MASTER_DMA 0
#define _START_SUBSYSTEM_RESET 1
#define _DISABLE_SUBSYSTEM_RESET 0

/* Defined ranges for POS Register 2: Access Control */
/* No defined values */


/* Defined ranges for POS Register 3: DMA Control */
#define _ENABLE_ASYNC_CHANNEL_CHECKS 1
#define _DISABLE_ASYNC_CHANNEL_CHECKS 0
#define _ENABLE_PARITY 1
#define _DISABLE_PARITY 0
#define _ENABLE_FEEDBACK 1
#define _DISABLE_FEEDBACK 0
#define _ENABLE_STREAMING 1
#define _DISABLE_STREAMING 0

/* Defined ranges for POS Register 3A: I/O Control */
/* No defined values */


/* Defined ranges for POS Register 3B: Channel 2 Control */
#define _DISABLE_ALTERNATIVE_DMA_LEVEL 0


/* Defined ranges for POS Register 5: Master Control */
/* No defined values */


/* Defined ranges for POS Register 4B: Slave Control */
#define _ENABLE_CHANNEL_CHECK 1
#define _DISABLE_CHANNEL_CHECK 0

#define STAT_ENTRIES 18

typedef struct {
	int word_value;
	char description[100];
} STAT_WORD;


STAT_WORD TSB_Status_Word1[] = { {0x0100, "Error Prevented Completion"},
				 { 0x0200, "Short-Length Exception" },
				 { 0x0400, "Device Dependent Info" },
				 { 0x0800, "Device Dependent Info" },
				 { 0x0C00, "Device Dependent Info" },
				 { 0x1000, "Specification Check" },
				 { 0x2000, "Long-Length Exception" },
				 { 0x4000, "Reserved" },
				 { 0x8000, "Interrupt Requested" },
				 { 0x01, "TSB Available" },
				 { 0x02, "TSB Available" },
				 { 0x03, "TSB Available" },
				 { 0x04, "Device Overrun" },
				 { 0x08, "Initialize" },
				 { 0x10, "Major Error" },
				 { 0x20, "Chain Field Valid" },
				 { 0x40, "Suspended" },
				 { 0x80, "Extended Status" } };

char TSB_Status_Word2[][100] = { "Compression Not Complete Error",
                                "Reserved",
				"Original Data Interface Parity Error"
                                "Original Data Interface CRC Error",
                                "Original Data Interface Overflow Error",
                                "Original/Compressed Data Interface Timeout Error",
                                "Original/Compressed Data Length Overflow Error",
                                "Original Data Length Underflow Error",
                                "Compressed Data Interface Parity Error",
                                "Compressed Data Interface CRC Error",
                                "Compressed Data Interface Overflow Error",
                                "Compressed Data Interface Timeout Error",
                                "Compressed Data Interface Length Overflow Error",
                                "Compressed Data Length Underflow Error",
                                "FIFO CRC Error",
                                "Original Data Read Back Check CRC Error",
                                "Compressed Data Read Back Check CRC Error",
                                "Channel Write Without Compression CRC Error",
                                "Channel Read Without Compression CRC Error",
                                "Decoder Done Timeout Error",
                                "Compression and Decompression History Buffer Sizes Not Equal",
                                "No History Buffer or Invalid History Buffer Sizes Selected",
                                "No History Buffer or Invalid History Buffer Sizes Selected",
                                "FIFO Underflow/Overflow Error",
                                "FIFO Underflow/Overflow Error",
                                "Control Code Detected in Compression Data Stream",
                                "Decompression End Error",
                                "Microchannel Address Parity Error",
                                "Loss of Channel Indicator",
                                "Invalid Data Strobe Signal Combination",
                                "Card Selected Feedback Return Indicator Not Asserted",
                                "Channel Check Occured",
                                "Microchannel Data Parity Error",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Reserved",
                                "Microchannel Data Transfer Timeout",
                                "Compression Timeout Error",
                                "Non-supported Command",
                                "Chain ID field non-zero",
                                "Address not divisible by 4",
                                "Size of Data Block Exceeds Maximum",
                                "Invalid Run Diagnostic Sub-Command",
                                "Microcode Download Pairing Error",
                                "Reserved",
                                "Data Block Length Zero",
                                "Reserved",
                                "Compression Chip Internal Error",
                                "Reserved Bits Set",
                                "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
                                "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved" };




#endif
