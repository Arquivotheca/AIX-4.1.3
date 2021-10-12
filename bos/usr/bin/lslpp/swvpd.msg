$  @(#)29  1.26.2.20  src/bos/usr/bin/lslpp/swvpd.msg, cmdswvpd, bos411, 9428A410j 6/6/94 18:21:17
$
$
$   COMPONENT_NAME: CMDSWVPD
$
$ FUNCTIONS: swvpd.msg
$
$ ORIGINS: 27
$
$   (C) COPYRIGHT International Business Machines Corp. 1991,1993
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ Message strings for VPD (Vital Product Data) programs
$quote "
$ This set is for the vpdcreate program
$set 1
MSG_USAGE "usage: %s\n"
MSG_DSKERR "Disk I/O error occurred while creating the table.\n"
MSG_PRMSSN "You are not authorized to create \'%s\'.\n"
MSG_GENERR "An error occurred while creating \'%s\'. \n\
\tThe table may already exist.\n"
ERR_TBLERR "Unknown table name \'%s\'.\n"


$ All sets below here are for lslpp, lppchk, and vpd_perror().
$ This set is error messages that correspond to the VPD_xxx error numbers.
$ If these messages are changed, the corresponding messages in
$ vpd_perror.c must be changed as well.
$quote "
$set MSGS_SWVPD
MSG_SWVPD_OK            "%s: 0504-300  Success.\n"
MSG_SWVPD_SYS           "%1$s: 0504-302  ODM error %2$d occurred trying to read SWVPD data.\n"
MSG_SWVPD_BADCOL        "%s: 0504-304  An unsearchable input was specified for SWVPD data search.\n"
MSG_SWVPD_SQLMAX        "%1$s: 0504-306  The search string limit of %2$d has been exceeded.\n"
MSG_SWVPD_NOMATCH       "%s: 0504-307  No match was found for the SWVPD data search.\n"
MSG_SWVPD_NOID          "%s: 0504-308  Cannot match fileset name with fileset ID.\n"
MSG_SWVPD_UNKNOWN       "%s: 0504-333  SWVPD returned unknown error: %2$d.\n"

$set MSGS_GENERAL
MSG_NOMEM               "%s:  Out of memory.\n"

$ If these are changed, the corresponding #defined constants in swvpd_str.h
$ must be changed.
$quote "
$set MSGS_LSLPP


MSG_LSLPP_SINGLE_ENTRY       "%1$s: 0504-100  -%2$s may only be entered once.\n"
MSG_LSLPP_MISSING_ARGUMENTS  "%1$s: 0504-101  Missing option-arguments with -O.\n"
MSG_LSLPP_INVALID_ARGUMENTS  "%1$s: 0504-102  Invalid option-arguments with -O.\n"
MSG_LSLPP_INVALID_OPTION     "%1$s: 0504-103  -%2$s is an invalid option.\n"
MSG_LSLPP_EXCLUSIVE_FLAGS_OLD "%1$s: \
0504-104  The following flags are mutually exclusive: A,d,f,h,i,l,p.\n"

MSG_LSLPP_COMBINING_J        "%1$s: 0504-106  -J may not be used with -%2$s.\n"
MSG_LSLPP_COMBINING_A_AND_B  "%1$s: \
0504-107  -a may not be used with -%2$s combined with -B.\n"
$ MSG_LSLPP_COMBINING_A_AND_F_NEW was created.

MSG_LSLPP_COMBINING_M        "%1$s: 0504-109  -m may not be used with -%2$s.\n"
MSG_LSLPP_COMBINING_I_AND_B  "%1$s: 0504-110  -I may not be used with -B.\n"
MSG_LSLPP_NO_FIXES_M         "%1$s: \
0504-111  There is no PTF ID with microcode in %2$s\n \
\tthat matches \"%3$s\".\n"
$ %2$s is a path name

MSG_LSLPP_NO_FIXES           "%1$s: \
0504-112  There is no PTF ID in %2$s that matches \n\t\"%3$s\".\n"
$ %2$s is a path name

MSG_LSLPP_NO_LPP_M           "%1$s: \
0504-113  There is no fileset with microcode in %2$s\n \
\tthat matches \"%3$s\".\n"
$ %2$s is a path name

MSG_LSLPP_NO_INPUT_M         "%1$s: \
0504-115  There is no fileset or PTF ID with microcode in\n \
\t%2$s that matches \"%3$s\".\n"
$ %2$s is a path name

MSG_LSLPP_NO_INPUT           "%1$s: \
0504-116  There is no fileset or PTF ID in %2$s\n\tthat matches \"%3$s\".\n"
$ %2$s is a path name

MSG_LSLPP_ALL_NO_FIXES_M     "%1$s: \
0504-117  There is no PTF ID with microcode in %2$s,\n \
\t%3$s, or %4$s that matches \"%5$s\".\n"
$ %2$s %3$s and %4$s are all path names

MSG_LSLPP_ALL_NO_FIXES       "%1$s: \
0504-118  There is no PTF ID in %2$s, %3$s,\n \
\tor %4$s that matches \"%5$s\".\n"
$ %2$s %3$s and %4$s are all path names

MSG_LSLPP_ALL_NO_LPP         "%1$s: 0504-120  There is no fileset in %2$s, %3$s,\n \
\tor %4$s that matches \"%5$s\".\n"
$ %2$s %3$s and %4$s are all path names

MSG_LSLPP_NO_INSTALL         "%1$s: \
0504-124  There is no installation record for this fileset \"%2$s\".\n"
MSG_LSLPP_VPD_ERR            "%1$s: 0504-125  Error while processing fileset %2$s.\n"

MSG_LSLPP_NONE               "NONE"
MSG_LSLPP_NON_IBM            "Vendor Fileset"
MSG_LSLPP_ALL                "all"
MSG_LSLPP_COMBINING_FLAGS    "%1$s: 0504-126  -%2$s may not be used with -%3$s.\n"
MSG_LSLPP_OPTION_ARGS        "%1$s: \
0504-127  The option-argument of -O only accepts one occurrence of %2$s.\n"
MSG_LSLPP_COMBINING_A_AND_F_NEW  "%1$s: 0504-108  -a may not be used with -f.\n"
MSG_LSLPP_NO_LPP_NEW         "%1$s: \
0504-114  There is no fileset in %2$s that matches \n\t\"%3$s\".\n"
$ %2$s is a path name

MSG_LSLPP_ALL_NO_LPP_M_NEW   "%1$s: \
0504-119  There is no fileset with microcode in %2$s,\n \
\t%3$s, or %4$s that matches \"%5$s\".\n"
$ %2$s %3$s and %4$s are all path names

MSG_LSLPP_ALL_NO_INPUT_M_NEW "%1$s: \
0504-121  There is no fileset or PTF ID with microcode in \n \
\t%2$s, %3$s, or %4$s\n\tthat matches \"%5$s\".\n"
$ %2$s %3$s and %4$s are all path names

MSG_LSLPP_ALL_NO_INPUT_NEW   "%1$s: \
0504-122  There is no fileset or PTF ID in %2$s,\n \
\t%3$s, or %4$s that matches \"%5$s\".\n"
$ %2$s %3$s and %4$s are all path names

MSG_LSLPP_PREREQ_COLONS      "At \
least %d of the following must be installed {"
MSG_LSLPP_PREREQA_NEW        "At \
least %d of the following\n  %-*s  %-*s  %-*s  %smust be installed {\n"
$ Maximum screen length for the text is 32 columns

MSG_LSLPP_PREREQ_NEW        "At least %d of the following must be installed {\n"
$ Maximum screen length for the text is 54 columns

MSG_LSLPP_AVAILABLE_ON       "  (Available on %1$s)"
$ %1$s is a description of media (tape, disk, file)

MSG_LSLPP_EXPLAIN_DEPENDS    "<Name> is a requisite of <Dependents>\n"
$ <Name> is referring to the header in HEADER_MSGS, offset 28, the words must
$ match.
$ <Dependents> is referring to the header in HEADER_MSGS, offset 8, the words
$ must match.

MSG_LSLPP_NO_PATH            "%1$s: \
0504-128  %2$s does not exist as a path on your\n \
\tsystem.  This probably means nothing has been installed on that path.\n"

MSG_LSLPP_EXCLUSIVE_FLAGS    "%1$s: \
0504-129  The following flags are mutually exclusive: d,f,h,i,L,l,p.\n"

MSG_LSLPP_USAGE         "Usage: \
lslpp {-d|-f|-h|-i|-L|-l|-p} [-B | -I] [-acJq] [-O{[r][s][u]}]\n \
             [fileset ... | ptf_id ... | all]\n \
\t-a Displays additional (\"all\") information when combined with\n \
\t   other flags.  (Not valid with -f, only valid with -B when\n \
\t   combined with -h)\n \
\t-B Permits PTF ID input.  (Not valid with -L)\n \
\t-c Colon-separated output.  (Not valid with -La)\n \
\t-d Dependents (filesets for which this is a requisite).\n \
\t-f Files that belong to this fileset.\n \
\t-h History information.\n \
\t-I Limits listings to base level filesets (no updates displayed).\n \
\t-i Product Identification information (requested per fileset).\n \
\t-J Use list as the output format.  (Valid with -l and -L)\n \
\t-L Lists fileset names, latest level, states, and descriptions.\n \
\t   (Consolidates usr, root and share part information.)\n \
\t-l Lists fileset names, latest level, states, and descriptions.\n \
\t   (Separates usr, root and share part information.)\n \
\t-O Data comes from [r] root and/or [s] share and/or [u] usr.\n \
\t   (Not valid with -L)\n \
\t-p Requisites of installed filesets.\n \
\t-q Quiet (no column headers).\n"
$ do not translate "lslpp" or any of the flags.

MSG_LSLPP_SUPERSEDED   "%1$s: 0504-130  -%2$s used without -L.\n"

MSG_LSLPP_L_ERROR   "%1$s: 0504-131  -L may not be used with -%2$s. \n"

MSG_LSLPP_PRODUCT_NOT_INSTALLED  "%1$s: 0504-132  Fileset %2$s not installed.\n"

MSG_LSLPP_VPD_CORRUPTED  "%1$s: 0504-133  VPD Corrupted : Could not locate \
                          %2$s for %3$s.\n"

MSG_LSLPP_USAGE_OF_EX_FLAGS "\
\t One of the following mutually exclusive flags: d,f,h,i,L,l,p\n \
\t must be specified.\n"

MSG_LSLPP_LEGEND      "\n\nState Codes: \n \
A -- Applied. \n \
B -- Broken. \n \
C -- Committed. \n \
O -- Obsolete.  (partially migrated to newer version) \n \
? -- Inconsistent State...Run lppchk -v.\n"

MSG_LSLPP_UPDT_PCKG   "Fileset Update"

MSG_LSLPP_SUPERSEDES  "Supersedes:"

MSG_LSLPP_NO_MAINTENANCE_LEVEL  "No Maintenance Level Applied."

MSG_LSLPP_WAIT_PROCESSING "Processing.....Please Wait.\n"

MSG_LSLPP_COMBINING_L_AND_CA  "%1$s: \
0504-134  -a may not be used with -%2$s combined with -L.\n"

MSG_LSLPP_NO_A_FLAG "\
0504-135 The -A flag is not valid.  For information on APARS,\n\
see the \"instfix\" command.\n"

MSG_LSLPP_COMBINING_L_AND_B  "%1$s: \
0504-136  -B may not be used with -L.\n"

MSG_LSLPP_B_NEEDS_PTF_ID  "%1$s: \
0504-137  -B requires one or more PTF IDs (ex. lslpp -B U412345).\n"

MSG_LSLPP_NO_PTF_MATCH "\
%1$s: 0504-138  Fileset with PTF ID %2$s is not installed.\n"

$ If these are changed, the corresponding strings in swvpd_str.h must be
$ changed as well.
$ Also, the corresponding #defined constants in swvpd_str.h must be changed.
$set HEADER_MSGS

1       "State"
$ Maximum screen length is 12 columns

2       "Description"
$ Maximum screen length is 32 columns

3       "Status"
$ Maximum screen length is 10 columns

4       "Action"
$ Maximum screen length is 10 columns

5       "User Name"
$ Maximum screen length is 10 columns

6       "Level"
$ Maximum screen length is 15 columns

7       "Requisites"
$ Maximum screen length is 32 columns

8       "Dependents"
$ Maximum screen length is 20 columns

9       "File"
$ Maximum screen length is 20 columns

10      "Date"
$ Maximum screen length is 9 columns

11      "Product Id"
$ Maximum screen length is 10 columns

12      "Feature Id"
$ Maximum screen length is 10 columns

13      "Package Name"
$ Maximum screen length is 20 columns

14      "Source"
15      "PTF Id"
$ Maximum screen length is 7 columns

16      "Fix Information"
$ Maximum screen length is 32 columns

17      "Dependents Fix"
$ Maximum screen length is 14 columns

18      "Dependents State"
$ Maximum screen length is 16 columns

19      "Time"
$ Maximum screen length is 8 columns

20      "Vendor"
$ Maximum screen length is 6 columns

21      "Vendr"
$ Maximum screen length is 5 columns

22      "Code"
$ Maximum screen length is 5 columns

23      "Package"
$ Maximum screen length is 10 columns

24      "Feature"
$ Maximum screen length is 10 columns

25      "Id"
$ Maximum screen length is 10 columns

26      "Fix State"
$ Maximum screen length is 12 columns

27      "State"
$ Maximum screen length is 12 columns

28      "Fileset"
$ Maximum screen length is 20 columns

29      "Path"

$ If these are changed, the corresponding strings in swvpd_str.h must be
$ changed as well.
$set LSTATE_MSGS
1      "AVAILABLE"
2      "APPLYING"
3      "APPLIED"
4      "COMMITTING"
5      "COMMITTED"
6      "REJECTING"
7      "BROKEN"
8      "DEINSTALLING"
9      "APPLY-HOLD"
10     "COMMIT-HOLD"
11     "OBSOLETE"
LSTATE_MSG_UNKNOWN      "UNKNOWN"

$ If these are changed, the corresponding strings in swvpd_str.h must be
$ changed as well.
$set HSTATE_MSGS
1      "COMPLETE"
2      "PENDING"
3      "BROKEN"
4      "CANCELLED"
5      "DEINSTALLED"
HSTATE_MSG_UNKNOWN     "UNKNOWN"

$ If these are changed, the corresponding strings in swvpd_str.h must be
$ changed as well.
$set EVENT_MSGS
1      "APPLY"
2      "COMMIT"
3      "REJECT"
4      "CLEANUP"
5      "DEINSTALL"
EVENT_MSG_UNKNOWN      "UNKNOWN"

$ inventory file type strings
$ If these are changed, the corresponding strings in swvpd_str.h must be
$ changed as well.
$set INV_FTYPE_MSGS
1      "CONFIGURATION"
2      "SOURCE"
3      "ARCHIVE"
INV_FTYPE_UNKNOWN      "UNKNOWN"

$ inventory format strings
$ If these are changed, the corresponding strings in swvpd_str.h must be
$ changed as well.
$set INV_FORMAT_MSGS
1      "File"
2      "Archive entry"
3      "Object Data Manager"
4      "Data Management Services"
INV_FORMAT_UNKNOWN     "UNKNOWN"


$ If the messages in the following set are changed, the corresponding 
$ strings in lppchk.h must also be changed.
$ messages for the lppchk program
$set MSGS_CHK

MSG_CHK_BAD_REP "\
%1$s: 0504-201  Internal error - unexpected code for database - %2$c.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is an internal character code.

MSG_CHK_BAD_OPT "\
%1$s: 0504-202  Internal error - unexpected function code - %2$c.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is an internal character code.

MSG_CHK_VPDERR "\
%1$s: 0504-203  Error reading lpp database - rc = %2$d, odmerrno = %3$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is return code from an internal routine,
$ Third insertion is the odm error return code

MSG_CHK_UPDATE "\
%1$s: 0504-204  Inventory entry for file %2$s updated\n\
\twith size %3$d and checksum %4$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the name of the file refered to in the inventory
$ Third insertion is the size of that file
$ Fourth insertion is the new checksum value for that file.

MSG_CHK_NOPERM "\
%1$s: 0504-205  Unable to access file %2$s because of permissions.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the file name from the inventory

MSG_CHK_NOFILE "\
%1$s: 0504-206  File %2$s could not be located.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the name of the file referenced from the inventory

MSG_CHK_NEWSZ  "\
%1$s: 0504-207  Size value for %2$s changed from %3$d to %4$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the file name, Third insertion is the size
$ value from the inventory, Fourth insertion is the actual file size.

MSG_CHK_BADSZ  "\
%1$s: 0504-208  Size of %2$s is %3$d, expected value was %4$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the file name, Third insertion is the actual
$ file size, Fourth insertion is the size value found in the inventory.

MSG_CHK_NOCKSUM "\
%1$s: 0504-209  File %2$s has no checksum in the inventory, \n\
\tno verification can be done.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the file name.  Inventory has no value for the
$ checksum and no verification can be done.

MSG_CHK_RDPERM "\
%1$s: 0504-210  Unable to read file %2$s because of permissions.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the file name being accessed.

MSG_CHK_NEWSUM "\
%1$s: 0504-211  Changing checksum for file %2$s, from %3$d to %4$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the file name, Third insertion is the checksum
$ value from the inventory,  Fourth insertion is newly computed value.

MSG_CHK_BADCK "\
%1$s: 0504-212  The checksum for file %2$s is %3$d, \n\
\texpected value is %4$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the file name.  Third insertion is the computed
$ checksum value, Fourth insertion is the checksum value from the
$ inventory.

MSG_CHK_NOPIPE "\
%1$s: 0504-213  Unable to open pipe needed to check archive, error was %2$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is errno value from system 'pipe' command.

MSG_CHK_NOFORK "\
%1$s: 0504-214  Unable to fork process needed to check archive, error was %2$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the errno value from the system 'fork' command.

MSG_CHK_ARERR "\
%1$s: 0504-215  Program %2$s returned the following error text.\n"
$ First insertion is the command name (lppchk),
$ The second insertion is the name of the command used to read the archive
$ member.

MSG_CHK_EXEC  "\
%1$s: 0504-216  Unable to exec the program %2$s, error code %3$d.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the program we tried to exec,  Third insertion
$ is the errno returned by the 'exec' system call.

MSG_CHK_VNOTFND "\
%1$s: 0504-218  The fileset %2$s, level %3$d.%4$d.%5$d %6$s, \n\
\twas present in the %7$s data but not in %8$s. \n"
$ First insertion is the command name (lppchk),
$ Second insertion is the name of the fileset.  The third, fourth
$ fifth and sixth insertions specify the level (version, release,
$ modificaion, fix) of the fileset. The fileset
$ was in the inventory specified by the 7th insertion but not in
$ the inventory specified by the 8th insertion.

MSG_CHK_VSTATE "\
%1$s: 0504-218  The fileset %2$s, level %3$d.%4$d.%5$d %6$s, \n\
\tis not uniformly applied to the system.  It is in \n\
\tstate %7$s in %8$s but in state %9$s in %10$s. \n\
\tThis will probably function properly but should be corrected.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the name of a fileset.  The third, fourth
$ and fifth insertions specify the level (version, release, modification)
$ of the fileset.  Insertion 6 is the PTF identifier for the fileset.  The
$ 7th and 8th insertions specify the status and location where it was
$ found first and the 9th and 10th insertions specify the other status
$ and location.

MSG_CHK_VINST "\
%1$s: 0504-219  The fileset %2$s, level %3$d.%4$d.%5$d %6$s, \n\
\tis not uniformly applied to the system.  It is in \n\
\tstate %7$s in %8$s but in state %9$s in %10$s.\n\
\tThis may cause the system to function incorrectly.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the name of a fileset.  The third, fourth
$ and fifth insertions specify the level (version, release, modification)
$ of the fileset.  Insertion 6 is the PTF identifier for the fileset.  The
$ 7th and 8th insertions specify the status and location where it was
$ found first and the 9th and 10th insertions specify the other status
$ and location.


MSG_CHK_LNOSRC "\
%1$s: 0504-220  No link found from %2$s to %3$s.\n"
$ First insertion is the command name (lppchk),
$ There was no symbolic link from 'insertion 2' to 'insertion 3'

MSG_CHK_LRLERR "\
%1$s: 0504-221  Unable to read existing link at %2$s.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is name of an existing symbolic link file entry

MSG_CHK_LNEQ "\
%1$s: 0504-222  Existing symbolic link at %2$s is not a link to %3$s.\n"
$ First insertion is the command name (lppchk),
$ Insertion 2 and 3 are file names.

MSG_CHK_LNLNK "\
%1$s: 0504-223  Existing entry at %2$s is not a link to %3$s.\n"
$ First insertion is the command name (lppchk),
$ Insertion 2 and 3 are file names

MSG_CHK_LNEWE "\
%1$s: 0504-224  An error occurred while creating the link from %2$s to %3$s. \n\
\tThe error code was %4$d.\n"
$ First insertion is the command name (lppchk),
$ insertions 2 and 3 are file names. Insertion 4 is an errno value.

MSG_CHK_LNEW "\
%1$s: 0504-225  New link created from %2$s to %3$s.\n"
$ First insertion is the command name (lppchk),
$ insertions 2 and 3 are file names.

MSG_CHK_NOLPP "\
%1$s: 0504-226  No fileset entries were found that match the name - %2$s.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the specfied fileset to be processed

MSG_CHK_FC0 "\
%1$s: 0504-227  No files were found that matched the file name given for fileset %2$s.\n"
$ First insertion is the command name (lppchk),
$ Second insertion is the specfied fileset to be processed

MSG_CHK_PRQ "\
%1$s: 0504-228  The requsites for %2$s, %3$d,%4$d.%5$d.%6$d %7$s,\n\
were not satisfied by the current status of the system.\n\
This should be corrected to assure proper operation.\n"

MSG_CHK_QPRQ "\
%1$s: 0504-229  The requsites for %2$s, %3$d,%4$d.%5$d.%6$d %7$s,\n\
were satisfied by the current status of the system but at\n\
least one tested fileset was not committed, while this fileset\n\
is already committed.  This probably needs to be corrected to\n\
ensure proper operation of the system.\n"

MSG_CHK_PROGRESS "\
%1$s: 0504-230  %2$d files have been checked.\n"
$ First insertion is the command name (lppchk)
$ second insertion is the number of files processed thus far.

MSG_CHK_PRQH "\
%1$s: 0504-231 The following inconsistencies were found \n\
\twhile verifying fileset requisites.\n"
$ First insertion is the command name (lppchk)
$ This message will preceed one or more occurances of messages indicating
$ which requisite tests have failed.

MSG_CHK_PRQP "\
\nFileset  %1$s (%2$s part)\n\
at level %3$02d.%4$02d.%5$04d.%6$04d %7$s requires:\n%8$s"
$ First insertion is a fileset name (e.g., bos.net.tcp.client)
$ Second insertion is either 'usr', 'root' or 'share'
$ Insertions 3 through 7 are the level of the fileset as installed.
$ insertion 8 is a list of requisites that were not successfully tested.

MSG_CHK_PRQAL "\
%1$s%2$sAt least %3$d of:\n%4$s"
$ Used to build up a list of requisites when only part of the list
$ is required.  Please do not modify order or spacing of insertions.

MSG_CHK_USAGE0 "\
%1$s Usage:\n\
%2$s -{f|c|l|v} [-u] [-O{[r][s][u]}] [-mn] [fileset [filelist]]\n\
\t-f  Fast check (file existence, file length)\n\
\t-c  Checksum verification\n\
\t-v  Fileset version consistency check\n\
\t-l  File link verification\n\
\t-u  Update inventory (only valid with -c or -l)\n\
\t-O  Data base(s) to be processed, default is all\n\
\t    u = /usr/lib/objrepos, r = /etc/objrepos,\n\
\t    s = /usr/share/lib/objrepos\n\
\t-mn n=1-3 controls detail of messages, 3 is most verbose\n\
\tfileset specifies filesets to be checked, may include\n\
\t    \"*\", etc to specify multiple filesets\n\
\tfilelist one or more file names (optionally using \"*\", etc.)\n\
\t    to be checked.  May be in form  'member:archive' to specify\n\
\t    archive members that are to be checked.\n\
\t--- one and only one of the flags -f, -c, -l and -v may be specified\n\
\t--- filelist is not allowed with -v option\n"


$ If these are changed, the corresponding strings in swvpd_str.h must be
$ changed as well.
$set MEDIA_MSGS
1      "diskette"
2      "tape"
3      "file"
MEDIA_MSG_UNKNOWN      "UNKNOWN"

