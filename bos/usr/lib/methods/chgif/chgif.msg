$ @(#)03	1.15  src/bos/usr/lib/methods/chgif/chgif.msg, cmdnet, bos411, 9428A410j 2/17/94 10:29:10
$
$   COMPONENT_NAME: cmdnet
$
$   FUNCTIONS: attribute
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Messages for CHGIF.c
$
$quote "
$ Do not translate chgif
$ Message numbers have been assigned a range of 222-232.
$set MS_CHGIF

BADADDCUAT "0821-222 chgif: Cannot add customized attribute.\n"
GETCUAT "0821-223 chgif: Cannot get records from CuAt.\n"
$ Do not translate CuAt.
NOTONEIF "0821-224 chgif: Expected one if instance, retrieved %d.\n"
GETCUDV "0821-226 chgif: Cannot get records from CuDv.\n"
$ Do not translate CuDv.
USAGE1 "\nusage:\t%s  [-d] -l interface [-a attr=val ...] ...\n"
BADATTR "0821-228 chgif: Bad attribute(s) or attribute value(s): %s\n"
IFCFGFAIL "0821-229 chgif: ifconfig command failed.\nThe status of\"%s\" Interface in the current running system is uncertain.\n"
OPENCUDV "0821-230 chgif: Cannot open CuDv.\n"
MASKCHANGED "chgif: invalid netmask value %s changed to %s\n"
GETMASK "chgif: error getting netmask attribute for %s\n"
PUTMASK "chgif: error saving changed netmask value\n"
