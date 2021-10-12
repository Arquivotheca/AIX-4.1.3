$ @(#)00	1.37.1.2  src/bos/usr/bin/smit/odm_msg/smit.99.msg, odm_msg, bos411, 9434B411a 8/22/94 11:01:53

$
$  COMPONENT_NAME: (cmdsmit) SMIT -- System Management Interface Tool
$
$  FUNCTIONS:  none
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote '
$set 99
$ Where the comment "$ 50 column message.------..." occurs:
$       the message must be 50 columns or less wide,
$       subsequent message lines should not have leading \t characters,
$       and the last message line should not end with a \n.
$ do not translate SMIT, ODM, AIX


ERR_1800_074 '\
1800-074 A -m may not follow a -n or -d flag.\n\
\tUse only one of the -m, -n, or -d FastPath options.\n'
$ do not translate -m -n -d

ERR_1800_075 '\
1800-075 A -n may not follow a -m or -d flag.\n\
\tUse only one of the -m, -n, or -d FastPath options.\n'
$ do not translate -m -n -d

ERR_1800_076 '\
1800-076 A -d may not follow a -m or -n flag.\n\
\tUse only one of the -m, -n, or -d FastPath options.\n'
$ do not translate -m -n -d

ERR_1800_008 '\
1800-008 Specified FastPath key:\n\
\t"%1$s",\n\
\tis longer than %2$d characters. Reenter SMIT command line with corrected\n\
\tFastPath key.\n'
$ do not translate "%1$s"
$ do not translate %2$d

ERR_1800_009 '\
1800-009 Specified FastPath key sequence number:\n\
\t"%1$s",\n\
\tis longer than %2$d characters. Reenter SMIT command line with corrected\n\
\tFastPath key sequence number.\n'
$ do not translate "%1$s"
$ do not translate %2$d

MSG_USAGE '\
Usage [-CDfhMtvXx] [-o Object_repository_path]\n\
             [-l SMIT_main_log_file_path]\n\
             [-s SMIT_script_log_file_path]\n\
             [[-{n,m,d}] Id_FastPath]\n\
\t-d Id_FastPath = start with this dialog screen\n\
\t-C = start smit in the Curses mode\n\
\t-D = debug; set -t, -v, and internal trace modes\n\
\t-f = filter mode (do not flush stdin between screens and\n\
\t     do not check for a tty on stdout)\n\
\t-h = this help/usage message\n\
\t-M = start smit in the windows mode\n\
\t-m Id_FastPath = start with this menu screen\n\
\t-n Id_FastPath = start with this selector screen\n\
\t-o Object_repository_path = alternate SMIT/ODM database object repository\n\
\t     (directory) path (default path is /usr/lib/objrepos)\n\
\t-t = generate trace information for (main) log file\n\
\t-v = verbose (main) log file mode; show generated CommandStrings\n\
\t     for intermediate commands and the target task commands, together\n\
\t     with their output after they are run\n\
\t-x = do not run any Command_to_Execute (the actual task\n\
\t     CommandString), but continue to log any Command_to_Execute\n\
\t     to the (script, main) log files as usual\n\
\t-X = do not run any Command_to_Discover, Command_to_List,\n\
\t     Command_to_Classify, Command_to_Execute\n'
$ do not translate -DfhtvXx, -o, [-{n,m,d}]
$ do not translate -d
$ do not translate -D, -t, -v
$ do not translate -f, stdin
$ do not translate stdout
$ do not translate Curses
$ do not translate -h
$ do not translate -m
$ do not translate -o ODM
$ do not translate /usr/lib/objrepos
$ do not translate -t
$ do not translate -v
$ do not translate -x
$ do not translate -X

ERR_1800_093 '\
1800-093 Could not open SMIT log file:\n\
"%s".\n\
This session will not be logged. If logging is\n\
required, check path and permissions or use local\n\
problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

ERR_1800_102 '\
1800-102 Could not open SMIT script file:\n\
"%s".\n\
The commands run will not be logged.  If command\n\
logging is required, check path and permissions or\n\
use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

ERR_1800_001 '\
1800-001 TERM environment variable entry missing. TERM should be set to\n\
\tyour terminal type; set your TERM environment variable or use\n\
\tlocal problem reporting procecures\n'
$ do not translate TERM

ERR_1800_002 '\
1800-002 Could not access terminfo database for your terminal type. Use\n\
\tlocal problem reporting procedures.\n'
$ do not translate terminfo

ERR_1800_003 '\
1800-003 No tty on stdout; SMIT needs a terminal for normal interactive\n\
\tuse.  Use the -f flag to redirect stdin or stdout.\n'
$ do not translate tty, stdout
$ do not translate -f. stdin, stdout

ERR_1800_004 '\
1800-004 Initscr() error occurred. This may be due to insufficient storage\n\
\tfor starting program. Try again later or use local problem reporting\n\
\tprocedures.\n'

ERR_1800_005 '\
1800-005 SMIT requires a minimum of 24 line by 80 column output display.\n\
\tCheck terminal or window size and try again.\n'

ERR_1800_006 '\
1800-006 Internal error: unknown asl_init() return code. Use local problem\n\
\treporting procedures.\n'
$ do not translate asl_init()

ERR_1800_007 '\
1800-007 There are currently no SMIT\n\
screen entries available for this FastPath.\n\
This FastPath may require installation of\n\
additional software before it can be accessed.\n\
\n\
(Could not find SMIT database entry\n\
for given FastPath id:\n\
"%s".)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

ERR_1800_010 '\
1800-010 More than 1 matching menu item was\n\
found during Fast Path search.\n\
Extra matching entries will be ignored.\n\
\n\
(Internal error in contents of\n\
SMIT/ODM database entries; exactly one\n\
unique match should exist.)'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_011 '\
1800-011 There are currently no additional\n\
SMIT menu screen entries available for this\n\
item.  This item may require installation of\n\
additional software before it can be accessed.\n\
\n\
(Specified sm_menu_opt with\n\
id         = "%1$s",\n\
id_seq_num = "%2$s",\n\
was not found in SMIT/ODM database.)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate sm_menu_opt
$ do not translate id = "%1$s"
$ do not translate id_seq_num = "%2$s"

ERR_1800_012 '\
1800-012 Internal error: odm_get_list():\n\
object class = sm_menu_opt,\n\
id           = "%1$s",\n\
id_seq_num   = "%2$s".\n\
ODM error message:\n\
%3$s\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_get_list()
$ do not translate sm_menu_opt
$ do not translate id = "%1$s"
$ do not translate id_seq_num = "%2$s"
$ do not translate %3$s

MSG_002 '\
(Menu screen selected,\n\
\tFastPath    = "%1$s",\n\
\tid_seq_num  = "%2$s",\n\
\tnext_id     = "%3$s",\n\
\ttitle       = "%4$s".)'
$ do not translate "%1$s"
$ do not translate id_seq_num = "%2$s"
$ do not translate next_id = "%3$s"
$ do not translate title = "%4$s"

MSG_081 '\
(Menu screen selected as FastPath,\n\
\tid          = "%1$s",\n\
\tid_seq_num  = "%2$s",\n\
\tnext_id     = "%3$s",\n\
\ttitle       = "%4$s".)'
$ do not translate "%1$s"
$ do not translate id_seq_num = "%2$s"
$ do not translate next_id = "%3$s"
$ do not translate title = "%4$s"

MSG_003 '\
(Last entry above was an alias for next entry below.)'

ERR_1800_013 '\
1800-013 Internal error: odm_get_list():\n\
object_class = sm_menu_opt,\n\
id           = "%1$s",\n\
ODM error message:\n\
%2$s\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_get_list()
$ do not translate sm_menu_opt
$ do not translate id = "%1$s"
$ do not translate %2$s

ERR_1800_014 '\
1800-014 There are currently no additional\n\
SMIT menu screen entries available for this\n\
item.  This item may require installation of\n\
additional software before it can be accessed.\n\
\n\
(Specified sm_menu_opt with\n\
id = "%s",\n\
was not found in SMIT/ODM database.)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate sm_menu_opt
$ do not translate id = "%s"

ERR_1800_015 '\
1800-015 More than %d sm_menu_opt (menu)\n\
objects for next menu screen; extras ignored.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %d sm_menu_opt

MSG_004 '\
1 asl_screen() returned action/index = %1$d / %2$d'
$ do not translate asl_screen(), %1$d, %2$d'

MSG_005 '\
Help will appear when supported on this system.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_006 '\
This item is for information only. Press enter\n\
to exit this panel and then press F1 or Esc+1 =\n\
Help to check for additional information on this\n\
item.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_007 '\
2--exit smit_menu() action/index = %1$d / %2$d'
$ do not translate smit_menu(), %1$d, %2$d

ERR_1800_016 '\
1800-016 Error in SMIT-ODM database\n\
sm_menu_opt data descriptors.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate sm_menu_opt

MSG_009 '\
------entering smit_name_select (%1$s, %2$s, %3$d)'
$ do not translate smit_name_select, %1$s, %2$s, %3$d

ERR_1800_017 '\
1800-017 Internal error: odm_get_list() failed:\n\
object class = sm_name_hdr,\n\
id           = "%1$s".\n\
ODM error message:\n    %2$s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_get_list()
$ do not translate sm_name_hdr
$ do not translate id = "%1$s"

ERR_1800_018 '\
1800-018 There are currently no additional\n\
SMIT selector screen entries available for this\n\
item.  This item may require installation of\n\
additional software before it can be accessed.\n\
\n\
(Specified sm_name_hdr with\n\
id = "%s",\n\
was not found in SMIT/ODM database.)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate sm_name_hdr
$ do not translate id = "%s"

ERR_1800_019 '\
1800-019 Internal error: odm_get_list() failed:\n\
object class = sm_cmd_opt,\n\
id           = "%1$s".\n\
ODM error message:\n\
    %2$s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_get_list()
$ do not translate sm_cmd_opt
$ do not translate id, "%1$s"
$ do not translate  %2$s

ERR_1800_020 '\
1800-020 Some information needed for selected\n\
item is missing from SMIT ODM database.\n\
(Specified sm_cmd_opt with\n\
id         = "%1$s",\n\
id_seq_num = "%2$s",\n\
was not found in SMIT/ODM database.)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate sm_cmd_opt
$ do not translate id = "%1$s"
$ do not translate id_seq_num = "%2$s"

MSG_012 '\
Type or select a value for the entry field.\n\
Press Enter AFTER making all desired changes.'

MSG_013 '\
(Selector screen selected,\n\
\tFastPath  = "%1$s",\n\
\tid        = "%2$s",\n\
\tnext_id   = "%3$s",\n\
\ttitle     = "%4$s".)'
$ do not translate "%1$s"
$ do not translate id = "%2$s"
$ do not translate next_id = "%3$s"
$ do not translate title = "%4$s"

MSG_014 '\
1 asl_screen() returned action/index = %1$d / %2$d'
$ do not translate asl_screen(), %1$d, %2$d

MSG_015 '\
Help will soon be appearing here.'

ERR_1800_021 '\
1800-021 List generating command missing from SMIT/ODM database.'

MSG_017 '\
        Running the following Command_to_Classify:\n> %s\n'
$ do not translate >, %s

ERR_1800_022 '\
1800-022 Error exit status (%1$d) returned by\n\
Command_to_Classify; the output is:\n\
"%2$s"\n\
(The Command_to_Classify is:\n\
"%3$s".)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %1$d
$ do not translate "%2$s"
$ do not translate "%3$s"

MSG_018 '\
    Output from Command_to_Classify follows:\n\
"%s"\n'
$ do not translate "%s"

ERR_1800_023 '\
1800-023 This selector screen does not\n\
use a command_to_classify.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_024 '\
1800-024 Invalid name_hdr.type specified in SMIT/ODM database'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate name_hdr.type

ERR_1800_025 '\
1800-025 Invalid name_hdr.next_type specified in SMIT/ODM database'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate name_hdr.next_type

MSG_020 '\
2--exit smit_menu() action/index = %1$d / %2$d'
$ do not translate smit_menu(), %1$d,  %2$d

MSG_SM_MENU_OPT '\
    Object class: sm_menu_opt,\n\
        id   = "%1$s", id_seq_num = "%2$s", next_id = "%3$s",\n\
        text = "%4$s"'
$ ONLY translate Object class

MSG_SM_NAME_HDR '\
    Object class: sm_name_hdr,\n\
        id   = "%1$s", next_id = "%2$s", option_id = "%3$s",\n\
        name = "%3$s"'
$ ONLY translate Object class

MSG_SM_CMD_HDR '\
    Object class: sm_cmd_hdr,\n\
        id   = "%1s", option_id = "%2s",\n\
        name = "%3s"'
$ ONLY translate Object class

MSG_SM_CMD_OPT '\
    Object class: sm_cmd_opt,\n\
        id   = "%1$s", id_seq_num = "%2$s",\n\
        name = "%3$s"'
$ ONLY translate Object class

MSG_022 '\
----entering smit_dialog (%1$s, %2$s, %3$d)'
$ do not translate smit_dialog, %1$s, %2$s, %3$d

ERR_1800_026 '\
1800-026 Internal error: odm_get_list() failed:\n\
object class = sm_cmd_hdr,\n\
id           = "%1$s".\n\
ODM error message:\n\
    %2$s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_get_list()
$ do not translate sm_cmd_hdr
$ do not translate id, "%1$s"
$ do not translate %2$s

ERR_1800_027 '\
1800-027 There are currently no additional\n\
SMIT dialog screen entries available for this\n\
item.  This item may require installation of\n\
additional software before it can be accessed.\n\
\n\
(Specified sm_cmd_hdr with\n\
id = "%s",\n\
was not found in SMIT/ODM database.)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate sm_cmd_hdr
$ do not translate id = "%s"

MSG_024 '\
(A dialog matching the given FastPath was found. It was skipped since it\n\
requires a preceeding name select screen.  To override this action and to\n\
view (but not run) this dialog, insert the -Xd flags immediately before the \n\
FastPath on the SMIT CommandString.)'
$ do not translate -Xd

ERR_1800_094 '\
1800-094 The dialog screen with FastPath:\n\
"%s",\n\
requires a preceeding name select screen.  This\n\
dialog may be viewed (but not run) by inserting\n\
the -Xd flags immediately before the FastPath on\n\
the SMIT CommandString.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"
$ do not translate -Xd

MSG_025 '\
(Dialogue screen selected,\n\
\tFastPath    = "%1$s",\n\
\tid          = "%2$s",\n\
\ttitle       = "%3$s".)'
$ do not translate "%1$s"
$ do not translate id, "%2$s"
$ do not translate title, "%3$s"

ERR_1800_028 '\
1800-028 Internal error: odm_get_list() failed:\n\
object class = sm_cmd_opt,\n\
id           = "%1$s".\n\
ODM error message:\n\
    %2$s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_get_list()
$ do not translate sm_cmd_opt
$ do not translate id = "%1$s"
$ do not translate

ERR_1800_029 '\
1800-029 Too many command option entries for dialog menu; extras ignored.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_030 '\
1800-030 Required second line missing from\n\
current_value_set:\n\
"%s".\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

ERR_1800_101 '\
1800-030 Required second line missing from\n\
current_value_set.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_026 '\
Help will soon be appearing here.'

ERR_1800_031 '\
1800-031 No list generating command.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_027 '\
Running command...'

MSG_028 '\
2--exit sm_dialog() action/index = %1$d / %2$d'
$ do not translate sm_dialog(), %1$d, %2$d'

ERR_1800_032 '\
1800-032 Error detected while building the\n\
command line about to be displayed.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_033 '\
1800-033 Command will not be run due to error\n\
detected while building command line.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_029 '\
    Command_to_Execute follows below:\n\
>> %s\n'
$ do not translate >> %s\n

ERR_1800_034 '\
1800-034 Exit status (%d) returned by command.\n\
(Command process used the stdout and stderr\n\
file descriptors inherited from the SMIT process.)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %d
$ do not translate stdout, stderr

ERR_1800_035 '\
1800-035 Exit status (%d) returned by command.\n\
(Command process used stdout and stderr file\n\
descriptors connected to SMIT via pipes.)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %d
$ do not translate stdout, stderr

MSG_030 '\
    Size of output = %d bytes.'
$ do not translate %d

MSG_031 '\
    Output from Command_to_Execute follows below:'

MSG_START '\
---- start ----'

MSG_END '\
---- end ----'

MSG_CMD_STAT '\
COMMAND STATUS'

MSG_YES '\
yes'
$ 19 col. msg.--19|------------------------50|-----------------70|------79|

MSG_NO '\
no'
$ 19 col. msg.--19|------------------------50|-----------------70|------79|

MSG_FAILED '\
failed'
$ 19 col. msg.--19|------------------------50|-----------------70|------79|

MSG_RUNNING '\
running'
$ 19 col. msg.--19|------------------------50|-----------------70|------79|
$ "running" (executing) as in "the command is now running on the computer"

MSG_OK '\
OK'
$ 19 col. msg.--19|------------------------50|-----------------70|------79|

ERR_1800_082 '\
1800-082 SMIT/ODM database data error: invalid\n\
ring index = %1$d does not correspond to valid ring\n\
field option for command option entry with name:\n\
"%2$s".\n\
Last ring entry will be used as default. Use\n\
local problem reporting procedures.'
$ do not translate %1$d
$ do not translate "%2$s"
$ a ring entry is a multiple choice field

MSG_037 '\
\n\
[%s]\n\
    Starting SMIT'
$ do not translate %s

ERR_1800_037 '\
1800-037 Cannot open catalog message file;\n\
using ODM English language message fields.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_038 '\
1800-038 odm_initialize() failed,\n\
ODM error message:\n\
%s\n\
Check path and permissions for\n\
the object repository path of\n\
the SMIT/ODM database or use local\n\
problem reporting procedures.\n\
The -o command line parameter may be\n\
used to change this path from its\n\
default value.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_initialize()
$ do not translate %s
$ do not translate SMIT/ODM

ERR_1800_078 '\
1800-078 ODM odm_set_path() failed on object\n\
repository path (of the SMIT/ODM database):\n\
"%s".\n\
Check path and permissions for\n\
the object repository path of\n\
the SMIT/ODM database or use local\n\
problem reporting procedures.\n\
The -o command line parameter may be\n\
used to change this path from its\n\
default value.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_set_path()
$ do not translate SMIT/ODM
$ do not translate "%s"
$ do not translate -o

ERR_1800_036 '\
1800-036 ODM odm_open_class() failed for\n\
(SMIT/ODM database) object class (file):\n\
"%1$s"\n\
on object repository path:\n\
"%2$s".\n\
Check path and permissions for\n\
the object repository path of\n\
the SMIT/ODM database or use local\n\
problem reporting procedures.\n\
The -o command line parameter may be\n\
used to change this path from its\n\
default value.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_open_class()
$ do not translate SMIT/ODM
$ do not translate "%1$s"
$ do not translate "%2$s"
$ do not translate -o

ERR_1800_040 '\
1800-040 Cannot open SoftCopy help information\n\
database.  Help is not available for this\n\
SMIT session.  You may continue from here or\n\
you can check the $LANG environment variable\n\
setting or use local problem reporting\n\
procedures.   For the current setting of\n\
$LANG, the help database files should be:\n\
"%s",\n\
"%s".'

$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate $LANG
$ do not translate .key, "%s"
$ do not translate .rom, "%s"
$ do not translate .sys, "%s"

MSG_038 '\
\tORIERR: %d, ori2_open() on SoftCopy help information datbase failed.'

ERR_1800_041 '\
1800-041 odm_close_class() for object sm_menu_opt.\n\
ODM error message:\n\
%s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_close_class(), sm_menu_opt
$ do not translate %s

ERR_1800_042 '\
1800-042 odm_close_class() for object sm_name_hdr.\n\
ODM error message:\n\
%s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_close_class(), sm_name_hdr
$ do not translate %s

ERR_1800_043 '\
1800-043 odm_close_class() for object sm_cmd_hdr.\n\
ODM error message:\n\
%s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_close_class(), sm_cmd_hdr
$ do not translate %s

ERR_1800_044 '\
1800-044 odm_close_class() for object sm_cmd_opt.\n\
ODM error message:\n\
%s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_close_class(), sm_cmd_opt
$ do not translate %s

ERR_1800_045 '\
1800-045 ORIERR: %d, database close failed.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate  %d

ERR_1800_046 '\
1800-046 odm_terminate() failed.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_terminate()

ERR_1800_047 '\
1800-047 An abnormal condition forced SMIT to exit prematurely.\n\
\tThe log file may contain additional information.\n'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_040 '\
       Doing following Command_to_List:\n\
>%s\n'
$ do not translate >%s

ERR_1800_050 '\
1800-050 Error exit status (%1$d) returned by\n\
Command_to_List; the output is:\n\
"%2$s"\n\
(The Command_to_List is:\n\
"%3$s".)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %1$d
$ do not translate "%2$s"
$ do not translate "%3$s"

ERR_1800_051 '\
1800-051 There are no items of this type.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_042 '\
    Output from Command_to_List follows:\n\
%s\n'
$ do not translate %s

ERR_1800_049 '\
1800-049 Invalid Command_to_List mode,\n\
"%s", specified in SMIT database.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

MSG_043 '\
    Command_to_Discover follows:\n\
%s'
$ do not translate %s

ERR_1800_081 '\
1800-081 Error exit status (%1$d) returned by\n\
Command_to_Discover; the output is:\n\
"%2$s".\n\
\n\
(The Command_to_Discover is:\n\
"%3$s".)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %1$d
$ do not translate "%2$s"
$ do not translate %3$s

ERR_1800_091 '\
1800-091 Format error ("#" missing) in output\n\
from the Command_to_Discover:\n\
"%1$s".\n\
\n\
(The Command_to_Discover is:\n\
"%2$s".)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "#"
$ do not translate %1$s
$ do not translate %2$s

ERR_1800_092 '\
1800-092 Format error: "\\n" missing in output\n\
from following Command_to_Discover:\n\
"%1$s".\n\
\n\
(The Command_to_Discover is:\n\
"%2$s".)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "\\n"
$ do not translate %1$s
$ do not translate %2$s

MSG_044 '\
    Command_to_Discover output follows:\n\
%s'
$ do not translate %s

ERR_1800_083 '\
1800-083 Data error: could not find matching ring\n\
field value for corresponding Command_to_Discover\n\
value.  sm_cmd_opt[%1$d].disc_field_name:\n\
"%2$s".\n\
The current actual value will be used as the\n\
default value for the following dialog.  Use\n\
local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate sm_cmd_opt[%1$d].disc_field_name
$ do not translate "%2$s"

ERR_1800_052 '\
1800-052 xsystem(): pipe() system call\n\
on stdout failed:\n\
%s.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), pipe()
$ do not translate stdout
$ do not translate %s

ERR_1800_053 '\
1800-053 xsystem(): pipe() system call\n\
on stderr failed:\n\
%s.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), pipe()
$ do not translate stderr
$ do not translate %s

ERR_1800_077 '\
1800-077 xsystem(): execl() system call failed:\n\
%s.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), execl()
$ do not translate %s

ERR_1800_054 '\
1800-054 xsystem(): execl() system call failed,\n\
invalid return code = %d.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), execl()
$ do not translate %d

ERR_1800_055 '\
1800-055 xsystem(): fork() system call failed.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), fork()

ERR_1800_056 '\
1800-056 xsystem(): fcntl() error on stdoutd[0], errno = %d\n.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), fcntl(), stdoutd[0], errno, %d.

ERR_1800_057 '\
1800-057 xsystem(): fcntl() error on stderrfd[0], errno = %d\n.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), fcntl(), stderrd[0], errno, %d.

MSG_045 '\
stdinfd[1] = %1$d, stdoutfd[0] = %2$d, stderrfd[0] = %3$d (decimal)'
$ ONLY translate decimal

ERR_1800_058 '\
1800-058 xsystem(): select() failed:\n\
%s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ ONLY translate failed

MSG_050 '\
(select() returned errno = EINTR, was ignored).'
$ do not translate select(), errno, EINTR

MSG_051 '\
polrc: %1$d, rtnevents[0]: %2$x (hex), [1]: %3$x'
$ ONLY translate hex

MSG_052 '\
selrc: %1$d, read_list.fdsmask[0]: %2$8x (hex)'
$ ONLY translate hex

MSG_053 '\
selrc: %1$d, write_list.fdsmask[0]: %2$8x (hex)'
$ ONLY translate hex

ERR_1800_059 '\
1800-059 xsystem(): read(stdout) failed,\n\
returned errno = %d'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), read(stdout)
$ do not translate errno, %d

