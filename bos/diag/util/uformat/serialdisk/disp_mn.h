/* @(#)16       1.6  src/bos/diag/util/uformat/serialdisk/disp_mn.h, dsauformat, bos41J, 9522A_all 5/29/95 22:48:19 */
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Set up menus to be displayed 
 */
struct msglist menu_f_certify_after[] =
{
	{DUFORMAT_SET, MENU_F_CERTIFY_AFTER},
	{DUFORMAT_SET, OPTION_F_CERTIFY}, 
	{DUFORMAT_SET, OPTION_F_NO_CERTIFY}, 
	{DUFORMAT_SET, OPTION_DCLASS_DRIVE}, 
	{DUFORMAT_SET, OPTION_CHOOSE}, (int)NULL
};


struct msglist menu_f_a_certify_after[] = 
{
	{DUFORMAT_SET, MENU_FORMAT_OR_CERTIFY},
	{DUFORMAT_SET, OPTION_FORMAT_DRIVE},
	{DUFORMAT_SET, OPTION_CERTIFY_DRIVE_MEDIA},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};

struct msglist menu_f_warning[] = 
{
	{DUFORMAT_SET, MENU_F_WARNING},
	{DUFORMAT_SET, OPTION_YES},
	{DUFORMAT_SET, OPTION_NO},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};



struct msglist menu_please_stand_by[] = 
{
	{DUFORMAT_SET, MENU_PLEASE_STAND_BY},
	(int)NULL
};

struct msglist menu_f_complete[] = 
{
	{DUFORMAT_SET, MENU_F_COMPLETE}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist menu_f_complete_nowait[] =
{
        {DUFORMAT_SET, MENU_F_COMPLETE},
        (int)NULL
};

struct msglist menu_terminated[] = 
{
	{DUFORMAT_SET, MENU_TERMINATED}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist menu_should_format[] = 
{
	{DUFORMAT_SET, MENU_SHOULD_FORMAT}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist menu_esc_f_warning_2[] = 
{
	{DUFORMAT_SET, MENU_ESC_F_WARNING_2}, 
	{DUFORMAT_SET, OPTION_YES},
	{DUFORMAT_SET, OPTION_NO},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};

struct msglist menu_esc_c_warning_2[] = 
{
	{DUFORMAT_SET, MENU_ESC_C_WARNING_2}, 
	{DUFORMAT_SET, OPTION_YES},
	{DUFORMAT_SET, OPTION_NO},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};

struct msglist menu_f_use_floppy_2[] = 
{
	{DUFORMAT_SET, MENU_F_USE_FLOPPY_2}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};


struct msglist menu_c_use_floppy_2[] = 
{
	{DUFORMAT_SET, MENU_C_USE_FLOPPY_2}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist no_hard_disks[] = 
{
	{DUFORMAT_SET, NO_HARD_DISKS}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist format_in_progress[] = 
{
	{DUFORMAT_SET, FORMAT_IN_PROGRESS}, 
	(int)NULL 
};

struct msglist menu_no_attr_2[] = 
{
	{DUFORMAT_SET, MENU_NO_ATTR_2}, 
	{DUFORMAT_SET, OPTION_RETURN},
	(int)NULL
};

struct msglist os_warn_continue[] = 
{
	{DUFORMAT_SET, OS_WARN_CONTINUE}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};


struct msglist os_warn_return[] = 
{
	{DUFORMAT_SET, OS_WARN_RETURN}, 
	{DUFORMAT_SET, OPTION_RETURN},
	(int)NULL
};

struct msglist os_and_data_warn[] = 
{
	{DUFORMAT_SET, OS_AND_DATA_WARN}, 
	{DUFORMAT_SET, OPTION_ENTER}, 
	(int)NULL
};

struct msglist adapter_configure[] = 
{
	{DUFORMAT_SET, ADAPTER_CONFIGURE}, 
	{DUFORMAT_SET, OPTION_RETURN},
	(int)NULL
};

struct msglist device_configure[] = 
{
	{DUFORMAT_SET, DEVICE_CONFIGURE}, 
	{DUFORMAT_SET, OPTION_RETURN},
	(int)NULL
};

struct msglist menu_esc_ud_warning[] = 
{
	{DUFORMAT_SET, MENU_ESC_UD_WARNING}, 
	{DUFORMAT_SET, OPTION_YES},
	{DUFORMAT_SET, OPTION_NO},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};


struct msglist menu_e_use_floppy[] = 
{
	{DUFORMAT_SET, MENU_E_USE_FLOPPY}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist d_class_disk[] = 
{
	{DUFORMAT_SET, D_CLASS_DISK}, 
	{DUFORMAT_SET, OPTION_YES},
	{DUFORMAT_SET, OPTION_NO},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};


struct msglist menu_dclass_complete[] = 
{
	{DUFORMAT_SET, MENU_DCLASS_COMPLETE}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};


struct msglist no_bad_blocks[] = 
{
	{DUFORMAT_SET, NO_BAD_BLOCKS}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

/*struct msglist bad_blocks_start[] = 
{ 
	{DUFORMAT_SET, BAD_BLOCKS_FOUND}, 
	{DUFORMAT_SET, BAD_BLOCKS_START}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist bad_blocks_erase[] = 
{ 
	{DUFORMAT_SET, BAD_BLOCKS_FOUND}, 
	{DUFORMAT_SET, BAD_BLOCKS_ERASE},  
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};*/

struct msglist cmd_not_sup[] = 
{
	{DUFORMAT_SET, CMD_NOT_SUP}, 
	{DUFORMAT_SET, OPTION_YES},
	{DUFORMAT_SET, OPTION_NO},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};

struct msglist unknown_err[] = 
{
	{DUFORMAT_SET, UNKNOWN_ERR}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist menu_write_failed[] = 
{
	{DUFORMAT_SET, MENU_WRITE_FAILED}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist menu_suggest_format[] = 
{
	{DUFORMAT_SET, MENU_SUGGEST_FORMAT}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist no_such_block[] = 
{
	{DUFORMAT_SET, NO_SUCH_BLOCK}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist menu_format_or_certify[] = 
{
	{DUFORMAT_SET, MENU_FORMAT_OR_CERTIFY}, 
	{DUFORMAT_SET, OPTION_FORMAT_DRIVE},
	{DUFORMAT_SET, OPTION_CERTIFY_DRIVE_MEDIA},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};

struct msglist menu_c_complete_bad[] = 
{
	{DUFORMAT_SET, MENU_C_COMPLETE_BAD}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist menu_f_sr_certify_after[] =
{
	{DUFORMAT_SET, MENU_F_SR_CERTIFY_AFTER},
	{DUFORMAT_SET, OPTION_F_CERTIFY}, 
	{DUFORMAT_SET, OPTION_F_NO_CERTIFY}, 
	{DUFORMAT_SET, OPTION_CHOOSE}, (int)NULL
};

struct msglist data_err[] = 
{
	{DUFORMAT_SET, DATA_ERR}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist equip_err[] = 
{
	{DUFORMAT_SET, EQUIP_ERR}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

struct msglist menu_read_or_write[] = 
{
	{DUFORMAT_SET, MENU_READ_OR_WRITE}, 
	{DUFORMAT_SET, OPTION_READ_DRIVE},
	{DUFORMAT_SET, OPTION_WRITE_DRIVE},
	{DUFORMAT_SET, OPTION_CHOOSE},
	(int)NULL
};

struct msglist menu_c_complete_warning[] = 
{
	{DUFORMAT_SET, MENU_C_COMPLETE_WARNING}, 
	{DUFORMAT_SET, OPTION_ENTER},
	(int)NULL
};

