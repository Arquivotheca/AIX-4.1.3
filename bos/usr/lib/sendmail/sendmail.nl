#  aix_sccsid[] = "src/bos/usr/lib/sendmail/sendmail.nl, cmdsend, bos411, 9428A410j AIX 6/15/90 23:25:54"
# 
# COMPONENT_NAME: CMDSEND sendmail.nl
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#  Created: 03/24/89, INTERACTIVE Systems Corporation
#
###################################################################
#
#  This file contains lists of regular expressions which are 
#  compared against the destination address when sending out
#  mail to other systems.  Each comma-separated list is preceeded
#  by either "NLS:" or "8859:".  Addresses which match an item
#  in these lists will have the body of the mail item encoded
#  as either NLS escape sequences or ISO 8859/1 characters,
#  respectively.
#
#  Before each address is compared with these lists it is passed
#  through ruleset 7, which normally strips the user information
#  from uucp-style addresses and route and user information from
#  domain-style addresses.  
#
#  The following example lists shows how this file might look:
#
#	#list the nls compatible systems
#	NLS:	^@.*madrid\.,
#		^@rome,
#		^@.*italy\.europe$,
#		!nagasaki!$,
#		lisbon!,
#		munich,
#		berlin
#
#	#list the ISO-8859 compatible systems
#	8859:	.*vienna!$,
#		^@bangkok\.thailand,
#		^@tangiers,
#		^@kinshasa
#
#  Note that all dots "." in an address must be escaped with 
#  a backslash "\".  All standard regular expression rules apply.
#  Several of the above examples are not recommended for actual 
#  use but are shown to give some idea of the flexibility that 
#  is allowed.  For example, the first example "madrid" will match 
#  any domain-style address which has this name as a subdomain.  
#  A more likely example of this type is "italy.europe" which will 
#  match all domains that are in that sub-domain.  The examples 
#  "munich" and "berlin" are not recommended forms either since they
#  will match a variety of addresses.
#
#  This file must be compiled using the "-bn" option before sendmail 
#  can use it.  It is recommended that this file be tested against
#  common addresses using the "-br" option to verify that addresses
#  are being interpreted correctly.
#  
###################################################################
