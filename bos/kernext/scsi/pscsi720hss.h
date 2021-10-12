/* @(#)07	1.1  src/bos/kernext/scsi/pscsi720hss.h, sysxscsi, bos411, 9432A411a 7/30/94 16:11:27  */
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
typedef unsigned long ULONG;

#define E_abort_bdr_msg_in_addr	0x00000000
#define E_abort_bdr_msg_out_addr	0x00000000
#define E_cmd_msg_in_addr	0x00000000
#define E_extended_msg_addr	0x00000000
#define E_identify_msg_addr	0x00000000
#define E_lun_msg_addr	0x00000000
#define E_reject_msg_addr	0x00000000
#define E_scsi_1_lun	0x00000000
#define E_scsi_0_lun	0x00000000
#define E_scsi_2_lun	0x00000000
#define E_scsi_3_lun	0x00000000
#define E_scsi_4_lun	0x00000000
#define E_scsi_5_lun	0x00000000
#define E_scsi_6_lun	0x00000000
#define E_scsi_7_lun	0x00000000
#define E_status_addr	0x00000000
#define E_sync_msg_out_addr	0x00000000
#define E_sync_msg_out_addr2	0x00000000
#define E_tag_msg_addr	0x00000000
#define R_ext_msg_size	0x00000000
#define R_target_id	0x00000001
#define R_sxfer_patch	0x00000008
#define R_scntl3_patch	0x00000013
#define R_scntl1_patch	0x0000007F
#define R_tag_patch	0x00000099
#define R_abdr_tag_patch	0x000000AA
#define A_phase_error	0x00000001
#define A_io_done_after_data	0x00000005
#define A_io_done	0x00000006
#define A_unknown_msg	0x00000009
#define A_ext_msg	0x0000000A
#define A_check_next_io	0x0000000B
#define A_check_next_io_data	0x0000000C
#define A_cmd_select_atn_failed	0x00000012
#define A_err_not_ext_msg	0x00000013
#define A_sync_neg_done	0x00000014
#define A_sync_msg_reject	0x00000016
#define A_neg_select_failed	0x00000018
#define A_abort_select_failed	0x00000019
#define A_abort_io_complete	0x0000001A
#define A_unknown_reselect_id	0x0000001D
#define A_uninitialized_reselect	0x0000001E
#define A_suspended	0x00000023
#define A_NEXUS_data_base_adr1	0x00000088
#define A_NEXUS_data_base_adr2	0x00000088
#define A_NEXUS_data_base_adr0	0x00000088
#define A_NEXUS_data_base_adr3	0x00000088
#define Ent_abdr2_sequence   	0x00000970
#define Ent_abort_bdr_msg_in_buf	0x00000AE0
#define Ent_abort_bdr_msg_out_buf	0x00000AD8
#define Ent_abort_sequence   	0x00000930
#define Ent_after_data_move_check	0x000005C8
#define Ent_bdr_sequence     	0x00000900
#define Ent_cleanup_phase    	0x000004B8
#define Ent_cmd_buf          	0x00000AA0
#define Ent_cmd_msg_in_buf   	0x00000A98
#define Ent_complete_ext_msg 	0x00000698
#define Ent_disconnect_point 	0x000007C0
#define Ent_disconnect_point_1	0x000007E0
#define Ent_ext_msg_patch    	0x000006A8
#define Ent_ext_msg_handler  	0x00000680
#define Ent_extended_msg_buf 	0x00000AD0
#define Ent_failed_abort_bdr_selection_hdlr	0x00000A78
#define Ent_failed_selection_hdlr	0x00000800
#define Ent_failed_sync_selection_hdlr	0x000008E0
#define Ent_goto_cleanup     	0x00000750
#define Ent_identify_msg_buf 	0x00000AB8
#define Ent_init_dsa         	0x00000348
#define Ent_init_index       	0x000002F0
#define Ent_iowait_entry_point	0x00000000
#define Ent_iowait_patch_point	0x00000018
#define Ent_lun_msg_buf      	0x00000AE8
#define Ent_msg_done_1       	0x00000668
#define Ent_msg_done         	0x00000628
#define Ent_msg_hdlr         	0x000005F0
#define Ent_message_loop     	0x000004B0
#define Ent_msg_hdlr_1       	0x00000638
#define Ent_queue_tag        	0x00000410
#define Ent_receive_data     	0x000005E0
#define Ent_regular_phase_hdlr	0x00000500
#define Ent_reject_cleanup   	0x00000740
#define Ent_reject_msg_buf   	0x00000AF0
#define Ent_reject_target_sync	0x000006E0
#define Ent_renegotiate_sync 	0x00000760
#define Ent_reselect_router  	0x00000008
#define Ent_script_reconnect_point	0x00000438
#define Ent_scripts_entry_point	0x00000490
#define Ent_scsi_id_0        	0x00000060
#define Ent_scsi_id_1        	0x000000B0
#define Ent_scsi_id_2        	0x00000100
#define Ent_scsi_id_3        	0x00000150
#define Ent_scsi_id_4        	0x000001A0
#define Ent_scsi_id_5        	0x000001F0
#define Ent_scsi_id_6        	0x00000240
#define Ent_scsi_id_7        	0x00000290
#define Ent_send_command     	0x000004F0
#define Ent_send_data        	0x000005C0
#define Ent_sig_jump         	0x000002E0
#define Ent_start_abdr2_msg_out_phase	0x000009A8
#define Ent_start_abort_msg_out_phase	0x00000960
#define Ent_start_bdr_msg_out_phase	0x000009D0
#define Ent_start_sync_msg_in_phase	0x00000878
#define Ent_start_sync_msg_out	0x00000850
#define Ent_status_buf       	0x00000AB0
#define Ent_status_complete  	0x00000550
#define Ent_status_complete_data	0x00000588
#define Ent_sync_msg_in_rejected	0x000008B0
#define Ent_sync_msg_out_buf 	0x00000AC0
#define Ent_sync_msg_out_buf2	0x00000AC8
#define Ent_sync_negotiation 	0x00000820
#define Ent_sync_phase_hdlr  	0x000008C0
#define Ent_tag_msg_buf      	0x00000AF8
#define Ent_unknown_msg_hdlr 	0x00000678
