$ @(#)43	1.91  src/bos/usr/ccs/bin/ld/bind/bind.msg, cmdld, bos412, 9445C412a 11/10/94 16:36:18
$
$
$  COMPONENT_NAME: (CMDLD) bind - xcoff linkage editor
$
$  FUNCTIONS: Message catalog for the linkage editor (bind)
$
$  ORIGINS: 27
$
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$  All Rights Reserved
$  Licensed Materials - Property of IBM
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$  *****************************************************************
$ $
$ $  GENERAL COMMENTS:
$ $
$ $	TRANSLATION INFORMATION:
$ $
$ $	Most messages are not terminated with a \n (New line).  During
$ $	translation, a newline must not be added.  However, unless otherwise
$ $	noted, messages may be reformatted in an attempt to keep all lines
$ $	within a display width of 80 columns.  To reformat a message:
$ $	1) Where a line break is needed, insert a newline and a tab ("\n\t").
$ $	2) Do not add a final newline unless the English message has a
$ $	final newline.
$ $
$ $	If you have a translation for "link-editor" you can use it whereever
$ $	you see the word "binder".  Otherwise, do not translate "binder".
$ $
$ $	OTHER INFORMATION
$ $
$ $	Return codes and error levels are defined as follows:
$ $
$ $		RC_OK		0
$ $		RC_WARNING	4
$ $		RC_ERROR	8
$ $		RC_SEVERE	12
$ $		RC_PGMERR	16
$ $
$ $	The nature of RC_PGMERR (Internal Errors) is of more interest to the
$ $	developers of the binder than to the user of the binder.
$ $	Perhaps the first course of action should be to recompile the object
$ $	modules and rebind.  If the error still occurs, contact
$ $	your nearest rep.
$ $
$ $	A filter is used to generate an array of default messages from this
$ $	 file.
$ $	The following conventions are observed:
$ $	Blank lines and "$" comment lines are deleted.
$ $     The message identifiers are deleted
$ $	The messages are copied literally.
$ $

$quote "

$set 1

UNUSED1 	""
$		Translation Information: Do not translate.
$		Not used. Avoids conflict between old and new messages.

UNUSED2 	""
$		Translation Information: Do not translate.
$		Not used. Avoids conflict between old and new messages.

UNUSED3 	""
$		Translation Information: Do not translate.
$		Not used. Avoids conflict between old and new messages.

