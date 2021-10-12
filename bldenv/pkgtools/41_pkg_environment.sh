# @(#)52        1.5  src/bldenv/pkgtools/41_pkg_environment.sh, pkgtools, bos412, 9445B.bldenv 11/8/94 10:09:02
######################################################################
#
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Purpose:  environment variables for v4.1 packaging defaults
#
# Usage: ". 41_pkg_environment"
#
######################################################################
BASE_PROD_DIR=${BASE_PROD_DIR:-prod}
PROD_DIRNAME=${PROD_DIRNAME:-/afs/austin/aix/ptf.images/4.1/$BASE_PROD_DIR}
export PROD_DIRNAME
SHIP_DIRNAME=${SHIP_DIRNAME:-/afs/austin/aix/ptf.images/4.1/ship}
export SHIP_DIRNAME
PROD_DIRNAME_PATTERN=${PROD_DIRNAME_PATTERN:-/afs/austin.*/aix/ptf.images/.*/$BASE_PROD_DIR}
export PROD_DIRNAME_PATTERN
XMIT_TABLE=${XMIT_TABLE:-${ODE_TOOLS:-/afs/austin/aix/410/project/aix4/build/latest/ode_tools/power}/usr/lib/xmit_ptf.table}
export XMIT_TABLE
LOG_FILE=${LOG_FILE:-/afs/austin/aix/ptf.images/4.1/$BASE_PROD_DIR/log_file.41}
export LOG_FILE
#### End of 41_pkg_environment ####
