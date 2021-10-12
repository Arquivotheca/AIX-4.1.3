$                    "@(#)77\t1.10.2.2  src/bos/usr/ccs/bin/nm/nm.msg, cmdaout, bos411, 9428A410j 3/2/94 13:42:30"
$ COMPONENT_NAME: CMDAOUT (nm command)   
$  FUNCTIONS: none   
$  ORIGINS: 27   
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when   
$  combined with the aggregated modules for this product)   
$                   SOURCE MATERIALS   
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$  All Rights Reserved   
$  US Government Users Restricted Rights - Use, duplication or   
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.   
$ Message strings for the nm command   
$quote "         define quote character.

$len 
$set MS_NM        Define initial set#

$ Do not translate nm:  
$ Do not translate %c, %s etc. 
$ Do not translate command name or options in the following   

USAGE_MSG            "Usage: nm [-ACfhrTv] [-B|-P] [-e|-g|-u] [-d|-o|-x|{-t [d|x|o]}] [--] File ...\n"

FMTS_D_0_MSG         "Name              Type    Value     Size\n\n"
$ This, and the following 11 messages are used as headers to output sections.   
$ The length of these strings is critical to the format of the output.   
$ When translating be sure that the starting character of each header item 
$ is in the same column as in the English version. 
$ Class refers to storage class. 

FMTS_D_1_MSG         "Name              Type    Value     Size\n\n"

FMTS_D_SDB_0_MSG     "Name              Type    Value     Size\n\n"

FMTS_D_SDB_1_MSG     "Name              Type    Value     Size\n\n"

FMTS_X_0_MSG         "Name              Type    Value     Size\n\n"

FMTS_X_1_MSG         "Name              Type    Value     Size\n\n"

FMTS_X_SDB_0_MSG     "Name              Type    Value     Size\n\n"

FMTS_X_SDB_1_MSG     "Name              Type    Value     Size\n\n"

FMTS_O_0_MSG         "Name              Type    Value     Size\n\n"

FMTS_O_1_MSG         "Name              Type    Value     Size\n\n"

FMTS_O_SDB_0_MSG     "Name              Type    Value     Size\n\n"

FMTS_O_SDB_1_MSG     "Name              Type    Value     Size\n\n"

FILE_NAME_MSG        "Filename"

NO_NAME_STR_MSG      "**_no_name_**"
$ The following message strings are used to indicate special occurences   
$ in the output. Message expansion should be kept to a minimum.   

BAD_NAME_STR_MSG     "**_incorrect_symbol_name_**"

NO_SCN_STR_MSG       "**_no_scn_**"
$ Scn is section.  

UNDEF_SYM_FIL_MSG    "\n\n\
Undefined symbols from %s:\n\n"
$ %s is the name of object file   

UNDEF_SYM_MEM_MSG    "\n\n\
Undefined symbols from %1$s[%2$.14s]:\n"
$ %1s is the name of the archive file and %2$.14 is the member name.  

DEF_SYM_FIL_MSG      "\n\n\
Symbols from %s:\n\n"
$ %s is the name of object file   

DEF_SYM_MEM_MSG      "\n\n\
Symbols from %1$s[%2$.14s]:\n\n"
$ %1s is the name of the archive file and %2$.14 is the member name.  

OPEN_ERR_MSG         "0654-200 Cannot open the specified file.\n"

REOPEN_ERR_MSG       "0654-201 Cannot open the specified file.\n"

NOT_OBJECT_ERR_MSG   "0654-202 Error while reading object file header.\n"

NOT_XCOFF_ERR_MSG    "0654-203 Specify an XCOFF object module.\n"
$ XCOFF is another name for the object format.   

NO_SYMBOLS_MSG       "0654-204 There are no symbols in the file.\n"

TEMP_OPEN_ERR_MSG    "0654-205 Cannot open the temporary file required with the -vn flags.\n"

BAD_SYMTAB_ERR_MSG   "0654-206 Cannot process the symbol table.\n"

SORT_ABEND_ERR_MSG   "0654-207 The sort process was stopped prematurely.\n"

UNKNOWN_OPT_ERR_MSG  "0654-208 %c is not a recognized flag.\n"

NO_MEM_MSG           "0654-209 There is not enough memory available now.\n"
