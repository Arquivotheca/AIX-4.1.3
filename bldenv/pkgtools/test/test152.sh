
cat > defectlist <<!
120060
!

cat > compids.table <<!
test152:575603001:5005:410:410:TX2527:bos410:IBM:1234567:12345678901:
test152.test:575603001:5005:410:410:TX2527:bos410:IBM:1234567:12345678901:
!

cat > Makefile <<!
# @(#)60        1.2  src/bldenv/pkgtools/test/test152.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:10
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
PROGRAMS	= test152
ILIST		= test152
IDIR		= /usr/bin/
.include <\${RULES_MK}>
!
File -create src/bldenv/pkgtools/test/build/test152/Makefile \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f Makefile

cat > Makefile <<!
# @(#)60        1.2  src/bldenv/pkgtools/test/test152.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:10
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
BFF             = test152
ILIST           = ${BFF}
IDIR            = /
OPTIONS         = test152.test
.include <packdep.mk>
.include <\${RULES_MK}>
!
File -create src/packages/test/test152/Makefile \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f Makefile

cat > packdep.mk <<!
# @(#)60        1.2  src/bldenv/pkgtools/test/test152.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:10
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
USR_LIBLPP_LIST         += test152.test.inv_u
ROOT_LIBLPP_LIST        +=
!
File -create src/packages/test/test152/packdep.mk \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f packdep.mk

cat > test152.test.cr <<!
# @(#)60        1.2  src/bldenv/pkgtools/test/test152.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:10
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
File -create src/packages/test/test152/test152.test.cr \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test152.test.cr

cat > test152.test.il <<!
# @(#)60        1.2  src/bldenv/pkgtools/test/test152.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:10
# COMPONENT_NAME: PKGTOOLS
# FUNCTIONS:  inslist
# ORIGINS: 27
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
F 2 2 555	/usr/bin/test152 subsystem152
S 		/usr/bin/test152 /usr/dak/test152_link subsystem152
!
File -create src/packages/test/test152/test152.test.il \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test152.test.il

cat > test152.test.inv_u.S <<!
# @(#)60        1.2  src/bldenv/pkgtools/test/test152.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:10
# COMPONENT_NAME: PKGTOOLS
# FUNCTIONS:  inslist
# ORIGINS: 27
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
S 		/usr/bin/test152 /usr/dak/test152_link subsystem152
!
File -create src/packages/test/test152/usr/test152.test.inv_u.S \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test152.test.inv_u.S

cat > test152.test.lp <<!
# @(#)60        1.2  src/bldenv/pkgtools/test/test152.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:10
# COMPONENT_NAME: PKGTOOLS
# FUNCTIONS:
# ORIGINS: 27
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Material - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
test152 N B En_US Test For Adding New Shipped File
!
File -create src/packages/test/test152/test152.test.lp \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test152.test.lp

cat > test152.c <<!
static char sccsid[] = "@(#)60  1.2  src/bldenv/pkgtools/test/test152.sh, pkgtools, bos412, GOLDA411a 1/3/94 17:52:10";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *   FUNCTIONS: test152
 *   ORIGINS: 27
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
main()
{
   printf("Test 152.c\n");
}
!
File -create src/bldenv/pkgtools/test/build/test152/test152.c \
     -release bos410 \
     -def 120060 \
     -component pkgtools
rm -f test152.c

export origin=`pwd`
export COMPIDS=${origin}/compids.table

workon -sb dkoenig <<!
unset TOSTAGE
cd ..
pwd=\$(pwd)
export TOP=\${pwd}/selfix
export BLDCYCLE=2152A
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
cd src/bldenv/pkgtools/test/build/test152
# Build everything in the build directory.  None of this build should be in
# the lmmakelist files.
unset UPDATE
unset TOSTAGE
build build_all
build TOSTAGE=\${pwd}/ship/power install_all
# Build the build directory with update on.  No files should ship.
export UPDATE=TRUE
export TDPATH=\${TOP}/PTF/\${BLDCYCLE}/bos410/lmmakelist.1
build build_all
build TOSTAGE=\${pwd}/ship/power install_all
# Build everything in the packaging directory.  None of this build should be in
# the lmmakelist files.
cd ../../../../../packages/test/test152
unset UPDATE
unset TDPATH
build build_all
export UPDATE=TRUE
export TDPATH=\${TOP}/PTF/\${BLDCYCLE}/bos410/lmmakelist.2
# Build the packaging directory with update on.  This will be a inslist only
# type change.  Only .il and .inv_u.S files should look like they were 
# dropped with defect.
touch test152.test.il usr/test152.test.inv_u.S
build build_all
bldinit v3bld
bldsetstatus \${_TYPE} \${T_V3BLD} \${_BLDCYCLE} \${BLDCYCLE} \${_LEVEL} test152 \${_RELEASE} \${CMVC_RELEASE} \${_UPDATE} \${U_UPDATE}
subptf
!

rm -f defectlist compids.table
File -undo src/bldenv/pkgtools/test/build/test152/Makefile \
           src/packages/test/test152/Makefile \
           src/packages/test/test152/packdep.mk \
           src/packages/test/test152/test152.test.cr \
           src/packages/test/test152/test152.test.il \
           src/packages/test/test152/usr/test152.test.inv_u.S \
           src/packages/test/test152/test152.test.lp \
           src/bldenv/pkgtools/test/build/test152/test152.c \
     -release bos410 \
     -def 120060
