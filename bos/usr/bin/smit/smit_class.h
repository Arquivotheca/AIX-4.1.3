/* @(#)69       1.4  src/bos/usr/bin/smit/smit_class.h, smitobj, bos411, 9428A410j 6/18/91 13:05:14 */

/*
 *   COMPONENT_NAME: CMDSMIT
 *
 * FUNCTIONS:  none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1989,1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include <odmi.h>

struct sm_cmd_opt {
	long _id;
	long _reserved;
	long _scratch;
	char id_seq_num[17];
	char id[65];
	char *disc_field_name;
	char *name;
	char *name_msg_file;
	long name_msg_set;
	long name_msg_id;
	char op_type[2];
	char entry_type[2];
	long entry_size;
	char required[2];
	char *prefix;
	char cmd_to_list_mode[2];
	char *cmd_to_list;
	char *cmd_to_list_postfix;
	char multi_select[2];
	long value_index;
	char *disp_values;
	char *values_msg_file;
	long values_msg_set;
	long values_msg_id;
	char *aix_values;
	char help_msg_id[17];
	char *help_msg_loc;
	char *help_msg_base;
	char *help_msg_book;
	};
#define sm_cmd_opt_Descs 26

extern struct Class sm_cmd_opt_CLASS[];
#define get_sm_cmd_opt_list(a,b,c,d,e) (struct sm_cmd_opt * )odm_get_list(a,b,c,d,e)

struct sm_cmd_hdr {
	long _id;
	long _reserved;
	long _scratch;
	char id[65];
	char *option_id;
	char has_name_select[2];
	char *name;
	char *name_msg_file;
	long name_msg_set;
	long name_msg_id;
	char *cmd_to_exec;
	char ask[2];
	char exec_mode[2];
	char ghost[2];
	char *cmd_to_discover;
	char *cmd_to_discover_postfix;
	long name_size;
	long value_size;
	char help_msg_id[17];
	char *help_msg_loc;
	char *help_msg_base;
	char *help_msg_book;
	};
#define sm_cmd_hdr_Descs 19

extern struct Class sm_cmd_hdr_CLASS[];
#define get_sm_cmd_hdr_list(a,b,c,d,e) (struct sm_cmd_hdr * )odm_get_list(a,b,c,d,e)

struct sm_menu_opt {
	long _id;
	long _reserved;
	long _scratch;
	char id_seq_num[17];
	char id[65];
	char next_id[65];
	char *text;
	char *text_msg_file;
	long text_msg_set;
	long text_msg_id;
	char next_type[2];
	char alias[2];
	char help_msg_id[17];
	char *help_msg_loc;
	char *help_msg_base;
	char *help_msg_book;
	};
#define sm_menu_opt_Descs 13

extern struct Class sm_menu_opt_CLASS[];
#define get_sm_menu_opt_list(a,b,c,d,e) (struct sm_menu_opt * )odm_get_list(a,b,c,d,e)

struct sm_name_hdr {
	long _id;
	long _reserved;
	long _scratch;
	char id[65];
	char *next_id;
	char *option_id;
	char has_name_select[2];
	char *name;
	char *name_msg_file;
	long name_msg_set;
	long name_msg_id;
	char type[2];
	char ghost[2];
	char *cmd_to_classify;
	char *cmd_to_classify_postfix;
	char *raw_field_name;
	char *cooked_field_name;
	char next_type[2];
	char help_msg_id[17];
	char *help_msg_loc;
	char *help_msg_base;
	char *help_msg_book;
	};
#define sm_name_hdr_Descs 19

extern struct Class sm_name_hdr_CLASS[];
#define get_sm_name_hdr_list(a,b,c,d,e) (struct sm_name_hdr * )odm_get_list(a,b,c,d,e)
