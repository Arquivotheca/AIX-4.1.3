/* @(#)54       1.1  src/bos/usr/include/diag/scsd.h, cmddiag, bos41B, 9505A 1/11/95 11:35:48 */
#ifndef _H_DIAGSCSD
#define _H_DIAGSCSD
/*
 * COMPONENT_NAME: cmddiag
 *
 * FUNCTIONS: SCSD header file
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* The following SCSD structure is used for SCSD
   CD-ROMs, Disks, and R/W optical drives.
*/


#pragma options align=packed
struct disk_diag_scsd_cuat {
        uchar   operation_flags;        /* Miscellaneous flags               */
        uchar   diagdata1;              /* Future diag flags.                */
	ushort data_err_mult;
	uchar data_err_base_ten;
	ushort equip_chk_mult;
	uchar equip_chk_base_ten;
	ushort err_rate_mult;
	uchar err_rate_base_ten;

};
#pragma options align=reset

#define NO_INQ_PAGE         -1	      /* If no extended SCSI inquiry is needed*/

#define DIAG_SCSD_CERT_TIME	1.0/80.0  /* This fraction represents 1 min */
					   /* to format 80 MB                */


#define SCSD_LEN_DIAG2  9      /* Length of SCSD diag data2   */
#define SCSD_MODE_LENGTH  150
#define SCSD_CD_MEDIA_PN  12
/* 
 *		  Common Header for SCSD VPD page
 * +=====-=======-=======-=======-=======-=======-=======-=======-=======+
 * |  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * |Byte |       |       |       |       |       |       |       |       |
 * |=====+===============================================================|
 * | 0   |   0       0       0   |    Peripheral Device Type             |
 * |-----+---------------------------------------------------------------|
 * | 1   |   			Page Code = 0xc7		    	 |
 * |-----+---------------------------------------------------------------|
 * | 2   |   	Reserved = 0					      	 |
 * |-----+---------------------------------------------------------------|
 * | 3   |      Page Length =    				     	 |
 * |-----+---------------------------------------------------------------|
 * | 4   |      Length of SCDD ID field = 4			         |
 * |-----+---------------------------------------------------------------|
 * | 5-8 |      SCDD						     	 |
 * |-----+---------------------------------------------------------------|
 */
struct  scsd_inqry_header	{
	uchar   pdevtype;		/* Peripheral Qualifier & device type*/
	uchar	page_code;		/* SCSD Page Code Number	     */
	uchar	resvd1;			/* Reserved for future use.	     */
	uchar	page_length;		/* Length of this page in bytes	     */
	uchar	scsd_id_length;		/* Length of SCSD keyword in bytes   */
	uchar	scsd_id_field[4];	/* SCSD keyword identification field */
	uchar   misc[260];

};

