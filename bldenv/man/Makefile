#
# @(#)00 1.5 src/bldenv/man/Makefile, pkgtools, bos412, GOLDA411a 6/28/94 11:36:29
#
#  COMPONENT_NAME: (pkgtools) Aix Build/Packaging/Distribution Tools
#
#  FUNCTIONS: Makefile for man pages
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1993
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

OTHERS			= AddAbstractAsSymptom \
		BIGxref ChangeAbstracts ChangeMemos ChangeSymptoms \
		CheckStatus CheckSymptom DeleteStatus ENV_variables \
		OVERVIEW OrderImages QueryStatus SetStatus adecopyright \
		adeinv adelppname adepackage adeprodid bldCMVCfamily \
		bldabstracts bldgetfamily bldhostsfile bldlock bldlog \
		bldloginit bldlogset bldmid bldnodenames bldperllog \
		bldsetstatus bldstatus bldtrackingdefect.dat \
		bldtrackingdefect ilverify makecdfs makediagboot \
		makestack mkodmextract mkodmupdt odmupdate prebuild \
		processStanza ptfpkg showlog showstatus stack_32 subptf

ILIST			=${OTHERS} 

IDIR			=/usr/share/man/


.include <${RULES_MK}>

.SUFFIXES: .man
.man:
	${CPP} -P ${.OODATE} 2>/dev/null | ${SED} -e "/^#/d" >${.TARGET} \
	|| { ${RM} ${.TARGET}; ${FALSE};}
