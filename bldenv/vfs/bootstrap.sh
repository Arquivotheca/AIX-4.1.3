#! /bin/sh
# @(#)66        1.3  src/bldenv/vfs/bootstrap.sh, ade_build, bos412, GOLDA411a 2/8/94 16:12:53
#
#   COMPONENT_NAME: BLDPROCESS
#
#   FUNCTIONS:
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

if [ ! -d ${BLDENV_TOOLS}/etc ]
then
    mkdir -p ${BLDENV_TOOLS}/etc
fi

#
# Install the vfs configuration template file which will be used by the
# install_tools script.
#
cp `srcpath bldenv/vfs/vfs.tmplt` ${BLDENV_TOOLS}/etc

#
# Append the code for creating the vfs configuration file to the end of
# the install_tools script.
#
cat >>${BLDENV_TOOLS}/usr/bin/install_tools <<END_INSTALL_TOOLS
BLDENV_TOOLS=\`pwd\`

sed -e "s|__BUILD__|\${BLDENV_TOOLS}|g" etc/vfs.tmplt >etc/vfs

END_INSTALL_TOOLS

#
# Make sure the install_tools script is executable.
#
chmod +x ${BLDENV_TOOLS}/usr/bin/install_tools