/* Inquiry format for SCSD VPD page for CD-ROM and Read/Write Optical Drives */
/* 
 *			CD-ROM and Read/Write Optical SCSD VPD page
 * +=====-=======-=======-=======-=======-=======-=======-=======-=======+
 * |  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * |Byte |       |       |       |       |       |       |       |       |
 * |=====+===============================================================|
 * | 0   |   0       0       0   |    Peripheral Device Type             |
 * |-----+---------------------------------------------------------------|
 * | 1   |   			Page Code = 0xc7		    	 |
 * |-----+---------------------------------------------------------------|
 * | 2   |   	Reserved = 0					      	 |
 * |-----+---------------------------------------------------------------|
 * | 3   |      Page Length = 0x60				     	 |
 * |-----+---------------------------------------------------------------|
 * | 4   |      Length of SCDD ID field = 4			         |
 * |-----+---------------------------------------------------------------|
 * | 5-8 |      SCDD						     	 |
 * |-----+---------------------------------------------------------------|
 * | 9   |      Number of LUN's supported by device		     	 |
 * |-----+---------------------------------------------------------------|
 * |10-13|  	Storage Capacity in Megabytes			     	 |    
 * |-----+---------------------------------------------------------------|
 * |14   |  0    |Tray   | SP Flg| AS Flg|  ACA  | FlushQ| FRU   |ucod   |
 * |-----+---------------------------------------------------------------|
 * |15   |  Reserved for Future Diagnostic use			      	 |
 * |-----+---------------------------------------------------------------|
 * |16-18|            Reserved					      	 |
 * |-----+---------------------------------------------------------------|
 * |19   |  MSB of Command Queue Depth				      	 |
 * |-----+---------------------------------------------------------------|
 * |20   |  LSB of Command Queue Depth				      	 |
 * |-----+---------------------------------------------------------------|
 * |21   |   						      		 |
 * |-----+--	                                                   ------|
 * |22   | 		Bus Reset Delay Time in milliseconds	      	 |  
 * |-----+--							   ------|
 * |23   |                                                               |
 * |-----+---------------------------------------------------------------|
 * |24   |   							      	 |     
 * |-----+--	Recoverable Data Error Multiplier                  ------|
 * |25   |							      	 |
 * |-----+---------------------------------------------------------------|
 * |26   |   	Recoverable Data Error Base 10 exponent  	      	 |
 * |-----+---------------------------------------------------------------|
 * |27   |							      	 |
 * |-----+--	Recoverable Data Error Check Multiplier            ------|
 * |28   |   							      	 |
 * |-----+---------------------------------------------------------------|
 * |29   |	Recoverable Data Error Check Base 10 exponent	      	 |  
 * |-----+---------------------------------------------------------------|
 * |30   |   							       	 |
 * |-----+--	Hard Error Rate Multipler                          ------|
 * |31   |  							      	 |
 * |-----+---------------------------------------------------------------|
 * |32   |      Hard Error Rate Base 10 Exponent		      	 |
 * |-----+---------------------------------------------------------------|
 * |33   |      Technology Code					      	 |
 * |-----+---------------------------------------------------------------|
 * |34   |      Interface					      	 |
 * |-----+---------------------------------------------------------------|
 * |35-46|      Test Media Part Number           		      	 |
 * |-----+---------------------------------------------------------------|
 * |47   |   0       0       0       0   | R Disc| Play A|Mem Dmp|Diagsrc|
 * |-----+---------------------------------------------------------------|
 * |48-50|            Reserved					         |
 * |-----+---------------------------------------------------------------|
 * |51   |   							     	 |
 * |-----+--	   Read/Write Command Timeout                      ------|
 * |52   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |53   |   							      	 |
 * |-----+--	   Write Buffer Command Timeout                    ------|
 * |54   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |55   |   							      	 |
 * |-----+--	   Read Buffer Command Timeout                     ------| 
 * |56   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |57   |   							      	 |
 * |-----+--	   Receive Diagnostics Command Timeout             ------|
 * |58   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |59   |   							       	 |
 * |-----+--	   Send Diagnostics Command Timeout                ------|
 * |60   |  							       	 |
 * |-----+---------------------------------------------------------------| 
 * |61   |   							      	 |
 * |-----+--	   Start Unit Command Timeout                      ------|
 * |62   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |63   |   							      	 |
 * |-----+--	   Format Unit Command Timeout                     ------|
 * |64   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |65   |   							      	 |
 * |-----+--	   Reassign Block Command Timeout                  ------|
 * |66   |  							       	 |
 * |-----+---------------------------------------------------------------|
 * |67-74|            Reserved					      	 |
 * |-----+---------------------------------------------------------------|
 * |75   |            Length of Operating System Identifer	      	 |
 * |-----+---------------------------------------------------------------|
 * |76-84|            Operating System Identifier (AIX)		      	 |
 * |-----+---------------------------------------------------------------|
 * |85   |   							      	 |
 * |-----+--	                                                   ------|
 * |86   | 		Left Justified LED              	       	 |  
 * |-----+--							   ------|
 * |87   |                                                               |
 * |-----+---------------------------------------------------------------|
 * |88   |            Max Retry count				      	 |
 * |-----+---------------------------------------------------------------|
 * |89   |   0       0       0       0       0       0   | Q Type|Qerr   |
 * |-----+---------------------------------------------------------------|
 * |90   |   0       0       0       0       0   | CDDA  | M2F2  |M2F1   |
 * |-----+---------------------------------------------------------------|
 * |91   |            Mode 2 Form 1 Density Code		      	 |
 * |-----+---------------------------------------------------------------|
 * |92   |            Mode 2 Form 2 Density Code		      	 |
 * |-----+---------------------------------------------------------------|
 * |93   |            CD DA Density Code			      	 |
 * |-----+---------------------------------------------------------------|
 *
 */

