# !/bin/sh
# @(#)20        1.3  src/bldenv/compilers/xlC/xlC.sh, ade_build, bos412, GOLDA411a 6/13/94 18:54:12
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
COMPILER_BASE=${ODE_TOOLS:-$BLDENV_TOOLS}
COMPILER=`${ODE_TOOLS}/usr/bin/basename $0`
LANG=En_US  \
LOCPATH=${COMPILER_BASE}/usr/lib/nls/325loc \
LIBPATH=${COMPILER_BASE}/usr/lpp/xlC/lib:/lib:/usr/lib  \
NLSPATH=${COMPILER_BASE}/usr/lib/nls/msg/%L/%N \
${COMPILER_BASE}/usr/bin/compilers/${COMPILER} "$@"
