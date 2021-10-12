# @(#)84	1.14  src/bos/diag/boot/config4.dep.S, diagboot, bos41J, 9525A_all 6/16/95 16:12:04
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Configuration dependency file
#	     This file has all the methods and drivers needed
#	     to configure all Commo devices.
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	enet
./usr/lib/asw/geteibid.com
./usr/lib/methods/cfgent
./usr/lib/methods/ucfgent
./usr/lib/drivers/entdd
./usr/lib/drivers/eth_demux
./usr/lib/drivers/cfgdmxeth
#	integrated Ethernet
./usr/lib/methods/cfgeth
./usr/lib/drivers/ethdd
#	mpqp
./usr/lib/methods/cfgmpqp
./usr/lib/methods/cfgmpqd
./usr/lib/methods/defmpqd
./usr/lib/drivers/mpqpdd
./usr/lib/asw/mpqpasw
#	token ring
./usr/lib/methods/cfgtok
./usr/lib/methods/ucfgtok
./usr/lib/methods/cfgdmxtok
./usr/lib/drivers/tok_demux
./usr/lib/drivers/tokdd
./usr/lib/microcode/8fc8m.00.02
./usr/lib/microcode/8fc8p.00.01
#	mps
./usr/lib/methods/cfgmps
./usr/lib/drivers/mpsdd
#	mpa
./usr/lib/methods/cfgmpaa
./usr/lib/methods/cfgmpa
./usr/lib/drivers/mpadd
#	x25
./usr/lib/methods/cfgampx
./usr/lib/methods/cfgapm
./usr/lib/methods/cfgx25
./usr/lib/methods/chggen
./usr/lib/microcode/f0efd.00.02
#       fddi
./usr/lib/methods/cfgfddi
./usr/lib/methods/ucfgfddi
./usr/lib/drivers/fddidd
./usr/lib/microcode/8ef4m.02.04
./usr/lib/methods/cfgdmxfddi
./usr/lib/drivers/fddi_demux
#       370pc
./usr/lib/methods/cfgcat
./usr/lib/drivers/catdd
./usr/lib/microcode/fe92d.00.01
./usr/lib/microcode/fe92.00.03
./usr/lib/microcode/cu3088b.00.00
./usr/lib/microcode/cu3088e.00.00
./usr/lib/microcode/cu3172b.00.00
#	Artic
./usr/lib/microcode/708fd.00.00
#	3270
./usr/lib/methods/cfg3270
./usr/lib/drivers/c327dd
#	gio, dials, lpfks
./usr/lib/methods/cfggio
./usr/lib/methods/cfggiodev
./usr/lib/drivers/giodd
./usr/lib/methods/cfgsgio
./usr/lib/methods/startsgio
./usr/lib/drivers/sgiodd
#	Optics 2 port
./usr/lib/methods/cfgsol
./usr/lib/drivers/soldd
./usr/lib/drivers/solddpin
#	parallel
./usr/lib/methods/cfgppa
./usr/lib/methods/cfgpp
./usr/lib/drivers/ppdd
./usr/lib/drivers/ppddpin