MSG_054 '\
xsystem(): selected read(stdout) returned %d characters'
$ do not translate xsystem(), read(stdout), %d

MSG_055 '\
xsystem(): read(): "%*.*s"\n'
$ do not translate this entry

ERR_1800_060 '\
1800-060 xsystem(): read(stderr) failed,\n\
returned errno = %d'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), read(stderr)
$ do not translate errno = %d

MSG_056 '\
xsystem(): selected read(stderr) returned %d characters'
$ do not translate xsystem(), read(stderr), %d

MSG_057 '\
xsystem(): wait() return value = %1$d, (errno = %2$d)'
$ do not translate xsystem(), wait(), %1$d, errno, %2$d

ERR_1800_089 '\
1800-089 The command being run has been\n\
halted.  (The process for the command was\n\
halted by signal = %d.)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %d

MSG_058 '\
xsystem(): wait() signal return status = %x (hex), %d (decimal)'
$ do not translate xsystem(), wait(), %x, %d

MSG_059 '\
xsystem(): wait() exit return status =  %x (hex), %d (decimal)'
$ do not translate xsystem(), wait(), %x, %d

MSG_060 '\
xsystem_term(): Signal caught: %d'
$ do not translate xsystem_term(), %d

MSG_061 '\
Exiting due to SIGTERM Signal.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate SIGTERM

