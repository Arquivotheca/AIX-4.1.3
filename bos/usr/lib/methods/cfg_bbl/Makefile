ILIST		= cfg_bbl __bbl_efc6_post_i
cfg_bbl_IDIR	= /usr/lib/methods/
__bbl_efc6_post_i_IDIR = /usr/lpp/devices.mca.efc6/


OTHERS=

OTHERS+=cfg_bbl
cfg_bbl:cfg_bbl.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=__bbl_efc6_post_i
__bbl_efc6_post_i:__bbl_efc6_post_i.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
