$ @(#)05        1.8 4/25/91 19:26:37
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$ Do not translate , SCCS, SID, MR
$ Do not translate %s, %c, %1$s, %2$d, etc...
$ Do not translate the parenthesis (co#), or (ut#) at the end of each line.
$len 
$set 3        Define initial set#

NOTSCCS              "\n\
1255-053 Specify an SCCS file with this command. (co1)\n"
MRTHNONELNK          "\n\
1255-054 SCCS files can have only one link or name. (co3)\n"
FRMTERR              "1255-055 There is a format error at line %u. (co4)\n"
PRMTREOF             "\n\
1255-056 Missing newline at the end of the file. (co5)\n"
CORUPTFILE           "\n\
1255-057 The file is damaged. (co6)\n"
PATHTOOLNG           "\n\
1255-058 The maximum path name size is 510 characters. (co7)\n"
INVSID               "\n\
1255-059 Specify a valid SID number. (co8)\n"
$ Do not translate sact, SID, p-file
CLKWRNG              "\n\
1255-060 The creation date of the newest delta in the SCCS file\n\
\tis later than the current time. (co10)\n"
CLKMAYWRNG           "WARNING: The difference between the current date and the creation date of\n\
\tthe newest delta in the SCCS file is greater than 1 year. (co11)\n"
BR                   "1255-062 The 2 limits of a delta range must be in ascending order. (co12)\n"
DLS                  "1255-063 The delta list syntax is not correct. (co13)\n"
NOTAUTH              "\n\
1255-064 You are not authorized to make deltas. (co14)\n"
RLSLTFLR             "1255-065 Release %1$u is less than lowest allowed release %2$u\n\
\t(the floor). (co15)\n"
$ Release %1$u is the specified release. 
$ Release %2$u is the lowest allowed release. 
RLSGTCLNG            "1255-066 Release %1$u is greater than highest allowed release %2$u\n\
\t(the ceiling). (co16)\n"
$ Release %1$u is the specified release. 
$ Release %2$u is the highest allowed release. 
BADPFILE             "\n\
1255-067 The p-file is damaged. (co17)\n"
$ Do not translate p-file.
CANTFORK             "\n\
1255-068 Cannot create another process. (co20)\n"
TOOMANY              "\n\
1255-069  The maximum number of MRs allowed is 64. (co21)\n"
$ Do not translate MRs
FILLCKD              "1255-070 The SCCS file is locked against editing. (co23)\n"
RLSLCKD              "1255-071 Release %d is locked against editing. (co23)\n"
CANTEXEC             "1255-072 Cannot execute %s. (co50)\n"
OTOFSPC              "\n\
1255-073 There is not enough memory available. (ut9)\n"
FFREE                "\n\
1255-074 An attempt to free memory failed.\n"
INEXCNFLT            "An include or exclude conflict %1$s at line %2$u. (%3$s)\n"
$ %1$s will be replaced by the word "begins" or by the word "ends" from the 
$  28 or the BEGINS message. 
$ %u is a line number. 
$ %2$s will be replaced by "(co25)" or "(co26)". 
TYPTERM              "Type %s, terminated with an End of File character\n\
\tor a blank line.\n"
$ %s is replaced by the "comments" message from the delta.msg message catalog. 
$ (MCOMMENTS). 
INCLUDED             "Included:\n"
EXCLUDED             "Excluded:\n"
CORRUPT              "\n\
1255-079 The file is damaged. (co6)\n"
ENDS                 "ends"
$ Note to translator: Please translate this message literally. 
$ This one-word message will replace %1$s in the INEXCNFLT message. 
BEGINS               "begins"
$ Note to translator: Please translate this message literally. 
$ This one-word message will replace %1$s in the INEXCNFLT message. 
LCNTWARN             "WARNING: The number of lines inserted, deleted or unchanged\n\
\texceeds 99999.  The header section of the s. file will record 99999\n\
\tin the appropriate field.\n"
$ The 99999 line limit in one of the statistics field has been exceeded. 
$ The command will succeed, but the field will contain the value 99999. 