MSG_062 '\
Command: %1$-14.14sstdout: %2$-14.14sstderr: %3$-14.14s\n\n\
Before command completion, additional instructions may appear below.'
$ do not translate %1$-14.14sstdout, %2$-14.14sstderr, %3$-14.14s

ERR_1800_080 '\
1800-080 Signal (SIGTERM) received = %d;\n\
Treating as an unconditional exit request.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate SIGTERM, %d

ERR_1800_061 '\
\n\
1800-061 Exiting SMIT (signal %d received).\n'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %d

MSG_064 '\
error %d occurred.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %d

ERR_1800_090 '\
1800-090 Possible SMIT/ODM data value error;\n\
lookup of following command postfix string token\n\
failed:\n\
"%s".\n\
It will be ignored. Use local problem reporting\n\
procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

MSG_066 '\
Looked up name:\n\
    "%1$s"\n\
with value string:\n\
    "%2$s"'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%1$s"
$ do not translate "%2$s"

ERR_1800_096 '\
1800-096 Newline missing in current_value_set.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_067 '\
New discover string:\n\
    "%s"'
$ do not translate "%s"

MSG_068 '\
Getting help information ...'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_070 '\
1800-070 Exit status (%1$d) returned by man command\n\
while attempting to retrieve help information.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %1$d

