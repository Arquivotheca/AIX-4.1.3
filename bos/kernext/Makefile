# @(#)35  1.52 src/bos/kernext/Makefile, Makefiles, bos41J, 9514A_all  4/4/95  13:48:02
#
# COMPONENT_NAME: 
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

EXPLIB_SUBDIRS	= dlc 

EXPINC_SUBDIRS  = lft rcm disp dlc inputdd fddi x25

POWER_SUBDIRS	= c327 cat disk ent fd mpa mpqp dartic msla \
		  psla scsi sol tape tm mps_tok tok dmpa tokdiag entdiag \
		  fddidiag ientdiag inputdd fddi ient dtok32_kext array

SUBDIRS		= trace aio audio lft rcm disp dmx lvm jfsc \
		  inet xns bpf intf rif prnt exp cfs mirror diagex \
		  ${${TARGET_MACHINE}_SUBDIRS} aixif pse tty pty dlc cie \
		  baud mca_async isa_async dlpi

.include <${RULES_MK}>
