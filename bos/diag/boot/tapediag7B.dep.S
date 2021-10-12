# @(#)17	1.6  src/bos/diag/boot/tapediag7B.dep.S, diagboot, bos41J, 9513A_all 3/21/95 12:54:23
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Diagnostic tape 7B dependency file
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
#	parallel
./usr/lib/methods/cfgppa
./usr/lib/methods/cfgpp
./usr/lib/methos/ucfgpp
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
./usr/lib/drivers/ktsdd
./usr/lib/drivers/tabletdd
./usr/lib/drivers/kbddd
./usr/lib/drivers/mousedd
./usr/lpp/diagnostics/da/dkmta
./usr/lpp/diagnostics/da/dkbd
./usr/lpp/diagnostics/da/dkbdsal
./usr/lpp/diagnostics/da/dkeytaba
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