ERR_1800_063 '\
1800-063 Could not successfully complete database\n\
search for requested help information.\n\
Use local problem reporting procedures.\n\
\n\
(ori2_search() failed:\n\
%1$s,\n\
ori2_search(): ori_query parameter is:\n\
"%2$s".)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate ORIERR, %1$d, ori2_search()
$ do not translate ori2_search(), ori_query
$ do not translate "%2$s"

ERR_1800_098 '\
1800-098 Help information missing from softcopy\n\
database.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_069 '\
ori2_search() found no matches;\n\
ori2_search(): ori_query parameter is:\n\
"%s"'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate ori2_search()
$ do not translate ori2_search(), ori_query
$ do not translate "%s"

ERR_1800_064 '\
1800-064 Could not read help information from\n\
softcopy database; no data available for this\n\
item.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_071 '\
ori2_readf() failed.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate ori2_readf()

MSG_BEGIN_RAW_TEXT '\
-------- Beginning of raw help text buffer --------'


MSG_END_RAW_TEXT '\
-------- End of raw help text buffer --------'

ERR_1800_065 '\
1800-065 ori2_seek() failed. ori_errno is %d.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate ori2_seek()
$ do not translate ori_errno
$ do not translate %d

ERR_1800_068 '\
1800-068 Multiple help entries were found.\n\
Only the first entry found will be displayed.\n\
Use local problem reporting procedures.\n\
\n\
(ori2_search(): found %1$d matching tags;\n\
ori_query parameter is:\n\
"%2$s".)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate ori2_search(), ori_query
$ do not translate %1$d
$ do not translate %2$s

