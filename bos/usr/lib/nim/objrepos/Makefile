# @(#)97 1.15.1.3  src/bos/usr/lib/nim/objrepos/Makefile, cmdnim, bos41J, 9521A_all  5/22/95  15:38:48

#
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./lib/nim/objrepos/Makefile
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
################################################################################

INCLUDES = cmdnim_msg.h cmdnim_msgdefs.h cmdnim_db.h
MSGHDRS  = cmdnim_msg.h

ILIST =	\
	nim_pdattr.vc nim_pdattr 	\
	nim_attr.vc   nim_attr 		\
	nim_object.vc nim_object \
	cmdnim_errors.shh

IDIR = /etc/objrepos/

cmdnim_msg.h_IDIR 		= /usr/include/
cmdnim_msgdefs.h_IDIR 	= /usr/include/
cmdnim_db.h_IDIR 			= /usr/include/
cmdnim_errors.shh_IDIR 	= /usr/lpp/bos.sysmgt/nim/methods/

CATFILES = cmdnim.cat
OTHERS   = cmdnim_db.cre pop cmdnim_errors.shh

cmdnim_db.cre: cmdnim.cre
	${RM} -f ${.TARGET} ${.TARGET:.cre=.c} ${.TARGET:.cre=.h} odm.list
	#
	# create cmdnim.cre
	#
	${CPP}  ${_CC_INCDIRS_} ${_CC_NOSTDINC_} ${_GENINC_} cmdnim.cre > ${.TARGET}
	#
	# odmcreate the object classes 
	#
	ODMDIR=\. ${ODMCREATE} ${.TARGET} > odm.list
	${MV} cmdnim_db.h tmp.h

cmdnim.cre: cmdnim.db.cre 
	#
	# copy: cmdnim.db.cre 
	#
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET} 


cmdnim_errors.shh: cmdnim_msgdefs.h cmdnim_err.awk
	${RM} -f ${.TARGET}
	#
	# build cmdnim_errors.shh
	#
	${AWK} -f cmdnim_err.awk cmdnim_msgdefs.h > cmdnim_errors.shh

cmdnim_err.awk: cmdnim_errors.awk  
	#
	# copy: cmdnim_errors.awk  
	#
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET} 

cmdnim_msgdefs.h: cmdnim_msg cmdnimT.awk 
	${RM} -f ${.TARGET}
	#
	# build cmdnim_msgdefs.h
	#
	${GREP} -E "^#define" cmdnim_msg.h >  cmdnim_msgdefs.h
	${AWK} -f cmdnimT.awk cmdnim_msg   >> cmdnim_msgdefs.h

cmdnimT.awk: cmdnim_T.awk  
	#
	# copy: cmdnim_T.awk  
	#
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET} 

cmdnim_msg: cmdnim.msg  
	#
	# copy: cmdnim.msg  
	#
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET} 

cmdnim_db.h: tmp.h cmdnimdb.awk 
	${RM} -f ${.TARGET}
	#
	# create cmdnim_db.h 
	#
	${AWK} -f cmdnimdb.awk tmp.h > cmdnim_db.h

tmp.h: cmdnim_db.cre 
	

cmdnimdb.awk: cmdnim_db.awk  
	#
	# copy: cmdnim_db.awk  
	#
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET} 

cpp.pd.add: cmdnimstrip.awk cmdnimdb.pd.add
	${CPP}  ${_CC_INCDIRS_} ${_GENINC_} cmdnimdb.pd.add | \
	${AWK} -f cmdnimstrip.awk > cpp.pd.add || exit 1

pop: cpp.pd.add cmdnim.db.cre
	#
	# ------------------ Populate the NIM database ------------------
	#

.if exists(odm.list)
	#
	#  deleting existing objects 
	#
	for i in `${CAT} odm.list`; do /usr/bin/odmdelete -o $$i; done
.endif
	ODMDIR=\. ${ODMADD} cpp.pd.add || exit 1
	${TOUCH} pop


	#
	# ------------------ End of Populate ------------------
	#

cmdnimstrip.awk: cmdnim_strip.awk
	#
	# copy: cmdnim_strip.awk
	#
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET} 

cmdnimdb.pd.add: cmdnim_db.pd.add
	#
	# copy: cmdnim_db.pd.add
	#
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET} 

.include <${RULES_MK}>
