#!/bin/sh
# @(#)08        1.2  src/bldenv/compilers/xlC/create_cfgs.sh, ade_build, bos41J, 9511A_all 3/3/95 09:05:27
#
#   COMPONENT_NAME: ADE_BUILD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
BLDENV_TOOLS=`pwd`
BASE=`dirname ${BLDENV_TOOLS}`
BASE=`dirname ${BASE}`
target_machine=power

#
# if building on AIX Version 3, use -qarch=pwr to build tools
#
if [ "`uname -v`" = "3" ]
then
    other_deflt_options="-qarch=pwr"
else
    other_deflt_options="-qarch=com"
fi

#
# Create the C++ configuration files.
#

sed -e "s|__OTHER_DEFAULT_OPTIONS__|${other_deflt_options}|g" \
    -e "s|__XLC_PATH__|${BLDENV_TOOLS}|g" \
    -e "s|__BASE__|${BASE}|g" \
    etc/boot.xlC.cfg.tmplt >etc/bootstrap.xlC.cfg

sed -e "s|__OTHER_DEFAULT_OPTIONS__|${other_deflt_options}|g" \
    -e "s|__XLC_PATH__|${BLDENV_TOOLS}|g" \
    -e "s|__BASE__|${BASE}|g" \
    etc/bldenv.xlC.cfg.tmplt >etc/bldenv.xlC.cfg

sed -e "s|__XLC_PATH__|${BLDENV_TOOLS}|g" \
    -e "s|__BASE__|${BASE}|g" \
    -e "s|__TARGET_MACHINE__|${target_machine}|g" \
    etc/xlC.cfg.tmplt >etc/xlC.cfg

sed -e "s|__XLC_PATH__|${BLDENV_TOOLS}|g" \
    -e "s|__BASE__|${BASE}|g" \
    -e "s|__TARGET_MACHINE__|${target_machine}|g" \
    etc/dce.xlC.cfg.tmplt >etc/dce.xlC.cfg

#
# Create the fortran configuration files
#

sed -e "s|__XLF_PATH__|${BLDENV_TOOLS}|g" \
    -e "s|__OTHER_DEFAULT_OPTIONS__|${other_deflt_options}|g" \
    etc/boot.xlf.cfg.tmplt >etc/bootstrap.xlf.cfg

sed -e "s|__BUILD__|${BLDENV_TOOLS}|g" \
    -e "s|__OTHER_DEFAULT_OPTIONS__|${other_deflt_options}|g" \
    etc/bldenv.xlf.cfg.tmplt >etc/bldenv.xlf.cfg

sed -e "s|__BUILD__|${BLDENV_TOOLS}|g" \
    -e "s|__TARGET_MACHINE__|${target_machine}|g" \
    etc/xlf.cfg.tmplt >etc/xlf.cfg

#
# Create the vfs configuration file
#

sed -e "s|__BUILD__|${BLDENV_TOOLS}|g" etc/vfs.tmplt >etc/vfs

