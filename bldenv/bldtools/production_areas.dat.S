#
# @(#)50        1.6  src/bldenv/bldtools/production_areas.dat.S, bldprocess, bos412, GOLDA411a 11/23/93 09:02:55
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: None
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: production_areas.dat
#
# FUNCTION: Contains default list of areas prebuild will allow to do
#           production builds.
#
# FORMAT: Area names are enclosed in single quotes (').  Areas are
#         separated by a comma (,).  All areas must be on one line.
#         Blank lines and those beginning with a # are ignored.
#
# EXAMPLE: 'AUS/D05','AUS/D05S'
#
# All Alphabetic characters will be converted to uppercase by prebuild
# before being used.
#
# The area names are those listed in a 'Report -view Users' command.
#

'AUS/D66','AUS/D66S','AUS/D05','AUS/D05S','AUS/j64s','AUS/e86s'