ERR_1800_069 '\
1800-069 Could not successfully complete database\n\
access for requested help information.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_095 '\
1800-095 For more information on this topic,\n\
proceed to the next level of SMIT.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_071 '\
1800-071 No help available for this topic.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_097 '\
1800-097 Retrieval of ODM error message failed.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_073 '\
1800-073 ODM error number %d'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate %d

ERR_1800_039 '\
1800-039 Internal error: odm_free_list() failed.\n\
Object class:\n\
    %s\n\
Use local error reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_free_list()
$ do not translate %s

MSG_077 '\
Help is not available in this version'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_084 '\
1800-084 System error: reduce_sbrks():\n\
malloc(): There is not enough memory available\n\
now. Try again later or use local problem\n\
reporting procedures.\n\
SMIT will exit immediately.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate reduce_sbrks()
$ do not translate malloc()

MSG_078 '\
+ reduce_sbrks ptr = 0x%x'

ERR_1800_085 '\
1800-085 Internal error: internally constructed\n\
string would be too long for target field if\n\
copied; result will be truncated:\n\
"%1$.100s"....\n\
Maximum size of %2$d characters exceeded.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%1$.100s"
$ do not translate %2$d

ERR_1800_086 '\
1800-086 Internal error: internally constructed\n\
string would be too long if following strings\n\
are concatenated; result will be truncated:\n\
"%1$.100s"....\n\
"%2$.100s"....\n\
Maximum size of %3$d characters exceeded.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%1$.100s"
$ do not translate "%2$.100s"
$ do not translate %3$d

