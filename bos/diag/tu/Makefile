# @(#)75	1.44  src/bos/diag/tu/Makefile, cmddiag, bos41J, 9520B_all 5/17/95 16:39:02
# COMPONENT_NAME: CMDDIAG
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# If the tu code is building a library for use by the Diagnostic Application,
# then add the tu directory here. 
EXPLIB_SUBDIRS  = 370pca artic bblue corv dca eth ethsal ethstw fd \
		  gem gio iopsl kbd lega msla mouse niokta ped4 scsi sgasl \
		  sky sol swmono tablet tok mps wga iop \
		  siosl ruby nep baud pcitok gga bloom ktat
# NOTE: siosl MUST be last after kbd, tablet

# If the tu code is shipping any file, then also add the tu directory here. 
# This includes things like interrupt handlers, catalog files, microcode
# or anything else.
SUBDIRS		= 370pca artic corv gio kbd lega ped4 tablet mps siosl pcitok \
		  bloom ktat

.include <${RULES_MK}>