/* Use pragma to force alignment with the VPD information */
#pragma options align=packed 
struct  cd_scsd_inqry_data	{
	uchar   pdevtype;		/* Peripheral Qualifier & device type*/
	uchar	page_code;		/* SCSD Page Code Number	     */
	uchar	resvd1;			/* Reserved for future use.	     */
	uchar	page_length;		/* Length of this page in bytes	     */
	uchar	scsd_id_length;		/* Length of SCSD keyword in bytes   */
	uchar	scsd_id_field[4];	/* SCSD keyword identification field */
	uchar	num_luns;		/* number of luns supported	     */
	uchar	storage_capacity[4];	/* Storage capacity in Megabytes     */
	uchar	operation_flags;        /* Miscellaneous flags		     */
#define SCSD_UCODE_DWNLD_FLG    0x01	/* Mask to extract ucode download    */
#define SCSD_FRU_FLG	        0x02    /* Mask to extract FRU Flg	     */
#define SCSD_FLUSH_Q_FLG        0x04	/* Mask to extract flush queue flag  */
#define SCSD_ACA_FLG            0x08	/* Mask to extract ACA flag	     */
#define SCSD_AUTO_SENSE_FLG     0x10	/* Mask to extract autosense flag    */
#define SCSD_SAVE_PAGE _FLG     0x20	/* Mask to extract save page flag    */
#define SCSD_TRAY_FLG     	0x40	/* Mask to extract tray flag         */
	uchar	diagdata1;		/* Future diag flags.		     */
	uchar	resvd2[3];		/* Reserved for future use.	     */
	ushort	queue_depth;	        /* Maximum device queue depth        */
	uchar	reset_delay[3];		/* Time in milliseconds before device*/
					/* is ready for use after BDR        */
	union {
		struct {
			uchar bytes[SCSD_LEN_DIAG2];
		} diagbytes;
		struct {
			ushort data_err_mult;
			uchar data_err_base_ten;
			ushort equip_chk_mult;
			uchar equip_chk_base_ten;
			ushort err_rate_mult;
			uchar err_rate_base_ten;
		} diag_values;
	} diagdata2;
	uchar	technology_code;	
#define	SCSD_OEM_CD_ROM		0x1	/* Other SCSI CD-ROM		     */
#define	SCSD_CD_ROM		0x2	/* Supported CD-ROM		     */
#define	SCSD_MM_CD_ROM		0x3	/* Supported Multimedia CD-ROM 	     */
#define	SCSD_OEM_RWOPT		0x1	/* Other SCSI R/W Optical 	     */
#define	SCSD_RWOPT		0x2	/* Supported SCSI R/W Optical	     */
#define	SCSD_MM_RWOPT		0x3	/* Supported Multimedia R/W Optical  */


	uchar	interface_id;
#define SCSD_SE			0x1	/* Single Ended SCSI device	     */
#define SCSD_DIFF		0x2	/* Differential SCSI device	     */
#define SCSD_P1394		0x3
#define SCSD_SSA		0x4
#define SCSD_FCS		0x5

	uchar test_media_pn[SCSD_CD_MEDIA_PN];/* ASCII Test Media Part Number*/
					/* (For removable media devices that */
					/* have test media).		     */

	uchar	scsi_support_flags;
#define SCSD_CD_DIAG_SRC_FLG	0x1	/* Mask to extract diagnostic source */
#define SCSD_CD_MEM_DUMP_FLG	0x2	/* Mask to extract memory dump flag  */
#define SCSD_CD_PLAY_AUDIO_FLG	0x4	/* Mask to extract SCSI-2 play audio */
					/* command set supported flag        */
#define SCSD_CD_READ_DISC_FLG	0x8	/* Mask to extract read discinfo SCSI*/
					/* command supported flag 	     */

	uchar	resvd3[3];		/* Reserved for future use.	     */
	ushort	rw_timeout;		/* Read/write command timeout	     */
	ushort	wbuff_timeout;		/* Write buffer command timeout	     */
	ushort	rbuff_timeout;		/* Read buffer command timeout	     */
	ushort	recv_diag_timeout;	/* Receive diagnostic command timeout*/
	ushort	send_diag_timeout;	/* Send diagnostics command timeout  */
	ushort	start_timeout;		/* Start unit command timeout	     */
	ushort	reassign_timeout;	/* Reassign command timeout	     */
	ushort	fmt_timeout;		/* Format unit command timeout	     */
	uchar	resvd4[8];		/* Reserved for future use.	     */
	uchar   os_info_length;		/* Length of Operating system info   */
	uchar   os_info[9];             /* Operating System Info	     */
	uchar	led_no[3];		/* Left justified LED value	     */
        
	uchar	recovery_limit;		
	uchar   q_flags;		/* Command tag queuing flags.	     */
#define SCSD_QERR_FLG  		0x1	/* Mask to extract Qerr Flag 	     */
#define SCSD_Q_TYPE_FLG  	0x2	/* Mask to extract queue type	     */
	uchar	cd_mode_byte;		/* Valid CD-ROM density codes	     */

#define SCSD_CD_M2F1 		0x1
#define SCSD_CD_M2F2 		0x2
#define SCSD_CD_DA 		0x4

