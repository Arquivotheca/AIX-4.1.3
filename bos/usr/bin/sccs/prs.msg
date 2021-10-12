$ static char sccsid[] = "@(#)13  1.8  src/bos/usr/bin/sccs/prs.msg, cmdsccs, bos411, 9428A410j 9/21/93 18:29:09";
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$len 
$ Do not translate prs:
$set 9        Define initial set#

DEFLINE              ":Dt:\t:DL:\n\
MRs:\n\
:MR:COMMENTS:\n\
:C:"
$ Do not translate anything on this line except for Modification Requests and 
$ Comments. Everything else will not be seen by the user and must remain 
$ exactly as it is. 
INVCUTOFF            "prs: 1255-177 Specify the cutoff date in the form: yy[mm[dd[hh[mm[ss]]]]]. (prs4)\n"
$ yy is replaced by a 2 digit number for year. 
$ mm is replaced by a 2 digit number for month. 
$ dd is replaced by a 2 digit number for day. 
$ hh is replaced by a 2 digit number for hour. 
$ mm is replaced by a 2 digit number for minute. 
$ ss is replaced by a 2 digit number for seconds. 
$ Note to translators--Do not change the order of the format. 
$ Do not translate (prs4). 
CANTSPECDATE         "prs: 1255-178 Cannot specify both a cutoff date and an SID. (prs5)\n"
$ Do not translate (prs5). 
$ Do not translate SID. 
SPECEORLWITHC        "prs: 1255-179 Specify either the -e flag or the -l flag with the -c flag. (prs6)\n"
$ This means that either -e or -l must be specified with -c, but not both.
$ Do not translate -e, -l, or -c. They are flags. 
$ Do not translate (prs6) 
NONEMSG              "none"
YESMSG               "yes"
NOMSG                "no"
BRANCH               "branch\n"
CEILING              "ceiling\t%s\n"
$ %s is the release number. 
DEFAULTSID           "default SID\t%s\n"
$ Do not translate SID. 
$ %s is the default SID number. 
FLOOR                "floor\t%s\n"
$ %s is the release number. 
IDKEYWRDERR          "id keywd err/warn\n"
JOINTEDIT            "joint edit\n"
$ A joint edit is a simultaneous edit. 
LOCKRLS              "locked releases\t%s\n"
$ %s is the locked release or releases. 
MODULE               "module\t%s\n"
$ %s is the module name. 
NULLDELTA            "null delta\n"
$ A null delta is an empty delta. 
CSECTNAME            "csect name\t%s\n"
$ %s is text. 
TYPE                 "The  keyword is replaced by \t%s.\n"
$ Do not translate . 
$ %s is a type. 
VALIDATE             "The program that validates Modification Requests is\t"
PRS_USAGE	"Usage: prs [-a] [-d String] [-r[SID] | -c Cutoff] [-e | -l] File...\n"
