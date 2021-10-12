# @(#)07	1.13  src/bos/diag/boot/tape7.dep.S, diagboot, bos41J, 9525A_all 6/16/95 16:12:12
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Diagnostic tape 7A dependency file
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	gio, dials, lpfks
./usr/lib/methods/cfggio
./usr/lib/methods/cfggiodev
./usr/lib/drivers/giodd
./usr/lib/methods/cfgsgio
./usr/lib/methods/startsgio
./usr/lib/drivers/sgiodd
./usr/lpp/diagnostics/da/dgioa
./usr/lpp/diagnostics/catalog/default/dgioa.cat
./usr/lpp/diagnostics/da/dlpfk
./usr/lpp/diagnostics/catalog/default/dlpfk.cat
./usr/lpp/diagnostics/da/ddials
./usr/lpp/diagnostics/catalog/default/ddials.cat
./usr/lpp/diagnostics/bin/ucfgdev
./usr/lpp/diagnostics/catalog/default/ucfgdev.cat
./usr/lpp/diagnostics/slih/gio_slih
#       async
./usr/lib/methods/rsddload
./usr/lib/methods/cxload
./usr/lib/methods/cfgasync
./usr/lib/methods/ucfgasync
./usr/lib/methods/cfgtty
./usr/lib/methods/chgtty
./usr/lib/methods/ucfgtty
./usr/lib/methods/starttty
./usr/lib/drivers/liondd
./usr/lib/drivers/rsdd
./usr/lib/drivers/tioc
./usr/lib/drivers/ldterm
./usr/lib/drivers/pse/pse
./usr/lpp/diagnostics/da/dasync
./usr/lpp/diagnostics/catalog/default/dasync.cat
# 	128 port
./usr/lbin/tty/stty-cxma
./usr/lib/methods/cfgsync
./usr/lib/methods/ucfgsync
./usr/lib/methods/cfgcxma
./usr/lib/methods/ucfgcxma
./usr/lib/methods/chgcxma
./usr/lib/drivers/cxmadd
./usr/lib/microcode/cxbios.02.09
./usr/lib/microcode/cxfep.02.09
#	parallel
./usr/lib/methods/cfgppa
./usr/lib/methods/cfgpp
./usr/lib/methods/ucfgpp
./usr/lib/drivers/ppdd
./usr/lib/drivers/ppddpin
./usr/lpp/diagnostics/da/dpp
./usr/lpp/diagnostics/catalog/default/dpp.cat
#	mouse, tablet, keyboard
./usr/lib/methods/cfgkma
./usr/lib/methods/cfgktsm
./usr/lib/methods/cfgkbd_std
./usr/lib/methods/cfgmouse_std
./usr/lib/methods/cfgtablet_std
./usr/lib/methods/ucfgktm_std
./usr/lib/drivers/kbddd
./usr/lib/drivers/ktsdd
./usr/lib/drivers/tabletdd
./usr/lib/drivers/mousedd
./usr/lpp/diagnostics/da/dkeytaba
./usr/lpp/diagnostics/da/dkmta
./usr/lpp/diagnostics/da/dkbd
./usr/lpp/diagnostics/da/dkbdsal
./usr/lpp/diagnostics/slih/kbd_slih
./usr/lpp/diagnostics/catalog/default/dkbd_a.cat
./usr/lpp/diagnostics/catalog/default/dkbd.cat
./usr/lpp/diagnostics/catalog/default/tab_a.cat
./usr/lpp/diagnostics/da/dtablet
./usr/lpp/diagnostics/da/dtabletsal
./usr/lpp/diagnostics/catalog/default/dtablet.cat
./usr/lpp/diagnostics/catalog/default/mouse_a.cat
./usr/lpp/diagnostics/da/dmouse
./usr/lpp/diagnostics/catalog/default/mouse.cat
