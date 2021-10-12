$                    "@(#)79\t1.8  src/bos/usr/ccs/bin/size/size.msg, cmdaout, bos411, 9428A410j 5/18/94 17:43:16" 
$ COMPONENT_NAME: CMDAOUT (size command)  
$  FUNCTIONS: none  
$  ORIGINS: 27  
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$  combined with the aggregated modules for this product)  
$                   SOURCE MATERIALS  
$  (C) COPYRIGHT International Business Machines Corp. 1989  
$  All Rights Reserved  
$  US Government Users Restricted Rights - Use, duplication or  
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$quote "         define quote character.

$len 
$set MS_SIZE        Define initial set#

PRTOTAL_0_MSG        "\n\n\
Size of %s:  0x%lx\n"
$ %s is the file section.  
$ 0x%lx is the file section size in hexadecimal notation. Do not translate. 

PRTOTAL_1_MSG        "\n\n\
Size of %s:  0%lo\n"
$ %s is the file section.  
$ 0%lo is the file section size in octal notation. Do not translate.  

PRTOTAL_2_MSG        "\n\n\
Size of %s:  %ld\n"
$ %s is the file section.  
$ %ld is the file section size in decimal notation. Do not translate.  

USAGE_ERR_MSG        "Usage: size [-o|-x|-d|-f|-V] [File ...] \n"
$ Do not translate command name "size" or options in USAGE_ERR_MSG  

OPTION_ERR_MSG       "size: 0654-301 %c is not a recognized flag.\n"
$ Do not translate size: 
$ %c is the flag that is not recognized by the size command.  

AR_HDR_ERROR_MSG     "size: 0654-302 Cannot size %s.\n\
\tArchive header is not readable.\n"
$ Do not translate size: 
$ %s is the archive file name.  

BAD_MAGIC_MSG        "size: 0654-303 Cannot size %s.\n\
\tSpecify an XCOFF object file or an archive of XCOFF object files.\n"
$ Do not translate size: 
$ %s is the file that is not an object file or an archive  

OPEN_ERR_MSG         "size: 0654-304 Cannot open %s.\n"
$ Do not translate size: 
$ %s is the file name.  

TOTAL_ERROR_MSG      "size: 0654-305 File is corrupted. Cannot determine the total size.\n"
$ Do not translate size: 

SCN_HDR_ERROR_MSG    "size: 0654-306 Cannot read a section header.\n"
$ Do not translate size: 
