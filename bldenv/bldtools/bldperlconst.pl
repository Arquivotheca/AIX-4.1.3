#
# @(#)58	1.7  src/bldenv/bldtools/bldperlconst.pl, bldtools, bos412, GOLDA411a 11/17/92 10:58:02
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldperlconst
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991,1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

######################## constants for perl programs #########################
$MAXRECORDSIZE=1024;  # max record size for database files (mapped AAs)
$PSEP="|";  # primary field seperator
$SEP=",";  # field seperator
$STATUS="statusfunctions -t 180";  # lock constants
$MAXITERATIONS = 50000;  # maximum dependents for any file
$DEFINED=1;
$SUCCESS=0;
$FAILURE=1;
$FATAL=2;
$TRUE=1;
$FALSE=0;
$DONE=0;
$NOTDONE=1;

$_TYPE="-a";
$_BLDCYCLE="-b";
$_RELEASE="-c";
$_SUBTYPE="-i";
$T_PREBUILD="prebuild";
$S_DELTAEXTRACT="cmvcextract";
