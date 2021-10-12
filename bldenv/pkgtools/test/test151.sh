
cat > defectlist <<!
120060
!

cat > compids.table <<!
test151:575603001:5005:410:410:TX2527:bos410:IBM:1234567:12345678901:
test151.test:575603001:5005:410:410:TX2527:bos410:IBM:1234567:12345678901:
!

cat > Makefile <<!
# @(#)59        1.2  src/bldenv/pkgtools/test/test151.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:06
#   COMPONENT_NAME: PKGTOOLS
#   FUNCTIONS:
#   ORIGINS: 27
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#   (C) COPYRIGHT International Business Machines Corp. 1992,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
PROGRAMS	= test151
ILIST		= test151
IDIR		= /usr/bin/
.include <\${RULES_MK}>
!
File -create src/bldenv/pkgtools/test/build/test151/Makefile \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f Makefile

cat > Makefile <<!
# @(#)59        1.2  src/bldenv/pkgtools/test/test151.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:06
# COMPONENT_NAME: PKGTOOLS
# FUNCTIONS: Makefile
# ORIGINS: 27
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
BFF             = test151
ILIST           = ${BFF}
IDIR            = /
OPTIONS         = test151.test
.include <packdep.mk>
.include <\${RULES_MK}>
!
File -create src/packages/test/test151/Makefile \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f Makefile

cat > packdep.mk <<!
# @(#)59        1.2  src/bldenv/pkgtools/test/test151.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:06
# COMPONENT_NAME: PKGTOOLS
# FUNCTIONS: packaging definition
# ORIGINS: 27
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# OBJECT CODE ONLY SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# *_LIBLPP_LIST contains the list of files that need to be added to
#               the respective liblpp.a other than the copyright, .al,
#               .tcb, .size, and .inventory files.  For example, any
#               config scripts or prereq files would be included here.
#               The variable may be left blank if there are no additional
#               files, but the option will exist.
#
INFO_FILES              +=
USR_LIBLPP_LIST         +=
ROOT_LIBLPP_LIST        +=
!
File -create src/packages/test/test151/packdep.mk \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f packdep.mk

cat > test151.test.cr <<!
# @(#)59        1.2  src/bldenv/pkgtools/test/test151.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:06
# COMPONENT_NAME: PKGTOOLS
# FUNCTIONS: copyright
# ORIGINS: 27
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
# Licensed Material - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# NOTE:  This copyright is used for updates. It should be kept in sync
#        with the install copyright which is in com/bosinst/bosrvg.sh!
%%_IBMa
%%_ATTa
%%_OSFa
%%_MITa
%%_BSDa
%%_SUNa
%%_MENTATa
%%_ISCa
!
File -create src/packages/test/test151/test151.test.cr \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test151.test.cr

cat > test151.test.il <<!
# @(#)59        1.2  src/bldenv/pkgtools/test/test151.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:06
# COMPONENT_NAME: PKGTOOLS
# FUNCTIONS:  inslist
# ORIGINS: 27
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
F 2 2 555       /usr/bin/test151 subsystem151
!
File -create src/packages/test/test151/test151.test.il \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test151.test.il

cat > test151.test.lp <<!
# @(#)59        1.2  src/bldenv/pkgtools/test/test151.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:06
# COMPONENT_NAME: PKGTOOLS
# FUNCTIONS:
# ORIGINS: 27
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Material - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
test151.test N B En_US Test For Adding New Shipped File
!
File -create src/packages/test/test151/test151.test.lp \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test151.test.lp

cat > test151.c <<!
static char sccsid[] = "@(#)59  1.2  src/bldenv/pkgtools/test/test151.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:06";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: test151
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
main()
{
   printf("Test 151.c\n");
}
!
File -create src/bldenv/pkgtools/test/build/test151/test151.c \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test151.c

export origin=`pwd`
export COMPIDS=${origin}/compids.table

workon -sb dkoenig <<!
unset TOSTAGE
cd ..
pwd=\$(pwd)
export TOP=\${pwd}/selfix
export BLDCYCLE=2151A
export CMVC_RELEASE=bos410
export STATUS_FILE=\${TOP}/HISTORY/STATUS_FILE.\${BLDCYCLE}
rm -f \${STATUS_FILE}
touch \${STATUS_FILE}
rm -fr \${TOP}/PTF/\${BLDCYCLE}
rm -fr \${TOP}/LOG/\${BLDCYCLE}
bldstatus -o \${BLDCYCLE}
export BUILD_TYPE=areabuild
export ODE_TOOLS=\${pwd}/link/ode_tools/power
export ADECOPYRIGHT="\${ODE_TOOLS}/usr/bin/adecopyright -f \${ODE_TOOLS}/usr/lib/copyright.map -t \${COMPIDS}"
export ADELPPNAME="\${ODE_TOOLS}/usr/bin/adelppname -c \${COMPIDS}"
export SHIP=\${pwd}/ship/power:\${pwd}/link/ship/power
. \${ODE_TOOLS}/usr/bld/bldkshconst
. \${ODE_TOOLS}/usr/bld/bldinitfunc
ptfsetup -n -r bos410 -f \${origin}/defectlist
/afs/austin/u1/dkoenig/bin/track.ext bos410 120060 \${pwd}
cd src/bldenv/pkgtools/test/build/test151
build TOSTAGE=\${pwd}/ship/power install_all
export UPDATE=TRUE
export TDPATH=\${TOP}/PTF/\${BLDCYCLE}/bos410/lmmakelist.1
build build_all
build TOSTAGE=\${pwd}/ship/power install_all
cd ../../../../../packages/test/test151
export UPDATE=TRUE
export TDPATH=\${TOP}/PTF/\${BLDCYCLE}/bos410/lmmakelist.2
build build_all
bldinit v3bld
bldsetstatus \${_TYPE} \${T_V3BLD} \${_BLDCYCLE} \${BLDCYCLE} \${_LEVEL} test151 \${_RELEASE} \${CMVC_RELEASE} \${_UPDATE} \${U_UPDATE}
subptf
!

rm -f defectlist compids.table
File -undo src/bldenv/pkgtools/test/build/test151/Makefile \
           src/packages/test/test151/Makefile \
           src/packages/test/test151/packdep.mk \
           src/packages/test/test151/test151.test.cr \
           src/packages/test/test151/test151.test.il \
           src/packages/test/test151/test151.test.lp \
           src/bldenv/pkgtools/test/build/test151/test151.c \
     -release bos410 \
     -def 120060