	uchar	cd_m2f1;		/* CD-ROM Mode 2 Form 1 density code */
	uchar	cd_m2f2;		/* CD-ROM Mode 2 Form 2 density code */
	uchar	cd_da;			/* CD-DA (Digital Audio) density code*/
	uchar	resvd5[162];
};
#pragma options align=reset

/* Inquiry format for SCSD VPD page for CD-ROM and Read/Write Optical Drives */
/* 
 *			CD-ROM and Read/Write Optical SCSD VPD Mode pages
 * +=====-=======-=======-=======-=======-=======-=======-=======-=======+
 * |  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * |Byte |       |       |       |       |       |       |       |       |
 * |=====+===============================================================|
 * | 0   |   0       0       0   |    Peripheral Device Type             |
 * |-----+---------------------------------------------------------------|
 * | 1   |   			Page Code = 0xc8 (or 0xc9)	    	 |
 * |-----+---------------------------------------------------------------|
 * | 2   |   	Reserved = 0					      	 |
 * |-----+---------------------------------------------------------------|
 * | 3   |      Page Length = 0x60				     	 |
 * |-----+---------------------------------------------------------------|
 * | 4   |            Length of Operating System Identifer	      	 |
 * |-----+---------------------------------------------------------------|
 * |5-13 |            Operating System Identifier (AIX)		      	 |
 * |-----+---------------------------------------------------------------|
 * |14-99|            Mode Data						 |
 * |-----+---------------------------------------------------------------|
 */

/* Use pragma to force alignment with the VPD information */

#pragma options align=packed 
struct  cd_scsd_mode_data	{
	uchar   pdevtype;		/* Peripheral Qualifier & device type*/
	uchar	page_code;		/* SCSD Page Code Number	     */
	uchar	resvd1;			/* Reserved for future use.	     */
	uchar	page_length;		/* Length of this page in bytes	     */
	uchar   os_info_length;		/* Length of Operating system info   */
	uchar   os_info[9];             /* Operating System Info	     */
	uchar   mode_data[SCSD_MODE_LENGTH];
};
#pragma options align=reset

/* Inquiry format for SCSD VPD page for Disk Drives */

