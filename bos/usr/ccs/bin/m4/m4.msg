$ @(#)85	1.11  src/bos/usr/ccs/bin/m4/m4.msg, cmdm4, bos411, 9428A410j 9/9/93 08:36:10
$ 
$  COMPONENT_NAME: (CMDM4) Macroprocessor 
$                                                                    
$  FUNCTIONS: 
$                                                                    
$  ORIGINS: 3, 27
$ 
$  This module contains IBM CONFIDENTIAL code. -- (IBM
$  Confidential Restricted when combined with the aggregated
$  modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$  All Rights Reserved
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$quote "         define quote character.
$set MS_M4        Define initial set#
$
QEOF "1252-201 A quote is not complete.\n\
\tCheck that the end quote character exists.\n"
CEOF "1252-202 A comment is not complete.\n\
\tCheck that the end comment character exists.\n"
AEOF "1252-203 A parameter list is not complete.\n\
\tCheck that the ) character exists.\n"
OFL "1252-204 Cannot open the specified file.\n\
\tCheck path name and permissions.\n"
OST "1252-205 There is not enough memory available now.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"
ST ",line %d: %s\n"
$ format for line number information displayed before error message
QLNB "1252-206 The quote symbols cannot be more than %d bytes long.\n"
QLNC "1252-207 The quote symbols cannot be more than %d characters long.\n"
BMN "1252-208 A macro name cannot begin with a number and can\n\
\tcontain only alphabetic letters, numbers, and the _ character.\n"
MD "1252-209 The macro name and definition cannot be identical.\n"
IE "1252-210 A parameter to the eval macro is not correct.\n\
\tCheck the parameter for errors.\n"
$ Do not translate "eval" -- it is an m4 specific subcommand
IFLN "1252-211 Do not use more than 9 levels of\n\
\tinclude and sinclude macros.\n"
$ Do not translate "include" or "sinclude" -- they are macro names
MTB "1252-212 Do not specify more than %d bytes in a word.\n\
\tUse the -T flag to increase the size of the buffer.\n"
MTC "1252-213 Do not specify more than %d characters in a word.\n\
\tUse the -T flag to increase the size of the buffer.\n"
MTI "1252-214 A parameter list cannot contain more than %d items.\n\
\tUse the -S flag to increase the size of the buffer.\n"
PBMB "1252-215 The push-back buffer cannot contain more than %d bytes.\n\
\tUse the -B flag to increase the size of the buffer.\n"
PBMC "1252-216 The push-back buffer cannot contain more than %d characters.\n\
\tUse the -B flag to increase the size of the buffer.\n"
CMLB "1252-217 A comment marker cannot be more than %d bytes long.\n"
CMLC "1252-218 A comment marker cannot be more than %d characters long.\n"
AUGT "1252-219 The parameters to the specified macro\n\
\tcannot contain more than %d bytes of text.\n"
MIA "1252-220 %s is not a valid value for the -H flag.\n\
\tSpecify a numerical value greater than zero.\n"
USAGE "Usage: m4 [-els] [-B Number] [-D Name[=Value]]... [-H Number]\n\
       [-I Directory] [-S Number] [-T Number] [-U Name]... [File...] \n"
TR "Tracing (%1$d): %2$s"
$ informational message that tracing is turned on
MBWC "1252-222 An invalid multi-byte character is in the string: %s\n"
WCMB "1252-223 Not able to convert wide char string to multi-byte string.\n\
\tInvalid wide character string.\n"
WCDISP "1252-224 Not able to calculate display width.\n\
\tInvalid wide character string.\n"
BADTMP "1252-225 The temporary file %s already exists.\n\
\tPlease try again with trailing X's following the file name."
TOOLONG "1252-226 The temporary file name is too long.\n"
