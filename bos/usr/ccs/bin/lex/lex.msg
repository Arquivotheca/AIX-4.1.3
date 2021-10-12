$ @(#)13        1.11  src/bos/usr/ccs/bin/lex/lex.msg, cmdlang, bos411, 9428A410j 8/17/93 12:47:26
$
$ COMPONENT_NAME: (CMDLANG) Language Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1991
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$len 
$set MS_LEX
$quote "
$ Message numbers have been assigned a range of 300-499 
$ Do not translate lex:, %d, %s, %1$d, %1$s, etc ... 
BADOPTION       "1285-300 The %c flag is not valid."
BADSTATE        "1285-301 The state %1$d %2$o is not valid."
BIGPAR          "1285-302 The parse tree is too large. %s"
$ Do not move the %s from the end of line. 
BIGPAR2         "\n\tTry using the following: %e Number"
$ Do not translate %e. %e Number is a parameter line in the file passed.
$ Do not move them. 
CALLOCFAILED    "1285-303 There is not enough memory available now."
CHARPUSH        "1285-304 Cannot push more than 1000 characters."
CHARRANGE       "1285-305 The character value %d is out of range."
CHTAB1          "1285-306 The character table declaration is not correct."
DCHAR0          "1285-307 Cannot use character %o (octal) more than once."
DCHARC          "1285-308 Cannot use the %c character more than once."
EMAXPOS         "1285-309 There are too many positions. %s"
$ Do not move the %s from the end of line. 
EMAXPOS1        "\n\tTry using the following: %p Number"
$ Do not translate %p. %p Number is a parameter line in the file passed.
$ Do not move them. 
ENDLESSTR       "1285-310 The string does not end."
ENTRANS         "1285-311 There are too many transitions. %s"
$ Do not move the %s from the end of line. 
ENTRANS1        "\n\tTry using the following: %a Number"
$ Do not translate %a. %a Number is a parameter line in the file passed.
$ Do not move them. 
EOFCOM          "1285-312 End of file reached inside comment."
EOFSTR          "1285-313 End of file reached in string or character constant."
EPOS            "1285-314 Cannot have more than 300 positions for one state."
EREVERSE        "1285-315 There is not enough memory for character table reverse."
ERTEXTS         "1285-316 There is not enough memory for right contexts."
$ right is opposite of left. 
ERR             "Error: "
$ Leave space at the end of the message
EXSTATES        "1285-317 lex rule statements should occur right after %%%%"
$ Do not translate %%%%
INVALREQ        "1285-318 %S is not a valid request."
LANG2LATE       "1285-319 Cannot specify language at this point."
LINE            "Line %1$d: %2$s\n"
LONGDEFS        "1285-320 Definition cannot be longer than 1000 bytes."
LONGSTART       "1285-321 Start conditions cannot be longer than 1024 bytes."
MANYCLASSES     "1285-322 There are too many large character classes."
MANYDEFS        "1285-323 There are too many definitions."
MANYSTATES      "1285-324 There are too many states. %s"
$ Do not move the %s from the end of line. 
MANYSTATES2     "\n\tTry using the following: %n Number"
$ Do not translate %n. %n Number is a parameter line in the file passed.
$ Do not move them. 
NEGINTR         "1285-325 The iteration range must be positive."
NOCORE1         "1285-326 There is not enough memory to begin."
NOCORE2         "1285-327 There is not enough memory for state generation."
$ state generation is a technical term. 
NOCORE3         "1285-328 There is not enough memory for final processing."
NOCORE4         "1285-329 There is not enough memory for the parse tree."
$ parse tree is a technical term. 
NODEF           "1285-330 The definition %S is not found."
NOINPUT         "1285-331 Cannot read input file %s"
NOSTDIN         "1285-332 Cannot read standard input."
NOLDIGITS       "1285-333 Substitution strings cannot begin with digits."
NOLEXDRIV       "1285-334 Cannot open the lex driver %s."
NONTERM         "1285-335 The lex action does not end."
$ action is a lex term. 
NOPEN           "1285-336 Cannot open %s"
NOPEN2          "1285-337 Cannot open file %s"
NOPORTCHAR      "1285-338 Character class is not portable."
NORATFOR        "1285-339 Ratfor is not supported with lex."
NORULESEND      "1285-340 The rules section delimiter is missing."
NOSTART         "1285-341 Start condition %S is not defined."
NOTRANS         "1285-342 Definition not given; null string is assumed"
OVERFL          "1285-343 There is an output table overflow."
PEOF            "1285-344 End of file reached before expected."
SHIFT           "1285-345 Found a shift character."
STRLONG         "1285-346 String cannot exceed 1000 bytes."
MANYPCLASSES    "1285-347 There are too many packed character classes."
TSTART          "1285-348 Cannot use more than 256 start conditions."
TSTART1         "1285-349 Cannot define more than 256 start conditions."
WARN            "Warning: "
$ Leave space at the end of the message
XTRASLASH       "Extra slash removed"
STATISTIC	"%d/%d nodes(%%e); %d/%d positions(%%p); %d/%d (%%n); %ld transitions\n"
$ Do not translate
STATISTIC2      "%d/%d packed char classes(%%k)\n"
$ Do not translate
STATISTIC3      "%d/%d packed transitions(%%a)\n"
$ Do not translate
STATISTIC4	"%d/%d output slots(%%o)\n"
$ Do not translate
BADCHAR		"Invalid Character\n"
STATISTIC5	"%d/%d multi-byte output slots(%%h)\n"
$ Do not translate
STATISTIC6	"%d/%d multi-byte character class output slots(%%z)\n"
$ Do not translate
STATISTIC7	"%d/%d multi-byte character class match table slots(%%m)\n"
$ Do not translate
STATISTIC8	"%d%% hash table vacancy(%%v)\n"
$ Do not translate
EBADMBO		"1285-350 Invalid number of multi-byte output slots specified.  %d"
EMAXMBO		"1285-351 There are too many multi-byte character transitions.%s"
$ Do not move the %s from the end of the line
EMAXMBO1	"\n\tTry using the following: %h Number"
$ Do not translate %h
EMAXMBCCL	"1285-352 There are too many multi-byte character classes.%s"
$ Do not move the %s from the end of the line
EMAXMBCCL1	"\n\tTry using the following: %z Number"
$ Do not translate %z
EMAXMBM		"1285-353 There are too many multi-byte character class character matches.%s"
$ Do not move the %s from the end of the line
EMAXMBM1	"\n\tTry using the following: %m Number"
$ Do not translate %m
EXECS           "1285-354 Executable statements should occur right after %%%%."
$ Do not translate %%%%
EBRACKET        "1285-355 Invalid bracket expression."
LONGNAME        "1285-356 Class name or collating element name is too long."
BADMBCHAR       "1285-357 Invalid multi-byte character."
ECLASSNAME      "1285-358 Invalid character class name."
NOHORM          "1285-359 [^ ] will not match multibyte characters unless %%h and %%m are set greater than zero."
$ Do not translate [^ ], %%h, %%m.
NOZ             "1285-360 . will not match multibyte characters unless %%z is set greater than zero."
$ Do not translate ., %%z.
