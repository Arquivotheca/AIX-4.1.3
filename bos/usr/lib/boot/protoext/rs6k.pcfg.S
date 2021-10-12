# @(#)58    1.3  src/bos/usr/lib/boot/protoext/rs6k.pcfg.S, bosboot, bos41J, 9514A_all 4/4/95 18:10:28
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
#	NAME:	rs6k.pcfg
#
#	FUNCTION:	This file provides the set of proto extension
#		files needed to be added to the base proto for the
#		rs6k platform. It is used by bosboot to generate the
#		names of the proto extension files. The scheme used in
#		generating the proto extension file names is:
#
#		<base>.proto.ext.<entry-in-this-file>*
#
#		e.g., for base=disk, with entries in this file to be:
#			base.
#			scsi.
#		the proto extension file names generated will be:
#		disk.proto.ext.base.*
#		disk.proto.ext.scsi.*
#
#		NOTE: make sure a dot "." is included if it is necessary
#		to delimit the field.
#
base.
graphics.
mca.
scsi.
sio.
sys.mca.
sys.sga.
sys.sgabus.
tty.
