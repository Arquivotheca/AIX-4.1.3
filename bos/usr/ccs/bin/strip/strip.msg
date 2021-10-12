$                    "@(#)83\t1.7.1.2  src/bos/usr/ccs/bin/strip/strip.msg, cmdaout, bos411, 9428A410j 2/4/94 18:23:14"
$ COMPONENT_NAME: CMDAOUT (strip command)  
$  FUNCTIONS: none  
$  ORIGINS: 27  
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$  combined with the aggregated modules for this product)  
$                   SOURCE MATERIALS  
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$  All Rights Reserved  
$  US Government Users Restricted Rights - Use, duplication or  
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$quote "         define quote character.

$len 
$set MS_STRIP        Define initial set#

AR_STRIPPED_MSG      "0654-419 The specified archive file was already stripped."

ALREADY_STRIPPED_MSG "0654-420 The file was already stripped as specified."

USAGE_MSG            "Usage: strip [-V] [-r[-l]|-x[-l]|-t|-H|-e|-E] File ..."
$ Do not translate command name (strip) or options in USAGE_MSG  

CANT_OPEN_MSG        "0654-400 Cannot open file."

CANT_OPEN_TMP_MSG    "0654-401 Cannot open temporary file %s."

CANT_RECR_MSG        "0654-402 Cannot write to file, permissions are not correct."

READ_ERR_MSG         "0654-403 Cannot read from file."

WRITE_ERR_MSG        "0654-404 Cannot write to file."

NOT_XCOFF_MSG        "0654-405 Specify an XCOFF object file or an archive of\n\
\tXCOFF object files."
$ Do not translate XCOFF.  

READ_TAB_ERR_MSG     "0654-406 Internal error while reading file tables."

PROCESS_TAB_ERR_MSG  "0654-407 Internal error while processing file tables."

COPY_SCN_ERR_MSG     "0654-408 Internal error while copying sections."

COPY_SYM_ERR_MSG     "0654-409 Internal error while copying symbol table."

WRITE_TAB_ERR_MSG    "0654-410 Internal error while writing headers."

OPTMIX_MSG           "0654-411 The specified flags are mutually exclusive."

TRUNCATE_ERR_MSG     "0654-412 The ftruncate system call failed."
$ Do not translate ftruncate.  

SEEK_ERR_MSG         "0654-413 File seek error."

SEEKT_ERR_MSG         "0654-414 Temporary file %s: seek error."

ZERO_ERR_MSG         "0654-415 File size was zero."

ZEROT_ERR_MSG         "0654-416 Temporary file %s: size was zero."

CLOSE_ERR_MSG        "0654-417 File close error."

CLOSET_ERR_MSG        "0654-418 Temporary file %s: close error."

WRITET_ERR_MSG        "0654-421 Cannot write to temporary file %s."

AR_LOADONLY_ERR_MSG   "0654-422 Cannot use -e or -E flags with an archive."
