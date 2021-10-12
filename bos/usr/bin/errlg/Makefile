# @(#)56        1.18  src/bos/usr/bin/errlg/Makefile, cmderrlg, bos411, 9436D411a 9/8/94 16:33:30
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# errids.h is built from errids.desc in usr/include/sys/Makefile

VPATH		= en_US

EXPINC_SUBDIRS	= msg odm libras

EXPLIB_SUBDIRS	=  libras liberrlg 

SUBDIRS		= msg en_US odm libras liberrlg errclear errdead errdemon \
		errlogger errinstall errmsg errpt errstop errupdate smutil \
		convert_errlog samples rd_chkstop

OTHERS		= errtmplt
SCRIPTS		= bos.sysmgt.serv_aid.cron convert_errnotify

ILIST		= ${OTHERS} ${SCRIPTS} codepoint.cat
IDIR		= /var/adm/ras/
bos.sysmgt.serv_aid.cron_IDIR	=	/usr/lpp/bos.sysmgt/serv_aid/
convert_errnotify_IDIR	=	/usr/lpp/bos/migrate/

INCLUDES	= errlg.h codepoint.h
EXPDIR		= /usr/include/

NOUPDATE_LIST = \
    /var/adm/ras/errtmplt \
    /var/adm/ras/codepoint.cat 

.include <${RULES_MK}>

errtmplt:	errtmplt.desc
	${RM} -f ${.TARGET}
	${ERRUPDATE} -q -y errtmplt ${errtmplt.desc:P}
