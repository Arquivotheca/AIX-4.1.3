#!/bin/ksh
#
# @(#)04        1.2  src/bos/usr/lib/assist/assist_migrate.sh, cmdassist, bos411, 9428A410j 6/21/94 16:31:54
#
# COMPONENT_NAME: cmdassist
#
# FUNCTIONS: Removes install assistant stanzas from ODM and
#	     re-links the files in add_files back to the
#	     /usr/lib/assist directory.
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------#



ODMDIR=/usr/lib/objrepos
export ODMDIR

for i in `ls /usr/lib/assist/add_files/*.add`
do
awk '
BEGIN {type="";CMD="echo\""}

/sm_cmd_hdr/ { CMD = sprintf ("%s\" -o %s > /dev/null 2>&1\n",CMD,type)
	       system(CMD)
	       type="sm_cmd_hdr"
	       LINK=""
	       CMD = sprintf("%s", "odmdelete -q \"")}
/sm_menu_opt/ { CMD = sprintf ("%s\" -o %s > /dev/null 2>&1\n",CMD,type)
		system(CMD)
		type="sm_menu_opt"
	        LINK=""
	        CMD = sprintf("%s", "odmdelete -q \"")}
/sm_name_hdr/ { CMD = sprintf ("%s\" -o %s > /dev/null 2>&1\n",CMD,type)
		system(CMD)
		type="sm_name_hdr"
	        LINK=""
	        CMD = sprintf("%s", "odmdelete -q \"")}
/sm_cmd_opt/ {  CMD = sprintf ("%s\" -o %s > /dev/null 2>&1\n",CMD,type)
		system(CMD)
		type="sm_cmd_opt"
	        LINK=""
	        CMD = sprintf("%s", "odmdelete -q \"")}

# main loop -- apply to all input lines
{
gsub(/"/,"'\''",$3)

if (type=="sm_cmd_hdr")
{
  if ($3 != "'\'\''") {

        if ($1 == "id") {
                ID = sprintf ("id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,ID)
		LINK="AND"
        }

        if ($1 == "help_msg_id") {
                MSG = sprintf ("help_msg_id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,MSG)
		LINK="AND"
        }

        if ($1 == "has_name_select") {
                NAMESEL = sprintf("has_name_select = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,NAMESEL)
		LINK="AND"
        }

        if ($1 == "ghost") {
                GHOST = sprintf("ghost = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,GHOST)
		LINK="AND"
        }

        if ($1 == "exec_mode") {
                EXEC = sprintf("exec_mode = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,EXEC)
		LINK="AND"
        }
  }
 } # end sm_cmd_opt

if (type=="sm_menu_opt")
{
  if ($3 != "'\'\''") {

        if ($1 == "id_seq_num") {
                ID_SEQ_NUM = sprintf ("id_seq_num = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,ID_SEQ_NUM)
		LINK="AND"
        }

        if ($1 == "id") {
                ID = sprintf ("id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,ID)
		LINK="AND"
        }

	if ($1 == "help_msg_id") {
                MSG = sprintf ("help_msg_id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,MSG)
		LINK="AND"
        }

        if ($1 == "alias") {
                ALIAS = sprintf("alias = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,ALIAS)
		LINK="AND"
        }

        if ($1 == "next_id") {
                NEXT_ID = sprintf("next_id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,NEXT_ID)
		LINK="AND"
        }
  }

 } # end if sm_menu_opt

if (type=="sm_name_hdr")
{
  if ($3 != "'\'\''") {
        if ($1 == "id") {
                ID = sprintf ("id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,ID)
		LINK="AND"
        }

        if ($1 == "help_msg_id") {
                MSG = sprintf ("help_msg_id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,MSG)
		LINK="AND"
        }


        if ($1 == "ghost") {
                GHOST = sprintf("ghost = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,GHOST)
		LINK="AND"
        }

        if ($1 == "type") {
                TYPE = sprintf("type = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,TYPE)
		LINK="AND"
        }

        if ($1 == "next_type") {
                NEXT_TYPE = sprintf("next_type = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,NEXT_TYPE)
		LINK="AND"
        }
  }
 } # end if name_hdr

if (type=="sm_cmd_opt")
{
  if ($3 != "'\'\''") {

        if ($1 == "id_seq_num") {
                ID_SEQ_NUM = sprintf ("id_seq_num = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,ID_SEQ_NUM)
		LINK="AND"
        }

        if ($1 == "id") {
                ID = sprintf ("id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,ID)
		LINK="AND"
        }

        if ($1 == "help_msg_id") {
                MSG = sprintf ("help_msg_id = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,MSG)
		LINK="AND"
        }


        if ($1 == "op_type") {
                OP_TYPE = sprintf("op_type = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,OP_TYPE)
		LINK="AND"
        }

        if ($1 == "entry_type") {
                ENTRY_TYPE = sprintf("entry_type = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,ENTRY_TYPE)
		LINK="AND"
        }
        if ($1 == "required") {
                REQUIRED = sprintf("required = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,REQUIRED)
	        LINK="AND"
        }

        if ($1 == "cmd_to_list_mode") {
                CMD_TO_LIST_MODE = sprintf("cmd_to_list_mode = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,CMD_TO_LIST_MODE)
		LINK="AND"
        }

        if ($1 == "multi_select") {
                MULTI_SELECT = sprintf("multi_select = %s", $3)
                CMD = sprintf ("%s %s %s",CMD,LINK,MULTI_SELECT)
		LINK="AND"
        }
  }
 } # end if cmd_opt
} # end main loop
END { CMD = sprintf ("%s\" -o %s  > /dev/null 2>&1\n",CMD,type); system (CMD) } ' $i
done

#link files from save directory to /usr/lib/assist.
#If one of the .add files is missing, then it's stanzas will not
#have been deleted in the first place, so no error message is printed.
ln -f /usr/lib/assist/add_files/sm_instdev.add /usr/lib/assist/sm_instdev.add > /dev/null 2>&1
ln -f /usr/lib/assist/add_files/sm_isa.add /usr/lib/assist/sm_isa.add > /dev/null 2>&1
ln -f /usr/lib/assist/add_files/sm_migrate.add /usr/lib/assist/sm_migrate.add > /dev/null 2>&1 
ln -f /usr/lib/assist/add_files/sm_overwrite.add /usr/lib/assist/sm_overwrite.add > /dev/null 2>&1
ln -f /usr/lib/assist/add_files/sm_preinstall.add /usr/lib/assist/sm_preinstall.add  > /dev/null 2>&1
ln -f /usr/lib/assist/add_files/sm_preserve.add /usr/lib/assist/sm_preserve.add  > /dev/null 2>&1