MSG_079 '\
+ smit_free()    pointer = 0x%1$x    (call line number = %2$d)'
$ do not translate smit_free(), 0x%1$x, %2$d

ERR_1800_087 '\
1800-087 System error: smit_realloc(): realloc():\n\
There is not enough memory available now.\n\
Try again later or use local problem reporting\n\
procedures.\n\
SMIT will exit immediately.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate smit_realloc(), realloc()

ERR_1800_088 '\
1800-088 System error: smit_malloct(): malloc():\n\
There is not enough memory available now.\n\
Try again later or use local problem reporting\n\
procedures.\n\
SMIT will exit immediately.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate smit_malloct(), malloc()

ERR_1800_099 '\
1800-099 NOTE: This command may change a terminal\n\
mode that may affect SMIT, or it may need to be\n\
run outside of SMIT for other reasons.\n\
Therefore SMIT will exit immediately before\n\
running this command.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_080 '\
(A selector screen entry matching the given FastPath was found. It was skipped\n\
since it requires a preceeding selector screen.  To override this action and to\n\
view (but not run) this dialog, insert the -Xn flags immediately before the \n\
FastPath on the SMIT CommandString.)'
$ do not translate -Xn

ERR_1800_100 '\
1800-100 The selector screen with FastPath:\n\
"%s",\n\
requires a preceeding selector screen.  This\n\
selector screen may be viewed (but not run) by\n\
inserting the -Xn flags immediately before the\n\
FastPath on the SMIT CommandString.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"
$ do not translate -Xn

MSG_INIT_SMIT '\
Initializing SMIT ...'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_PROCESSING_DATA '\
Processing data ...'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_ORIGIN '\
Origin Date = %1$s and Time = %2$s'
$ do not translate %1$s,  %2$s

MSG_EXIT_SMIT '\
Exiting SMIT'

MSG_CMD_NOT_RUN '\
#\n\
\n\
**** internal command was not run. (SMIT -X CommandString flag set.) ****'
$ do not translate #\n
$ do not translate SMIT, -X

MSG_CMD_LOGGED '\
Command has been logged to script file.\n\
(SMIT -x CommandString flag set)'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate SMIT, -x

ERR_1800_104 '\
1800-104 An alias entry id duplicates another\n\
alias or menu entry in the SMIT/ODM database.\n\
(Specified sm_menu_opt has the following\n\
duplicate id:\n\
"%1$s".)\n\
Duplicate entries are ignored.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate SMIT/ODM
$ do not translate id
$ do not translate "%1$s"

MSG_RANGE '\
The value for this entry field must be in the\n\
range shown below.\n\
Press Enter or Cancel to return to the entry field,\n\
and enter the desired value.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_ORI '\
ORI error number: %d'
$ do not translate ORI, %d
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_PRESS_ENTER '\
\n\
Press Enter to continue.\n'
$ do not translate \n
$ 79 column message.---------------------------50|-----------------70|------79|

ERR_1800_105 '\
1800-105 system("sync"): system call failed:\n\
%s.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate system("sync"):
$ do not translate %s

MSG_BAD_TERM '\
The environment variable TERM is currently set to a terminal type that does\n\
not support the full screen display capabilities required for SMIT.\n\
Set TERM to an appropriate terminal type or use another terminal.'
$ 80 column message.---------------------------50|-----------------70|------79|
$ Do not translate TERM.

ERR_1800_106 '\
1800-106 An error occurred:\n\
\n\
%1$s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%1$s"

ERR_1800_109 '\
1800-109 There are currently no additional\n\
SMIT screen entries available for this\n\
item.  This item may require installation of\n\
additional software before it can be accessed.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1800_110 '\
1800-007 There are currently no SMIT\n\
screen entries available for this FastPath.\n\
This FastPath may require installation of\n\
additional software before it can be accessed.'

ERR_1800_111 '\
1800-111 A value must be specified in the\n\
entry field for:\n\
"%s".'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

ERR_1800_GEN_CLOSE '\
1800-041 odm_close_class() for object %s.\n\
ODM error message:\n\
%s'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate odm_close_class(), sm_menu_opt
$ do not translate %s

PRESS_ENTER_TO_CONTINUE 'press Enter to continue'
$ There should be no continuation symbols '\' or new lines

PRESS_THE_ENTER_KEY_TO_CONTINUE 'Press the enter key to continue'
$ There should be no continuation symbols '\' or new lines

PRESS_THE_ENTER_KEY_TO_CONTINUE_2 'Press the Enter key to continue'
$ There should be no continuation symbols '\' or new lines

MSG_SUPERUSER '\
You must be a superuser to run DSMIT.'
 
MSG_GET_LICENSE '\
\nGetting license.....\n'

MSG_STARTING_DSMIT '\
\n\
[%s]\n\
    Starting Distributed SMIT'
$ do not translate %s

MSG_EXIT_DSMIT '\
Exiting Distributed SMIT'


$ #####################################################################################
$set 100

MSG_GEN_HELP_1 '\
                                Using SMIT\n\
Table of Contents\n\
  1. Overview\n\
  2. Understanding the Menu Screen\n\
  3. Understanding the Dialog Screen\n\
  4. Understanding the Command Status Panel\n\
  5. Making Selections from a List of Choices\n\
  6. Using SMIT Function Keys \n\
  7. Using SMIT Fastpath\n\
  8. Related information in InfoExplorer\n\
\n\
\n\
  Home or Esc-<        Scroll to top of command output.\n\
\n\
  End or Esc->         Scroll to bottom of command output.\n\
\n\
  PageUp or Esc-V      Scroll to the previous page.\n\
\n\
  PageDown or Ctrl-V   Scroll to the next page.\n\
\n\
\n\
1. Overview\n\
\n\
  SMIT (System Management Interface Tool) is an interactive interface\n\
  application designed to simplify system management tasks.  It displays a\n\
  hierarchy of menus that lead to interactive dialogs.\n\
\n\
\n\
2. Understanding the Menu Screen\n\
\n\
  The menu title appears at the top of the screen.  The function keys for\n\
  the menu appear at the bottom of the screen.  A list of menu items\n\
  appears on the left of the screen.\n\
\n\
  Use the cursor keys (up and down arrow keys) to move to any of the menu\n\
  items.  Press Enter (Return) to select a menu item.  Then one of the\n\
  following occurs:\n\
\n\
    * another menu appears\n\
    * command output appears (i.e. for a list request)\n\
    * a pop-up select screen appears\n\
    * a dialog screen appears\n\
\n\
'
$ 78 column message.---------------------------50|-----------------70|------79|

