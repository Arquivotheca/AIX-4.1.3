# @(#)20        1.4  src/bos/usr/lib/nim/README.S, cmdnim, bos41J, 9524A_all 6/12/95 09:28:05
#
# COMPONENT_NAME:  (CMDNIM)
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

CHANGES TO NIM IN AIX 4.1.3
___________________________
 
CHANGES TO NIM CLIENT NETWORK ADAPTER HARDWARE ADDRESS SPECIFICATION
 
NIM now permits 0 to be specified as a network adapter hardware address when
defining NIM clients.  This is helpful when the client machine is not active
and the hardware address cannot be obtained or when defining a large number
of clients.  If a 0 is used to specify a client's network adapter
hardware address, BOOTP requests initiated from the IPLROM menus from a
system console on the client must have the IP address specified.  A BOOTP
request that broadcasts packets without an IP address will not work for a NIM
client which has been defined with a network adapter address of 0.

All machines in the NIM environment which serve SPOTs to clients that are
defined with a network adapter address of 0 must have the bos.sysmgt.nim.client 
fileset installed at version 4.1.3.0 or greater.
 

ENHANCEMENTS TO THE NIM BOS_INST OPERATION
 
These are the new attributes for the NIM bos_inst operation.

 
force_push
 
The force_push attribute, when assigned a value of yes, tells NIM that the
target of the bos_inst operation is a machine running AIX 3.2 or AIX Version
4 and that the client does not necessarily have the bos.sysmgt.nim.client
fileset installed and configured.  Accordingly, NIM will attempt to nfs mount
or copy the minimal client support to the target system to perform an
unattended installation or migration of the base operating system.  If client
support is copied to the target machine, filesystems on the target will be
automatically expanded unless the auto_expand attribute to bos_inst is set to
no.  Use of the force_push attribute requires that the client grant root rsh
permissions to the master and that the key on the client be in the normal
position.  The force_push attribute also requires that a bosinst_data file be
allocated to the target machine to indicate that an no-prompt installation
should take place.  Finally, this function requires that a network adapter 
logical device name be defined as the fourth field of the "if" attribute 
of the target NIM object if the target has more than one interface of a
given type.  (e.g. if1="net1 sailfish.austin.ibm.com 0 tok0" where
net1 is the name of the NIM network to which the target is connected, sailfish 
is the hostname of the target machine, 0 has been specified as the network 
adapter hardware address, and tok0 is the network adapter logical device name.)

This function will not set up and migrate a machine from AIX 3.1 to AIX 
Version 4 and is not capable of providing unattended installation of 
hardware systems which do not support modification of the boot device 
list via the AIX bootlist command.  

When using SMIT to perform the bos_inst operation, the force_push attribute 
is set to yes by setting "Force Unattended Installation Enablement?" to yes.
 

no_client_boot
 
The no_client_boot attribute, when assigned a value of yes, is used to
instruct NIM not to attempt to initiate the bos installation on the target
machine after setting up the installation with the bos_inst operation.  This
serves the purpose of allowing a bos installation to be set up while
deferring the actual installation until the client is rebooted at a later
time.  Additionally, if the client is not a running machine, this attribute
will avoid having to wait on the reboot attempt to time-out or fail.  If the 
installation of the client system is going to be initiated later from the 
server then the normal mode boot device list on the client must be set so
that a network boot is attempted when the client is rebooted.  No attempt is 
made to modify the boot list when no_client_boot is set to yes unless the 
force_push or set_bootlist (described below) attributes are specified and 
set to a value of yes. 

When using SMIT to perform the bos_inst operation, the no_client_boot attribute 
is set to yes by setting "Initiate Boot Operation on Client?" to no.
 

set_bootlist
 
The set_bootlist attribute can be used in conjunction with the no_client_boot
attribute to modify the boot device list on the client for normal mode so that
a network boot is attempted when the client is rebooted.  It is not necessary 
to specify the set_bootlist attribute if the force_push attribute is set to 
yes or if no_client_boot is unspecified or set to no.  In both cases, the 
default action is to modify the boot list.  The only valid values for 
set_bootlist are yes and no.  In order to use the set_bootlist attribute on 
bos_inst operations the bos.sysmgt.nim.client fileset must be installed at 
version 4.1.3.0 or greater on target clients.
 
When using SMIT to perform the bos_inst operation, the set_bootlist attribute 
is set to yes by setting "Set Boot List if Boot not Initiated on Client?" to
yes.
 
