static char sccsid[] = "@(#)68  1.4  src/bos/usr/bin/smit/smit_class.c, smitobj, bos411, 9428A410j 6/18/91 13:06:09";

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




#include "smit_class.h"
static struct ClassElem sm_cmd_opt_ClassElem[] =  {
 { "id_seq_num",ODM_CHAR, 12,17, NULL,NULL,0,NULL ,-1,0},
 { "id",ODM_CHAR, 29,65, NULL,NULL,0,NULL ,-1,0},
 { "disc_field_name",ODM_VCHAR, 96,65, NULL,NULL,0,NULL ,-1,0},
 { "name",ODM_VCHAR, 100,1025, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_file",ODM_VCHAR, 104,1025, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_set",ODM_LONG, 108, 4, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_id",ODM_LONG, 112, 4, NULL,NULL,0,NULL ,-1,0},
 { "op_type",ODM_CHAR, 116,2, NULL,NULL,0,NULL ,-1,0},
 { "entry_type",ODM_CHAR, 118,2, NULL,NULL,0,NULL ,-1,0},
 { "entry_size",ODM_LONG, 120, 4, NULL,NULL,0,NULL ,-1,0},
 { "required",ODM_CHAR, 124,2, NULL,NULL,0,NULL ,-1,0},
 { "prefix",ODM_VCHAR, 128,1025, NULL,NULL,0,NULL ,-1,0},
 { "cmd_to_list_mode",ODM_CHAR, 132,2, NULL,NULL,0,NULL ,-1,0},
 { "cmd_to_list",ODM_VCHAR, 136,1025, NULL,NULL,0,NULL ,-1,0},
 { "cmd_to_list_postfix",ODM_VCHAR, 140,1025, NULL,NULL,0,NULL ,-1,0},
 { "multi_select",ODM_CHAR, 144,2, NULL,NULL,0,NULL ,-1,0},
 { "value_index",ODM_LONG, 148, 4, NULL,NULL,0,NULL ,-1,0},
 { "disp_values",ODM_VCHAR, 152,1025, NULL,NULL,0,NULL ,-1,0},
 { "values_msg_file",ODM_VCHAR, 156,1025, NULL,NULL,0,NULL ,-1,0},
 { "values_msg_set",ODM_LONG, 160, 4, NULL,NULL,0,NULL ,-1,0},
 { "values_msg_id",ODM_LONG, 164, 4, NULL,NULL,0,NULL ,-1,0},
 { "aix_values",ODM_VCHAR, 168,1025, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_id",ODM_CHAR, 172,17, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_loc",ODM_VCHAR, 192,1025, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_base",ODM_VCHAR, 196,64, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_book",ODM_VCHAR, 200,64, NULL,NULL,0,NULL ,-1,0},
 };
struct StringClxn sm_cmd_opt_STRINGS[] = {
 "sm_cmd_opt.vc", 0,NULL,NULL,0,0,0
 };
struct Class sm_cmd_opt_CLASS[] = {
 ODMI_MAGIC, "sm_cmd_opt", sizeof(struct sm_cmd_opt), sm_cmd_opt_Descs, sm_cmd_opt_ClassElem, sm_cmd_opt_STRINGS,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };
static struct ClassElem sm_cmd_hdr_ClassElem[] =  {
 { "id",ODM_CHAR, 12,65, NULL,NULL,0,NULL ,-1,0},
 { "option_id",ODM_VCHAR, 80,65, NULL,NULL,0,NULL ,-1,0},
 { "has_name_select",ODM_CHAR, 84,2, NULL,NULL,0,NULL ,-1,0},
 { "name",ODM_VCHAR, 88,1025, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_file",ODM_VCHAR, 92,1025, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_set",ODM_LONG, 96, 4, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_id",ODM_LONG, 100, 4, NULL,NULL,0,NULL ,-1,0},
 { "cmd_to_exec",ODM_VCHAR, 104,1025, NULL,NULL,0,NULL ,-1,0},
 { "ask",ODM_CHAR, 108,2, NULL,NULL,0,NULL ,-1,0},
 { "exec_mode",ODM_CHAR, 110,2, NULL,NULL,0,NULL ,-1,0},
 { "ghost",ODM_CHAR, 112,2, NULL,NULL,0,NULL ,-1,0},
 { "cmd_to_discover",ODM_VCHAR, 116,1025, NULL,NULL,0,NULL ,-1,0},
 { "cmd_to_discover_postfix",ODM_VCHAR, 120,1025, NULL,NULL,0,NULL ,-1,0},
 { "name_size",ODM_LONG, 124, 4, NULL,NULL,0,NULL ,-1,0},
 { "value_size",ODM_LONG, 128, 4, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_id",ODM_CHAR, 132,17, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_loc",ODM_VCHAR, 152,1025, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_base",ODM_VCHAR, 156,64, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_book",ODM_VCHAR, 160,64, NULL,NULL,0,NULL ,-1,0},
 };
struct StringClxn sm_cmd_hdr_STRINGS[] = {
 "sm_cmd_hdr.vc", 0,NULL,NULL,0,0,0
 };
struct Class sm_cmd_hdr_CLASS[] = {
 ODMI_MAGIC, "sm_cmd_hdr", sizeof(struct sm_cmd_hdr), sm_cmd_hdr_Descs, sm_cmd_hdr_ClassElem, sm_cmd_hdr_STRINGS,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };
static struct ClassElem sm_menu_opt_ClassElem[] =  {
 { "id_seq_num",ODM_CHAR, 12,17, NULL,NULL,0,NULL ,-1,0},
 { "id",ODM_CHAR, 29,65, NULL,NULL,0,NULL ,-1,0},
 { "next_id",ODM_CHAR, 94,65, NULL,NULL,0,NULL ,-1,0},
 { "text",ODM_VCHAR, 160,1025, NULL,NULL,0,NULL ,-1,0},
 { "text_msg_file",ODM_VCHAR, 164,1025, NULL,NULL,0,NULL ,-1,0},
 { "text_msg_set",ODM_LONG, 168, 4, NULL,NULL,0,NULL ,-1,0},
 { "text_msg_id",ODM_LONG, 172, 4, NULL,NULL,0,NULL ,-1,0},
 { "next_type",ODM_CHAR, 176,2, NULL,NULL,0,NULL ,-1,0},
 { "alias",ODM_CHAR, 178,2, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_id",ODM_CHAR, 180,17, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_loc",ODM_VCHAR, 200,1025, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_base",ODM_VCHAR, 204,64, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_book",ODM_VCHAR, 208,64, NULL,NULL,0,NULL ,-1,0},
 };
struct StringClxn sm_menu_opt_STRINGS[] = {
 "sm_menu_opt.vc", 0,NULL,NULL,0,0,0
 };
struct Class sm_menu_opt_CLASS[] = {
 ODMI_MAGIC, "sm_menu_opt", sizeof(struct sm_menu_opt), sm_menu_opt_Descs, sm_menu_opt_ClassElem, sm_menu_opt_STRINGS,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };
static struct ClassElem sm_name_hdr_ClassElem[] =  {
 { "id",ODM_CHAR, 12,65, NULL,NULL,0,NULL ,-1,0},
 { "next_id",ODM_VCHAR, 80,65, NULL,NULL,0,NULL ,-1,0},
 { "option_id",ODM_VCHAR, 84,65, NULL,NULL,0,NULL ,-1,0},
 { "has_name_select",ODM_CHAR, 88,2, NULL,NULL,0,NULL ,-1,0},
 { "name",ODM_VCHAR, 92,1025, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_file",ODM_VCHAR, 96,1025, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_set",ODM_LONG, 100, 4, NULL,NULL,0,NULL ,-1,0},
 { "name_msg_id",ODM_LONG, 104, 4, NULL,NULL,0,NULL ,-1,0},
 { "type",ODM_CHAR, 108,2, NULL,NULL,0,NULL ,-1,0},
 { "ghost",ODM_CHAR, 110,2, NULL,NULL,0,NULL ,-1,0},
 { "cmd_to_classify",ODM_VCHAR, 112,1025, NULL,NULL,0,NULL ,-1,0},
 { "cmd_to_classify_postfix",ODM_VCHAR, 116,1025, NULL,NULL,0,NULL ,-1,0},
 { "raw_field_name",ODM_VCHAR, 120,1025, NULL,NULL,0,NULL ,-1,0},
 { "cooked_field_name",ODM_VCHAR, 124,1025, NULL,NULL,0,NULL ,-1,0},
 { "next_type",ODM_CHAR, 128,2, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_id",ODM_CHAR, 130,17, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_loc",ODM_VCHAR, 148,1025, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_base",ODM_VCHAR, 152,64, NULL,NULL,0,NULL ,-1,0},
 { "help_msg_book",ODM_VCHAR, 156,64, NULL,NULL,0,NULL ,-1,0},
 };
struct StringClxn sm_name_hdr_STRINGS[] = {
 "sm_name_hdr.vc", 0,NULL,NULL,0,0,0
 };
struct Class sm_name_hdr_CLASS[] = {
 ODMI_MAGIC, "sm_name_hdr", sizeof(struct sm_name_hdr), sm_name_hdr_Descs, sm_name_hdr_ClassElem, sm_name_hdr_STRINGS,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };
