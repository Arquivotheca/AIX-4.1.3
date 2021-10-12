$ @(#)60        1.9.1.2  src/bos/usr/bin/tcbck/tcbck.msg, cmdsadm, bos411, 9439B411a 9/27/94 14:31:19
$ COMPONENT_NAME: (CMDSADM) tcbck - system configuration checker
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$quote "

$set MS_SCCS_ID

$set MS_TCBCK

1  "3001-002 A value may not be specified for %s.\n"
2  "3001-003 A value must be specified for %s.\n"

3  "3001-004 Specify only one of the options ALL or tree.\n"
$ Do not translate the words "ALL" or "tree"

4  "3001-005 Do not specify filenames and classes\n\
         with the ALL or tree options.\n"
$ Do not translate the words "ALL" or "tree"

5  "3001-006 Specify only one of the options -p, -n, -y or -t.\n"
6  "3001-007 The entry for %s appears more than once in the database.\n"
7  "3001-008 %s appears in the entries for files %s\n\
         and %s.\n"
8  "3001-009 The database entries for %s\n\
         and %s refer to the same file.\n"
9  "3001-010 The entry for the directory %s may not\n\
         specify hard links.\n"
10 "3001-011 The program %s cannot be executed.\n"
11 "3001-012 The program %s encountered an error during execution.\n"
12 "3001-013 The program %s was not successful\n\
         verifying the file %s.\n"

13 "3001-014 The program %s is not part of the TCB.\n"
$ Do not translate TCB

14 "3001-015 The entry for %s is not valid.\n"
15 "3001-016 The file %s is of an unknown type.\n"

16 "3001-017 The file %s is an unregistered set-UID program.\n"
$ Do not translate set-UID

17 "3001-018 The file %s is an unregistered device.\n"

18 "3001-019 The file %s is an unregistered TCB file.\n"
$ Do not translate TCB

19 "3001-020 The file %s was not found.\n"

20 "3001-021 The file %s does not exist and the entry has\n\
         no source attributes.\n"
$ Do not translate the word "source"

21 "3001-022 The file %s is the wrong file type.\n"
22 "3001-023 The file %s has the wrong file mode.\n"
23 "3001-024 The file %s has the wrong file owner.\n"
24 "3001-025 The file %s has the wrong file group.\n"
25 "3001-026 The file %s has the wrong number of links.\n"

26 "3001-027 The file %s has the wrong TCB attribute value.\n"
$ Do not translate TCB

27 "3001-028 The file %s has the wrong checksum value.\n"
28 "3001-029 The file %s has the wrong access control list.\n"
29 "3001-030 The file %s has the wrong privilege control list.\n"
30 "3001-031 The link from the file %s\n\
         to %s does not exist.\n"
31 "3001-032 The link from the file %s\n\
         to %s should not exist.\n"
32 "3001-033 The file %s is not a link\n\
         to %s.\n"

33 "3001-034 The directory %s must be part of the TCB.\n"
$ Do not translate TCB

34 "3001-035 The file %s must be an absolute path name.\n"
35 "3001-036 The link %s must be an absolute path name.\n"
36 "3001-037 The program %s must be an absolute path name.\n"
37 "3001-038 The name %s is not a known user.\n"
38 "3001-039 The name %s is not a known group.\n"
39 "3001-040 The file %s has a file mode or flag %s\n\
         which is not valid.\n"
40 "3001-041 The file %s has too many links.\n"
41 "3001-042 A request for additional memory failed.\n"

42 "3001-043 Use the tree option to find extra links.\n"
$ Do not translate the word tree

43 "3001-044 Cannot open the installation file %s.\n"
44 "3001-045 Cannot add the vital product data for %s.\n"
45 "3001-046 An error occurred removing the entry for %s.\n"
46 "3001-047 There is no matching file entry for %s.\n"
47 "3001-048 There is no matching class entry for %s.\n"
48 "3001-049 The file %s has the wrong file size.\n"
49 "3001-050 The %s flag must be specified before the permission bits.\n"

50 "3001-051 The file %s has the wrong TP attribute value.\n"
$ Do not translate TP

51 "3001-052 The file %s is an unregistered TP file.\n"
$ Do not translate TP

52 "3001-053 The file %s is an unregistered privileged file.\n"

53 "3001-054 The file %s is an unregistered set-GID program.\n"
$ Do not translate set-GID

54 "3001-055 The attribute %s is not valid for file %s.\n"
55 "3001-056 The entry for %s has a type %s\n\
         which is not valid.\n"
56 "3001-057 The file %s does not exist or the entry has\n\
         no type attribute.\n"
57 "3001-058 Cannot open the vital product database.\n"
58 "3001-059 The copy from file %s\n\
         to %s was not successful.\n"
59 "3001-060 Setting permissions on the file %s\n\
         was not successful.\n"
60 "3001-061 Errors were detected validating file %s.\n"
61 "3001-062 Clear the access control list for %s? "
62 "3001-063 Clear the privilege control list for %s? "
63 "3001-064 Clear the illegal file mode for %s? "
64 "3001-065 Create the directory %s? "
65 "3001-066 Create the link for %s? "
66 "3001-067 Create the file %s from the source location? "
67 "3001-068 Remove the illegal link %s? "
68 "3001-069 Remove the file %s? "
69 "3001-070 Remove the special file %s? "
70 "3001-071 Change the access control list for %s? "
71 "3001-072 Change the privilege control list for %s? "
72 "3001-073 Change the file owner for %s? "
73 "3001-074 Change the file group for %s? "
74 "3001-075 Change the file modes for %s? "
75 "3001-076 Permission was denied for the requested operation.\n"
76 "3001-077 The attribute %s has a value %s which is not valid.\n"
77 "3001-078 An error was encountered updating the attribute %s.\n"
78 "3001-079 The file %s must have an absolute path name\n\
         for the source attribute %s.\n"
79 "3001-080 The file %s has a source file\n\
         %s that does not exist.\n"
80 "3001-081 Errors were detected validating the files\n\
         for package %s.\n"
81 "3001-082 No errors were detected validating the files\n\
         for package %s.\n"
82 "3001-083 The attribute %s for the file %s\n\
         has a value which is not valid.\n"
83 "3001-084 An error was encountered reading the database.\n"
84 "3001-085 The last valid stanza read was %s.\n"
85 "3001-086 No valid stanzas were read.\n"
86 "3001-087 The symbolic link from the file %s\n\
         to %s does not exist.\n"
87 "3001-088 The file %s is not a symbolic link\n\
         to %s.\n"
88 "3001-089 The symbolic link from the file %s\n\
         to %s should not exist.\n"
89 "3001-090 Depending upon where this product was acquired,\n\
         contact a Service Representative or\n\
         the Approved Supplier.\n"
90 "3001-091 The attribute %s must be specified for\n\
         file %s.\n"
91 "3001-092 The creation of file %s was not successful.\n"
92 "3001-093 An error was encountered reading the input file.\n"
93 "3001-094 Add the new entry for file %s? "
94 "3001-095 Add the new link for %s? "
95 "3001-096 Add the new symbolic link for %s? "
96 "3001-097 Add the new file mode for %s? "

97 "The files for package %s are being verified.\n\
This may take several minutes, please wait.\n"
$ This is really message 3001-098.

98 "Tcbck has processed %d files, please wait.\n"
$ Tcbck is the name of the program, do not translate.
$ This is really message 3001-099.

99 "3001-100 File %s is missing the type attribute.\n"
$ "type" is the name of an attribute, do not translate.

100  "3001-001 The value for %s must be true or false.\n"

101 "3001-101 The Trusted Computing Base is not enabled on this machine.\n\
\tTo enable the Trusted Computing Base, you must reinstall and\n\
\tset the 'Install Trusted Computing Base' option to YES.\n\
\tNo checking is being performed.\n"

102	"3001-102 The %s attribute cannot be found in the data base\n\
\tor the attribute has an invalid value. To correct this problem,\n\
\tyou must reinstall the system. No checking is being performed.\n"

103  	"3001-103 Use the -l option to register devices on the Trusted\n\
\t Computing Base.\n"
$ This message is no longer in use.  Message 106 is replacing it.

104	"3001-104 Error opening temporary file\n"

105	"3001-105 Error updating temporary file\n"

106	"3001-106 If the unregistered device is trusted, use the -l option\n\
\t to register the device as part of the Trusted Computing Base.\n"
$ This is a replacement message for the existing Message 103.

$set	MS_TCBCK_USE

1  "Usage: tcbck -a <filename> [ [ <attr> | <attr>=[<value> | ] ] ... ]",
2  "             -a -f <filename>",
3  "             -a sysck [ treeck_novfs=<dir> ] [ checksum=<program> ]",
4  "             -d <filename> | <class> [ <filename> | <class> ] ...",
5  "             -d -f <filename>",
6  "		 -l /dev/<filename> [ /dev/<filename> ] ... ",
7  "             -(p|y|n|t) [ [<filename>|<class>] ... | ALL | tree ]",

$set	MS_TCBCK_ATTR

1  "3001-199 Illegal Attribute, must be one of"
2  "         class=<class_name,...>  - classes file is a member of"
3  "         type                    - FILE, DIRECTORY, etc. (computed)"
4  "         owner                   - owner ID of file (computed)"
5  "         group                   - group ID of file (computed)"
6  "         acl                     - ACL of file (computed)"
7  "         mode                    - Mode bits of file only (computed)"
8  "         links=<filename,...>    - list of hard links to file"
9  "         symlinks=<filename,...> - list of symbolic links to file"
10 "         checksum                - checksum for file (computed)"
11 "         program=\"<program_name>[,<arg,...>]\""
12 "                                 - program to verify file"
13 "         source=[<filename>]     - location of original copy of file"
14 "         size                    - file size (computed)"
15 "         target=[<filename>]     - location of file to link to"
16 " "
17 "         Valid in sysck stanza only"
18 " "
19 "         treeck_novfs=<mount_point,...>"
20 "                                 - list of mount points to ignore"
21 "         treeck_nodir=<directory,...>"
22 "                                 - list of directories to ignore"
23 "         checksum=\"program_name flags\""
24 "                                 - command for computing checksums"
25 "         setuids=<user,...>      - list of administrative users"
26 "         setgids=<group,...>     - list of administrative groups"
