$ @(#)60	1.9  src/bos/usr/bin/touch/touch.msg, cmdscan, bos411, 9428A410j 10/21/92 11:48:36
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.
$len
$set MS_TOUCH        Define initial set#
$ Do not translate touch, %s.

TUSAGE               "Usage: touch [-amcf] [mmddhhmm[yy]] [-t [[CC]YY]MMDDhhmm[.SS]] \n\t\t[-r ref_file] File ...\n"
$       Do not translate touch.
$       Do not translate the flags.
$       mm is represents a  2-digit number for month.
$       dd is represents a  2-digit number for day.
$       hh is represents a  2-digit number for hour.
$       mm is represents a  2-digit number for minute.
$       yy is represents a  2-digit number for year.
$	MM - The month of the year (01-12).
$	DD - The day of the month (01-31).
$	hh - The hour of the day (00-23). 
$	mm - The minute of the hour (00-59)
$	CC - The first two digits of the year (the century).
$	YY - The second two digits of the year.
$	SS - the second of the minute (00-61).
TCONV                "touch: 0652-045 The access or modification time format is: [mmddhhmm[yy]].\n"
$       mm is represents a  2-digit number for month.
$       dd is represents a  2-digit number for day.
$       hh is represents a  2-digit number for hour.
$       mm is represents a  2-digit number for minute.
$       yy is represents a  2-digit number for year.
TCREATE              "touch: 0652-046 Cannot create %s.\n"
TSTAT                "touch: 0652-047 Cannot get status on %s.\n"
TCHTIME              "touch: 0652-048 Cannot change the modification time on %s.\n"
SPECTIME	"touch: 0652-141 The specified time format is: [[[CC]YY]MMDDhhmm[.SS]].\n"
$	MM - The month of the year (01-12).
$	DD - The day of the month (01-31).
$	hh - The hour of the day (00-23). 
$	mm - The minute of the hour (00-59)
$	CC - The first two digits of the year (the century).
$	YY - The second two digits of the year.
$	SS - the second of the minute (00-61).
