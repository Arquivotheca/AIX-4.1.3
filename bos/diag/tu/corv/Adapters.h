/* @(#)31       1.1  R2/cmd/diag/tu/corv/Adapters.h, tu_corv, bos325 7/22/93 18:57:16 */
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
#ifndef ADAPTERS
#define ADAPTERS

#define ADAPTER_COMMAND COMMAND
#define REPLY_ELEMENT COMMAND
#define ADAPTER_RESPONSE char*
typedef unsigned long int ADAPTER_HANDLE; 

extern unsigned char Interrupt_Status_Register;

#define ADAPTER_TIMEOUT_ERROR -1

typedef struct {
     char *handle;                  /* handle for adapter used by diagex */
     unsigned int card_id;          /* card id for adapter               */
     char *dds;                     /* device structure used by diagex   */
     char *vpd_info;                /* vendor product data for adapter   */
     int error_log[100];            /* contains errors on adapter        */
     int num_errors;                /* contains number of errors         */
     char *adapter_specific_info;   /* adapter defined information       */
     int adapter_specific_info_length;
} ADAPTER_STRUCT;

typedef struct {
     char *user_address;
     char *page_address;
     unsigned long dma_page;
     unsigned long dma_address;
} DMA_STRUCT;

DMA_STRUCT dma_allocate();

extern void post_error();
extern unsigned int get_card_id();
extern int initialize_adapter_info();
extern void *adapter_open();
extern int adapter_close();
extern unloadkext();
extern int format_TSB_data();

#endif
