$ @(#)91	1.3.1.1  src/bos/usr/bin/more/more.msg, cmdscan, bos411, 9428A410j 7/26/93 11:57:50
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$ ORIGINS: 85, 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$
$ OSF/1 1.2
$
$ @(#)$RCSfile: more.msg,v $ $Revision: 1.5.4.5 $ (OSF) $Date: 1992/11/17 20:18:58 $

$
$ Messages for the more command.
$
$quote "
$set MS_MORE

$
$ Messages for ch.c
$
PIPE_ERR	"0652-250 Pipe error"
NOBUFF		"0652-251 Cannot allocate %d buffers"

$
$ Messages for command.c
$
FILEMSG		" file %d/%d"
LINEMSG		" line %d"
BYTEMSG		" byte %d"
PERCENT		"/%d percent %d%%"
EOFNEXT		": END (next file: "
END		": END"
PHELP		"[Press space to continue, q to quit, h for help]"
HELP		"help"
PTAG		"Tag: "
EXAMINE		"Examine: "
NOSTDIN		"Cannot edit standard input"
MARK		"mark: "
GMARK		"goto mark: "
PRESSH		"[Press 'h' for instructions.]"
NOFARGS		"No files provided as arguments."
STDIN		"Standard input"
SHDONE		"!done"

$
$ Messages for linenum.c
$
ELINENO		"Calculating line numbers"


$
$ Messages for main.c
$
NOCURR		"No current file"
NOPREV		"No previous file"
NOVIEW		"Can view standard input only once"
NOTERM		"0652-252 Cannot take input from a terminal"
NONTHF		"No (N-th) next file"
NONTHF2		"No (N-th) previous file"
NOMEM		"0652-253 Cannot allocate memory"

$
$ Messages for option.c
$
BADSCREEN	"Bad screen size.\n"
BADWARG		"0652-254 -W option is not valid.\n"
USAGE		"Usage: %s [-Ncdeisuvz] [-t tag] [-x tabs] [-p command] [-n number]\n\t    [-W option] [file ...]\n"

$
$ Messages for os.c.c
$
ISADIR	 	"%s is a directory"
TOOLONG		"0652-255 Command string is too long"

$
$ Messages for output.c
$
PRETURN		"(press RETURN)"
INTRTOAB	"... (interrupt to abort)"
OUTOF_SEQ	"Out of sequence of standout display mode"

$
$ Message for prim.c
$
SKIP		"...skipping...\n"
NOSEEK		"Cannot seek to end of file"
NOBEGIN		"Cannot get to beginning of file"
ONLYN		"File has only %d lines"
NOLENGTH	"Do not know length of file"
CANTSEEK	"Cannot seek to that position"
BADLET		"Choose a letter between 'a' and 'z'"
$ Do not translate 'a', 'z'.
NOMARK		"mark not set"
NOREG		"No previous regular expression"
NOSEARCH	"Nothing to search"
NOPAT		"Pattern not found"
$
$ Messages for tags.c
$
NOTAGF		"0652-256 No tags file"
NOSUCHTAG	"0652-257 No such tag in tags file"
TNOTFOUND	"0652-258 Tag not found"

$
$ Messages for help.c
$

$ These messages must remain in order
$
HELP_01 "  Commands flagged with an asterisk (``*'') may be preceeded by a number.\n"
HELP_02 "  Commands of the form ``^X'' are control characters, i.e. control-X.\n"
HELP_03 "\n"
HELP_04 "  h              Display this help.\n"
HELP_05 "\n"
HELP_06 "  f, ^F, SPACE * Forward  N lines, default one screen.\n"
HELP_07 "  b, ^B        * Backward N lines, default one screen.\n"
HELP_08 "  j, CR, ^E    * Forward  N lines, default 1 line.\n"
HELP_09 "  k, ^Y        * Backward N lines, default 1 line.\n"
HELP_10 "  d, ^D        * Forward  N lines, default half screen or last N to d/u.\n"
HELP_11 "  u, ^U        * Backward N lines, default half screen or last N to d/u.\n"
HELP_12 "  g            * Go to line N, default 1.\n"
HELP_13 "  G            * Go to line N, default the end of the file.\n"
HELP_14 "  p, %         * Position to N percent into the file.\n"
HELP_15 "\n"
HELP_16 "  r, ^L          Repaint screen.\n"
HELP_17 "  R              Repaint screen, discarding buffered input.\n"
HELP_18 "\n"
HELP_19 "  m[a-z]         Mark the current position with the supplied letter.\n"
HELP_20 "  '[a-z]         Return to the position previously marked by this letter.\n"
HELP_21 "  ''             Return to previous position.\n"
HELP_22 "\n"
HELP_23 "  /pattern     * Search forward  for N-th line containing the pattern.\n"
HELP_24 "  /!pattern    * Search forward  for N-th line NOT containing the pattern.\n"
HELP_25 "  ?pattern     * Search backward for N-th line containing the pattern.\n"
HELP_26 "  ?!pattern    * Search backward for N-th line NOT containing the pattern.\n"
HELP_27 "  n            * Repeat previous search (for N-th occurence).\n"
HELP_28 "\n"
HELP_29 "  :a              Display the list of files.\n"
HELP_30 "  :e, E [file]    Examine a new file.\n"
HELP_31 "  :n, N        *  Examine the next file.\n"
HELP_32 "  :p, P        *  Examine the previous file.\n"
HELP_33 "  :t [tag]        Examine the tag.\n"
HELP_34 "  v               Run an editor on the current file.\n"
HELP_35 "\n"
HELP_36 "  =, ^G, :f       Print current file name and stats.\n"
HELP_37 "\n"
HELP_39 "  q, :q, or ZZ    Exit.\n"
HELP_40 "  !command        Invoke a shell with command. \n"
HELP_41 "  :!command       Invoke a shell with command. \n"
HELP_42 "    z          *  Scroll N lines, default one screen.\n" 


HELPEND		 "End of help"

