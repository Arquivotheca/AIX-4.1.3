$ @(#)89	1.15.2.1  src/bos/usr/bin/ex/ex.msg, cmdedit, bos41J, 9515B_all 3/31/95 16:13:20
$
$   COMPONENT_NAME: CMDEDIT
$
$   FUNCTIONS: none
$
$   ORIGINS: 3,10,13,26,27
$
$   This module contains IBM CONFIDENTIAL code. -- (IBM
$   Confidential Restricted when combined with the aggregated
$   modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1984,1995
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$


$len 
$set MS_EX
$quote "

$ Some messages have a @ or | character in them. This controls how much of 
$ that message will be shown. Ex, vi, view, vedit, edit, and tvi (the editors 
$ that use this message catalog) have a "terse" option to control how much 
$ information the user gets: 
$ The space should remain 
$       original    how it's shown      how it's shown 
$       message:     if terse:          if not terse: 
$       ________    _______________     ______________ 
$       xxx@yyy         xxx               xxx yyy  (the @ becomes a space) 
$       xxx|yyy         xxx               yyy 
$  M_499 through M_506 are used only with tvi. Other messages in this 
$       message catalog not listed below are also used by tvi which is also 
$       known as the trusted editor. 
$  M_600 through M_605 are the only messages used by ctags, a program used to 
$       create a tags file for use by vi, view, vedit, edit, and ex. 
$  M_258 through M_267 are the only messages used by expreserve, a program used 
$       to preserve files during an editing session or system failure. 
$  M_268 thorough M_276, M_278, M_287, and M_288 are the only messages used by 
$       exrecover, a program used to recover files saved by expreserve. 
$       Some messages begin with a space. Please preserve the space in the 
$       translation.
$       Do not translate %s, %c, %d, etc. These will be replaced by file names,
$       numbers, etc.
$       The term "regular expression" is a Unix technical term.
$
M_000 "ex: 0602-000 %s is not a recognized flag.\n"
M_002 " Address_1 > address_2.|ex: 0602-001 The first address cannot exceed the second address."
M_003 " Count is not correct.|ex: 0602-002 The specified count must be greater than 0."
M_004 "ex: 0602-003 An address is not allowed@with the specified command."
M_005 "ex: 0602-004 The specified address is not valid."
M_006 " No match to end of file.|ex: 0602-005 The address search reached the end of the file without matching the specified pattern."
M_007 " No match to top of file.|ex: 0602-006 The address search reached the beginning of the file without matching the specified pattern."
M_008 " Not found|ex: 0602-007 The pattern does not exist."
M_009 "ex: 0602-008 A mark must be the ' character or a single character in the range a through z."
M_010 "ex: 0602-009 The specified mark is not defined@and cannot be referenced."
M_011 "ex: 0602-010 An address cannot be less than 0.@The first buffer line is 1."
M_012 "ex: 0602-011 Not that many lines@exist in the file."
M_013 "ex: 0602-012 The address offset after the command is too large."
M_015 "ex: 0602-013 Cannot get the value of the HOME environment variable."
M_016 " No parameter.|ex: 0602-014 The %s command must be followed by a letter."
M_017 " Parameter not correct.|ex: 0602-015 The parameter to the mark command must be a lower case letter."
M_018 "ex: 0602-016 The preserve command failed."
$       Do not translate preserve.
M_019 " The file is preserved."
M_020 "ex: 0602-017 No write subcommand@since the file was last modified.  Use :rew! to force the action."
$       Do not translate rew!
M_021 "Old tty driver|Not using new tty driver/shell."
$       Not part of present executable. Do not translate message. 
M_022 "ex: 0602-018 Buffer names must be a single character, a through z or A through Z"
M_023 " Current line is EOF.|ex: 0602-019 The current line is at the end of the file."
M_024 " Command not known.|ex: 0602-020 %c is not a known command character."
M_025 " Extra characters.|ex: 0602-021 There are extra characters at the end of the command."
M_026 " [Warning - %s is incomplete]"
M_027 " %d files@to edit."
M_028 " No more files@to edit."
M_029 " Extra characters.|ex: 0602-022 There are extra characters at the end of the %s command."
M_032 "ex: 0602-023 No write@since the file was last changed.  The %s! command will force the action."
$       Leave the ! character after the %s. 
M_033 " No such open or visual command|ex: 0602-024 %S is not a recognized command while in open or visual mode."
$       Do not translate open or visual. 
M_034 " No such command|ex: 0602-025 %S is not a recognized command."
M_035 " [Press return to continue] "
$       Leave the space at the end. 
M_036 "ex: 0602-026 No more memory@for the append. Too many lines in the file."
BAD_1 " Line too long.|ex: 0602-027 The result of a line join would be greater than 1024 characters."
M_038 " No address for %s.|ex: 0602-028 The %s command requires a destination address."
M_039 "ex: 0602-029 The specified move command would have no effect."
M_040 "ex: 0602-030 The destination address of a move command cannot be within the source address."
M_041 "ex: 0602-031 Cannot use the put command while inside a global command or macro."
M_042 " Line too long.|ex: 0602-032 A shift cannot create a line longer than 1024 characters."
M_043 " Tag not correct|ex: 0602-033 Only one tag is allowed per line."
$       do not translate tag. 
M_044 "ex: 0602-034 There is no previous tag."
$       do not translate tag. 
M_045 "ex: 0602-035 The %S entry in the tags file is not correct."
M_046 "ex: 0602-036 No write has occurred@since last change."
M_047 "ex: 0602-037 No tags file exists.  Run the ctags command on the file or files being edited."
$       do not translate tag. 
M_048 "ex: 0602-038 %S is not a valid tag and is not in the tags file."
$       do not translate tag. 
M_049 "ex: 0602-039 Cannot use the yank command inside a global command or macro."
M_050 "\nAt the end of the file."
M_051 "At the end of the file."
M_052 "Reached the end of the file."
M_053 "Reached the beginning of the file."
M_054 "ex: 0602-040 Cannot use the undo command@in a global command."
$       "undo" is a Unix command.
M_055 "ex: 0602-041 There is no command to undo."
M_056 "No change.|ex: 0602-042 The last command to undo did not change anything."
M_057 "ex: 0602-043 Specify a left side to the definition."
M_058 "ex: 0602-044 Specify a right side to the definition."
M_060 "ex: 0602-045 Definitions do not allow recursion."
M_061 "ex: 0602-046 The left hand side of a map definition must be only 1 character and cannot include the : character."
M_062 "ex: 0602-047 Definitions do not allow recursion."
M_064 "%S not defined|ex: 0602-048 The macro %S is not defined."
M_065 "ex: 0602-049 Cannot create more than 32 macro definitions."
M_066 "ex: 0602-050 Cannot allocate more than 512 characters to macro text."
BAD_2 "An input line cannot be longer than 1024 characters."
M_068 "ex: 0602-051 The Cntl-H character is discarded in input lines.\n"
M_069 "No file name.|ex: 0602-052 No file name is currently defined."
M_070 " [Read only]"
M_071 " [Not edited]"
M_072 " [Modified]"
M_073 " No defined file name "
M_074 " The cursor is at line %1$d of %2$d --%3$ld%%-- ."
$       Please leave multiple % in the message.
M_075 "The pattern is too long."
M_076 "ex: 0602-053 The maximum length of an argument is 1024 characters."
M_077 "ex: 0602-054 No alternate file name@to substitute for the # character."
M_078 "ex: 0602-055 No current file name@to substitute for the %% character."
M_079 "ex: 0602-056 The pipe system call failed."
$       Do not translate pipe.
M_080 "ex: 0602-057 Cannot create a process at this time."
M_081 "NCARGS parameter list is too long"
M_082 "NARGS parameter list is too long"
$       NCARGS and NARGS are system defined do not translate. 
M_083 "ex: 0602-058 There is no file name that matches."
M_084 "ex: 0602-059 Specify a file name"
M_085 "Ambiguous|ex: 0602-060 There are too many file names"
M_086 "ex: 0602-061 A file name cannot be longer then 1020 characters"
M_087 " [New file]"
M_088 " Block special file"
M_089 " Teletype"
M_090 " Character special file"
M_091 " Directory"
M_092 " Executable file"
M_093 " Archive file"
M_095 " [Read only]"
M_096 "ex: 0602-062 Use w or w>> to write the file."
M_097 "No file name.|ex: 0602-063 No file name is currently defined."
M_098 " File already exists.|ex: 0602-064 The file already exists. Use w! %s to force the write."
$       w! is a is a subcommand. Do not translate. 
M_099 "ex: 0602-065 The file has read permission only."
M_101 "ex: 0602-066 Use w! to write the partial buffer."
$       w! is a is a subcommand. Do not translate. 
M_102 " [New file]"
M_103 " [Existing file]"
M_104 " [Last line is not complete]"
BAD_3 " A line cannot be longer than 1024 characters."
M_106 "The limit is 200 nested source files."
M_107 " %1$d/%2$ld"
$        Do not translate.  This is the terse message for M_292, M_295, and M_296. 
M_109 " (%ld null character)"
$       This will be used after M_107, M_292, M_295, or M_296. 
M_111 "ex: 0602-067 A global within a global@command is not allowed."
M_112 "Global requires regular expression.|ex: 0602-068 A global command requires a regular expression."
M_113 "ex: 0602-069 The command in a global cannot be longer than 256 characters."
M_114 "ex: 0602-070 An error in the requested substitution has created a loop."
M_115 "No match.|ex: 0602-071 No match exists for the substitute pattern."
M_116 "Substitute needs regular expression.|ex: 0602-072 Specify a regular expression for the substitute command."
M_117 "No previous regular expression|ex: 0602-073 Specify a regular expression."
M_118 "No previous substitute regular expression|ex: 0602-074 Specify a substitute regular expression for the & character to repeat."
M_119 "ex: 0602-075 The replacement pattern is too long.@The limit is 256 characters."
M_120 "ex: 0602-076 The substitution creates a line that is too long.@ The limit is 1024 characters."
M_121 "%d substitutions|%d substitutions were made"
M_122 " on %d lines"
$       M_122 will be displayed after M_121 if displayed anywhere.
M_123 "ex: 0602-077 Regular expressions delimiters cannot be letters or digits."
M_124 "No previous scan regular expression.|ex: 0602-078 no previous scanning regular expression is defined."
M_126 "A regular expression is not correct.|ex: 0602-079 A regular expression \\ must be followed by a / or a ? character."
M_128 "ex: 0602-080 Specify a closing delimiter@for the regular expression."
M_129 "Regular expression too long.|ex: 0602-081 The regular expression cannot expand to more than 512 characters."
M_130 "No match for \\( delimiter.|ex: 0602-082 There are more \\( start delimiters than end \\) delimiters."
M_131 "Too many \\( expressions.|ex: 0602-083 Specify no more than 9 \\(...\\) constructs in a regular expression."
M_132 "No match for \\) delimiter.|ex: 0602-084 There are more end \\) delimiters than start \\( delimiters."
M_133 "ex: 0602-085 Cannot use &@- for a replacement pattern in a regular expression."
M_134 "ex: 0602-086 Cannot use \\d@- for a replacement pattern in a regular expression."
M_135 "* character is not valid.|ex: 0602-087 The * cannot be used after a \\(...\\) construct in a regular expression."
M_137 "Empty character class.|ex: 0602-088 Cannot use an empty [] or [^] character class."
M_138 "ex: 0602-089 Specify a closing ] delimiter."
M_139 "No new lines in re.|ex: 0602-090 Regular expressions cannot contain new line characters."
M_140 "No closing delimiter.|ex: 0602-091 The regular expression must have a closing delimiter."
M_141 "ex: 0602-092 Internal error in regular expression."
M_142 "ex: 0602-093 %S is not a recognized option.@Use set all to see all options."
M_143 "ex: 0602-094 Option %s must be set to a value."
M_144 "ex: 0602-095 Specify the = character@in the assignment for option %s."
M_145 "ex: 0602-096 Specify a number@after the = operator."
M_146 "ex: 0602-097 Option string is too long.@The limit is 128 characters."
M_147 "ex: 0602-098 Cannot change terminal type while in open or visual mode."
M_148 "%d lines"
M_149 "ex: 0602-099 An address cannot be zero@when using the specified command."
M_150 "ex: 0602-100 No lines@exist in the specified buffer."
M_153 "ex: 0602-101 Out of memory@saving lines for undo."
$       "undo" is a Unix command.
M_190 "ex: 0602-102 There is a system call error number %d."
M_191 " \r\nYour file has been preserved\r\n"
M_192 "An interrupt signal occurred."
M_193 "\nAn interrupt signal occurred."
M_194 "ex: 0602-103 The temporary file is too large to be placed in the /tmp directory."
M_196 "ex: 0602-104 Out of space in a named buffer or a numbered buffer."
M_197 "ex: 0602-105 Editing buffer %c is empty."
M_198 "ex: 0602-106 Cannot put a partial line inside macro"
M_199 "ex: 0602-107 Editing buffer %c is empty."
M_200 "The register is greater than 1024 characters."
M_202 "ex: 0602-108 %s is not a recognized terminal type."
M_203 "ex: 0602-109 Specify a shell command@or use the :shell command."
M_204 "ex: 0602-110 A command cannot be longer than 128 characters."
M_205 "ex: 0602-111 No previous command@exists for the ! substitution."
$       Leave the ! character after the. 
M_206 "ex: 0602-112 No alternate file name@exists for the # substitution."
M_207 "ex: 0602-113 No file name@exists for the %% substitution."
$       needs to have multiple % characters so that when displayed only one appears. 
M_208 "[No write]|[There has been no write since the last change]"
M_209 "ex: 0602-114 No previous command@is defined to repeat."
M_210 "ex: 0602-115 Cannot create a pipe."
M_211 "ex: 0602-116 Cannot create a process at this time."
M_212 "No %s!\n"
$       Leave the ! character after the %s. 
M_213 "ex: 0602-117 Cannot create a pipe."
M_214 "ex: 0602-118 Cannot create another process at this time."
M_215 "ex: 0602-119 Cannot create a pipe for the recovery."
M_216 "ex: 0602-120 Cannot create a process for the recovery."
M_217 "ex: 0602-121 No recovery routine is available."
M_218 "ex: 0602-122 Process number %1$d ended with a signal %2$d."
M_219 "Memory image file created."
$       M_219 will be displayed after M_218 if displayed anywhere. 
M_220 "Pattern does not exist.|ex: 0602-123 The pattern does not exist on the specified line."
M_221 "ex: 0602-124 You are already in open or visual mode."
M_222 "ex: 0602-125 Open or visual mode is not available when the novice option is set."
M_223 "ex: 0602-126 %s is a generic terminal type."
M_224 "[Using open mode]"
M_225 "ex: 0602-127 Visual mode requires more cursor capability than the terminal provides."
M_226 "ex: 0602-128 Cannot use visual mode on a terminal which overstrikes."
M_227 "ex: 0602-129 Visual mode requires clear screen capability."
M_228 "ex: 0602-130 Visual mode requires scrolling capability."
M_229 "The screen is too large for the internal buffer"
M_230 "Do not know enough about the terminal type to use %s"
M_231 "The terminal is too wide"
M_232 "The screen is too large"
$       M_229 through M_232 are debug messages. 
M_233 "ex: 0602-131 There is an internal software error in the vscroll function."
$       Do not translate vscroll. 
M_234 "ex: 0602-132 The specified buffer is empty."
BAD_6 "ex: 0602-133 There is an internal software error in the vredraw function."
$       Do not translate vredraw. 
M_236 "ex: 0602-134 An error occurred while reading input."
M_238 "The macro is too long.@It may be recursive."
M_239 "ex: 0602-135 A macro cannot be nested to more than 256 levels."
M_240 "ex: 0602-136 Use Q to enter ex command mode or use :q to exit vi."
M_241 "REPLACE 1 CHARACTER"
$       message M_241 must not exceed a display width of 19 columns
$       when translated
$       message M_241 is the 'verbose' half of M_279 
M_242 "REPLACE MODE"
$       message M_242 must not exceed a display width of 19 columns
$       when translated
$       message M_242 is the 'verbose' half of M_280 
M_243 "INPUT MODE"
$       message M_243 must not exceed a display width of 19 columns
$       when translated
$       message M_243 is the 'verbose' half of M_281 
M_245 "ex: 0602-137 A macro cannot be nested to more than 256 levels."
BAD_4 "ex: 0602-138 An input line cannot be longer than 1024 characters."
M_249 "ex: 0602-140 The window is too small to display the current line."
M_250 "The line is too long for the open command"
M_251 "The line is too long"
M_252 "ex: 0602-141 The character class is not recognized@in the [: ... :] construct."
M_253 "ex: 0602-142 There is an internal software error in the cclass function."
$       Do not translate cclass. 
M_258 "expreserve: 0602-143 Only a user with proper authority can preserve files.\n"
M_259 "Buffer format error"
M_260 "Buffer read error"
M_261 "Cannot find a name"
M_262 "A copy of a file you were editing was saved when the system went down.\n"
M_263 "A copy of a file you were editing was saved when the editor was killed.\n"
$      "Kill" is a Unix technical term.
M_264 "No name was associated with this file so it has been named LOST.\n"
M_265 "A copy of your file %s was saved when the system went down.\n"
M_266 "A copy of your file %s was saved when the editor was killed.\n"
M_267 "This file can be retrieved using the recover command of the editor.\nAn easy way to do this is to give the command:\"ex -r %s\".\nThis works for the \"edit\" and \"vi\" editors also.\n"
M_268 " The number of parameters to exrecover is not correct."
$       Do not translate exrecover. 
M_269 " [Dated %1$s. This version is the newest of %2$d saved copies.]"
$       "Dated" is used in the sense of assigning a date.
M_270 " [Dated %s]"
M_271 "exrecover: 0602-144 There is not enough memory available now."
M_272 "exrecover: 0602-145 There are no preserved files.\n"
M_273 "%1$s saved %2$d lines of file %3$s\n"
M_274 "exrecover: 0602-146 The specified file does not exist."
M_275 "Lost line(s):"
M_277 " Temporary file is too large"
M_278 "exrecover: 0602-147 System call error %d has occurred."
M_279 "r"
$       This, and the following 2 messages should not be translated.
M_280 "R"
M_281 "I"
M_282 " %1$d more lines@in the file after the %2$s command."
M_283 "%1$d fewer lines@in the file after the %2$s command."
M_284 "%d more lines"
M_285 "%d fewer lines"
M_286 "%d lines"
M_287 "%a %sD %T"
$       This is a format string for strftime, not printf.
$	It may be reordered as needed for each country.
$	%a is the short day of the week.
$	%sD is the short date with format controled by LC_TIME.
$	%sT is the time with format controlled by LC_TIME.
M_288 "On %a %sD at %sT"
$       This is a format string for strftime, not printf.
$	It may be reordered as needed for each country.
$	%a is the short day of the week.
$	%sD is the short date with format controled by LC_TIME.
$	%sT is the time ommiting the seconds with format controlled by LC_TIME.
M_289 "ex: 0602-148 The parameter to the partial character option must be a single character."
M_290 "ex: 0602-149 Specify an f, g, r, or w@parameter after the = character."
M_291 "There are %d more files to edit."
M_292 " %1$d line, %2$ld character"
M_293 "ex: 0602-150 There are fewer characters than specified "
BAD_5 "There are %d more files to edit."
M_295 " %1$d line, %2$ld characters"
M_296 " %1$d lines, %2$ld characters"
M_297 " (%ld null characters)"
$       This will be used after M_107, M_292, M_295, or M_296. 
$	M_298 to M_302 inclusive are included at the end of the file.
M_499 "tvi: 0602-151 New mappings cannot be defined."
M_500 "tvi: 0602-152 Tags are not allowed."
M_501 "tvi: 0602-153 Shell escapes are not allowed."
M_502 "tvi: 0602-154 The Q command is not allowed."
M_503 "tvi: 0602-155 The recover command is not allowed."
M_504 "tvi: 0602-156 The open command is not allowed."
M_505 "tvi: 0602-157 The preserve command is not allowed."
$       Do not translate preserve.
M_506 "tvi: 0602-158 Mode switching is not allowed."
M_507 "The value is set to %d."
$       This is appended to M_512 
M_508 " The value is set to %d."
$       This is appended to M_510 
M_509 " The value is set to %d."
$       This is appended to M_511 
M_510 "ex: 0602-159 The tabstop option cannot be %d.  Use a value greater than 0."
$        Do not translate tabstop. 
M_511 "ex: 0602-160 The hardtabs option cannot be %d.  Use a value greater than 0."
$       Do not translate hardtabs. 
M_512 "ex: 0602-161 The shiftwidth option cannot be %d. Use a value greater than 0."
$       Do not translate shiftwidth. 
M_513 "ex: 0602-162 Cannot find the terminfo database."
$       Do not translate terminfo. 
$	M_514 included later in the file.
M_600 "Usage: ctags [-BFamtuwvx] [-f Tagfile] File ...\n"
M_601 "ctags: 0602-164 There is not enough memory available to sort the functions requested.\n"
M_602 "ctags: 0602-165 The file %1$s at line %2$d contains a duplicate entry %3$s.\n"
M_603 "ctags: 0602-166 The files %1$s and %2$s contain a duplicate function %3$s. This is a warning only.\n"
M_604 "ctags: 0602-163 The -%c option requires an operand.\n"
M_605 "ctags: 0602-174 The -%c option is invalid.\n"
$
$  The following 4 messages were changed for apar ix12676 and the old ones
$       above have an id of BAD_x (where x = 1-4).  The old messages are
$       kept for backward compatability.
M_037 " Line too long.|ex: 0602-027 The result of a line join would be greater than 2048 characters."
M_067 "An input line cannot be longer than 2048 characters."
M_105 " A line cannot be longer than 2048 characters."
M_246 "ex: 0602-138 An input line cannot be longer than 2048 characters."
$  The following messages were also modified.
M_298 " [Shared lock]"
M_299 " [Exclusive lock]"
M_300 " [FILE BEING MODIFIED BY ANOTHER PROCESS]"
M_301 " [File open by another process]"
M_302 " Cannot set novice interactively - invoke as vedit"
M_304 "ex: The -%c option requires an option argument."
M_305 "ex: The -%c option requires an integer option argument."
M_514 "The %s option must be greater than 0. Default value used."
M_650 "ex: 0602-167 Invalid multibyte character string encountered, conversion failed."
M_651 "ex: 0602-168 Invalid wide character string encountered, conversion failed."
M_652 "ex: 0602-169 Incomplete or invalid multibyte character encountered, conversion failed."
M_653 "ex: 0602-170 Wide character could not be converted to file code form."
M_654 "ex: 0602-171 Invalid multibyte character encountered with mblen."
M_670 "ex: 0602-172 Property invalid for the current locale."
M_671 "ex: 0602-173 Error in character collation value."
M_294 "There is %d more file to edit."
M_235 "ex: 0602-133 Unable to redraw screen. Switching to line mode.\nIssue the vi subcommand to return to visual mode."
M_672 "ex: 0602-174 The macro text for the macro %S is at least 100 characters long."

M_673 "ex: 0602-175 Specify a valid numeric interval."
M_674 "ex: 0602-176 Only one comma allowed in interval expressions."
M_675 "ex: 0602-177 Terminate an interval expression with \\}."
M_676 "ex: 0602-178 Numbers specified in interval expressions must be\nless than RE_DUP_MAX."