/* 
 *			Disk SCSD VPD page
 * +=====-=======-=======-=======-=======-=======-=======-=======-=======+
 * |  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * |Byte |       |       |       |       |       |       |       |       |
 * |=====+===============================================================|
 * | 0   |   0       0       0   |    Peripheral Device Type             |
 * |-----+---------------------------------------------------------------|
 * | 1   |   			Page Code = 0xc7		     	 |
 * |-----+---------------------------------------------------------------|
 * | 2   |   	Reserved = 0					      	 |
 * |-----+---------------------------------------------------------------|
 * | 3   |      Page Length = 113 Dec				    	 |
 * |-----+---------------------------------------------------------------|
 * | 4   |      Length of SCDD ID field = 4			         |
 * |-----+---------------------------------------------------------------|
 * | 5-8 |      SCDD					       		 |
 * |-----+---------------------------------------------------------------|
 * | 9   |      Number of LUN's supported by device		      	 |
 * |-----+---------------------------------------------------------------|
 * |10-13|  	Storage Capacity in Megabytes			      	 |    
 * |-----+---------------------------------------------------------------|
 * |14   |  0    |   0   | SP Flg| AS Flg| ACA   |Flush Q | FRU  |ucod   |
 * |-----+---------------------------------------------------------------|
 * |15   |  Reserved for Future Diagnostic use			       	 |
 * |-----+---------------------------------------------------------------|
 * |16-18|  Size of Microcode file in bytes sent per write buffer      	 |
 * |-----+---------------------------------------------------------------|
 * |19   |  MSB of Command Queue Depth				       	 |
 * |-----+---------------------------------------------------------------|
 * |20   |  LSB of Command Queue Depth				       	 |
 * |-----+---------------------------------------------------------------|
 * |21   |   							       	 |
 * |-----+--	                                                   ------|
 * |22   | 		Bus Reset Delay Time in milliseconds	       	 |  
 * |-----+--							   ------|
 * |23   |                                                               |
 * |-----+---------------------------------------------------------------|
 * |24   |   							      	 |   
 * |-----+--	Recoverable Data Error Multiplier                  ------|
 * |25   |							     	 |
 * |-----+---------------------------------------------------------------|
 * |26   |   	Recoverable Data Error Base 10 exponent  	      	 |
 * |-----+---------------------------------------------------------------|
 * |27   |						      		 |
 * |-----+--	Recoverable Data Error Check Multiplier            ------|
 * |28   |   							     	 |
 * |-----+---------------------------------------------------------------|
 * |29   |	Recoverable Data Error Check Base 10 exponent	      	 |    
 * |-----+---------------------------------------------------------------|
 * |30   |   							      	 |
 * |-----+--	Hard Error Rate Multipler                          ------|
 * |31   |  							      	 |
 * |-----+---------------------------------------------------------------|
 * |32   |      Hard Error Rate Base 10 Exponent		      	 |
 * |-----+---------------------------------------------------------------|
 * |33   |      Technology Code					      	 |
 * |-----+---------------------------------------------------------------|
 * |34   |      Interface					      	 |
 * |-----+---------------------------------------------------------------|
 * |35   |   							      	 |
 * |-----+--	   Read/Write Command Timeout                      ------|
 * |36   |  							       	 |
 * |-----+---------------------------------------------------------------| 
 * |37   |   							       	 |
 * |-----+--	   Write Buffer Command Timeout                    ------|
 * |38   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |39   |   							      	 |
 * |-----+--	   Read Buffer Command Timeout                     ------| 
 * |40   |  						      		 |
 * |-----+---------------------------------------------------------------| 
 * |41   |   						      		 |
 * |-----+--	   Send Diagnostics Command Timeout                ------|
 * |42   |  						       		 |
 * |-----+---------------------------------------------------------------| 
 * |43   |   						      		 |
 * |-----+--	   Format Unit Command Timeout                     ------|
 * |44   |  						      		 |
 * |-----+---------------------------------------------------------------| 
 * |45   |   						      		 |
 * |-----+--	   Start Unit Command Timeout                      ------|
 * |46   |  						       		 |
 * |-----+---------------------------------------------------------------| 
 * |47   |   						      		 |
 * |-----+--	   Reassign Block Command Timeout                  ------|
 * |58   |  						      		 |
 * |-----+---------------------------------------------------------------| 
 * |49-58|            Reserved				       		 |
 * |-----+---------------------------------------------------------------|
 * |59   |            Length of Operating System Identifer	      	 |
 * |-----+---------------------------------------------------------------|
 * |60-68|            Operating System Identifier (AIX)		      	 |
 * |-----+---------------------------------------------------------------|
 * |69   |   							     	 |
 * |-----+--	                                                   ------|
 * |70   | 		Left Justified LED              	      	 |  
 * |-----+--							   ------|
 * |71   |                                                               |
 * |-----+---------------------------------------------------------------|
 * |72   |            Max Retry count				     	 |
 * |-----+---------------------------------------------------------------|
 * |73   |   0       0       0       0        0      0   | Q Type |Qerr  |
 * |-----+---------------------------------------------------------------|
 */

/* Use pragma to force alignment with the VPD information */
#pragma options align=packed 
struct  disk_scsd_inqry_data	{
	uchar   pdevtype;		/* Peripheral Qualifier & device type*/
	uchar	page_code;		/* SCSD Page Code Number	     */
	uchar	resvd1;			/* Reserved for future use.	     */
	uchar	page_length;		/* Length of this page in bytes	     */
	uchar	scsd_id_length;		/* Length of SCSD keyword in bytes   */
	uchar	scsd_id_field[4];	/* SCSD keyword identification field.*/
	uchar	num_luns;		/* number of luns supported	     */
	uchar	storage_capacity[4];	/* Storage capacity in Megabytes     */
	uchar	operation_flags;        /* Miscellaneous flags		     */
					/* See operation_flags defines for CD*/

	uchar	diagdata1;		/* Future diag flags.		     */
	uchar   ucode_size[3];		/* Size of ucode file in bytes/write */
	ushort	queue_depth;	        /* Maximum device queue depth        */
	uchar	reset_delay[3];		/* Time in milliseconds before device*/
					/* is ready for use after BDR        */
	union {
		struct {
			uchar bytes[SCSD_LEN_DIAG2];
		} diagbytes;
		struct {
			ushort data_err_mult;
			uchar data_err_base_ten;
			ushort equip_chk_mult;
			uchar equip_chk_base_ten;
			ushort err_rate_mult;
			uchar err_rate_base_ten;
		} diag_values;
	} diagdata2;

	uchar	technology_code;	
#define	SCSD_OEM_DISK		0x1	/* Other SCSI Disk		     */
#define	SCSD_DISK		0x2	/* Supported SCSI disk		     */

