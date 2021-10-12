$ @(#)76	1.19  src/bos/usr/bin/gencat/msgfac.msg, cmdmsg, bos411, 9428A410j 2/26/94 14:09:43
 
$ COMPONENT_NAME: CMDMSG
$
$ FUNCTION: msgfac.msg
$ 
$ ORIGINS: 27
$ 
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.



$quote "         define quote character.

$set MS_DSPCAT        Define initial set#

M_CAT_NO_OPEN	"dspcat: 1312-001 Cannot open specified catalog: %s\n"
M_NO_ID	"dspcat: 1312-002 Cannot specify message number with the -g flag."
M_USAGE	"Usage:  dspcat [-g] CatalogName [SetNumber [MessageNumber]]"
M_BADSET "dspcat: 1312-003 Specify a valid set number."
M_BADMSG "dspcat: 1312-004 Specify a valid message number."


$set MS_GENCAT        Redefine set#

M_MSG_0	"Usage: gencat CatalogFile SourceFile..."
M_MSG_1	"gencat: 1312-005 Cannot open target file %s.\n"
M_MSG_2	"gencat: 1312-006 The realloc system call failed."
M_MSG_3	"gencat: 1312-007 The following message text is longer than the $len value:\n\t%s\n"
M_MSG_5	"gencat: 1312-008 The set number in the following line is not valid:\n\t%s\n"
M_MSG_7	"gencat: 1312-009 The length of the hex number in the following line is not valid.\n\
It must be either two or four digits.\n\t%s\n"
M_MSG_8	"gencat: 1312-010 Reached end of line before the defined closing quote.\n\t%s\n"
M_MSG_9	"gencat: 1312-011 The following message string is longer than NL_TEXTMAX:\n%s\n"
M_MSG_10	"gencat: 1312-012 Reached end of string before expected.\n\t%s\n"
M_MSG_11	"gencat: 1312-013 Internal error: The file pointer offset is not correct."
M_MSG_12	"gencat: 1312-014 Internal error: The file pointer offset is not correct."
M_MSG_13	"gencat: 1312-015 Cannot load the catalog file %s.\n"
M_MSG_14	"gencat: 1312-016 Cannot read the existing catalog file %s.\n"
M_MSG_15	"gencat: 1312-017 There is not enough memory available now."
M_MSG_18	"gencat: 1312-020 The following set uses a symbolic identifier:\n\t%s\n"
M_MSG_19	"gencat: 1312-021 The following message uses a symbolic identifier:\n\t%s\n"
M_NOMSG	"gencat: 1312-019 There is no message defined in a source file."
M_ORDER	"gencat: 1312-022 Set or message numbers are not in ascending sequence after:\n\
\tMessage:  %d,  Set: %d\n\
\t%s\n"

$set MS_MKCATDEFS        Redefine set#

MKCATUSAGE	"Usage: %s SymbolName SourceFile[...SourceFile] [-h]\n"
MNAMTOOLONG	"mkcatdefs: 1312-025 The message catalog name is too long.\n"
MKCATOPN	"mkcatdefs: 1312-026 Cannot open %s.\n"
INVTAG	"mkcatdefs: 1312-027 The symbolic message identifier is not valid:\n\
\t%s\n"
INVLDSET	"mkcatdefs: 1312-028 A set number is not in ascending sequence.\n"
ZEROINV	"mkcatdefs: 1312-029 %s is not a valid set identifier.\n"
MULTOPN	"mkcatdefs: 1312-030 The message identifier %s is used more than once.\n"
MULTNR	"mkcatdefs: 1312-031 The message id %s has already been assigned to identifier.\n"
NOTASC	"mkcatdefs: 1312-032 The message numbers are not in ascending sequence.\n"
READERRS	"mkcatdefs: 1312-033 There were read errors in file %s.\n"
WRITERRS	"mkcatdefs: 1312-034 There were write errors in file %s.\n"
ERRFND	"mkcatdefs: 1312-035 Errors found: The %s header file is not created.\n"
HCREAT	"mkcatdefs: %s is created.\n"
NOSYMB	"mkcatdefs: 1312-036 No symbolic identifiers; %s is not created.\n"
NOHDR	"mkcatdefs: The %s header file is not created.\n"
IMBCHD	"mkcatdefs: 1312-037 Catname contains invalid International Language character.\n"
IMBCTX	"mkcatdefs: 1312-038 SourceFile contains invalid International Language character in the\
 following text:\n\t%s\n"

$set MS_DSPMSG        Define initial set#

M_DSPMSG	"Usage: dspmsg [-s SetNumber] CatalogName MessageNumber ['default' [Argument...]]"
M_REORDER	"\ndspmsg: 1312-040 Either all conversion specification in the message\n\
must be in the %n$ format or none of them should be in this format."
M_INVRE		"\ndspmsg: 1312-041 The message is missing a $ in a conversion\n\
specification of type %n$"
M_REINDEX	"\ndspmsg: 1312-042 Invalid argument index in message.  May need\n\
more arguments on the command line."
M_NOSUPPORT	"\ndspmsg: 1312-043 Messages must use a supported conversion\n\
specifier, either %s or %ld."
M_NODEFAULT	"\ndspmsg: 1312-044 Missing a default message."

$set MS_RUNCAT        Define initial set#

M_RUNCAT	"Usage: runcat CatalogName SourceFile [CatalogFile]\n"