MSG_GEN_HELP_2 '\
3. Understanding the Dialog Screen\n\
\n\
  The dialog title appears at the top of the screen.  The function keys for\n\
  the dialog appear at the bottom of the screen.  Descriptions of the various\n\
  fields appear on the left of the screen.  Entry fields appear on the right\n\
  side of of the dialog screen.  Use the cursor keys to move to any entry\n\
  field.\n\
\n\
  In a dialog, you are asked to supply details of the task you previously\n\
  selected.  Symbols are used to indicate various types of fields.  The\n\
  symbols may be displayed in various places in the dialog.  These symbols\n\
  are not displayed in a menu.\n\
\n\
  Symbol   Meaning\n\
\n\
  [ ]      Indicates a typeable entry field.\n\
\n\
\n\
  < >      Indicates there is more text to the left or right of the visible\n\
           entry field.\n\
\n\
  *        Indicates this field requires a value.\n\
\n\
    The following symbols appear on the right side of the screen:\n\
\n\
  +        Indicates a list of choices (or an option ring) is available.\n\
           Press F4 to display the list or use the Tab key to display one\n\
           choice at a time.\n\
\n\
  #        Indicates a numeric field.\n\
\n\
  X        Indicates a hexadecimal field.\n\
\n\
  /        Indicates an accessible file name.\n\
\n\
\n\
  Highlighted entry\n\
           Indicates a user modified entry.  Highlighted entries are in\n\
           reverse video.\n\
\n\
'
$ 78 column message.---------------------------50|-----------------70|------79|

MSG_GEN_HELP_3 '\
4. Understanding the Command Status Panel\n\
\n\
  A command associated with a dialog screen is invoked when Enter is pressed.\n\
  The normal result from issuing a command is to have a command status screen\n\
  displayed.  While the command is running, "Command: running" will be\n\
  displayed in the upper left portion of the screen and the function keys will\n\
  not be displayed.  As output is generated by the command, it will be\n\
  displayed on the screen.  When the command completes, Command: OK or\n\
  Command: Failed appears and the first lines of the command output will be\n\
  redisplayed in the the panel.\n\
\n\
  If the output does not fit on the screen, the cursor keys can be used to\n\
  scroll through the information.  The key sequences detailed in Using SMIT\n\
  Function Keys can also be used for scrolling.\n\
\n\
    [TOP]       indicates the first line of the text.\n\
    [BOTTOM]    indicates the last line of text.\n\
    [MORE...nn] indicates nn lines of text are not displayed on the screen.\n\
\n\
  Besides the normal command status panel used for output, there are\n\
  special cases where SMIT clears the screen and gives control of the screen\n\
  to the application being invoked.\n\
\n\
'
$ 78 column message.---------------------------50|-----------------70|------79|

MSG_GEN_HELP_4 '\
5. Making Selections from a List of Choices\n\
\n\
  Up and Down Cursor Keys\n\
    Moves between fields to select an entry.\n\
\n\
  Tab Key\n\
    Press to display the choices for an entry with a "+" symbol to the right\n\
    of the entry.  If the entry field is blank,  press the List key instead\n\
    of using Tab.  Press Shift-Tab to display the previous choice.\n\
\n\
  To select one item from a single select pop-up:\n\
\n\
    Move the cursor to the correct item.\n\
    Press Enter to select the item.\n\
\n\
  To select more than one item from a multiple select pop-up:\n\
\n\
    Move the cursor to the first item to select.\n\
    Press F7 to select the individual item (Press F7 a second time to\n\
    deselect the item).\n\
    Continue selecting until all desired items are highlighted.  Then press\n\
    Enter to select the entire group of selections.\n\
\n\
6. Using SMIT function keys\n\
\n\
  In SMIT, various function keys appear at the bottom of the menu or dialog\n\
  screens.  Only those functions that are valid for the specific menu or\n\
  dialog are displayed.  The following describes these function keys.  If a\n\
  key sequence (Esc+number) is used, press the Esc key, quickly followed by\n\
  the letter key.\n\
\n\
  The following functions are valid if your keyboard has not been remapped.\n\
\n\
'
$ 78 column message.---------------------------50|-----------------70|------79|

MSG_GEN_HELP_5 '\
  Functions       Description\n\
\n\
  F1 or Esc+1 = Help\n\
                  Gives more information on the item selected.  In\n\
                  some areas of SMIT, for more information on the topic\n\
                  go to the next level.\n\
\n\
  F2 or Esc+2 = Refresh\n\
                  Redraws the screen. Use if console messages overwrite the\n\
                  screen.\n\
\n\
  F3 or Esc+3 = Cancel\n\
                  Backs up one level to the previous screen.\n\
\n\
  F4 or Esc+4 = List\n\
                  Presents a list of possible choices for an entry field. A\n\
                  scrollable pop-up is displayed.\n\
\n\
  F5 or Esc+5 = Reset\n\
                  Resets the entry field to the original setting.\n\
\n\
  F6 or Esc+6 = Command\n\
                  Displays the command that SMIT is building.\n\
\n\
  F7 or Esc+7 = Edit\n\
                  Allows editing of an entry field.\n\
                  Use if a larger entry field is needed.\n\
\n\
  F7 or Esc+7 = Select\n\
                  Makes individual selections on muti-select lists.\n\
\n\
  F8 or Esc+8 = Image\n\
                  Saves screen image in the smit.log file. Also displays the\n\
                  FastPath.\n\
\n\
  F9 or Esc+9 = Shell\n\
                  Escapes to a shell.\n\
\n\
  F10 or Esc+0 = Exit\n\
                  Exit from SMIT.\n\
\n\
  Enter = Do      Runs the command built by SMIT.\n\
\n\
  Home or Esc-<   Scroll to top of command output.\n\
\n\
  End or Esc->    Scroll to bottom of command output.\n\
\n\
  PageUp or Esc-V\n\
                  Scroll to the previous page.\n\
\n\
  PageDown or Ctrl-V\n\
                  Scroll to the next page.\n\
\n\
'
$ 78 column message.---------------------------50|-----------------70|------79|