	uchar	interface_id;
					/* See interface_id defines for CD   */

	ushort	rw_timeout;		/* Read/write command timeout	     */
	ushort	wbuff_timeout;		/* Write buffer command timeout	     */
	ushort	rbuff_timeout;		/* Read buffer command timeout	     */
	ushort	send_diag_timeout;	/* Send diagnostics command timeout  */
	ushort	fmt_timeout;		/* Format unit command timeout	     */
	ushort	start_timeout;		/* Start unit command timeout	     */
	ushort	reassign_timeout;	/* Reassign command timeout	     */
	uchar	resvd2[10];		/* Reserved for future use.	     */
	uchar   os_info_length;		/* Length of Operating system info   */
	uchar   os_info[9];             /* Operating System Info	     */
	uchar	led_no[3];		/* Left justified LED value	     */
	uchar	recovery_limit;		
	uchar   q_flags;		/* Command tag queuing flags.	     */
					/* See q_flags defines for CD        */
	uchar	resvd3[162];
};
#pragma options align=reset

/* Inquiry format for SCSD VPD page for Disk Drives */
/* 
 *			Disk SCSD VPD Mode pages
 * +=====-=======-=======-=======-=======-=======-=======-=======-=======+
 * |  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * |Byte |       |       |       |       |       |       |       |       |
 * |=====+===============================================================|
 * | 0   |   0       0       0   |    Peripheral Device Type             |
 * |-----+---------------------------------------------------------------|
 * | 1   |   			Page Code = 0xc8 (or 0xc9)	    	 |
 * |-----+---------------------------------------------------------------|
 * | 2   |   	Reserved = 0					      	 |
 * |-----+---------------------------------------------------------------|
 * | 3   |      Page Length = 155 Dec    			     	 |
 * |-----+---------------------------------------------------------------|
 * | 4   |            Length of Operating System Identifer	      	 |
 * |-----+---------------------------------------------------------------|
 * |5-13 |            Operating System Identifier (AIX)		      	 |
 * |-----+---------------------------------------------------------------|
 * |14-99|            Mode Data						 |
 * |-----+---------------------------------------------------------------|
 */

/* Use pragma to force alignment with the VPD information */

#pragma options align=packed
struct  disk_scsd_mode_data	{
	uchar   pdevtype;		/* Peripheral Qualifier & device type*/
	uchar	page_code;		/* SCSD Page Code Number	     */
	uchar	resvd1;			/* Reserved for future use.	     */
	uchar	page_length;		/* Length of this page in bytes	     */
	uchar   os_info_length;		/* Length of Operating system info   */
	uchar   os_info[9];             /* Operating System Info	     */
	uchar   mode_data[SCSD_MODE_LENGTH];
};
#pragma options align=reset





