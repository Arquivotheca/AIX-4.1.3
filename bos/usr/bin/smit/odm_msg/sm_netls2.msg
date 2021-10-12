$ @(#)48	1.1.1.1  src/bos/usr/bin/smit/odm_msg/sm_netls2.msg, netls, bos411, 9431A411a 8/3/94 16:15:30
$
$ COMPONENT_NAME:  (cmdnetls) netls.45
$
$ FUNCTIONS: (none)
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ Warning -- do not remove the "$quote "" line below.
$            It is needed to keep gencat from failing when this
$            set is merged with other sets that use other quote characters.

$ $ netls messages
$set 54
$quote "
$
$ The messages in this set are used in the smit scripts for creating and
$ changing AIX floating user licenses.
$ 
$ All messages in this set are ADMIN messages.
$ All messages in this set should be translated.
$ Do not translate words enclosed by double-quotes.
$ Do not translate path names or file names.
$
$ The term "license" refers to a usage allowance for a software application.
$ When a user purchases a software application, he receives a "license" to
$ use the application.  In a network, licenses can be installed on a server
$ and distributed to clients as they to need them to run applications.
$ Normally the vendor of a software product will give the end-user the
$ licenses when the software is purchased.
$
$ The following words have special technical meanings and should not be
$ translated unless there is an equivalent term in the target language:
$   "compound"       (as in "compound license") -- A "compound license" is
$                    a special license that allows a user to create his own 
$		     licenses.
$   "floating user"  (as in "floating user license") -- a "floating user
$		     license" is a license that can be used anywhere on
$		     a network.  It "floats" to where it is needed.
$   "Targetid"       A "targetid" is an identifying attribute of a machine
$		     where licenses will be installed.
$   "Timestamp"      A "timestamp" is a moment in time that something occurs.
$		     Timestamps are used to distinguish between occurences
$		     of the similar events.

1 "You must be root to run this program.  Login as \"root\" or use the \"su\" command\nbefore running this program.\n"

2 "The license server was not found to be running on this machine.  If the\nlicense server is already configured on this machine, then simply run the\nprogram \"/usr/lib/netls/bin/srv_start\" to start your daemons.  If the\nlicense server has not yet been configured on this machine, then go to the\ndirectory \"/usr/lib/netls/conf\" and run the program \"netls_config\".  This\nwill generate the program \"netls_first_time\", which you must then run to\nstart your license server properly.\n"

3 "Installing a compound license on this machine...\n"

4 "The compound license is already installed.\n"

5 "The compound license was successfully installed.\n"

6 "The compound license was not installed.  Please verify that your license\nserver is configured and running, and try again.\n"

7 "Checking for floating user licenses already installed on this machine...\n"

8 "Floating user licenses are already installed on this machine.  Use the SMIT\nscreen for \"Change the Number of Installed Floating User Licenses\" to update\nyour license count.\n"
$ In this message "Change the Number of Floating User Licenses" is a SMIT
$ menu, and should be translated to match the SMIT header.

9 "No licenses currently installed.\n"

10 "Licenses = "

11 "Targetid = "

12 "The licenses are being appended to the file \"/usr/lib/netls/conf/AIX.FLOAT\".\n"

13 "Your licenses were successfully created.  Adding licenses to the server...\n"

14 "Your licenses were not successfully added.\n"

15 "Check the file \"/usr/lib/netls/conf/AIX.FLOAT\" for details.\n"

16 "Your licenses were successfully added.\n"

17 "Your licenses were not created.  Check the \"/usr/lib/netls/conf/AIX.FLOAT\" file\nfor error messages.\n"

18 "Deleting the floating user licenses previously installed on this machine...\n"

19 "Timestamp = "

20 "No floating user license could be found installed with the specified\ntimestamp.\n"

21 "The floating user license previously installed on this machine could not be\ndeleted.  See the file \"/usr/lib/netls/conf/AIX.FLOAT\" for details.\n"

22 "Creating and adding the new floating user licenses...\n"

23 "The floating user license previously installed on this machine has been\ndeleted.\n"

$ The following messages are for the server SMIT stanzas
24 "Manage AIX Floating User Licenses for this Server"
25 "Register Floating User Licences"
26 "Change the Number of Installed Floating User Licenses"
27 "Number of Licenses to Install"
28 "AIX User License to Change"
29 "New Number of Licenses"