MSG_GEN_HELP_6 '\
7. SMIT FastPath\n\
\n\
  The SMIT FastPath is a shortcut method to display a SMIT dialog panel\n\
  or menu directly from the command line.   Press the Image key\n\
  to display the FastPath for the current screen.\n\
\n\
  To derive the SMIT FastPath for device related tasks, combine an\n\
  Action Prefix with the Device Abbreviation.\n\
\n\
  Action          Prefix                Device        Abbreviation\n\
  ------          ------                ------        ------------\n\
\n\
  (add)             mk                 (printer)         prt\n\
\n\
  (change)          ch                 (tty)             tty\n\
\n\
  (list)            ls                 (pty)             pty\n\
\n\
  (remove)          rm                 (disk)            dsk\n\
\n\
                                       (cdrom)           cdr\n\
\n\
                                       (diskette)        dskt\n\
\n\
                                       (tape)            tpe\n\
\n\
                                       (adapter)         adp\n\
\n\
  Examples of FastPaths for devices are:\n\
\n\
  1. To add a printer device, enter:\n\
\n\
     smit mkprt\n\
\n\
  This takes you to the menu that guides you through the steps of adding a\n\
  printer device.\n\
\n\
  2. To list tape devices you want to work with, enter:\n\
\n\
     smit lstpe\n\
\n\
  This takes you to the menu or dialog that lists tape devices that are\n\
  configured on your system.\n\
\n\
'
$ 78 column message.---------------------------50|-----------------70|------79|

MSG_GEN_HELP_7 '\
8. Related Information.\n\
\n\
  The InfoExplorer system also contains information describing the use\n\
  SMIT for System Management tasks.  The following are articles that can\n\
  be referenced in the InfoExplorer data base.\n\
\n\
         "SMIT Command" provides information on the smit command;\n\
         what flags can be invoked to vary the output of SMIT, and\n\
         how to use the smit.script file and the smit.log file.\n\
\n\
         "SMIT Overview for Programmers" provides information you can\n\
         use to understand various programming aspects of the SMIT menu\n\
         hierarchy, including how to add new menus and dialogs.\n\
\n\
         "Installation and Maintenance in SMIT" provides information you\n\
         can use for installing and updating software, and other tasks.\n\
\n\
         "Devices in SMIT" provides information you can use to add, change,\n\
         list or remove devices, among other tasks.\n\
\n\
         "Physical and Logical Storage in SMIT" provides information you\n\
         can use to add, change, remove, list, mount, work with network\n\
         file systems, among other tasks.\n\
\n\
         "Security and Users in SMIT" provides information you can use to\n\
         add, change, list or remove users or user groups from the system,\n\
         among other tasks.\n\
\n\
         "Communications Applications and Services in SMIT" provides\n\
         information you can use to perform configuration tasks for\n\
         Network File Systems (NFS), SNA, TCP/IP, and HCON.\n\
\n\
         "Spooler in SMIT" provides information you can use to start or\n\
         cancel printing jobs.  You can also manage queues or queue\n\
         devices.\n\
\n\
         "Problem Determination in SMIT" provides information you can use\n\
         to work with error logs, reports, and dumps.  You can also run\n\
         hardware diagnostics.\n\
\n\
         "Performance and Resource Scheduling in SMIT" provides information\n\
         you can use to set the system runlevel, report system activity or\n\
         schedule jobs, among other tasks.\n\
\n\
         "System Environments in SMIT" provides information you can use to\n\
         manage processes, broadcast messages to users, stop your system,\n\
         assign consoles, or change keyboard mappings, among other tasks.'

MSG_ON '\
 on '

ERR_NO_VALUE '\
No value_set found for a given host.'

MSG_090 '\
Submit Command for Execution.'

MSG_091 '\
Continuing will submit the command(s) to all the machines\n\
included in the Collective. This is your last chance to\n\
stop before execution.\n\
   Press Enter to execute command(s) on all machines.\n\
   Press Cancel to return to dialogue.'

MSG_092 '\
Machine for Specific Selections'

MSG_093 '\
The following hosts have been excluded due\n\
to consistency reasons: %s'

MSG_094 '\
Common Dialogue for '

MSG_095 '\
# Distributed Execution Mode\nsequential\nconcurrent'

MSG_096 '\
Failed host(s) have been auto-excluded:\n\n%s'

MSG_097 '\
Conflict in selecting the next screen!\n\
Make other selections in previous screens, and/or\n\
exclude some hosts from the Collective.'

ERR_1800_201 '\
1800-201 Too many hosts included in the collective; extras ignored.'

ERR_1800_203 '\
1800-203 Failure to restore consistency.'

ERR_1800_204 '\
1800-204 Redefining collective failed:\n\n%s'

ERR_1800_205 '\
1800-205 Failed attempt to get the local host name.'

ERR_1800_206 '\
1800-206 Unsuccesful execution.'

ERR_1800_207 '\
1800-207 No included hosts in the Collective.'

ERR_1800_208 '\
1800-208 Failure to auto-exclude failed host(s):\n\n         %s'

ERR_1800_209 '\
1800-209 Redefining collective failed:\n\n%s\n\
Check if the host(s) are defined in\n\
/usr/share/DSMIT/hosts.'

ERR_1800_210 '\
1800-210 Redefining collective failed:\n\n\
Check if the domain(s) are defined correctly in\n\
/usr/share/DSMIT/domains.'

ERR_1800_211 '\
1800-211 Redefining collective failed:\n\n%s\n\
Check if the host(s) are defined in\n\
/usr/share/DSMIT/hosts.\n\n\
Check if the domain(s) are defined in\n\
/usr/share/DSMIT/domains.'

ERR_1800_220 '\
1800-220 xsystem(): execl() system call failed:\n\
%s.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), execl()
$ do not translate %s

ERR_1800_221 '\
1800-221 xsystem(): execl() system call failed,\n\
invalid return code = %d.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate xsystem(), execl()
$ do not translate %d

MSG_098 '\
Host'

MSG_099 '\
Domain'
