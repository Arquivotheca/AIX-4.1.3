$ @(#)70	1.17  src/bos/usr/bin/smit/odm_msg/smit.101.msg, cmdmsmit, bos411, 9428A410j 5/13/94 17:19:22
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

$set 101
$quote '
$ Motif messages

$ Title declarations
SMIT_TITLE_STRING 'System Management Interface Tool'
$ This is the title of the application.

COW_TITLE_STRING 'Command Output'
$ Command refers to an executed command

EDIT_TITLE_STRING 'Edit Field'
$ This is the title of a window which allows the user
$ to edit or change a field of the command

SHOW_COMMAND_TITLE_STRING 'Show Command'
$ This is the title of a window which displays the command
$ which will be run.

INFO_TITLE_STRING 'Informational Message'
$ Title indicating that the message is for information.

ERROR_TITLE_STRING 'Error Message'
$ Title indicating that the message is an error message.


HELP_CONTEXT_TITLE_STRING 'Help Context'
$ Title indicating that the window is displaying contextual help.

HELP_PROMPT_TITLE_STRING 'Help Message'
$ Title indicating that a message is a help message.

EDIT_PROMPT_TITLE_STRING 'Edit Message'
$ Title indicating that a message is an edit message.

RESET_PROMPT_TITLE_STRING 'Reset Message'
$ Title indicating that a message is a reset message.


SINGLE_LIST_TITLE_STRING 'Single Select List'
$ Title indicating that a list is a single select list.  This type of
$ list displays a list of selections to the user and they are allowed
$ to select one of the choices.

MULTIPLE_LIST_TITLE_STRING 'Multi-select List'
$ Title indicating that a list is a multiple selection list.  This type
$ of list displays a list of selections to the user and they are allowed
$ to select one or more of the choices.


$ Push button declarations
CANCEL_STRING 'Cancel'
$ This string is used for the Cancel buttons on a Motif application.  This
$ string should be the exact same string that Motif will default to for
$ a Cancel button. If you do not know what know what that string is, please
$ find out. This string should be as short as possible.

OK_STRING 'OK'
$ This string is used for the OK buttons on a Motif application. This
$ string should be the exact same string that Motif will default to for
$ an OK button.  If you do not know what that string is, please find out.
$ This string should be as short as possible.

DO_STRING 'Do'
$ This string is used for the button that will take the user to the next
$ task.  It should indicated that pressing this button will do the
$ next task.  This string should be as short as possible.

CLOSE_STRING 'Close'
$ This string goes on a push button and indicates that if the user
$ pushes the button, the pop-up will be closed.  This string should
$ be as short as possible.

LIST_STRING 'List'
$ This string goes on a push button and indicates that if the user presses
$ the button, a list of choices will be displayed. This string should be
$ as short as possible.

SELECT_ALL_STRING 'Select All'
$ This string goes on a push button and indicates that if the user presses
$ the button, all the items in a list will be selected. This string should be
$ as short as possible.

STOP_STRING 'Stop'
$ This string goes on a push button and indicates that if the user presses
$ the button, the command which is currently running will be stopped. This
$ string should be as short as possible.

DONE_STRING 'Done'
$ This string goes on a push button and indicates that if the user presses
$ the button, they are indicating that they are done with the window. This
$ string should be as short as possible.

KEEP_STRING 'Keep'
$ This string goes on a push button and indicates that if the user
$ presses the button, the pop-up will stay on the screen. The user
$ will keep the pop-up on the screen.


$ Other text declarations
SHOW_COMMAND_LABEL_STRING 'Command:'
$ This string is the title of a window which displays a command being run.

RETURN_TO_TITLE_STRING 'Return To:'
$ This string is the title of a window containing push buttons.  If the
$ user presses one of those buttons, they will return to the place in
$ the application indicated by the label on the button.

SINGLE_LIST_LABEL_STRING 'Select one item from the list.'
$ This string tells the user that only one item may be selected from a
$ list of choices.

MULTIPLE_LIST_LABEL_STRING 'Select one or more items from the list.'
$ This string tells the user that one or more items may be selected from
$ a list of choices.

COW_OUTPUT_STRING 'Output:'
$ This string is the title of a window which displays the text ouput from
$ a command.

COW_SUCCESS_STRING 'Ok'
$ This is a string indicating that a command was successfully executed. This
$ should be a short string indicating success.

COW_FAILURE_STRING 'Failed'
$ This is a string indicating that a command was not successfully executed.
$ This should be a short string indicating failure.

FAST_PATH_LABEL_STRING '\n\nCurrent FastPath: %s\n\n'
$ This is a string used to display the current FastPath into the application.

HELP_PROMPT_STRING 'Select any highlighted label to see help on that item.'
$ This prompt tells the user to select any of the highlighted labels and help
$ will be displayed for that item.

EDIT_PROMPT_STRING 'Select any highlighted label to see or edit that item.'
$ This prompt tells the user to select any of the highlighted labels
$ and an edit window will be displayed for that item.

RESET_PROMPT_STRING 'Select any highlighted label to reset to original value.'
$ This promt tells the user to select any of the highlighted labels to
$ restore that item to its original value.



$ Menu bar declarations
EXIT_STRING 'Exit'
$ This string is a menu header for the exit menu.
$
$ ATTENTION: You are currently translating the text of a menu bar. The
$    menu bar header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this menubar. Make the header string as short as possible  This menu bar
$    contains the headers 'Exit', 'Edit', 'Show' and 'Help' with the
$    mnemonics             'x',   'E',    'S',       'H' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    menubar. Make the header string as short as possible

EXIT_MNEMONIC 'x'
$ This is the mnemonic for the 'Exit' header.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'x' is correct for 'Exit'. A 'x' will not work.
$
$ ATTENTION: You are currently translating the text of a menu bar. The
$    menu bar header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this menubar.  This menu bar
$    contains the headers 'Exit', 'Edit', 'Show' and 'Help' with the
$    mnemonics             'x',   'E',    'S',       'H' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    menubar. Make the header string as short as possible

EDIT_STRING 'Edit'
$ This string is the header to a menu which allows the user to edit fields.
$
$ ATTENTION: You are currently translating the text of a menu bar. The
$    menu bar header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this menubar.  This menu bar
$    contains the headers 'Exit', 'Edit', 'Show' and 'Help' with the
$    mnemonics             'x',   'E',    'S',       'H' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    menubar. Make the header string as short as possible

EDIT_MNEMONIC 'E'
$ This is the mnemonic for the 'Edit' header.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'E' is correct for 'Edit'. A 'e' will not work.
$
$ ATTENTION: You are currently translating the text of a menu bar. The
$    menu bar header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this menubar.  This menu bar
$    contains the headers 'Exit', 'Edit', 'Show' and 'Help' with the
$    mnemonics             'x',   'E',    'S',       'H' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    menubar. Make the header string as short as possible

SHOW_STRING 'Show'
$ This is the header to a menu which will show various items.
$
$ ATTENTION: You are currently translating the text of a menu bar. The
$    menu bar header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this menubar.  This menu bar
$    contains the headers 'Exit', 'Edit', 'Show' and 'Help' with the
$    mnemonics             'x',   'E',    'S',       'H' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    menubar. Make the header string as short as possible

SHOW_MNEMONIC 'S'
$ This is the mnemonic for the 'Show' header.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'S' is correct for 'Show'. A 's' will not work.
$
$ ATTENTION: You are currently translating the text of a menu bar. The
$    menu bar header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this menubar.  This menu bar
$    contains the headers 'Exit', 'Edit', 'Show' and 'Help' with the
$    mnemonics             'x',   'E',    'S',       'H' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    menubar. Make the header string as short as possible

HELP_STRING 'Help'
$ This is the header to a menu for the help.
$
$ ATTENTION: You are currently translating the text of a menu bar. The
$    menu bar header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this menubar.  This menu bar
$    contains the headers 'Exit', 'Edit', 'Show' and 'Help' with the
$    mnemonics             'x',   'E',    'S',       'H' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    menubar. Make the header string as short as possible

HELP_MNEMONIC 'H'
$ This is the mnemonic for the 'Help' header.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'H' is correct for 'Help'. A 'h' will not work.
$
$ ATTENTION: You are currently translating the text of a menu bar. The
$    menu bar header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this menubar.  This menu bar
$    contains the headers 'Exit', 'Edit', 'Show' and 'Help' with the
$    mnemonics             'x',   'E',    'S',       'H' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    menubar. Make the header string as short as possible

$ Exit Pulldown declaration
EXIT_SMIT_STRING 'Exit SMIT'
$ This string should indicate that if the user selects this choice, the
$ smit application will exit. Do not translate SMIT.

EXIT_SMIT_MNEMONIC 'E'
$ This is the mnemonic for 'Exit SMIT'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'E' is correct for 'Exit SMIT'. A 'e' will not work.


$ Edit Pulldown declaration

EDIT_FIELD_STRING 'Edit Field'
$ This string should indicate that this menu choice allows the user to
$ edit a field.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Edit Field', 'Reset Field', 'Reset All' with the
$    mnemonics            'E',                'F',    and    'A' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible

EDIT_FIELD_MNEMONIC 'E'
$ This is the mnemonic for 'Edit Field'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'E' is correct for 'Edit Field'. A 'e' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Edit Field', 'Reset Field', 'Reset All' with the
$    mnemonics            'E',                'F',    and    'A' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible

RESET_FIELD_STRING 'Reset Field'
$ This string should indicate that this menu choice allows the user to
$ reset a field to its original value.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Edit Field', 'Reset Field', 'Reset All' with the
$    mnemonics            'E',                'F',    and    'A' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible

RESET_FIELD_MNEMONIC 'F'
$ This is the mnemonic for 'Reset Field'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'F' is correct for 'Reset Field'. A 'f' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Edit Field', 'Reset Field', 'Reset All' with the
$    mnemonics            'E',                'F',    and    'A' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

RESET_ALL_STRING 'Reset All'
$ This string should indicate that this menu choice will reset all of the
$ changed values back to their original value.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Edit Field', 'Reset Field', 'Reset All' with the
$    mnemonics            'E',                'F',    and    'A' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

RESET_ALL_MNEMONIC 'A'
$ This is the mnemonic for 'Reset All'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'A' is correct for 'Reset All'. A 'a' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Edit Field', 'Reset Field', 'Reset All' with the
$    mnemonics            'E',                'F',    and    'A' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.


$ Show Pulldown declaration
COMMAND_STRING 'Command'
$ NEW PULLDOWN MENU!
$ This menu choice should indicate that this choice will display the
$ executable command.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Command', 'FastPath', 'Find', 'Find Next', with
$    the mnemonics        'C',           'P',    'F',  and    'N' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

COMMAND_MNEMONIC 'C'
$ This is the mnemonic for 'Command'. This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'C' is correct for 'Command'. A 'c' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Command', 'FastPath', 'Find', 'Find Next', with
$    the mnemonics        'C',           'P',    'F',  and    'N' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

FAST_PATH_STRING 'FastPath'
$ This menu choice should indicate that this choice will display the
$ FastPath to the current location.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Command', 'FastPath', 'Find', 'Find Next', with
$    the mnemonics        'C',           'P',    'F',  and    'N' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

FAST_PATH_MNEMONIC 'P'
$ This is the mnemonic for 'FastPath'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'P' is correct for 'FastPath'. A 'p' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Command', 'FastPath', 'Find', 'Find Next', with
$    the mnemonics        'C',           'P',    'F',  and    'N' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

FIND_STRING 'Find'
$ This menu choice should indicate that it allows the user to find a
$ pattern in a list of text.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Command', 'FastPath', 'Find', 'Find Next', with
$    the mnemonics        'C',           'P',    'F',  and    'N' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

FIND_MNEMONIC 'F'
$ This is the mnemonic for 'Find'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'F' is correct for 'Find'. A 'f' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Command', 'FastPath', 'Find', 'Find Next', with
$    the mnemonics        'C',           'P',    'F',  and    'N' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

FIND_NEXT_STRING 'Find Next'
$ This menu choice should indicate that it will find the next pattern
$ in the output text.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Command', 'FastPath', 'Find', 'Find Next', with
$    the mnemonics        'C',           'P',    'F',  and    'N' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

FIND_NEXT_MNEMONIC 'N'
$ This is the mnemonic for 'Find Next'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'N' is correct for 'Find Next'. A 'N' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$    menu header strings and their mnemonics (one character values which
$    refer to the header) must be unique within this pulldown menu.  This menu
$    contains the headers 'Command', 'FastPath', 'Find', 'Find Next', with
$    the mnemonics        'C',           'P',    'F',  and    'N' respectively.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.


$ Help Pulldown declaration
HELP_CONTEXT_STRING 'On Context'
$ NEW PULLDOWN MENU! The translation for this pulldown may be found
$ in the Motif style guide. Refer to the 'Help' menu for your language.
$
$ This menu should indicate that it will provide contextual help.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

HELP_CONTEXT_MNEMONIC 'C'
$ This is the mnemonic for 'On Context'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'C' is correct for 'On Context'. A 'c' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

HELP_WINDOW_STRING 'On Window'
$ This string should indicate that it provides help about the window.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.


HELP_WINDOW_MNEMONIC 'W'
$ This is the mnemonic for 'On Window'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'W' is correct for 'On Window'. A 'w' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

HELP_KEYS_STRING 'On Keys'
$ This menu choice should indicate that it will provide help on the
$ keyboard keys which can be used in SMIT.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

HELP_KEYS_MNEMONIC 'K'
$ This is the mnemonic for 'On Keys'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'K' is correct for 'On Keys'. A 'k' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.


HELP_INDEX_STRING 'Index'
$ This menu choice should indicate that it will provide an index of
$ help topics.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

HELP_INDEX_MNEMONIC 'I'
$ This is the mnemonic for 'Index'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'I' is correct for 'Index'. A 'i' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.


HELP_HELP_STRING 'On Help'
$ This menu choice should indicate that it provide help on using help.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.

HELP_HELP_MNEMONIC 'H'
$ This is the mnemonic for 'On Help'.  This value is case sensitive
$ and must be the same case as the letter in the header text.  That is
$ a 'H' is correct for 'On Help'. A 'h' will not work.
$
$ ATTENTION: You are currently translating the text of a pulldown menu. The
$   menu header strings and their mnemonics (one character values which
$   refer to the header) must be unique within this pulldown menu.  This menu
$   contains the headers
$             'On Context, 'On Window', 'On Keys', 'Index', 'On Help' with the
$    mnemonics   'C',         'W',         'K',    'I', and    'H'.
$    You must make sure that the headers are unique within themselves and
$    that the mnemonics are also unique within themselves.
$    You only need to ensure the uniqueness of these items within this
$    pulldown. Make the header string as short as possible.


$ Other Messages
MSMIT_STARTING 'Starting Motif SMIT'
$ Do not translate Motif or smit.

MSMIT_STOPPING 'Stopping Motif SMIT'
$ Do not translate Motif or smit.

MSMIT_RANGE '\
The value of this entry field must be in the range\n\
shown below\n\n\
%s'
$ Do not translate %s.


$ Help Messages

MOTIF_GEN_HELP_0 '\
Overview\n\
Understanding the Menu Window\n\
Understanding the Dialog Window\n\
Understanding the Path Window\n\
Understanding the Command Output Panel\n\
Making Selections from a List of Choices\n\
Using SMIT Functions \n\
Using SMIT FastPath\n\
Related information in InfoExplorer'
$ Do not translate SMIT or InfoExplorer

MOTIF_GEN_HELP_1 '\
  SMIT (System Management Interface Tool) is an interactive interface\n\
  application designed to simplify system management tasks.  It displays a\n\
  hierarchy of menus that lead to interactive dialogs.'
$ Do not translate SMIT

MOTIF_GEN_HELP_2 '\
  The menu window is the lower window of the primary SMIT screen. \n\
  The menu title appears at the top of the window.  The allowable \n\
  functions will be displayed in the menu bar. A list of menu items\n\
  appears in the menu screen.\n\
\n\
  Select any of the items in the menu screen. Both the text and the \n\
  button to the left of the text are selectable.  One of the following \n\
  will then occur:\n\
\n\
    * another menu appears\n\
    * command output appears (i.e. for a list request)\n\
    * a pop-up select screen appears\n\
    * a dialog screen appears'
$ Do not translate SMIT

MOTIF_GEN_HELP_3 '\
  The dialog window is the lower window of the primary SMIT window. \n\
  The dialog title appears at the top of the window.  The allowable \n\
  functions will be displayed in the menu bar or on the right side of \n\
  the window.  Descriptions of the various fields appear on the \n\
  left side of the window.  Entry fields appear in the center of the \n\
  dialog screen.  Select the Do button to perform the task.       \n\
\n\
  In a dialog, you are asked to supply details of the task you previously\n\
  selected.  Symbols are used to indicate various types of fields.  The\n\
  symbols may be displayed in various places in the dialog.  These symbols\n\
  are not displayed in a menu.\n\
\n\
  Symbol           Meaning\n\
\n\
  *               Indicates this field requires a value.\n\
\n\
    The following symbols appear on the right side of the screen:\n\
\n\
  List            Indicates a list of choices is available.\n\
                  Select this button to display the list. \n\
\n\
  Up Arrow or     Indicates an option ring is available.  Select the\n\
  Down Arrow      down arrow to move forward through the list, the \n\
                  up arrow to move backward.\n\
\n\
  #               Indicates a numeric field.\n\
\n\
  X               Indicates a hexadecimal field.\n\
\n\
  /               Indicates an accessible file name.\n\
\n\
\n\
  Highlighted entry\n\
           Indicates a user modified entry.  Highlighted entries have a\n\
           white background.'
$ 'Do' should be the the same as the 'Do' previously translated.
$ Do not translate SMIT.

MOTIF_GEN_HELP_4 '\
  The path window is the top window of the primary SMIT window.  It shows \n\
  menus which have been traversed in order to get to the current menu or \n\
  dialog.  If an entry in the path window is selected, the menu associated \n\
  with that entry will be redisplayed.\n'
$ Do not translate SMIT.

MOTIF_GEN_HELP_5 '\
  A command associated with a dialog screen is invoked when Do is selected.\n\
  The normal result from issuing a command is to have a command output screen\n\
  displayed.  While the command is running, a person running will be\n\
  displayed in the upper right portion of the screen and the only active \n\
  button will be the Stop button. As output is generated by the command, \n\
  it will be displayed on the screen.  If the command completes successfully,\n\
  the word OK will be displayed in the upper right hand corner of the window.\n\
  If the command fails, the word Failed will be displayed.\n\
\n\
  Besides the normal command status panel used for output, there are\n\
  special cases where SMIT will run the command in a separate screen and\n\
  gives control of that screen to the application being invoked.'

$ 'Do' should be the the same as the 'Do' previously translated.
$ 'Ok' and 'Failed' should be translated to the same text you translated
$ for the command success and failure.

MOTIF_GEN_HELP_6 '\
  To display a list of choices, press the List button.\n\
\n\
  To select one item from a single select pop-up:\n\
\n\
     Select the desired item from the list. The list pop-up will go \n\
     away and the selected item will be entered into the field.\n\
\n\
  To select more than one item from a multiple select pop-up:\n\
\n\
     Select each desired item from the list. Select OK and the \n\
     list pop-up will go away and the selected items will be entered \n\
     into the field.\n\
\n\
     Select the Select All button and all the items in the list will \n\
     be marked as selected. Deselect the items which are not needed and \n\
     then select the OK button. The list pop-up will go away and the \n\
     selected items will be entered into the field.\n\
\n\
  An item may be selected from an option ring by either displaying the\n\
  list or by selecting the up and down arrow buttons.  Selecting the\n\
  down arrow button will select the next item in the list.  Selecting \n\
  the up arrow button will select the previous item in the list. '
$ 'OK' should be translated to be the same as the Motif OK button.

MOTIF_GEN_HELP_7_1 '\
  Functions       Description\n\
\n\
  Exit SMIT       (From the Exit menu.) Exits SMIT. All SMIT windows will\n\
                  be closed.\n\
\n\
  Edit Field      (From the Edit menu.) Active only on a dialog screen. \n\
                  Highlights the fields which can be edited and prompts \n\
                  to select one of them.  That field is then displayed \n\
                  in a wide pop-up.  This function can be used to display\n\
                  a long field which will not fit in the entry area of a\n\
                  dialog window.\n\
\n\
  Reset Field     (From the Edit menu.) Active only on a dialog screen.\n\
                  Highlights the fields which can be reset and prompts \n\
                  to select one of them.  That field is then set to its \n\
                  original value.\n\
\n\
  Reset All       (From the Edit menu.) Active only on a dialog screen.\n\
                  Resets all the fields to their original values.\n\
\n\
  Command         (From the Show menu.) Active only on a dialog screen.\n\
                  Displays the command that SMIT is building.\n\
\n\
  FastPath        (From the Show menu.) Displays the current FastPath.\n\
\n\
  Find            (From the Show menu.) Active only in the command output \n\
                  window. Prompts for a pattern to find in the output window.\n\
                  If that pattern is found, it is highlighted in the output \n\
                  window. Simple regular expression patterns are supported.\n\
\n\
  Find Next       (From the Show menu.) Active only in the command output \n\
                  window.  Finds the next string which matches the \n\
                  pattern entered on the previous Find.\n'
$ Do not translate SMIT.

MOTIF_GEN_HELP_7_2 '\
  On Context      (From the Help menu.) Highlights the fields which \n\
                  have contextual help and prompts to select one of them.\n\
                  The help for the selected item will appear in a pop-up \n\
                  window.  Contextual help can be retrieved for any item \n\
                  until the Keep or Cancel button in the help pop-up is \n\
                  selected. The help pop-up will remain on the screen if \n\
                  the Keep button in the help pop-up is selected.\n\
\n\
  On Window       (From the Help menu.) Displays help about the \n\
                  window from which this selection was made.\n\
\n\
  On Keys         (From the Help menu.) Displays the function keys and the \n\
                  keyboard accelerators.\n\
\n\
  Index           (From the Help menu.) Displays an index of help topics, \n\
		  and provides help for each of the topics.\n\
\n\
  On Help         (From the Help menu.) Displays help about the use of \n\
                  the help functions.\n\
\n\
  Help            (From a pop-up.) Displays help on how to use the pop-up.\n'

$ 'OK' and 'Cancel' should be translated to be the same as the Motif
$ OK and Cancel buttons.

MOTIF_GEN_HELP_7_3 '\
  Cancel          (From the primary window.) Backs up to the previous menu.\n\
                  If no previous menu exists, SMIT will exit.\n\
\n\
  Do              (From the primary window.) Active only on a dialog screen.\n\
                  Performs the current task. Another dialog or the command\n\
                  output panel may appear.\n\
\n\
  List button     Active only on a dialog screen for entries which have a \n\
                  list of choices available. Displays a selectable list of \n\
                  choices for a field.  Selections made from this list will \n\
                  be entered into the field.\n\
\n\
  Down arrow      Active only on a dialog screen for entries which have an \n\
  button          option ring.  Selects the next entry from the option ring.\n\
\n\
  Up arrow        Active only on a dialog screen for entries which have an \n\
  button          option ring.  Selects the previous entry from the option \n\
                  ring.\n\
\n\
  Select All     (From a multi-select list pop-up.) Selects all the entries\n\
                 in the list, but does not close the list pop-up. Individual\n\
                 list items can be deselected in the list.\n\
\n\
  Stop           (From the command output window.) Stops the execution of the\n\
                 command.\n\
\n\
  Done           (From the command output window.) Closes the command output\n\
                 window.'
$ 'OK' and 'Cancel' should be translated to be the same as the Motif
$ OK and Cancel buttons.
$ 'Do' should be the the same as the 'Do' previously translated.

MOTIF_GEN_HELP_8 '\
  The SMIT FastPath is a shortcut method to display a SMIT dialog panel\n\
  or menu directly from the command line.   Select FastPath from \n\
  the Show menu to display the FastPath for the current screen.\n\
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
  configured on your system.'

$ Do not translate SMIT. Do not translate any of the example FastPaths.

MOTIF_LIST_HELP_TITLE '\
How to use a pop-up list'

MOTIF_EDIT_HELP_STRING '\
An Edit Field pop-up will display a field in a larger window.  If the \n\
field is editable, the value can be changed by typing in the text area \n\
of the pop-up.  If an option ring is available, up and down arrows \n\
will be displayed to the right of the field.  Pressing the down arrow \n\
will select the next entry in the list. Pressing the up arrow will \n\
select the previous entry.\n\
\n\
Select the OK button to close the pop-up and the field in the dialog \n\
window will be changed.  Select the Cancel button to close the pop-up\n\
without changing the field.'
$ 'OK' and 'Cancel' should be translated to be the same as the Motif
$ OK and Cancel buttons.

MOTIF_FASTPATH_HELP_STRING '\
The SMIT FastPath is a shortcut method to display a SMIT dialog panel\n\
or menu directly from the command line.   Type:\n\
\n\
    smit %s\n\
\n\
to display the screen associated with the %s FastPath.'
$ Do not translate smit.

MOTIF_COMMAND_HELP_STRING '\
This is the command which will be run by SMIT when the Do button in the\n\
primary SMIT window is selected.'
$ 'Do' should be the the same as the 'Do' previously translated.



ERR_NEED_A_VALUE '\
A value must be specified in the\n\
entry field for:\n\
"%s".'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"


ERR_NO_POPUP_LIST '\
Popup list is not available for this\n\
entry field.'
$ 50 column message.---------------------------50|-----------------70|------79|


ERR_NO_OPEN_CAT '\
Cannot open catalog message file;\n\
using English language messages.'
$ 50 column message.---------------------------50|-----------------70|------79|




ERR_FORK_FAILED '\
System error: fork(): Cannot create\n\
another process at this time.  Try again later\n\
or use local problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate fork()


ERR_EXEC_FAILED '\
System error: exec(): Cannot run:\n\
%s".\n\
Check path name and permissions or use local\n\
problem reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate exec()
$ do not translate %s


ERR_RING_SIZE '\
Internal error: ring field size:\n\
index (%1$d),\n\
maximum size (%2$d),\n\
length (%3$d).\n\
Use local problem reporting proceedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate (%1$d), (%2$d), (%3$d)
$ a ring field is a multiple choice field



ERR_SPECIFY_PATH '\
A path or file name must be\n\
specified in the entry field for:\n\
"%s".'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"



MSG_CUR_FASTPATH '\
Current fast path:\n\
    "%s"'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"

MSG_SMIT_EXEC_EXIT '\
NOTE: This command has been selected to run\n\
      outside of SMIT.  Therefore, SMIT will\n\
      exit immediately before running this command.'


MSG_CONFIRMATION '\
ARE YOU SURE?\n\n\
Continuing may delete information you may want\n\
to keep.  This is your last chance to stop\n\
before continuing.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ Indent last two sentences 4 columns.

ERR_INTERNAL_ERROR '\
An internal error or system error has\n\
occurred.  See the log file for further \n\
information.'
$ 50 column message.---------------------------50|-----------------70|------79|


ERR_OPEN_ERROR '\
System error: open(): Cannot open the file:\n\
    "%s" \n\
Try again later or use local problem \n\
reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"


ERR_WRITE_ERROR '\
System error: write(): Cannot write to the file:\n\
    "%s" \n\
Try again later or use local problem \n\
reporting procedures.'
$ 50 column message.---------------------------50|-----------------70|------79|
$ do not translate "%s"


MSG_HELP_ON_KEYS_ACCEL '\
The following accelerators provide a shortcut to the functions\n\
shown in the main menu bar.\n\
\n\
 Accelerator                                 Action       \n\
-------------                           ---------------   \n\
   /                                    Find              \n\
   n                                    Find Next         \n\
   F6                                   Show Command      \n\
   F8                                   Show FastPath     \n\
   F9                                   Reset All Fields  \n\
   F12                                  Exit SMIT         \n\
   Ctrl+F1                              Help on Context   \n\
   Ctrl+F5                              Reset Field       \n\
   Ctrl+F7                              Edit Field        \n\
\n'
$ An Accelerator is a Motif property.  Check with a Motif guide
$ to determine the correct translation for the word 'Accelerator'.
$ Do not translate F6, F8, F9, F12.
$ Keep the translated text in the same column format.

MSG_HELP_ON_KEYS_TRANS '\
\n\
The following table describes the key translations defined for the \n\
text fields.\n\
\n\
  Translation                              Task Performed            \n\
 --------------                          -------------------         \n\
\n\
   F5                                    Resets the text field to the\n\
                                         default value.\n\
\n\
   F7                                    Provides an edit popup to edit\n\
                                         the text field.\n\
\n\
  Enter                                  Performs the current task.\n\
\n'
$ A Translation is a Motif property. Check with the Motif guide
$ to determine the correct translation for the word 'Translation'.
$ Do not translate F5, F7.
$ Keep the translated text in the same column format.


MSG_HELP_ON_LISTL '\
  Ctrl+l                                 Displays a list of choices\n\
                                         available for the text field'


MSG_HELP_ON_LISTF4 '\
  F4                                     Displays a list of choices\n\
                                         available for the text field'
$ Do not translate F4
$ Keep the translated text in the same column format

MSG_HELP_WINDOW_HELP '\
This window displays help text. A Keep button will appear in the dialog\n\
if you selected Help On Context.  Pressing this button will keep the help\n\
dialog on the screen and end the contextual help mode. When you are done\n\
viewing the text, select the Cancel button to close the help pop-up.'

MSG_SMIT_WINDOW_HELP '\
%s\n\n\
For more information about SMIT, select Index from the Help menu.'
$ Do not translate SMIT.

CLEAR_STRING 'Clear'

HELP_ON_HELP 'Help on Help'

HELP_ON_KEYS 'Help on Keys'

HELP_ON_INDEX 'Help Index'

HELP_ON_WINDOW 'Help on Window'

HELP_TITLE 'Help'
$ Do not put mnemonic

FAST_PATH_LABEL 'FastPath'
$ Do not put mnemonic

FIND_LABEL 'Find'
$ Do not put mnemonic

FIND_NEXT_LABEL 'Find Next'
$ Do not put mnemonic

