/* @(#)31       1.1  src/bos/diag/tu/corv/libadapters.h, tu_corv, bos411, 9428A410j 7/22/93 18:57:16 */
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

#include <stdio.h>

#define ADAPTER_COMMAND COMMAND
#define ADAPTER_RESPONSE char*
typedef unsigned long int ADAPTER_HANDLE;

#define ADAPTER_TIMEOUT_ERROR -1

#define NO_PRINTING 0x00
#define SCREEN_PRINTING 0x01
#define FILE_PRINTING 0x02
#define VERBOSE 0x03
#define CRYPTIC 0x04

typedef struct {
     unsigned char trace_printing;
     unsigned char trace_format;
     FILE *trace_fp;
     unsigned char error_printing;
     unsigned char error_format;
     FILE *error_fp;
     unsigned char record_printing;
     FILE *record_fp;
     int timeout_value;
     char interrupt_routine[100];
#ifdef OHIO
     char device_name[30];
#elif CORVETTE
     char device_name[30];
#endif
} ENV_CONTROLS;

typedef struct {
     char *handle;                              /* handle for adapter used by diagex   */
     unsigned int card_id;                      /* card id for adapter                 */
     char *dds;                                 /* device structure used by diagex     */
     char *vpd_info;                            /* vendor product data for adapter     */
     int error_log[100];                        /* contains errors on adapter          */
     int num_errors;                            /* contains number of errors           */
     char *adapter_specific_info;               /* adapter defined information         */
     int adapter_specific_info_length;          /* size of adapter defined information */
     ENV_CONTROLS environment;                  /* environment variable settings       */
} ADAPTER_STRUCT;

typedef struct {
     char *user_address;
     char *page_address;
     unsigned long dma_page;
     unsigned long dma_address;
} DMA_STRUCT;

DMA_STRUCT dma_allocate();

#endif