/* Inquiry format for Tape Drives */
/* 
 *			Tape SCSD VPD page
 * +=====-=======-=======-=======-=======-=======-=======-=======-=======+
 * |  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * |Byte |       |       |       |       |       |       |       |       |
 * |=====+===============================================================|
 * | 0   |   0       0       0   |    Peripheral Device Type             |
 * |-----+---------------------------------------------------------------|
 * | 1   |   			Page Code = 0xc7		    	 |
 * |-----+---------------------------------------------------------------|
 * | 2   |   	Reserved = 0					      	 |
 * |-----+---------------------------------------------------------------|
 * | 3   |      Page Length = 113 (dec) 			     	 |
 * |-----+---------------------------------------------------------------|
 * | 4   |      Length of SCDD ID field = 4			         |
 * |-----+---------------------------------------------------------------|
 * | 5-8 |      SCDD						     	 |
 * |-----+---------------------------------------------------------------|
 * | 9   |      Number of LUN's supported by device		     	 |
 * |-----+---------------------------------------------------------------|
 * |10-13|  	Storage Capacity in Megabytes			     	 |    
 * |-----+---------------------------------------------------------------|
 * |14   |  0    |   0   | SP Flg| AS Flg| ACA   |Flush Q | FRU  |ucod   |
 * |-----+---------------------------------------------------------------|
 * |15   |  Reserved for Future Diagnostic use			       	 |
 * |-----+---------------------------------------------------------------|
 * |16-18|  Size of Microcode file in bytes sent per write buffer      	 |
 * |-----+---------------------------------------------------------------|
 * |19   |  MSB of Command Queue Depth				       	 |
 * |-----+---------------------------------------------------------------|
 * |20   |  LSB of Command Queue Depth				       	 |
 * |-----+---------------------------------------------------------------|
 * |21   |   							       	 |
 * |-----+--	                                                   ------|
 * |22   | 		Bus Reset Delay Time in milliseconds	       	 |  
 * |-----+--							   ------|
 * |23   |                                                               |
 * |-----+---------------------------------------------------------------|
 * |24-26|   	Reserved         				      	 |
 * |-----+---------------------------------------------------------------|
 * |27   |      Technology Code					      	 |
 * |-----+---------------------------------------------------------------|
 * |28   |      Interface					      	 |
 * |-----+---------------------------------------------------------------|
 * |29-40|      Test Media Part Number           		      	 |
 * |-----+---------------------------------------------------------------|
 * |41   |      							 |
 * |-----+--	Maximum Transfer Size in bytes per SCSI cmd        ------|
 * |42  |  							       	 |
 * |-----+---------------------------------------------------------------|
 * |43   |  	Byte Count for the Density Values Field          	 |
 * |-----+---------------------------------------------------------------| 
 * |44-53|      Supported Density Values				 |
 * |-----+---------------------------------------------------------------|
 * |54   |   0       0   |MemDmp |Clean B|Diagsrc|Retent  |RCOMP  |WCOMP |
 * |-----+---------------------------------------------------------------| 
 * |55   |       Reserved				       		 |
 * |-----+---------------------------------------------------------------|
 * |56   |   							      	 |
 * |-----+--	   Erase Command Timeout                           ------|
 * |57   |  							       	 |
 * |-----+---------------------------------------------------------------| 
 * |58   |   							       	 |
 * |-----+--	   Load/Unload Command Timeout                     ------|
 * |59   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |60   |   							      	 |
 * |-----+--	   Locate Command Timeout                          ------| 
 * |61   |  						      		 |
 * |-----+---------------------------------------------------------------| 
 * |62   |   							       	 |
 * |-----+--	   Rewind Command Timeout                          ------|
 * |63   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |64   |   							      	 |
 * |-----+--	   Space Command Timeout                           ------| 
 * |65   |  						      		 |
 * |-----+---------------------------------------------------------------| 
 * |66   |   							      	 |
 * |-----+--	   Write Filemark Command Timeout                  ------| 
 * |67   |  						      		 |
 * |-----+---------------------------------------------------------------| 
 * |68   |   							     	 |
 * |-----+--	   Read/Write Command Timeout                      ------|
 * |69   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |70   |   							      	 |
 * |-----+--	   Write Buffer Command Timeout                    ------|
 * |71   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |72   |   							      	 |
 * |-----+--	   Read Buffer Command Timeout                     ------| 
 * |73   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |74   |   							      	 |
 * |-----+--	   Receive Diagnostics Command Timeout             ------|
 * |75   |  							      	 |
 * |-----+---------------------------------------------------------------| 
 * |76   |   						      		 |
 * |-----+--	   Send Diagnostics Command Timeout                ------|
 * |77   |  						       		 |
 * |-----+---------------------------------------------------------------| 
 * |78-87|            Reserved				       		 |
 * |-----+---------------------------------------------------------------|
 * |88   |            Length of Operating System Identifer	      	 |
 * |-----+---------------------------------------------------------------|
 * |89-97|            Operating System Identifier (AIX)		      	 |
 * |-----+---------------------------------------------------------------|
 * |98   |   							     	 |
 * |-----+--	                                                   ------|
 * |99   | 		Left Justified LED              	      	 |  
 * |-----+--							   ------|
 * |100  |                                                               |
 * |-----+---------------------------------------------------------------|
 * |101  |            Max Retry count				     	 |
 * |-----+---------------------------------------------------------------|
 * |102  |   0       0       0       0        0      0   | Q Type |Qerr  |
 * |-----+---------------------------------------------------------------|
 */


/* Use pragma to force alignment with the VPD information */

#pragma options align=packed
struct  tape_scsd_inqry_data	{
	uchar   pdevtype;		/* Peripheral Qualifier & device type*/
	uchar	page_code;		/* SCSD Page Code Number	     */
	uchar	resvd1;			/* Reserved for future use.	     */
	uchar	page_length;		/* Length of this page in bytes	     */
	uchar	scsd_id_length;		/* Length of SCSD keyword in bytes   */
	uchar	scsd_id_field[4];	/* SCSD keyword identification field.*/
	uchar	num_luns;		/* number of luns supported	     */
	uchar	storage_capacity[4];	/* Storage capacity in Megabytes     */
	uchar	operation_flags;        /* Miscellaneous flags		     */
					/* See operation_flags defines for CD*/