** IMPORTANT REMINDER ON bosinst_data FILES FOR UNATTENDED INSTALLATION
**
   In order for an installation to continue in unattended mode, it is necessary
   that the information in the bosinst_data file be sufficient and valid.  If
   there are errors in the bosinst_data file, the installation will leave 
   no-prompt mode and require attention at the console.

   The console must be filled in and valid.  Run lscons on the target system to
   get a valid console value.  See the comments in /var/adm/ras/bosinst.data and
   the installation guide for information on other required fields.

   Here are the currently valid values available for locale information:

   BOSINST_LANG:     <blank>,C,de_DE,en_US,es_ES,fr_FR,it_IT,nl_BE,sv_SE

   CULTURAL_CONVENTION: 
                     <blank>,C,Ar_AA,Iw_IL,Ja_JP,ZH_CN,ar_AA,bg_BG,ca_ES,
                     cs_CZ,da_DK,de_CH,de_DE,el_GR,en_GB,en_US,es_ES,
                     fi_FI,fr_BE,fr_CA,fr_CH,fr_FR,hr_HR,hu_HU,is_IS,
                     it_IT,iw_IL,ja_JP,ko_KR,mk_MK,nl_BE,nl_NL,no_NO,
                     pl_PL,pt_BR,pt_PT,ro_RO,ru_RU,sh_SP,sk_SK,sl_SI,
                     sr_SP,sv_SE,tr_TR,zh_CN,zh_TW

   MESSAGES:         <blank>,C,Ja_JP,de_DE,en_US,es_ES,fr_FR,it_IT,ja_JP,
                     ko_KR, nl_BE,no_NO,sv_SE,zh_TW

   KEYBOARD:         <blank>,C,Ar_AA,Iw_IL,Ja_JP,ZH_CN,ar_AA,bg_BG,cs_CZ,da_DK,
                     de_CH, de_DE,el_GR,en_GB,en_US,es_ES,fi_FI,fr_BE,fr_CA,
                     fr_CH,fr_FR,hr_HR,hu_HU,is_IS,it_IT,iw_IL,ja_JP,ko_KR,
                     mk_MK,nl_BE,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ro_RO,ru_RU,
                     sh_SP,sk_SK,sl_SI,sr_SP,sv_SE,tr_TR,zh_CN,zh_TW

THE NIM REBOOT OPERATION
 
In AIX 4.1.3, NIM provides a reboot operation which can be used to reboot a
NIM client from the master.  If a bos installation has been previously
set up on the target, NIM will issue a warning message with the wall command 
informing users on the target that re-installation is about to occur.  The
warning can be suppressed by setting the inst_warning attribute to no when
the reboot command is issued.
 
 
ENHANCEMENTS TO THE NIM CUST OPERATION
 
In AIX 4.1.3, NIM provides the fixes attribute to the cust operation to
manage software updates in a NIM environment.  The fixes attribute can be
used to specify a fix or set of fixes to be installed without knowing any
information other than the Authorized Program Analysis Report (APAR) number
or other unique keywords that identify the fix.  When the fixes attribute is
set to the special keyword update_all, all software installed on the target
SPOT or client will be updated with applicable updates from the installation
media or lpp_source specified.
 
The fix_bundle resource may be used to specify a list of APARs or keywords to
the cust operation.  Like the installp_bundle resource the file should
contain one keyword per line.
 
In order to use the the fixes attribute or fix_bundle resource to update
software the bos.sysmgt.nim.client fileset must be installed at version
4.1.3.0 or greater on target NIM clients.
 
 
THE NIM FIX_QUERY OPERATION
 
In AIX 4.1.3, NIM provides the fix_query operation as an interface to the
instfix command to provide the ability to list information about installed
fixes.  The fix_query operation can be applied to SPOT resources or NIM
clients.  When invoked without any optional attributes information about 
all installed fixes are displayed on the target.  The fixes attribute and 
fix_bundle resource are available to the fix_query operation to install 
particular fixes.  

In order to use the the fix_query operation the bos.sysmgt.nim.client fileset
must be installed at version 4.1.3.0 or greater on target NIM clients.

EXAMPLES

1.  To specify a network address of 0 when defining a standalone machine, 
mac1, which has a hostname of redfish on the network net1, enter the following:

    nim -o define -t standalone -a if1="net1 redfish 0" -a ring_speed=16 mac1

2. To perform an unattended migration from AIX version 3.2 to AIX version 4, 
using the force_push attribute, define the target machine as a NIM client
(mac1) on the master and create the required resources for a bos 
installation (SPOT and lpp_source) plus a bosinst_data file that defines a
no prompt bos installation.  Then perform the following steps:
 
    o Add a "<hostname> root" entry for the master in the /.rhosts file on
      the target machine.
    o Ensure that the key on the target is in the Normal position.
    o Allocate the required resources to the client (SPOT, lpp_source, 
      bosinst_data).
    o Invoke the bos_inst operation with the force_push attribute:
          nim -o bos_inst -a force_push=yes mac1

3. To set up a bos installation on a machine that is not currently running
without waiting for attempts to modify the bootlist and reboot the client 
to time-out, enter the following:

    nim -o bos_inst -a no_client_boot=yes mac1

4. To set up a bos installation that is to be initiated at a later time on 
the running NIM client mac1, allocate all required and optional resources 
then enter the following on the master:

    nim -o bos_inst -a no_client_boot=yes -a set_bootlist=yes mac1

5. To reboot and start a bos installation on a client, mac1, which has 
previously been set up for bos installation, enter the following on 
the master:

    nim -o reboot mac1

6. To update all of the software that is currently installed on a client, mac1,
with the latest applicable software residing in the lpp_source, 413images,
enter the following:

    nim -o allocate -alpp_source=413images mac1
    nim -o cust -a fixes=update_all mac1

7. To update the software installed in a SPOT, 41spot, with applicable updates
associated with an APAR, IX12345, from the lpp_source, 413images, enter 
the following:

    nim -o cust -a fixes=IX12345 -a lpp_source=413images 412spot

8. To install all updates applicable to software installed on a machine,
mac1, that are associated with APARs listed in file /tmp/apar.list, using
the lpp_source, 413images, enter the following:

    nim -o define -t fix_bundle -a location=/tmp/apar.list \
                                -a server=master \
                             my_fix_bundle
    nim -o allocate -a fix_bundle=my_fix_bundle -a lpp_source=413images mac1
    nim -o cust mac1

9.  To list the installation status of fixes identified by an APAR, IX12345, 
in a SPOT, 41spot, enter the following:

        nim -o fix_query -a fixes=IX12345 41spot
  
