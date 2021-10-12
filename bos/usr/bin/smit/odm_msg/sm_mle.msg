$  @(#)01  1.11  src/bos/usr/bin/smit/odm_msg/sm_mle.msg, cmdmle, bos411, 9428A410j 6/12/94 19:27:21
$
$   COMPONENT_NAME: cmdmle
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "      define quote character.
$set 53

$ SMIT labels for Manage Language Environment
$
$ note that the following messages numbers do not exist:
$   3,6,12,16,18,21,23,24,25,27,33,34,36,37,38,39,40,
$   41,42,49,53,54,55
$

1   "Change/Show Primary Language Environment"
2   "Select Set of Cultural Convention, Language, and Keyboard"
4   "Change/Show Cultural Convention, Language, or Keyboard"
5   "Select SET\n     (cultural convention, language and keyboard)"
7   "Selected set's CODESET"
8   "Selected set's CULTURAL convention"
9   "Selected set's LANGUAGE translation"
10  "Selected set's KEYBOARD"
11  "INPUT device/directory for software"
12  "EXTEND file systems if space needed?"
13  "Primary CULTURAL convention"
14  "Primary LANGUAGE translation" 
15  "Primary KEYBOARD"
17  "Add Additional Language Environments"
19  "CULTURAL convention to install"
20  "LANGUAGE translation to install"
22  "Remove Language Environments"
26  "Change/Show Language Hierarchy"
28  "1ST language translation choice" 
29  "2ND language translation choice"
30  "3RD language translation choice"
31  "Change/Show Applications for a Language"
32  "List Applications Installed for a Language"
35  "Add Language for Application Already Installed"
43  "Installed APPLICATION"
44  "Remove Language for Applications"
45  "LANGUAGE translation to remove"
46  "CULTURAL convention to remove"
47  "Installed LANGUAGE translation"
48  "Selected APPLICATION"
50  "Convert System Messages and Flat Files"
51  "Convert System Messages"
52  "Convert Flat Files"


$
$ Error messages (referenced in sm_mle.add and/or lsmle.c):

53  "Error:  Empty locale string."
54  "Error:  Language codeset does not match keyboard and cultural\n\tconvention."
55  "Error:  Keyboard and cultural convention codesets do not match."
56  "Error:  Cultural convention or keyboard information missing in ODM."
57  "Error:  Language information missing in ODM."
58  "Error:  Keyboard information missing in ODM."
59  "Error:  Font information missing in ODM."
60  "undefined"
65  "ALL"
70  "Error:  The selected settings for cultural convention, language,\n\
        and keyboard require the installation of additional filesets\n\
        that are not currently installed.  Select an installation device\n\
        that contains the following filesets:\n\n"
75  "Error:  You must be logged in as root to run this command.\n"
$   Do not translate 'root'.
