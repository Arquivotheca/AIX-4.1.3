# @(#)10        1.47  src/bos/objclass/tapeboot/Makefile, bosboot, bos41J, 9514A_all 4/4/95 11:54:08
# COMPONENT_NAME: BOSBOOT
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# This Makefile creates the ODM that resides in the RAM filesystem
# during tape boot.  This ODM should contain only the .add files for
# tape, disk, display, diskette devices and their parent devices.

VPATH =		..:../${TARGET_MACHINE}

TAPE_LIST =	\
		adapter.mca.8efc.add \
		adapter.mca.8fba.add \
		adapter.mca.8ff4.add \
		adapter.mca.hscsi.add \
		adapter.mca.serdasda.add \
		adapter.mca.sio.add \
		adapter.mca.sio_1.add \
		adapter.mca.sio_2.add \
		adapter.mca.sio_3.add \
		adapter.serdasda.serdasdc.add \
		adapter.sio.8efc.add \
		adapter.sio.fda.add \
		adapter.sio.fda_2.add \
		adapter.sio.fda_iod.add \
		adapter.sio.hscsi.add \
		adapter.sio.s1a.add \
		adapter.sio.s2a.add \
		adapter.sio.s3a.add \
		bus.sys.mca.add \
		cdrom.scsi.scsd.add \
		cfgrule.lft.add \
		cfgrule.lvm.add \
		cfgrule.sys.add \
		cfgrule.tty.add \
		container.cabinet.op_panel1.add \
		container.cabinet.power_supply1.add \
		container.cabinet.sif1.add \
		container.sys.cabinet1.add \
		container.sys.smpdrawer1.add \
		cpucard.sys.cpucard1.add \
		devno.add \
		disk.mca.badisk.add \
		disk.scsi.1000mb.add \
		disk.scsi.1000mb16bit.add \
		disk.scsi.1000mb16bitde.add \
		disk.scsi.1000mbde.add \
		disk.scsi.1100mb.add \
		disk.scsi.1100mb16bit.add \
		disk.scsi.1100mb16bitde.add \
		disk.scsi.1200mb.add \
		disk.scsi.1370mb.add \
		disk.scsi.160mb.add \
		disk.scsi.2000mb.add \
		disk.scsi.2000mb16bit.add \
		disk.scsi.2000mb16bitde.add \
		disk.scsi.2000mbde.add \
		disk.scsi.200mb.add \
		disk.scsi.2200mb.add \
		disk.scsi.2200mb16bit.add \
		disk.scsi.2200mb16bitde.add \
		disk.scsi.320mb.add \
		disk.scsi.355mb.add \
		disk.scsi.400mb.add \
		disk.scsi.4500mb16bit.add \
		disk.scsi.4500mb16bitde.add \
		disk.scsi.540mb.add \
		disk.scsi.670mb.add \
		disk.scsi.730mb.add \
		disk.scsi.857mb.add \
		disk.scsi.osdisk.add \
		disk.scsi.scsd.add \
		disk.serdasdc.2000mb.add \
		disk.serdasdc.serdasdd.add \
		disk.serdasdc.serdasdd2.add \
		disk.serdasdc.serdasdd4.add \
		diskette.siofd.fd.add \
		driver.scsi_scb.vscsi.add \
		gemini.add \
		ioplanar.planar.mcaplanar1.add \
		ioplanar.sys.iodplanar1.add \
		ioplanar.sys.ioplanar.add \
		ioplanar.sys.ioplanar_1.add \
		ioplanar.sys.ioplanar_2.add \
		kbd.add \
		kts.add \
		lft.add \
		logvol.add \
		lvm.add \
		mca_kma.add \
		memory.sys.L2cache.add \
		memory.sys.memory.add \
		memory.sys.simm.add \
		mid.add \
		nep.add \
		planar.sys.sysplanar1.add \
		planar.sys.sysplanar2.add \
		planar.sys.sysplanar3.add \
		planar.sys.sysplanar_p.add \
		processor.sys.proc1.add \
		rwoptical.scsi.scsd.add \
		sga.add \
		sgabus.add \
		skycolor.add \
		skymono.add \
		std_keyboard.add \
		sys.node.sys.add \
		sys.node.sys1.add \
		sys.node.sys_p.add \
		tape.scsi.1200mb-c.add \
		tape.scsi.150mb.add \
		tape.scsi.3490e.add \
		tape.scsi.4mm2gb.add \
		tape.scsi.4mm4gb.add \
		tape.scsi.525mb.add \
		tape.scsi.8mm.add \
		tape.scsi.8mm5gb.add \
		tape.scsi.8mm7gb.add \
		tape.scsi.9trk.add \
		tape.scsi.ost.add \
		tty.add

BUILD_LIST =	Config_Rules \
		CuAt \
		CuAt.vc \
		CuDep \
		CuDv \
		CuDvDr \
		CuVPD \
		PdAt \
		PdAt.vc \
		PdCn \
		PdDv \
		PdDv.vc

NOUPDATE_LIST =	\
	/tmp/tapeboot_IBM/Config_Rules \
	/tmp/tapeboot_IBM/CuAt \
	/tmp/tapeboot_IBM/CuAt.vc \
	/tmp/tapeboot_IBM/CuDep \
	/tmp/tapeboot_IBM/CuDv \
	/tmp/tapeboot_IBM/CuDvDr \
	/tmp/tapeboot_IBM/CuVPD \
	/tmp/tapeboot_IBM/PdAt \
	/tmp/tapeboot_IBM/PdAt.vc \
	/tmp/tapeboot_IBM/PdCn \
	/tmp/tapeboot_IBM/PdDv \
	/tmp/tapeboot_IBM/PdDv.vc

ILIST		= ${BUILD_LIST}

IDIR		= /tmp/tapeboot_IBM/

OTHERS	= temp

.include <${RULES_MK}>

temp	: ${TAPE_LIST} cfgodm.cre
	${RM} -f temp cfgodm.ipl cfgodm.fdb
	${CP} ${cfgodm.cre:P} .
	ODMDIR=. ${ODMCREATE} -c -p cfgodm.cre \
	|| { ${RM} -f cfgodm.cre; ${FALSE} ; }
	${RM} -f cfgodm.cre
	ODMDIR=. ${ODMADD} ${TAPE_LIST:@TMP@${${TMP}:P}@} > /dev/null \
	|| { ${RM} -f ${BUILD_LIST}; ${FALSE} ; }
	${TOUCH} temp