	uchar	diagdata1;		/* Future diag flags.		     */
	uchar   ucode_size[3];		/* Size of ucode file in bytes/write */
	ushort	queue_depth;	        /* Maximum device queue depth        */
	uchar	reset_delay[3];		/* Time in milliseconds before device*/
					/* is ready for use after BDR        */
	uchar   diagdata2[3];

	uchar	technology_code;	
#define	SCSD_OEM_TAPE		0x01	/* Other SCSI Tape		     */
#define	SCSD_TAPE		0x02	/* Supported SCSI Tape		     */
#define	SCSD_TAPE_QUARTER	0x03
#define	SCSD_TAPE_8MM		0x04
#define	SCSD_TAPE_9TRACK	0x05
#define	SCSD_TAPE_HALF_CART	0x06
#define	SCSD_TAPE_4MM		0x07
#define	SCSD_TAPE_DLT		0x08


	uchar	interface_id;
					/* See interface_id defines for CD   */

	uchar	test_media_pn[12];	/* ASCII Test Media Part Number. (For*/
					/* removable media devices that have */
					/* test media)			     */
	ushort  max_transfer_size;
	uchar   length_densities;	
	uchar	densities[10];
	uchar   flags;
#define SCSD_TAPE_WCOMP	       0x01
#define SCSD_TAPE_RCOMP	       0x02
#define SCSD_TAPE_RETENT       0x04
#define SCSD_TAPE_DIAGSRC      0x08
#define SCSD_TAPE_CLEAN	       0x10
#define SCSD_TAPE_MEMDMP       0x20
	uchar   resvd2;


	ushort	erase_timeout;		/* Erase command timeout	     */
	ushort	load_timeout;		/* Load command timeout	             */
	ushort	locate_timeout;		/* Locate command timeout	     */
	ushort	rewind_timeout;		/* Rewind command timeout            */
	ushort	space_timeout;		/* Space command timeout             */
	ushort	write_FOM_timeout;	/* Write FOM command timeout         */
	ushort	rw_timeout;		/* Read/write command timeout	     */
	ushort	wbuff_timeout;		/* Write buffer command timeout	     */
	ushort	rbuff_timeout;		/* Read buffer command timeout	     */
	ushort	recv_diag_timeout;	/* Recv diagnostics command timeout  */
	ushort	send_diag_timeout;	/* Send diagnostics command timeout  */
	uchar	resvd3[10];		/* Reserved for future use.	     */

	uchar   os_info_length;		/* Length of Operating system info   */
	uchar   os_info[9];             /* Operating System Info	     */
	uchar	led_no[3];		/* Left justified LED value	     */
	uchar	recovery_limit;		
	uchar   q_flags;		/* Command tag queuing flags.	     */
					/* See q_flags defines for CD        */
	uchar	resvd4[162];
};
#pragma options align=reset

/* Inquiry format for SCSD VPD page for Tape Drives */
/* 
 *			Tape SCSD VPD Mode pages
 * +=====-=======-=======-=======-=======-=======-=======-=======-=======+
 * |  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * |Byte |       |       |       |       |       |       |       |       |
 * |=====+===============================================================|
 * | 0   |   0       0       0   |    Peripheral Device Type             |
 * |-----+---------------------------------------------------------------|
 * | 1   |   			Page Code = 0xc8 (or 0xc9)	    	 |
 * |-----+---------------------------------------------------------------|
 * | 2   |   	Reserved = 0					      	 |
 * |-----+---------------------------------------------------------------|
 * | 3   |      Page Length = 155 Dec    			     	 |
 * |-----+---------------------------------------------------------------|
 * | 4   |            Length of Operating System Identifer	      	 |
 * |-----+---------------------------------------------------------------|
 * |5-14 |            Operating System Identifier (AIX)		      	 |
 * |-----+---------------------------------------------------------------|
 * |15-  |            Mode Data						 |
 * |-----+---------------------------------------------------------------|
 */

/* Use pragma to force alignment with the VPD information */

#pragma options align=packed
struct  tape_scsd_mode_data	{
	uchar   pdevtype;		/* Peripheral Qualifier & device type*/
	uchar	page_code;		/* SCSD Page Code Number	     */
	uchar	resvd1;			/* Reserved for future use.	     */
	uchar	page_length;		/* Length of this page in bytes	     */
	uchar   os_info_length;		/* Length of Operating system info   */
	uchar   os_info[9];             /* Operating System Info	     */
	uchar   mode_data[SCSD_MODE_LENGTH];
};
#pragma options align=reset
#endif /* _H_DIAG */