BIND_MSG_ERROR	"ld: 0711-100 INTERNAL MESSAGE ERROR.\n\
\tDepending on where this product was acquired, contact your service\n\
\trepresentative or the approved supplier."
$ $		Cause:	SAY_NO_NLS was probably not set when it should have
$ $			been.  This is an internal error.
$ $		Action:	This message is displayed instead of intended message.
$ $			If message is going to a file (such as a address map
$ $			file, this message goes to the files as well.
$ $		Technical Information:
$		This error is never generated explicitly.  It is used in case
$		of an internal error that causes an out-or-range message
$		number to be used.
$ $		Translation information:

ALLOC_FAILED	"%1$s: 0711-101 FATAL ERROR: Allocation of %2$u bytes failed\n\
\tin routine %3$s. There is not enough memory available.\n\
\tPlease check your ulimit or paging space\n\
\tor use local problem reporting procedures."
$ $		Cause:	No memory could be allocated by malloc().
$ $		Action:	Program exits.
$ $		Technical Information:
$ $			Displayed when malloc() returns NULL, if caller chooses
$ $			not to handle the failure.
$ $		Translation Information:
$ $			Do not translate: ulimit
$ $			%1$s 	"ld" or "bind"
$ $			%2$u	Number of bytes to be allocated.
$ $			%3$s	Name of routine allocating memory

ALLOC_FAILED2	"%1$s: 0711-104 FATAL ERROR: Reallocation of %2$u bytes \
failed\n\tin routine %3$s. There is not enough memory available.\n\
\tPlease check your ulimit or paging space\n\
\tor use local problem reporting procedures."
$ $		Cause:	No memory could be allocated by realloc().
$ $		Action:	Program exits.
$ $		Technical Information:
$ $			Displayed when realloc() returns NULL, if caller chooses
$ $			not to handle the failure.
$ $		Translation Information:
$ $			Do not translate: ulimit
$ $			%1$s 	"ld" or "bind"
$ $			%2$u	Number of bytes to be allocated.
$ $			%3$s	Name of routine allocating memory

ALLOC_ZERO	"%1$s: 0711-113 WARNING: Allocation of zero bytes requested \
in routine %2$s."
$ $		Cause:	Allocation of 0 bytes was requested.
$ $		Action:	A single byte is allocated.
$ $		Technical Information:
$ $			This message is only displayed when ALLOCZEROMSG is
$ $			defined.  The code should never allocate 0 bytes.
$ $		Translation Information:	DO NOT TRANSLATE.
$ $			%1$s	"ld" or "bind"
$ $			%2$s	name of routine allocating memory

CMD_ABORT	"%s: Binder stops."
$ $		Cause:
$ $			"ABORT" subcommand was issued.
$ $		Action:
$ $			Binder exits immediately
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	Subcommand name (ABORT)

MAIN_RETCODE	"%1$s: The return code is %2$d."
$ $		Cause:
$ $		Action:
$ $		Technical Information:
$ $			Shows the return code value of current subcommand run.
$ $			This is used for all subcommands that have a "non-zero"
$ $				return code value.
$ $		Translation Information:
$ $			%1$s	name of binder subcommand
$ $			%2$d	return code

MAIN_RETCODE2	"The return code is %d."
$ $		Cause:
$ $		Action:
$ $		Technical Information:
$ $			Shows the return code value for invalid subcommands.
$ $		Translation Information:
$ $			%d	return code

MAIN_INTERRUPT	"%1$s: 0711-635 WARNING: Binder command %2$s interrupted."
$ $		Cause:
$ $		Action:
$ $		Technical Information:	Interactive mode only.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of interrupted binder command

MAIN_INTERRUPT2	"%1$s: 0711-639 WARNING: Binder commands interrupted."
$ $		Cause:
$ $		Action:
$ $		Technical Information:	Interactive mode only.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"

MAIN_NULLS_KEYBOARD	"%s: 0711-636 WARNING: Null characters are not allowed in a binder command."
$ $		Translation Information:
$ $			%s	"ld" or "bind"

MAIN_NULLS_FILE		"Binder command contains null characters.\n\
%1$s: 0711-638 SEVERE ERROR Check input file %2$s\n\
\tThe remaining commands in the file are being ignored."
$ $		Translation Information:
$ $			The initial part of the message before %1$s is
$ $				intentional.  Preserve the form of this message.
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input file in error

MAIN_NULLS_STDIN	"Binder command contains null characters.\n\
%1$s: 0711-637 SEVERE ERROR Check standard input.\n\
\tThe remaining commands are being ignored."
$ $		Translation Information:
$ $			The initial part of the message before %1$s is
$ $				intentional.  Preserve the form of this message.
$ $			%1$s	"ld" or "bind"

NESTING_TOO_DEEP	"%1$s: 0711-634 SEVERE ERROR: EXEC binder commands nested too deeply.\n\
\tMaximum recursion level is %2$d. Returning to previous level."
$ $		Translation Information:
$ $			Do not translate EXEC
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Maximum recursion level allowed.

MAIN_CONTINUE	"Continue with remaining binder commands in file %s [NYA]? "
$ $		Translation Information:
$ $			%s	Subcommand file being processed
$			WARNING:  The translation of the next 2 messages may
$				be affected by the translation of this message.
$			[NYA]	Indicates that appropriate responses are
$				N (for NO), Y (for YES) or A (for ABORT).
$				This should be translated using the first
$				letters of the translated versions of
$				no and yes.  The 'A' should not be translated
$				unless the translated version of 'YES' or 'NO'
$				begins with 'A'.  In this case, the 'A' should
$				be translated to the first letter of the
$				translation of message LIT_ABORT.
$				(See the next 2 messages.)

MAIN_QUERY		"Please answer NO or YES or ABORT."
$ $		Cause:
$			User provided invalid answer
$ $		Action:
$ $		Technical Information:
$ $		Translation Information:
$			WARNING:  The translation of the previous and
$				following messages may be affected by the
$				translation of this message.
$			'YES' and 'NO' should be translated.  'ABORT' should
$			not be translated unless the translated version of 'YES'
$			or 'NO' begins with 'A'.  In this case, 'ABORT' should
$			be translated to the same value as message LIT_ABORT.
$			(See the previous and following messages.)

LIT_ABORT	"ABORT"
$ $		Translation Information:
$			WARNING:  The translation of the previous 2 messages may
$				be affected by the translation of this message.
$			This message should not be translated unless the
$				translated version of 'YES' or 'NO'
$				begins with 'A'.  In this case, an appropriate
$				word should be selected so as not to conflict
$				with the translated words for 'YES' or 'NO'.
$			The translation of the message must be entirely in
$				upper case.

MAIN_QUIT_ABORT2		"Type\n\
\t\tabort\n\
\tto terminate the binder or\n\
\t\tquit\n\
\tto exit recursive EXEC binder command execution."
$ $		Translation Information:
$ $			Do not translate: abort quit EXEC
$ $			You may convert the following words to
$ $				uppercase: abort quit

MAIN_QUIT_ABORT		"Type\n\
\t\tabort\n\
\tto terminate the binder or\n\
\t\tquit\n\
\tto continue executing binder commands from file %s"
$ $		Translation Information:
$ $			Do not translate: abort quit
$ $			%s	Subcommand file being processed
$ $			You may convert the following words to
$ $				uppercase: abort quit

MAIN_QUIT		"Type\n\
\t\tquit\n\
\tto terminate the binder."
$ $		Translation Information:
$ $			Do not translate quit
$ $			You may convert the following word to uppercase: quit

HALT_QUERY	"%1$s: The binder will terminate on return codes greater than %2$d."
$ $		Cause:
$ $			The "HALT" subcommand was issued.
$ $		Action:
$ $			Displays the error level (subcommand return code value)
$ $				above which, the binder will quit.
$ $		Technical Information:
$ $		(Ref 3.4.22)
$ $		Translation Information:
$ $			%1$s	Subcommand (HALT)
$ $			%2$d	Current halt setting

PAD_SET		"%1$s: The object file pad values are .text: %2$d  .data: %3$d  .loader: %4$d"
$ $		Cause:
$ $			Response to the "PAD" subcommand
$ $		Action:
$ $		Technical Information:
$ $			A "PAD" subcommand was issued to ensure that the
$ $				text, data, and loader sections begin on
$ $				specific boundaries.
$ $		Translation Information:
$ $			Do not translate: .text .data .loader
$ $			%1$s	Subcommand (PAD)
$ $			%2$d	.text boundary value
$ $			%3$d	.data boundary value
$ $			%4$d	.loader boundary value

MAXSTACK_SET	"%1$s: The stack size limit field in the auxiliary header of the\n\
\toutput object file will be set to: %2$d (0x%2$X) bytes"
$ $		Cause:
$ $			The "MAXSTACK" subcommand was issued.
$ $		Action:
$ $			Displays the stack size limit field value that will
$ $			be placed in the aouthdr of the object file being
$ $			generated.
$ $		Translation Information:
$ $			%1$s	Subcommand name (MAXSTACK)
$ $			%2$d	Maximum stack size

MAXSTACK_ZERO	"%s: The stack size limit field in the auxiliary header of the output\n\
\tobject file will be set to zero. The system will use the default limit."
$ $		Cause:
$ $			The "MAXSTACK" subcommand was issued.
$ $		Action:
$ $			Displays the stack size limit field value that will
$ $			be placed in the aouthdr of the object file being
$ $			generated.
$ $		Technical Information:
$ $			When the maxstack value is zero or not specified,
$ $			the system default limit is used. Currently set at
$ $			1 meg.
$ $		Translation Information:
$ $			%s	Subcommand name (MAXSTACK)

MAXDATA_SET	"%1$s: The maximum .data section size field in the auxiliary header of the\n\
\toutput object file will be set to: %2$d (0x%2$X) bytes"
$ $		Cause:
$ $			The "MAXDATA" subcommand was issued.
$ $		Action:
$ $			Displays the data size limit field value that will
$ $			be placed in the aouthdr of the object file being
$ $			generated.
$ $		Translation Information:
$ $			Do not translate: .data
$ $			%1$s	Subcommand name (MAXDATA)
$ $			%2$d	Maximum data size

MAXDATA_ZERO	"%s: The maximum .data section size field in the auxiliary header of the output\n\
\tobject file will be set to zero. The system will use the default limit."
$ $		Cause:
$ $			The "MAXDATA" subcommand was issued.
$ $		Action:
$ $			Displays the data size limit field value that will
$ $			be placed in the aouthdr of the object file being
$ $			generated.
$ $		Technical Information:
$ $			When the maxstack value is zero or not specified,
$ $			the system default limit is used.
$ $		Translation Information:
$ $			%s	Subcommand name (MAXDATA)

CMD_NOSUP	"%1$s: 0711-144 WARNING: Binder command %2$s is unsupported."
$ $		Technical Information:
$ $			A number of subcommands are in the command
$ $			dictionary that are not supported.
$ $			Work arounds are beyond the scope of this function
$ $			call.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Binder command name

CMD_IGNORE	"%1$s: 0711-149 Binder command %2$s is ignored."
$ $		Technical Information:
$ $			A number of subcommands are in the command
$ $			dictionary that are not supported.
$ $			Work arounds are beyond the scope of this function
$ $			call.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Binder subcommand name

CMD_LOADMAP_NOARG	"%1$s: 0711-150 SEVERE ERROR: Missing pathname for SETOPT loadmap:pathname"
$ $		Translation Information:
$ $			Do not translate: SETOPT loadmap
$ $			Note:  :pathname represents an arbitrary pathname a
$ $				user could supply when typing
$ $				"SETOPT loadmap:pathname".
$ $				"pathname" can be translated.
$ $			%1$s	"ld" or "bind"

CMD_BADARG	"%1$s: 0711-151 SEVERE ERROR: %2$s: Invalid option name: %3$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of subcommand (SETOPT)
$ $			%3$s	Bad option name

CMD_SETOPT_NOARG	"%1$s: 0711-278 SEVERE ERROR: %2$s: Argument missing for option %3$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of subcommand (SETOPT)
$ $			%3$s	Name of binder option

CMD_INVALID	"%1$s: 0711-152 SEVERE ERROR: Invalid binder command: %2$s"
$ $		Cause:
$ $			A subcommand was given that is not contained in the
$ $			subcommand dictionary.
$ $		Action:
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Invalid binder command specified.

SETOPT_BAD	"%1$s: 0711-153 WARNING: File %2$s\n\
\tOption %3$.*4$s may not be changed within an import or export file."
$ $		Translation Information:
$ $			%1$s		"ld" or "bind"
$ $			%2$s		File name containing option
$ $			%3$.*4$s	Name of binder option (and length)

CMD_NEG_ARG	"%1$s: 0711-154 ERROR: Binder command %2$s requires non-negative arguments."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of binder subcommand

CMD_TOOFEW	"%1$s: 0711-156 SEVERE ERROR: Binder command %2$s requires %3$d arguments."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of binder command
$ $			%3$d	Number of arguments required

CMD_TOOFEW2	"%1$s: 0711-157 SEVERE ERROR: Binder command %2$s requires %3$d or more arguments."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of binder command
$ $			%3$d	Number of arguments required

CMD_ARGCOUNT	"%1$s: 0711-158 SEVERE ERROR: Binder command %2$s requires %3$d to %4$d arguments."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of binder command
$ $			%3$d	Minimum number of arguments required
$ $			%4$d	Maximum number of arguments required

CMD_NOARG	"%1$s: 0711-159 SEVERE ERROR: Binder command %2$s: Missing argument."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of binder command

SETOPT_BADARG	"%1$s: 0711-155 SEVERE ERROR: %2$s option %3$s: Invalid argument: %4$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of subcommand (SETOPT)
$ $			%3$s	Name of option
$ $			%4$s	Invalid argument

SETOPT_IGNORE	"%1$s: 0711-275 WARNING: %2$s option %3$s is ignored."
$ $		Translation Information:
$ $			Do not translate: SETOPT
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of subcommand (SETOPT)
$ $			%3$s	Name of option

FILE_CANNOT_OPEN	"%1$s: 0711-160 SEVERE ERROR: Cannot open file: %2$s\n\
\t%1$s:%3$s() %4$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file being opened
$ $			%3$s	System call used to open file
$ $			%4$s	Message from strerror()

FILE_TRUNCATED	"%1$s: 0711-161 ERROR: File %2$s cannot be processed.\n\
\tThe file was found to be truncated and is being ignored."
$ $		Cause:	An archive or xcoff file is too short to contain its
$ $			header.
$ $		Action:	The file is ignored.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of truncated file

CMPCTS_REL	"%1$s: Kept %2$d of %3$d relocation entries."
$ $		Cause:
$ $			Displayed after COMPRLD subcommand.
$ $		Technical Information:
$ $			Shows amount of compression accomplished by cmprld().
$ $		Translation Information:
$ $			%1$s	Subcommand name (COMPRLD)
$ $			%2$d	Number of RLD entries kept
$ $			%3$d	Number of RLD entries examined.

CMPCTS_SYMS_GC	"%1$s: Number of symbols garbage collected: %2$d of %3$d"
$ $		Cause:
$ $			Displayed after RESOLVE subcommand.
$ $		Translation Information:
$ $			"garbage collection" is a technical term
$ $			that refers to the process of deleting
$ $			unneeded symbols.  If necessary, translate
$ $			this message as if it said:
$ $			"Number of symbols deleted by garbage collection:"
$ $			or simply "Number of symbols deleted:"
$ $			%1$s	Subcommand name (RESOLVE)
$ $			%2$d	number of symbols collected (or deleted).
$ $			%3$d	number of symbols examined.

CMPCT_RCW_DUP1	"%1$s: 0711-224 WARNING: Duplicate symbol: %2$s"
$ $		Translation Information:
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Symbol name

MISMATCH_SYM	"%1$s: 0711-197 ERROR: Type mismatches for symbol: %2$s"
$ $		Translation Information:
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Symbol name

UNRESOLVE_HEADING	"%1$s: 0711-317 ERROR: Undefined symbol: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name

DUPLICATE_MSG	"%1$s: 0711-228 WARNING: Duplicate symbols were found while resolving symbols.\n\
\tThe following duplicates were found:"
$ $		Translation Information:
$ $			This is a message that is printed at the top of
$ $			a list of duplicate symbols.
$ $			%1$s:	"ld" or "bind"

MISMATCH_HEAD	"%1$s: 0711-189 ERROR: Type mismatches were detected.\n\
\tThe following symbols are in error:"
$ $		Translation Information:
$ $			This is a message that is printed at the top of
$ $			a list of duplicate symbols.
$ $			%1$s:	"ld" or "bind"

UNRES_MSG	"%1$s: 0711-318 ERROR: Undefined symbols were found.\n\
\tThe following symbols are in error:"
$ $		Translation Information:
$ $			This is a message that is printed at the top of
$ $			a list of duplicate symbols.
$ $			%1$s	"ld" or "bind"

DUPLICATE_HEAD1	"\
 Symbol                   %s Inpndx  Address  TY CL Length Align Source-File(Object-File) OR Import-File{Shared-object}\n\
 -------------------------%s ------- -------- -- -- ------ ----- ------------------------------------------------------"
$ $      1         2           3         4         5         6         7         8         9        10        11
$ 345678901234567890123456  7890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567
$ $		Translation Information:
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			The last column begins with Source-File.
$ $			For all other columns, the heading is a single word without blanks.
$ $			The underlining for the last column should be the same length
$ $				as the column heading.
$ $			%s: Leave both instances in place.  Assume that they will
$ $				be replaced by null strings.
$ $		    The column headings mean:
$ $			Symbol: A symbol name
$ $			Inpndx: Input symbol-table index
$ $			Address: 8-digit hex address
$ $			TY: Symbol Type
$ $			CL: Storage-mapping Class
$ $			Length: Decimal length
$ $			Align: Decimal Alignment
$ $			Source-File ...: Path name of input object

DUPLICATE_HEAD2	"\
 Symbol                    Source-File(Object) OR Import-File{Shared-object}\n\
 ------------------------- -------------------------------------------------"
$ $      1         2         3         4         5         6         7
$ 34567890123456789012345678901234567890123456789012345678901234567890123456
$ $		Translation Information:
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			The last column begins with Source-File.
$ $			The underlining for the last column should be the same length
$ $				as the column heading.
$ $		    The column headings mean:
$ $			Symbol: A symbol name
$ $			Source-File ...: Path name of input object

DUPLICATE_TOC_HEAD1	"\
 Referenced Symbol         TOC Symbol               %s Inpndx  Address  TY Source-File(Object-File) OR Import-File{Shared-object}\n\
 ------------------------- -------------------------%s ------- -------- -- ------------------------------------------------------"
$ $      1         2         3         4         5           6         7         8         9        10        11        12
$ 34567890123456789012345678901234567890123456789012  345678901234567890123456789012345678901234567890123456789012345678901234567
$ $		Translation Information:
$ $			Do not translate: TOC
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			The first 2 columns have two-word headings.
$ $			The last column begins with Source-File.
$ $			For all other columns, the heading is a single word without blanks.
$ $			The underlining for the last column should be the same length
$ $				as the column heading.
$ $			%s: Leave both instances in place. Assume that they will
$ $				be replaced by null strings.
$ $		    The column headings mean:
$ $			Referenced Symbol: Name of a symbol referenced from TOC symbol
$ $			TOC Symbol: Name of a TOC symbol
$ $			Inpndx: Input symbol-table index
$ $			Address: 8-digit hex address
$ $			TY: Symbol Type
$ $			Source-File ...: Path name of input object

DUPLICATE_TOC_HEAD2	"\
 Referenced Symbol         TOC Symbol                Source-File(Object) OR Import-File{Shared-object}\n\
 ------------------------- ------------------------- -------------------------------------------------"
$ $      1         2         3         4         5         6         7         8         9        10
$ 3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
$ $		Translation Information:
$ $			Do not translate: TOC
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			The first 2 columns have two-word headings.
$ $			The third column begins with Source-File.
$ $			The underlining for the last column should be the same length
$ $				as the column heading.
$ $		    The column headings mean:
$ $			Referenced Symbol: Name of a symbol referenced from TOC symbol
$ $			TOC Symbol: Name of a TOC symbol
$ $			Source-File ...: Path name of input object

MISMATCH_HEAD1	"\
 Symbol                    Hash                  %s Inpndx  TY CL Source-File(Object-File) OR Import-File{Shared-object}\n\
                                            RLD:%s Address  Section  Rld-type%s Referencing Symbol\n\
 ------------------------- ----------------------%s --------------------------------------------------------------------"
$ $      1         2         3         4           5         6         7         8         9        10        11
$ 34567890123456789012345678901234567890123456789  012345678901234567890123456789012345678901234567890123456789012345678
$ $		Translation Information:
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			Exception: If RLD is translated, its translation should end in the
$ $				same column as the D in RLD.  "RLD" stands for
$ $				"relocation dictionary entry"
$ $			The last column begins with Source-File.
$ $			The last column on the second line begins with Referencing
$ $			For all other columns, the heading is a single word without blanks.
$ $			The underlining for the last several columns should extend to
$ $				the end of the last column on the first line.
$ $			%s: Leave all instances in place. Assume that they will
$ $				be replaced by null strings.
$ $		    The column headings mean:
$ $			Symbol: Name of a symbol
$ $			Hash: Formatted type-checking hash string
$ $			Inpndx: Input symbol-table index
$ $			TY: Symbol Type
$ $			CL: Storage-mapping Class
$ $			Source-File ...: Path name of input object
$ $		    The second line lists alternate column headings:
$ $			Address: 8-digit hex address
$ $			Section: Section name: ".text" or ".data"
$ $			Rld-type: Symbolic name of RLD type
$ $			Referencing Symbol: A symbol name

MISMATCH_HEAD2	"\
 Symbol                    Hash                  %s Inpndx  TY CL Source-File(Object-File) OR Import-File{Shared-object}\n\
 ------------------------- ----------------------%s ------- -- -- ------------------------------------------------------"
$ $      1         2         3         4           5         6         7         8         9        10        11
$ 34567890123456789012345678901234567890123456789  012345678901234567890123456789012345678901234567890123456789012345678
$ $		Translation Information:
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			The last column begins with Source-File.
$ $			For all other columns, the heading is a single word without blanks.
$ $			The underlining for the last several columns should extend to
$ $				the end of the last column on the first line.
$ $			%s: Leave all instances in place. Assume that they will
$ $				be replaced by null strings.
$ $		    The column headings mean:
$ $			Symbol: Name of a symbol
$ $			Hash: Formatted type-checking hash string
$ $			Inpndx: Input symbol-table index
$ $			TY: Symbol Type
$ $			CL: Storage-mapping Class
$ $			Source-File ...: Path name of input object

UNRES_HEAD1	"\
 Symbol                   %s Inpndx  TY CL Source-File(Object-File) OR Import-File{Shared-object}\n\
                              RLD:%s Address  Section  Rld-type%s Referencing Symbol\n\
 -------------------------%s---------------------------------------------------------------------"
$ $      1         2           3         4         5         6         7         8         9
$ 345678901234567890123456  789012345678901234567890123456789012345678901234567890123456789012345
$ $		Translation Information:
$ $		    Preserve alignment of columns: Numbers show column positions.
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			Exception: If RLD is translated, its translation should end in the
$ $				same column as the D in RLD.
$ $			The last column on the first line begins with Source-File.
$ $			The last column on the second line begins with Referencing
$ $			For all other column, the heading is a single word without blanks.
$ $			%s: Leave all instances in place.
$ $		    The column headings mean:
$ $			Inpndx: Input index
$ $			TY: Symbol Type
$ $			CL: Storage-mapping Class
$ $			Source-File ...: Path name of input object
$ $		    The second line lists alternate column headings:
$ $			Address: 8-digit hex address
$ $			Section: Section name: ".text" or ".data"
$ $			Rld-type: Symbolic name of RLD type
$ $			Referencing Symbol: A symbol name

UNRES_HEAD2	"\
 Symbol                   %s Inpndx  TY CL Source-File(Object-File) OR Import-File{Shared-object}\n\
 -------------------------%s ------- -- -- ------------------------------------------------------"
$ $      1         2         3         4         5         6         7         8         9
$ 34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567
$ $		Translation Information:
$ $		    Preserve alignment of columns: Numbers show column positions.
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			Exception: If RLD is translated, its translation should end in the
$ $				same column as the D in RLD.
$ $			The last column begins with Source-File.
$ $			For all other column, the heading is a single word without blanks.
$ $			%s: Leave all instances in place.
$ $		    The column headings mean:
$ $			Inpndx: Input index
$ $			TY: Symbol Type
$ $			CL: Storage-mapping Class
$ $			Source-File ...: Path name of input object

UNRES_KEEP	"%1$s: 0711-301 WARNING: Symbol specified with the -u flag not defined: %2$s"
$ $		Translation Information:
$ $			Do not translate: -u
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name

UNRES_EXPORT	"%1$s: 0711-319 WARNING: Exported symbol not defined: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name

RSLV_NOTCMATCH	"%1$s: 0711-194 WARNING: TOC entries for symbol %2$s contain different\n\
\tvalues. The entries are not being combined."
$ $		Cause:
$ $		Action:
$ $		Technical Information:
$ $			If DBOPT4 is set, this will display if the actual
$ $				contents of two equal TOC entries are not
$ $				the same.
$ $		Translation Information: DO NOT TRANSLATE
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Symbol name

CMPCTS_KEPT	"%1$s: %2$d of %3$d symbols were kept."
$ $		Cause:
$ $			Displays the results of "RESOLVE" subcommand
$ $			processing.
$ $		Action:
$ $		Technical Information:
$ $			Last statement of resolve.
$ $		Translation Information:
$ $			%1$s	Subcommand name (RESOLVE)
$ $			%2$d	Number of symbols that survived.
$ $			%3$d	Original number of symbols.

CMPCT_RCW_NDESC	"%1$s: 0711-220 WARNING: The entry point is not a descriptor: %2$s"
$ $		Cause:
$ $			The symbol specified on the "-e symbol" option of
$ $				the ld command is not the symbol of a
$ $				descriptor.
$ $		Translation Information:
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Name of entry point

CMPCT_LOADMAP1	"%1$s: 0711-344 See the loadmap file %2$s for more information."
$ $		Technical Information:
$ $			Printed after the first occurrence of a message that
$ $			writes additional information to the loadmap file.
$ $		Translation Information:
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Pathname of current loadmap file.

CMPCT_LOADMAP2	"%1$s: 0711-345 Use the -bloadmap or -bnoquiet option to obtain more information."
$ $		Technical Information:
$ $			Printed after the first occurrence of a message that
$ $			could write additional information to the loadmap file,
$ $			if there is no loadmap file.
$ $		Translation Information:
$ $			Do not translate: -bloadmap -bnoquiet
$ $			%1$s:	"ld" or "bind"

CMPCT_RCW_CONST	"%1$s: 0711-227 WARNING: Symbol %2$s may be a CONSTANT."
$ $		Translation Information: DO NOT TRANSLATE

DATAS_NORDONLY	"%s: No .loader section relocation entries exist for the .text section."
$ $		Cause:	The desired message when "setopt R/O->W" is used.
$ $			Indicates that the .text is really read-only.
$ $		Action:
$ $		Technical Information:
$ $		Translation Information:
$ $			%s:	"ld" or "bind"
$ $			Do not translate .text .loader

MISMATCH_NONE	"%s: No type mismatches exist."
$ $		Cause:
$ $			Success message. There were no parameter type
$ $			mismatches found in the results of the bind.
$ $		Action:
$ $		Technical Information:
$ $			If the MISMATCH subcommand does not find any mismatches,
$ $			this statement is displayed.  (RC_OK)
$ $		Translation Information:
$ $			%s:	Subcommand name (MISMATCH)

DATAS_CSDUP	"%1$s: 0711-295 ERROR: The -bdelcsect option was used and the undefined symbol\n\
\tis in a csect containing duplicate symbol: %2$s"
$ $		Translation Information:
$ $			Do not translate: csect
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Symbol name

DATAS_NOUNRES	"%s: There are no unresolved symbols."
$ $		Translation Information:
$ $			%s:	Subcommand name (ER)

DATA_RDONLYREF	"%s: 0711-310 ERROR: Relocation entries from the .text section have been\n\
\twritten to the .loader section. The following csects are in error:"
$ $		Translation Information:
$ $			Do not translate .text csect .loader
$ $			%s:	"ld" or "bind"

GLUE_COUNT	"%1$s: Glink code added for %2$d symbols."
$		Translation Information:
$ $			%1$s:	"ld" or "bind"
$ $			%2$d:	Number of symbols processed

GLUE_REPLACEMENT	"%1$s: Glink symbol %2$s replaces"
$ $		Translation Information:
$ $			This message is followed by the name of
$ $			(and information about) another symbol.
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Name of symbol

GLUE_PTRGL_NO	"%1$s: 0711-362 ERROR: Glink code cannot be added for function: %2$s\n\
\tThis function is used by compilers and must be called directly."
$ $		Translation Information:
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Name of reserved function

ADDNAMEDGLUE_NO_MATCH	"%1$s: 0711-363 WARNING: %2$s: No function names match pattern %3$s"
$ $		Translation Information:
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	Subcommand (ADDNAMEDGLUE)
$ $			%3$s:	Pattern

GLUE_BAD_FILE	"%1$s: 0711-364 ERROR: Invalid glink code file: %2$s\n\
\tAn XCOFF file is needed. No glink code added."
$ $		Translation Information:
$ $			%1$s:	"ld" or "bind"
$ $			%2$s:	File name

GLUE_BAD_TYPE		"%1$s: 0711-365 ERROR: Unknown glink type: %2$s.\n\
\tChoices are:"
$ $		Translation Information:
$ $			The valid choices are listed after this message.
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Glink type that was specified.

GLUE_TYPECHK		"%1$s: 0711-587 WARNING: Object %2$s:\n\
\tType-checking information from a glink file is ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of glink file.

GLUE_TOO_MANY_NAMES	"%1$s: 0711-595 ERROR: Only one external name is allowed in a glink file.\n\
\tThe glink file %2$s is invalid."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of glink file.

GENERATE_DESC	"%1$s: Generating descriptor: %2$s"
$ $		Translation Information:
$ $			%1$s	Subcommand name (RESOLVE)
$ $			%2$s	Symbol name

GENERATE_TOC_ENTRY	"%1$s: Generating TOC entry for %2$s"
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			%1$s	Subcommand name (RESOLVE)
$ $			%2$s	Symbol name

IMPSS_IMP	"%1$s: Generating implicitly imported symbol: %2$s"
$ $		Translation Information:
$ $			%1$s	Subcommand name (RESOLVE)
$ $			%2$s	Symbol name

IMPORTS_COUNT	"%1$s: Symbols imported from import file %2$s: %3$d"
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of import file
$ $			%3$d	Number of symbols imported.

SHARED_IMPORT	"%1$s: Symbol imported: %2$s"
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of imported symbol

SHARED_COUNT	"%1$s: Shared object %2$s: %3$d symbols imported."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object
$ $			%3$d	Number of symbols imported from object.

SHARED_NOLDR1		"%1$s: 0711-434 SEVERE ERROR: Shared object %2$s\n\
\tThe shared object has no .loader section and is being ignored."
$ $		Translation Information:
$ $			Do not translate: .loader
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object

SHARED_NOLDR4		"%1$s: 0711-441 SEVERE ERROR: Shared object %2$s\n\
\tThe s_scnptr or s_size field is invalid in the .loader section header.\n\
\tThe object is being ignored."
$ $		Translation Information:
$ $			Do not translate: s_scnptr s_size .loader
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object

SHARED_NOLDR5		"%1$s: 0711-444 SEVERE ERROR: Shared object %2$s\n\
\tThe section headers could not be read. The object is being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object

SHARED_NOLDR6		"%1$s: 0711-445 SEVERE ERROR: Shared object %2$s\n\
\tCannot read the .loader section. The object is being ignored."
$		Translation Information:
$ $			Do not translate: .loader
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object

SHARED_BAD_LDR_OFFSET	"%1$s: 0711-449 SEVERE ERROR: Shared object %2$s\n\
\tSymbol table entry %3$d  in the .loader section is invalid. The value of\n\
\tthe l_offset field is %4$d, but should be between 0 and %5$d.\n\
\tThe symbol is being ignored."
$ $		Translation Information:
$ $			Do not translate: l_offset .loader
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object
$ $			%3$d	Number of invalid loader symbol table entry
$ $			%4$d	Value of l_offset found.
$ $			%5$d	Max allowed value of l_offset.

SHARED_BAD_LOADER	"%1$s: 0711-446 SEVERE ERROR: Shared object %2$s\n\
\tSymbol table entry %3$d (symbol %4$s) in the .loader section is invalid.\n\
\tThe value of the l_parm field is %5$d, but should be be between\n\
\t0 and %6$d. No type-checking string will be read for the symbol."
$ $		Translation Information:
$ $			Do not translate: l_parm .loader
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object
$ $			%3$d	Number of invalid loader symbol table entry
$ $			%4$s	Name of invalid loader symbol table entry
$ $			%5$d	Value of l_parm found.
$ $			%6$d	Maximum allowed value of l_parm.

SHARED_INTERRUPTED	"%1$s: 0711-448 WARNING: Some symbols were not read from shared object %2$s.\n\
\tAdditional errors may occur."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object

SHARED_TRUNCATED	"%1$s: 0711-447 SEVERE ERROR: Shared object %2$s\n\
\tThe .loader section has been truncated. The object is being ignored."
$ $		Translation Information:
$ $			Do not translate: .loader
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of shared object

ENTRY_NONE	"%s: There is no entry point."
$		Technical Information:
$			This is displayed by ENTRY subcommand whenever
$			ENTRY is entered without an argument and the
$			entry point has not been set.
$		Translation Information:
$ $			%s	Subcommand name (ENTRY)

ENTRY_QUERY	"%1$s: The entry point is %2$s"
$		Translation Information:
$ $			%s	Subcommand name (ENTRY)
$ $			%2$s	Entry point name

ENTRY_SET	"%1$s: Entry point set to %2$s"
$ $		Translation Information:
$ $			%s	Subcommand name (ENTRY)
$ $			%2$s	Entry point name

LIBPATH_QUERY	"%1$s: The library path is %2$s"
$ $		Translation Information:
$ $			%s	Subcommand name (LIBPATH)
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Library path (a set of pathnames
$ $				separated by :)

LIBPATH_SET	"%1$s: Library path set to %2$s"
$ $		Translation Information:
$ $			%s	Subcommand name (LIBPATH)
$ $			%2$s	Library path (a set of pathnames
$ $				separated by :)

RC_QUERY	"%1$s: Highest return code was %2$d."
$ $		Translation Information:
$ $			%1$s	Subcommand name (RC)
$ $			%2$d	Maximum return code

FILELIST_COUNT	"%1$s: Number of previously inserted files processed: %2$d"
$		Translation Information:
$ $			%1$s	Subcommand name (FILELIST)
$ $			%2$d	number of files processed

INSRT_UNKSECT	"%1$s: 0711-523 WARNING: Section %2$d (%3$.8s) in object %4$s:\n\
\tSection type %5$d is not recognized. The section is being ignored."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad section header
$ $			%3$.8s	Name of bad section header
$ $			%4$s	Name of object file being read.
$ $			%5$d	Type of bad section header.

INSRT_RLDSECT	"%1$s: 0711-547 SEVERE ERROR: Object %2$s cannot be processed.\n\
\tRLD address 0x%3$x for section %4$d (%5$s) is\n\
\tnot contained in the section."
$		Translation Information:
$ $			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object file being read.
$ $			%3$d	Address contained in bad RLD
$ $			%4$d	Section number for section being processed.
$ $			%5$s	Section name for section being processed.

INSRT_RLDORDER	"%1$s: 0711-548 SEVERE ERROR: Object %2$s cannot be processed.\n\
\tRLD address 0x%3$x for section %4$d (%5$s) is\n\
\tout of order. RLDs must be in ascending address order."
$		Translation Information:
$ $			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object file being read.
$ $			%3$d	Address contained in bad RLD
$ $			%4$d	Section number for section being processed.
$ $			%5$s	Section name for section being processed.

INSRT_SYMTABCOR	"%1$s: 0711-562 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n\
\tAn auxiliary symbol table entry is required for a\n\
\tC_EXT or C_HIDEXT entry."
$		Translation Information:
$ $			Do not translate: C_EXT C_HIDEXT
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object file being read.

ALIGN_UPDATE	"%1$s: The alignment of %2$s was %3$d."
$ $		Translation Information:
$ $			%1$s	Subcommand name (ALIGN)
$ $			%2$s	Symbol name
$ $			%3$d	Alignment (an integer)

ALIGN_UPDATE2	"%1$s: The alignment of the csect containing the label %2$s was %3$d."
$ $		Translation Information:
$ $			%1$s	Subcommand name (ALIGN)
$ $			%2$s	Label name
$ $			%3$d	Alignment (an integer)

ALIGN_BADARG	"%1$s: 0711-611 SEVERE ERROR: %2$s: Invalid alignment.\n\
\tValue must be in the range 0-31. Value specified is %3$s.\n\
\tUSAGE: ALIGN [pattern] {0-31}"
$		Translation Information:
$ $			Do not translate: ALIGN {0-31}
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Value specified

ALIGN_NOMATCH	"%1$s: 0711-613 WARNING: %2$s: No global symbols match pattern: %3$s"
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Subcommand name (ALIGN)
$ $			%3$s	pattern specified with ALIGN subcommand

NAMES_NOTFOUND	"%1$s: 0711-673 ERROR: %2$s: Symbol %3$s was not found."
$		Translation Information:
$			%1$s	"ld" or "bind"
$			%2$s:	Subcommand name (RENAME)
$			%3$s:	name of symbol not found.

IFILE_EMPTY	"%1$s: 0711-711 ERROR: Input file %2$s is empty.\n\
\tThe file is being ignored."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of empty file.

IFILE_CANNOT_FREE_SEG	"%s: 0711-165 SEVERE ERROR: Cannot free memory segments."
$ $		Translation Information:
$ $			%s	"ld" or "bind"
IFILE_SIZE_ERROR	"%1$s: 0711-166 SEVERE ERROR: Size of file %2$s\n\
\thas changed since it was first opened."
$ $		Cause:	An object file was modified during the bind
$ $		Action:	The file is closed and cannot be used again.
$ $		Technical Information:
$ $			This can only happen if so many files are being linked
$ $			that some files have to be closed and reopened during
$ $			the link
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file whose size has changed

INPUT_BAD_FILE	"%1$s: 0711-168 SEVERE ERROR: Input file: %2$s\n\
\tInput files must be regular files."
$ $		Translation Information:
$ $			"regular file" is a technical term
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file

IFILE_UNKNOWN_TYPE	"%1$s: 0711-715 ERROR: File %2$s cannot be processed.\n\
\tThe file must be an object file, an import file, or an archive."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file

SAVE_LDRINFO	"%1$s: LDRINFO string added to .loader section: %2$s"
$		Translation Information:
$ $			Do not translate: LDRINFO .loader
$ $			%1$s	"SAVE"
$ $			%2$s	String being added

SAVE_SIZES	"%1$s: Section sizes = %2$d+%3$d+%4$d (0x%2$X+0x%3$X+0x%4$X hex)"
$		Translation Information:
$ $			%1$s		"SAVE"
$ $			%2$d,%2$X	Size of .text section
$ $			%3$d,%3$X	Size of .data section
$ $			%4$d,%4$X	Size of .bss section
$ $			hex		short for hexadecimal

SAVE_TOC_SIZE	"%1$s: Size of TOC: %2$d (0x%2$X hex)"
$		Cause:	Always reported after successful save.
$		Technical Information:
$		Translation Information:
$			Do not translate: TOC
$ $			%1$s		"SAVE"
$ $			%2$d,%2$X	Size of TOC
$ $			hex		short for hexadecimal

SAVE_BAD_TOCLOAD	"%1$s: 0711-768 WARNING: Object %2$s, section %3$d, function %4$s:\n\
\tThe branch at address 0x%5$x is not followed by a recognized no-op\n\
\tor TOC-reload instruction. The unrecognized instruction is 0x%6$X."
$		Cause:
$		Action:
$		Technical Information:
$			Discover what should have been a load instruction
$			following a branch and link instruction,
$			but it was not.
$		Translation Information:
$ $			Do not translate: TOC
$ $			A TOC-reload instruction is an instruction that loads
$ $			a previous value into a machine register (called the
$ $			TOC register.)
$ $			no-op:	An abbreviation for no-operation
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input object in error
$ $			%3$d	Section number of erroneous section
$ $			%4$s	Symbol name
$ $			%5$x	Address of bad instruction
$ $			%6$X	Hex value of instruction

SAVE_RELFLDOVRF	"%1$s: 0711-780 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tRelocation overflow in reference to: %5$s (entry %6$d)\n\
\tAddress: 0x%7$08x;    RLD type: %8$s;   RLD length: %9$d"
$		Translation Information:
$			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol making bad reference
$ $			%3$d	Symbol table index of above symbol
$ $			%4$s	Input file in error
$ $			%5$s	Name of referenced symbol
$ $			%6$d	Symbol table index of referenced symbol
$ $			0x%7$08x	Address of bad refernce
$ $			%8$s	RLD type of bad reference
$ $			%9$d	RLD length of bad reference

SAVE_TOCOVRF	"%1$s: 0711-781 ERROR: TOC overflow. TOC size: %2$d\tMaximum size: %3$d"
$		Translation Information:
$			Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Current size of TOC
$ $			%3$d	Maximum size of TOC

SAVE_TOCOVRF2	"%1$s: 0711-783 WARNING: TOC overflow. TOC size: %2$d\tMaximum size: %3$d\n\
\tExtra instructions are being generated for each reference to a TOC\n\
\tsymbol if the symbol is in the TOC overflow area."
$		Translation Information:
$			Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Current size of TOC
$ $			%3$d	Maximum size of TOC

RELOC_BAD_RLD	"%1$s: 0711-782 SEVERE ERROR: Object %2$s, section %3$d\n\
\tThe RLD at address %4$x has an unrecognized RLD type 0x%5$x.\n\
\tThe RLD is not being processed."
$		Translation Information:
$			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input object in error
$ $			%3$d	Section number of erroneous section
$ $			%4$x	Address of bad RLD
$ $			%5$x	Hex value of bad RLD

RELOC_CANNOT_FIXUP1	"%1$s: 0711-793 SEVERE ERROR: Object %2$s, RLD address 0x%3$x in section %4$d:\n\
\tThe instruction at address 0x%5$x cannot be fixed up:\n\
\tThe bit length of the reference is %6$d, not 16."
$			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input object in error
$ $			%3$x	Address of bad RLD
$ $			%4$d	Section number of erroneous section
$ $			%5$x	Address of bad instruction
$ $			%6$d	Bit length specified in RLD

RELOC_CANNOT_FIXUP2	"%1$s: 0711-794 SEVERE ERROR: Object %2$s, RLD address 0x%3$x in section %4$d:\n\
\tThe instruction at address 0x%5$x cannot be fixed up:\n\
\tThe opcode is %6$d."
$			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input object in error
$ $			%3$x	Address of bad RLD
$ $			%4$d	Section number of erroneous section
$ $			%5$x	Address of bad instruction
$ $			%6$d	Opcode used in bad instruction

FIXUP_BAD	"%1$s: 0711-788 SEVERE ERROR: Object %2$s, RLD address 0x%3$x in section %4$d:\n\
\tThe fixed-up instruction at address 0x%5$x is not a branch.\n\
\tIts opcode is %6$d."
$			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input object in error
$ $			%3$x	Address of bad RLD
$ $			%4$d	Section number of erroneous section
$ $			%5$x	Address of bad instruction
$ $			%6$d	Opcode used in bad instruction

FIXUP_CSECT_SIZE	"%1$s: Fixup csect size: %2$d"
$		Translation Information:
$			Do not translate: csect
$ $			%1$s	Subcommand name (SAVE)
$ $			%2$d	Size of csect

FIXUP_SIZE	"%1$s: Bytes written to fixup csect: %2$d"
$		Translation Information:
$			Do not translate: csect
$ $			%1$s	Subcommand name (SAVE)
$ $			%2$d	Bytes written to csect

SAVE_BAD_BRANCH	"%1$s: 0711-787 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe instruction at address 0x%5$08x is not a branch. It is 0x%6$08x.\n\
\tRLD type: %7$s;   RLD length: %8$d;    Referenced symbol: %9$s"
$		Cause:
$		Action:
$		Technical Information:
$			Discover what should have been a branch instruction,
$			but it was not.
$		Translation Information:
$ $			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol making bad reference
$ $			%3$d	Symbol table index of above symbol
$ $			%4$s	Input file in error
$ $			0x%5$08x	Address of bad instruction
$ $			0x%6$08x	Hex value of bad instruction
$ $			%7$s	RLD type of bad reference
$ $			%8$d	RLD length of bad reference
$ $			%9$s	Name of referenced symbol

SAVE_BA_BAD	"%1$s: 0711-795 SEVERE ERROR: Object %2$s, section %3$d\n\
\tThe relocation type of the RLD for address 0x%4$x is R_BA,\n\
\tbut the instruction is not an absolute branch.\n\
\tThe referenced symbol is %5$s"
$		Translation Information:
$			Do not translate: RLD R_BA
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input file in error
$ $			%3$d	Section number of erroneous instruction
$ $			%4$x	Address of bad instruction
$ $			%5$s	Name of referenced symbol

SAVE_BA_RELOC	"%1$s: 0711-796 WARNING: Object %2$s, section %3$d\n\
\tThe branch instruction at address 0x%4$x, which has RLD type R_BA,\n\
\trefers to symbol %5$s, which is a relocatable symbol."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input file in error
$ $			%3$d	Section number of erroneous instruction
$ $			%4$x	Address of bad instruction
$ $			%5$s	Name of referenced symbol

SAVE_BR_BAD	"%1$s: 0711-797 SEVERE ERROR: Object %2$s, section %3$d\n\
\tThe relocation type of the RLD for address 0x%4$x is R_BR,\n\
\tbut the instruction is an absolute branch.\n\
\tThe referenced symbol is %5$s"
$		Translation Information:
$			Do not translate: RLD R_BR
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input file in error
$ $			%3$d	Section number of erroneous instruction
$ $			%4$x	Address of bad instruction
$ $			%5$s	Name of referenced symbol

SAVE_BR_RELOC	"%1$s: 0711-798 WARNING: Object %2$s, section %3$d\n\
\tThe branch instruction at address 0x%4$x, which has RLD type R_BR,\n\
\trefers to symbol %5$s, which is not a relocatable symbol."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input file in error
$ $			%3$d	Section number of erroneous instruction
$ $			%4$x	Address of bad instruction
$ $			%5$s	Name of referenced symbol

BRANCH_UNALIGN	"%1$s: 0711-799 SEVERE ERROR: Object %2$s, section %3$d\n\
\tThe target of the branch instruction at address %4$x is not aligned\n\
\ton a word boundary. The target address is being truncated."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input object in error
$ $			%3$d	Section number of erroneous section
$ $			%4$x	Address of branch with bad target.

SAVE_NO_TOC	"%1$s: 0711-868 INTERNAL ERROR: The TOC symbol should have already been written."
$		Translation Information: DO NOT TRANSLATE
$ $			Do not translate: TOC
$ $			%1$s	"ld" or "bind"

SAVE_RCW_BADSC	"%1$s: 0711-870 WARNING: Symbol table entry %2$d in object %3$s:\n\
\tUnknown symbol type %4$d. The symbol is being copied to the output file."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object file being processed.
$ $			%4$d	Type of bad symbol.

STAT_SYM_MISSING	"%1$s: 0711-871 WARNING: Symbol table entry %2$d in object %3$s:\n\
\tSymbol contains a reference to a static csect that was not saved."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object file being processed.

SAVE_MISCOUNT1	"%s: 0711-885 INTERNAL ERROR: Number of sections miscounted.\n\
\tThe .loader section cannot be written."
$		Cause:
$		Action:
$		Technical Information:
$			Binder internal flags became confused.
$		Translation Information:
$ $			Do not translate: .loader
$ $			%s	"ld" or "bind"

SAVE_BADNAME	"%1$s: 0711-933 SEVERE ERROR: Pathname %2$s is invalid or inaccessible.\n\
\t%1$s:%3$s() %4$s"
$		Cause:	A bad file name was specified.
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file
$ $			%3$s	Name of failing subcommand
$ $			%4$s	Message from strerror()

SAVE_BAD_OUTPUT	"%1$s: 0711-934 SEVERE ERROR: Cannot create the output file: %2$s\n\
\t%1$s:open() %3$s"
$		Translation Information:
$ $			Do not translate: open()
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file
$ $			%3$s	Reason for failure (from strerror())

SAVE_READONLY	"%1$s: 0711-936 SEVERE ERROR: Output file: %2$s\n\
\tThe file is on a read-only file system."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of output file.

SAVE_NOSAVE1	"%1$s: 0711-937 SEVERE ERROR: Severe errors have occurred and\
 the output\n\
\tfile (%2$s) already exists. The file will not be overwritten."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of output file.

SAVE_NOSAVE2	"%1$s: 0711-938 ERROR: Errors have occurred and \
the output file %2$s,\n\
\twhich already exists, was used as an input file.\n\
\tThe file will not be overwritten."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of output file.

SAVE_TMP_CREAT	"%1$s: 0711-925 SEVERE ERROR: Cannot create temporary output file: %2$s\n\
\t%1$s:open() %3$s"
$		Translation Information:
$ $			Do not translate: open()
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of temporary file
$ $			%3$s	Message from strerror()

SAVE_NOSHMATMP	"%1$s: 0711-929 SEVERE ERROR: Cannot map the temporary output file: %2$s\n\
\t%1$s:%3$s() %4$s"
$		Technical Information:
$			The output file couldn't be mapped (using shmat()).
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of temporary file
$ $			%3$s	Name of failing subcommand
$ $			%4$s	Message from strerror()

SAVE_BADNAME1		"%1$s: 0711-876 ERROR: Filename %2$s,\n\
\tspecified by the SAVENAME command, was invalid."
$ $		Translation Information:
$ $			Do not translate: SAVENAME
$ $			%1$s	"ld" or "bind"
$ $			%2$s	File name specified.

SAVE_BADNAME2		"%1$s: 0711-877 ERROR: Filename must be %2$s\n\
\tUse the same name as specified with the SAVENAME binder command."
$ $		Translation Information:
$ $			Do not translate: SAVENAME
$ $			%1$s	"ld" or "bind"
$ $			%2$s	File name specified.

SAVE_BADTYPE		"%1$s: 0711-875 SEVERE ERROR: Output file: %2$s\n\
\tThe output file must be a regular file."
$ $		Translation Information:
$ $			"regular file" is a technical term
$ $			%1$s	"ld" or "bind"
$ $			%2$s	File name specified.

SAVE_NOCLOSE2		"%1$s: 0711-855 SEVERE ERROR: Cannot close the output file: %2$s\n\
\t%1$s:close() %3$s"
$ $		Translation Information:
$ $			Do not translate: close()
$ $			You may translate: "Cannot close ..."
$ $			%1$s	"ld" or "bind"
$ $			%2$s	File name specified.
$ $			%3$s	Message from strerror()

SAVE_NULL	"%1$s: 0711-244 ERROR: No csects or exported symbols have been saved."
$ $		Translation Information:
$ $			Do not translate: csect (csects is the plural of csect)
$ $			%1$s	"ld" or "bind"

SAVE_NULL2	"%1$s: 0711-245 WARNING: No csects or exported symbols have been saved."
$ $		Translation Information:
$ $			This message is identical to the previous message
$ $			except that ERROR has been replaced by WARNING.
$ $			Do not translate: csect (csects is the plural of csect)
$ $			%1$s	"ld" or "bind"

SAVE_RENMTMPTOOUT1	"%1$s: 0711-851 SEVERE ERROR: Output file: %2$s\n\
\tThe file is in use and cannot be overwritten."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of output file.

SAVE_RENMTMPTOOUT3	"%1$s: 0711-853 SEVERE ERROR: Cannot rename the temporary object file: %2$s\n\
\tto be the output object file: %3$s\n\
\t%1$s:rename() %4$s"
$ $		Translation Information:
$ $			Do not translate: rename()
$ $			You may translate: "Cannot rename ..."
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of temporary file
$ $			%3$s	Name of output file.
$ $			%4$s	Message from strerror()

GEN_MAP3	"ADDRESS MAP FOR %s\n"
$		Translation Information:
$ $			Keep the final newline.
$ $			%s	Output file name

GEN_MAP2	"ADDRESS MAP FOR %s WITH REFERENCES\n"
$		Translation Information:
$ $			Keep the final newline.
$ $			%s	Output file name

GEN_MAP1	"ADDRESS MAP FOR %s WITH CROSS REFERENCES\n"
$		Translation Information:
$ $			Keep the final newline.
$ $			%s	Output file name

GEN_TITLE_DEFAULT2	"ADDRESS MAP FOR INPUT FILES\n"
$		Translation Information:
$ $			Keep the final newline.

GEN_TITLE_CALLS2	"ADDRESS MAP FOR INPUT FILES WITH REFERENCES\n"
$		Translation Information:
$ $			Keep the final newline.

GEN_TITLE_XREF2		"ADDRESS MAP FOR INPUT FILES WITH CROSS REFERENCES\n"
$		Translation Information:
$ $			Keep the final newline.

GEN_TITLE_DEFAULT_S	"SORTED ADDRESS MAP FOR %s\n"
$		Translation Information:
$ $			Keep the final newline.
$ $			%s	Output file name

GEN_TITLE_CALLS_S	"SORTED ADDRESS MAP FOR %s WITH REFERENCES\n"
$		Translation Information:
$ $			Keep the final newline.
$ $			%s	Output file name

GEN_TITLE_XREF_S	"SORTED ADDRESS MAP FOR %s WITH CROSS REFERENCES\n"
$		Translation Information:
$ $			Keep the final newline.
$ $			%s	Output file name

GEN_TITLE_DEFAULT2_S	"SORTED ADDRESS MAP FOR INPUT FILES\n"
$		Translation Information:
$ $			Keep the final newline.

GEN_TITLE_CALLS2_S	"SORTED ADDRESS MAP FOR INPUT FILES WITH REFERENCES\n"
$		Translation Information:
$ $			Keep the final newline.

GEN_TITLE_XREF2_S	"SORTED ADDRESS MAP FOR INPUT FILES WITH CROSS REFERENCES\n"
$		Translation Information:
$ $			Keep the final newline.

GEN_HDR		"*%s%s\
 ADDRESS  LENGTH AL CL TY Sym#  NAME                      SOURCE-FILE(OBJECT) or IMPORT-FILE{SHARED-OBJECT}\n\
%s---\
 -------- ------ -- -- -- ----- ------------------------- -------------------------------------------------\n"
$ $   1         2         3         4         5         6         7         8         9        10        11
$ 6789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901
$ $		Translation Information:
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			Each translated column heading should start in the same
$ $				position as the English heading.
$ $			Do not translate or alter "*" at the beginning of
$ $				the message.
$ $			The last heading begins with SOURCE-FILE.
$ $			For all other headings, the heading is a single word without blanks.
$ $			The underlining for the last column should be the same length
$ $				as the column heading.
$ $			%s: Leave all instances in place.  The first 2 instances
$ $				will be replaced by single characters.
$ $		    The column headings mean:
$ $			ADDRESS: 8-digit hex address
$ $			LENGTH: Decimal length
$ $			AL: Symbol Alignment
$ $			CL: Storage-mapping Class
$ $			TY: Symbol Type
$ $			Sym#: An internal symbol number
$ $			NAME: A symbol name
$ $			SOURCE-FILE ...: Path name of input object

GEN_XREF_HDR	"\
                                  References from:  ADDRESS  RLD-type(bit-length) Sym#  NAME\n"
$ $      1         2         3         4         5         6         7         8         9        10        11
$ 3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901
$ $		Translation Information:
$ $		    Preserve alignment of headings (numbers show column positions).
$ $			The "References from:" is a single heading and its
$ $				translation should END in the same position as the
$ $				English heading.
$ $			All other translated headings should start in the same
$ $				position as the English heading.
$ $		    The column headings mean:
$ $			Address: 8-digit hex address
$ $			RLD-type(bit-Length): A string giving the RLD type
$ $				and the length of the RLD
$ $			Sym#: An internal symbol number
$ $			NAME: A symbol name

GEN_CALLS_HDR	"\
    ADDRESS                         References to:  ADDRESS  RLD-type(bit-length) Sym#  NAME\n"
$ $      1         2         3         4         5         6         7         8         9        10        11
$ 3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901
$ $		Translation Information:
$ $		    Preserve alignment of headings (numbers show column positions).
$ $			The "References to:" is a single heading and its
$ $				translation should END in the same position as the
$ $				English heading.
$ $			All other translated headings should start in the same
$ $				position as the English heading.
$ $		    The column headings mean:
$ $			Address (2 instance): 8-digit hex address
$ $			RLD-type(bit-Length): A string giving the RLD type
$ $				and the length of the RLD
$ $			Sym#: An internal symbol number
$ $			NAME: A symbol name

GEN_BAD_TYPE	"%1$s: 0711-834 WARNING: Invalid address map type %2$s.\n\
\tChoices are: %3$s"
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Erroneous map type
$ $			%3$s	Valid map types (which will be separated
$ $				by spaces)

GEN_NO_OPEN	"%1$s: 0711-836 WARNING: Cannot open the address map file: %2$s\n\
\t%1$s:fopen() %3$s"
$		Translation Information:
$ $			Do not translate: fopen()
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file
$ $			%3$s	Error from strerror()

GEN_NO_WRITE	"%1$s: 0711-837 ERROR: Cannot write to the address map file: %2$s\n\
\t%1$s:fprintf() %3$s\n\
\tThe file may be incomplete."
$		Translation Information:
$ $			Do not translate: fprintf()
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file
$ $			%3$s	Error from strerror()

GEN_NO_MEM	"%1$s: 0711-838 WARNING: There is not enough memory available.\n\
\tThe address map file %2$s has not been generated."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Address map file name

GEN_NAMETOOLONG	"%1$s: 0711-839 WARNING: The pathname is too long.\n\
\tThe maximum length of a pathname is %2$d.\n\
\tThe address map file has not been generated."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Maximum length allowed (MAXPATHLEN)

FILE_WR_PROTECT	"%1$s: 0711-931 SEVERE ERROR: Output file: %2$s\n\
\tThe file is write-protected."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of output file.

ARCHIVE_NO_MTAB	"%1$s: 0711-200 SEVERE ERROR: Archive file %2$s cannot be processed.\n\
\tThe archive member table does not exist."
$ $		Cause:	The offset to the global symbol table is 0.
$ $		Action:	The archive is ignored.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive.

ARCHIVE_MISSING_SYM	"%1$s: 0711-201 SEVERE ERROR: External symbol %2$s, defined in member %3$s\n\
\tof archive %4$s, was not found in the archive's global symbol table."
$ $		Cause:	The global symbol table is inconsistent.
$ $		Action:	The symbol is treated as an internal symbol.  Other
$ $			errors may occur.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol
$ $			%3$s	Name of archive member
$ $			%4$s	Name of archive file

ARCHIVE_MTAB_BADOFF	"%1$s: 0711-202 SEVERE ERROR: Archive file %2$s cannot be processed.\n\
\tThe offset to the archive member table (%3$d), found in the archive\n\
\theader, is negative or is not consistent with the length of the archive."
$ $		Cause:	Archive has inconsistent information.
$ $		Action:	The archive is not read.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Offset to archive member table.

ARCHIVE_BAD_MEMTAB2	"%1$s: 0711-203 SEVERE ERROR: Archive file %2$s cannot be processed.\n\
\tThe length of the archive member table (%3$d) is less than %4$d\n\
\tor is not consistent with the length of the archive."
$ $		Cause:	The length of the archive member table is not consistent.
$ $		Action:	The archive is not read.
$ $		Technical Information: The length of the archive member table
$ $			must contain at least one ASCII number--the number
$ $			of members.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Length of archive member table.
$ $			%4$d	Minimum allowable length

ARCHIVE_BAD_MEMTAB3	"%1$s: 0711-204 SEVERE ERROR: Archive file %2$s cannot be processed.\n\
\tThe number of archive members (%3$d), specified in the archive member\n\
\ttable, is less than or equal to 0 or is not consistent with the length\n\
\tof the member table."
$ $		Cause:	Archive contains inconsistent information
$ $		Action:	The archive is not read.
$ $		Technical Information: The minimum length of the archive member
$ $			table is 12 + 12n + 2n, where n is the number of archive
$ $			members.  The maximum length depends on the lengths of
$ $			the member names.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Number of archive members.

ARCHIVE_BAD_GST		"%1$s: 0711-205 SEVERE ERROR: Archive file %2$s cannot be\n\
\tprocessed completely. Offset %3$d, found in the global symbol\n\
\ttable for symbol %4$s, does not match the offset of any member."
$ $		Cause:	The global symbol table is not consistent.
$ $		Action:	The symbol is ignored.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Offset from global symbol table
$ $			%4$s	Name of symbol

ARCHIVE_BAD_GST2	"%1$s: 0711-206 SEVERE ERROR: Archive file %2$s cannot be\n\
\tprocessed completely. The length of the global symbol table (%3$d)\n\
\tis less than 4 or is not consistent with the length of the archive."
$ $		Cause:	The global symbol table is not consistent.
$ $		Action:	The global symbol table is not processed.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Length of global symbol table

ARCHIVE_BAD_GST3	"%1$s: 0711-207 SEVERE ERROR: Archive file %2$s cannot be\n\
\tprocessed completely. The global symbol table cannot be read."
$ $		Cause:	A seek() or read() failed with the archive.
$ $		Action:	The archive is only partially read.
$ $		Technical Information:	This error can only occur if the file
$ $			is not mapped but is being read directly.  A message
$ $			should have been printed about the read or seek error
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member

ARCHIVE_BAD_GST_OFF	"%1$s: 0711-208 SEVERE ERROR: Archive file %2$s cannot be processed.\n\
\tThe offset to the global symbol table (%3$d), found in the archive\n\
\theader, is negative or is not consistent with the length of the archive."
$ $		Cause:	Archive contains inconsistent information
$ $		Action:	The archive is not read.
$		Technical Information:
$			File size was smaller than the minimum required for
$			the header.
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Offset to the global symbol table

ARCHIVE_BAD_SYM_COUNT	"%1$s: 0711-209 SEVERE ERROR: Archive file %2$s cannot be\n\
\tprocessed completely. The global symbol table should\n\
\tcontain %3$d names. Only %4$d names were found."
$		Cause:	End of symbol table was reached before all symbols had
$			been read.
$		Action:	Reading of the symbol table terminates.
$		Technical Information:
$			Count of global symbols was inconsistent with number
$			of names in table.
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Count of symbols from 1st 4 bytes of table.
$ $			%4$d	Count of symbols read.

ARCHIVE_BAD_NAMELEN	"%1$s: 0711-210 SEVERE ERROR: The length of archive member %2$s (%3$d)\n\
\tis negative or is not consistent with the length of the archive.\n\
\tThe member is being ignored."
$ $		Cause:	Archive member header is not consistent.
$ $		Action:	Member is ignored.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Length of the member name.

ARCHIVE_PREMEND		"%1$s: 0711-211 SEVERE ERROR: Archive member %2$s was found to be\n\
\ttruncated. The member is being ignored."
$ $		Cause:	An archive member is an xcoff file, but it is too short
$			 to contain its header.
$ $		Action:	The member is ignored.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and truncated member

ARCHIVE_GST_BAD		"%1$s: 0711-212 SEVERE ERROR: Symbol %2$s, found in the global symbol table\n\
\tof archive %3$s, was not defined in archive member %4$s."
$ $		Cause:	The global symbol table is not consistent.
$ $		Action:	The symbol is ignored.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol
$ $			%3$s	Name of archive file
$ $			%4$s	Name of archive member

ARCHIVE_BAD_NAMELN1	"%1$s: 0711-213 SEVERE ERROR: Archive file %2$s is corrupt.\n\
\tThe member header at offset %3$u is invalid. The length of the\n\
\tmember name (%4$d) is greater than 255, less than or\n\
\tequal to 0, or is not consistent with the length of the archive.\n\
\tThe member is being ignored."
$ $		Cause:	A member header contains inconsistent information.
$ $		Action:	The member is ignored.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$s	Offset to bad member header.
$ $			%4$s	Length of name in header.

ARCHIVE_BAD    "%1$s: 0711-214 SEVERE ERROR: Archive file %2$s cannot be processed."
$ $		Cause:	A seek() or read() failed with the archive.
$ $		Action:	The archive is not read.
$ $		Technical Information:	This error can only occur if the file
$ $			is not mapped but is being read directly.  A message
$ $			should have been printed about the read or seek error
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member

ARCHIVE_BADMEMOFF1	"%1$s: 0711-215 SEVERE ERROR: Archive file %2$s cannot be processed.\n\
\tThe offset to the first archive member header (%3$d),\n\
\tfound in the archive header, is negative or is not consistent\n\
\twith the length of the archive."
$ $		Cause:	Archive contains inconsistent information
$ $		Action:	The archive is not read.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Contents of fl_fstmoff field.

ARCHIVE_BADMEMOFF2	"%1$s: 0711-216 SEVERE ERROR: Archive file %2$s cannot be\n\
\tprocessed completely. The offset to the next member header (%4$d),\n\
\tfound in the member header beginning at offset %3$u,\n\
\tis negative or is not consistent with the length of the archive."
$ $		Translation Information:
$ $			Note:  %4$d comes before %3$u.  This is intentional
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$u	Offset to bad member header.
$ $			%4$d	Contents of ar_nxtmem field.

ARCHIVE_BAD3	"%1$s: 0711-217 SEVERE ERROR: Archive file %2$s was not completely read."
$ $		Cause:	A member header couldn't be read
$ $		Action:	Reading of the archive is terminated.
$ $		Technical Information:	This error can only occur if the file
$ $			is not mapped but is being read directly.  A message
$ $			should have been printed about the read or seek error.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member

ARCHIVE_BAD4	"%1$s: 0711-218 SEVERE ERROR: Archive file %2$s cannot be\n\
\tprocessed completely. The number of members (%3$d), as specified\n\
\tby the archive member table, is not consistent with the number\n\
\tof member headers found (%4$d)."
$ $		Cause:
$ $		Action:	Reading of archive terminates.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member
$ $			%3$d	Number of members expected
$ $			%4$d	Number of members read

ARCHIVE_LOADONLY	"%1$s: 0711-219 WARNING: Archive member %2$s\n\
\tThe member is being ignored because the F_LOADONLY flag is set."
$ $		Translation Information:
$ $			Do not translate: F_LOADONLY
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member

ARCHIVE_NESTED	"%1$s: 0711-230 WARNING: Nested archives are not supported.\n\
\tArchive member %2$s is being ignored."
$ $		Cause:	An archive is a member of another archive.
$ $		Action:	The archive member is ignored.
$ $		Technical Information:
$ $			Nested archives are not supported, primarily because
$ $			shared objects in a nested archive couldn't be
$ $			specified in the loader section of a file.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member

ARCHIVE_INTERRUPT1	"%1$s: 0711-231 WARNING: Reading of archive file %2$s interrupted.\n\
\tSome members were not read. Additional errors may occur."
$ $		Cause:	^C was pressed while a archive members were being read.
$ $		Action:	Reading of the file stops.  Some information may be
$ $			inconsistent, leading to more errors.
$ $		Technical Information:	Interactive mode only
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive

ARCHIVE_INTERRUPT2	"%1$s: 0711-232 WARNING: Reading of archive file %2$s interrupted.\n\
\tSome symbols from the global symbol table were not read.\n\
\tAdditional errors may occur."
$ $		Cause:	^C was pressed while the archive symbol table was
$ $			being read
$ $		Action:	Reading of the file stops.  Some information may be
$ $			inconsistent, leading to more errors.
$ $		Technical Information:	Interactive mode only
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive

ARCHIVE_BAD_MEMBER	"%1$s: 0711-235 WARNING: Archive member %2$s is not\n\
\tan object file or an import list. The member is being ignored."
$ $		Cause:	An archive includes an unrecognized member.
$ $		Action:	The member is ignored.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member

ARCHIVE_NO_MEMS	"%1$s: 0711-236 WARNING: Archive %2$s does not contain\n\
\tany members. The archive is being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive and member

EXPORT_SYSCALL1	"\tAdding syscall attribute for symbol."
$ $		Translation Information:
$ $			This message is printed after a symbol name.
$ $			Do not translate: syscall

EXPORT_SYSCALL2	"\tRemoving syscall attribute for symbol."
$ $		Translation Information:
$ $			This message is printed after a symbol name.
$ $			Do not translate: syscall

EXPORTS_COUNT		"%1$s: Symbols exported: %2$d"
$ $		Translation Information:
$ $			%1$s	Subcommand name (EXPORTS)
$ $			%2$d	Number of exported symbols

FOUND_CM		"%1$s: Found %2$d XTY_CMs."
$		Translation Information: DO NOT TRANSLATE.

FOUND_ER		"%1$s: Found %2$d XTY_ERs."
$		Translation Information: DO NOT TRANSLATE.

FOUND_FILE		"%1$s: Found %2$d C_FILEs."
$		Translation Information: DO NOT TRANSLATE.

FOUND_LD		"%1$s: Found %2$d XTY_LDs."
$		Translation Information: DO NOT TRANSLATE.

FOUND_SD		"%1$s: Found %2$d XTY_SDs."
$		Translation Information: DO NOT TRANSLATE.

KEEP_MSG		"%1$s: Symbol %2$s will be kept."
$		Translation Information:
$ $			%1$s	Subcommand name (KEEP)
$ $			%2$s	A symbol name

MEMBERS			"%s: These object files and archive members were\n\
\tused to create the output file:"
$		Translation Information:
$ $			This message is printed before a list if pathnames
$ $			is printed.
$ $			%s	"ld" or "bind"

SAVENAME_NAME 		"%1$s: The output filename is %2$s"
$		Translation Information:
$ $			%1$s	Subcommand name (SAVENAME)
$ $			%2$s	Output filename

SAVENAME_NONAME		"%s: No output filename has been specified yet."
$		Translation Information:
$ $			%s	Subcommand name (SAVENAME)

STRING_NO_STATS		"%s: String statistics:  No strings."
$ $		Translation Information: DO NOT TRANSLATE

STRING_STATS_HDR	"%s: String statistics:"
$ $		Translation Information: DO NOT TRANSLATE

STRING_TOO_LONG		"%1$s: 0711-950 ERROR: A name is too long.\n\
\tNames longer than 65535 characters are truncated.\n\
\tThe name begins: %2$-45.45s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$-45.45s	First 45 characters of bad name

BAD_ALIGN1		"%1$s: 0711-341 WARNING: Replaced XTY_CM symbol %2$s requires an\n\
\talignment more strict than the alignment of the symbol that replaces it."
$ $		Translation Information:
$ $			Do not translate: XTY_CM
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol being replaced

BAD_TEXT_ALIGN	"%s: 0711-356 WARNING: The .text section may not be aligned properly."
$ $		Translation Information:
$ $			Do not translate: .text
$ $			%1$s	"ld" or "bind"

BAD_DATA_ALIGN	"%s: 0711-357 WARNING: The .data section may not be aligned properly."
$ $		Translation Information:
$ $			Do not translate: .data
$ $			%1$s	"ld" or "bind"

BAD_AUX_NAME		"%1$s: 0711-252 SEVERE ERROR: File auxiliary symbol entry %2$d in object %3$s:\n\
\tField x_offset contains %4$d. Valid values are between 4 and %5$d.\n\
\tThe object name is being substituted."
$ $		Translation Information:
$ $			Do not translate: x_offset
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object file being read.
$ $			%4$d	x_offset from auxiliary symbol
$ $			%5$d	Length of string table

BAD_LABEL	"%1$s: 0711-593 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe symbol refers to a csect with symbol number %5$d, which was not\n\
\tfound. The new symbol cannot be associated with a csect and\n\
\tis being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.
$ $			%5$d	Symbol table number contained in LD symbol

BAD_OVERFLOW		"%1$s: 0711-594 SEVERE ERROR: Overflow section header %2$d in object %3$s:\n\
\tThe section header does not refer to a .text or .data section header."
$ $		Translation Information:
$ $			Do not translate: .text .data
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad section header
$ $			%3$s	Name of file being processed.

BAD_SECTION2		"%1$s: 0711-591 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tA symbol with type XTY_SD must be in a .text or .data section."
$ $		Translation Information:
$ $			Do not translate: .text .data XTY_SD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.

BAD_SNHASH		"%1$s: 0711-253 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tx_snhash=%5$d, which is not the section number of a .typchk section"
$ $		Translation Information:
$ $			Do not translate: x_snhash .typchk
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.
$ $			%5$d	Section number specified by symbol table entry

XCOFF_END_SYMTAB	"%1$s: 0711-589 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n\
\tAuxiliary entries for the symbol are missing."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object file being read.

BAD_STRING_TAB		"%1$s: 0711-590 SEVERE ERROR: Object %2$s cannot be processed.\n\
\tThe length field at the beginning of the string table is invalid."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object being processed.

BAD_SYMBOL_NAME		"%1$s: 0711-251 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n\
\tField n_offset contains %4$d. Valid values are between 4 and %5$d."
$ $		Translation Information:
$ $			Do not translate: n_offset
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object file being read.
$ $			%4$d	Offset to symbol name (n_offset field)

BAD_SYMBOL_NAME2	"%1$s: 0711-255 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n\
\tThe symbol name was not null-terminated. Either the string table is\n\
\tdamaged or field n_offset (which contains %4$d) is invalid."
$ $		Translation Information:
$ $			Do not translate: n_offset
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object file being read.
$ $			%4$d	Offset to symbol name (n_offset field)

BAD_SYMBOL_SCNUM	"%1$s: 0711-563 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe n_scnum field contains %5$d.\n\
\tThe section number must be between 1 and %6$d."
$ $		Translation Information:
$ $			Do not translate: n_scnum
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.
$ $			%5$d	Section number specified in symbol
$ $			%6$d	Number of sections in object.

BAD_SYM_TAB		"%1$s: 0711-598 SEVERE ERROR: Object %2$s cannot be processed.\n\
\tThe XCOFF header contains an invalid value for the offset to the\n\
\tsymbol table or for the number of symbol table entries."
$ $		Translation Information:
$ $			Do not translate: XCOFF
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object being processed.

BAD_TOC2		"%1$s: 0711-865 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tExtra auxiliary entries for the TOC anchor are not being copied to\n\
\tthe output file."
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being processed.

BAD_TOCS	"%1$s: 0711-553 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tA TOC anchor for section %5$d has already been seen.\n\
\tThe symbol is being ignored."
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.
$ $			%5$d	Section number containing multiple TOCs

BAD_TOC_SECTION	"%1$s: 0711-592 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe TOC anchor is not contained in a .text or .data section."
$ $		Translation Information:
$ $			Do not translate: TOC .text .data
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.

CMD_TTY_CANNOT_OPEN	"%1$s: 0711-982 SEVERE ERROR: Cannot open /dev/tty."
$ $		Translation Information:
$ $			Do not translate: /dev/tty
$ $			%1$s	"ld" or "bind"

CMPCT_DUP2	"%1$s: 0711-585 WARNING: Object %2$s: Duplicate symbol deleted: %3$s"
$ $		Technical Information:
$			Lists symbols deleted because of delcsect processing.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object being processed.
$ $			%3$s	Name of symbol being deleted.

CM_ALIGNED_GLUE	"%1$s: 0711-358 WARNING: Alignment of XTY_CM symbol %2$s increased to %3$d."
$ $		Translation Information:
$ $			Do not translate: XTY_CM
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol
$ $			%3$d	New alignment

CM_BUMPED_GLUE	"%1$s: 0711-359 WARNING: Length of XTY_CM symbol %2$s increased to %3$d."
$ $		Translation Information:
$ $			Do not translate: XTY_CM
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol
$ $			%3$d	New length

CM_ALIGNED	"%1$s: 0711-348 WARNING: Alignment of XTY_CM symbol %2$s set to %3$d."
$ $		Translation Information:
$ $			Do not translate: XTY_CM
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol
$ $			%3$d	New alignment

CM_BUMPED	"%1$s: 0711-349 WARNING: Length of XTY_CM symbol %2$s set to %3$d."
$ $		Translation Information:
$ $			Do not translate: XTY_CM
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol
$ $			%3$d	New length

COMMAND_ONCE		"%1$s: 0711-874 WARNING: The binder command %2$s may not be called more than once."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of repeated binder command

ERROR_INTERNAL	"%1$s: 0711-759 INTERNAL ERROR: Source file %2$s, line %3$d.\n\
\tDepending on where this product was acquired, contact your service\n\
\trepresentative or the approved supplier."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Name of source file in error.
$ $			%3$s	Line number of error

ERROR_NOOPEN_LOADMAP	"%1$s: 0711-755 ERROR: Cannot open or create loadmap file: %2$s\n\
\t%1$s:fopen() %3$s"
$ $		Translation Information:
$ $			Do not translate: fopen()
$ $			%1$s	"ld" or "bind"
$ $			%2$d	File being opened
$ $			%3$s	Message from strerror()

IMPEXP_LOADMAP	"%1$s: 0711-756 WARNING: A filename must not be specified when the loadmap option\n\
\tis used in an import or export file. The option is being ignored."
$ $		Cause: loadmap:<filename> used in import or export file.
$ $		Action:
$ $		Technical Information:
$ $		Translation Information:
$ $			Do not translate: loadmap
$ $			%1$s	"ld" or "bind"

EXPORT_DUP	"%1$s: 0711-415 WARNING: Symbol %2$s is already exported."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of exported symbol

EXPORT_IN_USE	"%1$s: 0711-873 WARNING: Symbol %2$s cannot be exported as symbol %3$s.\n\
\tAnother instance of symbol %3$s is already exported."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of exported symbol
$ $			%3$s	Alternate name of exported symbol

BAD_EXPORT	"%1$s: 0711-320 WARNING: Symbol %2$s does not appear to be a\n\
\tprocedure and should not be exported as a system call."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of exported symbol

EXPORT_MOD	"%1$s: 0711-324 WARNING: Export of %2$s changed to export of %3$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of exported symbol as specified
$ $			%3$s	Name of exported symbol as interpreted

FILE_NAMETOOLONG	"%1$s: 0711-878 SEVERE ERROR: The pathname is too long.\n\
\tThe maximum length of a pathname is %2$d.\n\
\tBinder command %3$s cannot be executed."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Maximum length allowed (MAXPATHLEN)
$ $			%3$s	Name of binder subcommand

FILE_QUOTA		"%s: 0711-751 SEVERE ERROR: You have exceeded your disk quota."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"

FILE_SYSTEM_FULL	"%s: 0711-750 SEVERE ERROR: The file system is full."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"

FILE_ULIMIT	"%s: 0711-752 SEVERE ERROR: You have exceeded your filesize ulimit."
$ $		Translation Information:
$ $			Do not translate: ulimit
$ $			%1$s	"ld" or "bind"

GARBAGE_AT_EOF	"%1$s: 0711-597 WARNING: Object %2$s contains extra bytes at the end."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object being processed.

IMPSS_SYMBDEL	"%1$s: 0711-411 WARNING: Additional symbols deleted:"
$ $		Cause:
$ $		Action:
$ $		Technical Information:
$			Lists symbols deleted because of delcsect processing.
$ $		Translation Information:
$ $			This message is followed by a list of symbol names,
$ $				one per line.
$ $			%1$s	"ld" or "bind"

INSRT_RLDBADREF2	"%1$s: 0711-596 SEVERE ERROR: Object %2$s\n\
\tAn RLD for section %3$d (%4$s) refers to symbol %5$d,\n\
\tbut the storage class of the symbol is not C_EXT or C_HIDEXT."
$ $		Translation Information:
$ $			Do not translate: RLD C_EXT C_HIDEXT
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object being processed.
$ $			%3$d	Section number being processed
$ $			%4$s	Name of section being processed.
$ $			%5$d	Number of referred-to symbol.

INST_NOPS_VALID		"%s: The current no-op instructions are:"
$ $		Translation Information:
$ $			%s	"ld" or "bind"
$ $			no-op:	An abbreviation for no-operation

INST_TOCLOADS_VALID	"%s: The current TOC-reload instructions are:"
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			A TOC-reload instruction is an instruction that loads
$ $			a previous value into a machine register (called the
$ $			TOC register.)
$ $			%s	"ld" or "bind"

INST_BAD		"%1$s: 0711-280 SEVERE ERROR: Invalid instruction %2$s\n\
\tInstructions must consist of hex digits only."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Instruction specified.

INST_NOARG		"%s: 0711-282 SEVERE ERROR: Missing argument.\n\
\tSpecify a machine instruction after primary"
$ $		Translation Information:
$ $			Do not translate: primary
$ $				"primary" is a keyword and can be converted to
$ $				upper case if desired
$ $			%1$s	"ld" or "bind"
$ $			%2$X	Instruction specified.

INST_NOARG2		"%s: 0711-283 SEVERE ERROR: Missing argument.\n\
\tSpecify a machine instruction after no"
$ $		Translation Information:
$ $			Do not translate: no
$ $				"no" is a keyword and can be converted to
$ $				upper case if desired
$ $			%1$s	"ld" or "bind"

INST_NOP_NOT_FOUND	"%1$s: 0711-285 SEVERE ERROR: Instruction 0x%2$08x is not a current\n\
\tno-op instruction. It cannot be deleted."
$ $		Translation Information:
$ $			no-op:	An abbreviation for no-operation
$ $			%1$s	"ld" or "bind"
$ $			0x%2$08x	Instruction specified.

INST_NO_DELETE	"%s: 0711-281 SEVERE ERROR: The primary instruction cannot be deleted."
$ $		Translation Information:
$ $			"primary" has the sense of "main" or "principal"
$ $			%s	"ld" or "bind"

INST_TOCLOAD_NOT_FOUND	"%1$s: 0711-284 SEVERE ERROR: Instruction 0x%2$08x is not a current\n\
\tTOC-reload instruction. It cannot be deleted."
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			A TOC-reload instruction is an instruction that loads
$ $			a previous value into a machine register (called the
$ $			TOC register.)
$ $			%1$s	"ld" or "bind"
$ $			0x%2$08x	Instruction specified.

INTERNAL_UNRESOLVED	"%1$s: 0711-342 ERROR: Undefined internal symbol: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol

KEEP_FILE_NOT_INSERTED	"%1$s: 0711-569 WARNING: File %2$s has not been inserted yet."
$ $		Translation Information:
$ $			%1$s 	"ld" or "bind"
$ $			%2$s	File name.

MATCH_BAD_PAT	"%1$s: 0711-677 WARNING: regcomp() failed for pattern: %2$s\n\
\t%3$s"
$ $			Do Not Translate: regcomp()
$ $		Translation Information:
$ $			%1$s 	"ld" or "bind"
$ $			%2$s	Pattern--a character string.
$ $			%3$s	String returned by regerror()

MATCH_NONE1	"%1$s: 0711-678 WARNING: No symbols match pattern: %2$s"
$ $		Technical Information:
$ $			%1$s 	"ld" or "bind"
$ $			%2$s	Pattern--a character string.
$ $		Translation Information:

MATCH_NONE2	"%1$s: 0711-679 WARNING: Symbol %2$s does not exist."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	A symbol name

MULTIPLE_SECTIONS	"%1$s: 0711-586 WARNING: Object %2$s contains multiple %3$s sections.\n\
\tThe additional sections are being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object being processed
$ $			%3$s	".except" or ".debug"

NAME_IN_USE	"%1$s: 0711-277 ERROR: %2$s: New name %3$s is already in use."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Subcommand name (IMPNAME or EXPNAME)
$ $			%3$s	Name of symbol already in use.

NOROOM		"%1$s: 0711-346 ERROR: Replaced XTY_CM symbol %2$s is larger\n\
\tthan the space remaining after the label that replaces it."
$ $		Translation Information:
$ $			Do not translate: XTY_CM
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol being replaced

NOSHELL		"%1$s: 0711-135 SEVERE ERROR: Cannot execute subshell %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Subshell command being attempted.

NO_ENTRY	"%1$s: 0711-327 WARNING: Entry point not found: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"

VALIDATE_BAD_TOC0	"%1$s: 0711-572 WARNING: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe TOC anchor is not named %5$s."
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.
$ $			%5$s	Preferred name of TOC anchor (TOC)

VALIDATE_EXTRA_TYPECHK	"%1$s: 0711-254 WARNING: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tType-checking strings for C_HIDEXT symbols are ignored."
$ $		Translation Information:
$ $			Do not translate: C_HIDEXT
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.

VALIDATE_NULL_STAB	"%1$s: 0711-256 WARNING: Symbol entry %2$d in object %3$s:\n\
\tStabstring symbol has n_zeroes = n_offset = 0.\n\
\tThe symbol is being copied to the output file."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			Do not translate: n_zeroes n_offset
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object file being read.

VALIDATE_NULL_AUX_NAME	"%1$s: 0711-570 WARNING: Symbol table entry %2$d in object %3$s is invalid.\n\
\tFilename in C_FILE auxiliary symbol is null."
$ $		Translation Information:
$ $			Do not translate: C_FILE
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object being processed.

VALIDATE_NULL_FILE_NAME	"%1$s: 0711-571 WARNING: Symbol table entry %2$d in object %3$s is invalid.\n\
\tFilename in C_FILE symbol table entry is null."
$ $		Translation Information:
$ $			Do not translate: C_FILE
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad symbol table entry
$ $			%3$s	Name of object being processed.

VALIDATE_BAD_BSS_RELOC	"%1$s: 0711-576 WARNING: Section %2$d in object %3$s:\n\
\tA .bss section may not have relocation or line number entries.\n\
\tThe entries are being ignored."
$ $		Translation Information:
$ $			Do not translate: .bss
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Number of bad .bss section
$ $			%3$s	Name of object being processed.

VALIDATE_BAD_CM		"%1$s: 0711-573 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tA symbol with type XTY_CM must be in a .bss section and have a\n\
\tstorage-mapping class of XMC_BS, XMC_UC, or XMC_RW; or it must\n\
\tbe in a .data section and have a storage-mapping class of XMC_TD."
$ $		Translation Information:
$ $			Do not translate: XTY_CM .bss XMC_BS XMC_UC
$ $						XMC_RW .data XMC_TD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.

VALIDATE_BAD_LD		"%1$s: 0711-574 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tXTY_LD symbol contains invalid values."
$ $		Translation Information:
$ $			Do not translate: XTY_LD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.

VALIDATE_BAD_SYMBOL	"%1$s: 0711-577 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tExternal reference has invalid fields:\n\
\t\tn_sclass=%5$s; n_smtype=%6$s; n_value=%7$d; n_scnum=%8$d"
$ $		Translation Information:
$ $			Do not translate: n_sclass n_smtype n_value n_scnum
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol
$ $			%3$d	Number of bad symbol table entry.
$ $			%4$s	Name of object being processed.
$ $			%5$s	Supplied symbol class
$ $			%6$s	Supplied storage-mapping type
$ $			%7$d	Supplied symbol address
$ $			%8$d	Supplied section number

VALIDATE_TC0		"%1$s: 0711-575 WARNING: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe symbol table entry for the TOC anchor has bad values."
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol table entry
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.

OBJECT_STRIPPED	"%1$s: 0711-710 ERROR: Input file %2$s\n\
\tis stripped. The file is being ignored."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of stripped file.

MEMBER_STRIPPED	"%1$s: 0711-712 ERROR: Archive member %2$s\n\
\tis stripped. The member is being ignored."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of stripped archive member.

MEMBER_EMPTY	"%1$s: 0711-716 ERROR: Archive member %2$s is empty.\n\
\tThe member is being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of empty archive member.

OBJECT_NO_SECTIONS	"%1$s: 0711-717 WARNING: Object file %2$s\n\
\tdoes not have any sections. The file is being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file with no sections.

MEMBER_NO_SECTIONS	"%1$s: 0711-718 WARNING: Archive member %2$s\n\
\tdoes not have any sections. The member is being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of archive member with no sections.

OBJ_BADOVERFLOW	"%1$s: 0711-719 SEVERE ERROR: Section %2$d in object %3$s\
\tThe overflow section header is missing for this section."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Section number
$ $			%3$s	Name of object being processed

OBJ_BADFILE	"%1$s: 0711-725 ERROR: Object %2$s\n\
\thas no .text, .data, or .bss sections."
$		Translation Information:
$ $			Do not translate: .text .data .bss
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of object being processed

ORIGIN_PAGE_TEXT	"%1$s:\t.text (page boundary) = %2$d (%2$8X hex)"
$ $		Translation Information:
$ $			Do not translate: .text
$ $			"hex" is an abbreviation for "hexadecimal"

ORIGIN_TEXT	"%1$s:\t.text = %2$d (%2$8X hex)"
$ $		Translation Information:
$ $			Do not translate: .text
$ $			"hex" is an abbreviation for "hexadecimal"
$ $			%1$s	Subcommand name (ORIGIN)
$ $			%2$d, %2$8X	Specified origin

ORIGIN_DATA	"\t.data = %1$d (%1$8X hex)"
$ $		Translation Information:
$ $			Do not translate: .data
$ $			"hex" is an abbreviation for "hexadecimal"
$ $			%1$d, %1$8X	Specified origin

ORIGIN_PAGE_DATA	"\t.data (page boundary) = %1$d (%1$8X hex)"
$ $		Translation Information:
$ $			Do not translate: .data
$ $			"hex" is an abbreviation for "hexadecimal"

ORIGIN_PAGE_BAD	"%1$s: 0711-276 ERROR: %2$s: A page origin must be a multiple of the\n\
\tpage size (%3$d)."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Subcommand name (ORIGIN)
$ $			%3$d	Pagesize

PRINT_TOC	"%1$s: The following TOC Symbols were combined:"
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			%1$s	Subcommand name (RESOLVE)

RESOLVE_AGAIN	"%1$s: 0711-347 WARNING: RESOLVE already called and no new symbols\n\
\thave been inserted, exported, or kept."
$ $		Translation Information:
$ $			Do not translate: RESOLVE
$ $			%1$s	"ld" or "bind"

RESOLVE_NEEDED	"%1$s: 0711-300 ERROR: RESOLVE must be called before calling binder command %2$s."
$ $		Translation Information:
$ $			Do not translate: RESOLVE
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Binder command being attempted

RESOLVE_WARNING	"%1$s: 0711-329 WARNING: Binder command RESOLVE called more than once.\n\
\tMessages may differ from previous call."
$ $		Translation Information:
$ $			Do not translate: RESOLVE
$ $			%1$s	"ld" or "bind"

SD_SMALL	"%1$s: 0711-326 ERROR: Replaced XTY_CM symbol %2$s is larger\n\
\tthan the symbol that replaces it."
$ $		Translation Information:
$ $			Do not translate: XTY_CM
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol

SECTION_MISPLACED	"%1$s: 0711-599 SEVERE ERROR: Section header %2$d (%3$s) in object %4$s:\n\
\tFields in the header are invalid: s_scnptr = %5$d; s_size = %6$d"
$ $		Translation Information:
$ $			Do not translate: s_scnptr s_size
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad section header
$ $			%3$s	Name of bad section
$ $			%4$s	Name of object being processed.
$ $			%5$d	Supplied section pointer
$ $			%6$d	Supplied section size

SMC_CHANGE1		"%1$s: 0711-321 WARNING: Storage-mapping class of symbol %2$s\n\
\tchanged from XMC_GL to XMC_PR because of a reference to the symbol."
$ $		Translation Information:
$ $			Do not translate: XMC_GL XMC_PR
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Symbol name

SMC_CHANGE2	"%1$s: 0711-323 WARNING: Storage-mapping class of symbol %2$s\n\
\tchanged from XMC_UA to XMC_%3$s because of references."
$ $		Translation Information:
$ $			Do not translate: XMC_UA XMC_
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Symbol name
$ $			%3$s	Storage mapping class

STAB_NO_STATS		"%s: STABSTRING statistics:  No stabstrings."
$ $		Translation Information: DO NOT TRANSLATE

STAB_STATS_HDR		"%s: STABSTRING statistics:"
$ $		Translation Information: DO NOT TRANSLATE

STAB_ERROR1	"%1$s: 0711-382 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tUnexpected character in production %4$s."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$s	Production name

STAB_ERROR2	"%1$s: 0711-383 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tColon (:) marking beginning of stabstring not found."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_ERROR3	"%1$s: 0711-384 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tName expected."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_ERROR4	"%1$s: 0711-385 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tInteger expected."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_ERROR4A	"%1$s: 0711-381 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tReal expected."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_ERROR5	"%1$s: 0711-386 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tType number expected."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_ERROR6	"%1$s: 0711-387 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tSingle or double quote expected."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_ERROR7	"%1$s: 0711-388 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tUnexpected character after %4$s in %5$s."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$s	A single character: last legal character seen
$ $			%5$s	Production name

STAB_ERROR8	"%1$s: 0711-389 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tUnexpected end of stabstring in production %4$s."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$s	Production name

STAB_ERROR9	"%1$s: 0711-390 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tSyntax error after %4$s in production %5$s."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$s	A single character: last legal character seen
$ $			%5$s	Production name

STAB_ERROR10	"%1$s: 0711-391 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tExtra characters at end of stabstring."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_ERROR11	"%1$s: 0711-392 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tStabstring redefines predefined type %5$.*4$s."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%5$.*4$s Redefined type number.

STAB_ERROR12	"%1$s: 0711-393 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tA stabstring refers to type %4$d, which is not defined."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$d	Undefined type number.

STAB_ERROR13	"%1$s: 0711-394 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tClosing %4$s not found."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$s	Closing quote character: ' or "

STAB_ERROR14	"%1$s: 0711-397 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tCharacter %4$s expected in production %5$s."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$s	Expected character
$ $			%5$s	Production name

STAB_ERROR15	"%1$s: 0711-398 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tExponent expected."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_ERROR16	"%1$s: 0711-399 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tLength expected in production %4$s."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$s	Production name

STAB_ERROR17	"%1$s: 0711-379 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tExpected character %4$s not found before end of stabstring."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$s	Production name

STAB_BAD_CONTINUATION	"%1$s: 0711-378 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tA continued stabstring is at the end of the symbol table."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_REDEF	"%1$s: 0711-376 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tType %4$d has new definition replacing previous unnamed definition."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Name of object containing bad stabstring.
$ $			%4$d	Number of redefined stabstring

STAB_MULTIPLE	"%1$s: 0711-377 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tType %4$d has multiple definitions."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Name of object containing bad stabstring.
$ $			%4$d	Number of redefined stabstring

STAB_BAD_LEN2	"%1$s: 0711-380 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n\
\tLength of stabstring in .debug section is invalid.\n\
\tThe stabstring is being deleted."
$ $		Translation Information:
$ $			Do not translate: .debug
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.

STAB_BAD_MAP	"%1$s: 0711-396 INTERNAL ERROR: Symbol table entry %2$d, object file %3$s\n\
\tConflict in local mapping: old %4$d; new %5$d."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of symbol table entry in error.
$ $			%3$s	Number of object containing bad stabstring.
$ $			%4$d	An integer
$ $			%5$d	An integer

STAB_CHARACTER	"\tCharacter %c seen."
$ $		Translation Information:
$ $			This message always follows one of the "STABSTRING
$ $			ERROR" messages above.  It is used to display the next
$ $			character in the stabstring if the character is
$ $			printable.
$ $			%c	Next character in stabstring

STAB_ASCII	"\tASCII %d seen."
$ $		Translation Information:
$ $			Do not translate: ASCII
$ $			This message always follows one of the "STABSTRING
$ $			ERROR" messages above.  It is used to display the next
$ $			character in the stabstring if the character is
$ $			not printable.
$ $			%d	ASCII value of next character in stabstring

STAB_END	"\tEnd of stabstring reached."
$ $		Translation Information:
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			This message always follows one of the "STABSTRING
$ $			ERROR" messages above.  It is used when another
$ $			character is expected, but none was found.

STAB_REMAINDER	"\tRemainder of stabstring is:\n%s"
$ $			"STABSTRING" is a technical term, so it probably cannot
$ $				be translated.
$ $			This message always follows one of the "STABSTRING
$ $			ERROR" messages above.  It is used when parsing of
$ $			a stabstring is complete, but more characters were
$ $			found in the string.
$ $			%s	Remainder of stabstring

SYMBOL_TABLE_OVERFLOW	"%s: 0711-866 INTERNAL ERROR: Output symbol table size miscalculated."
$ $		Translation Information:
$ $			%s	"ld" or "bind"

TYPECHK_NO_STATS	"%s: Type-checking string statistics: No type-checking strings."
$ $		Translation Information: DO NOT TRANSLATE

TYPECHK_STATS_HDR	"%s: Type-checking string statistics:"
$ $		Translation Information: DO NOT TRANSLATE

TYPECHK_ERROR1	"%1$s: 0711-955 SEVERE ERROR: Length of type-checking string is invalid.\n\
\tThe string appears to extend beyond the end of the section."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"

USAGE_ALIGN	"%1$s: 0711-610 USAGE: ALIGN [pattern] {0-31}"
$		Translation Information:
$ $			Do Not Translate: ALIGN
$ $			%1$s	"ld" or "bind"

USAGE_ADDNAMEDGLUE	"%1$s: 0711-375 USAGE: ADDNAMEDGLUE fn pattern ..."
$ $		Translation Information:
$ $			Do Not Translate: ADDNAMEDGLUE
$ $			%1$s	"ld" or "bind"

USAGE_GLUE		"%1$s: 0711-372 USAGE: ADDGL fn"
$ $		Translation Information:
$ $			Do Not Translate: ADDGL
$ $			%1$s	"ld" or "bind"

USAGE_INSERT		"%1$s: 0711-532 USAGE: {I|INSERT} [KEEP] fn"
$ $		Translation Information:
$ $			Do Not Translate: I|INSERT KEEP
$ $			%1$s	"ld" or "bind"

FILE_UNEXPECTED_ERROR	"%1$s: 0711-753 SEVERE ERROR: Unexpected I/O error\n\
\t%1$s:%2$s() %3$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	System call that failed
$ $			%3$s	Result from strerror()

UNEXPECTED_EXCEPTION	"%1$s: 0711-997 SEVERE ERROR: Unexpected exception %2$d caught.\n\
\t%3$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Exception number
$ $			%3$s	Result from strerror()

BAD_SYSCALL	"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n\
\t%1$s:%2$s() %3$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of failing syscall
$ $			%3$s	Result from strerror()

READ_EOF	"%1$s: 0711-983 SEVERE ERROR: File %2$s\n\
\tPremature end-of-file reached while reading %3$d bytes."
$ $		Cause:	An fread() call couldn't read the number of bytes
$ $			requested.  This is the result of a corrupted offset
$ $			value in an input file or the result of a file being
$ $			truncated.
$ $		Action:	Processing of the file cannot continue.  The invalid
$ $			file should be examined.
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file being read.
$ $			%3$d	Number of bytes requested.

WHILE_READING_KEYBOARD	"%s: 0711-985 Error occurred while reading binder commands from the keyboard."
$ $		Cause:
$ $		Action:
$ $		Technical Information:
$ $			An error other then EINTR or EIO was returned by getc()
$ $		Translation Information:
$ $			%s	"ld" or "bind"

WHILE_READING_LEN	"%1$s: 0711-986 Error occurred while reading %2$d bytes from file: %3$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bytes being read from file.
$ $			%3$s	Name of file being read.

WHILE_READING		"%1$s: 0711-987 Error occurred while reading file: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file being read.

WHILE_READING_FILE	"%1$s: 0711-988 Error occurred while reading binder commands from file: %2$s"
$ $		Technical Information:
$ $			An error other then EINTR or EIO was returned by getc()
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of command file being read.

WHILE_READING_STDIN	"%s: 0711-989 Error occurred while reading binder commands from standard input."
$ $		Cause:
$ $		Action:
$ $		Technical Information:
$ $			An error other then EINTR or EIO was returned by getc()
$ $		Translation Information:
$ $			%s	"ld" or "bind"

WHILE_CLOSING_LOADMAP	"%1$s: 0711-990 Error occurred while closing loadmap file: %2$s"
$ $		Cause:	close() return non-zero
$ $		Action:
$ $		Technical Information:
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file being closed.

WHILE_WRITING_LOADMAP	"%1$s: 0711-991 Error occurred while writing to the loadmap file: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of loadmap file

WHILE_WR_STDOUT_LOADMAP	"%s: 0711-992 Error occurred while writing to standard output."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"

WHILE_WRITING_OUTPUT	"%1$s: 0711-993 Error occurred while writing to the output file: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of output file.

WHILE_WRITING_TEMP	"%1$s: 0711-994 Error occurred while writing to the temporary output file: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of temporary output file.

WHILE_PROCESSING_ADDR	"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file being processed.
$ $			%3$X	Address in file of erroneous operation

WHILE_PROCESSING	"%1$s: 0711-996 Error occurred while processing file: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of file being processed.

WHILE_CLOSING	"%1$s: 0711-997 Error occurred while closing file: %2$s"
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of output file

SYMBOLS_NOT_READ	"%s: 0711-416 Some symbols were not read."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"

HASH_INFO		"\tBuckets used=%1$d of %2$d Maximum-bucket-size=%3$d Strings=%4$d\n\
\tActual distribution=%5$.1f Random distribution=%6$.1f Ratio=%7$.3f\n\
\tBucket sizes: "
$ $		Translation Information: DO NOT TRANSLATE
$ $			%1$d	Integer
$ $			%2$d	Integer
$ $			%3$d	Integer
$ $			%4$d	Integer
$ $			%5$.1f	Floating number
$ $			%6$.1f	Floating number
$ $			%7$.3fd	Floating number

DBOPT1_ORDERING		"CSECT ordering:"
$ $		Translation Information: DO NOT TRANSLATE.

DBOPT1_DEFERRED_IMPORTS	"Deferred Imports (from file %s)"
$ $		Translation Information: DO NOT TRANSLATE.

DBOPT1_IMPORTS		"Imported symbols in %1$s (from file %2$s)"
$ $		Translation Information: DO NOT TRANSLATE.

DBOPT1_ADDRESSES	"\tOld address: %1$.8x New: %2$.8x "
$ $		Translation Information: DO NOT TRANSLATE.

DBOPT1_WITH_LABEL	"with label "
$ $		Translation Information: DO NOT TRANSLATE.

STATS_TOTAL1	"\nSymbols: Generated %1$d\tERs %2$d\tTotal %3$d"
$ $		Translation Information: DO NOT TRANSLATE

STATS_HEADER	"\t\t\
     Allocations Allocated    In-Use   Maximum"
$ 12         3         4         5         6
$ 90123456789012345678901234567890123456789012
$ $		Translation Information: DO NOT TRANSLATE
$ $	    Each translated word must END in the same column as the
$ $		corresponding English word.
$ $	    Numbers show column positions.

STATS_HEADER2	"\n\t\t\
          In-Use   Maximum     Total    Undone"
$ 12         3         4         5         6
$ 90123456789012345678901234567890123456789012
$ $		Translation Information: DO NOT TRANSLATE
$ $	    Each translated word must END in the same column as the
$ $		corresponding English word.
$ $	    Numbers show column positions.

LIT_FILES	"Files"
$ $		Translation Information:
$ $			Used in a table to label a row containing counts for
$ $			certain file-system files.

LIT_SHMAT_FILES	"Mapped with shmat()"
$ $		Translation Information:
$ $			Do not translate: shmat()
$ $			Used in a table to label a row containing counts for
$ $			certain file-system files.

LIT_MMAP_FILES	"Mapped with mmap()"
$ $		Translation Information:
$ $			Do not translate: mmap()
$ $			Used in a table to label a row containing counts for
$ $			certain file-system files.

LIT_UNKNOWN_COMPOSITE	"UNKNOWN_COMPOSITE(%s[%s])"
$ $		Translation Information: DO NOT TRANSLATE.

LIT_REFERENCES		"** References **"
$ $		Translation Information:
$ $			Used in a message at the beginning of a list of
$ $			references to a symbol with a given name.

LIT_OTHER_REFS		"** References Without Matching Definitions **"
$ $		Translation Information:
$ $			Used in a message at the beginning of a list of
$ $			references to a symbol with a given name, where the
$ $			references have type-checking strings that do not match
$ $			the type-checking string of any definition.

LIT_NO_HASH		"** No Hash **"
$ $		Translation Information:
$ $			Used in a message to indicate that a symbol has no
$ $			associated type-checking hash string information

LIT_UNDEFINED		"** Undefined Symbol **"
$ $		Translation Information:
$ $			Used in a message after a symbol name to indicate that
$ $			the symbol is used but not defined.

LIT_DUPLICATE		"** Duplicate **"
$ $		Translation Information:
$ $			Used in a message to indicate that one symbol is
$ $			a duplicate of another symbol with the same name.

LIT_NO_FILE	"<no file>"
$ $		Translation Information:
$ $			Used in a message listing information about a symbol
$ $			when the symbol has no associated source file.

LIT_VALUE	" Value = 0x%08x%*s"
$ $		Translation Information:
$ $			Used in a message to list the value of a symbol. The
$ $			value is printed as a hexadecimal number
$ $			%08x	The value

LIT_EXPORT	"E"
$ $		Translation Information:
$ $			The letter "E" stands for "exported".  The translation
$ $			must be different from the translation for LIT_IMPORT
$ $			(the next message). The translation
$ $			must occupy a single display column.

LIT_IMPORT	"I"
$ $		Translation Information:
$ $			The letter "I" stands for "imported".  The translation
$ $			must be different from the translation for LIT_EXPORT
$ $			(the previous message). The translation
$ $			must occupy a single display column.

DUMPLIT_GENERATED	"GENERATED"
$ $		Translation Information:
$ $			Appears as a single word in some diagnostic output to
$ $			designate an generated symbol.

DUMPLIT_IMPORT	"IMPORT"
$ $		Translation Information:
$ $			Appears as a single word in some diagnostic output to
$ $			designate an imported symbol.

DUMPLIT_ARCHIVE	"ARCHIVE"
$ $		Translation Information:
$ $			Appears as a single word in some diagnostic output to
$ $			designate a symbol read from an archive.

DUMPLIT_FIXUP	"FIXUP"
$ $		Translation Information:
$ $			Appears as a single word in some diagnostic output to
$ $			designate a symbol generated in a csect containing
$ $			fix-up code.

LIT_LANG_PLI	"PL/I"
$ $		Translation Information:
$ $			PL/I is the name of the PL/I programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_RPG	"RPG "
$ $		Translation Information:
$ $			RPG is the name of the RPG programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_MOD2	"Mod2"
$ $		Translation Information:
$ $			Mod2 is the name of the Modula 2 programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_PASC	"Pasc"
$ $		Translation Information:
$ $			Pasc is the name of the Pascal programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_LISP	"Lisp"
$ $		Translation Information:
$ $			Lisp is the name of the Lisp programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_FORT	"Fort"
$ $		Translation Information:
$ $			Fortran is the name of the Fortran programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_C	" C  "
$ $		Translation Information:
$ $			C is the name of the C programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_BASIC	"BASC"
$ $		Translation Information:
$ $			BASC is the name of the BASIC programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_COBOL	"Cobl"
$ $		Translation Information:
$ $			Cobl is the name of the COBOL programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_CPLUS	"C++ "
$ $		Translation Information:
$ $			C++ is the name of the C++ programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_ADA	"Ada "
$ $		Translation Information:
$ $			Ada is the name of the ADA programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LIT_LANG_PL8	"PL8 "
$ $		Translation Information:
$ $			PL8 is the name of the PL.8 programming language
$ $			Display width of translated string must be 4 columns
$ $				(pad with blanks or truncate if necessary).

LOC_SYM_IMPORTED	"%1$s: 0711-773 WARNING: Object %2$s, imported symbol %3$s\n\
\tSymbol was expected to be local. Extra instructions\n\
\tare being generated to reference the symbol."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Object file containing reference.
$ $			%3$s	Symbol name

TOCDATA_REF		"%1$s: 0711-774 WARNING: Symbol %2$s\n\
\tStorage-mapping class of symbol changed to XMC_TD because at least\n\
\tone reference expects the symbol to be in the TOC."
$ $		Translation Information:
$ $			Do not translate: XMC_TD TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name

TOCDATA_NOREF		"%1$s: 0711-775 WARNING: Symbol %2$s\n\
\tStorage-mapping class of symbol changed to XMC_RW because no\n\
\treferences expect the symbol to be in the TOC."
$ $		Translation Information:
$ $			Do not translate: XMC_RW TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name

IMPNAME_BAD	"%1$s: 0711-417 ERROR: Import object %2$s at line %3$d:\n\
\tThe pathname or member name specified after #! is too long.\n\
\tThe line is being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input object name
$ $			%3$d	Line number of bad line

SHARED_IMPORT_ADDRESS	"%1$s: Symbol imported (at address 0x%2$08X): %3$s"
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			0x%2$08X	Address of imported symbol
$ $			%3$s	Name of imported symbol

IMPORT_BAD_ADDRESS	"%1$s: 0711-418 ERROR: Import or export file %2$s at line %3$d:\n\
\tA symbol name may only be followed by an address or by the keyword\n\
\tsvc or syscall. The line is being ignored."
$		Translation Information:
$ $			Do not translate: svc syscall
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of import file
$ $			%3$d	Line number of bad line

IMPORT_SYNTAX	"%1$s: 0711-419 The symbol address %2$s is not a valid number."
$ $			%1$s	"ld" or "bind"
$ $			%3$d	Symbol address

IMPORT_MULTIPLE	"%1$s: 0711-413 ERROR: Import or export file %2$s at line %3$d:\n\
\tOnly one symbol name per line is allowed. The symbol name may be\n\
\tfollowed by svc, syscall, or an address. The line is being ignored."
$		Translation Information:
$ $			Do not translate: svc syscall
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of import file
$ $			%3$d	Line number of bad line

ROTEXT_HEAD1	"\
 CSECT or (Symbol in CSECT)%s   Inpndx  Address  TY CL Source-File(Object-File)\n\
                .loader section RLDs:%s Address  TY CL Rld-type%s Inpndx  Name\n\
 ----------------------------%s-------- -------- -- -- ------------------------%s"
$ $      1         2           3         4         5         6         7
$ 3456789012345678901234567  89012345678901234567890123456789012345678901234567
$ $		Translation Information:
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			The column heading ".loader section RLDs:" should END
$ $				in the same column as the English heading.
$ $			Do not tranlate .loader.
$ $			All other translated column headings should start in the same
$ $				position as the corresponding English heading.
$ $			The first column heading is "CSECT or (Symbol in CSECT)".
$ $			The last column heading is "Source-File(Object-File)".
$ $			For all other columns, the heading is a single word without blanks.
$ $			The underlining for the last column should extend as far as the
$ $				last character in either of the first two lines.
$ $			%s: Leave all instances in place.  Assume that they will
$ $				be replaced by null strings.
$ $		    The column headings mean:
$ $			CSECT or (Symbol in CSECT): A symbol name
$ $			Inpndx: Input symbol-table index
$ $			Address: 8-digit hex address
$ $			TY: Symbol Type
$ $			CL: Storage-mapping Class
$ $			Source-File ...: Path name of input object
$ $			Load-time RLDs: Relocation entries that will have to be
$ $				relocated at load-time.  Do not translate RLD.
$ $				RLDs is the plural of RLD.

ROTEXT_HEAD2	"\
 CSECT or (Symbol in CSECT)%s   Inpndx  Address  TY CL Source-File(Object-File)\n\
%s Symbols referenced with .loader section RLDs: TY CL %sInpndx  Name\n\
 ----------------------------%s----------------- -- -- ------------------------"
$ $      1         2           3         4         5         6         7
$ 3456789012345678901234567  89012345678901234567890123456789012345678901234567
$ $		Translation Information:
$ $		    Preserve alignment of columns (numbers show column positions).
$ $			The column heading
$ $				"Symbols referenced with .loader section RLDs:"
$ $				should END in the same column as the English
$ $				heading.
$ $			Do not translate: .loader
$ $			All other translated column headings should start in the same
$ $				position as the corresponding English heading.
$ $			The first column heading is "CSECT or (Symbol in CSECT)".
$ $			The last column heading is "Source-File(Object-File)".
$ $			For all other columns, the heading is a single word without blanks.
$ $			The underlining for the last column should extend as far as the
$ $				last character in either of the first two lines.
$ $			%s: Leave all instances in place.  Assume that they will
$ $				be replaced by null strings.
$ $		    The column headings mean:
$ $			CSECT or (Symbol in CSECT): A symbol name
$ $			Inpndx: Input symbol-table index
$ $			Address: 8-digit hex address
$ $			TY: Symbol Type
$ $			CL: Storage-mapping Class
$ $			Source-File ...: Path name of input object
$ $			Symbols referenced with load-time RLDs: Names of symbols
$ $				with corresponding relocation entries that will
$ $				have to be relocated at load-time.
$ $				Do not translate RLD. RLDs is the plural of RLD.

ROTEXT_WARN1	"%1$s: 0711-302 ERROR: Object %2$s, csect %3$s%4$s%5$s\n\
\tThe csect is part of the .text section, and relocation entries\n\
\tfrom the csect have been written to the .loader section."
$		Translation Information:
$ $			Do not translate: csect .loader
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Object file name
$ $			%3$s	"<" or ""
$ $			%4$s	Symbol name
$ $			%5$s	">" or ""

ROTEXT_WARN2	"%1$s: 0711-303 ERROR: Object %2$s, csect with label (%3$s)\n\
\tThe csect is part of the .text section, and relocation entries\n\
\tfrom the csect have been written to the .loader section."
$		Translation Information:
$ $			Do not translate: csect .loader
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Object file name
$ $			%3$s	Symbol name

BAD_OVERFLOW2		"%1$s: 0711-588 WARNING: Section header %2$d in object %3$s:\n\
\tFields in the section header are inconsistent with fields\n\
\tin the corresponding overflow section header %4$d."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad section header
$ $			%3$s	Name of file being processed.
$ $			%4$d	Number of bad overflow section header.

EXTRA_OVERFLOW		"%1$s: 0711-584 WARNING: Section header %2$d in object %3$s:\n\
\tAn overflow section header has already been found for section %4$d.\n\
\tThe extra overflow header is being ignored."
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$d	Number of bad section header
$ $			%3$s	Name of file being processed.
$ $			%4$d	Number of primary section header.

NO_TOC_ANCHOR	"%1$s: 0711-325 SEVERE ERROR: Symbol %2$s (entry %3$d) in section %4$d\n\
\tin object %5$s: There is a TOC-relative\n\
\treference to the symbol, but the section contains no TOC anchor."
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol
$ $			%3$d	Number of bad symbol
$ $			%4$d	Section number of bad symbol
$ $			%5$s	Name of object file being read.

NO_TOC_ANCHOR2	"%1$s: 0711-337 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThere is a TOC-relative reference to the symbol,\n\
\tbut the object contains no TOC anchor."
$ $		Translation Information:
$ $			Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol
$ $			%3$d	Number of bad symbol
$ $			%4$s	Name of object file being read.

FIXUP_BAD2 "%1$s: 0711-776 SEVERE ERROR: Object %2$s, RLD address 0x%3$x in section %4$d:\n\
\tThe generated instructions (at address 0x%5$s) for the fixed-up\n\
\tinstruction at address 0x%6$x are not recognized.\n\
\tThe instruction will not be restored or relocated."
$ $		Translation Information:
$			Do not translate: RLD
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Input object in error
$ $			%3$x	Address of bad RLD
$ $			%4$d	Section number of erroneous section
$ $			%5$x	Address of bad instruction fixup instructions
$ $			%6$x	Address of branch to fixup instructions

SMCLASS_UPDATE	"%1$s: The storage-mapping class of %2$s was %3$s."
$ $		Translation Information:
$ $			%1$s	Subcommand name ("stg")
$ $			%2$s	Symbol name
$ $			%3$s	Previous storage-mapping class (two characters)

SMCLASS2_UPDATE	"%1$s: The storage-mapping class of csect [%2$s]%3$s has been updated."
$ $		Translation Information:
$ $			%1$s	Subcommand name ("stg")
$ $			%2$s	Symbol number
$ $			%3$s	Csect name

SMCLASS3_UPDATE	"%1$s: The storage-mapping class of label [%2$s]%3$s has been updated."
$ $			Do not translate: csect
$ $		Translation Information:
$ $			%1$s	Subcommand name ("stg")
$ $			%2$s	Symbol number
$ $			%3$s	Label name

BAD_SMCLASS	"%1$s: 0711-614 SEVERE ERROR: Invalid storage-mapping class %2$s specified.\n\
\tValid storage-mapping classes are\n\t"
$ $		Translation Information:
$ $		Do not delete final newline and tab "\n\t".
$ $		This message is followed by a list of valid storage-mapping
$ $		classes, which are printed literally and must not be
$ $		translated.
$ $			%1$s	Subcommand name ("stg")
$ $			%2$s	Specified storage-mapping class

BAD_EXT_TOCREF	"%1$s: 0711-338 SEVERE ERROR: Symbol %2$s in object %3$s:\n\
\tThere is a TOC-relative reference to this global symbol\n\
\tbut its storage-mapping class is not XMC_TD or XMC_TC."
$ $		Translation Information:
$ $		Do not translate: XMC_TD XMC_TC TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name
$ $			%3$s	Input object in error

BAD_HIDEXT_TOC_REF	"%1$s: 0711-339 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThere is a TOC-relative reference to this internal symbol,\n\
\tbut its storage-mapping class is not XMC_TD or XMC_TC."
$ $		Translation Information:
$ $		Do not translate: XMC_TC TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name
$ $			%3$d	Symbol table index
$ $			%4$s	Input object in error

LABELED_TD	"%1$s: 0711-340 SEVERE ERROR Symbol %2$s in object %3$s\n\
\tA csect with multiple labels cannot be moved into the TOC."
$ $		Translation Information:
$ $		This message is followed by a list of valid storage-mapping
$ $		classes, which are printed literally and must not be
$ $		translated.
$ $		Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name
$ $			%3$s	Input object in error

TD_RESERVED	"%1$s: 0711-777 SEVERE ERROR Symbol %2$s\n\
\tThe symbol has a reserved name and cannot be mapped into the TOC."
$ $		Translation Information:
$ $		Do not translate: TOC
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name

IMPSS_SYMBDEL2	"%1$s: 0711-579 WARNING: Additional symbol deleted: %2$s"
$ $		Cause:
$ $		Action:
$ $		Technical Information:
$			Lists symbols deleted because of delcsect processing.
$ $		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of deleted symbol

UNRES_MSG2	"%1$s: 0711-305 ERROR: Undefined symbols were found. Symbols marked\n\
\twith %% were deleted because the -bdelcsect option was specified.\n\
\tThe following symbols are in error:"
$ $		Translation Information:
$ $			This is a message that is printed at the top of
$ $			a list of duplicate symbols.  This message is the same
$ $			as message UNRES_MSG (0711-318) except that an
$ $			additional sentence has been added.
$ $			%1$s	"ld" or "bind"
$ $			%%	%

UNRESOLVE_MSG	"%1$s: 0711-304 ERROR: Undefined symbol: %2$s\n\
\t(deleted because the -bdelcsect option was specified)"
$ $		Translation Information:
$ $			This message is the same as UNRESOLVE_HEADING (0711-317)
$ $			except that an additional clause has been added.
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Symbol name

CMPCT_DUP	"%1$s: 0711-578 WARNING: Duplicate symbols deleted:"
$ $		Technical Information:
$			Lists symbols deleted because of delcsect processing.
$ $		Translation Information:
$ $			This message is always followed by the name of the
$ $				symbol being deleted.  The name will be printed
$ $				either on the same line as this message or on
$ $				the following line.
$ $			%1$s	"ld" or "bind"

BAD_SYM_ADDR	"%1$s: 0711-583 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe symbol is not entirely contained within its section."
$		Translation Information:
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of symbol making bad reference
$ $			%3$d	Symbol table index of above symbol
$ $			%4$s	Input file in error

BAD_SNINFO		"%1$s: 0711-601 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe n_scnum field of the symbol contains %5$d, which is not\n\
\tthe section number of a .info section. The symbol is being deleted."
$ $		Translation Information:
$ $			Do not translate: n_scnum .info
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.
$ $			%5$d	Section number specified by symbol table entry

BAD_INFO_VAL		"%1$s: 0711-602 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe value of the symbol is not a valid offset into .info section %5$d.\n\
\tThe symbol is being deleted."
$ $		Translation Information:
$ $			Do not translate: .info
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.
$ $			%5$d	Section number specified by symbol table entry

BAD_INFO_LEN		"%1$s: 0711-603 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n\
\tThe length of the .info string referenced by the symbol is invalid.\n\
\tThe string appears to extend beyond the end of .info section %5$d.\n\
\tThe symbol is being deleted."
$ $		Translation Information:
$ $			Do not translate: .info
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of bad symbol
$ $			%3$d	Number of bad symbol table entry
$ $			%4$s	Name of object file being read.
$ $			%5$d	Section number specified by symbol table entry

SAVE_NOFCHMOD		"%1$s: 0711-856 ERROR: Cannot set the mode of the output file: %2$s\n\
\t%1$s:fchmod() %3$s"
$ $		Translation Information:
$ $			Do not translate: .info
$ $			%1$s	"ld" or "bind"
$ $			%2$s	Name of output file.
$ $			%3$s	Message from strerror()
