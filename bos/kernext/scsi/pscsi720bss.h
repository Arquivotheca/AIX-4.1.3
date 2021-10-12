/* @(#)09	1.1  src/bos/kernext/scsi/pscsi720bss.h, sysxscsi, bos411, 9432A411a 7/30/94 16:14:37  */
/*
 *   COMPONENT_NAME: sysxscsi
 *
 *   FUNCTIONS: none
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
ULONG E_scsi_1_lun_Used[] = {
	0x0000002F,
	0x00000031,
	0x00000033,
	0x00000035,
	0x00000037,
	0x00000039,
	0x0000003B,
	0x0000003D 
};

ULONG E_scsi_0_lun_Used[] = {
	0x0000001B,
	0x0000001D,
	0x0000001F,
	0x00000021,
	0x00000023,
	0x00000025,
	0x00000027,
	0x00000029 
};

ULONG E_scsi_2_lun_Used[] = {
	0x00000043,
	0x00000045,
	0x00000047,
	0x00000049,
	0x0000004B,
	0x0000004D,
	0x0000004F,
	0x00000051 
};

ULONG E_scsi_3_lun_Used[] = {
	0x00000057,
	0x00000059,
	0x0000005B,
	0x0000005D,
	0x0000005F,
	0x00000061,
	0x00000063,
	0x00000065 
};

ULONG E_scsi_4_lun_Used[] = {
	0x0000006B,
	0x0000006D,
	0x0000006F,
	0x00000071,
	0x00000073,
	0x00000075,
	0x00000077,
	0x00000079 
};

ULONG E_scsi_5_lun_Used[] = {
	0x0000007F,
	0x00000081,
	0x00000083,
	0x00000085,
	0x00000087,
	0x00000089,
	0x0000008B,
	0x0000008D 
};

ULONG E_scsi_6_lun_Used[] = {
	0x00000093,
	0x00000095,
	0x00000097,
	0x00000099,
	0x0000009B,
	0x0000009D,
	0x0000009F,
	0x000000A1 
};

ULONG E_scsi_7_lun_Used[] = {
	0x000000A7,
	0x000000A9,
	0x000000AB,
	0x000000AD,
	0x000000AF,
	0x000000B1,
	0x000000B3,
	0x000000B5 
};

ULONG R_ext_msg_size_Used[] = {
	0x000001A8 
};

ULONG R_sxfer_patch_Used[] = {
	0x00000114,
	0x00000134,
	0x000001BE,
	0x000001DE 
};

ULONG R_scntl3_patch_Used[] = {
	0x00000112,
	0x00000132,
	0x000001BC,
	0x000001DC 
};

ULONG R_scntl1_patch_Used[] = {
	0x00000110,
	0x00000130,
	0x000001BA,
	0x000001DA 
};

ULONG R_tag_patch_Used[] = {
	0x00000126,
	0x00000202,
	0x0000020A,
	0x0000023A 
};

ULONG R_abdr_tag_patch_Used[] = {
	0x00000242,
	0x0000024E,
	0x000002A0 
};

ULONG A_abort_select_failed_Used[] = {
	0x000002A5 
};

ULONG A_abort_io_complete_Used[] = {
	0x00000273 
};

ULONG A_unknown_reselect_id_Used[] = {
	0x00000017 
};

ULONG A_uninitialized_reselect_Used[] = {
	0x0000002B,
	0x0000003F,
	0x00000053,
	0x00000067,
	0x0000007B,
	0x0000008F,
	0x000000A3,
	0x000000B7 
};

ULONG	INSTRUCTIONS	= 0x00000160;
