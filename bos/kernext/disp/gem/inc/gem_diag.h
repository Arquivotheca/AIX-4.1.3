/* @(#)49       1.8.1.8  src/bos/kernext/disp/gem/inc/gem_diag.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:42:05 */
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


#define GEM_START_DIAG    0x10
#define GEM_QUERY_DIAG    0x11
#define GEM_STOP_DIAG     0x12
#define GEM_LOAD_UCODE    0x13
#define GEM_GET_STARTADDR 0x14
#define GEM_RET_STARTADDR 0x15
#define GEM_DMA_DIAG      0x16
#define SET_DISPLAY_HZ    0x17
#define GEM_DIAG_MODE     0xFF

#define GEM_DMA_SUCCESS   0x0000
#define GEM_DMA_READ      0x0000
#define GEM_DMA_WRITE     0x0001

/***************************************************************/
/* DMA Error Codes found in gem_dma_diag data structure        */
/***************************************************************/
#define XMATTACH_ERROR    0x0001
#define PINU_ERROR        0x0002
#define BUS_PARITY_ERROR  0x0003
#define WAIT_EVENT_ERROR  0x0004
#define DMA_ERROR         0x0005
#define XMDETACH_ERROR    0x0006
#define UNPINU_ERROR      0x0007

/***************************************************************/
/* GEMINI Diagnostic Openx structure                           */
/***************************************************************/
struct diag_open
{
	ulong diag_id;               /* init to 0x8ffd         */
	ulong flags;                 /* 0 = do not load ucode  */
				     /* 1 = load ucode         */
};

/***************************************************************/
/* GEMINI Diagnostic Interrupt Info Structure                  */
/***************************************************************/
struct gem_intr_count
{
	ulong sync_cnt[4];
	ulong thresh_cnt[8];
	ulong cvme_gcp_id;
	ulong cvme_drp_id;
	ulong cvme_shp_id;
	ulong cvme_imp_id;
	ulong mbc_status;
};

/**********************************************************************/
/* GEMINI DMA Diagnostic Request structure                            */
/**********************************************************************/
struct gem_dma_diag
{
	struct _dma_ctls {            /* DMA Diagnostic Controls      */
	  ushort dma_diag_error;     /* DMA Diagnostic Error code    */
				     /* 0 = successful               */
	  ushort dma_diag_flags;     /* DMA Diagnostic Req Flags     */
				     /* 0 = read 1 = write           */
	} dma_ctls;
	ulong dma_buff_len;          /* Length of DMA buffer in bytes*/
	char  *dma_buff_ptr;         /* Address of DMA Buffer        */
	ulong dma_dest_ptr;          /* Global memory address        */
};

/**********************************************************************/
/* GEMINI VPD Request structure                                       */
/**********************************************************************/
struct vpd_data
{
	unsigned char uch_vpd[256];
	unsigned char mgc_vpd[256];
	unsigned char gcp_vpd[128];
	unsigned char drp_vpd[128];
	unsigned char fbb_vpd[128];
	unsigned char shp_vpd[128];
	unsigned char imp_vpd[128];
	int  uch_slot;
	int  mgc_slot;
	int  gcp_slot;
	int  drp_slot;
	int  fbb_slot;
	int  shp_slot;
	int  imp_slot;
};
