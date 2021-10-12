# @(#)05	1.6  src/bos/etc/security/migration/Makefile, cfgsauth, bos41J, 9518A_all 4/26/95 14:46:13
#
# COMPONENT_NAME: CFGSAUTH
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SCRIPTS			= mkusr_mig secur_mig passwd_mig
SEDSCRIPTS		= events.sed newpaths.sed
PROGRAMS		= login_mrg user_mrg SysckMerge
HFILES			= mrg_util.h

login_mrg_OFILES	= login_mrg.o mrg_util.o
user_mrg_OFILES		= user_mrg.o mrg_util.o

NOUPDATE_LIST 	= /usr/lpp/bos/migrate/SysckMerge \
		  /usr/lpp/bos/migrate/mkusr_mig \
		  /usr/lpp/bos/migrate/secur_mig \
		  /usr/lpp/bos/migrate/passwd_mig \
		  /usr/lpp/bos/migrate/events.sed \
		  /usr/lpp/bos/migrate/newpaths.sed \
		  /usr/lpp/bos/migrate/login_mrg \
		  /usr/lpp/bos/migrate/user_mrg

ILIST			= ${PROGRAMS} ${SEDSCRIPTS} ${SCRIPTS}
IDIR			= /usr/lpp/bos/migrate/

LIBS			= -ls

.include <${RULES_MK}>

