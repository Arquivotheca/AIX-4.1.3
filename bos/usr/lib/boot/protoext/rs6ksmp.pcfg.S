# @(#)59    1.4  src/bos/usr/lib/boot/protoext/rs6ksmp.pcfg.S, bosboot, bos41J, 9514A_all 4/4/95 18:24:11
#
# COMPONENT_NAME: BOSBOOT
#
# FUNCTIONS:  proto config file
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	NAME:	rs6ksmp.pcfg
#
#	FUNCTION:	This file provides the set of proto extension
#		files needed to be added to the base proto for the
#		rs6ksmp platform. It is used by bosboot to generate the
#		names of the proto extension files. The scheme used in
#		generating the proto extension file names is:
#
#		<base>.proto.ext.<entry-in-this-file>*
#
#		e.g., for base=disk, with entries in this file to be:
#			rs6ksmp.base.
#			scsi.
#		the proto extension file names generated will be:
#		disk.proto.ext.rs6ksmp.base.*
#		disk.proto.ext.scsi.*
#
#		NOTE: make sure a dot "." is included if it is necessary
#		to delimit the field.
#
base.com
graphics.
mca.
rs6ksmp.base.
scsi.
sio.
sys.mca.
sys.sga.
sys.sgabus.
tty.
