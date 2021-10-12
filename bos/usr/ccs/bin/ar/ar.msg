$quote "         define quote character.

$len 

$                    "\"@(#)62\t1.7.1.3  src/bos/usr/ccs/bin/ar/ar.msg, cmdar, bos411, 9428A410j 2/1/94 13:02:21\""
$ COMPONENT_NAME: CMDAR (ar command)  
$  FUNCTIONS: none  
$  ORIGINS: 27  
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$  combined with the aggregated modules for this product)  
$                   SOURCE MATERIALS  
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$  All Rights Reserved  
$  US Government Users Restricted Rights - Use, duplication or  
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  

$set MS_AR        Redefine set#

$ Do not translate ar.  

AR_CREATING_MSG      "ar: Creating an archive file %s."
$ %s is an archive file.  

ORDERING_AR_MSG      "ar: Sequentially ordering and compressing %s."
$ %s is the archive file.  

AR_NOEXIST_MSG       "ar: 0707-100 %s does not exist."
$ %s is the specified archive file.  

AR_USAGE_MSG         "Usage:\
\tar [-closvCT] {-h|p|t|x} [--] Archive [File ...]\
\n\tar [-closvCT] {-m|r[u]} [{-a|b|i} {PositionName}] [--] Archive File ...\
\n\tar [-closvCT] {-d|q} [--] Archive File ...\
\n\tar [-clvCT] {-o|s|w} [--] Archive"
$               -->Do not translate command name or flags in AR_USAGE_MSG  

BAD_OPTION_MSG       "ar: 0707-101 %c is not a valid flag."
$ %c is a flag that is not valid.  
$ Do not translate flags.  

BAD_HEADER_MSG       "ar: 0707-102 Internal error while reading member header."

COPY_ERR_MSG         "ar: 0707-103 Internal error while copying archive member."

CORRUPT_OBJ_MSG      "ar: 0707-124 The object file %1$s is corrupted.\n\
\tSymbol table entries for %2$s cannot be added to the archive file."
$ %s is a file name.  

FCLEAR_ERR_MSG       "ar: 0707-104 The fclear system call failed."
$ Do not translate fclear.  

FFLUSH_ERR_MSG       "ar: 0707-105 The fflush system call failed."
$ Do not translate fflush.  

FIXHDR_ERR_MSG       "ar: 0707-106 Internal error while reading the fixed\n\
\theader of archive file %s."
$ %s is the archive file.  

FSLIMIT_ERR_MSG      "ar: 0707-107 The file size limit was reached when writing to the archive file.\n\
Use the ulimit command to increase the maximum user file size."

MAGIC_ERR_MSG        "ar: 0707-108 File %s is not an archive file."
$ %s is a file.  

MEM_NOTFOUND_MSG     "ar: 0707-109 Member name %s does not exist."
$ %s is a member name.  

NO_MEM_MSG           "ar: 0707-110 There is not enough memory available now."

NO_MEM_TAB_MSG       "ar: 0707-123 The archive file %1$s is corrupted.  \n\
\tThe member table is missing.\n\
\tThe command 'ar -o %2$s' may restore the member table."
$ The last line is indicating that if the command is tried, the member 
$ table could possibly be restored. 

NO_Q_POS_MSG         "ar: 0707-111 The -a, -b, or -i flag cannot be used with the -d or -q flag."
$ Do not translate a, b, i, d, or q.  

NO_SYM_TAB_MSG       "ar: 0707-112 There is no symbol table in %s.\n\
\tRecreate the symbol table by using the ar command with the -s flag."
$ %s is an archive file  
$ Do not translate -s.  

OPEN_ERR_MSG         "ar: 0707-113 The fopen system call failed on file %s."
$ %s is a file name.  
$ Do not translate fopen.  

READ_ERR_MSG         "ar: 0707-114 The fread system call failed."
$ Do not translate fread.  

SAFE_ERR_MSG         "ar: 0707-115 Internal error while copying the /tmp file\n\
\tto archive file %1$s.\n\
\tA valid copy was saved in temporary file %2$s.\n\
\tMake a copy of this file immediately."
$ Do not translate /tmp  

SEEK_ERR_MSG         "ar: 0707-116 The fseek system call failed."
$ Do not translate fseek.  

STAT_ERR_MSG         "ar: 0707-117 The fopen system call failed on file %s."
$ %s is the file name.  
$ Do not translate fopen.  

SYM_RDERROR_MSG      "ar: 0707-118 Internal error while reading the Global Symbol Table."

SYM_WRERROR_MSG      "ar: 0707-119 Internal error while writing to the Global Symbol Table."

TOO_MANY_CMDS_MSG    "ar: 0707-120 The specified flags are mutually exclusive.\n"

TRUNCATE_ERR_MSG     "ar: 0707-121 The ftruncate system call failed."
$ Do not translate ftruncate.  

WRITE_ERR_MSG        "ar: 0707-122 The fwrite system call failed."
$ Do not translate fwrite.  

EXTRACT_ERR_MSG      "ar: 0707-125 Cannot write over file %s when -C is used."
$ %s is a file name.  
