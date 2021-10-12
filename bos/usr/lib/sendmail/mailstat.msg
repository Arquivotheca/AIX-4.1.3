$ @(#)41	1.3  src/bos/usr/lib/sendmail/mailstat.msg, cmdsend, bos411, 9428A410j 7/16/91 10:13:34
$ 
$ COMPONENT_NAME: CMDSEND mailstat.msg
$ 
$ FUNCTIONS: 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for mailstats
$

$set MS_MAILSTAT

$quote "

USAGE           "usage: mailstats [-S<stats file path>] [-z]\n"
EXCESS          "mailstats: excess flag \"%s\"\n"
INVALID         "mailstats: invalid flag \"%s\"\n"
UNKNOWN         "mailstats: unknown flag \"%s\"\n"
UNKPARM         "mailstats: unknown parameter \"%s\"\n"
NODATA          "No statistics data in file \"%s\"\n"
SIZE            "mailstats: file size change; use previous mailstats version\n"
STATFILE        "Sendmail statistics from file \"%s\"\n"
COLLECT         "    Collection started at %s\n"
HEADER          "Mailer          msgs_from     bytes_from      msgs_to       bytes_to\n"
LINES           "------------    ---------     ----------    ---------     ----------\n"
