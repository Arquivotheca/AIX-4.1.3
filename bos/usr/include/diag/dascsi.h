/* @(#)64       1.2  src/bos/usr/include/diag/dascsi.h, cmddiag, bos411, 9428A410j 6/7/94 17:28:30 */
/*
 * COMPONENT_NAME: cmddiag
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define		MAX_MODE_PAGES	0x3f
#define		BLK_DESC_LN_INDEX 3
#define		HEADER_LENGTH 4
#define		RECOVERED_DATA_ERROR	1
#define		UNRECOVERED_DATA_ERROR	2
#define		RECOVERED_EQUIP_ERROR	3
#define		UNRECOVERED_EQUIP_ERROR	4
#define		PC	0x40
#define		PASS_PARAM	DKIOCMD

typedef struct scsi_mode_format {
	int page_index[MAX_MODE_PAGES];         /* offset to page    */
	int sense_length;                       /* length of sense   */
	uint block_length;                      /* device block lgth */
} DISK_MODE_FORMAT;


typedef struct {
        int     percent_complete;
        int     drive_capacity;         /* MB units */
        int     rec_data_errors;
        int     unrec_data_errors;
        int     rec_equ_check_errors;
        int     unrec_equ_check_errors;
        char    loc_code[80];
} DRIVE_INFO ;

typedef struct {
        short   de_card_fru;
        float   soft_data_error_ratio;
        float   soft_equip_error_ratio;
        int     max_hard_data_error;
        int     max_hard_equip_error;
        int     max_soft_data_error;
        int     max_soft_equip_error;
        int     certify_time;
        char    send_diag[5];
        char    mode_data[255];
        int     mode_data_length;
	short	use_subsystem_diag;
} ODM_INFO;

