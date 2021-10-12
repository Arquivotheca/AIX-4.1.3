$ @(#)51 1.38.2.2  src/bos/usr/ccs/lib/libasl/asl.msg, libasl, bos41J, 9520A_all 5/12/95 13:24:41

$
$  COMPONENT_NAME: (libasl) ASL -- AIX Screen Library
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
$set 1
$ Where the comment "$ 50 column message.------..." occurs:
$	the message must be 50 columns or less wide,
$	subsequent message lines should not have leading \t characters,
$ 	and the last message line should not end with a \n.
$ do not translate SMIT
$ Enter, Esc are names of keyboard keys
$ Cancel is is the keyboard function invoked by the F3 key or Esc+3 keys

ERR_1820_001 '\
1820-001 Internal error: asl_init() must be called before asl_screen().\n\
Use local problem reporting procedures.\n'
$ do not translate asl_init(), asl_screen()

ERR_1820_002 '\
1820-002 Cursor must be positioned on a valid\n\
entry field for resetting the default value.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1820_003 '\
1820-003 Cursor must be positioned on a valid\n\
entry field for editing.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1820_004 '\
1820-004 Cursor must be positioned on a valid\n\
entry field for generating popup list.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1820_005 '\
1820-005 Popup list is not available for this\n\
entry field.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1820_006 '\
1820-006 Internal error: asl_screen() called\n\
with an invalid screen code.  Use local problem\n\
reporting proceedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_screen()

MSG_001 '\
Doing shell escape.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_002 '\
Current fast path:\n\
    "%s"'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

ERR_1820_008 '\
1820-008 Internal error: ring field size:\n\
index (%1$d),\n\
maximum size (%2$d),\n\
length (%3$d).\n\
Use local problem reporting proceedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate (%1$d), (%2$d), (%3$d)
$ a ring field is a multiple choice field

ERR_1820_007 '\
1820-007 Internal error: asl_init() must\n\
\tbe called before asl_read().\n'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_init(),  asl_read()


ERR_1820_010 '\
1820-010 Log file for printing the screen has\n\
not been provided by calling application.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_003 '\
\f---------- start of screen ----------\n\n'

MSG_004 '\
\n\n---------- end of screen ----------\n\n'

MSG_005 '\
GENERAL HELP'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_006 '\
CONTEXTUAL HELP'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_007 '\
SHOW COMMAND STRING'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_008 '\
**** Title Missing ****'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_009 '\
Type or select values in entry fields.\n\
Press Enter AFTER making all desired changes.'

MSG_010 '\
Move cursor to desired item and press F7.\n\
    ONE OR MORE items can be selected.\n\
Press Enter AFTER making all selections.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Indent second sentence 4 columns.

MSG_011 '\
Move cursor to desired item and press Enter.'
$ 70 column message.---------------------------50|-----------------70|------79|

MSG_012 '\
Press Enter or Cancel to return to the\n\
application.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_EDIT_TYPE '\
Press Enter or Cancel to return to the application.\n\
    Continue typing in entry field.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Indent 2nd sentence 4 columns.

MSG_EDIT_NO_CHANGE '\
Press Enter or Cancel to return to the application.\n\
    Field cannot be changed.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Indent 2nd sentence 4 columns.

MSG_014 '\
Press Enter to save the screen image  \n\
    in the log file.\n\
Press Cancel to return to the application.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1820_011 '\
1820-011 Internal error: asl_init_screen() called\n\
with invalid screen_code: %d.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_init_screen(), %d

ERR_1820_012 '\
1820-012 Internal error: invalid object field\n\
scr_info[%1$d].op_type = "%2$c";\n\
scr_info[%3$d].text:\n\
"%4$s".\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate scr_info[%1$d].op_type = "%2$c"
$ do not translate scr_info[%3$d].text
$ do not translate "%4$s"

ERR_1820_013 '\
1820-013 Internal error: invalid object field\n\
scr_info[%1$d].entry_type = "%2$c";\n\
scr_info[%3$d].text:\n\
"%4$s".\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate scr_info[%1$d].entry_type = "%2$c"
$ do not translate scr_info[%3$d].text
$ do not translate "%4$s"

ERR_1820_021 '\
1820-021 Internal error: asl_clear_buffer_screen(): must call asl_init()\n\
\tbefore calling asl_clear_buffer_screen().  Use local problem reporting\n\
\tprocedures.\n'
$ do not translate asl_clear_buffer_screen(), asl_init(), 
$ do not translate asl_clear_buffer_screen()

ERR_1820_017 '\
1820-017 Internal error: asl_vnote(): must call\n\
asl_init() before asl_vnote(). Use local problem\n\
reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_vnote()
$ do not translate asl_init(), asl_vnote()

ERR_1820_016 '\
1820-016 Internal error: asl_msg(): formatted\n\
input overflowed internal buffer; format\n\
parameter (field size bounds) probably\n\
incorrectly specified by calling application.\n\
Will attempt to display part of the original\n\
message next. Use local problem reporting\n\
procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_msg()

ERR_1820_034 '\
1820-034 Cannot open catalog message file;\n\
using English language messages.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_016 '\
INFORMATION MESSAGE'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_018 '\
ERROR MESSAGE'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_019 '\
Press Enter or Cancel to return to the\n\
application.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_020 '\
CANCEL'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_021 '\
WARNING: Cancellation will cause a loss of all\n\
the values you have changed.\n\
    Press Enter to continue cancellation.\n\
    Press Cancel to return to the application.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ Indent the last two sentences 4 columns.

MSG_022 '\
EXIT'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_023 '\
Press Enter to exit the application.\n\
Press Cancel to return to the application.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_024 '\
SHELL ESCAPE'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_025 '\
Press Enter to proceed to your shell.\n\
    Once in the shell, normal exit will return\n\
    you to this application.\n\
Press Cancel to return to the application.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ Indent 2nd sentence 4 columns.

MSG_026 '\
PRINT SCREEN'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_028 '\
ARE YOU SURE?'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_029 '\
Continuing may delete information you may want\n\
to keep.  This is your last chance to stop\n\
before continuing.\n\
    Press Enter to continue.\n\
    Press Cancel to return to the application.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ Indent last two sentences 4 columns.

MSG_030 '\
Help Information'

ERR_1820_018 '\
1820-018 Internal error: asl_msg() called with\n\
invalid screen code of %d. Use local problem\n\
reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_msg()
$ do not translate %d

MSG_031 '\
If this is an Information Message popup, this has appeared as a\n\
result of something you have done.  To continue, make note of the\n\
information presented and respond as appropriate.'

MSG_032 '\
If this is an Error Message popup, this popup has appeared as a\n\
result of a system error.  To continue, respond to the error as\n\
described.'

MSG_033 '\
This popup allows you to open a new shell while remaining in SMIT.\n\
When you press Enter from this popup, you will get this message:\n\
\n\
    Starting up new shell ...\n\
    your_prompt>\n\
\n\
At that point you may do any task normally done in a shell.  A normal\n\
shell exit (e.g. type the shell command, exit, and press Enter)\n\
will take you back to SMIT.'
$ Indent the middle lines 4 columns

MSG_034 '\
If this is an ARE YOU SURE? popup, this popup is intended as a\n\
warning to you that continuing to commit (or save) the dialogue\n\
will result in a loss of stored information that is related\n\
to this dialogue.  If you want to keep from losing information,\n\
press Cancel to cancel this popup, then press Cancel to cancel\n\
the dialogue.'

ERR_1820_020 '\
1820-020 System error: exec(): Cannot run:\n\
%s".\n\
Check path name and permissions or use local\n\
problem reporting procedures.  Press Enter\n\
to continue.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate exec()
$ do not translate %s

MSG_035 '\
asl_shell(): wait() return value = %1$d, (errno = %2$d)'
$ do not translate asl_shell(), wait(), %1$d, errno,  %2$d

MSG_037 '\
asl_shell(): wait() exit status = %x (hex)'
$ do not translate asl_shell(), wait(), %x

ERR_1820_019 '\
1820-019 System error: fork(): Cannot create\n\
another process at this time.  Try again later\n\
or use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate fork()

ERR_1820_032 '\
1820-032 Internal error: asl_execute(): \n\
%1$s.\n\
Could not start up following command: "%2$s".\n\
Press Enter to continue.\n'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_execute()
$ do not translate %1$s
$ do not translate "%2$s"

ERR_1820_031 '\
1820-031 Internal error: asl_execute(): fork().\n\
Could not create process for the following\n\
command:\n\
"%s"'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_execute(), fork()
$ do not translate "%s"

MSG_038 '\
This panel shows you the current state of the command you are running\n\
through SMIT.  While the command is running, you should not use the\n\
keyboard unless input is specificially requested by the command.  When\n\
the command status line indicates that the command has finished running,\n\
you may scroll through the output, if any.  Then press Cancel to return\n\
to the preceding panel.  The command is normally logged in the files\n\
specified by the paths $HOME/smit.log and $HOME/smit.script (unless you\n\
have specified the -l or -s parameters on the smit CommandString or unless\n\
you do not have write permission for these paths.)\n\
\n\
\n\
Home or Esc-<        Scroll to top of command output.\n\
\n\
End or Esc->         Scroll to bottom of command output.\n\
\n\
PageUp or Esc-V      Scroll to the previous page.\n\
\n\
PageDown or Ctrl-V   Scroll to the next page.\n'
$ do not translate $HOME/smit.log
$ do not translate $HOME/smit.script

MSG_039 '\
This is a list of values for the current entry field.  Select one item\n\
and it will become the current choice when you press Enter.  To select,\n\
cursor to the item, and press Enter.  For more information on the\n\
specific entry field, go back to the dialogue panel, and press F1 for\n\
Help.'

MSG_040 '\
This is a list of values for the current entry field.  You may select\n\
more than one value before you press Enter.  To select, move the\n\
cursor to the desired item, and press F7.  A > will appear beside\n\
selected item.  To undo a selection, press F7 a second time.\n\
Press Enter to commit all the selections.\n\
These values will then display in the appropriate entry field.'

MSG_041 '\
Summary of Edit Keys\n\
\n\
    Documentation Conventions:\n\
\n\
	*  Esc-V means press (and release) the Esc key, then press V;\n\
	   case is not significant.\n\
\n\
	*  Ctrl-V means hold down the Ctrl key and press V;\n\
	   case is not significant.\n\
\n\
Function			Inboard Key	Outboard Key\n\
--------			-----------	------------\n\
\n\
Next character			Ctrl-F		Cursor right\n\
Previouse character		Ctrl-B		Cursor left\n\
\n\
Next line			Ctrl-N		Cursor down\n\
Previous line			Ctrl-P		Cursor up\n\
\n\
Next screen (page)		Ctrl-V		Page Down\n\
Previous screen (page)  	Esc-V, Esc-^	Page up\n\
\n\
Beginning of line		Ctrl-A\n\
End of line			Ctrl-E\n\
\n\
Beginning of file		Esc-<		Home\n\
End of file			Esc->		End\n\
\n\
Delete to end of line		Ctrl-K		\n\
Delete character at cursor			Delete\n\
Delete previous character	Ctrl-H		Backspace\n\
\n\
Exit current menu (cancel)  	Ctrl-G		F3  or Esc-3 = Cancel \n\
Exit SMIT altogether		Esc-Ctrl-G	F10 or Esc-0 = Exit\n\
\n\
Redraw screen			Ctrl-L		F2  or Esc-2 = Redraw\n\
\n\
Tab				Ctrl-I		Tab\n\
Enter				Ctrl-J, Ctrl-M	Enter'

MSG_042 '\
This popup shows you the current state of the command you are building\n\
through SMIT.  When you are finished building the command, it can\n\
be used from the command line to invoke the same function that you\n\
are currently doing through SMIT.'

MSG_043 '\
When you press Enter from this popup the current screen image will\n\
be stored in the SMIT log file (which may be specified using the\n\
-l option on the smit CommandString).  After you exit SMIT,\n\
you must print the file as you would any other file, e.g.,\n\
/bin/print lp0 $HOME/smit.log.'
$ do not translate /bin/print lp0 $HOME/smit.log.

MSG_044 '\
If you exit from this dialogue now, your entry field values will not\n\
be saved.  Only the default values will be displayed the next time you\n\
access this dialogue.\n\
\n\
To run the command with your current values, press Cancel to cancel this\n\
popup, then Commit the dialogue.'

MSG_045 '\
If you cancel this dialogue before commiting the values, it will be\n\
necessary to re-enter all the values in the dialogue.  No values\n\
will be saved except the default values.'

ERR_1820_024 '\
1820-024 Internal error: asl_help(): called with\n\
invalid screen code of %d.  Use local problem\n\
reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_help()
$ do not translate %d

MSG_046 '\
    Doing list select on following ring:\n>"%s"'
$ ring is a multiple choice item
$ do not translate >"%s"

ERR_1820_025 '\
1820-025 Internal error: asl_ring_list(): no\n\
data values were supplied for generating a popup\n\
list. Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_ring_list()

MSG_047 '\
\tNext entry returned from command_to_list follows:\n"%s"'
$ do not translate "%s"

ERR_1820_028 '\
1820-028 System error: asl_reduce_sbrks():\n\
malloc(): There is not enough memory available\n\
now. Try again later or use local problem\n\
reporting procedures.\n\
Program will exit immediately.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_reduce_sbrks()
$ do not translate malloc()

MSG_048 '\
+ asl_reduce_sbrks pointer = 0x%x'
$ do not translate asl_reduce_sbrks, 0x%x

ERR_1820_029 '\
1820-029 Internal error: internally constructed\n\
string would be too long for target field if\n\
copied; result will be truncated:\n\
"%1$.100s"....\n\
Maximum size of %2$d characters exceeded.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%1$.100s"
$ do not translate %2$d

ERR_1820_030 '\
1820-030 Internal error: internally constructed\n\
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


MSG_049 '\
+ asl_getch(): character = %1$x (hex), %2$o (octal)'
$ do not translate asl_getch(), %1$x, %2$o 

ERR_1820_035 '\
1820-035 A path or file name must be\n\
specified in the entry field for:\n\
"%s".'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

ERR_1820_023 '\
1820-023 The following file could not be\n\
accesed:\n\
"%1$s".\n\
It is specified in the entry field for:\n\
"%2$s".\n\
Check the file (path) name and make sure the\n\
file exists, and that you have access permission\n\
for it.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%1$s"
$ do not translate "%2$s"

ERR_1820_033 '\
1820-033 A value must be specified in the\n\
entry field for:\n\
"%s".'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

ERR_1820_022 '\
1820-022 Internal error: asl_wprintw(): output\n\
overflowed internal buffer of %d characters.\n\
Use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_wprintw()
$ do not translate %d

ERR_1820_026 '\
1820-026 System error: asl_realloc(): realloc():\n\
There is not enough memory available now.\n\
Try again later or use local problem reporting\n\
procedures.\n\
Program will exit immediately.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_realloc(), realloc()

ERR_1820_027 '\
1820-027 System error: asl_malloc(): malloc():\n\
There is not enough memory available now.\n\
Try again later or use local problem reporting\n\
procedures.\n\
Program will exit immediately.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_malloc(), malloc()

ERR_1820_036 '\
1820-036 Internal error: invalid list panel\n\
input format containing Esc sequence.  Resulting\n\
list may be incorrect.  Use local problem\n\
reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1820_037 '\
1820-037 An internal error or system error has\n\
occurred.  See the log file for further \n\
information.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_050 '\
Press Enter to run command.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_GEN_HELP '\
F1=GenHelp'
$ 19 column msg. -|19--------------------------50|

MSG_GEN_HELP_ESC '\
Esc+1=GenHelp'
$ 19 column msg. -|19--------------------------50|

MSG_ENTRY_FIELDS '\
[Entry Fields]'
$ 22 column message--|22-----------------------50|

MSG_TOP '\
TOP'

MSG_MORE '\
MORE'

MSG_BOTTOM '\
BOTTOM'

MSG_ERROR '\
ERROR'

MSG_CANCEL '\
F3=Cancel' 
$ 19 column msg. -|19--------------------------50|

MSG_HELP '\
F1=Help' 
$ 19 column msg. -|19--------------------------50|

MSG_HELP_ESC '\
Esc+1=Help' 
$ 19 column msg. -|19--------------------------50|

MSG_REFRESH '\
F2=Refresh' 
$ 19 column msg. -|19--------------------------50|

MSG_REFRESH_ESC '\
Esc+2=Refresh' 
$ 19 column msg. -|19--------------------------50|

MSG_EXIT '\
F10=Exit' 
$ 19 column msg. -|19--------------------------50|

MSG_EXIT_ESC '\
Esc+0=Exit' 
$ 19 column msg. -|19--------------------------50|

MSG_LIST '\
F4=List' 
$ 19 column msg. -|19--------------------------50|

MSG_LIST_ESC '\
Esc+4=List' 
$ 19 column msg. -|19--------------------------50|

MSG_UNDO '\
F5=Reset' 
$ 19 column msg. -|19--------------------------50|

MSG_UNDO_ESC '\
Esc+5=Reset' 
$ 19 column msg. -|19--------------------------50|

MSG_COMMAND '\
F6=Command' 
$ 19 column msg. -|19--------------------------50|

MSG_COMMAND_ESC '\
Esc+6=Command' 
$ 19 column msg. -|19--------------------------50|

MSG_COMMIT '\
F7=Select' 
$ 19 column msg. -|19--------------------------50|

MSG_COMMIT_ESC '\
Esc+7=Select' 
$ 19 column msg. -|19--------------------------50|

MSG_IMAGE '\
F8=Image' 
$ 19 column msg. -|19--------------------------50|

MSG_IMAGE_ESC '\
Esc+8=Image' 
$ 19 column msg. -|19--------------------------50|

MSG_SHELL '\
F9=Shell' 
$ 19 column msg. -|19--------------------------50|

MSG_SHELL_ESC '\
Esc+9=Shell' 
$ 19 column msg. -|19--------------------------50|

MSG_EDIT '\
F7=Edit' 
$ 19 column msg. -|19--------------------------50|

MSG_EDIT_ESC '\
Esc+7=Edit' 
$ 19 column msg. -|19--------------------------50|

MSG_BACK '\
F10=Back' 
$ 16 col. msg.-|16-----------------------------50|
$ "Back" means to "go back" or "backup" a given number of screens.

MSG_BACK_ESC '\
Esc+0=Back' 
$ 16 col. msg.-|16-----------------------------50|
$ "Back" means to "go back" or "backup" a given number of screens.

MSG_ENTER '\
Enter' 
$ 19 column msg. -|19--------------------------50|

MSG_ENTER_DO '\
Enter=Do' 
$ 19 column msg. -|19--------------------------50|

MSG_REAL_COMMIT '\
F7=Commit' 
$ 19 column msg. -|19--------------------------50|

MSG_HELP_TEXT_MISSING '\
**** Help Text Missing ****'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_ENTER_TO_RETURN '\
Press Enter or Cancel to return to application.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_NEW_SHELL '\
Starting up new command shell ...\n'

MSG_USE_PF4 '\
Use PF4 or Esc+4 to generate a list.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_NON_SELECT '\
This item is for information only.\n\
Please select another item.'
$ 50 column message.---------------------------50|-----------------70|------79|

ERR_1820_038 '\
1820-038 Internal error: error in call to\n\
libcur/libcurses at line %d.'
$ 50 column message.---------------------------50|-----------------70|------79|

MSG_CANCEL_ESC '\
Esc+3=Cancel' 
$ 19 column msg. -|19--------------------------50|

MSG_EDIT_TAB_LIST '\
    Use tab key to sequence through valid selections or return\n\
    to application and use PF4 or Esc+4 to generate a list.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Indent all lines 4 columns

MSG_EDIT_LIST_ONLY '\
    Return to application and use PF4 or Esc+4 to generate a list.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Indent all lines 4 columns

MSG_EDIT_NO_TYPE '\
Press Enter or Cancel to return to the application.\n\
    Field cannot be edited.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Indent 2nd sentence 4 columns.

MSG_051 '\
Press Enter or Cancel to return to the application.'
$ 70 column message.---------------------------50|-----------------70|------79|

MSG_010_ESC '\
Move cursor to desired item and press Esc+7.\n\
    ONE OR MORE items can be selected.\n\
Press Enter AFTER making all selections.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Indent second sentence 4 columns.

MSG_013 '\
Move cursor to desired item and press F7. Use arrow keys to scroll.\n\
    ONE OR MORE items can be selected.\n\
Press Enter AFTER making all selections.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Can combine first line into one sentence if needed.
$ Indent second sentence 4 columns.

MSG_013_ESC '\
Move cursor to desired item and press Esc+7. Use arrow keys to scroll.\n\
    ONE OR MORE items can be selected.\n\
Press Enter AFTER making all selections.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Can combine first line into one sentence if needed.
$ Indent second sentence 4 columns.

MSG_015 '\
Move cursor to desired item and press Enter. Use arrow keys to scroll.'
$ 70 column message.---------------------------50|-----------------70|------79|
$ Can combine into one sentence if needed.


MSG_MACHINE '\
Esc+m=By Machine'

MSG_FIELD '\
Esc+f=By Field'

MSG_COLLECTIVE '\
Esc+c=Collective'

MSG_052 '\
There is more than one group of machines in your Collective. \n\
Each group requires group specific data. \n\
Move cursor to desired item and press Enter.'

MSG_054 '\
Sequential Execution Interrupt.'

MSG_055 '\
Execution has been terminated on the current machine.\
  Press Enter to terminate execution on all machines.\
  Press Cancel to return to the application.'

MSG_056 '\
The following machines failed during set-up and have been \
temporarily removed from the Collective.'

MSG_057 '\
Values may apply to one, some, or all machines in the Collective.\n\
Type or select a machine name from the Collective for the entry field.\n\
Press Enter AFTER making all desired selections.'

MSG_059 '\
Groups in Collective'

MSG_060 '\
Entries by Machine'

MSG_061 '\
The List is for Display Only.'

MSG_062 '\
Enter Search Pattern.'

MSG_FIND '\
/=Find'
$ 19 column msg. -|19--------------------------50|

MSG_FIND_NEXT '\
n=Find Next'
$ 19 column msg. -|19--------------------------50|


MSG_PATTERN_NOT_FOUND '\
Pattern not found.'

MSG_FIND_STRING '\
Find'

MSG_FIND_NEXT_STRING '\
Find Next'

MSG_063 '\
When you press Enter from this popup the current screen image will\n\
be stored in the SMIT log file (which may be specified using the\n\
-l option on the smit CommandString).  After you exit SMIT,\n\
you must print the file as you would any other file, e.g.,\n\
/bin/lpr $HOME/smit.log.'
$ do not translate /bin/lpr $HOME/smit.log.
$ MSG_063 replaces MSG_043

MSG_AT_LEAST_ONE '\
You must enter a value in at least one input\n\
field if you select "yes" in the first entry.'

MSG_DEFAULT '\
You must either enter same value in the input\n\
field or leave it blank if you select "yes"\n\
in the first entry.'

MSG_WRONG_CODESET '\
Some entries in the next screen do not have\n\
the correct string length. Check your language\n\
environment variable and the code set.\n\
Cannot continue.'

MSG_SCHEDULER '\
Esc+s=Scheduler'

MSG_ASL_NO_SHELL 'The Shell function is not available for this\n\
session.'

ERR_1820_039 '\
\n1820-039 Internal error: asl_shell(): signal received.\n\
Error executing subshell: "%s".\n\
Cannot continue. SMIT Exiting.\n'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate asl_execute()
$ do not translate "%s"
