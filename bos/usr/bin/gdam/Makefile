#
#
#
# ORIGINS: 27
#
#


SCRIPTS		=	7135_subsys_config \
	7135_device_config \
	7135_drive_config \
	7135_get_subsys_info \
	7135_get_lun_info \
	7135_get_drive_info \
	7135_subsys_status \
	7135_device_status \
	7135_drive_status \
	7135_change_drive \
	7135_set_subsys_info \
	7135_set_lun_info \
	7135_ctrlr.active.pm \
	7135_ctrlr.dead.pm \
	7135_ctrlr.dualactive.pm \
	7135_ctrlr.passive.pm \
	scsipv.l.pm \
	scsipv.dead.pm \
	scsipv.opt.pm \
	scsipv.spare.pm \
	scsipv.warn.pm \
	scsipv.none.pm \
	lun.opt.pm \
	lun.warn.pm \
	lun.dead.pm \
	adapter.l.bm 


OTHERS1 = raid_disp_opt raid_disp_opt.vc raid_disp_hdr raid_disp_hdr.vc

ILIST = gdam \
	7135_subsys_config \
	7135_device_config \
	7135_drive_config \
	7135_get_subsys_info \
	7135_get_lun_info \
	7135_get_drive_info \
	7135_subsys_status \
	7135_device_status \
	7135_drive_status \
	7135_change_drive \
	7135_set_subsys_info \
	7135_set_lun_info \
	7135_ctrlr.active.pm \
	7135_ctrlr.dead.pm \
	7135_ctrlr.dualactive.pm \
	7135_ctrlr.passive.pm \
	scsipv.l.pm \
	scsipv.dead.pm \
	scsipv.opt.pm \
	scsipv.spare.pm \
	scsipv.warn.pm \
	scsipv.none.pm \
	lun.opt.pm \
	lun.warn.pm \
	lun.dead.pm \
	adapter.l.bm \
	raid_disp_hdr \
	raid_disp_opt \
	raid_disp_hdr.vc \
	raid_disp_opt.vc \

gdam_IDIR= /usr/bin/
7135_subsys_config_IDIR = /usr/bin/
7135_device_config_IDIR = /usr/bin/
7135_drive_config_IDIR = /usr/bin/
7135_get_subsys_info_IDIR = /usr/bin/
7135_get_lun_info_IDIR = /usr/bin/
7135_get_drive_info_IDIR = /usr/bin/
7135_subsys_status_IDIR = /usr/bin/
7135_device_status_IDIR = /usr/bin/
7135_drive_status_IDIR = /usr/bin/
7135_change_drive_IDIR = /usr/bin/
7135_set_subsys_info_IDIR = /usr/bin/
7135_set_lun_info_IDIR = /usr/bin/
7135_ctrlr.active.pm_IDIR = /etc/array/bitmaps/
7135_ctrlr.dead.pm_IDIR = /etc/array/bitmaps/
7135_ctrlr.dualactive.pm_IDIR = /etc/array/bitmaps/
7135_ctrlr.passive.pm_IDIR = /etc/array/bitmaps/
scsipv.l.pm_IDIR = /etc/array/bitmaps/
scsipv.dead.pm_IDIR = /etc/array/bitmaps/
scsipv.opt.pm_IDIR = /etc/array/bitmaps/
scsipv.spare.pm_IDIR = /etc/array/bitmaps/
scsipv.warn.pm_IDIR = /etc/array/bitmaps/
scsipv.none.pm_IDIR = /etc/array/bitmaps/
lun.opt.pm_IDIR = /etc/array/bitmaps/
lun.warn.pm_IDIR = /etc/array/bitmaps/
lun.dead.pm_IDIR = /etc/array/bitmaps/
adapter.l.bm_IDIR = /etc/array/bitmaps/
raid_disp_hdr_IDIR = /etc/objrepos/
raid_disp_opt_IDIR = /etc/objrepos/
raid_disp_hdr.vc_IDIR = /etc/objrepos/
raid_disp_opt.vc_IDIR = /etc/objrepos/

OTHERS=

OTHERS+= ${OTHERS1}
OTHERS+=gdam
OTHERS+=gdam.cat

.include <${RULES_MK}>


${OTHERS1}	: raid_disp.make

raid_disp.make	: raid_disp_hdr.cre raid_disp_hdr.add raid_disp_opt.cre raid_disp_opt.add
		${RM} -f ${OTHERS1}
		${ECHO} creating raid_disp_hdr
		${ODMCREATE} -c ${raid_disp_hdr.cre:P}
		${ODMADD} ${raid_disp_hdr.add:P}
		${ECHO} creating raid_disp_opt
		${ODMCREATE} -c ${raid_disp_opt.cre:P}
		${ODMADD} ${raid_disp_opt.add:P}
		${TOUCH} raid_disp.make


gdam:gdam.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

gdam.cat:gdam.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

